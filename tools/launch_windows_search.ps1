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

class LaunchContext {
    [string]$PackagePath
    [string]$RunPath
    LaunchContext([string]$PackagePath, [string]$RunPath) {
        $this.PackagePath = $PackagePath
        $this.RunPath = $RunPath
    }
}
function Quote-Argument([string]$Value) {
    [string]$escaped = [regex]::Replace($Value, '(\\*)"', '$1$1\"')
    [string]$trailingEscaped = [regex]::Replace($escaped, '(\\+)$', '$1$1')
    [string]$quoted = '"' + $trailingEscaped + '"'
    return $quoted
}
function Start-Owned([LaunchContext]$Context, [string]$Program, [string[]]$Arguments, [string]$Name, [switch]$Visible) {
    [string[]]$quoted = [string[]]::new($Arguments.Length)
    for ([int]$index = 0; $index -lt $Arguments.Length; $index++) {
        $quoted[$index] = Quote-Argument $Arguments[$index]
    }
    [string]$style = 'Hidden'
    if ($Visible) { $style = 'Normal' }
    [string]$stdout = Join-Path $Context.RunPath "$Name.stdout.log"
    [string]$stderr = Join-Path $Context.RunPath "$Name.stderr.log"
    [Diagnostics.Process]$owned = Start-Process -FilePath $Program -ArgumentList $quoted -WorkingDirectory $Context.PackagePath `
        -WindowStyle $style -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    return $owned
}
function Wait-Discovery([LaunchContext]$Context, [Diagnostics.Process]$Process, [string]$Directory) {
    [DateTime]$deadline = [DateTime]::UtcNow.AddSeconds(310)
    [string]$discoveryPath = Join-Path $Directory 'discovery.json'
    while (-not (Test-Path -LiteralPath $discoveryPath)) {
        if ($Process.HasExited) { throw "Service exited before discovery; inspect private logs in $($Context.RunPath)" }
        if ([DateTime]::UtcNow -ge $deadline) { throw "Service startup timed out; inspect $($Context.RunPath)" }
        Start-Sleep -Milliseconds 200
    }
}
# The caller retains service ownership. The temporary shutdown client is always
# disposed here, including failed or timed-out shutdown requests.
function Stop-Owned([LaunchContext]$Context, [Diagnostics.Process]$Process, [string]$Program, [string[]]$Arguments, [string]$Name) {
    if ($null -eq $Process -or $Process.HasExited) { return }
    [Diagnostics.Process]$stopper = $null
    try {
        $stopper = Start-Owned $Context $Program $Arguments "$Name-shutdown"
        if (-not $stopper.WaitForExit(8000)) { $stopper.Kill(); $stopper.WaitForExit() }
        if ($Process.WaitForExit(8000)) { return }
    } catch { Write-Warning "Graceful $Name shutdown failed: $_" }
    finally { if ($null -ne $stopper) { $stopper.Dispose() } }
    Write-Warning "Stopping only this launcher's owned $Name process after its shutdown deadline."
    if (-not $Process.HasExited) { $Process.Kill(); $Process.WaitForExit() }
}

# Short-lived commands have a separate owner; failure cannot leak a child or
# its process handle into the long-lived service lifecycle.
function Invoke-OwnedCommand([LaunchContext]$Context, [string]$Program, [string[]]$Arguments, [string]$Name, [int]$TimeoutMilliseconds) {
    [Diagnostics.Process]$command = Start-Owned $Context $Program $Arguments $Name
    try {
        if (-not $command.WaitForExit($TimeoutMilliseconds)) {
            $command.Kill()
            $command.WaitForExit()
            throw "$Name timed out; inspect $($Context.RunPath)"
        }
        if ($command.ExitCode -ne 0) { throw "$Name failed; inspect $($Context.RunPath)" }
    } finally { $command.Dispose() }
}

[string]$packagePath = (Resolve-Path -LiteralPath $PackageDirectory).ProviderPath
[string]$rootPath = (Resolve-Path -LiteralPath $Root).ProviderPath
if (-not (Test-Path -LiteralPath $rootPath -PathType Container) -or $rootPath.StartsWith('\\')) {
    throw 'Root must be an explicitly chosen existing local directory.'
}
[string]$application = Join-Path $packagePath 'File Manager.exe'
[string]$engine = Join-Path $packagePath 'components\fileman-engine.exe'
[string]$orchestrator = Join-Path $packagePath 'components\orchestrator.exe'
foreach ($binary in @($application, $engine, $orchestrator)) {
    if (-not (Test-Path -LiteralPath $binary -PathType Leaf)) { throw "Missing packaged binary: $binary" }
}
if ([string]::IsNullOrEmpty($StateDirectory)) {
    $StateDirectory = Join-Path ([IO.Path]::GetTempPath()) ('fileman-search-' + [Guid]::NewGuid().ToString('N'))
}
if (-not [IO.Path]::IsPathRooted($StateDirectory)) { throw 'StateDirectory must be absolute and new.' }
[string]$runPath = [IO.Path]::GetFullPath($StateDirectory)
[string]$rootPrefix = $rootPath.TrimEnd('\') + '\'
if ($runPath.Equals($rootPath, [StringComparison]::OrdinalIgnoreCase) -or
    $runPath.StartsWith($rootPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'StateDirectory must be outside the searched root; choose another local location.'
}
if (Test-Path -LiteralPath $runPath) { throw 'StateDirectory must be new; existing state is never overwritten.' }
if (-not (Test-Path -LiteralPath (Split-Path $runPath -Parent) -PathType Container)) {
    throw 'StateDirectory parent must already exist.'
}
New-Item -ItemType Directory -Path $runPath | Out-Null
[Security.Principal.WindowsIdentity]$identity = [Security.Principal.WindowsIdentity]::GetCurrent()
[Security.Principal.SecurityIdentifier]$sid = $identity.User
$identity.Dispose()
[Security.AccessControl.DirectorySecurity]$acl = [Security.AccessControl.DirectorySecurity]::new()
$acl.SetOwner($sid)
$acl.SetAccessRuleProtection($true, $false)
[Security.AccessControl.FileSystemAccessRule]$rule = [Security.AccessControl.FileSystemAccessRule]::new($sid, 'FullControl', 'ContainerInherit,ObjectInherit', 'None', 'Allow')
$acl.AddAccessRule($rule)
Set-Acl -LiteralPath $runPath -AclObject $acl

[LaunchContext]$context = [LaunchContext]::new($packagePath, $runPath)
[string]$engineRuntime = Join-Path $runPath 'engine-runtime'
[string]$orchestratorRuntime = Join-Path $runPath 'orchestrator-runtime'
[string]$manifest = Join-Path $runPath 'policy\manifest.json'
[string]$rootId = 'chosen-root'
[Diagnostics.Process]$engineProcess = $null
[Diagnostics.Process]$orchestratorProcess = $null
[Diagnostics.Process]$applicationProcess = $null
[string]$priorRuntime = $env:FILEMAN_ORCHESTRATOR_RUNTIME_DIR
try {
    [string[]]$manifestArguments = @('create-windows-manifest', '--deployment-id', 'explicit-file-manager-launch',
        '--root-id', $rootId, '--root-path', $rootPath, '--runtime-dir', $engineRuntime, '--output', $manifest)
    if ($IndexEnabled) { $manifestArguments += @('--index-enabled', '--store-root', (Join-Path $runPath 'store')) }
    Invoke-OwnedCommand $context $engine $manifestArguments 'manifest' 30000
    Write-Host "Private run state: $runPath"
    if ($IndexEnabled) { Write-Host 'Index enabled for exact criteria; text search uses live filename/path matching.' }
    else { Write-Host 'Live name/path substring search; no persistent catalogue.' }
    $engineProcess = Start-Owned $context $engine @('serve-windows', '--manifest', $manifest) 'engine'
    Wait-Discovery $context $engineProcess $engineRuntime
    $orchestratorProcess = Start-Owned $context $orchestrator @('serve-local', '--runtime-dir', $orchestratorRuntime,
        '--engine-runtime-dir', $engineRuntime) 'orchestrator'
    Wait-Discovery $context $orchestratorProcess $orchestratorRuntime
    $env:FILEMAN_ORCHESTRATOR_RUNTIME_DIR = $orchestratorRuntime
    Invoke-OwnedCommand $context $orchestrator @('call-local', 'status', '--json') 'status' 15000
    if (-not $CheckOnly) {
        $applicationProcess = Start-Owned $context $application @('--root', $rootPath, '--engine-root-id', $rootId) 'frontend' -Visible
        $applicationProcess.WaitForExit()
        if ($applicationProcess.ExitCode -ne 0) { throw "File Manager exited with code $($applicationProcess.ExitCode); inspect $runPath" }
    }
    Write-Host 'Explicit launch completed; stopping its services.'
} finally {
    $env:FILEMAN_ORCHESTRATOR_RUNTIME_DIR = $priorRuntime
    try {
        try {
            if ($null -ne $applicationProcess -and -not $applicationProcess.HasExited) {
                [void]$applicationProcess.CloseMainWindow()
                if (-not $applicationProcess.WaitForExit(8000)) {
                    Write-Warning 'Closing the owned frontend after its close deadline.'
                    $applicationProcess.Kill()
                    $applicationProcess.WaitForExit()
                }
            }
        } finally {
            try {
                Stop-Owned $context $orchestratorProcess $orchestrator @('call-local', 'shutdown', '--runtime-dir', $orchestratorRuntime, '--json') 'orchestrator'
            } finally {
                Stop-Owned $context $engineProcess $engine @('call-local', '--runtime-dir', $engineRuntime, '--authority', 'admin',
                    '--request', '{"id":"launcher-stop","method":"engine.shutdown","params":{}}') 'engine'
            }
        }
    } finally {
        foreach ($process in @($applicationProcess, $orchestratorProcess, $engineProcess)) {
            if ($null -ne $process) { $process.Dispose() }
        }
    }
}
