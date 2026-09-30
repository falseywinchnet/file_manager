# File Manager recovery assessment using Plan Paint

Date: 2026-09-29 (Shadow desktop local date).

Status: **OBSERVED source review and limited local checks; CANDIDATE execution
sequence.** This is not a replacement architecture decision or a release claim.

## Mandate and interpretation

**GIVEN, current owner request:** develop toward an ordinarily usable File
Manager with macOS, Windows and Linux installers, background indexing, useful
previews, efficient implementation, and a consistent interface. Work directly
on the Shadow desktop checkout; isolated development is no longer the operating
requirement. Use the nearby Plan Paint success as the blueprint. Optional
multi-machine file availability/transfer must not depend on WAN services.

This assessment interprets the final reference to “building up plan paint” as
building up File Manager using Plan Paint's work. It does not change Paint.
The supplied `programming-house-style (2).md` was read as reference material,
not as an independent instruction to rewrite either repository.

The current desktop direction supersedes the old Neo-only working-tree and
development-isolation assumptions for this work. It does not establish a new
filesystem identity, undo, codec, plugin or indexing-consent contract. Ordinary
real-root use should become a supported product mode. Fault-injection tests can
still create disposable fixtures without constraining everyday development.

## Evidence boundary

Reviewed File Manager at `7ce5cf4` and Plan Paint at `5f27981`, both initially
clean. Read the parent planning prerequisites, component instructions, contract
registry, product exclusions, positive interview transcription, search
reconciliation, visual constitution/verdicts, owner correction, component
readiness records, selected implementation paths, and Paint's build/package
and accessibility records. This is a targeted review, not an exhaustive audit
of every implementation or historic interview answer.

Paint's dependency was actually fetched with its checked-in fetcher. The
fetcher verified source SHA-256
`a368e03180a10911ff1316a4a0880028a4880bdbcf531456bd4ec35d0ace5a0f`
for `gui-forms-native-source-64248bcc06a1.tar.gz`, then verified and applied all
six patches in its lock file. The prepared source is local review evidence at
`../.build/review-2026-09-29/plan-paint-gui-forms/`; that directory is excluded
through the local `.git/info/exclude`. No production source was replaced.

Historical test and dogfood statements below are attributed records, not tests
rerun in this review. Release links and CI definitions were inspected as local
source; current remote CI status and published assets were not independently
audited. No native File Manager or Paint GUI was launched here.

## What the interviews already settle

**GIVEN/DECIDED:** the product is more specific than an Explorer clone. Keep:

- one location per window, a collapsible real Home/Volumes tree, one content
  field, and a collapsible preview/properties pane;
- separate search and inline editable breadcrumb/path surfaces, small icons by
  default, ordinary selection/open behavior and basename-first inline rename;
- compact useful menus/commands, the accepted House Composite visual language,
  and consistent retained rendering across platforms;
- useful browsing without Engine or Orchestrator augmentation; current-folder
  search scope, honest stale/unavailable states, and live-search fallback;
- explicit indexing consent and root policy, authoritative filesystem identity,
  and separately owned C++ frontend/GUI.Forms, Go Engine and Rust Orchestrator;
- build-time Web.Forms authoring under ADR-016, with no runtime browser.

The existing interview explicitly excludes tabs, dual panes, Recents and
Recommended landing pages, synthetic This PC/Quick Access roots, automatic
grouping, tags/notes, floating Quick Look, automatic search-scope widening,
cloud/account recommendations, and platform-dependent visual skins. The current
request does not explicitly reverse those choices.

Sources: [negative definition](PRODUCT_NEGATIVE.md),
[positive transcription](ARCHITECTURE_INPUTS.md),
[search reconciliation](search/SEARCH_RECONCILIATION_001.md),
[Design DNA](../frontend/planning/visual/DESIGN_DNA_006.md),
[owner verdicts](../frontend/planning/visual/DESIGN_DNA_VERDICTS_007.md), and
[ADR-016](../decisions/ADR-016-FILE-MANAGER-WEB-FORMS-AUTHORING-AND-GUI-FORMS-RUNTIME.md).

Do not restart the entire interview. Turn these accepted answers into a small
command/workflow acceptance matrix, with each row linked to its authority,
visible control, implementation, test, and latest native observation. Reopen
only actual conflicts or missing product choices.

## Findings that control the work

