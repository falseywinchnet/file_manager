# src/common.rs

Status: **Stable Core 1.0 projection**.

Defines common request, response, terminal status, error, version, and envelope bounds shared by every Core host.

Source: [src/common.rs](../../../src/common.rs)

## Responsibilities

- Represent exactly one terminal success result or typed error.
- Carry deadlines, cancellation identity, critical extensions, contract reference, and response budget.
- Publish structured CLI protocol identity and fixed envelope ceilings.

## Boundary

- Does not choose transport framing or provider behavior.

## Contract projections

- ORC-COM-001
- ORC-CLI-001

## Source inventory

### [PROTOCOL_FAMILY](../../../src/common.rs#L4)

`const` · `pub`

```rust
pub const PROTOCOL_FAMILY: &str = "orchestrator.cli.jsonl";
```

### [PROTOCOL_MAJOR](../../../src/common.rs#L5)

`const` · `pub`

```rust
pub const PROTOCOL_MAJOR: u16 = 1;
```

### [PROTOCOL_MINOR](../../../src/common.rs#L6)

`const` · `pub`

```rust
pub const PROTOCOL_MINOR: u16 = 0;
```

### [MAX_FRAME_BYTES](../../../src/common.rs#L7)

`const` · `pub`

```rust
pub const MAX_FRAME_BYTES: usize = 1_048_576;
```

### [MAX_REQUEST_ID_BYTES](../../../src/common.rs#L8)

`const` · `pub`

```rust
pub const MAX_REQUEST_ID_BYTES: usize = 128;
```

### [MAX_METHOD_BYTES](../../../src/common.rs#L9)

`const` · `pub`

```rust
pub const MAX_METHOD_BYTES: usize = 256;
```

### [MAX_CONTRACT_ID_BYTES](../../../src/common.rs#L10)

`const` · `pub`

```rust
pub const MAX_CONTRACT_ID_BYTES: usize = 64;
```

### [MAX_CANCELLATION_ID_BYTES](../../../src/common.rs#L11)

`const` · `pub`

```rust
pub const MAX_CANCELLATION_ID_BYTES: usize = 128;
```

### [MAX_CRITICAL_EXTENSIONS](../../../src/common.rs#L12)

`const` · `pub`

```rust
pub const MAX_CRITICAL_EXTENSIONS: usize = 16;
```

### [MAX_CRITICAL_EXTENSION_BYTES](../../../src/common.rs#L13)

`const` · `pub`

```rust
pub const MAX_CRITICAL_EXTENSION_BYTES: usize = 128;
```

### [TerminalStatus](../../../src/common.rs#L17)

`enum` · `pub`

```rust
pub enum TerminalStatus
```

### [ApiErrorCode](../../../src/common.rs#L35)

`enum` · `pub`

```rust
pub enum ApiErrorCode
```

### [ApiError](../../../src/common.rs#L47)

`struct` · `pub`

```rust
pub struct ApiError
```

### [new](../../../src/common.rs#L55)

`fn` · `pub`

```rust
pub fn new(code: ApiErrorCode, status: TerminalStatus, message: impl Into<String>) -> Self
```

### [ContractRef](../../../src/common.rs#L65)

`struct` · `pub`

```rust
pub struct ContractRef
```

### [Request](../../../src/common.rs#L72)

`struct` · `pub`

```rust
pub struct Request
```

### [local](../../../src/common.rs#L91)

`fn` · `pub`

```rust
pub fn local(id: impl Into<String>, method: impl Into<String>) -> Self
```

### [Response](../../../src/common.rs#L106)

`struct` · `pub`

```rust
pub struct Response
```

### [success](../../../src/common.rs#L117)

`fn` · `pub`

```rust
pub fn success(id: impl Into<String>, result: Value) -> Self
```

### [result](../../../src/common.rs#L127)

`fn` · `pub`

```rust
pub fn result(id: impl Into<String>, status: TerminalStatus, result: Value) -> Self
```

### [failure](../../../src/common.rs#L141)

`fn` · `pub`

```rust
pub fn failure(id: impl Into<String>, error: ApiError) -> Self
```

### [is_well_formed](../../../src/common.rs#L151)

`fn` · `pub`

```rust
pub const fn is_well_formed(&self) -> bool
```

### [response_has_exactly_one_terminal_payload](../../../src/common.rs#L165)

`fn` · `private`

```rust
fn response_has_exactly_one_terminal_payload()
```
