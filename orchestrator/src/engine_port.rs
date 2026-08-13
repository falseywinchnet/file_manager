use crate::common::TerminalStatus;
use crate::engine_contract::{
    EngineLiveQueryResultFixture, EngineQueryResultFixture, EngineSearchCursorSource,
    EngineSearchRequest,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EngineSearchPolicy {
    PreferCatalogue,
    CatalogueOnly,
    LiveOnly,
}

#[derive(Debug, Clone, PartialEq)]
pub enum EngineSearchOutcome {
    Catalogue(EngineQueryResultFixture),
    Live(EngineLiveQueryResultFixture),
}

impl EngineSearchOutcome {
    #[must_use]
    pub const fn terminal(&self) -> TerminalStatus {
        match self {
            Self::Catalogue(result) => result.terminal,
            Self::Live(result) => result.terminal,
        }
    }
}

pub trait EngineSearchProvider {
    fn query_catalogue(&mut self, request: &EngineSearchRequest) -> EngineQueryResultFixture;
    fn query_live(&mut self, request: &EngineSearchRequest) -> EngineLiveQueryResultFixture;

    fn live_available(&self) -> bool {
        true
    }
}

pub trait UnifiedEngineSearch: Send {
    fn search(&mut self, request: &EngineSearchRequest) -> EngineSearchOutcome;
    fn live_available(&self) -> bool;
}

#[derive(Debug)]
pub struct EngineSearchBroker<P> {
    provider: P,
}

impl<P: EngineSearchProvider> EngineSearchBroker<P> {
    #[must_use]
    pub const fn new(provider: P) -> Self {
        Self { provider }
    }

    pub fn search(
        &mut self,
        request: &EngineSearchRequest,
        policy: EngineSearchPolicy,
    ) -> EngineSearchOutcome {
        // The frozen live-filesystem lane currently supports bounded name/path
        // text only. Never widen a filtered catalogue request by silently
        // dropping its exact metadata predicates during fallback.
        if !request.filters.is_empty() {
            return EngineSearchOutcome::Catalogue(self.provider.query_catalogue(request));
        }
        if let Some(cursor) = request.cursor.as_ref() {
            return match cursor.source {
                EngineSearchCursorSource::Catalogue => {
                    EngineSearchOutcome::Catalogue(self.provider.query_catalogue(request))
                }
                EngineSearchCursorSource::LiveFilesystem => {
                    EngineSearchOutcome::Live(self.provider.query_live(request))
                }
            };
        }

        match policy {
            EngineSearchPolicy::CatalogueOnly => {
                EngineSearchOutcome::Catalogue(self.provider.query_catalogue(request))
            }
            EngineSearchPolicy::LiveOnly => {
                EngineSearchOutcome::Live(self.provider.query_live(request))
            }
            EngineSearchPolicy::PreferCatalogue => {
                let catalogue = self.provider.query_catalogue(request);
                if may_fall_back_to_live(catalogue.terminal) {
                    EngineSearchOutcome::Live(self.provider.query_live(request))
                } else {
                    EngineSearchOutcome::Catalogue(catalogue)
                }
            }
        }
    }

    #[must_use]
    pub fn into_provider(self) -> P {
        self.provider
    }

    pub const fn provider_mut(&mut self) -> &mut P {
        &mut self.provider
    }
}

impl<P: EngineSearchProvider + Send> UnifiedEngineSearch for EngineSearchBroker<P> {
    fn search(&mut self, request: &EngineSearchRequest) -> EngineSearchOutcome {
        self.search(request, EngineSearchPolicy::PreferCatalogue)
    }

    fn live_available(&self) -> bool {
        self.provider.live_available()
    }
}

const fn may_fall_back_to_live(terminal: TerminalStatus) -> bool {
    matches!(
        terminal,
        TerminalStatus::Unsupported
            | TerminalStatus::Unavailable
            | TerminalStatus::Stale
            | TerminalStatus::Quarantined
    )
}

#[cfg(test)]
mod tests {
    use super::may_fall_back_to_live;
    use crate::common::TerminalStatus;

    #[test]
    fn fallback_terminal_allowlist_is_narrow_and_authority_safe() {
        for terminal in [
            TerminalStatus::Unsupported,
            TerminalStatus::Unavailable,
            TerminalStatus::Stale,
            TerminalStatus::Quarantined,
        ] {
            assert!(may_fall_back_to_live(terminal), "{terminal:?}");
        }

        for terminal in [
            TerminalStatus::Success,
            TerminalStatus::Partial,
            TerminalStatus::Invalid,
            TerminalStatus::Denied,
            TerminalStatus::VersionMismatch,
            TerminalStatus::BudgetExceeded,
            TerminalStatus::Timeout,
            TerminalStatus::Cancelled,
            TerminalStatus::InternalFault,
        ] {
            assert!(!may_fall_back_to_live(terminal), "{terminal:?}");
        }
    }
}
