# src/runtime_health.rs

Status: **OBSERVED relaxed non-authoritative telemetry**.

Tracks saturating local-session pressure and request counters without participating in authority, routing, or permissions.

Source: [src/runtime_health.rs](../../../src/runtime_health.rs)

## Responsibilities

- Publish worker and queue ceilings.
- Count accepted, rejected, active, authenticated, failed, malformed, and completed observations.
- Saturate counters and prevent active-session underflow.

## Boundary

- Consistency is relaxed observability; values never authorize behavior.

## Contract projections

- Optional ORC-LIF-001 and ORC-FE-001 diagnostic fields

## Source inventory

### [LOCAL_SESSION_WORKERS](../../../src/runtime_health.rs#L4)

`const` · `pub`

```rust
pub const LOCAL_SESSION_WORKERS: usize = 4;
```

### [LOCAL_PENDING_SESSIONS](../../../src/runtime_health.rs#L5)

`const` · `pub`

```rust
pub const LOCAL_PENDING_SESSIONS: usize = 8;
```

### [RuntimeHealthSnapshot](../../../src/runtime_health.rs#L8)

`struct` · `pub`

```rust
pub struct RuntimeHealthSnapshot
```

### [RuntimeHealth](../../../src/runtime_health.rs#L24)

`struct` · `pub`

```rust
pub struct RuntimeHealth
```

### [in_process](../../../src/runtime_health.rs#L37)

`fn` · `pub`

```rust
pub fn in_process() -> Self
```

### [local_daemon](../../../src/runtime_health.rs#L42)

`fn` · `pub`

```rust
pub fn local_daemon() -> Self
```

### [new](../../../src/runtime_health.rs#L46)

`fn` · `private`

```rust
fn new(kind: &'static str) -> Self
```

### [record_accepted_session](../../../src/runtime_health.rs#L59)

`fn` · `pub`

```rust
pub fn record_accepted_session(&self)
```

### [record_rejected_session](../../../src/runtime_health.rs#L63)

`fn` · `pub`

```rust
pub fn record_rejected_session(&self)
```

### [record_session_opened](../../../src/runtime_health.rs#L67)

`fn` · `pub`

```rust
pub fn record_session_opened(&self)
```

### [record_session_closed](../../../src/runtime_health.rs#L71)

`fn` · `pub`

```rust
pub fn record_session_closed(&self)
```

### [record_authenticated_session](../../../src/runtime_health.rs#L79)

`fn` · `pub`

```rust
pub fn record_authenticated_session(&self)
```

### [record_authentication_failure](../../../src/runtime_health.rs#L83)

`fn` · `pub`

```rust
pub fn record_authentication_failure(&self)
```

### [record_completed_request](../../../src/runtime_health.rs#L87)

`fn` · `pub`

```rust
pub fn record_completed_request(&self)
```

### [record_malformed_session](../../../src/runtime_health.rs#L91)

`fn` · `pub`

```rust
pub fn record_malformed_session(&self)
```

### [snapshot](../../../src/runtime_health.rs#L96)

`fn` · `pub`

```rust
pub fn snapshot(&self) -> RuntimeHealthSnapshot
```

### [saturating_increment](../../../src/runtime_health.rs#L114)

`fn` · `private`

```rust
fn saturating_increment(counter: &AtomicU64)
```

### [counters_never_underflow_and_snapshot_is_explicitly_non_authoritative](../../../src/runtime_health.rs#L125)

`fn` · `private`

```rust
fn counters_never_underflow_and_snapshot_is_explicitly_non_authoritative()
```
