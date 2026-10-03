param([string]$Root = $PSScriptRoot)
$ErrorActionPreference = 'Stop'
[string]$engineRoot = [IO.Path]::GetFullPath((Join-Path $Root '../..'))
[string]$candidateRoot = Join-Path $Root 'candidate'
[string]$candidateEngine = Join-Path $candidateRoot 'engine'
[string]$baselineRoot = Join-Path $Root 'baseline'
[string]$baselineEngine = Join-Path $baselineRoot 'engine'
[string]$patchTool = 'C:/Program Files/Git/usr/bin/patch.exe'
[string]$diffTool = 'C:/Program Files/Git/usr/bin/diff.exe'
[string]$harness = Join-Path $engineRoot 'results/rejected/bounded-reader-2026-10-02/aggregate-harness.patch'
[string[]]$sources = @('internal/generation/segment.go', 'internal/generation/bounded_scan_candidate.go', 'internal/generation/bounded_scan_candidate_test.go', 'benchmarks/bounded_scan_candidate_test.go')
if (Test-Path -LiteralPath $candidateEngine) { throw 'Candidate snapshot already exists' }
New-Item -ItemType Directory -Path $candidateRoot -Force | Out-Null
Copy-Item -LiteralPath $baselineEngine -Destination $candidateEngine -Recurse
[string]$relative = ''
foreach ($relative in $sources) {
    Copy-Item -LiteralPath (Join-Path $engineRoot $relative) -Destination (Join-Path $candidateEngine $relative)
}
[string]$sourceTree = ''
foreach ($sourceTree in @($baselineEngine, $candidateEngine)) {
    foreach ($relative in @('internal/generation/segment.go', 'benchmarks/m2_generation_test.go')) {
        [string]$sourcePath = Join-Path $sourceTree $relative
        [string]$sourceText = [IO.File]::ReadAllText($sourcePath).Replace("`r`n", "`n")
        [IO.File]::WriteAllText($sourcePath, $sourceText)
    }
}
[string]$empty = Join-Path $Root 'empty.txt'
[IO.File]::WriteAllText($empty, '')
[Text.StringBuilder]$patchText = [Text.StringBuilder]::new()
foreach ($relative in $sources) {
    [string]$old = Join-Path $baselineEngine $relative
    [string]$label = "a/engine/$relative"
    if (-not [IO.File]::Exists($old)) { $old = $empty; $label = '/dev/null' }
    [string[]]$lines = & $diffTool --strip-trailing-cr -u --label $label --label "b/engine/$relative" $old (Join-Path $candidateEngine $relative)
    if ($LASTEXITCODE -gt 1) { throw "Diff failed: $relative" }
    [string]$line = ''
    foreach ($line in $lines) { [void]$patchText.AppendLine($line) }
}
[string]$patch = Join-Path $Root 'candidate.patch'
[IO.File]::WriteAllText($patch, $patchText.ToString().Replace("`r`n", "`n"))
# Reconstruct independently before the timing-only harness changes.
[string]$verificationRoot = Join-Path $Root 'verification'
New-Item -ItemType Directory -Path $verificationRoot -Force | Out-Null
Copy-Item -LiteralPath $baselineEngine -Destination (Join-Path $verificationRoot 'engine') -Recurse
& $patchTool --binary --fuzz=0 -p1 -d $verificationRoot -i $patch
if ($LASTEXITCODE -ne 0) { throw 'Candidate patch reconstruction failed' }
foreach ($relative in $sources) {
    [string]$expected = (Get-FileHash -LiteralPath (Join-Path $candidateEngine $relative) -Algorithm SHA256).Hash
    [string]$actual = (Get-FileHash -LiteralPath (Join-Path $verificationRoot "engine/$relative") -Algorithm SHA256).Hash
    # Existing segment.go may have CRLF while unified diff emits LF. Compare
    # normalized source bytes explicitly, recording raw hashes separately.
    [string]$expectedText = [IO.File]::ReadAllText((Join-Path $candidateEngine $relative)).Replace("`r`n", "`n")
    [string]$actualText = [IO.File]::ReadAllText((Join-Path $verificationRoot "engine/$relative")).Replace("`r`n", "`n")
    if ($expectedText -cne $actualText) { throw "Reconstruction mismatch: $relative $expected $actual" }
}
[string]$variantRoot = ''
foreach ($variantRoot in @($baselineRoot, $candidateRoot)) {
    & $patchTool --binary --fuzz=0 -p1 -d $variantRoot -i $harness
    if ($LASTEXITCODE -ne 0) { throw "Harness failed: $variantRoot" }
}
[string]$baselineHarness = [IO.File]::ReadAllText((Join-Path $baselineEngine 'benchmarks/m2_generation_test.go')).Replace("`r`n", "`n")
[string]$candidateHarness = [IO.File]::ReadAllText((Join-Path $candidateEngine 'benchmarks/m2_generation_test.go')).Replace("`r`n", "`n")
if ($baselineHarness -cne $candidateHarness) { throw 'Timing harness differs' }
[byte[]]$harnessBytes = [Text.Encoding]::UTF8.GetBytes($baselineHarness)
[Security.Cryptography.SHA256]$hasher = [Security.Cryptography.SHA256]::Create()
[byte[]]$digest = $hasher.ComputeHash($harnessBytes)
$hasher.Dispose()
[string]$harnessHash = [BitConverter]::ToString($digest).Replace('-', '').ToLowerInvariant()
if ($harnessHash -cne 'e15ee1d716ffbc3b09bd4a9ac7afb5ad0d2b443ea713a68cd986a342655f2c52') { throw "Harness hash changed: $harnessHash" }
Write-Output "Candidate reconstruction and archived harness verified: $harnessHash"
