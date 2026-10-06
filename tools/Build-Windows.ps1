[CmdletBinding()]
param(
    [ValidateSet('Toolkit', 'Frontend')]
    [string]$Component = 'Toolkit',
    [ValidateRange(1, 64)]
    [int]$Jobs = 3,
    [switch]$Test
)

$ErrorActionPreference = 'Stop'
[string]$fileManagerToolchainScript = Join-Path $PSScriptRoot 'Enter-WindowsToolchain.ps1'
. $fileManagerToolchainScript
[string]$fileManagerRoot = Split-Path $PSScriptRoot -Parent
[string]$fileManagerSdk = Join-Path $fileManagerRoot 'gui_forms\.build\shadow-clang-sdk'

function Invoke-FileManagerBuildCommand {
    param([string]$Program, [string[]]$Arguments)
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Program failed with exit code $LASTEXITCODE"
    }
}

Push-Location $fileManagerRoot
try {
    if ($Component -eq 'Toolkit') {
        [string]$fileManagerBuild = Join-Path $fileManagerRoot 'gui_forms\.build\shadow-windows-clang'
        Invoke-FileManagerBuildCommand 'cmake' @(
            '-S', 'gui_forms', '-B', $fileManagerBuild, '-G', 'Ninja',
            '-DCMAKE_BUILD_TYPE=Release', "-DCMAKE_C_COMPILER=$env:CC", "-DCMAKE_CXX_COMPILER=$env:CXX",
            '-DGUI_FORMS_ENABLE_WINDOWS_HOST=ON',
            '-DGUI_FORMS_ENABLE_MACOS_HOST=OFF', '-DGUI_FORMS_ENABLE_SKIA=OFF',
            '-DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=OFF', '-DGUI_FORMS_BUILD_GALLERY=OFF',
            '-DGUI_FORMS_BUILD_TESTS=ON', "-DCMAKE_INSTALL_PREFIX=$fileManagerSdk"
        )
    } else {
        [string]$fileManagerBuild = Join-Path $fileManagerRoot 'frontend\.build\shadow-windows-clang'
        [string]$fileManagerConfig = Join-Path $fileManagerSdk 'lib\cmake\GUIForms\GUIFormsConfig.cmake'
        if (-not (Test-Path -LiteralPath $fileManagerConfig)) {
            throw 'Build and install the Toolkit component first.'
        }
        Invoke-FileManagerBuildCommand 'cmake' @(
            '-S', 'frontend', '-B', $fileManagerBuild, '-G', 'Ninja',
            '-DCMAKE_BUILD_TYPE=Release', "-DCMAKE_C_COMPILER=$env:CC", "-DCMAKE_CXX_COMPILER=$env:CXX",
            "-DGUIForms_DIR=$fileManagerSdk/lib/cmake/GUIForms",
            "-DFILE_MANAGER_GUI_FORMS_MANIFEST=$fileManagerRoot/gui_forms/manifests/gui-forms-shadow-windows-x64-2026-09-29.json"
        )
    }
    Invoke-FileManagerBuildCommand 'cmake' @('--build', $fileManagerBuild, '--parallel', "$Jobs")
    if ($Test) {
        Invoke-FileManagerBuildCommand 'ctest' @('--test-dir', $fileManagerBuild, '--output-on-failure', '--timeout', '90')
    }
    if ($Component -eq 'Toolkit') {
        Invoke-FileManagerBuildCommand 'cmake' @('--install', $fileManagerBuild)
    }
} finally {
    Pop-Location
}
