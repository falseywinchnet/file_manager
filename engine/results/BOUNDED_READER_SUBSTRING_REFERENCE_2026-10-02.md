# Caller-owned bounded checked-reader access — 2026-10-02

Status: **REJECTED for acceptance: existing exact-search performance regression
remains after the single authorized optimization pass.** Correctness evidence
is retained; public substring admission remains CANDIDATE. The candidate edits
have been removed from the active implementation and preserved as research
patches, not an accepted or released implementation.

## Parent archival disposition

Root preserved the initial and corrected candidates plus the identical aggregate
measurement harness under `rejected/bounded-reader-2026-10-02/`. `manifest.json`
records the baseline commit, every source-byte hash and patch hash; source hashes
match the initial/final receipt below. Apply exactly one candidate patch against
baseline `0873f9cb02dbfaf811b1a31c90be3af5db1b100a`, then the optional aggregate
harness patch for the exact-control measurements. Patch line endings are LF;
manifest source-byte hashes identify the original measured files.

Root verified the corrected patch in reverse against the candidate checkout,
restored the six Engine source files to their pre-experiment state, and verified
forward application of both alternatives and the harness against that state.
The active Engine source has no candidate diff. The complete raw observations
below and temporary binaries/logs remain available. Neither frozen PR8 / 6009dd7
nor published release v0.001-alpha.0873f9c contains this rejected implementation.

The independent cache replacement-validity and path-offset checks identified
here remain follow-up candidates for a separate minimal fix and regression
review; they were not silently salvaged into production with the rejected reader.

## Final disposition after one narrow optimization pass

**GIVEN:** parent rejected the initial 13–17% median exact-consumer regression
and authorized one narrow pass to remove nil-budget machinery from the legacy
path while preserving one storage decoder and all bounded checks. No broader
optimization or architecture was opened.

**OBSERVED overhead:** the initial implementation entered reserve/check for
every fixed or string component even with a nil budget; the cache then called
check repeatedly at block/lock/copy boundaries. This inspection identifies
avoidable call work, not a sampled CPU attribution of all regression cost.

The correction changes only reader_access.go and cache.go: nil-budget fixed
reads call the legacy cache kernel directly; strings skip reserve/check calls
and select their cache mode before traversal. The legacy cache kernel preserves
extent checks, locking and failed-replacement invalidation without budget
machinery. Bounded cache checks are unchanged. The storage decoder remains
shared; the two cache traversal kernels do not decode storage records.

**MEASURED final six alternating aggregate pairs**, same baseline binary,
10k ScaleV1 workload, aggregate harness and environment described below:

| Existing control | Baseline median aggregate ns/query | Corrected median aggregate ns/query | Median paired slowdown |
|---|---:|---:|---:|
| Exact path | 2,151.15 | 2,259.85 | **+6.6%** |
| Name first 100 | 48,245.25 | 51,713.70 | **+7.9%** |
| Name pipeline first 100 | 59,037.95 | 65,875.05 | **+9.8%** |

Query digests and reference checks pass. Median allocated bytes/query remain
approximately 912.13/912.12, 48,374.61/48,374.69 and 65,441.38/65,444.60
(baseline/corrected respectively). These aggregate means do not establish
per-query tail latency. Host variance prevents exact causal decomposition;
the remaining regressions do not satisfy the parent's acceptance gate.

**REJECTED:** the optimization reduces the observed regression but does not
remove it. Do not promote this candidate or infer performance compatibility
from passing correctness checks. No second optimization pass was undertaken.
The corrected candidate remains available for root review; no commit or public
activation was made. Original negative source copies and timings remain intact.

Final-source validation passed after the correction:

- `go test -p 2 ./...`: generation 2.659 s, benchmarks 0.640 s.
- `go test -race -p 2 ./...`: generation 4.428 s, benchmarks 5.048 s.
- `go vet -p 2 ./...`: no diagnostics.

Some unaffected packages reused Go cache. Bounded fixture correctness remains
covered by these runs; the bounded-versus-formula performance numbers later in
this receipt belong to the initial candidate and were not rerun as final-source
timings. Final timings here measure the existing unbudgeted controls.

Final changed hashes (the other four source hashes in the initial table remain
unchanged):

- reader_access.go: `ed42447f0094bf161cec9b0d7f0804906c74ff41e739874d1c7b3d5a0e417ecb`.
- cache.go: `6aef2679f23b1615ae7a3ea07c59dc10a4030f81ba194691ce9dec5fd897ea77`.
- optimized-aggregate.test.exe: `86303724b21c14b6ce01e4bab1eb535b51c1982498897d45904081133f701d69`.

Final temporary source is under `.build/bounded-reader-exact-2026-10-02/optimized/`;
original rejected candidate remains under `working/`. Final raw logs are
`logs/correction-{0..5}-{baseline,optimized}.txt`. The aggregate harness hash is
unchanged at `e15ee1d716ffbc3b09bd4a9ac7afb5ad0d2b443ea713a68cd986a342655f2c52`.
These timings followed root's CPU-clear message and completed before releasing
the local CPU. Both binaries were compiled before timing.

