# Rust preview supervision experiment

2026-10-04 UTC. **CANDIDATE disposable laboratory**, not an installed provider,
opened plugin capability or selected production transport. This connects the
existing bounded Rust frame receiver to a real child lifecycle. It has its own
Cargo workspace and no daemon or GUI linkage. Rust remains the Orchestrator
implementation language. OS binding versions match the existing daemon pins.

## Question and workload

Can the host drain a real decoder's bounded image output and diagnostics, retain
deadline checks under output pressure, and refuse image publication until both
exact EOF and successful child exit? A shell pipeline does not establish that
ordering. This experiment rejects early publication, failed process exit,
truncation, trailing data, excessive diagnostics and stalled processes.

One controller owns one child, one existing frame receiver, one reusable 8 KiB
read buffer and a fixed 4 KiB diagnostic prefix. At most one chunk from each pipe
is read before the next deadline/exit check. Diagnostics admit at most 64 KiB in
total; the counter excludes the chunk that triggers refusal. Raster storage
remains bounded by the frame receiver's 4 MiB limit. No growing output capture,
line queue, anonymous callbacks or background reader threads are used.

Unix marks only the parent's pipe readers nonblocking through `fcntl` and handles
WouldBlock/Interrupted without converting them to EOF. Windows probes available
bytes using `PeekNamedPipe`, then reads at most that extent as the sole reader.
This Windows strategy is scoped to this single-threaded controller: the API's
synchronous-handle behavior is not evidence for a future multithreaded host.

Each iteration checks elapsed deadline/cancellation before I/O and observes the
exact child with `try_wait`. A complete frame from a still-running child stays
unpublished. Both pipes must reach actual EOF and the child must exit successfully
before the receiver is finalized; time is checked again after pixel validation.
Every error retires the child. Termination alone is not proof of exit; `wait`
must succeed. The owner also attempts retirement during unwinding.

**Limits:** launch and final cleanup wait are blocking. The deadline is an
observed processing cutoff, not a hard OS wall-clock ceiling. No descendant
handling, private source authority, filesystem/network confinement, process-memory
cap, provider identity, integration with the lifecycle slot model, or GUI
publication is implemented. The 1 ms idle sleep is a fixture polling choice,
not a measured production scheduling policy. Cancellation is a deterministic
elapsed-time trigger, not a frontend cancellation port.

## Source input is deliberately separate

The process fixtures use null stdin and closed, finite modes in the same built
fixture executable. The real-codec check inherits an already-open read-only
handle to a generated envelope file. It checks regular-file type and the existing
16 MiB plus header bound, but makes no hostile-source/snapshot claim. Its input
file is generated under ignored `.build/`, with no user-file mutation. This is
not a production disk-spooling choice and does not test writable-stdin progress,
backpressure, sender stalls or concurrent source replacement. No arbitrary shell
command is constructed. Third-party code remains in the child process.

## Verification

Use one local compiler job after `tools/Enter-WindowsToolchain.ps1`:

```powershell
cargo fmt --manifest-path orchestrator/experiments/preview_supervision/Cargo.toml -- --check
cargo test --manifest-path orchestrator/experiments/preview_supervision/Cargo.toml --locked --offline --jobs 1 --target-dir orchestrator/.build/preview-supervision -- --nocapture --test-threads=1
cargo clippy --manifest-path orchestrator/experiments/preview_supervision/Cargo.toml --locked --offline --all-targets --all-features --jobs 1 --target-dir orchestrator/.build/preview-supervision -- -D warnings
```

The four semantic suites require exact pixel bytes; nonzero exit after a complete
frame; empty/truncated/trailing output; four deadline termination/recovery cycles;
cancellation; and diagnostic output at/above its quota. The 5-second child stalls
are finite fixtures. Loose 2/3-second harness guards distinguish cancellation
from simply waiting for the child; they are not product performance thresholds.

The JPEG workflow builds Rust 1.87 on native Windows/macOS/Linux, runs these
suites sequentially, then supervises the separate C++ JPEG decoder on one
generated orientation-one source. The large result must be exactly 1024x768
with 3,145,728 opaque pixel bytes and a successful reaped exit. Color/orientation
oracles remain the independent earlier streams; this host check adds lifecycle
ordering, not a new color-accuracy claim.

**MEASURED local Windows:** Rust 1.98.1, debug, one compiler job: four suites
passed. Four 250 ms deadline fixtures returned after 251–252 ms with observed
reaping; the 100 ms cancellation fixture returned after 101 ms. The separately
supervised Release C++ decoder produced its admitted frame and exited in one
41 ms sample; the retained follow-up sample is 42 ms. These include process
launch/transport/validation and are not a latency distribution or application-speed
claim. The independent visible audit accepted the complete new
source, tests and workflow additions, plus the admitted-byte counter and final
budget check, with no blockers or remaining house-style violations. Exact scope,
raw reports and source hashes are retained in `results/`. Each timeout test now
requires all 68 fixture-frame bytes to have arrived before the timeout, excluding
a slow-launch false positive. Final local tests and Clippy pass.

**MEASURED native minimum-version verification:** Actions run
[37178271741](https://github.com/falseywinchnet/file_manager/actions/runs/37178271741)
passed on all three hosts at exact source
`89901ae29c6e982718e16909386d789cfb2844f0`. Jobs: Windows `111365393112`,
Mac `111365393132`, Linux `111365393042`. Each runs Rust 1.87 formatting,
warning-free Clippy, the four semantic suites (17 observed/reaped fixture
children), and the real C++ decoder check. Raw scoped excerpts are in
`results/Native-*-89901ae.log`. Single generated-JPEG host samples were Windows
44 ms, Mac 152 ms and Linux 39 ms; different hosted machines and debug Rust make
these correctness-run observations unsuitable for platform speed rankings.
The four 250 ms deadline cycles returned in 250–256 ms across these runs, including
cleanup. This is measured fixture behavior, not a hard cleanup guarantee.

The complete application merge matrices are separate gates; this native research
run does not by itself establish that PR44 is ready to merge or ship.

The full [programming house style](../../../planning/PROGRAMMING_HOUSE_STYLE.md)
applies to all authored Rust, tests and workflow changes. Source review, not only
passing tests and Clippy, is required. This is ordinary authored code, not the
stricter generated C++ profile.

Primary API semantics: [Rust child ownership and wait](https://doc.rust-lang.org/std/process/struct.Child.html),
[Windows pipe availability](https://learn.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-peeknamedpipe).
