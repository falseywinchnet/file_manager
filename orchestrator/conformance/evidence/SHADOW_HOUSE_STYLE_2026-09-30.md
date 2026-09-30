# Shadow Orchestrator house-style correction

## Status and scope

**GIVEN:** The owner required the full `programming-house-style (2).md` rules,
including language-neutral rules in Rust and PowerShell and the C++17 spelling
rules in the independent client. A clean spelling scan is not a compliance
waiver. Three Rust standard-thread closure bridges remain **OPEN**, not approved
and not described as compliant.

**OBSERVED:** The comparison baseline is `9ec42b1`. This work corrects the
Shadow Windows bringup/search integration and its tests/tooling. The shared C++
client is included because Windows compiles it. This is not a certification of
all inherited Orchestrator Rust source. Unchanged Unix host, development JSONL
worker, and other older subsystems still contain their pre-existing closures and
style forms; this correction does not silently approve those forms.

## Audit and correction

| Area | Baseline observation | Result |
|---|---|---|
| C++ explicit types and member access | `client.cpp`: 123 inferred-type, 15 arrow/trailing-return, five lambda candidates; `main.cpp`: nine inferred-type and five arrow candidates | Typed locals/iterators/borrows; dot pointer access; named variant visitor and visible capability loops; structured bindings removed |
| C++ value construction | Implicit default initialization and computed/allocating returns | Explicit initialization, named return values, separate request-ID mutation and named native-handle acquisition |
| C++ native transfer | Direction selected inside each fragment loop; unchecked native count narrowing at helper boundaries | Named read/write adapters selected before the loop; checked DWORD/int bounds; aligned token buffer has explicit count and zero initialization |
| Rust Windows host | Captured worker state, nested panic closure, session counter/pipe cleanup written after the call | Named `SessionWorkers`, `SessionWorker`, `ActiveSession`, and `WorkerStopGuard`; scoped borrows; session cleanup on unwind; all started workers joined before releasing pipe storage |
| Rust native adapter | Numeric operation discriminant, event allocated for every operation, dense conversion/construction paths | `PipeOperation` enum; one owned event per mutably borrowed pipe; reset before each operation; pending I/O still cancelled and drained before buffer/event release; explicit count conversion and timeout rounding |
| Search integration | Closure predicates and allocating chains in added routing/provider logic | Named capability predicate; visible cursor branch; explicit Windows token decoding/validation and runtime override selection; unchanged text/exact authority distinction |
| Launcher | Ambient path captures and anonymous per-argument pipeline; temporary process disposal only on success | Explicit `LaunchContext`, typed argument array and counted quoting loop, named short-command owner; `finally` disposal and nested shutdown cleanup |
| Tests | Anonymous panic wrapper around host tests, densely constructed added search cases | Named test server and host owner; owned cleanup; typed fixtures and separated construction in added cases |

**MEASURED:** `tools/check_house_style.py orchestrator/conformance/clients/cpp`
reported five files and zero spelling findings/review candidates after correction.
The baseline counts above were computed with the same checker on `git show
HEAD:<path>`, not by searching comments or string literals. Ownership, native
call lifetime, protocol behavior, and error handling were reviewed separately.

## Open thread bridges

These three source locators are open conflicts with the owner's prohibition on
anonymous execution:

- `src/service/windows.rs:137`, `SessionWorkers::run`: scoped spawn forwards one
  named `SessionWorker` context to its `run` method. The context borrows a distinct
  mutable pipe and shared immutable kernel/token/identity plus atomic revocation.
- `src/service/windows.rs:250`, `serve_local`: the standard scope call forwards
  the named `SessionWorkers` context and the scope handle to its named method.
- `src/service/windows.rs:403`, test `start`: spawn transfers the owned
  `TestServer` path context to its named method.

**OBSERVED:** Stable `std::thread::spawn`, scoped spawn, and `scope` accept
`FnOnce`. A named function item alone cannot retain these per-invocation contexts.
Implementing the callable traits on a named record is not available through the
stable language interface. The forwarding closures reduce hidden behavior but
do not meet the rule. No unsafe trampoline, global context registry, new thread
runtime, or owner waiver was introduced. The architect must resolve this remaining
constraint explicitly; this report does not resolve it by renaming a closure.

