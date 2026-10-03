# File Manager owner dogfood acceptance audit

Status: **ACTIVE; first source pass, not a completed whole-product audit**.

## Current checkpoint — 2026-10-02

**Search match/context development, 2026-10-03:** existing provider rank,
certainty and ordered evidence now survive client projection. Submitted query,
scope, filters and continuation have one immutable owner per page. Appended
correspondence rows keep their own source/generation and do not substitute a
later page's provenance. Windows integration passes 15 suites; native CI and
delivery remain pending. See
`../results/2026-10-03-search-match-context/README.md`. This is evidence integrity
and presentation work, not indexed-substring activation or measured acceleration.

**Latest delivery update:**
[`v0.001-alpha.d4dcb24`](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.d4dcb24)
is now published with New Folder naming and retained search source records.
Both native matrices pass; tested and rebase-merged trees match; independent
archive checks and Mac preview/Details image review are recorded in
`../results/2026-10-03-search-source-records/DELIVERY.md`. The dated paragraphs
below preserve their earlier development checkpoints. Catalogue substring
activation and thumbnails remain unfinished.

**Subsequent development checkpoint:** bounded scan correctness is established
on the generated fixtures, but the legacy performance gate remains unpassed;
the experiment is retained outside active source. Prepared-window immutable
input, aggregate admission and final-owner budget retirement pass five Windows
suites. Revocation now preserves epoch history. These are private foundations,
not evidence that text rendering or application responsiveness has improved.
The new GUI tests await native CI; the published package above remains current.

**Latest delivery, 2026-10-03 UTC:** `v0.001-alpha.1371514` is the downloadable
Windows/macOS/Linux checkpoint. It adds cancellable staged copying, explicit
terminal/cleanup reporting, basename-first rename and native no-replace
publication to the earlier search coverage, TXT/PNG and Details repairs. Both
native matrices passed; archive contents/checksums were independently verified.
Native Mac quarantine/xattr equivalence passed. Exact source/tree/release and
visual-evidence limits are in `../results/2026-10-03-cancellable-copy/DELIVERY.md`.

**Current development:** New Folder now offers naming after its created identity
appears in a refreshed listing, with navigation/edit retirement. Five assembled
application cases and both native matrices pass at `47dcf619`; the identical
tree is merged as `66757297`. Physical-input/visual acceptance and a new portable
release remain pending. See `../results/2026-10-03-new-folder/README.md`.

**Current source repair:** typed Engine identity/revision observations now survive
client decoding, background preparation and displayed-row lifetime. Shared page
provenance avoids per-row scan-token copies. Local validation and the remaining
native/identity-equivalence gaps are recorded in
`../results/2026-10-03-search-source-records/README.md`. This supplies neither
thumbnails nor indexed ordinary-text acceleration.

The initial tables below describe the original audit baseline. Current source
does have a Show/Hide preview control, a separate text coverage caption, four
sortable/resizable Details columns and modal operation errors. Their limited
fixture acceptance is recorded below; this is not whole-product acceptance or
confirmation of the owner's macOS retest. Broad preview formats, thumbnails,
folder aggregates, ribbon ergonomics and everyday operations remain incomplete.

The latest native Windows diagnostic is
`../results/2026-10-03-native-paint/README.md`: settled small-control repaints are
under 1 ms in a short local run, while full-window presentations remain roughly
26–41 ms after the first frame. It provides no physical-input or Mac speed claim.
No-replace publication and bounded cancellable regular-file copy are now in the
download above. Visible byte progress and the broader operation gaps remain.

The initial register and first-repair narrative below retain baseline evidence.
Preview release `v0.001-alpha.b1986ad` is published for Windows, macOS arm64 and
Linux after native CI and independent archive-content/checksum verification.
Its real Mac application probe renders UTF-8 text, a decoded PNG and the
unsupported-format explanation. This repairs the covered failure; it does not
extend formats or deliver thumbnails. The release remains a portable alpha.

Source development now includes four factual Details columns, sortable headers,
atomic row/indicator publication, and a measured reduction in PNG validation
cost. Source `17a748f` passed both native workflows, but visual review rejected
its clipped Date modified column and short-folder sort viewport. Corrections
and their exact local/native acceptance status are tracked in
`DETAILS_INTEGRATION_2026-10-02.md`. The corrected source is now published in
`v0.001-alpha.6e74434`, with both native matrices passing, independently verified
archives, rebase-merged source and a visually reviewed Mac screenshot. Physical-
input smoothness remains unverified.

