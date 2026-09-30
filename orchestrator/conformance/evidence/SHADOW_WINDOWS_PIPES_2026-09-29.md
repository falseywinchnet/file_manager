# Shadow Windows ORC1/ENG1 integration evidence

Date: 2026-09-29 local / 2026-09-30 UTC.
Status: **MEASURED native development transport and real-provider queries**.
Windows installation/supervision, durable settings and platform release promotion
remain open. This supersedes the unavailable-transport checkpoint in
`SHADOW_WINDOWS_2026-09-29.md`, without changing that historical evidence.

## Environment and native gate

Same Shadow Windows x64 checkout, Rust 1.98.1 GNU and shared MinGW GCC 16.2/CMake
4.4/Ninja as the earlier receipt. Builds used at most two jobs. Command:

```powershell
./orchestrator/tools/verify_windows.ps1
```

**MEASURED PASS:** format, 71 native Rust tests (54 unit, 5 CLI, 4 Engine
semantic, 6 route-policy, 2 registry/golden), all-target/all-feature Clippy with
warnings denied, deterministic fixture check, locked release binaries, independent
C++ warning-clean build and 1/1 CTest. Full local log:
`orchestrator/.build/windows-pipe-final.log` (ignored diagnostic product).

New native tests cover duplicate host refusal, actual server PID mismatch,
wrong credential, one-second idle authentication deadline, private-directory
ACL rejection before publication, counters, graceful cleanup and fresh-instance
restart. A semantic regression verifies an unsupervised Engine restart is
unavailable and never sends Engine shutdown. Unix-only transport tests remain
Unix-only; passing Windows tests is not new macOS/Linux evidence.

Final native artifact SHA-256:

| Artifact | SHA-256 |
|---|---|
| `orchestrator/target/release/orchestrator.exe` | `5c0e81c0736c0e3ea01d5b6f2a3f61b371d2e754fb04349e9196aae9dacab8fc` |
| independent C++ probe | `42a6f1a8c5a1610f9536defff15b66bd9edc34fab0ff81b1341bb602ca85fa44` |
| separate Engine provider | `294354eaa2feaacd3747d4f235e23d19ef3cee06a6287a95950d4fe7679ee95c` |

## Real processes and fixture workload

Created one owned source fixture containing `needle-report.txt`, outside all
Engine state leaves. No personal root was admitted. Go's
`create-windows-manifest` created current-user-private policy/runtime leaves
for root ID `docs`; hidden explicit processes ran `serve-windows` and Rust
`serve-local --runtime-dir ... --engine-runtime-dir ...`. A separate manifest
added `--index-enabled --store-root ...` for indexed mode. These were separate
Go and Rust processes, with independent C++ ORC1 requests; no JSONL child or fake
catalogue was substituted.

**MEASURED:** C++ default connection honored `FILEMAN_ORCHESTRATOR_RUNTIME_DIR`.
Live query `needle` returned `terminal=success source=live_filesystem
complete=true results=1 first=needle-report.txt`. Indexed exact query
`needle-report.txt` returned `terminal=success source=catalogue complete=true
results=1 first=needle-report.txt`. Engine status independently showed persistent
configuration, root generation 1, indexed=true, records=1, stale=false. Its
currentness remained manual-reconcile, without a watcher claim.

Bootstrap, settings schema/snapshot and service snapshots decoded through the
C++ probe. Orchestrator and Engine restart availability were false without an
admitted supervisor. The Orchestrator-owned opening gate was blocked. Embedded
`release=ready` still describes the historical first-platform macOS profile;
it is not Windows readiness. Windows settings here are in-memory only.

Restarting the indexed Go process rotated its identity. The already running
Orchestrator rediscovered it and again returned the catalogue result. An admin
reconcile command carrying the old process identity returned `stale`, without
replaying administration. All four fixture processes were gracefully shut down;
all four discovery records were absent afterward. Private fixture data/logs
remain ignored for diagnosis; no service registration/autostart was installed.

**MEASURED by coordinating frontend owner:** actual Application textbox/async
C++ client/result identity path passed with the current SDK. Live query elapsed
15.3626 ms (initial build 21.7404 ms); indexed full-name query elapsed 15.2553 ms
(initial 20.1805 ms). These are single one-file smoke observations, not throughput,
scale, cold-start or general latency claims.

## Retained failures and limits

