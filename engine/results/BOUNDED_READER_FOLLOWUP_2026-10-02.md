# Bounded-reader follow-up: profile-directed decoder experiment — 2026-10-02

Status: **REJECTED for admission on the legacy exact-search regression gate.**
One evidence-led candidate was evaluated. This is research evidence and a
reconstructible patch, not an active implementation or architecture decision.

Root archival checkpoint: candidate.patch and manifest.json are preserved under
`rejected/bounded-reader-followup-2026-10-02/`; logs.zip and profiles.zip retain
the exact log/profile files named below. The patch checksum matches the receipt
and forward application against the active, unchanged Engine source passes
`git apply --check`. No candidate implementation was copied into active source.

## Authorized boundary and preserved inputs

**GIVEN:** work was confined to `engine/.build/bounded-reader-followup/` and this
new receipt. No active implementation, prior receipt, rejected archive, public
API, capability, format, Git state or other chat was changed.

Inputs were read from the complete house style, the prior bounded-reader
receipt, and `results/rejected/bounded-reader-2026-10-02/manifest.json`.
Preserved baseline and corrected temporary source copies were copied into this
follow-up directory. Their corrected source hashes and aggregate harness hash
were checked against the archive. Baseline commit is
`0873f9cb02dbfaf811b1a31c90be3af5db1b100a`; no Git command was used here.

Archived patch hashes remained unchanged at completion:

| Preserved artifact | SHA-256 |
|---|---|
| initial.patch | `f5a4168ee223d96ed4918a9c870b8504b2d58217d835ced738d78092f203592e` |
| corrected.patch | `ad88ecc3720900a3eb1403ae6f80816e045a05f31c95bf9e8ab3865f923bed22` |
| aggregate-harness.patch | `5eb462b2a53dd4b6d1dd737131d43bee0db8a59484a1ef62a7d9715202962636` |

The unchanged instrumented exact harness in all temporary variants has SHA-256
`e15ee1d716ffbc3b09bd4a9ac7afb5ad0d2b443ea713a68cd986a342655f2c52`.
Active cache.go, segment.go and substring experiment source were compared
against the preserved baseline after newline normalization and matched.
New candidate-only files were written only below .build.

## What was profiled

**MEASURED:** baseline and archived corrected candidate each ran the existing
`TestImmutableGenerationDistribution` 30 times under Go CPU and allocation
profiling. Each run uses ScaleV1(10000,10,10), 20,000 exact-path queries,
2,000 name-first-100 queries, and 2,000 name-pipeline-first-100 queries, with
existing reference checks. These profiles also include fixture construction,
publication, recovery and update work; pprof query reports filter stacks by
`sampleGeneration`. Allocation profiles were retained, but attribution below
uses compiler escape reports and measured per-query allocation counters, not
a claimed allocation-profile differential.

All three variants were compiled with
`-gcflags=filemanager/engine/internal/generation=-m=2` so compiler decisions
could be inspected without changing optimization flags. Compiler reports are
`logs/{baseline,corrected,candidate}-compiler.txt`. CPU/alloc profiles are
`profiles/{baseline,corrected,candidate}.{cpu,alloc}.pprof`. Filtered top,
cumulative and baseline/corrected decoder source-line reports are retained
alongside them. Profile-run logs include all repeated correctness results.

Environment remains Windows 11 Home 10.0.22621, amd64, C: NTFS, reported
AMD EPYC 9354 32-Core Processor, 16,757,176 KiB visible RAM, Go 1.27.1,
GOMAXPROCS=2. Native build jobs used -p 2. Borrowed GCC was read-only.
No root C++ build overlapped a profile or paired timing sequence. The first
profiles finished before yielding CPU to the root consumer benchmark; candidate
build/timing followed its CPU-clear message. Hardware/storage/power state and
external host load were not controlled.

## Attribution: observed compiler decisions and sampled work