The search audit `SEARCH_NEXT_STAGE_AUDIT_2026-10-02.md` and Orchestrator
proposal `ENGINE_CATALOGUE_SUBSTRING_001.md` are **CANDIDATE** next-stage
records. Ordinary text currently uses live traversal, not catalogue substring
acceleration. A private 10k checked-generation reference now has differential
fixture, warm paired, cancellation-boundary and full Go test/race/vet evidence
in `../../engine/results/SUBSTRING_REFERENCE_EXPERIMENT_2026-10-02.md`.
It exposes strict read/decode/output bounds still needed before public admission;
it activates no search route. Contracts and typed identity projection remain
open. No new persistent index is selected.

## Text coverage follow-up — 2026-10-02

**OBSERVED:** the old loader appended its byte-limit notice to file contents,
where the seven-line excerpt could hide it. The new source returns a separate
initialized `text_truncated` fact and preserves only sanitized file contents in
the text payload. Application uses the existing persistent caption for
`Text excerpt · 64 KiB limit`, `Text excerpt · UTF-8`, or the empty-file state.
This discloses excerpt coverage even when the body elides lines. A subsequent
selection replaces the caption through its ordinary preview state.

Focused loader cases cover exact-limit input, UTF-8 splits at every byte of
multibyte characters, and empty input. The assembled test uses a longer-than-
limit, multiline file and requires the notice outside the body; switching to
PNG retires the text notice. Local 12/12 tests pass (3.21 seconds). The Mac
harness checks caption pixels and preserves a text-preview screenshot. Source
`0873f9cb02dbfaf811b1a31c90be3af5db1b100a` passed both complete native runs
`36992163185` and `36992189991`; PR7 rebase-merged as
`f03ae65034b487d87bd193cca48a9c3a5154ff27`, with identical complete tree
`2ca745b1aa7f4784951b41c2428ff50ede865a75`. Release
`v0.001-alpha.0873f9c` publishes the independently verified three archives
(40 Linux, 42 Mac, 44 Windows receipt-listed files). The actual Mac screenshot
was visually reviewed: text and its separate limit caption are readable;
the native test counted 235 caption pixels. This supersedes the preceding
downloadable checkpoint without changing the supported-format boundary.

The final named-caption assembly also passes the focused preview/interaction
checks (2/2, 2.74 seconds). The unavailable search placeholder now says
`Search not configured here`: the former `root not indexed` wording falsely
implied that ordinary live filename/path search requires a persistent index.
This wording correction does not activate a search provider.

**House-style review:** changed PreviewResult member, readable-text assembly,
Application's text branch, loader/interaction/native fixture assertions and
artifact path. Explicit values/types, owned caption/payload, no new callback or
retained borrow, no added per-character allocation or format dispatch. Existing
preview lifecycle and unrelated legacy test/tool implementation are not certified.

## Queued search supersession follow-up — 2026-10-02

**OBSERVED:** SearchWork and CriteriaWork previously connected and queried even
when their generation was already obsolete on dequeue. Both now use one named
SearchCancelled predicate before connection, after connection, after query and
on failure. It observes atomic shutdown/generation state; jobs retain Application
through invocation. Obsolete replies are retired before UI enqueue, with the
existing UI generation check still guarding the final enqueue/drain race.
Already-running synchronous connection/query calls remain non-interruptible
through this API. This change does not claim end-to-end transport cancellation
or measured input-to-present speed.

**MEASURED:** matching Windows frontend build and 12/12 tests pass (2.88 s).
The added interaction case verifies current/replacement eligibility, superseded
ordinary and criteria jobs producing no queued replies, and shutdown revocation.
It does not instrument the transport's connection count or inject cancellation
mid-connection/query; those boundaries were source reviewed. Existing installed
provider probes remain separate integration checks. Both native matrices
36994098674 and 36994102785 passed at 6009dd7. PR8 rebase-merged as
8f84952ad8ac86fb1ad43124a9f3ac38c931a501, with matching source tree
468cb119b5b4f609e100ff70bea28cc13392b4b8. It remains excluded from the already
published `v0.001-alpha.0873f9c` archive.

