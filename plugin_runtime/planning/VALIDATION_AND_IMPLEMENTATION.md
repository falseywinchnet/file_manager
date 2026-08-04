# Validation and implementation program

Status: **execution-ready research plan; milestone order is a candidate**.

## Correctness and security gates

Every experiment records commit/dependency locks, OS/kernel, architecture,
machine, cold/warm state, fixture digest, command, raw output, and scope of
inference. No latency result is accepted before the corresponding security
oracle passes.

### Contract laboratory

Implement the logical envelope, lifecycle state machine, budget arithmetic, and
in-memory transport before selecting a codec or OS sandbox. Build fake guests:

- honest success, unsupported, and partial-result guest;
- panic/abort/segfault guest;
- never-reply and cancellation-ignoring guest;
- oversized header/payload/dimensions and integer-overflow guest;
- duplicate/stale/wrong-nonce/out-of-order reply guest;
- fork/child-survival and inherited-handle guest;
- path/network/credential/clipboard probing guest;
- prior-job-memory and cross-plugin-channel probe;
- zip/image/font/document bombs and malformed corpus.

### Platform isolation suite

For each minimum OS:

1. prove allowed input works by handle/stream;
2. prove all other test-root files and representative home secrets fail;
3. prove source write/truncate/rename/link/delete fail;
4. prove DNS, loopback, LAN, Internet, and external IPC fail with no grant;
5. prove process spawn/escape, debugger/process access, GUI/display access, and
   device access fail according to contract;
6. prove wall/CPU/memory/output/open-handle/process-count limits terminate the
   entire worker tree;
7. prove crash/kill leaves supervisor and host responsive and no grant survives;
8. record exact active enforcement primitives; absent minimum primitive fails
   closed.

### Protocol/ABI suite

- grammar/property fuzz every decoder and state transition;
- fuzz fragmented/coalesced transport and abrupt EOF;
- differential decode across Rust/C++/Go generated implementations;
- golden compatibility tests for previous two experimental versions;
- C ABI symbol/layout/calling-convention tests on all targets;
- allocator and cancellation races under sanitizers/Miri where applicable;
- cargo-fuzz/libFuzzer, ASan/UBSan for native test guests, and loom-style model
  tests for supervisor concurrency candidates;
- arbitrary unknown field/version/critical-flag tests and downgrade attacks.

### Preview/thumbnail suite

- pixel/stride/format validation and canary pages around shared buffers;
- huge dimensions, zero dimensions, negative/overflow-derived stride, partial
  writes, malformed ICC/orientation, transparency and color correctness;
- cold/warm p50/p95/p99/worst wall time, CPU, peak RSS, bytes copied, first
  usable preview, cancellation-to-death, GUI responsiveness;
- 1, 10, 100 concurrent thumbnail requests with priority cancellation;
- index-policy proof: unindexed source never starts a thumbnail guest.

### Search/virtual suite

- exact host identity remains unchanged by plugin fields;
- deterministic unchanged-snapshot ordering, ambiguity, missing/offline objects,
  stale continuation tokens, cycles, duplicates, million-child enumeration;
- provider results always retain provenance and cannot impersonate local files;
- backpressure, cancellation, bounded result count and message volume;
- judged retrieval evaluates provider channels independently of core exact and
  lexical results before any fusion candidate.

## Performance workload and rejection metrics

Budgets are not yet numeric. First measure baselines:

- null job spawn/handshake/teardown;
- 64 KiB, 4 MiB, 64 MiB sequential input;
- 256x256 thumbnail and 1920x1080/4K preview surfaces;
- warm model/plugin with 50–500 MiB resident state;
- burst selection cancellation at 10/50/100 jobs per second;
- idle supervisor and idle pooled workers.

Record p50/p95/p99/worst, CPU, peak/steady RSS, handles, page faults, context
switches, bytes copied, and energy where available. Compare process-per-job,
per-plugin, pool, and optional WASM-inside-process using identical semantics.

Rejection examples:

- any confidentiality/integrity escape rejects the topology independent of
  speed;
- any worker descendant surviving supervisor tree kill rejects the launcher;
- output accepted after source generation/nonce changes rejects protocol state;
- a pooling win that leaks bytes or cannot reclaim memory rejects pooling;
- zero-copy transport that increases crash/hang/driver surface without a measured
  interactive win is rejected;
- a codec whose safe bounded decode cannot be demonstrated is rejected.

## Milestones and exit gates

### R0 — Contract and threat laboratory

Deliver protocol semantic types, lifecycle oracle, fake transport, hostile guest
fixtures, deterministic traces, and negative-result ledger. No production codec
or OS sandbox decision.

Exit: crash/hang/oversize/stale/cancel/capability-denial tests pass in memory and
the logical contract contains no generic path/write/network/UI authority.

### R1 — Codec and transport tournament

Implement at least two bounded codecs behind the same semantic model and stdio,
pipe/socket, plus bulk-data transport candidates. Fuzz and benchmark them.

Exit: evidence supports a proposed framing/codec ADR with reversal adapter.

### R2 — Native isolation spikes on all three OS families

Build one worker launcher per platform against the same hostile suite. Preserve
unsupported/failed primitives.

Exit: minimum containment contract passes on named OS versions or that platform
is explicitly unsupported; proposed topology ADR ready for owner approval.

### R3 — Preview and thumbnail vertical slice

One intentionally small, fully specified format example and one hostile native decoder
fixture. Host adapter requests result; supervisor enforces; GUI-independent
consumer validates pixels and cancellation. No GUI controls.

Exit: crashes contained, unindexed thumbnail denied, resource/output budgets
measured, provenance complete.

### R4 — Search and virtual-system vertical slices

Use synthetic provider IDs and read-only streams. Integrate only through neutral
fixtures, not File Manager internals.

Exit: provenance/identity/cancellation/pagination gates pass; no mutation path.

### R5 — Package, grant, and local development lifecycle

Deterministic pack/inspect/sign, bounded install staging, immutable activation,
namespaced settings, disable/quarantine/uninstall cleanup, developer mode.

Exit: malicious archives and downgrade/grant-widening fixtures fail; clean
uninstall accounts for all derived data.

### R6 — Host bridges and SDKs

Stable experimental C client, direct-protocol vs cgo Go comparison, Rust guest
SDK, conformance kit, plugin doctor.

Exit: C/C++/Go clients pass identical lifecycle and cancellation suite; no Rust
layout or panic crosses C.

### R7 — Packaging and compatibility proving release

Signed/notarized platform artifacts, two-version compatibility matrix, offline
manual install/update/rollback, local diagnostics export.

Exit: reproducible replay on clean machines; support matrix and known failures
published. Still no 1.0 promise without approved ADRs and numeric product budgets.

## Decision records required before production

1. trust tiers and unsigned development policy;
2. worker topology and sandbox minimum per OS;
3. native vs WASM guest support;
4. protocol framing/codec and compatibility horizon;
5. host C ABI and Go integration;
6. preview pixel transport and cache policy;
7. package/signing/discovery/update policy;
8. grant UX, audit retention, quarantine behavior;
9. numeric resource and interaction budgets;
10. extension-specific semantics and any expansion beyond the four GIVEN classes.

Each decision uses the root ADR template, names candidates, measurements,
failure modes, reversal path, and explicit owner approval.