**OBSERVED compiler decisions:** the baseline readString helper inlines at
cost 78 against the compiler's budget of 80. The corrected candidate introduces
additional forwarding frames that do not inline:

| Corrected function | Compiler cost / decision |
|---|---|
| Reader.recordAt | 84, cannot inline |
| Reader.bindingAt | 84, cannot inline |
| Reader.pathAt | 91, cannot inline |
| Reader.readString | 86, cannot inline |
| readerAccess.readString | 85, cannot inline |
| readerAccess.readAt | 219, cannot inline |

This establishes actual retained call boundaries, not just source-level
wrapper counting. Escape reports do not report an access wrapper moved to
heap, and fixed query decoder buffers are not among moved-to-heap buffers.
The string byte buffer is reported nonescaping; the returned string escapes.
Error construction and writer/open buffers retain their pre-existing escape
behavior. Aggregate allocation counts/bytes are effectively unchanged between
legacy controls, so a new wrapper heap allocation is not supported as the
regression explanation.

**MEASURED sampled evidence:** in the corrected profile, the forwarding
Reader.recordAt frame has 0.23 s flat samples, separately from 0.25 s in
readerAccess.recordAt. readerAccess.readString adds 0.16 s flat samples.
The baseline has a single Reader.recordAt decoder (0.25 s flat) and an inlined
readString helper. These new frames therefore consume sampled CPU. The common
cache remains visible (baseline 0.34 s flat / 0.80 s cumulative; corrected
0.36 / 0.96 s), but it is not the sole cost center.

Baseline profile duration/CPU samples were 11.41/12.38 s; corrected
13.85/15.12 s. Cumulative values overlap and must not be summed. Profiles were
sequential, not a randomized causal experiment, and sampling is coarse.
Path normalization, allocations and cache locks dominate much of both
profiles. The evidence attributes **some** added legacy work to non-inlined
forwarding layers; it does not establish the fraction of the overall slowdown
caused by each call or explain every remaining nanosecond.

## One candidate and its boundaries

**HYPOTHESIS tested:** remove the observed forwarding layers while retaining
one decoder and the budget's owner/lifetime rather than specializing or copying
the storage format decoder.

The candidate keeps the decoder on Reader. Private row/record/binding/object/
path/string methods take an explicit optional ReadBudget pointer; legacy
callers pass nil, bounded callers pass their query-owned budget. Reader stores
no mutable query state. readerAccess and the forwarding wrappers are removed.
Fixed records dispatch directly to the legacy cache when the budget is nil;
bounded calls enter the reservation helper. The redundant readString forwarding
helper is removed; callers use the common stored-string decoder.

The archived corrected cache kernels and integrity fixes are unchanged.
Bounded callers still validate their budget, charge logical requests including
cache hits, reserve string work before allocation, retain terminal failure,
publish no partial decoded result, and check context around individual read
boundaries. Query buffers/strings are not a copied whole catalogue. Public
entry-point signatures and the stored format are unchanged.

**OBSERVED final compiler/profile:** there is only one Reader.recordAt and one
Reader.pathAt decoder; the readerAccess frames and readString forwarding frame
are absent. The bounded-only readAtBounded helper does not inline (cost 141)
and is outside legacy fixed-read execution. The common decoders remain
non-inlined, as in the baseline. Candidate profile duration/CPU samples:
10.85/11.71 s. Reader.recordAt is 0.35 s flat / 4.93 s cumulative; cache
0.28 / 0.81 s. These sequential profile totals are not substituted for paired
timings or presented as a speed win.

## Six-pair legacy control and disposition

All variants were precompiled before measurement. Baseline and candidate ran
six pairs, baseline first on even pairs and candidate first on odd pairs.
The aggregate harness, workload and sample counts were unchanged. Aggregate
elapsed includes the existing timestamps, checks and digest bookkeeping
equally. The original per-call timer under-resolution still applies:
these are aggregate means, not reliable per-query percentiles.