## Verification and negative results

**MEASURED:** Native Windows x64 GNU Rust 1.98.1; at most two build jobs. The
repository `tools/verify_windows.ps1` workflow passed Rust formatting, all 76
enabled Rust tests (59 library and 17 integration), all-target/all-feature Clippy
with warnings denied, fixture check, optimized binary builds, C++17 warning-as-error
build, and the platform CTest. Unix/macOS-gated tests did not run on this host.

**OBSERVED:** Early full-test attempts exposed a peer-close/accept race. A peer
that closes before accept can yield Windows `ERROR_NO_DATA` (232). The host
previously ended its worker for this outcome; the existing authentication test
then saw a missing pipe. The corrected worker disconnects and reaccepts this
specific closed-peer outcome. Other accept errors still terminate the worker.
The first attempt also stalled in the terminal-reply test and its owned test
process was stopped; that observation is retained rather than counted as a pass.
Subsequent full runs and the focused 12-cycle terminal-reply test passed.

**OBSERVED:** The initial golden-fixture failure was already present at the
baseline: all 19 `release.rs` embedded byte inputs exactly matched `HEAD`, while
two response fixtures retained the older digest. Regeneration changed only:

- `release.response.json` → `result.provenance.digest`
- `frontend_bootstrap.response.json` → `result.release.provenance.digest`

Both now contain
`b8ab976a6581be8f37d40b819176cf66afa37213348316d2b7ed776bab6f6461` in place of
`a377407415fb962f0f0a5b525f771eb51f50c276c8f9c86a392ce6714131ec26`. No embedded
contract or registry input was edited to obtain a passing fixture.

**MEASURED:** PowerShell parsed the launcher without errors. A private copied
package with spaces in package/root/state paths passed `CheckOnly` in live-only
and indexed modes, authenticated status, and owned shutdown. An initial attempt
used an older `engine/.build/windows-validation` binary without the manifest
subcommand and failed during manifest creation; replacing only the temporary copy
with the existing native-package Engine resolved that setup error.

**MEASURED:** The rebuilt independent C++ executable connected to the owned
indexed fixture services, parsed bootstrap (restart unavailable, Windows opening
gate still blocked), and returned:

```text
terminal=success source=live_filesystem complete=true results=1 first=prefix-needle-suffix.txt
terminal=success source=catalogue complete=true generation=1 results=1 first=prefix-needle-suffix.txt
```

The first query used ordinary text `needle`; the second used explicit exact
`name=prefix-needle-suffix.txt`. The test services were shut down and no process
remained under the temporary package path. No GUI was launched. Test products
and logs remain ignored beneath `.build/orchestrator-house-style-*`.

No speedup, complete cross-platform verification, packaged release certification,
or resolution of the open Rust house-style conflict is claimed.

## Changed-file inventory

Paths are repository-relative. No commit or publication was made by this task.

```text
orchestrator/conformance/clients/cpp/include/fileman_orchestrator/client.hpp
orchestrator/conformance/clients/cpp/src/client.cpp
orchestrator/conformance/clients/cpp/src/client_windows.hpp
orchestrator/conformance/clients/cpp/src/main.cpp
orchestrator/conformance/clients/cpp/src/platform_checks.cpp
orchestrator/conformance/evidence/SHADOW_HOUSE_STYLE_2026-09-30.md
orchestrator/conformance/fixtures/bootstrap/frontend_bootstrap.response.json
orchestrator/conformance/fixtures/bootstrap/release.response.json
orchestrator/src/cli.rs
orchestrator/src/engine_jsonl.rs
orchestrator/src/engine_local.rs
orchestrator/src/kernel.rs
orchestrator/src/runtime_health.rs
orchestrator/src/service/local.rs
orchestrator/src/service/windows.rs
orchestrator/src/settings.rs
orchestrator/src/windows_local.rs
orchestrator/tests/cli.rs
orchestrator/tools/verify_windows.ps1
tools/launch_windows_search.ps1
```

**MEASURED:** Final optimized `orchestrator.exe` SHA-256:
`2d030a59fb14007767d4c09b0dedaf77672913df4157a6e303a54d90d941be49`.
