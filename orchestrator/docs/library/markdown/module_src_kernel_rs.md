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

### [Kernel](../../../src/kernel.rs#L22)

`struct` · `pub`

```rust
pub struct Kernel
```

### [default](../../../src/kernel.rs#L35)

`fn` · `private`

```rust
fn default() -> Self
```

### [live_search_available](../../../src/kernel.rs#L41)

`fn` · `private`

```rust
fn live_search_available(&self) -> bool
```

### [new](../../../src/kernel.rs#L46)

`fn` · `pub`

```rust
pub fn new() -> Self
```

### [for_local_daemon](../../../src/kernel.rs#L61)

`fn` · `pub`

```rust
pub fn for_local_daemon() -> Self
```

### [for_supervised_daemon](../../../src/kernel.rs#L76)

`fn` · `pub`

```rust
pub fn for_supervised_daemon() -> Self
```

### [with_engine_search](../../../src/kernel.rs#L91)

`fn` · `pub`

```rust
pub fn with_engine_search(mut self, search: impl UnifiedEngineSearch + 'static) -> Self
```

### [with_installed_engine_search](../../../src/kernel.rs#L100)

`fn` · `pub`

```rust
pub fn with_installed_engine_search( mut self, search: impl UnifiedEngineSearch + 'static, ) -> Self
```

### [with_installed_engine_admin](../../../src/kernel.rs#L112)

`fn` · `pub`

```rust
pub fn with_installed_engine_admin( mut self, caller: impl EngineJsonlCaller + Send + 'static, ) -> Self
```

### [with_settings](../../../src/kernel.rs#L121)

`fn` · `pub`

```rust
pub fn with_settings(mut self, settings: SettingsService) -> Self
```

### [with_instance_id](../../../src/kernel.rs#L127)

`fn` · `pub`

```rust
pub fn with_instance_id(mut self, instance_id: String) -> Self
```

### [runtime_health](../../../src/kernel.rs#L133)

`fn` · `pub`

```rust
pub fn runtime_health(&self) -> Arc<RuntimeHealth>
```

### [handle](../../../src/kernel.rs#L141)

`fn` · `pub`

```rust
pub fn handle(&self, request: Request) -> Response
```

### [is_stopped](../../../src/kernel.rs#L255)

`fn` · `pub`

```rust
pub fn is_stopped(&self) -> bool
```

### [lifecycle_generation](../../../src/kernel.rs#L260)

`fn` · `pub`

```rust
pub fn lifecycle_generation(&self) -> u64
```

### [status_result](../../../src/kernel.rs#L264)

`fn` · `private`

```rust
fn status_result(&self) -> Value
```

### [status_result_with](../../../src/kernel.rs#L269)

`fn` · `private`

```rust
fn status_result_with(&self, release: &crate::release::CoreReleaseManifest) -> Value
```

### [frontend_bootstrap_result](../../../src/kernel.rs#L303)

`fn` · `private`

```rust
fn frontend_bootstrap_result(&self) -> Result<Value, ApiError>
```

### [availability_result](../../../src/kernel.rs#L367)

`fn` · `private`

```rust
fn availability_result(&self) -> Value
```

### [availability_count](../../../src/kernel.rs#L428)

`fn` · `private`

```rust
fn availability_count(&self, state: AvailabilityState) -> usize
```

### [search_response](../../../src/kernel.rs#L458)

`fn` · `private`

```rust
fn search_response(&self, id: String, params: Value) -> Response
```

### [shutdown_result](../../../src/kernel.rs#L542)

`fn` · `private`

```rust
fn shutdown_result(&self) -> Result<Value, ApiError>
```

### [services_snapshot_result](../../../src/kernel.rs#L563)

`fn` · `private`

```rust
fn services_snapshot_result(&self) -> Value
```

### [engine_service_snapshot](../../../src/kernel.rs#L590)

`fn` · `private`

```rust
fn engine_service_snapshot(&self) -> Value
```

### [service_command_result](../../../src/kernel.rs#L631)

`fn` · `private`

```rust
fn service_command_result(&self, command: &ServiceCommandRequest) -> Result<Value, ApiError>
```

### [orchestrator_command_result](../../../src/kernel.rs#L639)

`fn` · `private`

```rust
fn orchestrator_command_result( &self, command: &ServiceCommandRequest, ) -> Result<Value, ApiError>
```

### [engine_command_result](../../../src/kernel.rs#L686)

`fn` · `private`

```rust
fn engine_command_result(&self, command: &ServiceCommandRequest) -> Result<Value, ApiError>
```

### [lifecycle_snapshot](../../../src/kernel.rs#L759)

`fn` · `private`

```rust
fn lifecycle_snapshot(&self) -> crate::lifecycle::LifecycleSnapshot
```

### [ServiceCommandRequest](../../../src/kernel.rs#L769)

`struct` · `private`

```rust
struct ServiceCommandRequest
```

### [unavailable_engine_service](../../../src/kernel.rs#L780)

`fn` · `private`

```rust
fn unavailable_engine_service(reason: &str) -> Value
```

### [required_root_id](../../../src/kernel.rs#L796)

`fn` · `private`

```rust
fn required_root_id(command: &ServiceCommandRequest) -> Result<&str, ApiError>
```

### [invalid_service_command](../../../src/kernel.rs#L804)

`fn` · `private`

```rust
fn invalid_service_command(message: &str) -> ApiError
```

