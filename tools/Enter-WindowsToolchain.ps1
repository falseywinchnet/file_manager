[CmdletBinding()]
param(
    [string]$ClangBin = $env:FILE_MANAGER_CLANG_BIN,
    [string]$GoBin = $env:FILE_MANAGER_GO_BIN,
    [string]$RustBin = (Join-Path $env:USERPROFILE '.cargo\bin')
)

# Dot-source to prepare only this session. The sibling toolchain stays read-only.
$ErrorActionPreference = 'Stop'
[string]$fileManagerRoot = Split-Path $PSScriptRoot -Parent
[string]$fileManagerParent = Split-Path $fileManagerRoot -Parent
if ([string]::IsNullOrWhiteSpace($ClangBin)) {
    $ClangBin = Join-Path $fileManagerParent 'plan-paint\build-deps\msys64\clang64\bin'
}
[string]$fileManagerRequiredTool = ''
foreach ($fileManagerRequiredTool in @('clang.exe', 'clang++.exe', 'cmake.exe', 'ninja.exe', 'python.exe')) {
    if (-not (Test-Path -LiteralPath (Join-Path $ClangBin $fileManagerRequiredTool) -PathType Leaf)) {
        throw "Missing CLANG64 tool $fileManagerRequiredTool in $ClangBin. Set FILE_MANAGER_CLANG_BIN to the MSYS2 clang64 bin directory."
    }
}
if ([string]::IsNullOrWhiteSpace($GoBin)) {
    $GoBin = [Environment]::GetEnvironmentVariable('FILE_MANAGER_GO_BIN', 'User')
}

[System.Collections.Generic.List[string]]$fileManagerToolBins = [System.Collections.Generic.List[string]]::new()
[string]$fileManagerToolBin = ''
foreach ($fileManagerToolBin in @($ClangBin, $GoBin, $RustBin)) {
    if (-not [string]::IsNullOrWhiteSpace($fileManagerToolBin) -and
        (Test-Path -LiteralPath $fileManagerToolBin -PathType Container)) {
        [System.Management.Automation.PathInfo]$fileManagerResolved = Resolve-Path -LiteralPath $fileManagerToolBin
        $fileManagerToolBins.Add($fileManagerResolved.Path)
    }
}
[string[]]$fileManagerOriginalBins = $env:PATH -split ';'
[string]$fileManagerOriginalBin = ''
foreach ($fileManagerOriginalBin in $fileManagerOriginalBins) {
    if (-not [string]::IsNullOrEmpty($fileManagerOriginalBin)) {
        $fileManagerToolBins.Add($fileManagerOriginalBin)
    }
}
# Keep the first occurrence, so explicitly selected tools precede inherited ones.
[System.Collections.Generic.HashSet[string]]$fileManagerSeenBins = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
[System.Collections.Generic.List[string]]$fileManagerUniqueBins = [System.Collections.Generic.List[string]]::new()
foreach ($fileManagerToolBin in $fileManagerToolBins) {
    if ($fileManagerSeenBins.Add($fileManagerToolBin)) {
        $fileManagerUniqueBins.Add($fileManagerToolBin)
    }
}
$env:PATH = [string]::Join(';', $fileManagerUniqueBins)
$env:FILE_MANAGER_CLANG_BIN = $ClangBin
$env:CC = Join-Path $ClangBin 'clang.exe'
$env:CXX = Join-Path $ClangBin 'clang++.exe'
if (-not [string]::IsNullOrWhiteSpace($GoBin)) {
    $env:FILE_MANAGER_GO_BIN = $GoBin
}

[string]$fileManagerToolName = ''
foreach ($fileManagerToolName in @('cmake', 'ninja', 'clang', 'clang++', 'python', 'go', 'cargo', 'rustc')) {
    [System.Management.Automation.ApplicationInfo[]]$fileManagerTools = @(Get-Command $fileManagerToolName -CommandType Application -ErrorAction SilentlyContinue)
    if ($fileManagerTools.Count -eq 0) {
        Write-Warning "$fileManagerToolName is not available in this session."
    } else {
        [System.Management.Automation.ApplicationInfo]$fileManagerTool = $fileManagerTools[0]
        Write-Host ('{0}: {1}' -f $fileManagerToolName, $fileManagerTool.Source)
    }
}
