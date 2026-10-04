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
corrected those warnings; no lint was disabled. House-style semantic review is
separate and remains pending at this checkpoint.

Scope is this crate, C++ fixture, dedicated workflow and the associated frame
draft/registry/negotiation additions. It excludes legacy frontend source, OS
supervision, codec behavior and all unrelated sibling-owned toolkit code.
Allocation refusal is handled but not fault-injected. The tests do not prove
authenticated peers, source snapshot correctness, kernel resource limits, global
receiver admission, pixel color correctness or actual GUI delivery.

Native CI pins Rust 1.87.0, uses platform-native C++, and runs the same stream
comparison on Windows, macOS and Linux. Native results will be appended after
inspection; local results do not imply those checks have already passed.
