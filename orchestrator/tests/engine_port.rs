use fileman_orchestrator::TerminalStatus;
use fileman_orchestrator::engine_contract::{
    EngineLiveQueryResultFixture, EngineQueryResultFixture, EngineSearchBudget, EngineSearchCursor,
    EngineSearchCursorSource, EngineSearchRequest,
};
use fileman_orchestrator::engine_port::{
    EngineSearchBroker, EngineSearchOutcome, EngineSearchPolicy, EngineSearchProvider,
};
use std::collections::BTreeMap;
use std::fs;
use std::path::{Path, PathBuf};

#[derive(Debug)]
struct FakeEngine {
    catalogue: EngineQueryResultFixture,
    live: EngineLiveQueryResultFixture,
    catalogue_calls: usize,
    live_calls: usize,
}

impl EngineSearchProvider for FakeEngine {
    fn query_catalogue(&mut self, _request: &EngineSearchRequest) -> EngineQueryResultFixture {
        self.catalogue_calls += 1;
        self.catalogue.clone()
    }

    fn query_live(&mut self, _request: &EngineSearchRequest) -> EngineLiveQueryResultFixture {
        self.live_calls += 1;
        self.live.clone()
    }
}

#[test]
fn catalogue_absence_falls_back_to_live_without_building_a_catalogue() {
    let provider = FakeEngine::new("query_provider_unavailable.json");
    let mut broker = EngineSearchBroker::new(provider);
    let outcome = broker.search(&request(), EngineSearchPolicy::PreferCatalogue);
    assert!(matches!(outcome, EngineSearchOutcome::Live(_)));
    assert_eq!(outcome.terminal(), TerminalStatus::Success);

    let provider = broker.into_provider();
    assert_eq!(provider.catalogue_calls, 1);
    assert_eq!(provider.live_calls, 1);
}

#[test]
fn authoritative_catalogue_no_match_does_not_trigger_an_expensive_live_scan() {
    let provider = FakeEngine::new("query_no_match.json");
    let mut broker = EngineSearchBroker::new(provider);
    let outcome = broker.search(&request(), EngineSearchPolicy::PreferCatalogue);
    assert!(matches!(outcome, EngineSearchOutcome::Catalogue(_)));

    let provider = broker.into_provider();
    assert_eq!(provider.catalogue_calls, 1);
    assert_eq!(provider.live_calls, 0);
}

#[test]
fn denial_does_not_bypass_authority_through_live_fallback() {
    let provider = FakeEngine::new_live_draft("query_catalogue_denied.json");
    let mut broker = EngineSearchBroker::new(provider);
    let outcome = broker.search(&request(), EngineSearchPolicy::PreferCatalogue);
    assert!(matches!(outcome, EngineSearchOutcome::Catalogue(_)));
    assert_eq!(outcome.terminal(), TerminalStatus::Denied);

    let provider = broker.into_provider();
    assert_eq!(provider.catalogue_calls, 1);
    assert_eq!(provider.live_calls, 0);
}

#[test]
fn live_only_policy_never_touches_the_catalogue_lane() {
    let provider = FakeEngine::new("query_no_match.json");
    let mut broker = EngineSearchBroker::new(provider);
    let outcome = broker.search(&request(), EngineSearchPolicy::LiveOnly);
    assert!(matches!(outcome, EngineSearchOutcome::Live(_)));

    let provider = broker.into_provider();
    assert_eq!(provider.catalogue_calls, 0);
    assert_eq!(provider.live_calls, 1);
}

#[test]
fn continuation_cursor_source_prevents_midstream_route_switching() {
    let provider = FakeEngine::new("query_provider_unavailable.json");
    let mut broker = EngineSearchBroker::new(provider);
    let mut continued = request();
    continued.cursor = Some(EngineSearchCursor {
        source: EngineSearchCursorSource::Catalogue,
        value: "catalogue-cursor".to_owned(),
    });
    let outcome = broker.search(&continued, EngineSearchPolicy::PreferCatalogue);
    assert!(matches!(outcome, EngineSearchOutcome::Catalogue(_)));
    assert_eq!(outcome.terminal(), TerminalStatus::Unavailable);

    let provider = broker.into_provider();
    assert_eq!(provider.catalogue_calls, 1);
    assert_eq!(provider.live_calls, 0);
}

#[test]
fn exact_metadata_filters_never_widen_into_the_live_lane() {
    const POLICIES: [EngineSearchPolicy; 3] = [
        EngineSearchPolicy::PreferCatalogue,
        EngineSearchPolicy::CatalogueOnly,
        EngineSearchPolicy::LiveOnly,
    ];
    let provider = FakeEngine::new("query_provider_unavailable.json");
    let mut broker = EngineSearchBroker::new(provider);
    let mut filtered = request();
    filtered.text.clear();
    filtered.filters = BTreeMap::from([("kind".to_owned(), "file".to_owned())]);
    assert!(filtered.is_well_formed());
    for policy in POLICIES {
        let outcome = broker.search(&filtered, policy);
        assert!(matches!(outcome, EngineSearchOutcome::Catalogue(_)));
        assert_eq!(outcome.terminal(), TerminalStatus::Unavailable);
    }

    let provider = broker.into_provider();
    assert_eq!(provider.catalogue_calls, POLICIES.len());
    assert_eq!(provider.live_calls, 0);
}

impl FakeEngine {
    fn new(catalogue_fixture: &str) -> Self {
        Self {
            catalogue: read_fixture(&semantic_fixture_root().join(catalogue_fixture)),
            live: read_fixture(&live_fixture_root().join("query_live_no_catalogue_page.json")),
            catalogue_calls: 0,
            live_calls: 0,
        }
    }

    fn new_live_draft(catalogue_fixture: &str) -> Self {
        Self {
            catalogue: read_fixture(&live_fixture_root().join(catalogue_fixture)),
            live: read_fixture(&live_fixture_root().join("query_live_no_catalogue_page.json")),
            catalogue_calls: 0,
            live_calls: 0,
        }
    }
}

fn request() -> EngineSearchRequest {
    EngineSearchRequest {
        query_id: "fixture-query-001".to_owned(),
        root_id: "docs".to_owned(),
        relative_path: None,
        descendants: true.into(),
        text: "ledger".to_owned(),
        filters: BTreeMap::new(),
        cursor: None,
        budget: EngineSearchBudget::default(),
    }
}

fn read_fixture<T: serde::de::DeserializeOwned>(path: &Path) -> T {
    let text = fs::read_to_string(path).unwrap_or_else(|error| {
        panic!("read fixture {}: {error}", path.display());
    });
    serde_json::from_str(&text).unwrap_or_else(|error| {
        panic!("decode fixture {}: {error}", path.display());
    })
}

fn semantic_fixture_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("conformance/fixtures/engine/semantic-v0")
}

fn live_fixture_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("conformance/fixtures/engine/live-query-v0-draft")
}
