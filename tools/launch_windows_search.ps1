<#
.SYNOPSIS
Launch File Manager with private, explicitly owned Windows search services.
.DESCRIPTION
Root is mandatory. Live name/path search is default. IndexEnabled opts into an
initial catalogue scan and private store for exact criteria. Ordinary text uses
bounded live name/path matching even with an index.
No SCM/autostart or persistent environment changes. Close File Manager to stop
both owned services. CheckOnly validates startup and tears down without opening UI.
#>
[CmdletBinding()]
param(
    [Alias('PackageRoot')][string]$PackageDirectory = $PSScriptRoot,
    [Parameter(Mandatory = $true)][string]$Root,
    [switch]$IndexEnabled,
    [string]$StateDirectory,
    [switch]$CheckOnly
)

$ErrorActionPreference = 'Stop'
if ([Environment]::OSVersion.Platform -ne 'Win32NT') { throw 'This launcher requires Windows.' }

function Quote-Argument([string]$Value) {
    $escaped = [regex]::Replace($Value, '(\\*)"', '$1$1\"')
    return '"' + [regex]::Replace($escaped, '(\\+)$', '$1$1') + '"'
}
function Start-Owned([string]$Program, [string[]]$Arguments, [string]$Name, [switch]$Visible) {
    $quoted = @($Arguments | ForEach-Object { Quote-Argument $_ })
    $style = if ($Visible) { 'Normal' } else { 'Hidden' }
    return Start-Process -FilePath $Program -ArgumentList $quoted -WorkingDirectory $packagePath `
        -WindowStyle $style -PassThru -RedirectStandardOutput (Join-Path $runPath "$Name.stdout.log") `
        -RedirectStandardError (Join-Path $runPath "$Name.stderr.log")
}
function Wait-Discovery($Process, [string]$Directory) {
    $deadline = [DateTime]::UtcNow.AddSeconds(310)
    while (-not (Test-Path -LiteralPath (Join-Path $Directory 'discovery.json'))) {
        if ($Process.HasExited) { throw "Service exited before discovery; inspect private logs in $runPath" }
        if ([DateTime]::UtcNow -ge $deadline) { throw "Service startup timed out; inspect $runPath" }
        Start-Sleep -Milliseconds 200
    }
}
function Stop-Owned($Process, [string]$Program, [string[]]$Arguments, [string]$Name) {
    if ($null -eq $Process -or $Process.HasExited) { return }
    try {
        $stopper = Start-Owned $Program $Arguments "$Name-shutdown"
        if (-not $stopper.WaitForExit(8000)) { $stopper.Kill(); $stopper.WaitForExit() }
        $stopper.Dispose()
        if ($Process.WaitForExit(8000)) { return }
    } catch { Write-Warning "Graceful $Name shutdown failed: $_" }
    Write-Warning "Stopping only this launcher's owned $Name process after its shutdown deadline."
    if (-not $Process.HasExited) { $Process.Kill(); $Process.WaitForExit() }
}

