# src/worker_pool.rs

Status: **OBSERVED available internal compute primitive**.

Provides a persistent fixed-worker synchronous batch executor with bounded atomic range claims and one formally documented unsafe type-erasure seam.

Source: [src/worker_pool.rs](../../../src/worker_pool.rs)

## Responsibilities

- Serialize borrowed batch publication.
- Claim every task exactly once with one acquire claim per participating worker and bounded relaxed range claims.
- Contain and count Rust task panics while draining remaining work.
- Join partial construction and destruction synchronously.

## Boundary

- Does not own service, provider, plugin, or filesystem authority.
- Does not replace blocking session workers or the owned Engine child worker.
- Makes no universal throughput-optimality claim.

## Contract projections

- Internal implementation primitive; no cross-project contract

## Source inventory

### [MAXIMUM_TASKS](../../../src/worker_pool.rs#L24)

`const` · `private`

```rust
const MAXIMUM_TASKS: usize = u32::MAX as usize;
```

### [CLAIMS_PER_WORKER_TARGET](../../../src/worker_pool.rs#L25)

`const` · `private`

```rust
const CLAIMS_PER_WORKER_TARGET: u32 = 8;
```

### [MAXIMUM_CLAIM_SIZE](../../../src/worker_pool.rs#L26)

`const` · `private`

```rust
const MAXIMUM_CLAIM_SIZE: u32 = 256;
```

### [MINIMUM_TASKS_PER_WORKER](../../../src/worker_pool.rs#L27)

`const` · `private`

```rust
const MINIMUM_TASKS_PER_WORKER: usize = 64;
```

### [BatchRunError](../../../src/worker_pool.rs#L30)

`enum` · `pub`

```rust
pub enum BatchRunError
```

### [fmt](../../../src/worker_pool.rs#L37)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result
```

### [SleepState](../../../src/worker_pool.rs#L52)

`struct` · `private`

```rust
struct SleepState
```

### [Core](../../../src/worker_pool.rs#L56)

`struct` · `private`

```rust
struct Core
```

### [ErasedBatch](../../../src/worker_pool.rs#L67)

`struct` · `private`

```rust
struct ErasedBatch
```

### [AtomicBatchPool](../../../src/worker_pool.rs#L80)

`struct` · `pub`

```rust
pub struct AtomicBatchPool
```

### [fmt](../../../src/worker_pool.rs#L86)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result
```

### [new](../../../src/worker_pool.rs#L101)

`fn` · `pub`

```rust
pub fn new(thread_count: usize) -> io::Result<Self>
```

### [thread_count](../../../src/worker_pool.rs#L142)

`fn` · `pub`

```rust
pub fn thread_count(&self) -> usize
```

### [for_each](../../../src/worker_pool.rs#L157)

`fn` · `pub`

```rust
pub fn for_each<T, F>(&self, tasks: &mut [T], function: F) -> Result<(), BatchRunError> where T: Send, F: Fn(&mut T) + Sync,
```

### [drop](../../../src/worker_pool.rs#L224)

`fn` · `private`

```rust
fn drop(&mut self)
```

### [valid_ticket](../../../src/worker_pool.rs#L234)

`fn` · `private`

```rust
fn valid_ticket(ticket: u64) -> bool
```

### [claim_size](../../../src/worker_pool.rs#L238)

`fn` · `private`

```rust
fn claim_size(task_count: u32, worker_count: usize) -> u32
```

### [worker_loop](../../../src/worker_pool.rs#L248)

`fn` · `private`

```rust
fn worker_loop(core: &Core)
```

### [call_one](../../../src/worker_pool.rs#L324)

`fn` · `private`

```rust
unsafe fn call_one<T, F>(tasks: *mut (), function: *const (), index: usize, panics: &AtomicU32) where T: Send, F: Fn(&mut T) + Sync,
```

### [request_termination](../../../src/worker_pool.rs#L341)

`fn` · `private`

```rust
fn request_termination(core: &Core)
```

### [lock_unpoisoned](../../../src/worker_pool.rs#L348)

`fn` · `private`

```rust
fn lock_unpoisoned<T>(mutex: &Mutex<T>) -> MutexGuard<'_, T>
```

### [wait_unpoisoned](../../../src/worker_pool.rs#L354)

`fn` · `private`

```rust
fn wait_unpoisoned<'a, T>(condition: &Condvar, guard: MutexGuard<'a, T>) -> MutexGuard<'a, T>
```

### [ModelState](../../../src/worker_pool.rs#L369)

`struct` · `private`

```rust
struct ModelState
```

### [bounded_model_checks_range_claims_and_completion_orders](../../../src/worker_pool.rs#L377)

`fn` · `private`

```rust
fn bounded_model_checks_range_claims_and_completion_orders()
```

### [TASKS](../../../src/worker_pool.rs#L378)

`const` · `private`

```rust
const TASKS: u8 = 6;
```

### [WORKERS](../../../src/worker_pool.rs#L379)

`const` · `private`

```rust
const WORKERS: u8 = 3;
```

### [MAX_CLAIM](../../../src/worker_pool.rs#L380)

`const` · `private`

```rust
const MAX_CLAIM: u8 = 3;
```

### [every_ticket_is_claimed_once_and_writes_are_visible](../../../src/worker_pool.rs#L446)

`fn` · `private`

```rust
fn every_ticket_is_claimed_once_and_writes_are_visible()
```

### [concurrent_callers_are_serialized_without_losing_work](../../../src/worker_pool.rs#L455)

`fn` · `private`

```rust
fn concurrent_callers_are_serialized_without_losing_work()
```

### [task_panics_are_counted_and_pool_remains_reusable](../../../src/worker_pool.rs#L485)

`fn` · `private`

```rust
fn task_panics_are_counted_and_pool_remains_reusable()
```

### [caller_lane_panics_are_counted_and_pool_remains_reusable](../../../src/worker_pool.rs#L505)

`fn` · `private`

```rust
fn caller_lane_panics_are_counted_and_pool_remains_reusable()
```

### [empty_is_a_noop_and_zero_sized_tasks_are_rejected](../../../src/worker_pool.rs#L519)

`fn` · `private`

```rust
fn empty_is_a_noop_and_zero_sized_tasks_are_rejected()
```

### [repeated_parallel_batches_cross_ticket_underflow_and_republication](../../../src/worker_pool.rs#L532)

`fn` · `private`

```rust
fn repeated_parallel_batches_cross_ticket_underflow_and_republication()
```
