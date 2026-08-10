# src/kernel.rs

Status: **OBSERVED Core 1.0 semantic dispatcher**.

Validates transport-neutral requests, dispatches Core methods, projects immutable status/bootstrap state, brokers search, and owns deterministic shutdown state.

Source: [src/kernel.rs](../../../src/kernel.rs)

## Responsibilities

- Validate attacker-controlled envelope fields before reflection.
- Enforce critical-extension, cancellation, deadline, contract, and response-budget law.
- Build status and frontend bootstrap snapshots from canonical catalogues.
- Route unified search through the typed provider port.
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

### [Kernel](../../../src/kernel.rs#L18)

`struct` · `pub`

```rust
pub struct Kernel
```

### [default](../../../src/kernel.rs#L26)

`fn` · `private`

```rust
fn default() -> Self
```

### [live_search_available](../../../src/kernel.rs#L32)

`fn` · `private`

```rust
fn live_search_available(&self) -> bool
```

### [new](../../../src/kernel.rs#L39)

`fn` · `pub`

```rust
pub fn new() -> Self
```

### [for_local_daemon](../../../src/kernel.rs#L49)

`fn` · `pub`

```rust
pub fn for_local_daemon() -> Self
```

### [for_supervised_daemon](../../../src/kernel.rs#L59)

`fn` · `pub`

```rust
pub fn for_supervised_daemon() -> Self
```

### [with_engine_search](../../../src/kernel.rs#L69)

`fn` · `pub`

```rust
pub fn with_engine_search(mut self, search: impl UnifiedEngineSearch + 'static) -> Self
```

### [runtime_health](../../../src/kernel.rs#L75)

`fn` · `pub`

```rust
pub fn runtime_health(&self) -> Arc<RuntimeHealth>
```

### [handle](../../../src/kernel.rs#L80)

`fn` · `pub`

```rust
pub fn handle(&mut self, request: Request) -> Response
```

### [is_stopped](../../../src/kernel.rs#L166)

`fn` · `pub`

```rust
pub fn is_stopped(&self) -> bool
```

### [lifecycle_generation](../../../src/kernel.rs#L171)

`fn` · `pub`

```rust
pub fn lifecycle_generation(&self) -> u64
```

### [status_result](../../../src/kernel.rs#L175)

`fn` · `private`

```rust
fn status_result(&self) -> Value
```

### [status_result_with](../../../src/kernel.rs#L180)

`fn` · `private`

```rust
fn status_result_with(&self, release: &crate::release::CoreReleaseManifest) -> Value
```

### [frontend_bootstrap_result](../../../src/kernel.rs#L214)

`fn` · `private`

```rust
fn frontend_bootstrap_result(&self) -> Value
```

### [availability_result](../../../src/kernel.rs#L259)

`fn` · `private`

```rust
fn availability_result(&self) -> Value
```

### [availability_count](../../../src/kernel.rs#L284)

`fn` · `private`

```rust
fn availability_count(&self, state: AvailabilityState) -> usize
```

### [search_response](../../../src/kernel.rs#L302)

`fn` · `private`

```rust
fn search_response(&mut self, id: String, params: Value) -> Response
```

### [shutdown_result](../../../src/kernel.rs#L374)

`fn` · `private`

```rust
fn shutdown_result(&mut self) -> Result<Value, ApiError>
```

### [frontend_opening_result](../../../src/kernel.rs#L389)

`fn` · `private`

```rust
fn frontend_opening_result( release: &crate::release::CoreReleaseManifest, capabilities: &[crate::availability::CapabilityAvailability], restart_eligible: bool, ) -> Value
```

### [validate_critical_extensions](../../../src/kernel.rs#L463)

`fn` · `private`

```rust
fn validate_critical_extensions(request: &Request) -> Result<(), ApiError>
```

### [validate_request_id](../../../src/kernel.rs#L474)

`fn` · `private`

```rust
fn validate_request_id(id: &str) -> Result<(), ApiError>
```

### [validate_request_shape](../../../src/kernel.rs#L485)

`fn` · `private`

```rust
fn validate_request_shape(request: &Request) -> Result<(), ApiError>
```

### [validate_cancellation](../../../src/kernel.rs#L536)

`fn` · `private`

```rust
fn validate_cancellation(request: &Request) -> Result<(), ApiError>
```

### [version_result](../../../src/kernel.rs#L554)

`fn` · `private`

```rust
fn version_result() -> Value
```

### [search_error_code](../../../src/kernel.rs#L575)

`fn` · `private`

```rust
const fn search_error_code(status: TerminalStatus) -> ApiErrorCode
```

### [stub_result](../../../src/kernel.rs#L592)

`fn` · `private`

```rust
fn stub_result(capability: &str, reason: &str) -> Value
```

### [stub_error](../../../src/kernel.rs#L600)

`fn` · `private`

```rust
fn stub_error(message: &str) -> ApiError
```

### [internal_serialization_error](../../../src/kernel.rs#L608)

`fn` · `private`

```rust
fn internal_serialization_error(error: &serde_json::Error) -> ApiError
```

### [validate_deadline](../../../src/kernel.rs#L616)

`fn` · `private`

```rust
fn validate_deadline(request: &Request) -> Result<(), ApiError>
```

### [validate_contract](../../../src/kernel.rs#L640)

`fn` · `private`

```rust
fn validate_contract(request: &Request) -> Result<(), ApiError>
```

### [enforce_response_budget](../../../src/kernel.rs#L669)

`fn` · `private`

```rust
fn enforce_response_budget( response_id: String, max_response_bytes: Option<u64>, response: Response, ) -> Response
```

### [kernel_reports_stubs_without_fact_or_plugin_operations](../../../src/kernel.rs#L706)

`fn` · `private`

```rust
fn kernel_reports_stubs_without_fact_or_plugin_operations()
```

### [shutdown_is_terminal](../../../src/kernel.rs#L720)

`fn` · `private`

```rust
fn shutdown_is_terminal()
```

### [frontend_bootstrap_is_one_bounded_immutable_snapshot](../../../src/kernel.rs#L731)

`fn` · `private`

```rust
fn frontend_bootstrap_is_one_bounded_immutable_snapshot()
```

### [separately_gated_provider_absence_does_not_block_a_ready_core_projection](../../../src/kernel.rs#L819)

`fn` · `private`

```rust
fn separately_gated_provider_absence_does_not_block_a_ready_core_projection()
```

### [attacker_controlled_envelope_fields_are_bounded_before_error_reflection](../../../src/kernel.rs#L857)

`fn` · `private`

```rust
fn attacker_controlled_envelope_fields_are_bounded_before_error_reflection()
```
