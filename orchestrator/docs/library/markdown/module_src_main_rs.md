# src/main.rs

Status: **OBSERVED thin process boundary**.

Maps the library CLI result to process exit status and a single stderr diagnostic prefix.

Source: [src/main.rs](../../../src/main.rs)

## Responsibilities

- Collect process arguments.
- Return success or failure exit status.
- Keep runtime and semantic policy out of the binary root.

## Boundary

- Owns no transport, provider, lifecycle, or contract semantics.

## Contract projections

- ORC-CLI-001 process projection

## Source inventory

### [main](../../../src/main.rs#L8)

`fn` · `private`

```rust
fn main() -> ExitCode
```