| Status | Finding and implication | Source |
|---|---|---|
| OBSERVED | The frontend explicitly rejects non-Apple configurations. Removing that check alone cannot produce a portable application. | `frontend/CMakeLists.txt:14` |
| OBSERVED | Frontend identity uses POSIX `lstat`/device/inode; native launch uses `posix_spawn` and `/usr/bin/open`. File operations reject broad/personal/repository roots. These are real implementation boundaries, not missing installer wrappers. | `frontend/src/filesystem_model.cpp`, `platform_commands.cpp`, `file_operations.cpp` |
| OBSERVED | This GUI.Forms tree has headless, macOS and Windows hosts, but no Linux host. Its installed Application target is in the macOS build section; Windows compatibility-host support is not equivalent to the portable application package Paint consumes. | `gui_forms/src/host/`, `gui_forms/CMakeLists.txt` |
| OBSERVED | Orchestrator's Windows local serve/call functions return “not implemented”; its C++ local client guards Unix sockets/POSIX code and supplies a non-Unix unavailable fallback. This is a missing runtime transport, not evidence that the client cannot compile on Windows. | `orchestrator/src/service/mod.rs`, `orchestrator/conformance/clients/cpp/src/client.cpp` |
| OBSERVED | Engine has platform identity adapters and Windows watcher code, but Windows reparse-point identity is explicitly pending and non-Linux/non-Darwin local peer authentication reports unavailable. | `engine/internal/identity/identity_windows.go`, `engine/internal/transport/peer_other.go` |
| OBSERVED | Engine has substantial exact catalogue, generation, reconciliation and recovery machinery worth preserving. Its README distinguishes live functionality from unadmitted storage/similarity experiments. | `engine/README.md` |
| OBSERVED historical assessment | Engine's 2026-08-10 readiness record explicitly says lexical text/phrase search is absent and always-current daily search is not earned. Its 69/100 rubric is not percent complete or a current benchmark. | `engine/docs/INTEGRATED_DOGFOOD_READINESS.md` |
| OBSERVED | Current previews cover bounded text and PNG, not a general useful preview catalogue. | `frontend/src/preview.cpp` |
| OBSERVED | Frontend handwritten code still uses `auto`, lambdas and arrow member access. GUI.Forms' earlier rewrite completion claim applies to its recorded scope, not automatically to the frontend. | `frontend/src/filesystem_model.cpp`, `file_operations.cpp`; `gui_forms_rewrite/COMPLETION_REPORT.md` |
| OBSERVED | Status contradicts status: root planning still calls frontend implementation gated; frontend README calls the product 0.001-alpha; the total implementation plan still says a protected-root 1.0 profile passed, and CMake declares 1.0.0. The explicit owner rejection controls product claims. | `planning/PROGRAM_MAP.md`, `frontend/README.md`, `frontend/planning/TOTAL_IMPLEMENTATION_PLAN.md`, `frontend/CMakeLists.txt`, `frontend/planning/OWNER_CORRECTION_2026-08-11.md` |

**Conclusion from these observations:** preserve the component boundaries and
useful implementations, but make platform support and complete user workflows
the organizing unit of repair. Another style-only library rewrite would leave
the principal product failures intact.

## What to carry back from Plan Paint

**OBSERVED:** Paint is a concrete consumer of an installed GUI.Forms SDK. Its
CMake probes required capabilities, its release frontend excludes the legacy
SDL/ImGui UI, and its build jobs build the toolkit, install it, build the app,
run tests, package dependencies/fonts, and launch packaged output. Windows uses
MinGW/MSYS2; Linux builds x64 and ARM64 musl packages and checks a package on a
glibc host. The Mac packaging script exists separately from that Windows/Linux
workflow. These are valuable working delivery patterns.

The local README identifies 1.1.2 and offers a Mac installer plus Windows/Linux
portable archives. Those archives are not yet the three native installers
requested for File Manager. The README explicitly discloses unsigned Mac
installer/notarization limits and X11/XWayland limitations.

### Actual toolkit difference

Comparison of the fetched, fully patched snapshot with `gui_forms/`, normalizing
CRLF/LF and comparing file contents:

| Area | Current files | Paint snapshot files | Changed shared files | Added in snapshot | Current-only paths |
|---|---:|---:|---:|---:|---:|
| Public include tree | 232 | 237 | 16 | 5 | 0 |
| Implementation tree | 287 | 314 | 38 | 27 | 0 |
| Tests | 83 | 99 | 15 | 16 | 0 |
| CMake helper tree | 14 | 14 | 3 | 0 | 0 |

These counts do not establish semantic compatibility or include the top-level
CMake file, assets and dependency trees. They establish a bounded comparison
surface, not a reason to overwrite the older tree wholesale.

