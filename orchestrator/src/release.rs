use crate::contract::{CONTRACTS, ContractStage};
use serde::Serialize;

pub const CORE_PROFILE_ID: &str = "orchestrator-core";
pub const CORE_TARGET_VERSION: &str = "1.0.0";
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
pub struct CoreReleaseManifest {
    pub profile: &'static str,
    pub target_version: &'static str,
    pub build_version: &'static str,
    pub state: &'static str,
    pub ready: bool,
    pub required_contracts: &'static [&'static str],
    pub requirements: Vec<ReleaseRequirement>,
}

#[must_use]
pub fn core_release_manifest() -> CoreReleaseManifest {
    let mut requirements: Vec<_> = CORE_REQUIRED_CONTRACTS
        .iter()
        .map(|id| contract_requirement(id))
        .collect();
    requirements.extend([
        pending_requirement(
            "daemon.discovery",
            "explicit private Unix discovery is implemented; launchd/login-session location and stale/restart promotion fixtures remain pending",
        ),
        pending_requirement(
            "session.authentication",
            "256-bit private credential hello is implemented on Unix; independent-client, rotation, and platform peer-identity fixtures remain pending",
        ),
        pending_requirement(
            "transport.local",
            "bounded ORC1 framing and Unix sockets are implemented; concurrent backpressure, hostile corpus, and Windows named pipes remain pending",
        ),
        pending_requirement(
            "client.independent",
            "an independent C++-suitable conformance client is not implemented",
        ),
        pending_requirement(
            "conformance.hostile_and_cross_version",
            "hostile framing and cross-version suites are incomplete",
        ),
        pending_requirement(
            "release.provenance",
            "the release manifest has no final digest or signature",
        ),
    ]);
    let ready = requirements
        .iter()
        .all(|requirement| requirement.state == RequirementState::Satisfied);
    CoreReleaseManifest {
        profile: CORE_PROFILE_ID,
        target_version: CORE_TARGET_VERSION,
        build_version: env!("CARGO_PKG_VERSION"),
        state: if ready { "ready" } else { "development" },
        ready,
        required_contracts: CORE_REQUIRED_CONTRACTS,
        requirements,
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
    fn core_release_cannot_claim_readiness_while_requirements_are_pending() {
        let manifest = core_release_manifest();
        assert_eq!(manifest.target_version, "1.0.0");
        assert_eq!(manifest.required_contracts, CORE_REQUIRED_CONTRACTS);
        assert!(!manifest.ready);
        assert_eq!(manifest.state, "development");
        assert!(
            manifest
                .requirements
                .iter()
                .any(|requirement| requirement.state == RequirementState::Pending)
        );
    }
}
