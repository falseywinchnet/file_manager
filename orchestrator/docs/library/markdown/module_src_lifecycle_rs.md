# src/lifecycle.rs

Status: **OBSERVED deterministic lifecycle state machine**.

Represents initialization, ready, draining, stopped, and failed transitions with a monotonically changing generation.

Source: [src/lifecycle.rs](../../../src/lifecycle.rs)

## Responsibilities

- Reject invalid state transitions.
- Publish immutable lifecycle snapshots.
- Support restart as a new process or explicit lifecycle generation.

## Boundary

- Does not supervise processes or activate platform services.

## Contract projections

- ORC-LIF-001

## Source inventory

### [LifecycleState](../../../src/lifecycle.rs#L6)

`enum` · `pub`

```rust
pub enum LifecycleState
```

### [LifecycleError](../../../src/lifecycle.rs#L16)

`struct` · `pub`

```rust
pub struct LifecycleError
```

### [fmt](../../../src/lifecycle.rs#L22)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result
```

### [LifecycleSnapshot](../../../src/lifecycle.rs#L34)

`struct` · `pub`

```rust
pub struct LifecycleSnapshot
```

### [Lifecycle](../../../src/lifecycle.rs#L40)

`struct` · `pub`

```rust
pub struct Lifecycle
```

### [default](../../../src/lifecycle.rs#L46)

`fn` · `private`

```rust
fn default() -> Self
```

### [started](../../../src/lifecycle.rs#L57)

`fn` · `pub`

```rust
pub const fn started() -> Self
```

### [snapshot](../../../src/lifecycle.rs#L65)

`fn` · `pub`

```rust
pub fn snapshot(&self) -> LifecycleSnapshot
```

### [start](../../../src/lifecycle.rs#L77)

`fn` · `pub`

```rust
pub fn start(&mut self) -> Result<(), LifecycleError>
```

### [begin_shutdown](../../../src/lifecycle.rs#L88)

`fn` · `pub`

```rust
pub fn begin_shutdown(&mut self) -> Result<(), LifecycleError>
```

### [finish_shutdown](../../../src/lifecycle.rs#L97)

`fn` · `pub`

```rust
pub fn finish_shutdown(&mut self) -> Result<(), LifecycleError>
```

### [fault](../../../src/lifecycle.rs#L106)

`fn` · `pub`

```rust
pub fn fault(&mut self) -> Result<(), LifecycleError>
```

### [transition](../../../src/lifecycle.rs#L110)

`fn` · `private`

```rust
fn transition(&mut self, next: LifecycleState) -> Result<(), LifecycleError>
```

### [lifecycle_is_deterministic_and_restartable](../../../src/lifecycle.rs#L143)

`fn` · `private`

```rust
fn lifecycle_is_deterministic_and_restartable()
```

### [invalid_transition_is_rejected](../../../src/lifecycle.rs#L155)

`fn` · `private`

```rust
fn invalid_transition_is_rejected()
```
