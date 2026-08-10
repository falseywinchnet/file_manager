#!/bin/sh
set -eu

/opt/homebrew/bin/cmake -S orchestrator/third_party/threadpool_atomic_fast \
  -B orchestrator/.build/threadpool-atomic-fast -DCMAKE_BUILD_TYPE=Release
/opt/homebrew/bin/cmake --build orchestrator/.build/threadpool-atomic-fast --parallel
/opt/homebrew/bin/ctest --test-dir orchestrator/.build/threadpool-atomic-fast \
  --output-on-failure
rustup run stable cargo fmt --manifest-path orchestrator/Cargo.toml -- --check
python3 orchestrator/tools/generate_service_docs.py --check
rustup run stable cargo test --manifest-path orchestrator/Cargo.toml --locked
rustup run stable cargo clippy --manifest-path orchestrator/Cargo.toml \
  --all-targets --all-features --locked -- -D warnings
rustup run stable cargo build --manifest-path orchestrator/Cargo.toml \
  --release --locked
