# src/service/stdio.rs

Status: **OBSERVED bounded conformance laboratory**.

Runs newline-delimited JSON requests through the same Kernel while draining oversized or invalid records without losing stream alignment.

Source: [src/service/stdio.rs](../../../src/service/stdio.rs)

## Responsibilities

- Enforce the one-MiB line ceiling before unbounded allocation.
- Return typed invalid/budget errors.
- Flush one response per request.
- Stop on the shared kernel shutdown lifecycle.

## Boundary

- JSONL stdio is fixture and development transport, not the installed local daemon wire.

## Contract projections

- ORC-COM-001 fixture projection
- ORC-CLI-001 structured service

## Source inventory

### [serve_stdio](../../../src/service/stdio.rs#L10)

`fn` · `pub(crate)`

```rust
pub(crate) fn serve_stdio() -> Result<(), String>
```

### [serve](../../../src/service/stdio.rs#L17)

`fn` · `private`

```rust
fn serve<R: BufRead, W: Write>(input: &mut R, output: &mut W) -> Result<(), String>
```

### [BoundedLine](../../../src/service/stdio.rs#L54)

`enum` · `private`

```rust
enum BoundedLine
```

### [read_bounded_line](../../../src/service/stdio.rs#L60)

`fn` · `private`

```rust
fn read_bounded_line<R: BufRead>(reader: &mut R, limit: usize) -> io::Result<BoundedLine>
```

### [bounded_line_drains_oversized_records_and_preserves_the_next_one](../../../src/service/stdio.rs#L117)

`fn` · `private`

```rust
fn bounded_line_drains_oversized_records_and_preserves_the_next_one()
```

### [stdio_host_and_kernel_share_shutdown_lifecycle](../../../src/service/stdio.rs#L136)

`fn` · `private`

```rust
fn stdio_host_and_kernel_share_shutdown_lifecycle()
```