The two corrected methods and legacy cache kernel were reviewed again against
the complete house style: explicit types/initialization, boundary mode
selection, lock exits, stable block storage, borrow lifetime, failure validity
and preservation of bounded checks. No new style violation was identified in
that reviewed scope. Unchanged legacy limitations listed below still apply.

Parent subsequently reported PR7 merged at
`f03ae65034b487d87bd193cca48a9c3a5154ff27`, complete tree
`2ca745b1aa7f4784951b41c2428ff50ede865a75`, identical to tested `0873f9c`, and
switched the shared checkout to `codex/file-manager-search-admission` without
changing these owned files. This rejected work remains excluded from that
release.

## Scope and reproduction anchors

**GIVEN:** parent authorized Engine-internal bounded access, its tests, adaptation
of the private substring experiment, and this new receipt. No engine/api,
wire/capability, storage-format, production search activation, or registry change
is included. The original
[substring receipt](SUBSTRING_REFERENCE_EXPERIMENT_2026-10-02.md) remains unchanged
historical evidence.

Parent-reported baseline: PR7 source `0873f9c`, including baseline experiment
`69b5eca`, over production dependency base
`47e24cd70109075ee327abcdeaa50a5bdba1cc30`. The latter has the same source tree
as released `6e74434`. The parent explicitly excluded this bounded-reader work
from PR7. The later parent-requested committed-baseline comparison used a
read-only `git archive 0873f9c engine` export into .build; no shared checkout,
index, branch, commit or remote was modified by this assignment. File hashes below
identify exactly the initial candidate's tested edits, independently of subsequent parent branch
management.

| File (relative to engine/) | SHA-256 |
|---|---|
| internal/generation/read_budget.go | `4119c905b029896e57f90d6fc6438d03d755ca95190319ebd9fa634d84445b25` |
| internal/generation/reader_access.go | `46e908c71b736726689eaea5069dc9ece14aaaa89a9b886824c62fb318f7fc28` |
| internal/generation/read_budget_test.go | `d57ed9506923911fd9af5380e5d60d6a59c3dbb682515db70a8f00836b5cc937` |
| internal/generation/cache.go | `d5f49309756162ab48048a2c4ecba204152feb80b0a81b4522bf2b3a1706a431` |
| internal/generation/segment.go | `3426cb45e0eaefc7fe90ad9818d0490e8016e94fda217ad23e12db354e784dc0` |
| benchmarks/substring_reference_experiment_test.go | `fbe33f97b0946ff83320dc775ee7d9ef823d086a2605075c9a73aa209ce6d194` |

## Access, accounting and failure contract

**OBSERVED:** `NewReadBudget` creates caller-owned context, remaining logical
bytes, maximum stored-string bytes, consumed bytes and terminal error state.
`RowBounded` and `RecordBounded` borrow that state only during a call. Reader
never installs or retains mutable query state. One sequential owner uses each
budget; independent owners may concurrently use the same checked immutable
reader. Zero/nil budgets are rejected at bounded entry points. The reader must
already have passed integrity checking and remain open throughout the borrow;
this API does not perform or replace Check, own a lease, or make concurrent
Check/Close safe.

Row, record, binding, object, path and string decoding now share the
`readerAccess` implementation. Existing exact methods construct an access with
no budget. Storage layout, checksums, object/binding joins, parent observations,
ordinal checks and result projection remain shared; there is no duplicate
decoder or complete query-time catalogue mirror.

Logical charge is each requested fixed record or stored-string length, including
repeated accesses and cache hits. A Row charges binding + two object records +
name + path-table entry + path. Record charges binding + one object record +
name + path-table entry + path. No charge is made for a reservation refused by
context, string limit or remaining bytes. An admitted read is charged in full
before cache access, including failed reads and cancellation after admission.
The remaining-byte comparison precedes subtraction/addition, preventing counter
wrap even for large caller allowances.

Stored strings pass the existing format/component bounds and then caller
string/byte limits **before** allocating their byte slice. The existing format
still limits each stored string to 1 MiB; a caller can choose less. Returned
strings own their storage. Fixed decoder buffers are local. Logical charges do
not count root-path joining, normalization copies, returned string copies,
encoded responses, allocator rounding or physical cache prefetch.

Every bounded entry-point error returns an empty row/record and false. Its first
error is terminal; later calls on that budget do no further reads or charges.
Out-of-range ordinals remain a successful absent result on a live budget.
A fresh budget is required for a deliberate restart. Earlier charges are not
rolled back. Cache warming or invalidation may survive a failed call, but no
partial decoded result is published. When cancellation and an I/O failure are
both observed at native-read return, the already recorded cancellation remains
the terminal bounded-call error.

