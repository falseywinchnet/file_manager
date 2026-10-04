# Independent supervision source review

2026-10-04 UTC. Authorized visible audit thread `01a0fb48-1437-7302-8b6f-f3bcf02e7319`.
Both reports are retained verbatim below, in review order. Rebase preserved the
complete implementation tree. Root separately reviewed canonical draft and
results-record additions; no unrelated legacy code is certified.

**No correctness or house-style blockers found in the reviewed supervision laboratory and workflow additions.** The source requires both validated stream completion and a successfully reaped child before returning a frame.

The important paths are consistent with the stated research scope:

- **Publication order:** each loop checks deadline/cancellation, observes child exit, then reads at most one bounded chunk from each pipe. A nonzero exit fails immediately. A frame can escape only after stdout EOF, stderr EOF, successful exit observation, `Receiver::finish`, and a final budget check. A complete frame from a still-running child remains unpublished.
- **Bounded storage and fairness:** stdout feeds the previously reviewed bounded frame receiver through one reusable 8 KiB chunk. Stderr has a 64 KiB admission quota and retains only its first 4 KiB. Subtraction precedes quota addition, and retained-copy arithmetic stays within the fixed array. Continuously available output cannot bypass the next budget check or indefinitely starve the other pipe.
- **Pipe handling:** Unix preparation preserves existing flags while adding nonblocking mode; interrupted and would-block reads return pending. The Windows path confines its unsafe operation to `PeekNamedPipe`, documents handle/pointer lifetimes, and reads no more than the reported available bytes. Its correctness depends on the declared sole-reader, single-threaded use.
- **Failure cleanup:** after spawn, the child has a named owner. Pipe setup, read, protocol, diagnostic quota, deadline and cancellation failures unwind the receiver and pipe owners before retirement. Retirement attempts termination and then waits; only observed wait success establishes `reaped`. A failed cleanup overrides the frame/result with `Failure::Cleanup`. The owner’s destructor also attempts retirement if needed.
- **Tests and workflow:** the four suites cover successful framing, nonzero exit after a frame, missing/truncated/trailing output, complete-frame stalls, deterministic cancellation, stderr quota boundaries and fresh-child recovery. The added workflow selects the declared toolchain, checks formatting/lints, runs tests sequentially, and feeds a generated bounded envelope file into the real decoder supervision check.

The real-decoder check validates framing, opacity through the receiver, expected dimensions and payload extent, EOF and successful reap. **It does not independently validate the decoded color anchors**; that evidence remains with the existing separate Rust JPEG checker and pipeline tests.

I applied the complete [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md), including language-neutral requirements for explicit types, named behavior, state and borrow ownership, initialization, conversions, operation order, failure cleanup and repeated-loop storage. No remaining violations were found in this scope.

Exact reviewed scope:

- [Cargo.toml](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/Cargo.toml) and [Cargo.lock](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/Cargo.lock).
- Full [src/lib.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/src/lib.rs).
- Full [src/pipe.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/src/pipe.rs).
- Full [src/bin/fixture.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/src/bin/fixture.rs).
- Full [src/bin/check_decoder.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/src/bin/check_decoder.rs).
- Full [tests/process.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/tests/process.rs).
- The added supervision path filters and supervision step in [jpeg-research.yml](C:/Users/Shadow/file_manager/.github/workflows/jpeg-research.yml).

The accepted limitations remain material: final retirement is blocking; synchronous spawn and cleanup have no hard wall-clock bound; command/input restrictions are caller obligations; the generated file’s metadata check is not immutable source acquisition; descendants, confinement, process-memory limits, production cancellation and a stdin pump are absent. Diagnostic counts stop at the admitted quota rather than counting all excess bytes. None of these is concealed by a successful frame result.

This was read-only source review, with no edits, builds, tests or Git mutations. Windows test and real-decoder passes remain root-reported measurements. README/results being authored separately, native Mac/Linux execution, and dependency implementation audits are outside this acceptance.

---

**The follow-up changes are accepted; no new blockers or house-style violations found.**

- `frame_bytes_admitted` starts at zero and increments only after a successful `Receiver::push`. Successful pushes cumulatively admit at most one 64-byte header plus a 4 MiB payload, so the addition is bounded. Failed chunks do not contribute.
- The deadline test now requires all **68 fixture bytes** to have been admitted before accepting `Failure::Deadline`. This closes the slow-launch false-positive case and supports the intended claim: a complete received frame remains unpublished while its child stalls.
- The budget check after `receiver.finish()` prevents successful frame return when the deadline or deterministic cancellation becomes due during final pixel validation. On that failure, the local completed frame drops its owned pixels.

The [README](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/README.md) accurately states the blocking launch/final wait, single-threaded Windows pipe strategy, diagnostic counter semantics, generated-file input limits, and absence of source authority, confinement, process-memory limits, lifecycle-slot integration and GUI publication. It also correctly separates the real-decoder geometry/lifecycle check from the earlier independent color oracle.

Exact additional reviewed scope: counter initialization, update and plumbing plus the post-validation check in [src/lib.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/src/lib.rs); the strengthened deadline assertion in [tests/process.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_supervision/tests/process.rs); and the full README.

The README’s statement that independent source review is pending can now be updated to reference this completed review. Its runtime measurements remain root-reported; I performed no builds, tests, edits or Git mutations.
