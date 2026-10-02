# Private checked-generation substring reference experiment

2026-10-02, Shadow. **CANDIDATE private experiment; MEASURED generated-fixture
results.** Parent opened only new benchmark/test code and this receipt. The
public substring proposal remains unreconciled; no runtime, API, capability,
index, storage selection or service deployment changed.

Source: `benchmarks/substring_reference_experiment_test.go`, SHA-256
`eca2ede6928f35f3395f3a6b28f47246580d989725fcbe3477f5d60567121c03`.
The investigation began from the parent-reported `17a748f` integration with
concurrent parent Details work. The current production dependency base is
parent-confirmed `47e24cd70109075ee327abcdeaa50a5bdba1cc30`, whose source tree
is identical to released `6e74434` (tree
`aba13e40b6a8a4fc10a1d065f39732a905792b25`). The parent switched the shared
checkout to `codex/file-manager-search-reference` without changing file content.
No Git command was used here to independently assert that base. The exact
experiment source hash is the reproduction anchor; production dependencies
were read-only. This experiment is subsequent work, not part of that release.

## Reader feasibility and explicit blockers

**OBSERVED:** `internal/generation/segment.go`, `Reader.Row`, decodes one binding,
object, parent, name and ordered path. `Reader.Record` resolves the exact joined
stored record for a matching ordinal. `Len`, `Metadata` and `Root` permit bounded
iteration of an already checked reader. The experiment writes a fixture segment,
opens it and successfully calls `Check` with the expected segment digest before
queries. It neither invokes `CandidateAll` nor reconstructs the catalogue on
the query heap.

**OBSERVED bounds:** `readBoundedString` rejects lengths above
`maximumStoredString` (1 MiB) before allocation. The direct-mapped read cache is
capped at 256 × 32 KiB = 8 MiB. Thus one decode has a format limit, and a fixed
row/page count gives a finite working set. This is not the proposed 8 MiB
query-scratch guarantee: decoded names/paths may each approach 1 MiB, normalized
strings and copies allocate, and a 128-record page can retain large records.

**BLOCKERS for the proposed public budget contract:** Row/Record take no context,
byte allowance or caller scratch; their internal cache reads expose no requested-
byte accounting. The private experiment cannot enforce a pre-read 4 MiB page
budget, bound response encoding, or interrupt a blocked `ReadAt`. Charging string
lengths after decoding would not establish that guarantee. No such accounting
is fabricated. The reader also decodes name/object/parent for nonmatches, then
repeats several reads for matching Record calls. The retained negative below
shows substantial allocation even on this small corpus. A future bounded-read
adapter/projection needs separate implementation authorization and source review.

## Fixture and independent oracle

**OBSERVED:** formula-v1 has exactly 10,000 bindings: 100 named directories and
99 files per directory. Paths contain mixed-case `AnCeStOr`; every seventh file
contains `Éclair` and decomposed `e` plus combining acute. The last pair of files
per directory shares a fixture object identity and intrinsic metadata, yielding
100 synthetic hard-link pairs. These are logical records, not a native hard-link
or identity oracle. No user files are read, and no source tree is scanned.
Segment and logical-source paths are separately generated with `t.TempDir`.

Corpus digest:
`83dccac06d0ee44ada61c0e97442c1485b89254727021642d4eafbb05f98afa1`.
Segment size: **1,590,176 bytes**, or 159.0176 bytes per binding for this synthetic
record distribution. This is not an added substring-index size or representative
384-byte/object gate result.

Construction uses the existing reference shard/writer and therefore temporarily
has a full construction corpus. Those owners leave scope before query measurement;
GC runs before measured query allocation/retained-heap baselines. The query path
retains only the checked reader, a page of at most 128 records, cursor and digest
state. Each page examines at most 257 rows; normalization state is local to a row.

The independent oracle regenerates formula entries one at a time, uses component
comparison for scope, and tests basename OR root-relative path. It never reads
the generation or calls the candidate scan's eligibility helper. Both sides
stream complete records into JSON/SHA-256, comparing identity, address, name,
kind, size, mode, timestamp and order—not just result counts. The formula oracle
is a correctness/control baseline, not a mature store or optimized in-memory
search competitor. Timings include its formula generation and both sides'
result serialization/hashing. Absolute temporary roots participate in result
hashes, so match hashes vary between runs; the logical corpus digest is stable.

