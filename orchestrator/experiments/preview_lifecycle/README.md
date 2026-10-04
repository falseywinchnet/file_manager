# Selected-preview lifecycle laboratory

This is an independently buildable, disposable implementation of the lifecycle
fragment in `../../spec/SELECTED_PREVIEW_LIFECYCLE_DRAFT.md`. It is **CANDIDATE
conformance work**, not an Orchestrator runtime module or File Manager feature.
No process, codec, filesystem, wire transport or GUI is linked or invoked.

The purpose is to catch stale-image publication, premature process-slot reuse,
unbounded selection queues and acknowledgement starvation before writing the
provider adapter. Fixed client storage and inert Copy records make ownership and
transition costs inspectable. Every returned effect must eventually be executed
by a host; this crate cannot prove that a host did so.

From the repository root on Shadow, select the normal read-only toolchain and
run one local compiler job (respecting the shared two-job host maximum):

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cargo fmt --manifest-path orchestrator/experiments/preview_lifecycle/Cargo.toml -- --check
cargo test --manifest-path orchestrator/experiments/preview_lifecycle/Cargo.toml --locked --offline --jobs 1 --target-dir orchestrator/.build/preview-lifecycle
cargo clippy --manifest-path orchestrator/experiments/preview_lifecycle/Cargo.toml --locked --offline --all-targets --all-features --jobs 1 --target-dir orchestrator/.build/preview-lifecycle -- -D warnings
```

The crate has no dependencies. Rust 1.87 or newer is required; Rust 2021 is the
declared source edition. Its lockfile and source belong in Git, build products
only in ignored `.build/`. The first local build briefly used Cargo's default
target directory; it was moved into Orchestrator's `.build/` after the process
finished. No running build tree was moved.

The complete `../../../planning/PROGRAMMING_HOUSE_STYLE.md` applies to the model,
fixtures and authored build/CI tooling. Rust enum destructuring and derives for
inert value records are Rust syntax; C++ spelling restrictions do not prohibit
them. Named operations, explicit state, no closures/callback capture, fixed
storage, checked counters, validation before mutation and complete ownership/
failure review remain required. Source review and test evidence are separate.
