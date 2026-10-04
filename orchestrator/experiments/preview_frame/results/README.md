# Preview byte-frame evidence

2026-10-04 UTC. **MEASURED local Windows:** six integration tests passed using
Rust/cargo 1.98.1 and one compiler job. Fragmentation/truncation tests enumerate
all boundaries in the 88-byte golden. The maximum-raster test admits 4 MiB;
invalid counts are rejected. Clippy with all targets/features and `-D warnings`
passes, as does rustfmt. C++ producer builds with GCC 16.2.0 Release, warning-free,
and its stream is accepted by the independent Rust CLI with exact 2x3 pixels.
The C++ spelling scanner reports one file, zero findings. These are conformance
observations, not performance benchmarks or production preview acceptance.

Initial Clippy rejected unnecessary explicit tail `return` statements and
range loops used only for indexing fixture tables. Named Rust tail expressions,
typed destructuring of borrowed table entries and explicit status-loop state
corrected those warnings; no lint was disabled. Independent house-style semantic
review accepted the named source/tooling/draft scope with no blockers or remaining
violations. The verbatim report is in `SOURCE_REVIEW.md`. Root separately reviewed
the later negotiation replies and evidence records; they change no behavior.

Scope is this crate, C++ fixture, dedicated workflow and the associated frame
draft/registry/negotiation additions. It excludes legacy frontend source, OS
supervision, codec behavior and all unrelated sibling-owned toolkit code.
Allocation refusal is handled but not fault-injected. The tests do not prove
authenticated peers, source snapshot correctness, kernel resource limits, global
receiver admission, pixel color correctness or actual GUI delivery.

Native CI pins Rust 1.87.0, uses platform-native C++, and runs the same stream
comparison on Windows, macOS and Linux. Native results are recorded below
independently of the local run.

## Native minimum-version matrix

Source `6a9b617df9244db6a318afcc7330f58f5fab484c`,
[run 37175907171](https://github.com/falseywinchnet/file_manager/actions/runs/37175907171):

| Host | C++ compiler | Job | Result |
|---|---|---|---|
| Windows 2022 | MSVC 19.44.35229.0 | 111358341203 | Six suites, fmt, Clippy and exact streamed pixels pass |
| macOS 26 arm64 | AppleClang 21.0.0.21000101 | 111358341125 | Same checks pass |
| Ubuntu 24.04 | GCC 13.3.0 | 111358341217 | Same checks pass |

All three selected Rust 1.87.0. Root inspected the actual per-job logs for the six
passing tests and independent C++ producer acceptance. Scoped validation log
excerpts are retained as `Native-{windows,mac,linux}-6a9b617.log`. Full job logs
remain in ignored `.build/preview-frame-*-6a9b617.log` on Shadow and linked CI.
No test is skipped. The subsequent source-review/evidence commit changes only
records, not executable behavior. Full application matrices remain a separate
pending PR gate; this result creates no new downloadable preview capability.
