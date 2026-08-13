# src/engine_jsonl.rs

Status: **OBSERVED bounded development adapter; not production IPC**.

Supervises a separately built Go Engine through one owned bounded worker over correlated JSONL and projects its catalogue/live responses into typed Orchestrator outcomes.

Source: [src/engine_jsonl.rs](../../../src/engine_jsonl.rs)

## Responsibilities

- Bound Engine JSONL frames and the worker command queue.
- Correlate exactly one response to each request.
- Validate version and required live-query capabilities.
- Fail closed on worker panic or timeout and kill the owned child to break blocked I/O.
- Map remote errors without empty-success substitution.
- Stop, join, and reap the child on adapter drop.

## Boundary

- Does not claim installed Engine discovery, authentication, endpoint separation, or service supervision.

## Contract projections

- Development projection of ORC-ENG-001 and ORC-ENG-004

## Source inventory

### [MAX_ENGINE_JSONL_FRAME_BYTES](../../../src/engine_jsonl.rs#L22)

`const` · `pub`

```rust
pub const MAX_ENGINE_JSONL_FRAME_BYTES: usize = 1_048_576;
```

### [ENGINE_CHILD_QUEUE_DEPTH](../../../src/engine_jsonl.rs#L23)

`const` · `private`

```rust
const ENGINE_CHILD_QUEUE_DEPTH: usize = 1;
```

### [ENGINE_CHILD_HANDSHAKE_TIMEOUT](../../../src/engine_jsonl.rs#L24)

`const` · `private`

```rust
const ENGINE_CHILD_HANDSHAKE_TIMEOUT: Duration = Duration::from_secs(2);
```

### [ENGINE_CHILD_CALL_TIMEOUT](../../../src/engine_jsonl.rs#L25)

`const` · `private`

```rust
const ENGINE_CHILD_CALL_TIMEOUT: Duration = Duration::from_secs(6);
```

### [EngineJsonlFault](../../../src/engine_jsonl.rs#L28)

`struct` · `pub`

```rust
pub struct EngineJsonlFault
```

### [EngineJsonlError](../../../src/engine_jsonl.rs#L34)

`enum` · `pub`

```rust
pub enum EngineJsonlError
```

