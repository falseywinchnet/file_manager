# Build and verification

Status: **MEASURED on the M4 Mac by the current refactor campaign**.

The component declares Rust 1.87 as its minimum supported compiler, uses the checked-in lockfile, resolves Homebrew CMake and Go explicitly in cross-language tests, and keeps all build products outside version control.

## M4 commands from the authoritative checkout

- /Users/ultimussecundai/.local/bin/m4build -- rustup run stable cargo fmt --manifest-path orchestrator/Cargo.toml -- --check
- /Users/ultimussecundai/.local/bin/m4build -- rustup run stable cargo test --manifest-path orchestrator/Cargo.toml
- /Users/ultimussecundai/.local/bin/m4build -- rustup run stable cargo clippy --manifest-path orchestrator/Cargo.toml --all-targets --all-features -- -D warnings
- /Users/ultimussecundai/.local/bin/m4build -- python3 orchestrator/tools/generate_service_docs.py --check
- /Users/ultimussecundai/.local/bin/m4build -- rustup run stable cargo build --manifest-path orchestrator/Cargo.toml --release --locked

## Verification scope

- Unit tests cover lifecycle, contract/capability catalogues, framing, authentication, publication, kernel validation, routing, and release digest.
- Integration tests cover CLI/stdio, real daemon discovery, slow and saturated peers, hostile framing, independent C++ bootstrap/restart, and a separately built Go Engine live-search route.
- The documentation check proves the checked-in HTML and Markdown mirrors match manual.json and the current Rust declaration inventory.
- The previous installed launchd measurement remains historical evidence; this verification does not install or mutate the user's persistent LaunchAgent.

## Authority and evidence locators

- [Cargo.toml](../../../Cargo.toml)
- [.gitignore](../../../.gitignore)
- [AGENTS.md](../../../AGENTS.md)
- [tests/cpp_client.rs](../../../tests/cpp_client.rs)
- [tests/live_search.rs](../../../tests/live_search.rs)
- [tools/generate_service_docs.py](../../../tools/generate_service_docs.py)
- [conformance/evidence/M4_SERVICE_SYSTEM_REFACTOR_2026-08-10.md](../../../conformance/evidence/M4_SERVICE_SYSTEM_REFACTOR_2026-08-10.md)
