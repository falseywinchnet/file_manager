# src/engine_contract.rs

Status: **Frozen experimental Engine semantic v0 and live-query draft projection**.

Defines typed Engine capabilities, currentness, search request/budget/cursor, catalogue results, live results, and work counters used by fixtures and the provider port.

Source: [src/engine_contract.rs](../../../src/engine_contract.rs)

## Responsibilities

- Keep catalogue and live source identities distinct.
- Enforce numeric live-search hard ceilings.
- Validate source-bound cursors and request shape.
- Represent partial and unavailable work explicitly.

## Boundary

- Does not implement traversal, catalogue storage, or routing policy.

## Contract projections

- ORC-ENG-001
- ORC-ENG-003
- ORC-ENG-004

## Source inventory

### [ENGINE_SEMANTIC_MAJOR](../../../src/engine_contract.rs#L5)

`const` · `pub`

```rust
pub const ENGINE_SEMANTIC_MAJOR: u16 = 0;
```

### [ENGINE_SEMANTIC_MINOR](../../../src/engine_contract.rs#L6)

`const` · `pub`

```rust
pub const ENGINE_SEMANTIC_MINOR: u16 = 1;
```

### [ENGINE_LIVE_SEMANTIC_MAJOR](../../../src/engine_contract.rs#L7)

`const` · `pub`

```rust
pub const ENGINE_LIVE_SEMANTIC_MAJOR: u16 = 0;
```

### [ENGINE_LIVE_SEMANTIC_MINOR](../../../src/engine_contract.rs#L8)

`const` · `pub`

```rust
pub const ENGINE_LIVE_SEMANTIC_MINOR: u16 = 1;
```

### [LIVE_QUERY_DEFAULT_RESULTS](../../../src/engine_contract.rs#L10)

`const` · `pub`

```rust
pub const LIVE_QUERY_DEFAULT_RESULTS: u32 = 128;
```

### [LIVE_QUERY_MAX_RESULTS](../../../src/engine_contract.rs#L11)

`const` · `pub`

```rust
pub const LIVE_QUERY_MAX_RESULTS: u32 = 1_000;
```

### [LIVE_QUERY_DEFAULT_VISITED_ENTRIES](../../../src/engine_contract.rs#L12)

`const` · `pub`

```rust
pub const LIVE_QUERY_DEFAULT_VISITED_ENTRIES: u64 = 100_000;
```

### [LIVE_QUERY_MAX_VISITED_ENTRIES](../../../src/engine_contract.rs#L13)

`const` · `pub`

```rust
pub const LIVE_QUERY_MAX_VISITED_ENTRIES: u64 = 1_000_000;
```

### [LIVE_QUERY_DEFAULT_STAT_CALLS](../../../src/engine_contract.rs#L14)

`const` · `pub`

```rust
pub const LIVE_QUERY_DEFAULT_STAT_CALLS: u64 = 4_096;
```

### [LIVE_QUERY_MAX_STAT_CALLS](../../../src/engine_contract.rs#L15)

`const` · `pub`

```rust
pub const LIVE_QUERY_MAX_STAT_CALLS: u64 = 65_536;
```

### [LIVE_QUERY_DEFAULT_WALL_TIME_MS](../../../src/engine_contract.rs#L16)

`const` · `pub`

```rust
pub const LIVE_QUERY_DEFAULT_WALL_TIME_MS: u64 = 250;
```

### [LIVE_QUERY_MAX_WALL_TIME_MS](../../../src/engine_contract.rs#L17)

`const` · `pub`

```rust
pub const LIVE_QUERY_MAX_WALL_TIME_MS: u64 = 5_000;
```

### [LIVE_QUERY_DEFAULT_OPEN_DIRECTORIES](../../../src/engine_contract.rs#L18)

`const` · `pub`

```rust
pub const LIVE_QUERY_DEFAULT_OPEN_DIRECTORIES: u16 = 8;
```

### [LIVE_QUERY_MAX_OPEN_DIRECTORIES](../../../src/engine_contract.rs#L19)

