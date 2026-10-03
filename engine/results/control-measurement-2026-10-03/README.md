# Exact-query measurement repair — 2026-10-03

**MEASURED:** the previous short aggregate comparisons contain material host/run
noise, and their individual Windows time.Now/Since samples are too coarsely
quantized to establish microsecond query tails. This receipt does not reverse
the bounded reader's rejection or establish its performance neutrality.
Production Reader, cache, exact query, service and transport source are unchanged.

## Same-binary control and archived comparison

`tools/measure_legacy_control_noise.py` verifies the exact archived baseline and
candidate binary SHA-256 values before executing them. Two warmups per binary,
then 12 same-binary pairs and 12 baseline/candidate pairs alternate both arm and
execution order. Every run uses ScaleV1(10000,10,10), GOMAXPROCS=2 and the old
identical harness: 20,000 path queries, 2,000 name-first-100 queries and 2,000
pipeline-first-100 queries. All expected counts, correctness digests and test
terminals must match before an observation is admitted. Raw logs and observations
are preserved, including values unfavorable to either interpretation.

| Aggregate control | Same binary median paired change | Same binary pair range | Candidate median paired change |
|---|---:|---:|---:|
| Exact path | +1.982% | -10.339% to +14.767% | +3.792% |
| Name first 100 | +2.471% | -13.689% to +11.966% | -5.717% |
| Pipeline first 100 | -1.249% | -18.432% to +11.349% | -2.202% |

These are paired changes in aggregate means, not confidence intervals or query
percentiles. The candidate's name/pipeline direction differs from the earlier
six-pair run; its path result remains slower as a point estimate. Neither a
regression cause nor no-regression is established. Do not admit the candidate
by subtracting the same-binary median from its median or relaxing the gate.

The verified binaries remain identified by the prior rejected experiment:

- Baseline: `5f945a6d5d2f5b7bff6ce263e4026cc3b77b275543af88785fd0ba1c3ae19b85`.
- Candidate: `e0f84ca82754797dce621dc840f3e6db2abbfe7e574b68f01ac405bb73148a2b`.

`legacy-manifest.json`, `legacy-observations.json`, `legacy-summary.json` and
`legacy-raw.zip` preserve all 52 invocations. The binaries' original sources
remain in `../rejected/bounded-scan-reader-2026-10-03/sources.zip`; the active
Engine does not contain that candidate. Source-level helper inlining alone did
not prove unchanged performance. The three inspected legacy symbols Path,
bindingAt and pathAt have equal respective symbol sizes in the two binaries
(512, 704 and 608 bytes); this is not instruction or runtime equivalence.

## Clock and allocation control

The new test-only control clock uses Windows QueryPerformanceCounter (QPC),
resolved once from the system DLL, with a serial owner and reusable initialized
native output slot. Pointers exist only at the synchronous native-call boundary;
the owner remains alive through the call. Tick subtraction precedes binary64
conversion. Other platforms use Go's monotonic time.Since path and still need
native calibration. No product timer, deadline, scheduler or clock changes.

QPC is the Windows interval API recommended for sub-microsecond measurements;
frequency is fixed for the system boot and obtained separately. See Microsoft's
[QPC contract](https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter)
and [interval guidance](https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps).

**MEASURED final Windows control:** frequency 10,000,000 ticks/s (100 ns ticks).
20,001 empty QPC intervals had 10,442 zero values, p95/p99 100 ns and maximum
5,300 ns. The counter/sample loop allocated zero bytes in zero allocations.
The equivalent Go time.Now/Since empty-interval control returned zero for all
20,001 samples. Empty intervals smaller than a tick may legitimately be zero;
none of the final actual query samples was zero. Counter-call time remains in
the query samples and is not subtracted as an invented correction.

The first QPC wrapper used LazyProc.Call, whose variadic argument path allocated
40,002 times / 320,016 bytes for 20,001 intervals even after moving the output
slot onto the clock owner. The final test-only boundary uses the standard
syscall.SyscallN directly with the pre-resolved address and synchronous pointer
conversion. This removes that measured allocation without changing the API
called. `initial-attempts.zip` retains the allocating wrapper, initial query
samples and a PowerShell argument-quoting failure. Final evidence is `precise-v2.zip`.

## Unchanged-reader baseline with the new clock

Three independent processes use the same 10k logical corpus, a checked published
generation, 128 warmup queries per operation, and preallocated sample arrays.
Each process preserves all 20,001 path and 2,001 name/pipeline durations before
sorting for nearest-rank percentiles. Every returned record is compared with the
reference outside its timed interval. Clock/error handling and normal allocator/
GC/scheduling effects remain; this is a warm component control, not physical
input latency, a cold query, one million records or a service/resource promotion.

