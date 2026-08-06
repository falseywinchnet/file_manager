use serde::Serialize;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum AvailabilityState {
    Available,
    Degraded,
    Negotiating,
    Unavailable,
    Deferred,
    Stubbed,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
pub struct CapabilityAvailability {
    pub id: &'static str,
    pub provider: &'static str,
    pub required: bool,
    pub state: AvailabilityState,
    pub reason: &'static str,
}

pub const CAPABILITIES: &[CapabilityAvailability] = &[
    CapabilityAvailability {
        id: "orchestrator.lifecycle",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Available,
        reason: "bootstrap lifecycle is executable",
    },
    CapabilityAvailability {
        id: "orchestrator.cli",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Available,
        reason: "human, JSON, and JSONL stdio projections are executable",
    },
    CapabilityAvailability {
        id: "orchestrator.contracts",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Available,
        reason: "bootstrap registry projection is executable",
    },
    CapabilityAvailability {
        id: "orchestrator.availability",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Available,
        reason: "required and available states are separately reported",
    },
    CapabilityAvailability {
        id: "orchestrator.release_readiness",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Available,
        reason: "Core 1.0 target and pending requirements are executable projections",
    },
    CapabilityAvailability {
        id: "orchestrator.transport.local_unix",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Available,
        reason: "macOS ORC1 1.0 framing, fixed-worker sessions, full-worker stall recovery, bounded stdio, panic containment, saturation, and hostile containment pass the first-platform gate",
    },
    CapabilityAvailability {
        id: "orchestrator.discovery.macos_launchd",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Degraded,
        reason: "macOS launchd adoption, stable default, and restart-safe activation retry are executable; installed LaunchAgent lifecycle evidence remains open",
    },
    CapabilityAvailability {
        id: "orchestrator.session.credential_auth",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Available,
        reason: "macOS private runtime ownership, kernel peer-UID verification, no-follow durable publication, zeroized Rust credentials, OS-random rotation, stale-token rejection, and instance verification pass",
    },
    CapabilityAvailability {
        id: "orchestrator.client.cpp_conformance",
        provider: "orchestrator-conformance",
        required: true,
        state: AvailabilityState::Available,
        reason: "the independent C++17 client passes macOS 1.0 bootstrap, golden digest, shutdown, and restart",
    },
    CapabilityAvailability {
        id: "engine.contract.semantic_v0",
        provider: "orchestrator-spec+engine",
        required: true,
        state: AvailabilityState::Available,
        reason: "ORC-ENG-001/002 semantic v0 is frozen for experimental implementation",
    },
    CapabilityAvailability {
        id: "engine.query.cached_exact",
        provider: "engine",
        required: true,
        state: AvailabilityState::Degraded,
        reason: "the Rust development JSONL adapter passes a real Go Engine query probe; installed discovery and authenticated transport remain absent",
    },
    CapabilityAvailability {
        id: "engine.adapter.development_jsonl",
        provider: "orchestrator",
        required: false,
        state: AvailabilityState::Available,
        reason: "bounded correlated version, status, exact query, and shutdown calls pass against a separately built Go Engine process",
    },
    CapabilityAvailability {
        id: "engine.query.manual_reconcile",
        provider: "engine",
        required: false,
        state: AvailabilityState::Unavailable,
        reason: "semantic operation is frozen; authenticated administrative adapter is not connected",
    },
    CapabilityAvailability {
        id: "engine.query.catalogue_free_fallback",
        provider: "engine",
        required: true,
        state: AvailabilityState::Negotiating,
        reason: "ADR-008 requires ORC-ENG-004; Engine implementation and conformance evidence are in progress",
    },
    CapabilityAvailability {
        id: "engine.background.currentness",
        provider: "engine-platform-adapters",
        required: false,
        state: AvailabilityState::Degraded,
        reason: "macOS and Windows adapters are experimental and declare incomplete exact-current coverage; Linux is absent",
    },
    CapabilityAvailability {
        id: "engine.status.subscription",
        provider: "engine",
        required: false,
        state: AvailabilityState::Negotiating,
        reason: "snapshot semantics are accepted; bounded replay and resynchronization remain open",
    },
    CapabilityAvailability {
        id: "engine.transport.installed_local",
        provider: "engine-platform-adapters",
        required: false,
        state: AvailabilityState::Deferred,
        reason: "framing, discovery, peer authentication, and query/admin endpoint separation are platform gates",
    },
    CapabilityAvailability {
        id: "gui_forms.consumption_manifest",
        provider: "gui-forms",
        required: true,
        state: AvailabilityState::Negotiating,
        reason: "round 001 requests a named experimental ABI manifest",
    },
    CapabilityAvailability {
        id: "frontend.bootstrap",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Degraded,
        reason: "the atomic ORC-FE-001 snapshot and independent C++ projection are executable; installed launchd lifecycle evidence remains open",
    },
    CapabilityAvailability {
        id: "kolmogrov.candidates",
        provider: "engine",
        required: false,
        state: AvailabilityState::Deferred,
        reason: "candidate channel is independent of the semantic-v0 exact query adapter",
    },
    CapabilityAvailability {
        id: "settings.transactions",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Deferred,
        reason: "schema and storage transaction contract is not implemented",
    },
    CapabilityAvailability {
        id: "handlers.registry",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Deferred,
        reason: "handler resolution remains an outline contract",
    },
    CapabilityAvailability {
        id: "commands.registry",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Deferred,
        reason: "declarative command semantics remain an outline contract",
    },
    CapabilityAvailability {
        id: "audit.local",
        provider: "orchestrator",
        required: true,
        state: AvailabilityState::Deferred,
        reason: "audit persistence and retention are unresolved",
    },
    CapabilityAvailability {
        id: "plugins.runtime",
        provider: "orchestrator",
        required: false,
        state: AvailabilityState::Stubbed,
        reason: "plugin and plugin-AI APIs are deliberately absent",
    },
    CapabilityAvailability {
        id: "semantic.facts",
        provider: "orchestrator",
        required: false,
        state: AvailabilityState::Stubbed,
        reason: "semantic-fact design is reserved for architect direction",
    },
    CapabilityAvailability {
        id: "federation.remote",
        provider: "future-adapters",
        required: false,
        state: AvailabilityState::Deferred,
        reason: "remote federation is not a bootstrap capability",
    },
];

#[must_use]
pub fn state_count(state: AvailabilityState) -> usize {
    CAPABILITIES
        .iter()
        .filter(|capability| capability.state == state)
        .count()
}

#[cfg(test)]
mod tests {
    use super::{AvailabilityState, CAPABILITIES};

    #[test]
    fn facts_and_plugins_are_explicit_stubs() {
        for id in ["plugins.runtime", "semantic.facts"] {
            let capability = CAPABILITIES
                .iter()
                .find(|candidate| candidate.id == id)
                .expect("stub capability is registered");
            assert_eq!(capability.state, AvailabilityState::Stubbed);
            assert!(!capability.required);
        }
    }

    #[test]
    fn development_engine_adapter_does_not_claim_installed_transport() {
        let development = CAPABILITIES
            .iter()
            .find(|candidate| candidate.id == "engine.adapter.development_jsonl")
            .expect("development adapter is registered");
        let installed = CAPABILITIES
            .iter()
            .find(|candidate| candidate.id == "engine.transport.installed_local")
            .expect("installed transport is registered");
        assert_eq!(development.state, AvailabilityState::Available);
        assert_eq!(installed.state, AvailabilityState::Deferred);
    }
}
