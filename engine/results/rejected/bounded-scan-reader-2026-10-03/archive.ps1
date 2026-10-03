param([string]$Root = $PSScriptRoot)
$ErrorActionPreference = 'Stop'
[string]$engineRoot = [IO.Path]::GetFullPath((Join-Path $Root '../..'))
[string]$archiveRoot = Join-Path $engineRoot 'results/rejected/bounded-scan-reader-2026-10-03'
if (Test-Path -LiteralPath $archiveRoot) { throw 'Archive already exists; do not overwrite evidence' }
[string]$finalRoot = Join-Path $Root 'final-source/engine'
New-Item -ItemType Directory -Path $finalRoot -Force | Out-Null
[string[]]$sources = @('internal/generation/segment.go', 'internal/generation/bounded_scan_candidate.go', 'internal/generation/bounded_scan_candidate_test.go', 'benchmarks/bounded_scan_candidate_test.go')
[string]$relative = ''
foreach ($relative in $sources) {
    [string]$destination = Join-Path $finalRoot $relative
    New-Item -ItemType Directory -Path (Split-Path $destination -Parent) -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $engineRoot $relative) -Destination $destination
}
[string]$diffTool = 'C:/Program Files/Git/usr/bin/diff.exe'
[string]$patchTool = 'C:/Program Files/Git/usr/bin/patch.exe'
[Text.StringBuilder]$patchText = [Text.StringBuilder]::new()
foreach ($relative in $sources) {
    [string]$old = Join-Path $Root "baseline/engine/$relative"
    [string]$label = "a/engine/$relative"
    if (-not [IO.File]::Exists($old)) { $old = Join-Path $Root 'empty.txt'; $label = '/dev/null' }
    [string[]]$lines = & $diffTool --strip-trailing-cr -u --label $label --label "b/engine/$relative" $old (Join-Path $finalRoot $relative)
    if ($LASTEXITCODE -gt 1) { throw "Final diff failed: $relative" }
    [string]$line = ''
    foreach ($line in $lines) { [void]$patchText.AppendLine($line) }
}
[string]$finalPatch = Join-Path $Root 'candidate-final-style.patch'
[IO.File]::WriteAllText($finalPatch, $patchText.ToString().Replace("`r`n", "`n"))
[string]$verification = Join-Path $Root 'final-verification'
New-Item -ItemType Directory -Path $verification -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $Root 'baseline/engine') -Destination (Join-Path $verification 'engine') -Recurse
& $patchTool --binary --fuzz=0 -p1 -d $verification -i $finalPatch
if ($LASTEXITCODE -ne 0) { throw 'Final patch reconstruction failed' }
[System.Collections.Generic.Dictionary[string,string]]$sourceHashes = [System.Collections.Generic.Dictionary[string,string]]::new()
foreach ($relative in $sources) {
    [string]$expectedPath = Join-Path $finalRoot $relative
    [string]$actualPath = Join-Path $verification "engine/$relative"
    [string]$expectedText = [IO.File]::ReadAllText($expectedPath).Replace("`r`n", "`n")
    [string]$actualText = [IO.File]::ReadAllText($actualPath).Replace("`r`n", "`n")
    if ($expectedText -cne $actualText) { throw "Final reconstruction mismatch: $relative" }
    $sourceHashes.Add("final-source/engine/$relative", (Get-FileHash -LiteralPath $expectedPath -Algorithm SHA256).Hash.ToLowerInvariant())
}
# Restore from a preserved raw CRLF source only after exact normalized equality
# with this assignment's pre-edit baseline; no older semantic changes may enter.
[string]$rawBaseline = Join-Path $engineRoot '.build/bounded-reader-followup/baseline/engine/internal/generation/segment.go'
[string]$rawText = [IO.File]::ReadAllText($rawBaseline).Replace("`r`n", "`n")
[string]$baselineText = [IO.File]::ReadAllText((Join-Path $Root 'baseline/engine/internal/generation/segment.go')).Replace("`r`n", "`n")
if ($rawText -cne $baselineText) { throw 'Raw baseline differs from current pre-edit snapshot' }
New-Item -ItemType Directory -Path $archiveRoot | Out-Null
Copy-Item -LiteralPath $rawBaseline -Destination (Join-Path $archiveRoot 'baseline-segment.go.txt')
Copy-Item -LiteralPath (Join-Path $Root 'candidate.patch') -Destination (Join-Path $archiveRoot 'measured.patch')
Copy-Item -LiteralPath $finalPatch -Destination (Join-Path $archiveRoot 'candidate.patch')
Copy-Item -LiteralPath (Join-Path $Root 'manifest.json') -Destination (Join-Path $archiveRoot 'measured-manifest.json')
Copy-Item -LiteralPath (Join-Path $Root 'prepare.ps1'),(Join-Path $Root 'audit.py'),(Join-Path $Root 'archive.ps1') -Destination $archiveRoot
[string[]]$sourceTrees = @((Join-Path $Root 'baseline'), (Join-Path $Root 'candidate'), (Join-Path $Root 'final-source'))
Compress-Archive -LiteralPath $sourceTrees -DestinationPath (Join-Path $archiveRoot 'sources.zip')
[System.Collections.Generic.List[string]]$logPaths = [System.Collections.Generic.List[string]]::new()
[IO.FileInfo]$item = $null
foreach ($item in (Get-ChildItem -LiteralPath $Root -File)) {
    if ($item.Extension -eq '.txt' -and $item.Name -ne 'archive-run.txt') { $logPaths.Add($item.FullName) }
}
Compress-Archive -LiteralPath $logPaths.ToArray() -DestinationPath (Join-Path $archiveRoot 'logs.zip')
[System.Collections.Generic.Dictionary[string,string]]$artifactHashes = [System.Collections.Generic.Dictionary[string,string]]::new()
foreach ($item in (Get-ChildItem -LiteralPath $archiveRoot -File)) {
    $artifactHashes.Add($item.Name, (Get-FileHash -LiteralPath $item.FullName -Algorithm SHA256).Hash.ToLowerInvariant())
}
[System.Collections.Specialized.OrderedDictionary]$manifest = [ordered]@{
    status = 'REJECTED for admission: no-regression gate not established; not a statistical slowdown proof'
    baseline_parent_reported = 'd4dcb245'
    measured_patch = 'measured.patch'
    final_style_patch = 'candidate.patch'
    final_style_validation = 'Reconstruction/source review only. No compiler after release; last tests and timing precede seven named-return style corrections.'
    artifacts = $artifactHashes
    final_sources = $sourceHashes
}
[string]$manifestJson = ConvertTo-Json -InputObject $manifest -Depth 8
[IO.File]::WriteAllText((Join-Path $archiveRoot 'manifest.json'), $manifestJson + "`n")
# Recheck ownership hashes immediately before restoring/removing only our files.
foreach ($relative in $sources) {
    [string]$activeHash = (Get-FileHash -LiteralPath (Join-Path $engineRoot $relative) -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($activeHash -cne $sourceHashes["final-source/engine/$relative"]) { throw "Concurrent edit detected: $relative" }
}
Copy-Item -LiteralPath (Join-Path $archiveRoot 'baseline-segment.go.txt') -Destination (Join-Path $engineRoot 'internal/generation/segment.go') -Force
[string[]]$newFiles = @('internal/generation/bounded_scan_candidate.go', 'internal/generation/bounded_scan_candidate_test.go', 'benchmarks/bounded_scan_candidate_test.go')
foreach ($relative in $newFiles) {
    [string]$target = [IO.Path]::GetFullPath((Join-Path $engineRoot $relative))
    if (-not $target.StartsWith($engineRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw "Out-of-scope target: $target" }
    Remove-Item -LiteralPath $target
}
[string]$restored = [IO.File]::ReadAllText((Join-Path $engineRoot 'internal/generation/segment.go')).Replace("`r`n", "`n")
if ($restored -cne $baselineText) { throw 'Restored source differs from pre-edit baseline' }
Write-Output 'Both candidate patches reconstruct. Archive hashes are recorded. Restored segment.go and removed only the three candidate Go files.'
Write-Output $manifestJson