Context checks occur before reservations, around cache access and copying,
after lock acquisition, immediately before and after individual native
ReadAt calls, and before bounded result publication. A synchronous native
ReadAt or mutex wait **cannot be interrupted until it returns**. These are
cooperative boundaries, not a hard wall-time or physical-I/O limit.

Two integrity details were tightened during source review:

- A cache replacement is marked invalid before bytes are overwritten. A failed
  or cancelled replacement cannot expose its partially replaced bytes under the
  previous valid block number. The existing per-slot locking remains authority.
- A stored path offset is checked against remaining component extent before
  adding the path-table length, preventing unsigned addition wrap.

Both changes reject invalid intermediate/storage states; valid exact behavior
is retained. The cache borrows an `io.ReaderAt` (production still supplies the
same os.File), allowing named generated-fixture probes at the existing I/O
boundary. It does not own or close the file.

## Tests and controls

**MEASURED:** all added tests pass, including under the full race run.

| Test | Evidence |
|---|---|
| Exact boundaries/cache | Less than one binding refuses before read/cache allocation; exact binding allowance stops before object; exact complete Row cost succeeds; one byte less refuses final string without partial output; warm and reopened cold readers charge identically |
| Long strings | A 64 KiB generated name is refused under a 32-byte limit; both oversized-string and insufficient-logical-byte direct string paths show zero allocations in 100 AllocsPerRun iterations and no reads; explicitly admitted long records equal legacy output |
| Cancellation/failed reads | Real context cancellation after native read; pre-cancel before any charge; explicit cancellation between binding/object decoding; admitted failed reads remain charged; later calls remain terminal |
| Concurrent owners | Two admitted query owners and one exhausted owner share one reader; bounded rows/records equal independently called legacy methods; no shared mutable budget or race reported |
| Cache block boundaries | Cancellation stops a two-block request after first native read while retaining full logical reservation; injected failed collision invalidates slot and subsequent old-block read reloads correct bytes |
| Corruption | Wrapped path offset rejects with empty result; restoring bytes does not revive failed budget; legacy consumer remains usable with restored fixture |

Existing generation exact-name/path/identity/reference, format/digest,
corruption, publication/recovery and other Engine controls ran unchanged as
part of the full suite. Their passing results establish compatibility within
their fixtures, not new platform or service admission.

The private substring experiment now creates a budget per page:
**4 MiB logical requested bytes, 64 KiB per stored string**, alongside the
existing 257-row and 128-result limits. These limits are private experiment
choices, not public policy. An error discards the page and does not publish a
partial cursor. The existing independent streaming formula oracle is unchanged;
all ten queries still match complete record digests. The logical corpus remains
`83dccac06d0ee44ada61c0e97442c1485b89254727021642d4eafbb05f98afa1`;
segment size remains **1,590,176 bytes** for 10,000 bindings. Absolute temporary
roots make result digests vary between runs; both lanes use the same root
within each comparison.

Whole-traversal logical reads: selective `ancestor/014` **2,172,200** bytes;
no-match **2,156,000**; all-match `a` **3,672,000**. These are cumulative across
pages, not physical bytes or a measured peak.

## Initial candidate validation and paired performance (retained)

**MEASURED environment:** Windows 11 Home 10.0.22621, amd64, C: NTFS,
reported AMD EPYC 9354 32-Core Processor, 16,757,176 KiB visible RAM,
Go 1.27.1, GOMAXPROCS=2, -p 2. Borrowed GCC remained read-only.
Parent confirmed its final C++ build complete before timing; full Go checks
also completed before timing. External host load/power/storage hardware were
not controlled or independently characterized.

Final-source commands all succeeded:

- `go test -p 2 ./...`: generation 2.662 s; benchmarks 0.625 s.
- `go test -race -p 2 ./...`: generation 4.026 s; benchmarks 4.889 s.
- `go vet -p 2 ./...`: no diagnostics.
- Opt-in uncached focused correctness and paired run: package 2.878 s.

Some full-suite packages reused Go cache; the full commands did not force
uncached execution. No test failure occurred during this assignment. The last
full run follows the final cache cancellation guard and path-offset regression.

Thirty pairs per query alternate order (even pairs formula first, odd pairs
reader first), after reader warmup. Both complete record digests are checked
each pair; allocation snapshots and logging sit outside timed intervals.
Nearest-rank p99 equals the maximum of only 30 observations and is not a stable
tail estimate. No cold cache, first-page, GUI/service latency or causal
before/after performance claim is made.

| Query | Formula p50 / p95 / p99=max, ms | Bounded reader p50 / p95 / p99=max, ms |
|---|---|---|
| ancestor/014 | 6.5070 / 9.5148 / 11.6085 | 6.5048 / 8.5019 / 9.5107 |
| missing | 6.0109 / 8.0661 / 8.5160 | 6.0206 / 7.0267 / 8.5109 |
| a | 23.0172 / 25.0033 / 26.0257 | 24.6114 / 28.5774 / 29.1025 |