**House-style source review:** SearchCancelled declaration/definition, the
changed SearchWork/CriteriaWork bodies, named test-probe methods and the new
interaction case. Explicit types/initialized state, owned job lifetime, atomic
observations, synchronous operation order, failure retirement and no added
per-result work were reviewed. The changed-scope spelling scanner reports zero
findings in three files; that is not a certification of unrelated legacy code.

## Search page publication follow-up — 2026-10-02

The next source moves route/identity/metadata observation and display formatting
to owned worker preparation. UI publication keeps supersession, deduplication,
selection and provenance. On six alternating generated Windows comparison
pairs, 500-result UI apply p50/p95 fell from 41.259/52.063 ms to 2.593/4.326 ms;
100-result p50/p95 fell from 7.484/12.794 to 0.413/0.712 ms. Preparation still
performs the filesystem work outside the UI; this is not an end-to-end speedup.
Local 12/12 tests pass, including cancellation, native facts, path refusal and
ownership checks. New native integration remains pending. Source review,
workload, raw measurements, limits and the initial test-fixture correction are
recorded in `../results/2026-10-02-search-publication/README.md`.

Two bounded Engine reader approaches remain **REJECTED** because legacy exact
queries regressed. Source is restored; reconstructible patches, hashes, compiler
and profiler evidence and raw paired observations are retained under
`../../engine/results/`. Passing correctness/race/vet did not admit either
candidate. Public substring semantics, identity projection and bounds remain open.

## Authority and evidence

- **GIVEN:** owner reports macOS previews fail across multiple file types,
  thumbnails absent, interaction slow and unsmooth, Details missing planned
  headers/columns, folder-size icon behavior absent, and interface/ribbon
  materially short of the interviews. No exact format corpus was supplied.
- **OBSERVED:** published baseline is `89846dfd4d7d7987beda6a0b59d77d504f53a32b`,
  version `0.001-alpha`. Native compilation/tests passing on three platforms
  establishes neither usable previews nor daily-use acceptance.
- Interview authority remains Design DNA 006 with later DDV-007 verdicts and
  explicit owner corrections. Candidates retain their labels. Existing repair
  ledger history remains evidence, including failures; stale status prose is
  not acceptance authority.
- Work is in the Shadow checkout. Plan Paint is a read-only behavioral/toolchain
  reference. The active Notepad sibling owns its declared title/menu and private
  prepared-window GUI.Forms files. No workers are created.

## Initial acceptance register