$packagePath = (Resolve-Path -LiteralPath $PackageDirectory).ProviderPath
$rootPath = (Resolve-Path -LiteralPath $Root).ProviderPath
if (-not (Test-Path -LiteralPath $rootPath -PathType Container) -or $rootPath.StartsWith('\\')) {
    throw 'Root must be an explicitly chosen existing local directory.'
}
$application = Join-Path $packagePath 'File Manager.exe'
$engine = Join-Path $packagePath 'components\fileman-engine.exe'
$orchestrator = Join-Path $packagePath 'components\orchestrator.exe'
foreach ($binary in @($application, $engine, $orchestrator)) {
    if (-not (Test-Path -LiteralPath $binary -PathType Leaf)) { throw "Missing packaged binary: $binary" }
}
if ([string]::IsNullOrEmpty($StateDirectory)) {
    $StateDirectory = Join-Path ([IO.Path]::GetTempPath()) ('fileman-search-' + [Guid]::NewGuid().ToString('N'))
}
if (-not [IO.Path]::IsPathRooted($StateDirectory)) { throw 'StateDirectory must be absolute and new.' }
$runPath = [IO.Path]::GetFullPath($StateDirectory)
$rootPrefix = $rootPath.TrimEnd('\') + '\'
if ($runPath.Equals($rootPath, [StringComparison]::OrdinalIgnoreCase) -or
    $runPath.StartsWith($rootPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'StateDirectory must be outside the searched root; choose another local location.'
}
if (Test-Path -LiteralPath $runPath) { throw 'StateDirectory must be new; existing state is never overwritten.' }
if (-not (Test-Path -LiteralPath (Split-Path $runPath -Parent) -PathType Container)) {
    throw 'StateDirectory parent must already exist.'
}
New-Item -ItemType Directory -Path $runPath | Out-Null
$sid = [Security.Principal.WindowsIdentity]::GetCurrent().User
$acl = New-Object Security.AccessControl.DirectorySecurity
$acl.SetOwner($sid)
$acl.SetAccessRuleProtection($true, $false)
$rule = New-Object Security.AccessControl.FileSystemAccessRule($sid, 'FullControl', 'ContainerInherit,ObjectInherit', 'None', 'Allow')
$acl.AddAccessRule($rule)
Set-Acl -LiteralPath $runPath -AclObject $acl

$engineRuntime = Join-Path $runPath 'engine-runtime'
$orchestratorRuntime = Join-Path $runPath 'orchestrator-runtime'
$manifest = Join-Path $runPath 'policy\manifest.json'
$rootId = 'chosen-root'
$engineProcess = $null
$orchestratorProcess = $null
$applicationProcess = $null
$priorRuntime = $env:FILEMAN_ORCHESTRATOR_RUNTIME_DIR
try {
    $manifestArguments = @('create-windows-manifest', '--deployment-id', 'explicit-file-manager-launch',
        '--root-id', $rootId, '--root-path', $rootPath, '--runtime-dir', $engineRuntime, '--output', $manifest)
    if ($IndexEnabled) { $manifestArguments += @('--index-enabled', '--store-root', (Join-Path $runPath 'store')) }
    $creator = Start-Owned $engine $manifestArguments 'manifest'
    if (-not $creator.WaitForExit(30000)) { $creator.Kill(); throw 'Manifest creation timed out.' }
    if ($creator.ExitCode -ne 0) { throw "Root admission failed; inspect private logs in $runPath" }
    $creator.Dispose()
    Write-Host "Private run state: $runPath"
    if ($IndexEnabled) { Write-Host 'Index enabled for exact criteria; text search uses live filename/path matching.' }
    else { Write-Host 'Live name/path substring search; no persistent catalogue.' }
    $engineProcess = Start-Owned $engine @('serve-windows', '--manifest', $manifest) 'engine'
    Wait-Discovery $engineProcess $engineRuntime
    $orchestratorProcess = Start-Owned $orchestrator @('serve-local', '--runtime-dir', $orchestratorRuntime,
        '--engine-runtime-dir', $engineRuntime) 'orchestrator'
    Wait-Discovery $orchestratorProcess $orchestratorRuntime
    $env:FILEMAN_ORCHESTRATOR_RUNTIME_DIR = $orchestratorRuntime
    $probe = Start-Owned $orchestrator @('call-local', 'status', '--json') 'status'
    if (-not $probe.WaitForExit(15000)) { $probe.Kill(); throw 'Authenticated startup check timed out.' }
    if ($probe.ExitCode -ne 0) { throw 'Authenticated startup check failed.' }
    $probe.Dispose()
    if (-not $CheckOnly) {
        $applicationProcess = Start-Owned $application @('--root', $rootPath, '--engine-root-id', $rootId) 'frontend' -Visible
        $applicationProcess.WaitForExit()
        if ($applicationProcess.ExitCode -ne 0) { throw "File Manager exited with code $($applicationProcess.ExitCode); inspect $runPath" }
    }
    Write-Host 'Explicit launch completed; stopping its services.'
} finally {
    $env:FILEMAN_ORCHESTRATOR_RUNTIME_DIR = $priorRuntime
    if ($null -ne $applicationProcess -and -not $applicationProcess.HasExited) {
        [void]$applicationProcess.CloseMainWindow()
        if (-not $applicationProcess.WaitForExit(8000)) {
            Write-Warning 'Closing the owned frontend after its close deadline.'
            $applicationProcess.Kill()
            $applicationProcess.WaitForExit()
        }
    }
    Stop-Owned $orchestratorProcess $orchestrator @('call-local', 'shutdown', '--runtime-dir', $orchestratorRuntime, '--json') 'orchestrator'
    Stop-Owned $engineProcess $engine @('call-local', '--runtime-dir', $engineRuntime, '--authority', 'admin',
        '--request', '{"id":"launcher-stop","method":"engine.shutdown","params":{}}') 'engine'
    foreach ($process in @($applicationProcess, $orchestratorProcess, $engineProcess)) {
        if ($null -ne $process) { $process.Dispose() }
    }
}