| Query | Formula mean bytes / allocations | Bounded reader mean bytes / allocations |
|---|---:|---:|
| ancestor/014 | 2,989,044 / 82,821 | 4,307,521 / 80,493 |
| missing | 2,928,045 / 82,408 | 4,224,707 / 79,580 |
| a | 8,373,636 / 112,434 | 12,965,169 / 169,397 |

Cache retained **1,540,096 bytes**; after-GC heap delta **1,542,640 bytes**.
Combined warmups, both lanes and bookkeeping allocated **1,096,714,616 bytes /
18,544,181 allocations**. The formula control regenerates records and is not an
optimized store competitor.

**Retained negative:** the bounded reader allocates more bytes for every query
and loses all-match p50/p95/max to the formula control in this run. Its read
checks are observable work, not a speed optimization. Historical unbudgeted
reader medians were lower, but separate-run timing cannot isolate causal
overhead. No index was added or selected.

## Existing unbudgeted exact-consumer regression control

**MEASURED separately from bounded-versus-formula:** the unchanged existing
`TestImmutableGenerationDistribution` was run from exported `0873f9c` and from
an identical temporary source copy with only the six Engine files listed above
overlaid. Temporary copies, precompiled test binaries and raw logs are retained
under `engine/.build/bounded-reader-exact-2026-10-02/`. The shared checkout was
never checked out or replaced. Both binaries were compiled before timing,
GOMAXPROCS=2; no C++ build overlapped. The original benchmark source hash in both
copies was `da2ee2de3056352799be7bed5097bca2f1e525060e6f8d59e6130409b1e267d9`.

The existing ScaleV1(10000,10,10) control measures 20,000 unique-path calls,
2,000 repeated-name first-100 calls and 2,000 exact-pipeline first-100 calls.
Six pairs alternated baseline-first on even pair numbers and working-first on
odd numbers. All runs passed reference checks and retained identical logged
query digests: path `6c4437450b9c86ff`, name/pipeline `a69b9113a32327fd`.

**Retained measurement limitation:** individual-call timing was under-resolved:
path p50/p95/p99 and name/pipeline p50 were zero in these runs. Name/pipeline
p95 clustered near 500 microseconds. This does not mean zero-cost operations
or demonstrate absence of regression. Original logs remain as
`logs/pair-{0..5}-{baseline,working}.txt`.

To make the same workload informative, both temporary copies then received
identical aggregate timing and memory snapshots around the existing sample
loops. No query operation or production code was changed for that addition.
Whole-loop elapsed divided by operation count avoids the per-call zero
readings. It includes the existing timestamp/check/digest bookkeeping equally
in both variants. These are aggregate means, **not per-query percentiles**.
Six further alternating pairs were run. Raw logs are
`logs/aggregate-{0..5}-{baseline,working}.txt`, retaining the existing distributions,
digests, build/recovery/storage/update observations and aggregate timings.

| Existing control | Baseline median aggregate ns/query | Working median aggregate ns/query | Median paired slowdown | Baseline / working median allocated bytes/query |
|---|---:|---:|---:|---:|
| Exact path | 2,054.05 | 2,415.60 | +17.4% | 912.12 / 912.12 |
| Name first 100 | 47,191.75 | 53,431.45 | +12.9% | 48,374.87 / 48,375.20 |
| Name pipeline first 100 | 58,018.15 | 68,213.50 | +17.4% | 65,443.48 / 65,445.95 |

Median means the mean of the middle two sorted values for six runs. Paired
slowdown is computed separately for each working/baseline pair, then its median
is reported; it is not the ratio of the two independently summarized medians.

**Retained negative / admission concern:** the shared decoder/cache changes
measurably slow these existing unbudgeted exact controls. Correctness passes
do not establish performance compatibility. This receipt does not recommend
production promotion on that basis; root must assess the regression or open a
separate evidence-led correction. No additional optimization was made.

Aggregate harness/source/binary anchors:

- Identically instrumented temporary `benchmarks/m2_generation_test.go`:
  `e15ee1d716ffbc3b09bd4a9ac7afb5ad0d2b443ea713a68cd986a342655f2c52`.
- Exported baseline archive: `dc74fbb0670e0d7c54d7d29a267afbb77e4e9062b2cd58070abdb1aa90e5178a`.
- `baseline-aggregate.test.exe`: `11f74514c1928a7dcba962b03458b27c98770d0055afb8810e01a04f39f1037b`.
- `working-aggregate.test.exe`: `f5bf99c24769b6c40ae47573c2ab4b54fb05bf96811765f1c159f1f7fe588f72`.

