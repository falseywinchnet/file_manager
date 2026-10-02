# File Manager owner dogfood acceptance audit

Status: **ACTIVE; first source pass, not a completed whole-product audit**.

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
sibling chat, `Audit File Manager Details against interviews`, owns only
`DETAILS_ACCEPTANCE_AUDIT_2026-10-02.md` in its current audit stage.
