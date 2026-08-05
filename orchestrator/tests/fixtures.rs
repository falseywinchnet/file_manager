use fileman_orchestrator::contract::CONTRACTS;
use fileman_orchestrator::{Kernel, Request};
use serde_json::Value;
use std::fs;
use std::path::Path;

#[test]
fn bootstrap_golden_fixtures_match() {
    for fixture in ["version", "status", "semantic_facts_stub", "unknown_method"] {
        assert_fixture(fixture);
    }
}

#[test]
fn executable_registry_projection_has_canonical_markdown_entries() {
    let registry =
        fs::read_to_string(Path::new(env!("CARGO_MANIFEST_DIR")).join("spec/CONTRACT_REGISTRY.md"))
            .expect("read canonical contract registry");
    for contract in CONTRACTS {
        assert!(
            registry.contains(&format!("| {} |", contract.id)),
            "{} is absent from the canonical registry",
            contract.id
        );
    }
}

fn assert_fixture(name: &str) {
    let root = Path::new(env!("CARGO_MANIFEST_DIR")).join("conformance/fixtures/bootstrap");
    let request_text = fs::read_to_string(root.join(format!("{name}.request.json")))
        .expect("read request fixture");
    let expected_text = fs::read_to_string(root.join(format!("{name}.response.json")))
        .expect("read response fixture");
    let request: Request = serde_json::from_str(&request_text).expect("decode request fixture");
    let expected: Value = serde_json::from_str(&expected_text).expect("decode response fixture");
    let actual = serde_json::to_value(Kernel::new().handle(request)).expect("encode response");
    assert_eq!(actual, expected, "fixture {name}");
}
