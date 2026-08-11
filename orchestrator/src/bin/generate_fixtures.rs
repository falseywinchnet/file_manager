use fileman_orchestrator::{Kernel, Request};
use std::env;
use std::fs;
use std::path::Path;

const FIXTURES: &[&str] = &[
    "version",
    "release",
    "status",
    "frontend_bootstrap",
    "expired_deadline",
    "response_budget",
    "version_mismatch",
    "semantic_facts_stub",
    "unknown_method",
    "cancellation_unsupported",
    "critical_extension",
    "unknown_optional",
];

fn main() {
    let write = match env::args().nth(1).as_deref() {
        Some("--write") => true,
        Some("--check") => false,
        _ => {
            eprintln!("usage: orchestrator-fixtures --check|--write");
            std::process::exit(2);
        }
    };
    let root = Path::new(env!("CARGO_MANIFEST_DIR")).join("conformance/fixtures/bootstrap");
    let mut changed = false;
    for name in FIXTURES {
        let request_path = root.join(format!("{name}.request.json"));
        let response_path = root.join(format!("{name}.response.json"));
        let request_text = fs::read_to_string(&request_path).expect("read fixture request");
        let request: Request = serde_json::from_str(&request_text).expect("decode fixture request");
        let response = Kernel::new().handle(request);
        let mut expected = serde_json::to_vec(&response).expect("encode fixture response");
        expected.push(b'\n');
        let existing = fs::read(&response_path).expect("read fixture response");
        if existing == expected {
            continue;
        }
        changed = true;
        if write {
            fs::write(&response_path, expected).expect("write fixture response");
            println!("updated {name}");
        } else {
            eprintln!("stale {name}");
        }
    }
    if changed && !write {
        std::process::exit(1);
    }
}