The added aggregate declarations and logging were reviewed for explicit types,
initialization and measurement ordering; unchanged legacy benchmark source is
not certified house-style compliant. Re-run retained binaries with
`FILEMAN_ENGINE_M2_MEASURE=1`, `FILEMAN_ENGINE_RECORDS=10000`, `GOMAXPROCS=2`,
arguments `-test.run=^TestImmutableGenerationDistribution$ -test.v -test.count=1`.

## Remaining limits and source review

The logical-read admission and per-string pre-allocation gates are now
implemented and tested privately. **Still unproved:** total allocation/peak
query scratch (including caller copies), encoded response budget, physical
prefetch bytes, hard cancellation latency, native long-path/identity coverage,
million-entry resource targets, service leases and public cursor security.
A 64 KiB accepted string and 128-result page are not an 8 MiB peak-scratch proof.
No promotion of the public substring proposal follows automatically.

**Source review against complete planning/PROGRAMMING_HOUSE_STYLE.md:**
all of read_budget.go, reader_access.go, read_budget_test.go; cache's changed
file interface and readAt/readAtControlled; segment's seven decoder delegation
methods (Row, recordAt, bindingAt, objectAt, pathAt, readString,
readBoundedString); the complete adapted experiment; and reproduction commands
below. Explicit types/initialization, named callback targets and state, budget
ownership, reader borrows, operation order, charge arithmetic, empty failure
results, lock exits and repeated-loop work were reviewed. No remaining
violation was identified in that scope. Format decoding still uses the
existing decodeObject implementation. Unchanged legacy code, including cache
bytes(), segment writer closures, inferred declarations and implicit zero
initialization elsewhere, is not certified or rewritten by this review.
Existing decoder/string allocation remains disclosed rather than described as
allocation-free.

## Reproduction

From engine/, after coordinating heavy local builds:

```powershell
[string]$substringGoBin = [Environment]::GetEnvironmentVariable('FILE_MANAGER_GO_BIN', 'User')
$env:PATH = "$substringGoBin;C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin;" + $env:PATH
$env:GOTOOLCHAIN = 'local'
$env:GOMAXPROCS = '2'
$env:CGO_ENABLED = '1'
$env:CC = 'C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin\gcc.exe'
Remove-Item Env:FILEMAN_SUBSTRING_REFERENCE_MEASURE -ErrorAction SilentlyContinue
go test -p 2 ./...
go test -race -p 2 ./...
go vet -p 2 ./...
$env:FILEMAN_SUBSTRING_REFERENCE_MEASURE = '1'
go test -p 2 -count=1 -run '^TestSubstringReference' -v ./benchmarks
```

## Raw corrected exact-control aggregate observations

```text
correction-0-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=44.1393ms ns_per_query=2207.0 allocated_bytes=18242248 mallocs=500021
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=102.0938ms ns_per_query=51046.9 allocated_bytes=96749000 mallocs=1060132
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=115.7865ms ns_per_query=57893.2 allocated_bytes=130893160 mallocs=1140844
correction-0-optimized.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=48.1184ms ns_per_query=2405.9 allocated_bytes=18241640 mallocs=500013
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=111.8087ms ns_per_query=55904.3 allocated_bytes=96749376 mallocs=1060131
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=133.0055ms ns_per_query=66502.8 allocated_bytes=130911952 mallocs=1140995
correction-1-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=42.9011ms ns_per_query=2145.1 allocated_bytes=18242664 mallocs=500027
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=93.3795ms ns_per_query=46689.8 allocated_bytes=96748264 mallocs=1060108
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=132.7767ms ns_per_query=66388.4 allocated_bytes=130869144 mallocs=1140689
correction-1-optimized.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=42.5591ms ns_per_query=2128.0 allocated_bytes=18242496 mallocs=500024
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=104.9592ms ns_per_query=52479.6 allocated_bytes=96749392 mallocs=1060134
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=128.8601ms ns_per_query=64430.1 allocated_bytes=130879208 mallocs=1140772
correction-2-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=44.6611ms ns_per_query=2233.1 allocated_bytes=18242136 mallocs=500020
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=101.5403ms ns_per_query=50770.2 allocated_bytes=96748568 mallocs=1060125
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=118.1921ms ns_per_query=59096.1 allocated_bytes=130884896 mallocs=1140816
correction-2-optimized.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=47.5755ms ns_per_query=2378.8 allocated_bytes=18242416 mallocs=500023
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=101.8956ms ns_per_query=50947.8 allocated_bytes=96749280 mallocs=1060128
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=130.4946ms ns_per_query=65247.3 allocated_bytes=130889384 mallocs=1140847
correction-3-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=42.5493ms ns_per_query=2127.5 allocated_bytes=18242744 mallocs=500028
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=98.0975ms ns_per_query=49048.8 allocated_bytes=96755488 mallocs=1060140
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=117.9597ms ns_per_query=58979.8 allocated_bytes=130886216 mallocs=1140823
correction-3-optimized.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=45.3474ms ns_per_query=2267.4 allocated_bytes=18242528 mallocs=500023
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=113.5487ms ns_per_query=56774.3 allocated_bytes=96747776 mallocs=1060113
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=133.8031ms ns_per_query=66901.6 allocated_bytes=130901896 mallocs=1140955
correction-4-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=41.5153ms ns_per_query=2075.8 allocated_bytes=18242944 mallocs=500028
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=94.8033ms ns_per_query=47401.7 allocated_bytes=96750208 mallocs=1060137
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=114.2052ms ns_per_query=57102.6 allocated_bytes=130880632 mallocs=1140781
correction-4-optimized.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=45.0453ms ns_per_query=2252.3 allocated_bytes=18242416 mallocs=500022
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=98.0532ms ns_per_query=49026.6 allocated_bytes=96750384 mallocs=1060140
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=124.5915ms ns_per_query=62295.8 allocated_bytes=130888880 mallocs=1140848
correction-5-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=43.1431ms ns_per_query=2157.2 allocated_bytes=18242648 mallocs=500026
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=94.8834ms ns_per_query=47441.7 allocated_bytes=96749424 mallocs=1060129
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=126.2714ms ns_per_query=63135.7 allocated_bytes=130875728 mallocs=1140742
correction-5-optimized.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=43.2179ms ns_per_query=2160.9 allocated_bytes=18242664 mallocs=500026
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=100.8118ms ns_per_query=50405.9 allocated_bytes=96752032 mallocs=1060163
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=134.8401ms ns_per_query=67420.1 allocated_bytes=130889008 mallocs=1140845
```