- An old test attempted to overwrite the running debug executable and received
  Windows access denied. The owned daemon was gracefully stopped, then fixture
  binaries were copied into their own run directory to avoid build locks.
- A cross-platform golden fixture caught native queue telemetry leaking into
  the in-process laboratory. Zero pending slots now applies only to the actual
  Windows daemon; laboratory fixture semantics remain platform-independent.
- The indexed query `needle` returned an authoritative catalogue no-match, while
  live substring query matched. The current adapter maps catalogue text to an
  exact case-sensitive filename, whereas live matching is substring based.
  This is a material product limitation. No automatic fallback around successful
  catalogue no-match or opportunistic semantic change was introduced. A separate
  negotiated matching improvement is required.
- Final registry additions changed embedded byte provenance; release/bootstrap
  fixtures were regenerated and checked. No readiness flag was promoted.
- No SCM, installed activation, signing, durable Windows settings, exact-current
  watcher, subscription, million-entry workload or full GUI accessibility claim.

Canonical Windows contract: `../../spec/WINDOWS_LOCAL_PROJECTION.md`.
Provider manifest/transport tests and offline dependencies: Engine reply 008 and
`../../../engine/docs/WINDOWS_LOCAL_DEPLOYMENT.md`.

## Follow-up: ordinary text correction and explicit launcher

The retained mismatch above led to explicit owner direction and Engine-owner
review of `../../spec/FRONTEND_TEXT_PREDICATE.md`. New ordinary text now plans
catalogue `unsupported` before any query and follows the existing permitted live
route; exact criteria and issued catalogue cursors retain frozen semantics.
Text plus metadata filters fails unsupported without dropping predicates.

**MEASURED PASS:** 75 Rust tests (58 unit plus the same 17 integration/fixture
tests), fmt, all-target/all-feature Clippy, fixtures, locked release and C++
build/CTest. Log: `orchestrator/.build/windows-text-correction-final.log`.
Corrected final Rust release SHA-256:
`99b45b5aaf7727f9055a9886964eb794a2b298863b8e26b175ac947e22d28ca4`.
The earlier artifact table describes the preceding transport checkpoint.

Against the real persistent indexed fixture, the independent C++ client returned:

```text
search docs needle: source=live_filesystem complete=true results=1 first=needle-report.txt
criteria docs name needle-report.txt: source=catalogue generation=1 results=1
criteria docs name needle: source=catalogue generation=1 results=0
```

The actual frontend owner then measured `needle` returning the identity-checked
fixture through its textbox/async path: initial 19.4576 ms, query 30.5365 ms,
`expected_file_present=true`. Log:
`frontend/results/2026-09-29-shadow-windows/search-indexed-partial-route.txt`.
This is bounded live substring search while an index exists, not indexed
substring acceleration. The historical failed exact-text smoke remains above.

The explicit launcher `../../../tools/launch_windows_search.ps1` requires a chosen
root, has opt-in indexing, process-scoped runtime environment and owned hidden
service lifetimes. Live/indexed CheckOnly passed with spaces, and live Unicode
Omega paths passed. A real packaged GUI launch remained alive, closed through
its owned window, exited zero and left neither Engine nor Orchestrator discovery.
These launcher lifecycle tests used clean package 21a89b5; corrected search was
independently checked using the final Rust release above. No personal root,
SCM/autostart, global environment or execution-policy change was involved.

### Final terminal-reply race repair

The corrected fixture's last shutdown exposed a real Windows close race: the
daemon stopped and removed discovery, but the Rust client received Windows error
233 instead of its terminal reply. `DisconnectNamedPipe` can discard unread
buffered output. The host now gives the client a bounded peer-close read drain
after writing the shutdown response. It does not use an unbounded
`FlushFileBuffers` wait or introduce a wire acknowledgement.

**MEASURED PASS:** the final full gate has 76 Rust tests (59 unit + 17 others),
including twelve consecutive native shutdown/restart rounds that consume the
terminal reply; fmt, Clippy, fixture check, release build and independent C++
CTest also pass. Final log: `orchestrator/.build/windows-final-terminal-drain.log`.
Eight additional separate-process rounds alternated Rust CLI and C++ shutdown
clients; all received their replies, exited successfully and removed discovery.

Final release artifact, superseding both preceding Rust hashes:
`f2d78b9d83f8976a8157e04cb096c817577ddf475ca9deef11de1328bf42f061`.
The repair changes terminal delivery only; predicate and actual frontend query
evidence above remains applicable.
