[CmdletBinding()]
param(
    [string]$MingwBin = $env:FILE_MANAGER_MINGW_BIN,
    [string]$GoBin = $env:FILE_MANAGER_GO_BIN,
    [string]$RustBin = (Join-Path $env:USERPROFILE '.cargo\bin')
)

# Dot-source this file to prepare this PowerShell session. No machine/user PATH
# is rewritten, and the sibling application's compiler installation is read only.
$ErrorActionPreference = 'Stop'
$fileManagerRoot = Split-Path $PSScriptRoot -Parent
if ([string]::IsNullOrWhiteSpace($MingwBin)) {
    $MingwBin = Join-Path (Split-Path $fileManagerRoot -Parent) 'plan-paint\build-deps\msys64\mingw64\bin'
}
if ([string]::IsNullOrWhiteSpace($GoBin)) {
    $GoBin = [Environment]::GetEnvironmentVariable('FILE_MANAGER_GO_BIN', 'User')
}

$fileManagerToolBins = @()
foreach ($fileManagerToolBin in @($MingwBin, $GoBin, $RustBin)) {
    if (-not [string]::IsNullOrWhiteSpace($fileManagerToolBin) -and
        (Test-Path -LiteralPath $fileManagerToolBin -PathType Container)) {
        $fileManagerToolBins += (Resolve-Path -LiteralPath $fileManagerToolBin).Path
    }
}
$fileManagerOriginalBins = @($env:PATH -split ';' | Where-Object { $_ })
$env:PATH = (($fileManagerToolBins + $fileManagerOriginalBins) | Select-Object -Unique) -join ';'
$env:FILE_MANAGER_MINGW_BIN = $MingwBin
if (-not [string]::IsNullOrWhiteSpace($GoBin)) { $env:FILE_MANAGER_GO_BIN = $GoBin }

foreach ($fileManagerToolName in @('cmake', 'ninja', 'g++', 'python', 'go', 'cargo', 'rustc')) {
    $fileManagerTool = Get-Command $fileManagerToolName -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($null -eq $fileManagerTool) {
        Write-Warning "$fileManagerToolName is not available in this session."
    } else {
        Write-Host ('{0}: {1}' -f $fileManagerToolName, $fileManagerTool.Source)
    }
}
