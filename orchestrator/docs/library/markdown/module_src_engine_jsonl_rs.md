# src/engine_jsonl.rs

Status: **OBSERVED bounded development adapter; not production IPC**.

Supervises a separately built Go Engine over correlated bounded JSONL and projects its catalogue/live responses into typed Orchestrator outcomes.

Source: [src/engine_jsonl.rs](../../../src/engine_jsonl.rs)

## Responsibilities

- Bound Engine JSONL frames.
- Correlate exactly one response to each request.
- Validate version and required live-query capabilities.
- Map remote errors without empty-success substitution.
- Terminate the child on adapter drop.

## Boundary

- Does not claim installed Engine discovery, authentication, endpoint separation, or service supervision.

## Contract projections

- Development projection of ORC-ENG-001 and ORC-ENG-004

## Source inventory

### [MAX_ENGINE_JSONL_FRAME_BYTES](../../../src/engine_jsonl.rs#L17)

`const` · `pub`

```rust
pub const MAX_ENGINE_JSONL_FRAME_BYTES: usize = 1_048_576;
```

### [EngineJsonlFault](../../../src/engine_jsonl.rs#L20)

`struct` · `pub`

```rust
pub struct EngineJsonlFault
```

### [EngineJsonlError](../../../src/engine_jsonl.rs#L26)

`enum` · `pub`

```rust
pub enum EngineJsonlError
```

