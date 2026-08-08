use serde::Serialize;
use std::collections::BTreeSet;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum ContractStage {
    Outline,
    Negotiating,
    Stubbed,
    FixtureDraft,
    FrozenV0,
    AcceptedProcess,
    Stable,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
pub struct ContractDescriptor {
    pub id: &'static str,
    pub name: &'static str,
    pub provider: &'static str,
    pub stage: ContractStage,
    pub executable: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SupportedContract {
    pub id: &'static str,
    pub major: u16,
    pub minor: u16,
}

#[must_use]
pub fn supported_contract_for_method(method: &str) -> Option<SupportedContract> {
    match method {
        "orchestrator.version"
        | "orchestrator.release"
        | "orchestrator.status"
        | "orchestrator.shutdown" => Some(SupportedContract {
            id: "ORC-LIF-001",
            major: 1,
            minor: 0,
        }),
        "orchestrator.frontend.bootstrap" | "orchestrator.search" => Some(SupportedContract {
            id: "ORC-FE-001",
            major: 1,
            minor: 0,
        }),
        "orchestrator.contracts.list" | "orchestrator.availability.list" => {
            Some(SupportedContract {
                id: "ORC-COM-001",
                major: 1,
                minor: 0,
            })
        }
        _ => None,
    }
}

pub const CONTRACTS: &[ContractDescriptor] = &[
    ContractDescriptor {
        id: "ORC-COM-001",
        name: "Common identities, terminal status, envelopes, and provenance",
        provider: "orchestrator",
        stage: ContractStage::Stable,
        executable: true,
    },
    ContractDescriptor {
        id: "ORC-LIF-001",
        name: "Service lifecycle, status, shutdown, and restart",
        provider: "orchestrator",
        stage: ContractStage::Stable,
        executable: true,
    },
    ContractDescriptor {
        id: "ORC-NEG-001",
        name: "Project-local interface proposal, reply, and reconciliation",
        provider: "orchestrator-spec",
        stage: ContractStage::AcceptedProcess,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-ENG-001",
        name: "Exact/file query, inspect, evidence, and pagination",
        provider: "engine",
        stage: ContractStage::FrozenV0,
        executable: true,
    },
    ContractDescriptor {
        id: "ORC-ENG-002",
        name: "Root policy, scan, integrity, and rebuild administration",
        provider: "engine",
        stage: ContractStage::FrozenV0,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-ENG-003",
        name: "Engine status and currentness snapshot",
        provider: "engine",
        stage: ContractStage::FrozenV0,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-ENG-004",
        name: "Catalogue-independent bounded live filesystem search",
        provider: "engine",
        stage: ContractStage::FixtureDraft,
        executable: true,
    },
    ContractDescriptor {
        id: "ORC-KOL-001",
        name: "Kolmogrov configuration, candidates, and evidence",
        provider: "engine",
        stage: ContractStage::Negotiating,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-GUI-001",
        name: "GUI.Forms consumption manifest, C ABI, and lifecycle",
        provider: "gui-forms",
        stage: ContractStage::Negotiating,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-FE-001",
        name: "File Manager bootstrap session, availability, and degraded fallback",
        provider: "orchestrator",
        stage: ContractStage::Stable,
        executable: true,
    },
    ContractDescriptor {
        id: "ORC-PLG-001",
        name: "Plugin package, grant, and lifecycle",
        provider: "orchestrator",
        stage: ContractStage::Stubbed,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-PLG-002",
        name: "Sandboxed plugin worker and capability protocol",
        provider: "orchestrator",
        stage: ContractStage::Stubbed,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-PLG-003",
        name: "Preview and thumbnail providers",
        provider: "plugin-workers",
        stage: ContractStage::Stubbed,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-PLG-004",
        name: "Search, virtual-system, and future plugin-AI exchange",
        provider: "plugin-workers",
        stage: ContractStage::Stubbed,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-HIV-001",
        name: "Provider schema and derived-generation deposit",
        provider: "orchestrator",
        stage: ContractStage::Stubbed,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-HIV-002",
        name: "Semantic fact operations",
        provider: "orchestrator",
        stage: ContractStage::Stubbed,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-HIV-003",
        name: "Hive lifecycle, quota, migration, and export",
        provider: "orchestrator",
        stage: ContractStage::Stubbed,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-HND-001",
        name: "Handler and type-association resolution",
        provider: "orchestrator",
        stage: ContractStage::Outline,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-CMD-001",
        name: "Declarative command registration and invocation",
        provider: "orchestrator",
        stage: ContractStage::Outline,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-SET-001",
        name: "Settings schema and value transactions",
        provider: "orchestrator",
        stage: ContractStage::Outline,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-CLI-001",
        name: "Human and structured local CLI",
        provider: "orchestrator",
        stage: ContractStage::Stable,
        executable: true,
    },
    ContractDescriptor {
        id: "ORC-INT-001",
        name: "Platform integration plan and status",
        provider: "platform-adapters",
        stage: ContractStage::Outline,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-FED-001",
        name: "Optional machine catalogue and federation",
        provider: "future-adapters",
        stage: ContractStage::Outline,
        executable: false,
    },
    ContractDescriptor {
        id: "ORC-AUD-001",
        name: "Local audit, provenance inspection, and redacted export",
        provider: "orchestrator",
        stage: ContractStage::Outline,
        executable: false,
    },
];

#[must_use]
pub fn identifiers_are_unique() -> bool {
    let unique: BTreeSet<_> = CONTRACTS.iter().map(|contract| contract.id).collect();
    unique.len() == CONTRACTS.len()
}

#[cfg(test)]
mod tests {
    use super::{CONTRACTS, identifiers_are_unique};

    #[test]
    fn contract_identifiers_are_unique() {
        assert!(identifiers_are_unique());
        assert!(
            CONTRACTS
                .iter()
                .all(|contract| contract.id.starts_with("ORC-"))
        );
    }
}