`const` · `pub`

```rust
pub const LIVE_QUERY_MAX_OPEN_DIRECTORIES: u16 = 32;
```

### [EngineFlag](../../../src/engine_contract.rs#L23)

`struct` · `pub`

```rust
pub struct EngineFlag(bool);
```

### [new](../../../src/engine_contract.rs#L27)

`fn` · `pub`

```rust
pub const fn new(value: bool) -> Self
```

### [is_set](../../../src/engine_contract.rs#L32)

`fn` · `pub`

```rust
pub const fn is_set(self) -> bool
```

### [from](../../../src/engine_contract.rs#L38)

`fn` · `private`

```rust
fn from(value: bool) -> Self
```

### [EngineCapabilityState](../../../src/engine_contract.rs#L45)

`enum` · `pub`

```rust
pub enum EngineCapabilityState
```

### [EngineCapability](../../../src/engine_contract.rs#L54)

`struct` · `pub`

```rust
pub struct EngineCapability
```

### [is_well_formed](../../../src/engine_contract.rs#L65)

`fn` · `pub`

```rust
pub fn is_well_formed(&self) -> bool
```

### [EngineCurrentness](../../../src/engine_contract.rs#L76)

`enum` · `pub`

```rust
pub enum EngineCurrentness
```

### [EngineWorkSnapshot](../../../src/engine_contract.rs#L87)

`struct` · `pub`

```rust
pub struct EngineWorkSnapshot
```

### [EngineStatusSnapshot](../../../src/engine_contract.rs#L99)

`struct` · `pub`

```rust
pub struct EngineStatusSnapshot
```

### [is_well_formed](../../../src/engine_contract.rs#L110)

`fn` · `pub`

```rust
pub fn is_well_formed(&self) -> bool
```

### [capability](../../../src/engine_contract.rs#L125)

`fn` · `pub`

```rust
pub fn capability(&self, id: &str) -> Option<&EngineCapability>
```

### [EngineQueryResultFixture](../../../src/engine_contract.rs#L133)

`struct` · `pub`

```rust
pub struct EngineQueryResultFixture
```

### [is_well_formed](../../../src/engine_contract.rs#L156)

`fn` · `pub`

```rust
pub fn is_well_formed(&self) -> bool
```

### [EngineSearchBudget](../../../src/engine_contract.rs#L173)

`struct` · `pub`

```rust
pub struct EngineSearchBudget
```

### [default](../../../src/engine_contract.rs#L183)

`fn` · `private`

```rust
fn default() -> Self
```

### [is_well_formed](../../../src/engine_contract.rs#L197)

`fn` · `pub`

```rust
pub const fn is_well_formed(&self) -> bool
```

### [EngineSearchRequest](../../../src/engine_contract.rs#L214)

`struct` · `pub`

```rust
pub struct EngineSearchRequest
```

### [EngineSearchCursorSource](../../../src/engine_contract.rs#L228)

`enum` · `pub`

```rust
pub enum EngineSearchCursorSource
```

### [EngineSearchCursor](../../../src/engine_contract.rs#L234)

`struct` · `pub`

```rust
pub struct EngineSearchCursor
```

### [is_well_formed](../../../src/engine_contract.rs#L241)

`fn` · `pub`

```rust
pub fn is_well_formed(&self) -> bool
```

### [EngineResultSource](../../../src/engine_contract.rs#L258)

`enum` · `pub`

```rust
pub enum EngineResultSource
```

### [EngineLiveWork](../../../src/engine_contract.rs#L263)

`struct` · `pub`

```rust
pub struct EngineLiveWork
```

### [EngineLiveQueryResultFixture](../../../src/engine_contract.rs#L270)

`struct` · `pub`

```rust
pub struct EngineLiveQueryResultFixture
```

### [is_well_formed](../../../src/engine_contract.rs#L292)

`fn` · `pub`

```rust
pub fn is_well_formed(&self) -> bool
```
