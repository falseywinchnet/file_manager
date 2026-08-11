# src/contract.rs

Status: **OBSERVED executable registry projection**.

Projects contract identifiers, ownership, stage, and Core method-version requirements used by request validation and frontend snapshots.

Source: [src/contract.rs](../../../src/contract.rs)

## Responsibilities

- Keep executable contract identifiers unique.
- Map stable Core methods to accepted contract ranges.
- Expose stages without promoting code existence.

## Boundary

- The Markdown contract registry remains semantic authority.

## Contract projections

- ORC-NEG-001 process
- Core registry projection

## Source inventory

### [ContractStage](../../../src/contract.rs#L6)

`enum` · `pub`

```rust
pub enum ContractStage
```

### [ContractDescriptor](../../../src/contract.rs#L17)

`struct` · `pub`

```rust
pub struct ContractDescriptor
```

### [SupportedContract](../../../src/contract.rs#L26)

`struct` · `pub`

```rust
pub struct SupportedContract
```

### [supported_contract_for_method](../../../src/contract.rs#L33)

`fn` · `pub`

```rust
pub fn supported_contract_for_method(method: &str) -> Option<SupportedContract>
```

### [CONTRACTS](../../../src/contract.rs#L73)

`const` · `pub`

```rust
pub const CONTRACTS: &[ContractDescriptor] = &[ ContractDescriptor
```

### [identifiers_are_unique](../../../src/contract.rs#L252)

`fn` · `pub`

```rust
pub fn identifiers_are_unique() -> bool
```

### [contract_identifiers_are_unique](../../../src/contract.rs#L262)

`fn` · `private`

```rust
fn contract_identifiers_are_unique()
```