**MEASURED correctness:** focused tests pass for all rows/pages of these queries:

| Text / scope | Matches | Pages | Empty pages |
|---|---:|---:|---:|
| `ancestor`, root descendants | 10,000 | 79 | 0 |
| `ancestor/014`, root descendants | 100 | 39 | 0 |
| `a`, root descendants | 10,000 | 79 | 0 |
| `ÉCLAIR`, root descendants | 1,400 | 39 | 0 |
| decomposed `e` + acute, root descendants | 1,400 | 39 | 0 |
| `missing`, root descendants | 0 | 39 | 39 |
| `ancestor`, direct children of `bucket-050-AnCeStOr` | 99 | 39 | 38 |
| `ancestor`, root direct children only | 100 | 39 | 0 |
| `099-plain`, root descendants | 100 | 39 | 0 |
| `plain`, valid absent scope `notes..old`, descendants | 0 | 39 | 39 |

Every completed query examines exactly 10,000 rows. Scope pushdown is not claimed.
The scoped case proves ancestor text before the scoped suffix still matches and
empty progressing pages are not confused with exhausted no-match. Cursor replay
produces equal pages; changed-query and changed-generation cursors reject.
A separately written/opened/checked generation 8 rejects a generation-7
continuation before any row read. This tests generation binding, not concurrent
live manifest publication or mutation.

Real `context.WithCancel` is checked before/after reads. A named source wrapper
cancels immediately after the seventh real Row call; the operation returns
`context.Canceled` and no partial page. A pre-cancelled call performs zero reads.
This is deterministic cooperative cancellation at read boundaries, not a
cross-process disconnect or blocked-I/O measurement. The trusted in-process
cursor is not authenticated, root/configuration/session-bound or restart-safe;
it is never exposed as a proposed wire token. Readers are test-owned and closed
with defers. No per-cursor resource is allocated, but abandonment of a service
lease is not exercised.

## Paired measurement: superseded first source

The following first-run tables are retained as superseded evidence. Source hash:
`77ffc305b7689f779e0da6ffa60a911f479cfb819311118420c26b02cf65494b`.
Parent review rejected implicit zero initialization and required hoisted page
invariants, segment-aware scope validation and skipping normalization of
ineligible rows. The revised source and rerun appear below. Raw pair order was
not retained for this first run; its aggregates cannot reconstruct those pairs.

**MEASURED environment:** Windows 11 Home 10.0.22621, amd64, C: NTFS, reported
AMD EPYC 9354 32-Core Processor, 16,757,176 KiB visible RAM; Go 1.27.1,
`GOMAXPROCS=2`, `go test -p 2`. Borrowed MinGW was read-only for race checks.
No intentional concurrent local workload was started; external host load was
not controlled. Remote CI is not this measurement environment.

Each query receives a reader warmup, then 30 paired runs with alternating
formula/reader order. Both outputs are checked after every pair. Nearest-rank
percentiles are used; with only 30 observations p99 equals max. Reader cache is
warm; no OS cache eviction, cold recovery timing or physical-I/O result is
claimed. Durations below are complete 10k traversal plus streaming match digest,
not first-page, first-correct-result, GUI or service latency.

| Query | Formula p50 / p95 / p99=max, ms | Reader p50 / p95 / p99=max, ms |
|---|---|---|
| `ancestor/014` | 6.5105 / 8.8891 / 9.0123 | 5.4986 / 6.5131 / 7.0027 |
| `missing` | 6.0090 / 7.5045 / 9.0078 | 4.9975 / 5.0724 / 6.0051 |
| `a` | 22.5477 / 24.6458 / 25.1066 | 22.5085 / 27.9081 / 28.0814 |

| Query | Formula mean allocated bytes / allocations | Reader mean allocated bytes / allocations |
|---|---:|---:|
| `ancestor/014` | 2,988,481 / 82,817 | 4,305,145 / 80,455 |
| `missing` | 2,928,053 / 82,409 | 4,222,215 / 79,541 |
| `a` | 8,373,375 / 112,431 | 12,959,316 / 169,313 |