## Raw initial exact-control aggregate observations

```text
aggregate-0-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=41.0433ms ns_per_query=2052.2 allocated_bytes=18242232 mallocs=500020
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=95.4736ms ns_per_query=47736.8 allocated_bytes=96748632 mallocs=1060115
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=113.5818ms ns_per_query=56790.9 allocated_bytes=130876408 mallocs=1140745
aggregate-0-working.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=49.5792ms ns_per_query=2479.0 allocated_bytes=18242400 mallocs=500022
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=102.9136ms ns_per_query=51456.8 allocated_bytes=96749024 mallocs=1060120
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=134.8867ms ns_per_query=67443.4 allocated_bytes=130887760 mallocs=1140836
aggregate-1-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=41.115ms ns_per_query=2055.8 allocated_bytes=18242480 mallocs=500024
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=93.2933ms ns_per_query=46646.7 allocated_bytes=96749536 mallocs=1060122
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=118.3443ms ns_per_query=59172.2 allocated_bytes=130879960 mallocs=1140743
aggregate-1-working.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=47.9324ms ns_per_query=2396.6 allocated_bytes=18242168 mallocs=500018
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=110.031ms ns_per_query=55015.5 allocated_bytes=96750792 mallocs=1060147
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=132.8213ms ns_per_query=66410.6 allocated_bytes=130891008 mallocs=1140865
aggregate-2-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=42.0236ms ns_per_query=2101.2 allocated_bytes=18242760 mallocs=500024
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=90.0556ms ns_per_query=45027.8 allocated_bytes=96749392 mallocs=1060129
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=115.7733ms ns_per_query=57886.7 allocated_bytes=130893952 mallocs=1140889
aggregate-2-working.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=48.0754ms ns_per_query=2403.8 allocated_bytes=18242416 mallocs=500022
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=110.1249ms ns_per_query=55062.4 allocated_bytes=96748256 mallocs=1060118
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=140.2952ms ns_per_query=70147.6 allocated_bytes=130892776 mallocs=1140842
aggregate-3-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=41.0453ms ns_per_query=2052.3 allocated_bytes=18242496 mallocs=500024
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=97.9584ms ns_per_query=48979.2 allocated_bytes=96749936 mallocs=1060133
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=133.9192ms ns_per_query=66959.6 allocated_bytes=130900672 mallocs=1140939
aggregate-3-working.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=48.5483ms ns_per_query=2427.4 allocated_bytes=18242648 mallocs=500026
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=103.6948ms ns_per_query=51847.4 allocated_bytes=96751800 mallocs=1060163
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=149.5172ms ns_per_query=74758.6 allocated_bytes=130904832 mallocs=1140976
aggregate-4-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=47.5124ms ns_per_query=2375.6 allocated_bytes=18243224 mallocs=500030
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=102.0604ms ns_per_query=51030.2 allocated_bytes=96750136 mallocs=1060134
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=111.1039ms ns_per_query=55551.9 allocated_bytes=130902392 mallocs=1140956
aggregate-4-working.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=47.6235ms ns_per_query=2381.2 allocated_bytes=18242384 mallocs=500022
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=100.0738ms ns_per_query=50036.9 allocated_bytes=96750000 mallocs=1060135
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=129.0554ms ns_per_query=64527.7 allocated_bytes=130894640 mallocs=1140891
aggregate-5-baseline.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=40.0496ms ns_per_query=2002.5 allocated_bytes=18242264 mallocs=500021
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=91.0714ms ns_per_query=45535.7 allocated_bytes=96753880 mallocs=1060126
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=116.2991ms ns_per_query=58149.6 allocated_bytes=130876200 mallocs=1140745
aggregate-5-working.txt
    m2_generation_test.go:148: aggregate=sampleGenerationPath calls=20000 elapsed=48.6402ms ns_per_query=2432.0 allocated_bytes=18242648 mallocs=500026
    m2_generation_test.go:149: aggregate=sampleGenerationName calls=2000 elapsed=119.1937ms ns_per_query=59596.8 allocated_bytes=96750856 mallocs=1060142
    m2_generation_test.go:154: aggregate=sampleGenerationPipeline calls=2000 elapsed=137.9671ms ns_per_query=68983.6 allocated_bytes=130883216 mallocs=1140798
```

