# src/kernel.rs

Status: **OBSERVED Core 1.0 semantic dispatcher**.

Validates transport-neutral requests, dispatches Core methods, projects immutable status/bootstrap state, brokers search, and owns deterministic shutdown state.

Source: [src/kernel.rs](../../../src/kernel.rs)

## Responsibilities

- Validate attacker-controlled envelope fields before reflection.
- Enforce critical-extension, cancellation, deadline, contract, and response-budget law.
- Build status and frontend bootstrap snapshots from canonical catalogues.
- Route unified search through a separately locked typed provider port without holding lifecycle state.
- Keep plugin and semantic-fact operations explicit stubs.

## Boundary

- Owns no socket, thread, UI, database, or provider-private type.
- Does not decide the external GUI.Forms or architect gates.

## Contract projections

- ORC-COM-001
- ORC-LIF-001
- ORC-FE-001
- ORC-ENG-004

## Source inventory

### [Kernel](../../../src/kernel.rs#L19)

`struct` · `pub`

```rust
pub struct Kernel
```

### [default](../../../src/kernel.rs#L28)

`fn` · `private`

```rust
fn default() -> Self
```

### [live_search_available](../../../src/kernel.rs#L34)

`fn` · `private`

```rust
fn live_search_available(&self) -> bool
```

### [new](../../../src/kernel.rs#L39)

`fn` · `pub`

```rust
pub fn new() -> Self
```

### [for_local_daemon](../../../src/kernel.rs#L50)

`fn` · `pub`

```rust
pub fn for_local_daemon() -> Self
```

### [for_supervised_daemon](../../../src/kernel.rs#L61)

`fn` · `pub`

```rust
pub fn for_supervised_daemon() -> Self
```

### [with_engine_search](../../../src/kernel.rs#L72)

`fn` · `pub`

```rust
pub fn with_engine_search(mut self, search: impl UnifiedEngineSearch + 'static) -> Self
```

### [runtime_health](../../../src/kernel.rs#L80)

`fn` · `pub`

```rust
pub fn runtime_health(&self) -> Arc<RuntimeHealth>
```

### [handle](../../../src/kernel.rs#L85)

`fn` · `pub`

```rust
pub fn handle(&self, request: Request) -> Response
```

### [is_stopped](../../../src/kernel.rs#L171)

`fn` · `pub`

```rust
pub fn is_stopped(&self) -> bool
```

### [lifecycle_generation](../../../src/kernel.rs#L176)

`fn` · `pub`

```rust
pub fn lifecycle_generation(&self) -> u64
```

### [status_result](../../../src/kernel.rs#L180)

`fn` · `private`

```rust
fn status_result(&self) -> Value
```

### [status_result_with](../../../src/kernel.rs#L185)

`fn` · `private`

```rust
fn status_result_with(&self, release: &crate::release::CoreReleaseManifest) -> Value
```

### [frontend_bootstrap_result](../../../src/kernel.rs#L219)

`fn` · `private`

```rust
fn frontend_bootstrap_result(&self) -> Value
```

### [availability_result](../../../src/kernel.rs#L264)

`fn` · `private`

```rust
fn availability_result(&self) -> Value
```

### [availability_count](../../../src/kernel.rs#L289)

`fn` · `private`

```rust
fn availability_count(&self, state: AvailabilityState) -> usize
```

### [search_response](../../../src/kernel.rs#L307)

`fn` · `private`

```rust
fn search_response(&self, id: String, params: Value) -> Response
```

### [shutdown_result](../../../src/kernel.rs#L391)

`fn` · `private`

```rust
fn shutdown_result(&self) -> Result<Value, ApiError>
```

### [lifecycle_snapshot](../../../src/kernel.rs#L412)

`fn` · `private`

```rust
fn lifecycle_snapshot(&self) -> crate::lifecycle::LifecycleSnapshot
```

### [frontend_opening_result](../../../src/kernel.rs#L420)

`fn` · `private`

```rust
fn frontend_opening_result( release: &crate::release::CoreReleaseManifest, capabilities: &[crate::availability::CapabilityAvailability], restart_eligible: bool, ) -> Value
```

### [validate_critical_extensions](../../../src/kernel.rs#L494)

`fn` · `private`

```rust
fn validate_critical_extensions(request: &Request) -> Result<(), ApiError>
```

