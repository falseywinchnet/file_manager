use fileman_orchestrator::TerminalStatus;
use fileman_orchestrator::engine_contract::{
    EngineCapabilityState, EngineCurrentness, EngineLiveQueryResultFixture,
    EngineQueryResultFixture, EngineResultSource, EngineStatusSnapshot,
};
use std::fs;
use std::path::{Path, PathBuf};

#[test]
fn coverage_incomplete_status_fails_closed_without_disabling_cached_exact_query() {
    let snapshot: EngineStatusSnapshot = read_fixture("status_coverage_incomplete.json");
    assert!(snapshot.is_well_formed());
    assert!(!snapshot.ready);
    assert_eq!(
        snapshot.work.currentness,
        EngineCurrentness::ObservationCoverageIncomplete
    );
    assert!(snapshot.work.coverage_incomplete.is_set());
    assert_eq!(
        snapshot
            .capability("engine.exact.query")
            .expect("exact query capability")
            .state,
        EngineCapabilityState::Available
    );
    assert_eq!(
        snapshot
            .capability("engine.live.query")
            .expect("live query capability")
            .state,
        EngineCapabilityState::Unavailable
    );
}

#[test]
fn cached_stale_and_no_match_are_distinct_from_provider_absence() {
    let stale: EngineQueryResultFixture = read_fixture("query_cached_stale.json");
    assert!(stale.is_well_formed());
    assert_eq!(stale.terminal, TerminalStatus::Partial);
    assert!(!stale.results.is_empty());
    assert_eq!(stale.stale_roots, ["docs"]);

    let no_match: EngineQueryResultFixture = read_fixture("query_no_match.json");
    assert!(no_match.is_well_formed());
    assert_eq!(no_match.terminal, TerminalStatus::Success);
    assert!(no_match.results.is_empty());

    let unavailable: EngineQueryResultFixture = read_fixture("query_provider_unavailable.json");
    assert!(unavailable.is_well_formed());
    assert_eq!(unavailable.terminal, TerminalStatus::Unavailable);
    assert!(unavailable.results.is_empty());
    assert!(unavailable.error.is_some());
}

#[test]
fn live_traversal_absence_is_an_explicit_unsupported_capability() {
    let response: EngineQueryResultFixture = read_fixture("query_live_unsupported.json");
    assert!(response.is_well_formed());
    assert_eq!(response.terminal, TerminalStatus::Unsupported);
    assert_eq!(response.unavailable_channels, ["engine.live.query"]);
}

#[test]
fn live_query_pages_are_source_explicit_bounded_and_resumable() {
    let page: EngineLiveQueryResultFixture = read_live_fixture("query_live_no_catalogue_page.json");
    assert!(page.is_well_formed());
    assert_eq!(page.source, EngineResultSource::LiveFilesystem);
    assert!(!page.complete.is_set());
    assert!(page.next_cursor.is_some());
    assert_eq!(page.work.visited_entries, 37);

    let partial: EngineLiveQueryResultFixture =
        read_live_fixture("query_live_permission_partial.json");
    assert!(partial.is_well_formed());
    assert_eq!(partial.terminal, TerminalStatus::Partial);
    assert!(partial.complete.is_set());
    assert_eq!(partial.unavailable_paths, ["private"]);
}

fn read_fixture<T: serde::de::DeserializeOwned>(name: &str) -> T {
    let path = fixture_root().join(name);
    let text = fs::read_to_string(&path).unwrap_or_else(|error| {
        panic!("read fixture {}: {error}", path.display());
    });
    serde_json::from_str(&text).unwrap_or_else(|error| {
        panic!("decode fixture {}: {error}", path.display());
    })
}

fn fixture_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("conformance/fixtures/engine/semantic-v0")
}

fn read_live_fixture<T: serde::de::DeserializeOwned>(name: &str) -> T {
    let path = Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("conformance/fixtures/engine/live-query-v0-draft")
        .join(name);
    let text = fs::read_to_string(&path).unwrap_or_else(|error| {
        panic!("read fixture {}: {error}", path.display());
    });
    serde_json::from_str(&text).unwrap_or_else(|error| {
        panic!("decode fixture {}: {error}", path.display());
    })
}
