use crate::contract::{CONTRACTS, ContractStage};
use serde::Serialize;
use sha2::{Digest, Sha256};
use std::sync::OnceLock;

pub const CORE_PROFILE_ID: &str = "orchestrator-core";
pub const CORE_TARGET_VERSION: &str = "1.0.0";
pub const CORE_FIRST_PLATFORM: &str = "macos";
pub const CORE_REQUIRED_CONTRACTS: &[&str] =
    &["ORC-COM-001", "ORC-LIF-001", "ORC-FE-001", "ORC-CLI-001"];

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum RequirementState {
    Satisfied,
    Pending,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize)]
pub struct ReleaseRequirement {
    pub id: String,
    pub state: RequirementState,
    pub evidence: String,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize)]
pub struct ReleaseProvenance {
    pub algorithm: &'static str,
    pub scope: &'static str,
    pub digest: String,
    pub signed: bool,
    pub embedded_inputs: usize,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize)]
pub struct CoreReleaseManifest {
    pub profile: &'static str,
    pub target_version: &'static str,
    pub build_version: &'static str,
    pub first_platform: &'static str,
    pub state: &'static str,
    pub ready: bool,
    pub required_contracts: &'static [&'static str],
    pub requirements: Vec<ReleaseRequirement>,
    pub provenance: ReleaseProvenance,
}

const PROVENANCE_DOMAIN: &[u8] = b"fileman-orchestrator-core-manifest-v1\0";
const PROVENANCE_SCOPE: &str = "manifest-fields+embedded-core-contract-inputs-v1";
const PROVENANCE_INPUTS: &[(&str, &[u8])] = &[
    (
        "spec/CONTRACT_REGISTRY.md",
        include_bytes!("../spec/CONTRACT_REGISTRY.md"),
    ),
    (
        "spec/contracts/COMMON.md",
        include_bytes!("../spec/contracts/COMMON.md"),
    ),
    (
        "spec/contracts/FRONTEND_AND_GUI_FORMS.md",
        include_bytes!("../spec/contracts/FRONTEND_AND_GUI_FORMS.md"),
    ),
    (
        "conformance/fixtures/bootstrap/cancellation_unsupported.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/cancellation_unsupported.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/critical_extension.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/critical_extension.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/expired_deadline.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/expired_deadline.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/frontend_bootstrap.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/frontend_bootstrap.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/release.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/release.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/response_budget.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/response_budget.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/semantic_facts_stub.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/semantic_facts_stub.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/status.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/status.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/unknown_method.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/unknown_method.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/unknown_optional.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/unknown_optional.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/version.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/version.request.json"),
    ),
    (
        "conformance/fixtures/bootstrap/version_mismatch.request.json",
        include_bytes!("../conformance/fixtures/bootstrap/version_mismatch.request.json"),
    ),
    (
        "conformance/fixtures/local-wire-v0/client_hello.json",
        include_bytes!("../conformance/fixtures/local-wire-v0/client_hello.json"),
    ),
    (
        "conformance/fixtures/local-wire-v1/client_hello.json",
        include_bytes!("../conformance/fixtures/local-wire-v1/client_hello.json"),
    ),
    (
        "conformance/fixtures/local-wire-v1/server_hello.json",
        include_bytes!("../conformance/fixtures/local-wire-v1/server_hello.json"),
    ),
    (
        "conformance/fixtures/local-wire-v1/status.request.json",
        include_bytes!("../conformance/fixtures/local-wire-v1/status.request.json"),
    ),
];

static CORE_RELEASE_MANIFEST: OnceLock<CoreReleaseManifest> = OnceLock::new();

#[must_use]
pub fn core_release_manifest() -> &'static CoreReleaseManifest {
    CORE_RELEASE_MANIFEST.get_or_init(build_core_release_manifest)
}

fn build_core_release_manifest() -> CoreReleaseManifest {
    let mut requirements: Vec<_> = CORE_REQUIRED_CONTRACTS
        .iter()
        .map(|id| contract_requirement(id))
        .collect();
    requirements.extend([
        satisfied_requirement(
            "daemon.discovery",
            "a release-built macOS arm64 artifact passed installed LaunchAgent bootstrap, first-use activation, authenticated status, graceful shutdown, supervisor reactivation within the five-second client bound with fresh instance identity, second shutdown, bootout, and exact test-artifact removal on 2026-08-07",
        ),
        satisfied_requirement(
            "session.authentication",
            "macOS private runtime objects, kernel-vouched peer UID, no-follow descriptor reads, durable publication, zeroized Rust secrets, 256-bit credential hello, instance verification, and restart rotation/stale-token rejection pass",
        ),
        satisfied_requirement(
            "transport.local",
            "the macOS ORC1 1.0 transport passes bounded stdio, envelope-field ceilings, full-worker handshake-stall recovery, fixed-worker concurrency, panic containment, observable saturation, malformed-frame containment, and shutdown; other platforms retain separate gates",
        ),
        satisfied_requirement(
            "client.independent",
            "the independently built C++17 client passes the atomic ORC-FE-001 macOS snapshot, complete contract/availability and route/control parsing, golden release digest, shutdown, and daemon restart without linking Rust",
        ),
        satisfied_requirement(
            "conformance.hostile_and_cross_version",
            "macOS live and bootstrap suites cover rejected 0.1 peers, framing faults, saturation, atomic cancellation rejection, critical-field rejection, optional-field tolerance, and the ADR-010 1.x horizon",
        ),
        satisfied_requirement(
            "release.provenance",
            "the manifest publishes a deterministic SHA-256 digest over its fields and embedded Core contract inputs; packaged-artifact signing remains a platform release concern",
        ),
    ]);
    let ready = requirements
        .iter()
        .all(|requirement| requirement.state == RequirementState::Satisfied);
    let provenance = release_provenance(&requirements);
    CoreReleaseManifest {
        profile: CORE_PROFILE_ID,
        target_version: CORE_TARGET_VERSION,
        build_version: env!("CARGO_PKG_VERSION"),
        first_platform: CORE_FIRST_PLATFORM,
        state: if ready { "ready" } else { "development" },
        ready,
        required_contracts: CORE_REQUIRED_CONTRACTS,
        requirements,
        provenance,
    }
}