| Operation | p50 across three processes | p95 range | p99 range | Observed maximum range |
|---|---:|---:|---:|---:|
| Exact path | 1.5 µs | 1.7–2.8 µs | 2.9–3.5 µs | 87.8–170.0 µs |
| Name first 100 | 35.2–35.6 µs | 60.9–82.8 µs | 183.6–224.8 µs | 332.2–487.0 µs |
| Pipeline first 100 | 45.7–46.1 µs | 80.9–105.1 µs | 296.8–319.5 µs | 434.5–744.2 µs |

No candidate was measured with this new clock yet. Do not compare these values
directly to old aggregate means, which include different instrumentation. The
next reader decision requires identical new harnesses in baseline/candidate,
adequate paired evidence plus same-binary controls, and the original correctness,
bounded-work and larger-corpus/native gates. Broader tails deserve attribution;
this receipt does not assume they are all GC or all storage.

Environment: Shadow Windows 11 10.0.22621, amd64, C: NTFS, reported AMD EPYC 9354,
four visible cores/eight logical processors, Go 1.27.1, GOMAXPROCS=2. No local
compiler was live during final sampling; coordinated sibling builds were clear.
Hypervisor/external host activity was not controlled. Raw samples contain the
Go/OS/architecture, corpus digest, frequency and acquisition order. CPU/RSS/I/O
traces were not collected in this clock repair; resource gates remain open.

Final measurement binary SHA-256:
`3d3a1ecbc7340c5080ecf5b2ccedebbd44a6fe1a1cad0142e5ce726615270c8d`.
Its source is the four new `benchmarks/*control*test.go` files in this checkpoint.
All three final corpus digests are
`50e692788dafb7f977eeaa8b7eb59838703c9cd1eb0f42a4268829acbdf4e9f8`.

| Preserved archive | SHA-256 |
|---|---|
| legacy-raw.zip | `96078570a169c41f2d7ca804238a2c910ffde78ef1f6d4df26e3310978eb3405` |
| precise-v2.zip | `a7c541a2cca23b87015804caeb14f669b268eadac2b60ceb4b236a082706dff2` |
| initial-attempts.zip | `1f69a2a7731354f6c42c8fb3227a2e09ec626d0823ac00e1af07c3db06fb8f6c` |

## Reproduction, validation and review

From Engine, using the selected Windows Go toolchain, first compile without
measurement. Then run the test binary with compiler activity paused:

```powershell
go test -c -p 1 -o .build/exact-control-clock.test.exe ./benchmarks
$env:GOMAXPROCS = '2'
$env:FILEMAN_ENGINE_CLOCK_MEASURE = '1'
./.build/exact-control-clock.test.exe '-test.run=^TestControlClock' '-test.v'
$env:FILEMAN_ENGINE_CONTROL_MEASURE = '1'
$env:FILEMAN_ENGINE_CONTROL_OUTPUT = 'C:\explicit\new-samples.json'
./.build/exact-control-clock.test.exe '-test.run=^TestExactControlClockDistribution$' '-test.v'
```

Use a fresh explicit absolute output filename for each process; an existing
file is refused. The old-noise tool similarly refuses an existing destination:

```text
python -B tools/measure_legacy_control_noise.py --baseline .build/bounded-scan-reader-2026-10-03/baseline.test.exe --candidate .build/bounded-scan-reader-2026-10-03/candidate.test.exe --output .build/fresh-control-noise --pairs 12
```

Final `go test -p 1 ./...`, `go test -race -p 1 ./...` and `go vet -p 1 ./...`
pass; logs are retained here. Unchanged package checks reused Go's cache and
are labeled cached in those logs. The benchmark test binary also cross-compiles
for Darwin arm64 and Linux amd64 with CGO disabled; this is compile evidence,
not native runtime or CGO-backed filesystem evidence.

Full house-style review covers all four new Go test files and the complete new
Python tool: explicit initialized types, named executable behavior, prepared
fixed sample storage, native borrow/owner lifetime, failure and output-file
preservation, integer-before-double conversion, no per-query format discovery,
and diagnostic allocation outside timed intervals. Existing referenceRecords,
sameRecords, workload/store/query implementations are dependencies, not newly
certified source. No vendored dependency was edited. The archived old reader and
its one-off tooling keep their previous explicit style limitations.