Reader cache after the run: **1,540,096 bytes**. After-GC heap growth across the
combined experiment: **1,543,232 bytes**. Combined allocation including warmups,
both lanes and measurement bookkeeping: **1,096,346,368 bytes / 18,538,610
allocations**. Per-lane allocation snapshots are outside timed intervals.
These are Go allocation/heap observations, not peak scratch, RSS/private bytes,
CPU time, physical reads, energy, process handle limits or service idle memory.

**Retained negative result:** reader allocated more bytes for every query and
had a slower all-match p95/p99 than the formula control. Do not claim an overall
speed win from the selective/no-match medians. Both controls are exhaustive;
no acceleration index was built. No failed correctness/test run occurred during
this experiment. Larger/deeper corpora, nested approved roots, malformed stored
strings, corruption, replacement/incarnation semantics and million-entry resource
targets are not established by this 10k fixture.

## Final source rerun and raw pairs

**MEASURED:** same environment, corpus digest, segment size and paired protocol
as above, after the source corrections. This is a small warm component experiment.
The nearest-rank p99 is the observed maximum of 30 samples, not a stable tail
estimate. Host variation and source changes are confounded; timing differences
between runs do not demonstrate a causal improvement from the corrections.

| Query | Formula p50 / p95 / p99=max, ms | Reader p50 / p95 / p99=max, ms |
|---|---|---|
| `ancestor/014` | 6.5102 / 8.0080 / 8.6019 | 5.0083 / 6.5034 / 7.0122 |
| `missing` | 6.0072 / 7.0750 / 8.4990 | 5.0008 / 5.5123 / 6.0054 |
| `a` | 23.0271 / 27.0829 / 27.1477 | 22.5527 / 25.0390 / 26.1360 |

| Query | Formula mean allocated bytes / allocations | Reader mean allocated bytes / allocations |
|---|---:|---:|
| `ancestor/014` | 2,989,030 / 82,821 | 4,305,285 / 80,456 |
| `missing` | 2,928,076 / 82,409 | 4,222,208 / 79,541 |
| `a` | 8,373,677 / 112,434 | 12,959,332 / 169,313 |

Cache: **1,540,096 bytes**; after-GC retained heap delta: **1,542,640 bytes**.
Combined allocation including warmups, both lanes and bookkeeping:
**1,096,393,016 bytes / 18,539,257 allocations**. Higher reader allocated bytes
persist in every case. These measurements do not establish peak scratch or the
strict read-byte, output-byte and scratch budgets for arbitrary permitted
1 MiB stored strings. Those public-contract gates remain unsatisfied.

The following arrays preserve original pair order before sorting, in
milliseconds. Index is pair number 0 through 29; even pairs execute formula
first, odd pairs execute reader first. Logging occurs outside timed intervals.

```text
ancestor/014 formula:
6.0649 6.5121 6.4982 6.5115 6.5041 7.0153 7.0051 6.5087 6.9999 7.087 5.4999 6 8.6019 7.0368 7.5004 6.5102 6.5695 6.4993 6.5153 6.5037 6.5049 6.0588 8.008 6.0095 6.5159 5.999 7.581 7.1116 6.5037 5.9969
ancestor/014 reader:
5.5115 4.4992 5.5136 4.9984 5.006 5.5886 5.4998 5.5104 5.0083 4.9999 5.5156 6.5034 5.0033 4.5008 6.0713 5.0027 4.9999 5.0133 4.497 5.011 5.5139 5.5006 5.0005 5.0069 5.0009 6.0918 5.06 5.0005 5.0081 7.0122
missing formula:
4.9951 5.5126 6.0068 6.0083 6.5114 5.5003 6.5615 5.4975 7.01 6.0086 6.0012 6.0134 6.0064 6.0865 5.4999 6.0072 5.5003 6.2469 6.0241 8.499 6.5123 5.5667 6.4995 6.0756 5.5017 5.5115 5.5007 7.075 5.0002 6.5097
missing reader:
5.5077 5.0018 6.0054 4.5039 5.0006 5.0679 4.5009 5.0119 5.5096 4.5043 4.5646 4.4992 4.5065 5.5025 5.0136 4.4961 5.5097 4.5074 5.0008 4.5815 5.5123 4.5011 4.5073 4.5029 5.0094 5.0016 5.0214 5.0004 5.5102 5.004
a formula:
22.0763 25.6175 22.5462 22.0152 26.084 21.6386 23.1014 21.5837 23.8504 22.5306 25.1608 24.1262 24.0934 22.1479 23.6378 24.0803 24.6056 23.0271 21.6006 24.5888 23.0188 27.1477 22.5288 22.0407 22.5686 22.5807 22.0747 23.0872 24.5181 27.0829
a reader:
22.0599 21.0298 24.5217 22.6112 21.5169 22.5527 21.5164 21.1497 22.1 26.136 22.0227 23.1054 22.0478 22.569 24.0471 23.6393 22.3559 22.1542 22.1464 23.5947 23.5909 23.1223 23.0788 22.5312 21.5928 23.6145 25.039 24.0386 22.4344 22.5907
```