Concrete reusable work includes Linux X11 hosting, clipboard/drop/dialog and
accessibility services; Windows Application packaging and accessibility;
popup ownership/focus corrections; scrolling/canvas/input extensions; fallback
fonts and shared font storage; and event-dispatch lifetime work. For example,
the newer Event implementation retains dispatch state across callbacks and
defers compaction until the outermost emission finishes, instead of copying the
slot vector for every emission. It needs its associated behavioral tests.

**Compatibility edges:** the host protocol changes from 5 to 7. Paint's Windows
build uses GDI with Skia/HarfBuzz disabled, while Mac/Linux use CPU Skia.
Consistent typography/rendering therefore needs evidence; copying build flags
does not settle File Manager's rendering requirements. The Linux release is
X11/XWayland, not native Wayland or mixed-DPI Linux. Windows accessibility is
MSAA, not a full UI Automation provider. Paint's accessibility document retains
end-to-end assistive-device limitations. Its older native-port document still
says Linux accessibility is absent, while later patches and accessibility docs
show an ATK/AT-SPI implementation: even the successful blueprint needs dated
evidence reconciliation.

**CANDIDATE adoption method:** establish one canonical GUI.Forms source and
versioned SDK; carry compatible changes back in compilable behavior-focused
batches with their tests; test both consumers against the same candidate SDK.
Keep Paint pinned to its released SDK until that compatibility run passes.
Negotiate the changed `ORC-GUI-001` consumption manifest and Web.Forms capability
manifest explicitly. Preserve File Manager's newer authored-layout work and
ADR-016 rather than copying Paint's entire application composition.

## Proposed execution sequence

Each stage ends in something runnable or installable. Documentation records
that result; it is not a substitute for it.

1. **Reconcile authority and establish the Shadow build.** Record the current
   direct-checkout operating direction, correct live status/version conflicts
   without deleting historical failures, and inventory visible commands against
   accepted behavior. Set up a reproducible Windows C++/CMake/Ninja, Go, Rust and
   Python toolchain; prefer Paint's demonstrated MinGW route for the first SDK
   build. Use separate ordinary build directories per platform, not separate
   development sandboxes. Establish a logged baseline before source changes.
   **Exit:** one documented command builds/tests the candidate Windows toolkit;
   failures and unavailable capabilities are visible.

2. **Converge GUI.Forms on the consumer-proven improvements.** Integrate platform
   hosts/package exports, lifetime/focus/input fixes, fonts and accessibility in
   reviewable batches. Test public package consumption and Web.Forms-generated
   controls, not only library internals. Run Paint as the regression consumer.
   **Exit:** the same public application contract launches a real window on
   Windows, macOS and Linux; repeated dialogs/dropdowns, keyboard focus, text,
   resize, scale and close/reopen behave as declared. Unsupported host features
   remain explicit. No blanket claim of complete cross-platform parity.

3. **Make one real Windows File Manager workflow complete.** Port identity,
   paths/volumes, launch, clipboard/drop and local service discovery/transport
   through named platform adapters. Preserve registered semantics while adding
   Windows and Linux projections. Separate platform code from domain logic and
   apply the supplied explicit style as files are repaired. Keep builds passing
   between behavior changes and mechanical cleanup.
   **Exit:** launch from the desktop, navigate Home and a real drive, edit the
   path, use tree/list/icons and keyboard selection, open files, inspect text or
   image previews, close/reopen, and remain useful with augmentation stopped.
   Actual UI comparison uses the approved File Manager composition, not Paint's
   ribbon or a substitute mockup.

4. **Promote ordinary file operations and background search.** Replace the
   prototype-only root restrictions with an explicit normal-user operating
   policy, retaining object revalidation, cancellation and comprehensible
   errors. Implement ordinary trash/recycle semantics and the approved undo
   boundary; resolve the prototype quarantine/two-step-delete mismatch. Exercise
   collisions, cross-volume copies/moves, links/reparse points, permissions,
   removable volumes and interrupted operations. Keep destructive fault
   campaigns on generated fixtures. For Engine, finish currentness and restart
   recovery, root consent, supervision, useful filename/path search and stable
   pagination before richer ranking.
   **Exit:** repeatable daily workflows on user-selected real locations, with
   restart and service-loss behavior, plus native filesystem correctness tests.

5. **Anneal the product through daily use.** Alternate a real task, a recorded
   failure, a focused regression test where appropriate, a small repair and a
   rerun of the task. Work on the measured slow path. Extend previews according
   to an explicit useful-format list; PNG-only renderer policy need not make
   the product PNG-only. Keep other codecs/providers outside the renderer core
   and within their admitted boundaries. Preserve selection and scroll position
   during refresh; no inert visible commands.
   **Exit:** a release checklist of demonstrated workflows and carried defects,
   not a collection of passing component counts.

