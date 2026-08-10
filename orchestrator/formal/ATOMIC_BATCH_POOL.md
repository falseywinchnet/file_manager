# Atomic batch pool safety argument

Status: **REASONED implementation proof plus executable bounded model; not a
universal optimality proof**.

The Rust pool and the vendored C control implement the same batch protocol. A
batch has `N` valid tickets, numbered `N..1`. Each worker first claims one
ticket with an acquire `remaining.fetch_sub(1)`. It then claims bounded,
contiguous ranges with relaxed `fetch_sub(K)` operations. The returned value
is the inclusive high ticket and `min(value, K)` is the number of valid
tickets in that range. Zero and the unsigned underflow region are invalid.

`K = ceil(N / (8 * workers))`, clamped to `1..=256`. The initial one-ticket
claim preserves publication acquisition per worker and ensures that an awake
worker can participate before another worker claims a range. The cap bounds
load imbalance while reducing per-item atomic contention on larger batches.

Batches with one configured worker or at most `64 * workers` tasks execute
directly on the serialized caller. That measured crossover policy avoids
paying wake/coordination cost when the M4 control workload has too little
parallel work. It uses ordinary language sequencing and does not enter the
unsafe erased-borrow protocol; task panic accounting has the same Rust result
on both lanes.

## Invariants

1. `run` serialization admits at most one published batch.
2. The release store `remaining=N` publishes the task/function pointers and
   `pending=N`.
3. Each participating worker's first acquire RMW observes that publication.
   Its later relaxed RMWs remain in the same batch and need no new publication.
4. Atomic modification order makes every claimed interval disjoint. Truncating
   the final interval at ticket one covers `N..1` exactly once, so distinct
   workers never form aliased mutable references to one task.
5. A worker performs one release decrement of `pending` after all tasks it
   claimed. The caller's acquire observation of zero synchronizes with every
   release RMW whose release sequence includes the zero-producing RMW.
6. The caller clears the erased pointer and lets borrowed storage expire only
   after invariant 5. An overshooting worker never dereferences the batch.
7. The sleep mutex protects the termination predicate and prevents a missed
   completion notification between the caller's predicate check and wait.

These invariants justify the two raw-pointer casts in `call_one`. Safe Rust
ownership prevents destruction while a borrowed `for_each` call is live.

## Panic and failure boundary

Rust task panics are caught per invocation, counted with saturation, and do not
skip completion accounting. A panic therefore cannot strand the synchronous
caller or retire borrowed storage early. The C control retains its explicit
requirement that callbacks return normally because C has no portable unwind
containment contract.

## Claims deliberately not made

- Lock-free progress: sleep, run serialization, and completion use mutexes and
  condition variables.
- Fair task distribution: tickets are intentionally greedy.
- Universal throughput optimality: topology, task duration, batch size, power
  state, and competing workloads change the optimum.
- Suitability for blocking session I/O or hostile plugin containment: this is a
  synchronous compute-batch primitive, not a dynamic job supervisor.

The executable tests exhaust repeated underflow/republish transitions,
concurrent callers, panic containment, exactly-once mutation, and destruction.
M4 benchmark evidence must name the measured workloads and controls before a
performance claim is promoted.
