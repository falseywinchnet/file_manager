#!/bin/sh
set -eu

rustup run stable cargo fmt --manifest-path orchestrator/Cargo.toml -- --check
python3 orchestrator/tools/generate_service_docs.py --check
rustup run stable cargo test --manifest-path orchestrator/Cargo.toml --locked
rustup run stable cargo clippy --manifest-path orchestrator/Cargo.toml \
  --all-targets --all-features --locked -- -D warnings
rustup run stable cargo build --manifest-path orchestrator/Cargo.toml \
  --release --locked
