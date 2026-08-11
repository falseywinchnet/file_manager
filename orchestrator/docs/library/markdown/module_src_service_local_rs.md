# src/service/local.rs

Status: **OBSERVED bounded Unix daemon host**.

Owns four fixed blocking session workers with independently owned bounded queues, authentication handoff, concurrent kernel dispatch, fail-closed panic containment, and coordinated shutdown.

Source: [src/service/local.rs](../../../src/service/local.rs)

## Responsibilities

- Bind or serve an owned UnixEndpoint.
- Attach the optional development Engine child.
- Round-robin accepted streams across independent worker queues while preserving the aggregate pending-session ceiling.
- Interrupt active sessions and join workers during shutdown.
- Record relaxed runtime health counters.

## Boundary

- Endpoint publication and authentication stay in local_endpoint.
- Frame encoding stays in local_wire.
- Kernel semantics stay in kernel.rs.
- The Engine adapter is development-only.

## Contract projections

- orchestrator.local 1.0 host
- ORC-LIF-001 lifecycle
- ORC-FE-001 local dispatch

## Source inventory

### [ACCEPT_POLL](../../../src/service/local.rs#L26)

`const` · `private`

```rust
const ACCEPT_POLL: Duration = Duration::from_millis(10);
```

### [SESSION_IDLE_TIMEOUT](../../../src/service/local.rs#L27)

`const` · `private`

```rust
const SESSION_IDLE_TIMEOUT: Duration = Duration::from_secs(5);
```

### [serve_local](../../../src/service/local.rs#L29)

`fn` · `pub(crate)`

```rust
pub(crate) fn serve_local( runtime_directory: &Path, engine_options: Option<EngineProviderConfig>, settings_directory: Option<&Path>, ) -> Result<(), String>
```

### [serve_owned](../../../src/service/local.rs#L52)

`fn` · `pub(crate)`

```rust
pub(crate) fn serve_owned(endpoint: UnixEndpoint, kernel: Kernel) -> Result<(), String>
```

### [serve_endpoint](../../../src/service/local.rs#L59)

`fn` · `pub(super)`

```rust
pub(super) fn serve_endpoint(endpoint: &UnixEndpoint, kernel: Kernel) -> Result<(), String>
```

### [run_contained_worker](../../../src/service/local.rs#L121)

`fn` · `private`

```rust
fn run_contained_worker<F>(shutdown: &AtomicBool, worker: F) -> Result<(), ()> where F: FnOnce(),
```

### [serve_connection](../../../src/service/local.rs#L133)

`fn` · `private`

```rust
fn serve_connection( stream: &mut UnixStream, kernel: &Arc<Kernel>, shutdown: &AtomicBool, health: &RuntimeHealth, ) -> Result<(), String>
```

### [accept_sessions](../../../src/service/local.rs#L156)

`fn` · `private`

```rust
fn accept_sessions( endpoint: &UnixEndpoint, senders: &[SyncSender<UnixStream>], shutdown: &AtomicBool, rejected: &AtomicU64, health: &RuntimeHealth, ) -> Result<(), String>
```

### [session_worker](../../../src/service/local.rs#L201)

`fn` · `private`

```rust
fn session_worker( worker_id: usize, endpoint: &UnixEndpoint, receiver: &Receiver<UnixStream>, kernel: &Arc<Kernel>, shutdown: &Arc<AtomicBool>, active: &Arc<Mutex<HashMap<usize, UnixStream>>>, health: &Arc<RuntimeHealth>, )
```

### [ActiveSessionHealth](../../../src/service/local.rs#L257)

`struct` · `private`

```rust
struct ActiveSessionHealth
```

### [new](../../../src/service/local.rs#L262)

`fn` · `private`

```rust
fn new(health: Arc<RuntimeHealth>) -> Self
```

### [drop](../../../src/service/local.rs#L269)

`fn` · `private`

```rust
fn drop(&mut self)
```

### [close_active_sessions](../../../src/service/local.rs#L274)

`fn` · `private`

```rust
fn close_active_sessions(active: &Mutex<HashMap<usize, UnixStream>>)
```

### [remove_active_session](../../../src/service/local.rs#L284)

`fn` · `private`

```rust
fn remove_active_session(active: &Mutex<HashMap<usize, UnixStream>>, worker_id: usize)
```

### [call_local](../../../src/service/local.rs#L290)

`fn` · `pub(crate)`

```rust
pub(crate) fn call_local(runtime_directory: &Path, request: &Request) -> Result<Response, String>
```

### [worker_panic_requests_daemon_shutdown](../../../src/service/local.rs#L303)

`fn` · `private`

```rust
fn worker_panic_requests_daemon_shutdown()
```