| Control | Baseline median aggregate ns/query | Candidate median aggregate ns/query | Median paired slowdown | Median allocated bytes/query baseline / candidate |
|---|---:|---:|---:|---:|
| Exact path | 2,040.00 | 2,170.30 | **+6.4%** | 912.12 / 912.11 |
| Name first 100 | 46,036.90 | 46,819.70 | **+2.8%** | 48,374.54 / 48,375.31 |
| Name pipeline first 100 | 57,448.35 | 58,938.20 | **+3.5%** | 65,449.39 / 65,444.70 |

Median is the average of the middle two values for six observations. Each
paired percentage is calculated before taking its median. Raw data is below;
individual pairs vary and include both wins and losses. Reference checks
and logged query digests match: path `6c4437450b9c86ff`, name/pipeline
`a69b9113a32327fd`.

**REJECTED for admission:** the candidate does not remove the legacy regression.
Name/pipeline results are closer to baseline than the prior separate run, but
that is not a controlled estimate of the improvement caused by this remedy.
No second candidate, threshold relaxation or automatic promotion followed.
A remaining exact-path regression is not explained away by the improved
profile structure. Further attribution would require separately authorized,
controlled work; the present evidence supports neither a new architecture nor
a claim that query-owned budgets inherently require this slowdown.

## Correctness and validation

**MEASURED final candidate**, entirely in the temporary source tree:

- `go test -p 2 ./...` passed; generation 2.825 s, benchmarks 0.647 s.
- `go test -race -p 2 ./...` passed; generation 3.997 s, benchmarks 5.009 s.
- `go vet -p 2 ./...` passed without diagnostics.
- Focused budget and substring fixture checks passed before full validation.
- All baseline/corrected/candidate profiling runs and all paired runs passed.

Full checks are recorded in `logs/candidate-full-{test,race,vet}.txt`.
No test failure was observed. The final full suite follows the removal of the
last redundant string helper. It includes existing exact controls and
format/integrity tests, plus archived budget boundary, cache-hit charge,
zero-allocation string refusal, real cancellation, independent concurrent
owner, failed-cache-replacement and path-offset corruption tests. Only their
private decoder call sites were adapted.

The unchanged substring oracle still checks all ten queries and full record
digests. Corpus digest:
`83dccac06d0ee44ada61c0e97442c1485b89254727021642d4eafbb05f98afa1`;
segment 1,590,176 bytes. Logical full-traversal reads remain 2,172,200 bytes for
ancestor/014, 2,156,000 for missing, and 3,672,000 for a. Per-page limits remain
4 MiB logical reads, 64 KiB per stored string, 257 rows and 128 results.
No new bounded-versus-formula performance campaign was needed or claimed here.

## Reconstructible patch, hashes and reviewed scope

All paths below are under `engine/.build/bounded-reader-followup/`.

`candidate.patch` is a complete six-file delta against the baseline Engine,
excluding the aggregate harness. Apply it from a temporary repository-shaped
root containing engine/. Apply the unchanged archived aggregate-harness.patch
only when reproducing measurements. GNU diff/patch from the installed tools
were used directly, without Git. Patch application to `verification/engine/`
succeeded; all six reconstructed source files match candidate bytes by SHA-256.
The scripts `make_patch.ps1` and `verify_manifest.ps1` reproduce the diff/hash
checks; `manifest.json` records sources, binaries, profiles, harness and tooling.