| Area | Authority and source observation | Required evidence / next work | State |
|---|---|---|---|
| Selected-file preview | Owner failure; DDV-007-08; baseline `src/preview.cpp` admits PNG and 26 UTF-8 text extensions only. `Application::apply_preview` validates and copies PNG into the image registry on the UI thread; actual decoding is renderer work. Existing loader test uses a signature with fake payload, not a decodable PNG. | Actual regular TXT and PNG selection through Application, image draw and text layout/contrast; then macOS native replay with ordinary files. Explicit unsupported/error states must be readable. | **OPEN; initial repair below, macOS report unresolved** |
| Preview adaptation | DDV-007-08 requires a collapsible identity header and one scroll plane. `update_adaptive_preview` automatically hides the surface below 560 logical units; user expansion control absent. | Explicit accessible expansion/collapse, usable short-window behavior, stable selection, correct PropertyList header extent and scroll ownership. | **OPEN** |
| Format coverage | Owner expects useful previews across types. Loader excludes JPEG, PDF and most ordinary media. | Record an honest format matrix; prioritize ordinary formats through admitted first-party decoding contracts, bounded background work, cancellation and visible failures. PDF/media/provider scope must be traced before choosing a new dependency. | **OPEN; PNG/text only in baseline** |
| Indexed thumbnails | DNA-O13 says thumbnails come from the index; unindexed folders retain material icons. `ORC-PLG-003` remains stubbed. No general application thumbnail pipeline found. | Trace engine/Orchestrator ownership and first-party thumbnail contract; cache by object revision, visible-range scheduling, memory limits, invalidation and stale state. Never imply that toggling icon view supplies thumbnails. | **OPEN; missing capability** |
| Details | Owner requires planned headers and more than two columns. Application currently sets ObjectView details mode/secondary text without a factual column model. | Recover exact interview columns, sorting, resizing, keyboard and persistence behavior; use/extend a shared public control, preserve object identities across views. | **OPEN; missing model, not just styling** |
| Folder size icons | DDV-007-17 and DNA-O06 require indexed allocated size and coarse embossed M/G/T marks with declared rounding. Exact size belongs in inspection. | Trace allocated-size aggregation/currentness through Engine and Orchestrator, then render real observations. Audit any additional thickness/fill semantics in interviews before treating a badge as the entire requirement. | **OPEN; missing capability** |
| Shelf/ribbon | DDV-007-03 and existing FM-R009 retain File Manager's two command groups. Owner rejects current ergonomics/finish. | Compare ordinary/narrow layouts with prototype and mature adaptive behavior; preserve command geography, readable labels, target sizes, state and overflow. | **OPEN** |
| Responsiveness | Owner reports unsmooth interaction. Existing matcher microbenchmarks do not demonstrate overall application speed. | Measure selection-to-preview, folder first paint, wide-folder scroll, resize, search input and cancellation; record environment, p50/p95/max, CPU/RSS and work performed on UI thread. Optimize reproduced bottlenecks. | **OPEN; no end-to-end speed claim** |
| Everyday operations | FM-R015–017: native Open exists; Open With, external drag/drop, conflict/merge flows and crash recovery incomplete. Baseline default launch is read-only. | Audit each menu/shelf/context action and admitted operation profile, recovery, identities and explicit failure. Source presence alone does not prove a practical daily-use workflow. | **OPEN** |
| Search | Existing engine search and bounded live fallback are distinct from the proposed indexed name/path candidate. | Catalogue oracle, realistic 10k/100k/1M workloads, exact verification, cancellation, freshness and generation-safe paging; preserve negative measurements. | **OPEN; see engine/docs/INDEXED_NAME_PATH_SEARCH_001.md** |
| Distribution | Three portable archives exist. macOS arm64 baseline is ad-hoc signed, not notarized; installers/update/uninstall not complete. | Reproducible source/receipt match, native package smoke checks, install/update/uninstall, diagnostics without secret/user-content leakage. | **OPEN; portable alpha only** |
| Full interaction/accessibility inventory | FM-R020/022 require every control and input path. Current source pass is partial. | Expand each row to cases traced to interview decisions, executable checks, native pointer/keyboard/assistive evidence, and owner verdict. | **OPEN; not exhaustively audited** |

## Preview coverage and remaining acceptance cases

**OBSERVED current source, not a promise of format completeness:**

| Input | Current behavior | Acceptance still needed |
|---|---|---|
| Regular PNG | Reads at most 16 MiB encoded; GUI.Forms validates/decodes; PictureBox scales into the preview | Native corrected-layer pixels; large decoded dimensions; transparency; rotated/metadata-heavy fixtures; repeated replacement and errors |
| UTF-8 text with an admitted extension | Reads a 64 KiB prefix; displays at most seven wrapped lines | Native representative Unicode; long unbroken lines; CRLF/BOM; truncation disclosure reachable when hidden lines exceed the visible body |
| Empty admitted text file | Explicit empty-file message | Native empty-state legibility |
| Malformed text, changed identity, unavailable path | Explicit refused/changed/unavailable message | Rapid selection changes and currentness under ordinary filesystem updates |
| JPEG, HEIC, WebP, GIF, PDF, audio/video | Unsupported | A separately bounded, negotiated format/provider stage and real native fixtures |
| UTF-16 text, extensionless text, unlisted text extensions | Not automatically recognized as supported text | Decide bounded encoding/sniffing policy; do not silently call this general text support |
| Folder/symbolic link | Material icon; no file-content loading through the link | Folder facts/indexed allocated-size presentation and explicit link identity |
| Indexed thumbnail | Missing | Revision-bound index projection, retrieval, visible-range scheduling and invalidation |

The admitted text extensions are `.txt`, `.md`, `.csv`, `.tsv`, `.json`,
`.xml`, `.yaml`, `.yml`, `.toml`, `.ini`, `.log`, `.c`, `.cc`, `.cpp`,
`.h`, `.hpp`, `.m`, `.mm`, `.go`, `.rs`, `.py`, `.sh`, `.html`, `.css`,
`.js`, and `.ts` (ASCII case-insensitive extension matching).

