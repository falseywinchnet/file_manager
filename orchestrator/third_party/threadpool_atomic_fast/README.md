# threadpool_atomic_fast

Status: **user-supplied C control, hardened local projection; available but not
selected as Orchestrator runtime architecture**.

This directory preserves the supplied synchronous atomic ticket-batch
algorithm as a standalone C11 library with a small C++20 RAII wrapper. The
checked API adds constructor/task validation and explicit failure results; the
source-compatible entry points remain available. The local optimized variant
uses bounded atomic range claims above a measured crossover and executes
smaller serialized batches directly on the caller.

The pool is suitable for homogeneous in-process compute batches whose callbacks
return normally. It is not a dynamic session queue, coroutine runtime, hostile
plugin sandbox, cancellation protocol, or authority boundary. Concurrent runs
serialize. Destruction still requires ordinary lifetime exclusion from new C
API calls; the C++ wrapper supplies that exclusion through object ownership.

The original files were supplied by the grand architect on 2026-08-10 from:

- `/Users/ultimussecundai/Downloads/threadpool_atomic_fast.h`
- `/Users/ultimussecundai/Downloads/threadpool_atomic_fast.c`

Redistribution/license provenance was not independently established. This local
integration does not assert third-party authorship, publication, or a universal
optimality result.

Build independently with:

```sh
cmake -S orchestrator/third_party/threadpool_atomic_fast \
  -B orchestrator/.build/threadpool-atomic-fast
cmake --build orchestrator/.build/threadpool-atomic-fast --parallel
ctest --test-dir orchestrator/.build/threadpool-atomic-fast --output-on-failure
```

The Rust sibling is `orchestrator/src/worker_pool.rs`; its proof obligations are
recorded in `orchestrator/formal/ATOMIC_BATCH_POOL.md`.
