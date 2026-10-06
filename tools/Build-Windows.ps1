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
[string]$fileManagerManifest = Join-Path $fileManagerSdk 'gui-forms-consumption.json'

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
        if (-not (Test-Path -LiteralPath $fileManagerConfig) -or
            -not (Test-Path -LiteralPath $fileManagerManifest)) {
            throw 'Build and install the Toolkit component first.'
        }
        Invoke-FileManagerBuildCommand 'cmake' @(
            '-S', 'frontend', '-B', $fileManagerBuild, '-G', 'Ninja',
            '-DCMAKE_BUILD_TYPE=Release', "-DCMAKE_C_COMPILER=$env:CC", "-DCMAKE_CXX_COMPILER=$env:CXX",
            "-DGUIForms_DIR=$fileManagerSdk/lib/cmake/GUIForms",
            "-DFILE_MANAGER_GUI_FORMS_MANIFEST=$fileManagerManifest"
        )
    }
    Invoke-FileManagerBuildCommand 'cmake' @('--build', $fileManagerBuild, '--parallel', "$Jobs")
    if ($Test) {
        Invoke-FileManagerBuildCommand 'ctest' @('--test-dir', $fileManagerBuild, '--output-on-failure', '--timeout', '90')
    }
    if ($Component -eq 'Toolkit') {
        Invoke-FileManagerBuildCommand 'cmake' @('--install', $fileManagerBuild)
        [string]$fileManagerRevision = & git rev-parse HEAD
        if ($LASTEXITCODE -ne 0) {
            throw 'Cannot record the source revision for the installed Clang SDK.'
        }
        [string]$fileManagerTestState = 'not run'
        if ($Test) {
            $fileManagerTestState = 'passed'
        }
        [hashtable]$fileManagerConsumption = @{
            identity = @{
                id = "gui-forms-windows-clang-development-$fileManagerRevision"
                state = 'development'
                source_revision = $fileManagerRevision
            }
            validation = @{ gui_forms_ctest = $fileManagerTestState }
            limits = @('Local development build; revision may include uncommitted edits. No release or historical GCC validation claim.')
        }
        [string]$fileManagerManifestJson = $fileManagerConsumption | ConvertTo-Json -Depth 4
        [System.Text.UTF8Encoding]$fileManagerEncoding = [System.Text.UTF8Encoding]::new($false)
        [System.IO.File]::WriteAllText($fileManagerManifest, $fileManagerManifestJson, $fileManagerEncoding)
    }
} finally {
    Pop-Location
}