**DECIDED boundary:** ADR-020 currently admits bounded UTF-8 and PNG built-ins.
Its reversal path requires independent bounds and evidence for further formats.
The older handler interview requires native/third-party providers behind a
process boundary; ORC-PLG remains stubbed. A source-compatible Paint codec is
research input, not automatic admission into File Manager or GUI.Forms.
JPEG and PDF are practical next-format **CANDIDATES**; their exact implementation
and resource/identity/cancellation contracts remain unresolved. The owner's
report across multiple types is broader than the single PNG layering defect.

The loader's trailing truncation sentence can itself be outside the seven-line
display; that is an **OBSERVED disclosure gap** requiring a persistent status
outside the elided body. Registry admission also precedes decoder execution,
so a registry-success caption is not a renderer-success receipt. These remain
open even after the native ordinary PNG fixture passes.

## Verification discipline

Keep loader unit tests, assembled-application tests and native dogfood evidence
separate. A screenshot is not timing evidence; a headless visible flag is not
proof of readable pixels. Record Windows observations as Windows observations,
never as reproduction of the owner's macOS failure. Preserve failed hypotheses.

Review every changed first-party source/test/tool against
`planning/PROGRAMMING_HOUSE_STYLE.md`: explicit types, named executable behavior,
retained callback state, ownership/borrow lifetimes, sequencing, initialization,
conversions, failure states and repeated storage/work. Record exact reviewed
files and unresolved violations with each implemented repair. Do not claim the
legacy tree compliant based on a scanner or a successful build.

## First repair: preview visibility and bounded text

**OBSERVED source defects:** the dynamically created preview Label inherited
dark theme text over the authored `#1d2a37` surface. Automatic collapse changed
the preview block's requested size but left PropertyList's separately owned
header height at 224. Unsupported-format messages competed for a narrow single
caption. The UTF-8 validator rejected an otherwise valid preview if the 64 KiB
read boundary split a multibyte code point.

**Implemented:** light preview text; Show/Hide preview button reachable through
ordinary pointer/keyboard/semantic control paths; session expansion preference
surviving resize; synchronized PropertyList header extent; readable unavailable
explanation in the preview body and an empty-text message. The loader retains
only the complete UTF-8 prefix at its read limit, while incomplete actual EOF
remains malformed. Format coverage is still PNG and the original text list.
No thumbnail, JPEG/PDF support or macOS dogfood acceptance is claimed.

**MEASURED, Shadow Windows x64 Release, MinGW toolchain selected by
`tools/Enter-WindowsToolchain.ps1`:**

- New loader regression linked against baseline `89846df` `preview.cpp` failed
  at `read limit inside a UTF-8 code point must retain the complete readable
  prefix`. Temporary comparison source/executable are under `.build/`.
- Updated loader covers every split position of two-, three- and four-byte
  code points, malformed EOF and empty text.
- Assembled Application regression selects files through public semantic
  actions, checks text paint color, actual PNG registry admission and image-draw
  geometry, short-window manual expansion, header-space recovery, retained
  selection, resize preference and unsupported-file explanation without stale
  image content. This is headless paint-command evidence, not native pixel
  decode evidence.
- Full frontend CMake build passed; CTest **12/12**, 3.30 seconds. The focused
  preview/interaction run immediately before the final full build passed 2/2.
- Released Windows program navigated to the generated TXT/PNG fixture through
  its native path editor. Another application became foreground during the
  subsequent selection attempt; native preview inspection was not completed.
  A separate idle sample of that released process was 0.015625 CPU seconds over
  5.0188 wall seconds (~0.31% of one core), 68.55 MiB working set. This single
  background-idle sample says nothing about active interaction smoothness.

**House-style source review:** reviewed the changed preview construction,
adaptation/toggle/completion functions and their state declaration in
`src/application.cpp` / `src/application.hpp`, UTF-8 extent and result assembly
in `src/preview.cpp`, and all newly added test code in
`tests/application_interaction_tests.cpp` / `tests/preview_tests.cpp`. Types and
conversions are explicit; toggle uses a named target on the existing revocable
subscription owner; polling predicates borrow window-owned controls only during
the synchronous test wait; worker ownership/identity/generation checks are
unchanged; bounded UTF-8 validation allocates nothing per character. Test fixture
construction intentionally allocates its small bounded corpus. Scanner on those
five files reported zero spelling candidates; `git diff --check` passed. No
whole-file or whole-library semantic compliance claim is made. Existing
GUI.Forms source and generated Web.Forms output require their own review.

