param(
    [string]$MingwBin = 'C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin',
    [string]$RustBin = (Join-Path $env:USERPROFILE '.cargo\bin')
)

$ErrorActionPreference = 'Stop'
[string]$component = Split-Path $PSScriptRoot -Parent
[string]$manifest = Join-Path $component 'Cargo.toml'
[string]$build = Join-Path $component '.build\windows-cpp'
[string]$originalPath = $env:PATH
function Invoke-Checked {
    param([string]$Program, [string[]]$Arguments)
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Program failed with exit code $LASTEXITCODE"
    }
}

[string]$priorJobs = $env:CARGO_BUILD_JOBS
[string]$clientSource = Join-Path $component 'conformance\clients\cpp'
try {
    $env:PATH = "$RustBin;$MingwBin;$originalPath"
    $env:CARGO_BUILD_JOBS = '2'
    Invoke-Checked 'rustc' @('--version', '--verbose')
    Invoke-Checked 'cargo' @('fmt', '--manifest-path', $manifest, '--', '--check')
    Invoke-Checked 'cargo' @('test', '--manifest-path', $manifest, '--locked')
    Invoke-Checked 'cargo' @('clippy', '--manifest-path', $manifest, '--all-targets', '--all-features', '--locked', '--', '-D', 'warnings')
    Invoke-Checked 'cargo' @('run', '--manifest-path', $manifest, '--locked', '--bin', 'orchestrator-fixtures', '--', '--check')
    Invoke-Checked 'cargo' @('build', '--manifest-path', $manifest, '--locked', '--release', '--bins')
    Invoke-Checked 'cmake' @('-S', $clientSource, '-B', $build, '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Release', '-DFILEMAN_ORCHESTRATOR_BUILD_PLATFORM_CHECKS=ON')
    Invoke-Checked 'cmake' @('--build', $build, '--parallel', '2')
    Invoke-Checked 'ctest' @('--test-dir', $build, '--output-on-failure')
} finally {
    $env:PATH = $originalPath
    $env:CARGO_BUILD_JOBS = $priorJobs
}