## Validation, reproduction and source review

**MEASURED final source:** focused ordinary tests plus the opt-in paired run
passed (2.709 s package time);
`go test -race -p 2 -count=1 -run '^TestSubstringReferenceExperiment$' ./benchmarks`
passed (4.311 s package time); `go vet -p 2 ./benchmarks` completed without
diagnostics.

**MEASURED full validation, 2026-10-02:** after explicit parent coordination,
`go test -p 2 ./...`, `go test -race -p 2 ./...`, and `go vet -p 2 ./...`
all completed successfully, sequentially, with `GOMAXPROCS=2`. Vet emitted no
diagnostics. Ordinary benchmark-package tests took 0.672 s; the race run took
4.710 s for that package. Several other package results were Go cache hits;
these commands did not force uncached execution. The opt-in timing experiment
was not enabled or rerun during these full checks. The parent held C++
compilation while these checks ran. A post-check SHA-256 confirmed the exact
experiment source hash above remained unchanged. No source fix was required.

From `engine/`, using the existing user-local Go and borrowed compiler:

```powershell
[string]$substringGoBin = [Environment]::GetEnvironmentVariable('FILE_MANAGER_GO_BIN', 'User')
$env:PATH = "$substringGoBin;C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin;" + $env:PATH
$env:GOTOOLCHAIN = 'local'
$env:GOMAXPROCS = '2'
$env:FILEMAN_SUBSTRING_REFERENCE_MEASURE = '1'
go test -p 2 -count=1 -run '^TestSubstringReference' -v ./benchmarks
$env:CGO_ENABLED = '1'
$env:CC = 'C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin\gcc.exe'
go test -race -p 2 -count=1 -run '^TestSubstringReferenceExperiment$' ./benchmarks
go vet -p 2 ./benchmarks
Remove-Item Env:FILEMAN_SUBSTRING_REFERENCE_MEASURE
go test -p 2 ./...
go test -race -p 2 ./...
go vet -p 2 ./...
```

**Source review against complete `planning/PROGRAMMING_HOUSE_STYLE.md`:** all
of the new benchmark file, including fixture formulas, page scan, independent
oracle, digest sink, cancellation wrapper, tests and measurement loop. Types
are explicit; named functions/methods own behavior; no closures or inferred
declarations were added. Parent review correctly rejected the first source's
implicit zero initialization; final locals and fixed arrays have explicit
initializers. Reader length and query invariants are hoisted outside the scan
loop. Scope validation checks components, allowing `notes..old` while rejecting
`.` and `..` components. Ineligible scan rows skip lowercasing. The source wrapper borrows a reader within test-owned
deferred close; its cancel function is owned by the test and invoked at a named
boundary. Ordinals narrow only after the reader count/position gate; page
publication occurs after successful bounded iteration, and cancellation returns
an empty page. Fixed page capacity prevents per-hit slice growth, but existing
reader/string/digest allocation remains disclosed. The independent formula
oracle intentionally generates per-row data; this is control work, not claimed
optimized production behavior. No remaining style violation was identified in
authored scope. Existing reader/writer/workload code was inspected for semantics
and limits, not certified or rewritten.

**CANDIDATE next decision:** accept the private reference's bounded row/result
and differential correctness evidence, while retaining strict decode/read-byte,
response-byte, scratch/time and cancellation-inside-I/O as blockers to the public
proposal. Reconcile that proposal and the typed consumer identity/provenance
gap before opening production work. This experiment selects no index.
