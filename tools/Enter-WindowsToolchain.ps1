[CmdletBinding()]
param(
    [string]$MingwBin = $env:FILE_MANAGER_MINGW_BIN,
    [string]$GoBin = $env:FILE_MANAGER_GO_BIN,
    [string]$RustBin = (Join-Path $env:USERPROFILE '.cargo\bin')
)

# Dot-source to prepare only this session. The sibling toolchain stays read-only.
$ErrorActionPreference = 'Stop'
[string]$fileManagerRoot = Split-Path $PSScriptRoot -Parent
[string]$fileManagerParent = Split-Path $fileManagerRoot -Parent
if ([string]::IsNullOrWhiteSpace($MingwBin)) {
    $MingwBin = Join-Path $fileManagerParent 'plan-paint\build-deps\msys64\mingw64\bin'
}
if ([string]::IsNullOrWhiteSpace($GoBin)) {
    $GoBin = [Environment]::GetEnvironmentVariable('FILE_MANAGER_GO_BIN', 'User')
}

[System.Collections.Generic.List[string]]$fileManagerToolBins = [System.Collections.Generic.List[string]]::new()
[string]$fileManagerToolBin = ''
foreach ($fileManagerToolBin in @($MingwBin, $GoBin, $RustBin)) {
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
$env:FILE_MANAGER_MINGW_BIN = $MingwBin
if (-not [string]::IsNullOrWhiteSpace($GoBin)) {
    $env:FILE_MANAGER_GO_BIN = $GoBin
}

[string]$fileManagerToolName = ''
foreach ($fileManagerToolName in @('cmake', 'ninja', 'g++', 'python', 'go', 'cargo', 'rustc')) {
    [System.Management.Automation.ApplicationInfo[]]$fileManagerTools = @(Get-Command $fileManagerToolName -CommandType Application -ErrorAction SilentlyContinue)
    if ($fileManagerTools.Count -eq 0) {
        Write-Warning "$fileManagerToolName is not available in this session."
    } else {
        [System.Management.Automation.ApplicationInfo]$fileManagerTool = $fileManagerTools[0]
        Write-Host ('{0}: {1}' -f $fileManagerToolName, $fileManagerTool.Source)
    }
}
