# src/service/launchd.rs

Status: **OBSERVED ADR-011 macOS adapter**.

Adopts the named launchd listener, hosts a supervised kernel, and prints the pure non-installing LaunchAgent definition.

Source: [src/service/launchd.rs](../../../src/service/launchd.rs)

## Responsibilities

- Adopt fd://orchestrator-control through service-binding.
- Preserve supervisor socket ownership.
- Select the supervised restart projection.
- Escape paths in generated XML.
- Retain the accepted one-second throttle and 0600 socket mode.

## Boundary

- Does not install, load, unload, or remove a LaunchAgent.
- Does not admit the development Engine child.

## Contract projections

- ORC-LIF-001 macOS activation
- orchestrator.local 1.0

## Source inventory

### [serve_launchd](../../../src/service/launchd.rs#L18)

`fn` · `pub(crate)`

```rust
pub(crate) fn serve_launchd( runtime_directory: &Path, settings_directory: &Path, ) -> Result<(), String>
```

### [print_launchd_plist](../../../src/service/launchd.rs#L68)

`fn` · `pub(crate)`

```rust
pub(crate) fn print_launchd_plist( runtime_directory: &Path, settings_directory: &Path, ) -> Result<(), String>
```

### [launchd_plist_document](../../../src/service/launchd.rs#L99)

`fn` · `private`

```rust
fn launchd_plist_document( executable: &str, runtime_directory: &str, socket: &str, settings_directory: &str, ) -> String
```

### [xml_escape](../../../src/service/launchd.rs#L140)

`fn` · `private`

```rust
fn xml_escape(value: &str) -> String
```

### [plist_preserves_the_accepted_activation_contract](../../../src/service/launchd.rs#L154)

`fn` · `private`

```rust
fn plist_preserves_the_accepted_activation_contract()
```
