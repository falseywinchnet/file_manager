# src/local_session.rs

Status: **Stable Core 1.0 hello/authentication projection**.

Defines rotating session tokens, bounded client/server hello values, local-wire version negotiation, and constant-time credential comparison.

Source: [src/local_session.rs](../../../src/local_session.rs)

## Responsibilities

- Generate credentials from OS randomness.
- Encode/decode exact token length.
- Redact token debug output and erase secret buffers.
- Reject incompatible wire ranges and wrong credentials.

## Boundary

- A token proves possession inside the accepted same-user authority class; it does not create plugin authority.

## Contract projections

- orchestrator.local 1.0 hello
- ORC-LIF-001 session identity

## Source inventory

### [LOCAL_WIRE_FAMILY](../../../src/local_session.rs#L6)

`const` · `pub`

```rust
pub const LOCAL_WIRE_FAMILY: &str = "orchestrator.local";
```

### [LOCAL_WIRE_MAJOR](../../../src/local_session.rs#L7)

`const` · `pub`

```rust
pub const LOCAL_WIRE_MAJOR: u16 = 1;
```

### [LOCAL_WIRE_MINOR](../../../src/local_session.rs#L8)

`const` · `pub`

```rust
pub const LOCAL_WIRE_MINOR: u16 = 0;
```

### [SESSION_TOKEN_BYTES](../../../src/local_session.rs#L9)

`const` · `private`

```rust
const SESSION_TOKEN_BYTES: usize = 32;
```

### [MAX_CLIENT_NAME_BYTES](../../../src/local_session.rs#L10)

`const` · `pub`

```rust
pub const MAX_CLIENT_NAME_BYTES: usize = 128;
```

### [SessionToken](../../../src/local_session.rs#L13)

`struct` · `pub`

```rust
pub struct SessionToken([u8; SESSION_TOKEN_BYTES]);
```

### [generate](../../../src/local_session.rs#L23)

`fn` · `pub`

```rust
pub fn generate() -> Result<Self, getrandom::Error>
```

### [encode](../../../src/local_session.rs#L30)

`fn` · `pub`

```rust
pub fn encode(&self) -> String
```

### [decode](../../../src/local_session.rs#L45)

`fn` · `pub`

```rust
pub fn decode(encoded: &str) -> Result<Self, SessionAuthError>
```

### [matches](../../../src/local_session.rs#L59)

`fn` · `pub`

```rust
pub fn matches(&self, candidate: &Self) -> bool
```

### [fmt](../../../src/local_session.rs#L71)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result
```

### [ClientHello](../../../src/local_session.rs#L77)

`struct` · `pub`

```rust
pub struct ClientHello
```

### [fmt](../../../src/local_session.rs#L90)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result
```

### [new](../../../src/local_session.rs#L104)

`fn` · `pub`

```rust
pub fn new(client: impl Into<String>, credential: &SessionToken) -> Self
```

### [ServerHello](../../../src/local_session.rs#L116)

`struct` · `pub`

```rust
pub struct ServerHello
```

### [SessionAuthError](../../../src/local_session.rs#L126)

`enum` · `pub`

```rust
pub enum SessionAuthError
```

### [fmt](../../../src/local_session.rs#L134)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result
```

### [authenticate_client](../../../src/local_session.rs#L156)

`fn` · `pub`

```rust
pub fn authenticate_client( expected: &SessionToken, hello: &ClientHello, ) -> Result<(), SessionAuthError>
```

### [credential_round_trip_authenticates_and_debug_is_redacted](../../../src/local_session.rs#L182)

`fn` · `private`

```rust
fn credential_round_trip_authenticates_and_debug_is_redacted()
```

### [wrong_credential_and_major_fail_closed](../../../src/local_session.rs#L195)

`fn` · `private`

```rust
fn wrong_credential_and_major_fail_closed()
```
