# M4 atomic batch worker evidence — 2026-08-10

Status: **MEASURED correctness and workload-specific performance; universal
optimality deliberately unclaimed**.

## Object and controls

- **GIVEN:** the grand architect supplied `threadpool_atomic_fast.c/.h` as the
  single-ticket atomic worker candidate.
- **OBSERVED:** the supplied core publishes a synchronous batch with a release
  store, gives each participating worker an acquire first claim, uses relaxed
  later claims, and completes with release decrements observed by an acquire
  waiter.
- **OBSERVED:** the supplied files did not check partial pthread construction,
  callback pointers, or callback failure, and required external exclusion from
  destruction.
- **DECIDED locally for this available primitive:** retain that publication and
  completion protocol, use a 64-bit ticket counter, claim bounded ranges after
  the first ticket, and bypass worker wakeup for one-worker or small batches.
- **REJECTED:** using this synchronous compute primitive for blocking local
  sessions or Engine child I/O. Those workers require owned queues, timeouts,
  and process teardown rather than borrowed batch completion.

The benchmark applies the same 32-round integer-mixing function to each item.
It measures batches of 1, 32, 256, 4,096, and 65,536 items. There are 200
samples through 4,096 and 40 samples at 65,536; reported values below are p50.
Task allocation and pool construction are outside the timed interval. Serial
loops are the per-language controls.

Environment:

- Apple M4, 10 cores (4 performance, 6 efficiency), arm64
- macOS 26.5 (25F71)
- rustc 1.87.0 / LLVM 20.1.1, optimized release profile
- Apple clang 21.0.0, C11/C++20 Release
- no affinity or frequency pinning; figures are workload observations, not
  deterministic platform guarantees

Reproduction:

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh orchestrator/tools/benchmark_worker_pool_m4.sh
```

## Optimization result

The original single-ticket 10-worker form regressed against serial on both
large workloads because every item performed a contended RMW. The optimized
form gives every participating worker one acquire ticket, then uses ranges
`ceil(items / (8 * workers))` clamped to `1..=256`. Batches at or below
`64 * workers`, and every one-worker batch, use the serialized caller lane.

| implementation | items | original one-ticket p50 | optimized p50 | optimized / original |
|---|---:|---:|---:|---:|
| Rust, 10 workers | 4,096 | 253,875 ns | 69,875 ns | 3.63× faster |
| Rust, 10 workers | 65,536 | 3,545,958 ns | 598,083 ns | 5.93× faster |
| C, 10 workers | 4,096 | 262,584 ns | 76,333 ns | 3.44× faster |
| C, 10 workers | 65,536 | 3,563,250 ns | 623,958 ns | 5.71× faster |

Final optimized comparison:

| implementation | workers | items | pool p50 | serial p50 | relative result |
|---|---:|---:|---:|---:|---:|
| Rust | 10 | 256 | 12,416 ns | 12,416 ns | 1.00× |
| Rust | 10 | 4,096 | 69,875 ns | 197,875 ns | 2.83× faster |
| Rust | 10 | 65,536 | 598,083 ns | 3,169,542 ns | 5.30× faster |
| C | 10 | 256 | 19,584 ns | 19,042 ns | 1.03× slower |
| C | 10 | 4,096 | 76,333 ns | 200,875 ns | 2.63× faster |
| C | 10 | 65,536 | 623,958 ns | 3,168,708 ns | 5.08× faster |
| Rust | 4 | 65,536 | 934,042 ns | 3,164,875 ns | 3.39× faster |
| C | 4 | 65,536 | 950,375 ns | 3,165,625 ns | 3.33× faster |

**MEASURED:** ten workers were the best of the measured 1, 4, and 10-worker
counts for the two parallel-sized uniform workloads. The crossover makes the
small measured workloads equivalent to their serial controls within noise.
This does not select a default pool size for heterogeneous production work.

## Correctness and proof status

- **MEASURED:** Rust unit tests cover exact-once mutation, acquire-visible
  writes, concurrent caller serialization, caller-lane and worker-lane panic
  accounting, reuse, and 2,000 parallel underflow/republish cycles.
- **MEASURED:** the independently built C/C++ test covers checked construction,
  invalid tasks, concurrent callers, exact-once completion, and 2,000 parallel
  underflow/republish cycles.
- **MEASURED:** the Rust bounded state explorer enumerates all range-claim and
  completion orders for six tickets, three workers, and claims of one through
  three while checking ownership, coverage, and pending accounting.
- **OBSERVED:** `formal/ATOMIC_BATCH_POOL.md` states the unsafe Rust obligations
  and memory-order proof. `formal/atomic_batch_pool.tla` plus its TLC config
  records the corresponding range-claim state machine.
- **UNVERIFIED:** TLC was not installed on the M4, so the checked-in TLA+ model
  was reviewed but not model-checked in this campaign. The Rust bounded model
  is the executable model evidence.

The result is a tight, reusable, correctness-argued primitive and the best
measured form among the declared candidates and workloads. A universal or
mathematical throughput optimum is neither meaningful nor proven: callback
cost distribution, topology, contention, and power state can move it.
