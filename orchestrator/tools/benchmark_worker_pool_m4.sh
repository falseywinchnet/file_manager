#!/bin/sh
set -eu

/opt/homebrew/bin/cmake -S orchestrator/third_party/threadpool_atomic_fast \
  -B orchestrator/.build/threadpool-atomic-fast -DCMAKE_BUILD_TYPE=Release
/opt/homebrew/bin/cmake --build orchestrator/.build/threadpool-atomic-fast --parallel
rustup run stable cargo build --manifest-path orchestrator/Cargo.toml \
  --release --bin orchestrator-worker-bench

for threads in 1 4 10; do
  orchestrator/.build/threadpool-atomic-fast/threadpool_atomic_fast_bench \
    "$threads"
  rustup run stable orchestrator/target/release/orchestrator-worker-bench \
    "$threads"
done