**Remaining:** native macOS reproduction, renderer pixel acceptance, large-image
limits/downsampling, useful format coverage, deferred/background work budgets,
thumbnail/index contracts and the rest of the initial register. A visible
sibling chat, `Audit File Manager Details against interviews`, completed its
audit and now owns the bounded ObjectView Details development model/control
and collection tests. Frontend integration and canonical contract reconciliation
remain with the parent. See `DETAILS_ACCEPTANCE_AUDIT_2026-10-02.md` and
`../../gui_forms/docs/OBJECT_VIEW_DETAILS_DEVELOPMENT_001.md`.

## Second repair: avoid wrapping hidden preview lines

**MEASURED defect:** `Label::measure` and `Label::paint_label_text` previously
called `label_lines` on the entire text, then resized the completed line vector
to `maximum_lines`. In a Windows Release renderer-neutral probe with 65,536
ASCII bytes (`one two three x ` repeated 4,096 times), 190×108 logical bounds,
13-unit font and seven visible lines, one paint issued **32,774** width queries
covering **335,998** bytes before drawing seven lines. The new work-budget
assertion failed on the old implementation.

**Implemented:** the source-private wrapping helper accepts the existing
Label line limit, stops after producing the exact requested prefix, and retains
zero as unlimited. Both measure and paint use it. Named local resolver objects
replace the two anonymous captured callbacks in the touched Label functions;
their control/painter borrows last only for the synchronous wrapping call.

**MEASURED result, same probe:** **92** width queries covering **1,028** bytes,
still seven drawn lines. This is a work-count reduction, not a measured native
latency speedup. Basic-controls tests pass, including a differential corpus
comparing limited output to the exact unlimited prefix across empty text,
blank/trailing lines, tabs/whitespace, long words, CJK, combining marks, joined
emoji, CRLF, no-wrap/word-wrap, three widths and every relevant line limit.

**Scope/review:** changes are confined to
`gui_forms/src/controls/basic/basic_control_rendering.hpp/.cpp`,
`gui_forms/src/controls/label/label.cpp`, and
`gui_forms/tests/basic_controls_tests.cpp`. Source review covers the changed
wrapping traversal, publication/early-return points, named borrowed resolvers
and added fixtures. No per-character allocation was added; existing line
scratch remains reusable inside each paragraph, and returned lines own their
strings. Limit selection happens before traversal. Scanner reports zero
spelling candidates in these four files. Remaining legacy costs include a full
display-string copy, finding paragraph/word ends, and constructing grapheme
metadata for a single oversized unbroken word. No claim of constant work for
all Unicode inputs or complete legacy house-style compliance is made.

Related Windows Release GUI.Forms checks passed **4/4** (basic controls,
collection controls, application contract and menu controls), 0.30 seconds.
That local development build includes the preserved title/menu sibling patch;
the committed native CI build is the clean verification authority for the
separately committed preview/label changes. No shared development SDK was
exported as part of this repair.

## macOS application pixel regression — native PNG failure reproduced

`tests/macos_preview_tests.mm` constructs the actual File Manager Application,
starts the public macOS host and selects generated ordinary UTF-8 TXT, a known
opaque cyan PNG and an unsupported binary-extension file through public
semantic selection. Each stage waits for its real asynchronous completion and
samples the preview control's window-space region in an AppKit bitmap snapshot.
TXT and unsupported explanation require light text pixels; PNG requires its
known cyan pixels. A timeout retains a native snapshot in CI diagnostics.

This closes a test-coverage gap only after it passes on macOS. Snapshot display
can force painting: this test does not establish ordinary pointer delivery,
selection latency, smooth animation, format completeness or acceptance on the
owner's machine. Source-only review is not a native execution result.