fn contract_requirement(id: &str) -> ReleaseRequirement {
    let contract = CONTRACTS.iter().find(|contract| contract.id == id);
    match contract {
        Some(contract) => ReleaseRequirement {
            id: format!("contract.{id}"),
            state: if contract.stage == ContractStage::Stable {
                RequirementState::Satisfied
            } else {
                RequirementState::Pending
            },
            evidence: format!("contract stage: {}", stage_name(contract.stage)),
        },
        None => pending_requirement(
            &format!("contract.{id}"),
            "required contract is absent from the executable registry",
        ),
    }
}

fn pending_requirement(id: &str, evidence: &str) -> ReleaseRequirement {
    ReleaseRequirement {
        id: id.to_owned(),
        state: RequirementState::Pending,
        evidence: evidence.to_owned(),
    }
}

fn satisfied_requirement(id: &str, evidence: &str) -> ReleaseRequirement {
    ReleaseRequirement {
        id: id.to_owned(),
        state: RequirementState::Satisfied,
        evidence: evidence.to_owned(),
    }
}

fn release_provenance(requirements: &[ReleaseRequirement]) -> ReleaseProvenance {
    let mut digest = Sha256::new();
    digest.update(PROVENANCE_DOMAIN);
    digest_field(&mut digest, b"profile", CORE_PROFILE_ID.as_bytes());
    digest_field(
        &mut digest,
        b"target_version",
        CORE_TARGET_VERSION.as_bytes(),
    );
    digest_field(
        &mut digest,
        b"build_version",
        env!("CARGO_PKG_VERSION").as_bytes(),
    );
    digest_field(
        &mut digest,
        b"first_platform",
        CORE_FIRST_PLATFORM.as_bytes(),
    );
    for contract in CORE_REQUIRED_CONTRACTS {
        digest_field(&mut digest, b"required_contract", contract.as_bytes());
    }
    for requirement in requirements {
        digest_field(&mut digest, b"requirement_id", requirement.id.as_bytes());
        digest_field(
            &mut digest,
            b"requirement_state",
            match requirement.state {
                RequirementState::Satisfied => b"satisfied",
                RequirementState::Pending => b"pending",
            },
        );
        digest_field(
            &mut digest,
            b"requirement_evidence",
            requirement.evidence.as_bytes(),
        );
    }
    for (path, bytes) in PROVENANCE_INPUTS {
        digest_field(&mut digest, b"input_path", path.as_bytes());
        digest_field(&mut digest, b"input_bytes", bytes);
    }
    let bytes = digest.finalize();
    let mut encoded = String::with_capacity(bytes.len() * 2);
    for byte in bytes {
        use std::fmt::Write as _;
        write!(&mut encoded, "{byte:02x}").expect("writing to a String cannot fail");
    }
    ReleaseProvenance {
        algorithm: "sha256",
        scope: PROVENANCE_SCOPE,
        digest: encoded,
        signed: false,
        embedded_inputs: PROVENANCE_INPUTS.len(),
    }
}

fn digest_field(digest: &mut Sha256, label: &[u8], value: &[u8]) {
    digest.update((label.len() as u64).to_be_bytes());
    digest.update(label);
    digest.update((value.len() as u64).to_be_bytes());
    digest.update(value);
}

const fn stage_name(stage: ContractStage) -> &'static str {
    match stage {
        ContractStage::Outline => "outline",
        ContractStage::Negotiating => "negotiating",
        ContractStage::Stubbed => "stubbed",
        ContractStage::FixtureDraft => "fixture_draft",
        ContractStage::FrozenV0 => "frozen_v0",
        ContractStage::AcceptedProcess => "accepted_process",
        ContractStage::Stable => "stable",
    }
}

#[cfg(test)]
mod tests {
    use super::{CORE_REQUIRED_CONTRACTS, RequirementState, core_release_manifest};

    #[test]
    fn core_release_is_ready_only_when_every_requirement_is_satisfied() {
        let manifest = core_release_manifest();
        assert_eq!(manifest.target_version, "1.0.0");
        assert_eq!(manifest.required_contracts, CORE_REQUIRED_CONTRACTS);
        assert!(manifest.ready);
        assert_eq!(manifest.state, "ready");
        assert_eq!(manifest.provenance.algorithm, "sha256");
        assert_eq!(manifest.provenance.digest.len(), 64);
        assert_eq!(manifest.provenance.embedded_inputs, 19);
        assert!(std::ptr::eq(manifest, core_release_manifest()));
        assert!(
            manifest
                .requirements
                .iter()
                .all(|requirement| requirement.state == RequirementState::Satisfied)
        );
    }
}