| Candidate artifact | SHA-256 |
|---|---|
| candidate.patch | `550d9305afc3905650d0550355f2e970f5c939a848ef0f119e3853ce3e5b38ad` |
| internal/generation/cache.go | `6aef2679f23b1615ae7a3ea07c59dc10a4030f81ba194691ce9dec5fd897ea77` |
| internal/generation/segment.go | `f5f1b21ade731dafa533ba7b3431cfbc24f1317c7fca2cb9124053acd91bff3b` |
| internal/generation/read_budget.go | `7717a998cceea9b2f57625a30306ff2b260d49d2fde989f117a930eae560a6a0` |
| internal/generation/reader_access.go | `b49b325afa8d97cdf75cf547a6de9c9185efc084049a90f8f0d612335e344623` |
| internal/generation/read_budget_test.go | `7816df31847a325a7b8b5003b978dcb3845b021501fdf8e142f624941878101a` |
| benchmarks/substring_reference_experiment_test.go | `fbe33f97b0946ff83320dc775ee7d9ef823d086a2605075c9a73aa209ce6d194` |

**Source review against complete planning/PROGRAMMING_HOUSE_STYLE.md:**
all candidate reader_access.go, read_budget.go and read_budget_test.go; retained
cache read kernels; the Row and nameAt methods and changed private-call
arguments in segment.go; the retained substring experiment; the identical
aggregate harness additions; both new PowerShell scripts and authored command
snippets. Explicit types/initializers, named callbacks/state, query ownership,
reader borrows, before-allocation ordering, empty terminal results, charged
failed reads, arithmetic guards, lock exits and repeated cache work were reviewed.

No new violation was identified in authored scope. The untouched legacy
declarations in Record, Path, pathIndexRelative, Name, ID and idBoundary retain
inferred locals even where a call argument now passes nil; segment writer
closures and cache bytes() also retain legacy style. Their existing syntax is
not certified compliant or rewritten. The unchanged exact harness retains
legacy inferred locals/loops outside its reviewed additions. Decoder/string
allocation is retained and disclosed, not described as allocation-free.

## Limits

Physical I/O/prefetch bytes, output encoding size, total allocation and peak
scratch are not bounded by the logical-read counter. A synchronous native
ReadAt or lock wait cannot be interrupted until it returns. The private budget
does not own a service lease or authenticate a cursor. No native long-path/
identity, cold-I/O, million-entry, cross-platform or end-to-end GUI claim is
made. The 10k generated workload and one Windows host do not establish a
general performance law. The cache-validity and path-offset fixes remain
unadmitted salvage candidates within this rejected patch.

## Reproduction commands

From each temporary variant's engine directory, compiling before timing:

```powershell
[string]$followRoot = 'C:\Users\Shadow\file_manager\engine\.build\bounded-reader-followup'
[string]$goBin = [Environment]::GetEnvironmentVariable('FILE_MANAGER_GO_BIN', 'User')
$env:PATH = "$goBin;" + $env:PATH
$env:GOTOOLCHAIN = 'local'
$env:GOMAXPROCS = '2'
# Set variant explicitly for each source copy before compilation.
[string]$variant = 'candidate'
go test -c -p 2 '-gcflags=filemanager/engine/internal/generation=-m=2' -o "$followRoot\$variant.test.exe" ./benchmarks
$env:FILEMAN_ENGINE_M2_MEASURE = '1'
$env:FILEMAN_ENGINE_RECORDS = '10000'
& "$followRoot\$variant.test.exe" '-test.run=^TestImmutableGenerationDistribution$' '-test.v' '-test.count=1'
# Profile separately; do not treat profiled durations as paired timings.
& "$followRoot\$variant.test.exe" '-test.run=^TestImmutableGenerationDistribution$' '-test.v' '-test.count=30' "-test.cpuprofile=$followRoot\profiles\$variant.cpu.pprof" "-test.memprofile=$followRoot\profiles\$variant.alloc.pprof"
go tool pprof -top -focus=sampleGeneration "$followRoot\$variant.test.exe" "$followRoot\profiles\$variant.cpu.pprof"
```

## Raw paired observations

Logs `logs/pair-{0..5}-{baseline,candidate}.txt` preserve the full output.
The aggregate observations are reproduced below in pair order. Within-pair
execution alternates as specified above.