The new test, Apple-only CMake target and diagnostic artifact rule were reviewed
for explicit types, named main-queue continuations, retained state ownership,
window borrow lifetime, shutdown, bounded pixel traversal and generated-fixture
cleanup. Main stops the worker before destroying the fixture; the fixture
removes only a newly acquired directory. The color probe allocates an AppKit
snapshot per attempt under an autorelease pool and scans only a bounded preview
region; it is deliberately a correctness test, not a timing harness. Scanner
reports zero candidates in the new Objective-C++ file. Windows CMake
reconfiguration and the unaffected frontend tests pass **12/12**, 2.70 seconds.
Native Mac compile/execution remain **pending** at this checkpoint.

**REJECTED initial harness run:** native CI `36975685152`, source `e123fd8`,
compiled successfully but timed out waiting for text pixels. Its retained
snapshot showed no application text and an automatically collapsed inspector;
host metrics reported an incomplete bundled font pack. The test executable had
been unbundled, while the native host resolves fonts from NSBundle. AppKit also
constrained its requested 1340×850 window to the runner's 1024×674 content view.
This is a harness failure, not proof that the shipped app loses fonts or that
the owner's bug has been reproduced. The corrected test uses an app bundle with
the same installed font resources and invokes the ordinary Properties command
and, if needed, Show preview.

**MEASURED corrected run:** native CI `36976907755`, source `eda1556`,
successfully compiled and painted the selected TXT fixture (1,078 readable
pixels). It then timed out at the PNG stage. The retained native screenshot
shows a visible dark preview, loaded-image caption and no cyan image. The
host reports no native callback faults. This reproduces a native image-preview
failure; a valid resource ID and direct control draw-command test were
insufficient acceptance evidence. It does not yet identify whether layout,
retained replay or native decoding/presentation is responsible. The owner's
multi-format report remains open.

The next diagnostic source `cc2fa2b` also retains the public visual-inspection
JSON on failure. The renderer-neutral application regression now requires
the selected image to appear during full-window retained replay and its
control to remain inside the preview surface. This local check passes, so
it is a narrower control-path check rather than evidence that macOS is fixed.

**OBSERVED root cause, native CI `36978072697`, source `cc2fa2b`:** the
retained JSON shows `fm.inspector.preview.image` visible, unclipped, at
782,312 with size 194×112 and a current image draw command. Its plane is
`backplane`, inherited from Panel. The authored opaque preview surface is in
the later `control` plane. Window replays the whole tree by plane, so the
parent surface covers the image. The absence of PictureBox's white background
in the screenshot is consistent with this ordering, not just decoder failure.

The frontend now assigns its preview PictureBox to the control plane and gives
it a transparent background so the authored dark surface remains the backing.
It leaves the generic Panel/PictureBox contract unchanged. The regression
primes retained chunks with the TXT selection, then checks that full-window
replay draws the selected PNG *after* the opaque preview fill. This assertion
fails against the previous frontend (0.97-second failing test run); a draw-count
assertion alone had passed. Native pixel confirmation of the repair is pending.

**MEASURED repaired Windows Release frontend:** all 12 tests pass in 2.83
seconds, including the previously failing replay-order assertion. Source
review covers the two preview configuration statements, explanatory layering
comment, scalar-only recording-painter order state and the TXT-to-PNG replay
fixture. The new observer adds no allocations or retained borrows; types and
initial state are explicit, and counter updates precede publication of the
observed order. The two changed C++ files have zero spelling candidates.
This scope does not certify the generic painter or the rest of the application.

**MEASURED native confirmation and delivery:** source `b1986ad` passed both
three-platform native runs `36979174514` and `36979177557`. The real macOS
application produced 1,078 readable TXT pixels, 12,544 matching PNG pixels
and 1,291 readable unsupported-explanation pixels. PR #5 merged by rebase as
`90ca7ee`; source and rebased main share tree
`7b185549f332a83ab5d7032982a26b3e9cca06fa`. The exact tested archives were
published as `v0.001-alpha.b1986ad`, with clean source/hash/receipt checks for
all packaged files (40 Linux, 42 Mac, 44 Windows). Portable packaging,
macOS 26 arm64/ad-hoc signing and the wider format/thumbnail/interaction gaps
remain. The later PNG-admission optimization below is not in that release.

## Obsolete preview work retirement

**OBSERVED:** a queued PreviewWork called the loader even after a newer selection
had superseded it. The loader checked cancellation only inside its read loop,
after canonicalization, path/link checks, opening, identity observation and
buffer allocation. Obsolete results were still queued for UI-side rejection.

