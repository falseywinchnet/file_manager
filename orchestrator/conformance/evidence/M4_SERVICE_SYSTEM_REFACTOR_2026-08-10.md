# M4 Orchestrator service-system refactor verification — 2026-08-10

Status: **MEASURED current working-tree verification; Core 1.0 behavior
preserved; no installed-service mutation performed**.

## Claim under test

The Orchestrator executable can be refactored into explicit CLI, stdio, bounded
Unix-host, and macOS launchd modules without changing the accepted Core 1.0
contract/wire behavior, and the current authoritative source can produce a
warning-free optimized arm64 artifact on the nearby M4 Mac.

Rejection criteria:

- stale generated documentation;
- formatting or Clippy failure under the declared Rust 1.87 minimum;
- any Rust, hostile-wire, independent C++, or real Go Engine integration test
  failure;
- changed golden bootstrap/release semantics;
- failed locked optimized build;
- release projection not reporting the accepted ready macOS Core 1.0 profile.

## Source and environment

- Authoritative checkout: `/Users/ultimussecundai/file_manager` on the Neo.
- Base Git revision: `044c47ca185ab0a570098ef3155e5356ed59b3a5` plus the
  uncommitted Orchestrator refactor documented here.
- Mirrored build root:
  `/Users/joshuahkuttenkuler/Developer/CodexBuilds/file_manager-2d80cdb86b7d`.
- M4 environment observed 2026-08-10 08:22:12 EDT: macOS 26.5 build 25F71,
  arm64.
- Rust: `rustc 1.87.0`; Cargo: `1.87.0`.
- Go: `go1.25.5 darwin/arm64`.
- CMake: `4.2.1`.
- Dependency input: checked-in `orchestrator/Cargo.lock`; verification used
  `--locked` for tests, Clippy, and release build.

The refactor lowers the previously declared but unavailable Rust 1.96 floor to
the oldest compiler exercised here, Rust 1.87, and removes the only newer
let-chain syntax. Cross-language tests resolve the Mini's absolute Homebrew Go
and CMake paths because the relay shell deliberately omits Homebrew from
`PATH`.

## Replay command

From the authoritative checkout:

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh orchestrator/tools/verify_m4.sh
```

The script executed:

```sh
rustup run stable cargo fmt --manifest-path orchestrator/Cargo.toml -- --check
python3 orchestrator/tools/generate_service_docs.py --check
rustup run stable cargo test --manifest-path orchestrator/Cargo.toml --locked
rustup run stable cargo clippy --manifest-path orchestrator/Cargo.toml \
  --all-targets --all-features --locked -- -D warnings
rustup run stable cargo build --manifest-path orchestrator/Cargo.toml \
  --release --locked
```

## Results

- **MEASURED:** the service atlas matched its checked-in output: 54 generated
  files covering seven system guides and nineteen Rust source modules, plus two
  manifests.
- **MEASURED:** 66 tests passed, zero failed: 42 library unit tests, nine CLI/
  daemon tests, one independent C++ bootstrap/restart test, four Engine
  contract tests, five routing tests, two golden/registry fixture tests, one
  real Go Engine zero-catalogue route, one hostile local-wire test, and one
  local-wire payload fixture test.
- **MEASURED:** the independently configured C++17 client built through CMake,
  authenticated, validated the atomic bootstrap and golden release digest,
  shut down the daemon, and reconnected to a fresh daemon generation.
- **MEASURED:** the separately built Go Engine returned the expected
  catalogue-independent live result through authenticated
  `orchestrator.search`; the frontend-facing request did not select a source
  lane.
- **MEASURED:** `cargo clippy --all-targets --all-features -- -D warnings`
  passed.
- **MEASURED:** the locked optimized build passed and produced a Mach-O 64-bit
  arm64 executable of 1,222,448 bytes.
- **MEASURED artifact SHA-256:**
  `02ccc07d14cdff6148b5303d608447fab81e90b1436da0f534fb9094f45c474b`.
- **OBSERVED executable projection:** `orchestrator release --json` reported
  `state: ready`, `ready: true`, target `1.0.0`, first platform `macos`, and
  release-manifest digest
  `ad79beea35c96ab88b127d132f8a1807c65ff3f92793d982755b63c8797c6cab`.
- **OBSERVED:** the release projection continues to report `signed: false`.

## Documentation surface verification

The in-place `docs/library/index.html` atlas was served only on loopback and
inspected through a browser. Desktop and 640-pixel responsive layouts rendered;
the manifest loaded all 26 entries; search reduced `launchd` to five relevant
entries; selecting `src/service/launchd.rs` updated the iframe, active
navigation state, reviewed boundaries, and generated declaration inventory.

## Scope of inference and remaining gates

- This run proves compilation and conformance of the refactored source on the
  named M4 environment. It is not a performance benchmark or long-run resource
  measurement.
- This run did not install, boot out, or modify the user's persistent
  LaunchAgent. The installed lifecycle claim remains the separate 2026-08-07
  disposable-artifact measurement.
- The development Go Engine route does not close installed Engine discovery,
  query/admin authentication, native NTFS/ext4, or million-entry promotion
  gates.
- Windows named pipes and Linux supervisor packaging remain open platform
  projections.
- Settings, handlers, commands, plugin execution, hives, semantic facts, and
  platform integration retain their independent contract gates.
- Orchestrator's Frontend 001 prerequisite is ready. Architect direction is
  already recorded; global Frontend 001 opening still waits on the GUI.Forms
  FM0 go-ahead under ADR-006. This record does not manufacture that external
  authority.