## Raw final correctness and paired output

Pair arrays retain execution order before sorting.

```text
=== RUN   TestSubstringReferenceExperiment
    substring_reference_experiment_test.go:342: fixture=formula-v1 records=10000 corpus_sha256=83dccac06d0ee44ada61c0e97442c1485b89254727021642d4eafbb05f98afa1 segment_bytes=1590176
    substring_reference_experiment_test.go:369: query="ancestor" scope="" descendants=true matches=10000 pages=79 empty_pages=0 examined=10000 logical_read_bytes=3672000 digest=c5d4fdf27fb22480eea61ecb52c76ba53532bb05fe095711b8bbdd6729464f7a
    substring_reference_experiment_test.go:369: query="ancestor/014" scope="" descendants=true matches=100 pages=39 empty_pages=0 examined=10000 logical_read_bytes=2172200 digest=3478540f7bde45ad90741c168bf12c75a883b15684000d0b36ac4377d17a5f2b
    substring_reference_experiment_test.go:369: query="a" scope="" descendants=true matches=10000 pages=79 empty_pages=0 examined=10000 logical_read_bytes=3672000 digest=c5d4fdf27fb22480eea61ecb52c76ba53532bb05fe095711b8bbdd6729464f7a
    substring_reference_experiment_test.go:369: query="ÉCLAIR" scope="" descendants=true matches=1400 pages=39 empty_pages=0 examined=10000 logical_read_bytes=2382800 digest=158749d871bd9f3e28ef506288eefa98b6b4a193ac6356c4950d517f326862f6
    substring_reference_experiment_test.go:369: query="é" scope="" descendants=true matches=1400 pages=39 empty_pages=0 examined=10000 logical_read_bytes=2382800 digest=158749d871bd9f3e28ef506288eefa98b6b4a193ac6356c4950d517f326862f6
    substring_reference_experiment_test.go:369: query="missing" scope="" descendants=true matches=0 pages=39 empty_pages=39 examined=10000 logical_read_bytes=2156000 digest=e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
    substring_reference_experiment_test.go:369: query="ancestor" scope="bucket-050-AnCeStOr" descendants=false matches=99 pages=39 empty_pages=38 examined=10000 logical_read_bytes=2171018 digest=d7bfcc9de2e797f3c886355a546da9f1a0ff0d60051c0e8569c5aafc161a4ea6
    substring_reference_experiment_test.go:369: query="ancestor" scope="" descendants=false matches=100 pages=39 empty_pages=0 examined=10000 logical_read_bytes=2170200 digest=6ba7248d0d966eac390ecdfa79ef048d23a4db545be8aa1662590e97d6b2c756
    substring_reference_experiment_test.go:369: query="099-plain" scope="" descendants=true matches=100 pages=39 empty_pages=0 examined=10000 logical_read_bytes=2171000 digest=f66c958b28423828331194b8c070f74187db0974ded034949eb5c0fdce1b7a87
    substring_reference_experiment_test.go:369: query="plain" scope="notes..old" descendants=true matches=0 pages=39 empty_pages=39 examined=10000 logical_read_bytes=2156000 digest=e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
    substring_reference_experiment_test.go:395: fixture=formula-v1 records=10000 corpus_sha256=83dccac06d0ee44ada61c0e97442c1485b89254727021642d4eafbb05f98afa1 segment_bytes=1590176
--- PASS: TestSubstringReferenceExperiment (0.27s)
=== RUN   TestSubstringReferenceMeasurement
    substring_reference_experiment_test.go:437: fixture=formula-v1 records=10000 corpus_sha256=83dccac06d0ee44ada61c0e97442c1485b89254727021642d4eafbb05f98afa1 segment_bytes=1590176
    substring_reference_experiment_test.go:489: query="ancestor/014" raw_formula=[6.513ms 6.0133ms 7.5719ms 6.5023ms 6.5056ms 6.5241ms 6.5129ms 6.0168ms 6.5105ms 6.0121ms 6.4311ms 7.5109ms 6.4954ms 6.507ms 6.0135ms 6.0183ms 6.499ms 6.0035ms 6.5096ms 6.5106ms 6.0024ms 6.4984ms 6.5129ms 7.0777ms 6.5076ms 11.6085ms 9.5148ms 6.6652ms 6.0057ms 6.5077ms] raw_reader=[7.0271ms 6.9992ms 6.4995ms 7.0136ms 7.0115ms 6.0623ms 5.9997ms 6.5196ms 7.5111ms 6.5001ms 6.5149ms 7.5158ms 6.072ms 6.5048ms 8.5019ms 6.0042ms 6.0169ms 6.5661ms 8.0143ms 6.4959ms 7.5286ms 6.0178ms 6.5035ms 7.007ms 6.0723ms 7.3557ms 9.5107ms 6.0036ms 6.4989ms 6.0118ms]
    substring_reference_experiment_test.go:492: query="ancestor/014" pairs=30 formula_p50=6.507ms p95=9.5148ms p99=11.6085ms max=11.6085ms reader_p50=6.5048ms p95=8.5019ms p99=9.5107ms max=9.5107ms
    substring_reference_experiment_test.go:495: query="ancestor/014" mean_formula_bytes=2989044 mean_reader_bytes=4307521 mean_formula_allocs=82821 mean_reader_allocs=80493
    substring_reference_experiment_test.go:489: query="missing" raw_formula=[6.5091ms 5.5041ms 6.5181ms 6.849ms 5.5025ms 6.0096ms 6.0128ms 6.0225ms 6.0109ms 6.0117ms 6.0866ms 8.0069ms 7.0049ms 5.5111ms 6.5784ms 5.9993ms 6.0077ms 6.0135ms 6.0022ms 6.5966ms 5.5053ms 5.9962ms 5.5749ms 5.5081ms 5.9992ms 8.516ms 8.0661ms 6.0044ms 6.0337ms 5.5007ms] raw_reader=[5.4995ms 6.0121ms 6.009ms 4.9978ms 5.5168ms 7.0267ms 5.9989ms 6.0786ms 7.0197ms 6.9974ms 6.5081ms 6.0608ms 6.0065ms 6.5023ms 7.0079ms 6.5073ms 6.0075ms 6.5029ms 6.0206ms 4.9981ms 5.7743ms 6.5239ms 6.5174ms 5.9972ms 6.0155ms 5.9982ms 8.5109ms 6.5396ms 6.0125ms 6.5048ms]
    substring_reference_experiment_test.go:492: query="missing" pairs=30 formula_p50=6.0109ms p95=8.0661ms p99=8.516ms max=8.516ms reader_p50=6.0206ms p95=7.0267ms p99=8.5109ms max=8.5109ms
    substring_reference_experiment_test.go:495: query="missing" mean_formula_bytes=2928045 mean_reader_bytes=4224707 mean_formula_allocs=82408 mean_reader_allocs=79580
    substring_reference_experiment_test.go:489: query="a" raw_formula=[22.029ms 22.5285ms 23.5288ms 22.614ms 24.0792ms 21.5856ms 23.5768ms 22.0204ms 24.0936ms 21.1641ms 23.5862ms 24.8245ms 22.583ms 25.0033ms 24.6231ms 22.03ms 21.514ms 23.0172ms 26.0257ms 24.5255ms 23.0413ms 23.3227ms 23.0972ms 23.5524ms 22.5989ms 22.5227ms 22.6556ms 22.0214ms 23.0829ms 21.5995ms] raw_reader=[25.6342ms 24.6375ms 24.3537ms 26.1117ms 27.0726ms 24.0521ms 24.5792ms 25.533ms 23.0189ms 25.5457ms 23.8233ms 29.1025ms 24.0849ms 26.5122ms 24.0172ms 24.0436ms 24.5332ms 27.6108ms 24.1011ms 27.3329ms 25.6107ms 24.5902ms 28.5774ms 24.6114ms 24.6597ms 23.0671ms 25.0236ms 26.1149ms 23.576ms 24.0747ms]
    substring_reference_experiment_test.go:492: query="a" pairs=30 formula_p50=23.0172ms p95=25.0033ms p99=26.0257ms max=26.0257ms reader_p50=24.6114ms p95=28.5774ms p99=29.1025ms max=29.1025ms
    substring_reference_experiment_test.go:495: query="a" mean_formula_bytes=8373636 mean_reader_bytes=12965169 mean_formula_allocs=112434 mean_reader_allocs=169397
    substring_reference_experiment_test.go:502: os=windows arch=amd64 go=go1.27.1 gomaxprocs=2 row_cap=257 result_cap=128 cache_bytes=1540096 combined_alloc_bytes=1096714616 combined_mallocs=18544181 retained_heap_delta=1542640
--- PASS: TestSubstringReferenceMeasurement (2.31s)
PASS
ok  	filemanager/engine/benchmarks	2.878s
```