```text
pair-0-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=41.0656ms ns_per_query=2053.3 allocated_bytes=18242528 mallocs=500023
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=91.9784ms ns_per_query=45989.2 allocated_bytes=96749808 mallocs=1060144
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=111.6413ms ns_per_query=55820.7 allocated_bytes=130908240 mallocs=1140999
pair-0-candidate.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=43.6992ms ns_per_query=2185.0 allocated_bytes=18242064 mallocs=500021
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=90.6507ms ns_per_query=45325.3 allocated_bytes=96750912 mallocs=1060146
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=116.9143ms ns_per_query=58457.2 allocated_bytes=130899048 mallocs=1140927
pair-1-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=40.5552ms ns_per_query=2027.8 allocated_bytes=18242760 mallocs=500027
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=97.2993ms ns_per_query=48649.7 allocated_bytes=96748008 mallocs=1060108
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=119.1596ms ns_per_query=59579.8 allocated_bytes=130906104 mallocs=1140985
pair-1-candidate.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=49.0718ms ns_per_query=2453.6 allocated_bytes=18242104 mallocs=500022
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=102.1967ms ns_per_query=51098.3 allocated_bytes=96750104 mallocs=1060140
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=123.8823ms ns_per_query=61941.2 allocated_bytes=130888608 mallocs=1140846
pair-2-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=42.8544ms ns_per_query=2142.7 allocated_bytes=18242528 mallocs=500024
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=92.1691ms ns_per_query=46084.6 allocated_bytes=96748896 mallocs=1060123
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=125.4555ms ns_per_query=62727.8 allocated_bytes=130881344 mallocs=1140784
pair-2-candidate.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=40.5044ms ns_per_query=2025.2 allocated_bytes=18242232 mallocs=500020
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=92.1945ms ns_per_query=46097.2 allocated_bytes=96750328 mallocs=1060138
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=118.721ms ns_per_query=59360.5 allocated_bytes=130882696 mallocs=1140799
pair-3-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=40.128ms ns_per_query=2006.4 allocated_bytes=18242384 mallocs=500022
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=88.4783ms ns_per_query=44239.2 allocated_bytes=96749280 mallocs=1060126
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=116.4412ms ns_per_query=58220.6 allocated_bytes=130889384 mallocs=1140845
pair-3-candidate.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=45.1722ms ns_per_query=2258.6 allocated_bytes=18242120 mallocs=500020
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=95.0845ms ns_per_query=47542.2 allocated_bytes=96749640 mallocs=1060135
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=120.1895ms ns_per_query=60094.8 allocated_bytes=130890208 mallocs=1140827
pair-4-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=41.0438ms ns_per_query=2052.2 allocated_bytes=18242248 mallocs=500020
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=87.7425ms ns_per_query=43871.2 allocated_bytes=96754616 mallocs=1060131
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=113.3522ms ns_per_query=56676.1 allocated_bytes=130910896 mallocs=1141021
pair-4-candidate.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=42.6022ms ns_per_query=2130.1 allocated_bytes=18242400 mallocs=500022
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=91.3018ms ns_per_query=45650.9 allocated_bytes=96752312 mallocs=1060165
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=114.5811ms ns_per_query=57290.6 allocated_bytes=130894912 mallocs=1140893
pair-5-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=40.4948ms ns_per_query=2024.7 allocated_bytes=18242416 mallocs=500022
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=96.0786ms ns_per_query=48039.3 allocated_bytes=96748616 mallocs=1060119
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=112.8432ms ns_per_query=56421.6 allocated_bytes=130891456 mallocs=1140869
pair-5-candidate.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=43.1125ms ns_per_query=2155.6 allocated_bytes=18242912 mallocs=500030
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=97.5399ms ns_per_query=48769.9 allocated_bytes=96753432 mallocs=1060127
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=117.0319ms ns_per_query=58515.9 allocated_bytes=130883784 mallocs=1140804
```
