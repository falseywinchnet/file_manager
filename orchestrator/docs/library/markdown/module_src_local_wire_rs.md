# src/local_wire.rs

Status: **Stable orchestrator.local 1.0 framing projection**.

Encodes and decodes ORC1 magic, unsigned big-endian payload length, and nonempty UTF-8 JSON within the one-MiB hard ceiling.

Source: [src/local_wire.rs](../../../src/local_wire.rs)

## Responsibilities

- Reject bad magic, empty frames, oversized declarations, truncated payloads, invalid UTF-8, and invalid JSON.
- Preserve fragmented and coalesced frame boundaries.
- Serialize without exposing Rust layout.

## Boundary

- Does not authenticate a peer or interpret semantic methods.

## Contract projections

- orchestrator.local 1.0

## Source inventory

### [LOCAL_WIRE_MAGIC](../../../src/local_wire.rs#L7)

`const` · `pub`

```rust
pub const LOCAL_WIRE_MAGIC: [u8; 4] = *b"ORC1";
```

### [LOCAL_WIRE_HEADER_BYTES](../../../src/local_wire.rs#L8)

`const` · `pub`

```rust
pub const LOCAL_WIRE_HEADER_BYTES: usize = 8;
```

### [MAX_LOCAL_WIRE_FRAME_BYTES](../../../src/local_wire.rs#L9)

`const` · `pub`

```rust
pub const MAX_LOCAL_WIRE_FRAME_BYTES: usize = 1_048_576;
```

### [LocalWireError](../../../src/local_wire.rs#L12)

`enum` · `pub`

```rust
pub enum LocalWireError
```

### [fmt](../../../src/local_wire.rs#L24)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result
```

### [source](../../../src/local_wire.rs#L39)

`fn` · `private`

```rust
fn source(&self) -> Option<&(dyn Error + 'static)>
```

### [write_json_frame](../../../src/local_wire.rs#L54)

`fn` · `pub`

```rust
pub fn write_json_frame<T: Serialize, W: Write>( writer: &mut W, value: &T, ) -> Result<(), LocalWireError>
```

### [read_json_frame](../../../src/local_wire.rs#L79)

`fn` · `pub`

```rust
pub fn read_json_frame<T: DeserializeOwned, R: Read>(reader: &mut R) -> Result<T, LocalWireError>
```

### [read_header](../../../src/local_wire.rs#L100)

`fn` · `private`

```rust
fn read_header<R: Read>(reader: &mut R, header: &mut [u8]) -> Result<(), LocalWireError>
```

### [map_payload_read_error](../../../src/local_wire.rs#L114)

`fn` · `private`

```rust
fn map_payload_read_error(error: std::io::Error) -> LocalWireError
```

### [from](../../../src/local_wire.rs#L123)

`fn` · `private`

```rust
fn from(error: std::io::Error) -> Self
```

### [round_trip_and_coalesced_frames_preserve_boundaries](../../../src/local_wire.rs#L138)

`fn` · `private`

```rust
fn round_trip_and_coalesced_frames_preserve_boundaries()
```

### [oversized_header_is_rejected_before_payload_allocation](../../../src/local_wire.rs#L154)

`fn` · `private`

```rust
fn oversized_header_is_rejected_before_payload_allocation()
```

### [bad_magic_and_truncated_payload_fail_closed](../../../src/local_wire.rs#L164)

`fn` · `private`

```rust
fn bad_magic_and_truncated_payload_fail_closed()
```
