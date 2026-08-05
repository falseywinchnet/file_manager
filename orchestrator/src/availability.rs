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
        id: "engine.query",
        provider: "engine",
        required: true,
        state: AvailabilityState::Negotiating,
        reason: "round 001 awaits engine reply and runtime discovery adapter",
    },
    CapabilityAvailability {
        id: "engine.administration",
        provider: "engine",
        required: true,
        state: AvailabilityState::Negotiating,
        reason: "query and administrative authority separation is unresolved",
    },
    CapabilityAvailability {
        id: "gui_forms.consumption_manifest",
        provider: "gui-forms",
        required: true,
        state: AvailabilityState::Negotiating,
        reason: "round 001 requests a named experimental ABI manifest",
    },
    CapabilityAvailability {
        id: "file_manager.client",
        provider: "file-manager",
        required: true,
        state: AvailabilityState::Negotiating,
        reason: "frontend client projection awaits round 001 reply",
    },
    CapabilityAvailability {
        id: "kolmogrov.candidates",
        provider: "engine",
        required: true,
        state: AvailabilityState::Deferred,
        reason: "first-class seam is required; transfer artifact is not ready",
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
}
