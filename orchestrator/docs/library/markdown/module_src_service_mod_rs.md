# src/service/mod.rs

Status: **OBSERVED service-host composition boundary**.

Collects platform-independent stdio, Unix local-daemon, and macOS launchd hosts behind the CLI.

Source: [src/service/mod.rs](../../../src/service/mod.rs)

## Responsibilities

- Name executable service projections.
- Carry the complete development Engine provider configuration.
- Return explicit unsupported-platform errors.

## Boundary

- The module is crate-private.
- It does not define semantic contract meaning.

## Contract projections

- ORC-CLI-001 host selection
- orchestrator.local 1.0 projection

## Source inventory

### [EngineProviderConfig](../../../src/service/mod.rs#L24)

`struct` · `pub(crate)`

```rust
pub(crate) struct EngineProviderConfig
```

### [serve_local](../../../src/service/mod.rs#L35)

`fn` · `pub(crate)`

```rust
pub(crate) fn serve_local( _runtime_directory: &std::path::Path, _engine_options: Option<EngineProviderConfig>, _settings_directory: Option<&std::path::Path>, ) -> Result<(), String>
```

### [call_local](../../../src/service/mod.rs#L44)

`fn` · `pub(crate)`

```rust
pub(crate) fn call_local( _runtime_directory: &std::path::Path, _request: crate::Request, ) -> Result<crate::Response, String>
```

### [serve_launchd](../../../src/service/mod.rs#L55)

`fn` · `pub(crate)`

```rust
pub(crate) fn serve_launchd( _runtime_directory: &std::path::Path, _settings_directory: &std::path::Path, ) -> Result<(), String>
```

### [print_launchd_plist](../../../src/service/mod.rs#L63)

`fn` · `pub(crate)`

```rust
pub(crate) fn print_launchd_plist( _runtime_directory: &std::path::Path, _settings_directory: &std::path::Path, ) -> Result<(), String>
```
