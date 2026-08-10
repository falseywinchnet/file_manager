# src/release.rs

Status: **OBSERVED ready macOS Core 1.0 manifest**.

Builds the deterministic Core release manifest and domain-separated SHA-256 provenance digest over named manifest and contract inputs.

Source: [src/release.rs](../../../src/release.rs)

## Responsibilities

- List every Core readiness requirement and evidence locator.
- Report ready only when all requirements are satisfied.
- Pin contract stages and first platform.
- Publish digest scope while keeping signed false explicit.

## Boundary

- The digest is content identity, not package signing or binary attestation.

## Contract projections

- ORC-LIF-001 release projection

## Source inventory

### [CORE_PROFILE_ID](../../../src/release.rs#L6)

`const` · `pub`

```rust
pub const CORE_PROFILE_ID: &str = "orchestrator-core";
```

### [CORE_TARGET_VERSION](../../../src/release.rs#L7)

`const` · `pub`

```rust
pub const CORE_TARGET_VERSION: &str = "1.0.0";
```

### [CORE_FIRST_PLATFORM](../../../src/release.rs#L8)

`const` · `pub`

```rust
pub const CORE_FIRST_PLATFORM: &str = "macos";
```

### [CORE_REQUIRED_CONTRACTS](../../../src/release.rs#L9)

`const` · `pub`

```rust
pub const CORE_REQUIRED_CONTRACTS: &[&str] = &["ORC-COM-001", "ORC-LIF-001", "ORC-FE-001", "ORC-CLI-001"];
```

### [RequirementState](../../../src/release.rs#L14)

`enum` · `pub`

```rust
pub enum RequirementState
```

### [ReleaseRequirement](../../../src/release.rs#L20)

`struct` · `pub`

```rust
pub struct ReleaseRequirement
```

### [ReleaseProvenance](../../../src/release.rs#L27)

`struct` · `pub`

```rust
pub struct ReleaseProvenance
```

### [CoreReleaseManifest](../../../src/release.rs#L36)

`struct` · `pub`

```rust
pub struct CoreReleaseManifest
```

### [PROVENANCE_DOMAIN](../../../src/release.rs#L48)

`const` · `private`

```rust
const PROVENANCE_DOMAIN: &[u8] = b"fileman-orchestrator-core-manifest-v1\0";
```

### [PROVENANCE_SCOPE](../../../src/release.rs#L49)

`const` · `private`

```rust
const PROVENANCE_SCOPE: &str = "manifest-fields+embedded-core-contract-inputs-v1";
```

### [PROVENANCE_INPUTS](../../../src/release.rs#L50)

`const` · `private`

```rust
const PROVENANCE_INPUTS: &[(&str, &[u8])] = &[ ( "spec/CONTRACT_REGISTRY.md", include_bytes!("../spec/CONTRACT_REGISTRY.md"), ), ( "spec/contracts/COMMON.md", include_bytes!("../spec/contracts/COMMON.md"), ), ( "spec/contracts/FRONTEND_AND_GUI_FORMS.md", include_bytes!("../spec/contracts/FRONTEND_AND_GUI_FORMS.md"), ), ( "conformance/fixtures/bootstrap/cancellation_unsupported.request.json", include_bytes!("../conformance/fixtures/bootstrap/cancellation_unsupported.request.json"), ), ( "conformance/fixtures/bootstrap/critical_extension.request.json", include_bytes!("../conformance/fixtures/bootstrap/critical_extension.request.json"), ), ( "conformance/fixtures/bootstrap/expired_deadline.request.json", include_bytes!("../conformance/fixtures/bootstrap/expired_deadline.request.json"), ), ( "conformance/fixtures/bootstrap/frontend_bootstrap.request.json", include_bytes!("../conformance/fixtures/bootstrap/frontend_bootstrap.request.json"), ), ( "conformance/fixtures/bootstrap/release.request.json", include_bytes!("../conformance/fixtures/bootstrap/release.request.json"), ), ( "conformance/fixtures/bootstrap/response_budget.request.json", include_bytes!("../conformance/fixtures/bootstrap/response_budget.request.json"), ), ( "conformance/fixtures/bootstrap/semantic_facts_stub.request.json", include_bytes!("../conformance/fixtures/bootstrap/semantic_facts_stub.request.json"), ), ( "conformance/fixtures/bootstrap/status.request.json", include_bytes!("../conformance/fixtures/bootstrap/status.request.json"), ), ( "conformance/fixtures/bootstrap/unknown_method.request.json", include_bytes!("../conformance/fixtures/bootstrap/unknown_method.request.json"), ), ( "conformance/fixtures/bootstrap/unknown_optional.request.json", include_bytes!("../conformance/fixtures/bootstrap/unknown_optional.request.json"), ), ( "conformance/fixtures/bootstrap/version.request.json", include_bytes!("../conformance/fixtures/bootstrap/version.request.json"), ), ( "conformance/fixtures/bootstrap/version_mismatch.request.json", include_bytes!("../conformance/fixtures/bootstrap/version_mismatch.request.json"), ), ( "conformance/fixtures/local-wire-v0/client_hello.json", include_bytes!("../conformance/fixtures/local-wire-v0/client_hello.json"), ), ( "conformance/fixtures/local-wire-v1/client_hello.json", include_bytes!("../conformance/fixtures/local-wire-v1/client_hello.json"), ), ( "conformance/fixtures/local-wire-v1/server_hello.json", include_bytes!("../conformance/fixtures/local-wire-v1/server_hello.json"), ), ( "conformance/fixtures/local-wire-v1/status.request.json", include_bytes!("../conformance/fixtures/local-wire-v1/status.request.json"), ), ];
```

### [CORE_RELEASE_MANIFEST](../../../src/release.rs#L129)

`static` · `private`

```rust
static CORE_RELEASE_MANIFEST: OnceLock<CoreReleaseManifest> = OnceLock::new();
```

### [core_release_manifest](../../../src/release.rs#L132)

`fn` · `pub`

```rust
pub fn core_release_manifest() -> &'static CoreReleaseManifest
```

### [build_core_release_manifest](../../../src/release.rs#L136)

`fn` · `private`

```rust
fn build_core_release_manifest() -> CoreReleaseManifest
```

### [contract_requirement](../../../src/release.rs#L184)

`fn` · `private`

```rust
fn contract_requirement(id: &str) -> ReleaseRequirement
```

### [pending_requirement](../../../src/release.rs#L203)

`fn` · `private`

```rust
fn pending_requirement(id: &str, evidence: &str) -> ReleaseRequirement
```

### [satisfied_requirement](../../../src/release.rs#L211)

`fn` · `private`

```rust
fn satisfied_requirement(id: &str, evidence: &str) -> ReleaseRequirement
```

### [release_provenance](../../../src/release.rs#L219)

`fn` · `private`

```rust
fn release_provenance(requirements: &[ReleaseRequirement]) -> ReleaseProvenance
```

### [digest_field](../../../src/release.rs#L276)

`fn` · `private`

```rust
fn digest_field(digest: &mut Sha256, label: &[u8], value: &[u8])
```

### [stage_name](../../../src/release.rs#L283)

`fn` · `private`

```rust
const fn stage_name(stage: ContractStage) -> &'static str
```

### [core_release_is_ready_only_when_every_requirement_is_satisfied](../../../src/release.rs#L300)

`fn` · `private`

```rust
fn core_release_is_ready_only_when_every_requirement_is_satisfied()
```
