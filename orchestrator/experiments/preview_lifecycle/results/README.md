# Lifecycle laboratory source checkpoint

2026-10-04 UTC. **MEASURED local deterministic correctness; CANDIDATE design.**

Environment: Shadow Windows 11 Home `10.0.22621`, `rustc 1.98.1
(48a229cea 2026-09-01)`, `cargo 1.98.1 (797e8a9bc 2026-08-05)`. Toolchain
selection used the normal Windows bring-up script. The final local test and
Clippy commands used one compilation job and
`--target-dir orchestrator/.build/preview-lifecycle`; no other compiler was
observed before the build. No dependency download was needed by this crate.

Root observed ten tests passing, with zero failures/ignored tests. The final
source build completed in 1.83 seconds and Clippy in 0.59 seconds; these are
build-command observations, not model or application performance measurements.
`cargo fmt --check` also passed after the draft's temporary stash/restoration.
Commands are recorded in the laboratory README. CI separately pins Rust
1.87.0 to check the declared minimum version on all three platforms; those
runs are not claimed here before execution.

The independently reviewed source, LF-normalized, is:

- `src/lib.rs`: `414A1C9625FAE3C2247DAC074C1B74A2D487BC7B789F3EEC4090D80947F3FC5B`
- `tests/lifecycle.rs`: `64A9FDF809A5CD3BF77364A71A1BE54142B7078F05F85CB718B016FDAAAB303E`

The raw source review and later workflow/line-ending confirmation are retained
in `SOURCE_REVIEW.md`. Scope includes model, tests, Cargo manifest, workflow,
draft, and the named registry/negotiation additions. It excludes unrelated
legacy Orchestrator/frontend code and any future adapter. The root separately
reviewed the index-row update, acceptance-ledger update and copied predecessor
native evidence for accurate scope; these change no executable behavior.

No phase can reuse a Running/Stopping slot until the modeled matching reap
event. Fixed pending storage survives the 10,000-selection fixture, and exact
deadline/acknowledgement expiry boundaries reject stale delivery. These are
useful protocol-ordering checks, not evidence that an OS process was stopped,
its descendants exited, pixels were released or a native preview appeared.
Nonce overflow is guarded in source but not directly reached by the public
fixtures. Actual effect execution, authentication, source handles/revisions,
transport framing, resource enforcement and an independent consumer remain
required before runtime adoption.
