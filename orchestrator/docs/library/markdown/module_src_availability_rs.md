# src/availability.rs

Status: **OBSERVED provider-honest availability projection**.

Defines required capabilities and their available, degraded, negotiating, unavailable, deferred, or stubbed state.

Source: [src/availability.rs](../../../src/availability.rs)

## Responsibilities

- Separate requirement from runtime availability.
- Name provider and reason for every capability.
- Keep plugins and semantic facts explicit stubs.
- Avoid promoting a development Engine adapter to installed transport.

## Boundary

- Dynamic live-search availability is overlaid by Kernel only when a connected provider advertises the contract.

## Contract projections

- ORC-FE-001 availability snapshot

## Source inventory

### [AvailabilityState](../../../src/availability.rs#L5)

`enum` · `pub`

```rust
pub enum AvailabilityState
```

### [CapabilityAvailability](../../../src/availability.rs#L15)

`struct` · `pub`

```rust
pub struct CapabilityAvailability
```

### [CAPABILITIES](../../../src/availability.rs#L23)

`const` · `pub`

```rust
pub const CAPABILITIES: &[CapabilityAvailability] = &[ CapabilityAvailability
```

### [state_count](../../../src/availability.rs#L216)

`fn` · `pub`

```rust
pub fn state_count(state: AvailabilityState) -> usize
```

### [facts_and_plugins_are_explicit_stubs](../../../src/availability.rs#L228)

`fn` · `private`

```rust
fn facts_and_plugins_are_explicit_stubs()
```

### [development_engine_adapter_does_not_claim_installed_transport](../../../src/availability.rs#L240)

`fn` · `private`

```rust
fn development_engine_adapter_does_not_claim_installed_transport()
```