### [engine_admin_error](../../../src/kernel.rs#L812)

`fn` · `private`

```rust
fn engine_admin_error(error: crate::engine_jsonl::EngineJsonlError, operation: &str) -> ApiError
```

### [frontend_opening_result](../../../src/kernel.rs#L817)

`fn` · `private`

```rust
fn frontend_opening_result( release: &crate::release::CoreReleaseManifest, capabilities: &[crate::availability::CapabilityAvailability], restart_eligible: bool, ) -> Value
```

### [validate_critical_extensions](../../../src/kernel.rs#L891)

`fn` · `private`

```rust
fn validate_critical_extensions(request: &Request) -> Result<(), ApiError>
```

### [validate_request_id](../../../src/kernel.rs#L902)

`fn` · `private`

```rust
fn validate_request_id(id: &str) -> Result<(), ApiError>
```

### [validate_request_shape](../../../src/kernel.rs#L913)

`fn` · `private`

```rust
fn validate_request_shape(request: &Request) -> Result<(), ApiError>
```

### [validate_cancellation](../../../src/kernel.rs#L964)

`fn` · `private`

```rust
fn validate_cancellation(request: &Request) -> Result<(), ApiError>
```

### [version_result](../../../src/kernel.rs#L982)

`fn` · `private`

```rust
fn version_result() -> Value
```

### [search_error_code](../../../src/kernel.rs#L1003)

`fn` · `private`

```rust
const fn search_error_code(status: TerminalStatus) -> ApiErrorCode
```

### [stub_result](../../../src/kernel.rs#L1020)

`fn` · `private`

```rust
fn stub_result(capability: &str, reason: &str) -> Value
```

### [stub_error](../../../src/kernel.rs#L1028)

`fn` · `private`

```rust
fn stub_error(message: &str) -> ApiError
```

### [internal_serialization_error](../../../src/kernel.rs#L1036)

`fn` · `private`

```rust
fn internal_serialization_error(error: &serde_json::Error) -> ApiError
```

### [validate_deadline](../../../src/kernel.rs#L1044)

`fn` · `private`

```rust
fn validate_deadline(request: &Request) -> Result<(), ApiError>
```

### [validate_contract](../../../src/kernel.rs#L1068)

`fn` · `private`

```rust
fn validate_contract(request: &Request) -> Result<(), ApiError>
```

### [enforce_response_budget](../../../src/kernel.rs#L1097)

`fn` · `private`

```rust
fn enforce_response_budget( response_id: String, max_response_bytes: Option<u64>, response: Response, ) -> Response
```

### [BlockingSearch](../../../src/kernel.rs#L1138)

`struct` · `private`

```rust
struct BlockingSearch
```

### [search](../../../src/kernel.rs#L1144)

`fn` · `private`

```rust
fn search( &mut self, _request: &crate::engine_contract::EngineSearchRequest, ) -> EngineSearchOutcome
```

### [live_available](../../../src/kernel.rs#L1157)

`fn` · `private`

```rust
fn live_available(&self) -> bool
```

### [BecomesUnavailable](../../../src/kernel.rs#L1162)

`struct` · `private`

```rust
struct BecomesUnavailable
```

### [search](../../../src/kernel.rs#L1167)

`fn` · `private`

```rust
fn search( &mut self, _request: &crate::engine_contract::EngineSearchRequest, ) -> EngineSearchOutcome
```

### [live_available](../../../src/kernel.rs#L1179)

`fn` · `private`

```rust
fn live_available(&self) -> bool
```

### [kernel_reports_stubs_without_fact_or_plugin_operations](../../../src/kernel.rs#L1185)

`fn` · `private`

```rust
fn kernel_reports_stubs_without_fact_or_plugin_operations()
```

### [settings_contract_uses_one_typed_optimistic_transaction](../../../src/kernel.rs#L1199)

`fn` · `private`

```rust
fn settings_contract_uses_one_typed_optimistic_transaction()
```

### [shutdown_is_terminal](../../../src/kernel.rs#L1234)

`fn` · `private`

```rust
fn shutdown_is_terminal()
```

### [frontend_bootstrap_is_one_bounded_immutable_snapshot](../../../src/kernel.rs#L1245)

`fn` · `private`

```rust
fn frontend_bootstrap_is_one_bounded_immutable_snapshot()
```

### [separately_gated_provider_absence_does_not_block_a_ready_core_projection](../../../src/kernel.rs#L1344)

`fn` · `private`

```rust
fn separately_gated_provider_absence_does_not_block_a_ready_core_projection()
```

### [attacker_controlled_envelope_fields_are_bounded_before_error_reflection](../../../src/kernel.rs#L1382)

`fn` · `private`

```rust
fn attacker_controlled_envelope_fields_are_bounded_before_error_reflection()
```

### [blocked_engine_provider_does_not_hold_core_status_or_lifecycle_state](../../../src/kernel.rs#L1409)

`fn` · `private`

```rust
fn blocked_engine_provider_does_not_hold_core_status_or_lifecycle_state()
```

### [provider_health_cache_updates_after_terminal_worker_call](../../../src/kernel.rs#L1455)

`fn` · `private`

```rust
fn provider_health_cache_updates_after_terminal_worker_call()
```

### [service_commands_bind_orchestrator_instance_and_generation](../../../src/kernel.rs#L1475)

`fn` · `private`

```rust
fn service_commands_bind_orchestrator_instance_and_generation()
```