The job now checks its existing generation/stop token before loading and again
before publishing a UI completion. The loader checks cancellation before path
resolution. UI-side generation/revision checks remain necessary for a selection
change after enqueue and are retained. This does not preempt a blocked OS call,
replace the application's single worker queue or claim an end-to-end latency
improvement.

**MEASURED:** the added already-cancelled request against an unavailable root
fails against `e123fd8`'s loader and passes after this change. It must retire as
cancelled before attempting root resolution. Current Windows Release preview
and assembled-application tests pass **2/2**, 2.39 seconds. Source review covers
the loader's new early exit, the complete PreviewWork call operator and its
existing owned generation token, and the new regression/named callback. No
borrow crosses a queue boundary; no extra per-selection buffer is introduced.
Spelling scanner on those three files reports zero candidates. The rest of
`application_jobs.hpp` is not certified by this scoped review.

## Ordinary-width adaptation and literal file text

The initial Mac harness exposed a product usability issue even though its
missing fonts were a test-packaging defect: the inner 900-unit automatic
collapse threshold hid the inspector at an ordinary 1024-wide desktop despite
space for content and a usable preview. The authored development thresholds
now collapse the outer folder tree below 840 units, and the inner inspector
below 600 units. With default allocations, the selected-file inspector remains
available at 1024×674 and 800×674. These are reversible implementation values
under DDV-007-02, not an owner-approved final breakpoint specification. Existing
user collapse overrides and minimum control sizes remain authoritative.

The assembled-application regression passes both sizes plus its existing short
height/manual-expansion checks and the larger adaptive-layout suite. Native
pixel evidence for this changed default is still pending. It does not establish
every dragged pane-width combination or all screen scales.

**OBSERVED:** Label defaults to mnemonic interpretation. The selected-text
preview now explicitly disables that behavior. A paint regression requires
literal `A&B && C&D`, preventing source code and ordinary ampersands from being
silently changed for display. This also avoids mnemonic parsing of the bounded
preview. The body still copies display text; no allocation-free claim is made.

Source review covers the authored split thresholds, the explicit Label setting,
new fixture stream lifetime/failure check, bounded recording-painter predicate,
and new layout/paint assertions. The two changed C++ files have zero spelling
scanner candidates. This is not certification of all generated UI code or
legacy Label transformation work.

## PNG admission cost

The provider checkpoint `aaca5d0bbbeb5f6454185996dfa6d8f1798a32c9` passed all
three native builds in Actions run 36982408043. Its matching provider/picker
SDK archives are published at release `sdk-aaca5d0` and their archive hashes
and both installed-file fingerprints were independently verified after download.
This provider checkpoint is distinct from the File Manager preview repair
release `v0.001-alpha.b1986ad` and does not contain the later frontend Details
integration or its pending-layout/atomic-sort corrections.

**MEASURED bottleneck:** `Application::apply_preview` calls `Window::load_png`
on the UI thread. Its registry admission validates every chunk with a
bit-at-a-time CRC before copying and hashing the encoded bytes. An isolated
Windows Release source comparison on generated valid, uncompressed 1024-wide
RGB PNGs measured 125.03–127.23 ms admission for 12,588,036 encoded bytes;
validation alone was 109.98–112.23 ms. File I/O and native decode were excluded.

The source-private CRC now uses a compile-time 256-entry (1 KiB) table with
the same reflected polynomial. Same-source/compiler comparisons reduce that
admission to 41.36–43.67 ms and validation to 26.67–27.73 ms. Smaller 0.75/6 MiB
fixtures show the same direction. The largest pair was repeated in reverse
order. This is measured admission work, not a threefold application-speed
claim. Decoding, remaining copies/hash and the still-synchronous ~42 ms large
admission need subsequent scheduling/ownership work.

The reproducible standalone probe, generated fixture tool, exact workload,
sample ranges and scoped house-style review are in
`../../gui_forms/tools/png_admission_probe/README.md`. No public image API,
resource identity, decoder policy, quota or checksum-error behavior changes.
The existing independent bitwise test oracle plus 24 added ancillary-chunk
cases pass in the full PNG registry suite (0.11 seconds, Windows renderer-neutral
build). Native platform checks for this later optimization are pending.