### [fmt](../../../src/engine_jsonl.rs#L39)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result
```

### [source](../../../src/engine_jsonl.rs#L68)

`fn` · `private`

```rust
fn source(&self) -> Option<&(dyn Error + 'static)>
```

### [from](../../../src/engine_jsonl.rs#L78)

`fn` · `private`

```rust
fn from(error: std::io::Error) -> Self
```

### [EngineJsonlRequest](../../../src/engine_jsonl.rs#L84)

`struct` · `private`

```rust
struct EngineJsonlRequest<'a>
```

### [EngineJsonlResponse](../../../src/engine_jsonl.rs#L91)

`struct` · `private`

```rust
struct EngineJsonlResponse
```

### [EngineJsonlPeer](../../../src/engine_jsonl.rs#L100)

`struct` · `pub`

```rust
pub struct EngineJsonlPeer<R, W>
```

### [new](../../../src/engine_jsonl.rs#L108)

`fn` · `pub`

```rust
pub const fn new(reader: R, writer: W) -> Self
```

### [call](../../../src/engine_jsonl.rs#L123)

`fn` · `pub`

```rust
pub fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError>
```

### [into_parts](../../../src/engine_jsonl.rs#L169)

`fn` · `pub`

```rust
pub fn into_parts(self) -> (R, W)
```

### [EngineCatalogueWirePlan](../../../src/engine_jsonl.rs#L175)

`struct` · `private`

```rust
struct EngineCatalogueWirePlan
```

### [EngineCatalogueWireResponse](../../../src/engine_jsonl.rs#L183)

`struct` · `private`

```rust
struct EngineCatalogueWireResponse
```

### [EngineLiveWireResponse](../../../src/engine_jsonl.rs#L196)

`struct` · `private`

```rust
struct EngineLiveWireResponse
```

### [EngineJsonlSearchAdapter](../../../src/engine_jsonl.rs#L219)

`struct` · `pub`

```rust
pub struct EngineJsonlSearchAdapter<R, W>
```

### [EngineJsonlChild](../../../src/engine_jsonl.rs#L224)

`struct` · `pub`

```rust
pub struct EngineJsonlChild
```

### [spawn](../../../src/engine_jsonl.rs#L238)

`fn` · `pub`

```rust
pub fn spawn( binary: &Path, sandbox_root: &Path, root_id: &str, root_path: &Path, ) -> Result<Self, EngineJsonlError>
```

### [query_catalogue](../../../src/engine_jsonl.rs#L288)

`fn` · `private`

```rust
fn query_catalogue(&mut self, request: &EngineSearchRequest) -> EngineQueryResultFixture
```

### [query_live](../../../src/engine_jsonl.rs#L292)

`fn` · `private`

```rust
fn query_live(&mut self, request: &EngineSearchRequest) -> EngineLiveQueryResultFixture
```

### [live_available](../../../src/engine_jsonl.rs#L296)

`fn` · `private`

```rust
fn live_available(&self) -> bool
```

### [drop](../../../src/engine_jsonl.rs#L302)

`fn` · `private`

```rust
fn drop(&mut self)
```

### [new](../../../src/engine_jsonl.rs#L310)

`fn` · `pub`

```rust
pub const fn new(peer: EngineJsonlPeer<R, W>) -> Self
```

### [into_peer](../../../src/engine_jsonl.rs#L315)

`fn` · `pub`

```rust
pub fn into_peer(self) -> EngineJsonlPeer<R, W>
```

### [peer_mut](../../../src/engine_jsonl.rs#L319)

`fn` · `pub`

```rust
pub fn peer_mut(&mut self) -> &mut EngineJsonlPeer<R, W>
```

### [catalogue_params](../../../src/engine_jsonl.rs#L323)

`fn` · `private`

```rust
fn catalogue_params(request: &EngineSearchRequest) -> Value
```

### [live_params](../../../src/engine_jsonl.rs#L341)

`fn` · `private`

```rust
fn live_params(request: &EngineSearchRequest) -> Value
```

### [query_catalogue](../../../src/engine_jsonl.rs#L360)

`fn` · `private`

```rust
fn query_catalogue(&mut self, request: &EngineSearchRequest) -> EngineQueryResultFixture
```

### [query_live](../../../src/engine_jsonl.rs#L413)

`fn` · `private`

```rust
fn query_live(&mut self, request: &EngineSearchRequest) -> EngineLiveQueryResultFixture
```

### [catalogue_failure](../../../src/engine_jsonl.rs#L462)

`fn` · `private`

```rust
fn catalogue_failure( terminal: TerminalStatus, code: &str, message: &str, ) -> EngineQueryResultFixture
```

### [live_failure](../../../src/engine_jsonl.rs#L485)

`fn` · `private`

```rust
fn live_failure( terminal: TerminalStatus, code: &str, message: &str, ) -> EngineLiveQueryResultFixture
```

### [project_engine_error](../../../src/engine_jsonl.rs#L513)

`fn` · `private`

```rust
fn project_engine_error( error: EngineJsonlError, operation: &str, ) -> (TerminalStatus, String, String)
```

### [error_kind](../../../src/engine_jsonl.rs#L543)

`fn` · `private`

```rust
const fn error_kind(error: &EngineJsonlError) -> &'static str
```

### [bounded_peer_correlates_one_successful_response](../../../src/engine_jsonl.rs#L569)

`fn` · `private`

```rust
fn bounded_peer_correlates_one_successful_response()
```

### [remote_fault_is_not_an_empty_success](../../../src/engine_jsonl.rs#L586)

`fn` · `private`

```rust
fn remote_fault_is_not_an_empty_success()
```

### [response_identity_mismatch_fails_closed](../../../src/engine_jsonl.rs#L597)

`fn` · `private`

```rust
fn response_identity_mismatch_fails_closed()
```

### [oversized_response_is_rejected_without_unbounded_allocation](../../../src/engine_jsonl.rs#L608)

`fn` · `private`

```rust
fn oversized_response_is_rejected_without_unbounded_allocation()
```

### [exact_wire_result_projects_generation_cursor_and_partial_roots](../../../src/engine_jsonl.rs#L620)

`fn` · `private`

```rust
fn exact_wire_result_projects_generation_cursor_and_partial_roots()
```

### [unavailable_live_method_projects_typed_unsupported_result](../../../src/engine_jsonl.rs#L634)

`fn` · `private`

```rust
fn unavailable_live_method_projects_typed_unsupported_result()
```

### [missing_catalogue_projects_unavailable_not_unsupported](../../../src/engine_jsonl.rs#L645)

`fn` · `private`

```rust
fn missing_catalogue_projects_unavailable_not_unsupported()
```

### [search_request](../../../src/engine_jsonl.rs#L655)

`fn` · `private`

```rust
fn search_request() -> EngineSearchRequest
```
