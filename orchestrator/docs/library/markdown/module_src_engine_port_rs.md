# src/engine_port.rs

Status: **OBSERVED provider-neutral broker**.

Defines the search provider trait and narrow catalogue-to-live fallback allowlist consumed by Kernel.

Source: [src/engine_port.rs](../../../src/engine_port.rs)

## Responsibilities

- Prefer catalogue unless policy or cursor selects live.
- Keep continuation on its original lane.
- Never fall back around denied, invalid, budget, timeout, or cancellation outcomes.
- Treat authoritative catalogue no-match as terminal.

## Boundary

- Does not define provider process transport or frontend presentation.

## Contract projections

- ORC-ENG-001
- ORC-ENG-004
- ORC-FE-001 unified search

## Source inventory

### [EngineSearchPolicy](../../../src/engine_port.rs#L8)

`enum` · `pub`

```rust
pub enum EngineSearchPolicy
```

### [EngineSearchOutcome](../../../src/engine_port.rs#L15)

`enum` · `pub`

```rust
pub enum EngineSearchOutcome
```

### [terminal](../../../src/engine_port.rs#L22)

`fn` · `pub`

```rust
pub const fn terminal(&self) -> TerminalStatus
```

### [EngineSearchProvider](../../../src/engine_port.rs#L30)

`trait` · `pub`

```rust
pub trait EngineSearchProvider
```

### [query_catalogue](../../../src/engine_port.rs#L31)

`fn` · `private`

```rust
fn query_catalogue(&mut self, request: &EngineSearchRequest) -> EngineQueryResultFixture;
```

### [query_live](../../../src/engine_port.rs#L32)

`fn` · `private`

```rust
fn query_live(&mut self, request: &EngineSearchRequest) -> EngineLiveQueryResultFixture;
```

### [live_available](../../../src/engine_port.rs#L34)

`fn` · `private`

```rust
fn live_available(&self) -> bool
```

### [UnifiedEngineSearch](../../../src/engine_port.rs#L39)

`trait` · `pub`

```rust
pub trait UnifiedEngineSearch: Send
```

### [search](../../../src/engine_port.rs#L40)

`fn` · `private`

```rust
fn search(&mut self, request: &EngineSearchRequest) -> EngineSearchOutcome;
```

### [live_available](../../../src/engine_port.rs#L41)

`fn` · `private`

```rust
fn live_available(&self) -> bool;
```

### [EngineSearchBroker](../../../src/engine_port.rs#L45)

`struct` · `pub`

```rust
pub struct EngineSearchBroker<P>
```

### [new](../../../src/engine_port.rs#L51)

`fn` · `pub`

```rust
pub const fn new(provider: P) -> Self
```

### [search](../../../src/engine_port.rs#L55)

`fn` · `pub`

```rust
pub fn search( &mut self, request: &EngineSearchRequest, policy: EngineSearchPolicy, ) -> EngineSearchOutcome
```

### [into_provider](../../../src/engine_port.rs#L90)

`fn` · `pub`

```rust
pub fn into_provider(self) -> P
```

### [provider_mut](../../../src/engine_port.rs#L94)

`fn` · `pub`

```rust
pub const fn provider_mut(&mut self) -> &mut P
```

### [search](../../../src/engine_port.rs#L100)

`fn` · `private`

```rust
fn search(&mut self, request: &EngineSearchRequest) -> EngineSearchOutcome
```

### [live_available](../../../src/engine_port.rs#L104)

`fn` · `private`

```rust
fn live_available(&self) -> bool
```

### [may_fall_back_to_live](../../../src/engine_port.rs#L109)

`fn` · `private`

```rust
const fn may_fall_back_to_live(terminal: TerminalStatus) -> bool
```

### [fallback_terminal_allowlist_is_narrow_and_authority_safe](../../../src/engine_port.rs#L125)

`fn` · `private`

```rust
fn fallback_terminal_allowlist_is_narrow_and_authority_safe()
```
