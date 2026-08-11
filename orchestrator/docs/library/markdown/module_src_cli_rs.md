# src/cli.rs

Status: **OBSERVED refactored command and presentation layer**.

Parses bounded command shapes, selects a service host or semantic method, and renders human or structured responses.

Source: [src/cli.rs](../../../src/cli.rs)

## Responsibilities

- Validate command-specific option combinations.
- Keep the four development Engine options atomic.
- Map CLI names to canonical Orchestrator methods.
- Render the common Response projection.

## Boundary

- Does not host sockets or workers.
- Does not implement semantic operations.
- Does not treat Engine child options as installed discovery.

## Contract projections

- ORC-CLI-001
- ORC-LIF-001
- ORC-FE-001

## Source inventory

### [run](../../../src/cli.rs#L20)

`fn` · `pub`

```rust
pub fn run(arguments: impl IntoIterator<Item = String>) -> Result<(), String>
```

### [remove_engine_options](../../../src/cli.rs#L120)

`fn` · `private`

```rust
fn remove_engine_options( arguments: &mut Vec<String>, ) -> Result<Option<EngineProviderConfig>, String>
```

### [resolve_runtime_directory](../../../src/cli.rs#L155)

`fn` · `private`

```rust
fn resolve_runtime_directory(explicit: Option<String>) -> Result<PathBuf, String>
```

### [resolve_settings_directory](../../../src/cli.rs#L163)

`fn` · `private`

```rust
fn resolve_settings_directory(explicit: Option<String>) -> Result<PathBuf, String>
```

### [resolve_settings_directory](../../../src/cli.rs#L170)

`fn` · `private`

```rust
fn resolve_settings_directory(explicit: Option<String>) -> Result<PathBuf, String>
```

### [resolve_runtime_directory](../../../src/cli.rs#L177)

`fn` · `private`

```rust
fn resolve_runtime_directory(explicit: Option<String>) -> Result<PathBuf, String>
```

### [remove_flag](../../../src/cli.rs#L183)

`fn` · `private`

```rust
fn remove_flag(arguments: &mut Vec<String>, flag: &str) -> bool
```

### [remove_option](../../../src/cli.rs#L192)

`fn` · `private`

```rust
fn remove_option(arguments: &mut Vec<String>, option: &str) -> Result<Option<String>, String>
```

### [method_for_command](../../../src/cli.rs#L207)

`fn` · `private`

```rust
fn method_for_command(command: &str) -> Option<&'static str>
```

### [local_request_for_arguments](../../../src/cli.rs#L225)

`fn` · `private`

```rust
fn local_request_for_arguments(arguments: &[String]) -> Result<Request, String>
```

### [render_response](../../../src/cli.rs#L314)

`fn` · `private`

```rust
fn render_response(command: &str, response: &Response, json_output: bool) -> Result<(), String>
```

### [print_human](../../../src/cli.rs#L329)

`fn` · `private`

```rust
fn print_human(command: &str, response: &Response) -> Result<(), String>
```

### [print_contracts](../../../src/cli.rs#L409)

`fn` · `private`

```rust
fn print_contracts(value: &Value) -> Result<(), String>
```

### [print_availability](../../../src/cli.rs#L424)

`fn` · `private`

```rust
fn print_availability(value: &Value) -> Result<(), String>
```

### [string_field](../../../src/cli.rs#L439)

`fn` · `private`

```rust
fn string_field<'a>(value: &'a Value, field: &str) -> Result<&'a str, String>
```

### [integer_field](../../../src/cli.rs#L445)

`fn` · `private`

```rust
fn integer_field(value: &Value, field: &str) -> Result<u64, String>
```

### [boolean_field](../../../src/cli.rs#L451)

`fn` · `private`

```rust
fn boolean_field(value: &Value, field: &str) -> Result<bool, String>
```

### [print_help](../../../src/cli.rs#L457)

`fn` · `private`

```rust
fn print_help()
```

### [command_projection_is_explicit](../../../src/cli.rs#L489)

`fn` · `private`

```rust
fn command_projection_is_explicit()
```

### [engine_provider_options_are_atomic](../../../src/cli.rs#L499)

`fn` · `private`

```rust
fn engine_provider_options_are_atomic()
```

### [settings_mutation_cli_builds_the_canonical_transaction](../../../src/cli.rs#L522)

`fn` · `private`

```rust
fn settings_mutation_cli_builds_the_canonical_transaction()
```

### [service_command_cli_preserves_identity_and_root](../../../src/cli.rs#L537)

`fn` · `private`

```rust
fn service_command_cli_preserves_identity_and_root()
```