### [fmt](../../../src/engine_jsonl.rs#L50)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result
```

### [source](../../../src/engine_jsonl.rs#L84)

`fn` · `private`

```rust
fn source(&self) -> Option<&(dyn Error + 'static)>
```

### [from](../../../src/engine_jsonl.rs#L94)

`fn` · `private`

```rust
fn from(error: std::io::Error) -> Self
```

### [EngineJsonlRequest](../../../src/engine_jsonl.rs#L100)

`struct` · `private`

```rust
struct EngineJsonlRequest<'a>
```

### [EngineJsonlResponse](../../../src/engine_jsonl.rs#L107)

`struct` · `private`

```rust
struct EngineJsonlResponse
```

### [EngineJsonlPeer](../../../src/engine_jsonl.rs#L116)

`struct` · `pub`

```rust
pub struct EngineJsonlPeer<R, W>
```

### [new](../../../src/engine_jsonl.rs#L124)

`fn` · `pub`

```rust
pub const fn new(reader: R, writer: W) -> Self
```

### [call](../../../src/engine_jsonl.rs#L139)

`fn` · `pub`

```rust
pub fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError>
```

### [into_parts](../../../src/engine_jsonl.rs#L185)

`fn` · `pub`

```rust
pub fn into_parts(self) -> (R, W)
```

### [EngineJsonlCaller](../../../src/engine_jsonl.rs#L190)

`trait` · `pub`

```rust
pub trait EngineJsonlCaller
```

### [call](../../../src/engine_jsonl.rs#L196)

`fn` · `private`

```rust
fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError>;
```

### [call](../../../src/engine_jsonl.rs#L200)

`fn` · `private`

```rust
fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError>
```

### [EngineCatalogueWirePlan](../../../src/engine_jsonl.rs#L206)

`struct` · `private`

```rust
struct EngineCatalogueWirePlan
```

### [EngineCatalogueWireResponse](../../../src/engine_jsonl.rs#L214)

`struct` · `private`

```rust
struct EngineCatalogueWireResponse
```

### [EngineLiveWireResponse](../../../src/engine_jsonl.rs#L227)

`struct` · `private`

```rust
struct EngineLiveWireResponse
```

### [EngineJsonlSearchAdapter](../../../src/engine_jsonl.rs#L250)

`struct` · `pub`

```rust
pub struct EngineJsonlSearchAdapter<C>
```

### [EngineJsonlChild](../../../src/engine_jsonl.rs#L255)

`struct` · `pub`

```rust
pub struct EngineJsonlChild
```

### [EngineWorkerPeer](../../../src/engine_jsonl.rs#L264)

`struct` · `private`

```rust
struct EngineWorkerPeer
```

### [EngineWorkerCommand](../../../src/engine_jsonl.rs#L272)

`enum` · `private`

```rust
enum EngineWorkerCommand
```

### [call](../../../src/engine_jsonl.rs#L282)

`fn` · `private`

```rust
fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError>
```

### [stop](../../../src/engine_jsonl.rs#L312)

`fn` · `private`

```rust
fn stop(&self)
```

### [run_engine_worker](../../../src/engine_jsonl.rs#L317)

`fn` · `private`

```rust
fn run_engine_worker<C: EngineJsonlCaller>( mut peer: C, receiver: &Receiver<EngineWorkerCommand>, healthy: &AtomicBool, )
```

### [engine_worker_error_is_terminal](../../../src/engine_jsonl.rs#L345)

`fn` · `private`

```rust
fn engine_worker_error_is_terminal(error: &EngineJsonlError) -> bool
```

### [kill_engine_child](../../../src/engine_jsonl.rs#L354)

`fn` · `private`

```rust
fn kill_engine_child(child: &Mutex<Child>)
```

### [wait_engine_child](../../../src/engine_jsonl.rs#L360)

`fn` · `private`

```rust
fn wait_engine_child(child: &Mutex<Child>)
```

### [spawn](../../../src/engine_jsonl.rs#L383)

`fn` · `pub`

```rust
pub fn spawn( binary: &Path, sandbox_root: &Path, root_id: &str, root_path: &Path, ) -> Result<Self, EngineJsonlError>
```

### [query_catalogue](../../../src/engine_jsonl.rs#L462)

`fn` · `private`

```rust
fn query_catalogue(&mut self, request: &EngineSearchRequest) -> EngineQueryResultFixture
```

### [query_live](../../../src/engine_jsonl.rs#L466)

`fn` · `private`

```rust
fn query_live(&mut self, request: &EngineSearchRequest) -> EngineLiveQueryResultFixture
```

### [live_available](../../../src/engine_jsonl.rs#L470)

`fn` · `private`

```rust
fn live_available(&self) -> bool
```

### [drop](../../../src/engine_jsonl.rs#L476)

`fn` · `private`

```rust
fn drop(&mut self)
```

### [new](../../../src/engine_jsonl.rs#L490)

`fn` · `pub`

```rust
pub const fn new(peer: C) -> Self
```

### [into_peer](../../../src/engine_jsonl.rs#L495)

`fn` · `pub`

```rust
pub fn into_peer(self) -> C
```

### [peer_mut](../../../src/engine_jsonl.rs#L499)

`fn` · `pub`

```rust
pub const fn peer_mut(&mut self) -> &mut C
```

### [catalogue_params](../../../src/engine_jsonl.rs#L505)

`fn` · `private`

```rust
fn catalogue_params(request: &EngineSearchRequest) -> Value
```

### [live_params](../../../src/engine_jsonl.rs#L527)

`fn` · `private`

```rust
fn live_params(request: &EngineSearchRequest) -> Value
```

### [query_catalogue](../../../src/engine_jsonl.rs#L546)

`fn` · `private`

```rust
fn query_catalogue(&mut self, request: &EngineSearchRequest) -> EngineQueryResultFixture
```

### [query_live](../../../src/engine_jsonl.rs#L599)

`fn` · `private`

```rust
fn query_live(&mut self, request: &EngineSearchRequest) -> EngineLiveQueryResultFixture
```

### [catalogue_failure](../../../src/engine_jsonl.rs#L648)

`fn` · `private`

```rust
fn catalogue_failure( terminal: TerminalStatus, code: &str, message: &str, ) -> EngineQueryResultFixture
```

### [live_failure](../../../src/engine_jsonl.rs#L671)

`fn` · `private`

```rust
fn live_failure( terminal: TerminalStatus, code: &str, message: &str, ) -> EngineLiveQueryResultFixture
```

### [project_engine_error](../../../src/engine_jsonl.rs#L699)

`fn` · `pub(crate)`

```rust
pub(crate) fn project_engine_error( error: EngineJsonlError, operation: &str, ) -> (TerminalStatus, String, String)
```

### [error_kind](../../../src/engine_jsonl.rs#L734)

`fn` · `private`

```rust
const fn error_kind(error: &EngineJsonlError) -> &'static str
```

### [PanickingCaller](../../../src/engine_jsonl.rs#L770)

`struct` · `private`

```rust
struct PanickingCaller;
```

### [call](../../../src/engine_jsonl.rs#L773)

`fn` · `private`

```rust
fn call( &mut self, _method: &str, _params: &serde_json::Value, ) -> Result<serde_json::Value, EngineJsonlError>
```

### [SlowCaller](../../../src/engine_jsonl.rs#L782)

`struct` · `private`

```rust
struct SlowCaller;
```

### [call](../../../src/engine_jsonl.rs#L785)

`fn` · `private`

```rust
fn call( &mut self, _method: &str, _params: &serde_json::Value, ) -> Result<serde_json::Value, EngineJsonlError>
```

### [RecordingCaller](../../../src/engine_jsonl.rs#L796)

`struct` · `private`

```rust
struct RecordingCaller
```

### [call](../../../src/engine_jsonl.rs#L802)

`fn` · `private`

```rust
fn call( &mut self, method: &str, params: &serde_json::Value, ) -> Result<serde_json::Value, EngineJsonlError>
```

### [bounded_peer_correlates_one_successful_response](../../../src/engine_jsonl.rs#L821)

`fn` · `private`

```rust
fn bounded_peer_correlates_one_successful_response()
```

### [remote_fault_is_not_an_empty_success](../../../src/engine_jsonl.rs#L838)

`fn` · `private`

```rust
fn remote_fault_is_not_an_empty_success()
```

### [response_identity_mismatch_fails_closed](../../../src/engine_jsonl.rs#L849)

`fn` · `private`

```rust
fn response_identity_mismatch_fails_closed()
```

### [oversized_response_is_rejected_without_unbounded_allocation](../../../src/engine_jsonl.rs#L860)

`fn` · `private`

```rust
fn oversized_response_is_rejected_without_unbounded_allocation()
```

### [exact_wire_result_projects_generation_cursor_and_partial_roots](../../../src/engine_jsonl.rs#L872)

`fn` · `private`

```rust
fn exact_wire_result_projects_generation_cursor_and_partial_roots()
```

### [exact_filter_only_request_projects_the_frozen_engine_filter_map](../../../src/engine_jsonl.rs#L886)

`fn` · `private`

```rust
fn exact_filter_only_request_projects_the_frozen_engine_filter_map()
```

### [unavailable_live_method_projects_typed_unsupported_result](../../../src/engine_jsonl.rs#L907)

`fn` · `private`

```rust
fn unavailable_live_method_projects_typed_unsupported_result()
```

### [missing_catalogue_projects_unavailable_not_unsupported](../../../src/engine_jsonl.rs#L918)

`fn` · `private`

```rust
fn missing_catalogue_projects_unavailable_not_unsupported()
```

### [engine_worker_panic_is_contained_and_marks_worker_unavailable](../../../src/engine_jsonl.rs#L929)

`fn` · `private`

```rust
fn engine_worker_panic_is_contained_and_marks_worker_unavailable()
```

### [engine_worker_timeout_kills_the_owned_child_and_fails_closed](../../../src/engine_jsonl.rs#L954)

`fn` · `private`

```rust
fn engine_worker_timeout_kills_the_owned_child_and_fails_closed()
```

### [search_request](../../../src/engine_jsonl.rs#L992)

`fn` · `private`

```rust
fn search_request() -> EngineSearchRequest
```