### [validate_request_id](../../../src/kernel.rs#L505)

`fn` · `private`

```rust
fn validate_request_id(id: &str) -> Result<(), ApiError>
```

### [validate_request_shape](../../../src/kernel.rs#L516)

`fn` · `private`

```rust
fn validate_request_shape(request: &Request) -> Result<(), ApiError>
```

### [validate_cancellation](../../../src/kernel.rs#L567)

`fn` · `private`

```rust
fn validate_cancellation(request: &Request) -> Result<(), ApiError>
```

### [version_result](../../../src/kernel.rs#L585)

`fn` · `private`

```rust
fn version_result() -> Value
```

### [search_error_code](../../../src/kernel.rs#L606)

`fn` · `private`

```rust
const fn search_error_code(status: TerminalStatus) -> ApiErrorCode
```

### [stub_result](../../../src/kernel.rs#L623)

`fn` · `private`

```rust
fn stub_result(capability: &str, reason: &str) -> Value
```

### [stub_error](../../../src/kernel.rs#L631)

`fn` · `private`

```rust
fn stub_error(message: &str) -> ApiError
```

### [internal_serialization_error](../../../src/kernel.rs#L639)

`fn` · `private`

```rust
fn internal_serialization_error(error: &serde_json::Error) -> ApiError
```

### [validate_deadline](../../../src/kernel.rs#L647)

`fn` · `private`

```rust
fn validate_deadline(request: &Request) -> Result<(), ApiError>
```

### [validate_contract](../../../src/kernel.rs#L671)

`fn` · `private`

```rust
fn validate_contract(request: &Request) -> Result<(), ApiError>
```

### [enforce_response_budget](../../../src/kernel.rs#L700)

`fn` · `private`

```rust
fn enforce_response_budget( response_id: String, max_response_bytes: Option<u64>, response: Response, ) -> Response
```

### [BlockingSearch](../../../src/kernel.rs#L741)

`struct` · `private`

```rust
struct BlockingSearch
```

### [search](../../../src/kernel.rs#L747)

`fn` · `private`

```rust
fn search( &mut self, _request: &crate::engine_contract::EngineSearchRequest, ) -> EngineSearchOutcome
```

### [live_available](../../../src/kernel.rs#L760)

`fn` · `private`

```rust
fn live_available(&self) -> bool
```

### [BecomesUnavailable](../../../src/kernel.rs#L765)

`struct` · `private`

```rust
struct BecomesUnavailable
```

### [search](../../../src/kernel.rs#L770)

`fn` · `private`

```rust
fn search( &mut self, _request: &crate::engine_contract::EngineSearchRequest, ) -> EngineSearchOutcome
```

### [live_available](../../../src/kernel.rs#L782)

`fn` · `private`

```rust
fn live_available(&self) -> bool
```

### [kernel_reports_stubs_without_fact_or_plugin_operations](../../../src/kernel.rs#L788)

`fn` · `private`

```rust
fn kernel_reports_stubs_without_fact_or_plugin_operations()
```

### [shutdown_is_terminal](../../../src/kernel.rs#L802)

`fn` · `private`

```rust
fn shutdown_is_terminal()
```

### [frontend_bootstrap_is_one_bounded_immutable_snapshot](../../../src/kernel.rs#L813)

`fn` · `private`

```rust
fn frontend_bootstrap_is_one_bounded_immutable_snapshot()
```

### [separately_gated_provider_absence_does_not_block_a_ready_core_projection](../../../src/kernel.rs#L901)

`fn` · `private`

```rust
fn separately_gated_provider_absence_does_not_block_a_ready_core_projection()
```

### [attacker_controlled_envelope_fields_are_bounded_before_error_reflection](../../../src/kernel.rs#L939)

`fn` · `private`

```rust
fn attacker_controlled_envelope_fields_are_bounded_before_error_reflection()
```

### [blocked_engine_provider_does_not_hold_core_status_or_lifecycle_state](../../../src/kernel.rs#L966)

`fn` · `private`

```rust
fn blocked_engine_provider_does_not_hold_core_status_or_lifecycle_state()
```

### [provider_health_cache_updates_after_terminal_worker_call](../../../src/kernel.rs#L1012)

`fn` · `private`

```rust
fn provider_health_cache_updates_after_terminal_worker_call()
```
