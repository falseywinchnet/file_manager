# Independent source review

2026-10-04 UTC. Visible audit thread `01a0fb48-1437-7302-8b6f-f3bcf02e7319`.
Verbatim completed report follows. The later negotiation/acceptance/evidence
record additions are separately reviewed by root; they add no executable code.

**No blockers or remaining house-style violations found in the reviewed preview-frame laboratory and draft additions.** The implementation is consistent with a disposable B0 conformance fixture; the documentation and registry do not promote it to a frozen protocol, live provider, or new capability.

The substantive checks are sound:

- **Header before allocation:** the receiver accumulates exactly 64 header bytes, then validates magic, version, header extent, ticket, reserved bytes, status, profile and geometry before reserving raster storage. Both axes are bounded to `1..=1024` before multiplication, so stride and payload calculations fit `u32`. Conversion to `usize` is checked, and live payload is capped at 4 MiB.
- **Fragmented input:** partial headers and bodies preserve their offsets correctly, including a chunk spanning the header/body boundary. Empty chunks do not mean EOF. Once allocated, raster storage does not grow per chunk. Excess bytes, including a second frame, are rejected.
- **Failure and ownership:** a failed `push` permanently poisons the receiver and replaces its pixel vector, releasing its allocation. Later pushes and `finish` refuse that receiver. `finish` consumes the owner, so truncated and nonopaque results drop their storage; successful completion moves the vector into the result without copying it.
- **Pixel validation:** every alpha byte is checked for 255 after exact payload completion. The draft correctly limits the claim: the receiver can validate shape, profile identifier and opacity, but cannot infer correct orientation or sRGB conversion from arbitrary color bytes.
- **EOF, reap and publication:** actual EOF is a caller obligation, and the CLI calls `finish` only after a zero-length stream read. The draft explicitly separates frame validity and EOF from process exit, matching reap, ticket freshness, source validation and publication permission. A valid frame followed by a failed process exit cannot publish under the proposed host rules.
- **Independent fixture and workflow:** the C++ producer writes explicit literal bytes rather than a native structure or shared encoder, checks writes and flush, and selects binary stdout on Windows. The Rust consumer compares all 24 expected pixel bytes. The workflow enables pipeline failure propagation, so producer failure is not masked by successful consumption.
- **Authority limits:** echoed tickets remain correlation only. The draft retains unresolved peer authentication, provider binding, filesystem identity/revocation, concurrent modification, ancestor replacement, global copy accounting and supervision. The registry addition preserves candidate status.

I reviewed the complete [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md), applying its language-neutral requirements to Rust and its C++ spelling rules to the producer. Explicit types, named behavior, borrows, initialization, conversions, operation order and failure cleanup are inspectable. Per-case receiver allocation in the tests serves independent lifecycle cases; the streaming path reuses its fixed chunk and admitted raster storage.

Exact reviewed scope:

- [Cargo.toml](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_frame/Cargo.toml) and [Cargo.lock](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_frame/Cargo.lock)
- [src/lib.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_frame/src/lib.rs)
- [src/bin/consume.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_frame/src/bin/consume.rs)
- [tests/framing.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_frame/tests/framing.rs)
- [fixture/CMakeLists.txt](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_frame/fixture/CMakeLists.txt) and [fixture/producer.cpp](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_frame/fixture/producer.cpp)
- [README.md](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_frame/README.md)
- [preview-frame.yml](C:/Users/Shadow/file_manager/.github/workflows/preview-frame.yml)
- [SELECTED_PREVIEW_FRAME_DRAFT.md](C:/Users/Shadow/file_manager/orchestrator/spec/SELECTED_PREVIEW_FRAME_DRAFT.md)
- The frame-fragment addition in [CONTRACT_REGISTRY.md](C:/Users/Shadow/file_manager/orchestrator/spec/CONTRACT_REGISTRY.md:76)

This was read-only source review. I ran no builds or tests and made no edits or Git mutations. Native CI results remain root-owned; allocation-refusal handling and storage release were assessed from source, without fault injection.