6. **Deliver installers and exercise their lifecycle.** Reuse Paint's locked
   dependency, font, license and packaged-launch methods. Add File Manager's
   own installer/service/update/uninstall behavior for each platform and record
   component versions independently. Choose exact formats and minimum OS
   versions explicitly. Prototype packaging as soon as the runnable chain
   exists; public release still requires its existing release gates.
   **Exit:** install on a machine without build tools, opt into indexing, use
   the app, update it, restart it, and uninstall without losing user files or
   leaving unintended services. Signing/notarization status is stated exactly.

The first implementation tranche should cover stages 1–3 through a working
Windows navigation slice. Do not bundle a storage redesign, LAN federation or
whole-repository formatting migration into that tranche.

## Performance and platform evidence

The existing ADR-001 budgets already provide useful rejection targets: idle
CPU below 0.1% over ten minutes, no unchanged-filesystem index writes, one-million
record exact/prefix p95 below 8 ms and p99 below 20 ms, and warm GUI-to-first-
correct-result below 50 ms. They are accepted targets, not achieved measurements.

Use identical declared corpora and cold/warm conditions for the current engine,
changes, mature-store controls and appropriate OS search comparisons. Distinguish
cached catalogue latency from fresh-result correctness. Measure query tails,
first paint, input stalls, private memory, background CPU, index size and write
amplification. Define lexical relevance tasks before declaring “better search.”
Windows Search/Spotlight comparisons must state scope and indexing state;
SQLite controls do not by themselves prove superiority to Explorer search.

**CANDIDATE test arrangement:** Windows is already available on Shadow, so a
Windows VM is unnecessary for the first native cycle. Use the M4 for actual
macOS build/dogfood once configured from this host. Use Linux CI for every
change and a small real Linux session for package, filesystem, clipboard/drop,
accessibility and service-lifecycle acceptance. A VM can supply that session;
it does not need to become the main workspace. No claim that the old AWDL alias
is reachable from Shadow was tested here.

Paint's dogfood substantially reduces GUI uncertainty, but does not validate
NTFS/ext4 identity, watcher loss, recycle/trash, service authentication or an
indexer's crash recovery. Cross-compiling and Wine remain useful evidence for
their own domains, not replacements for native runtime checks. This is a
targeted platform campaign, not a reason to postpone ordinary Windows use.

## Stretch scope and remaining product choices

**GIVEN:** optional cross-machine availability/transfer without WAN dependence.
**CANDIDATE:** explicitly paired LAN peers with a local identity, source-host
authority, offline status and explicit transfer. “Shared account” needs a
bounded local meaning; it does not imply a cloud login, remote discovery
service, bidirectional sync, remote execution or a personal assistant. Keep
this behind `ORC-FED-001` after the local application works.

The few choices to settle during the corresponding implementation slice are:
minimum supported OS/architectures and first Linux display profile; useful
preview-format priority; normal trash/undo behavior versus the protected-root
prototype; consent and initial index roots; concrete daily acceptance tasks;
and eventually LAN account/pairing semantics. None requires reopening the
accepted language, ownership or retained-UI decisions.

## Checks performed in this review

**MEASURED on this Shadow desktop:**

- Paint `scripts/check-style.py`: passed mechanical first-party exclusions.
- Paint `scripts/check-languages.py`: passed 22 packs, 679 entries each,
  including UTF-8, metadata, coverage, placeholders and whitespace.
- The pinned toolkit fetch/patch process completed with verified hashes. Git
  reported patch whitespace warnings; they were not application failures and
  were not repaired in this source review.
- `python -m unittest discover -s web_forms/tests -v`: 37 tests in 2.854 s;
  27 passed, seven skipped because no C++ compiler was available, three errored
  because browser discovery found no Chromium executable. Full output is at
  `../.build/review-2026-09-29/web-forms-tests.txt`. Browser discovery currently
  checks Mac paths and `google-chrome`/`chromium` on PATH, not ordinary Windows
  installations. This does not establish that the compiler's visual behavior
  failed.

**OBSERVED environment limit:** CMake/CTest/Ninja, C++ compilers, Go and Rust were
not discoverable on this session's PATH. Python was used from the desktop's
bundled runtime. No full native builds, CTest, Go suite, Rust suite, index
performance benchmark, installer test or live GUI check was completed here.

The review adds this assessment and its index entry only. It does not silently
promote a new GUI.Forms SDK, change accepted contracts, install services, scan
personal files, or claim that File Manager is ready for daily use.
