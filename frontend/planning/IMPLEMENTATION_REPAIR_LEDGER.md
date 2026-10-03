# File Manager implementation repair ledger

Status: **ACTIVE WORKING RECORD — 0.001-alpha repair toward real daily use**.

## Current owner dogfood correction — 2026-10-02

**GIVEN:** the owner tested the macOS package across multiple file types and
reported no working previews, no thumbnails, slow/unsmooth interaction,
incomplete Details columns and headers, missing content-size folder icons, and
substantial ribbon/interface deviations from the interviews. Treat these as
open acceptance failures. Earlier test passes or visual claims below do not
close them. The exact file-type corpus and macOS reproduction remain pending.

The active checkout is now `C:\Users\Shadow\file_manager`; the root AGENTS.md
Shadow directions supersede this ledger's historical Neo-only workflow.
Current product acceptance is tracked in
[`DOGFOOD_ACCEPTANCE_AUDIT_2026-10-02.md`](DOGFOOD_ACCEPTANCE_AUDIT_2026-10-02.md).
This is an extension of the repair register, not a declaration that earlier
rows are all solved or that the audit is exhaustive.

**2026-10-03 repair checkpoint:** no-replace publication, rename basename/no-op
behavior and cancellable staged copying passed native Windows/macOS/Linux
matrices and are published as `v0.001-alpha.1371514`. See
`../results/2026-10-03-cancellable-copy/DELIVERY.md`. New Folder's missing immediate
naming offer is now implemented with identity-bound refresh and supersession of
deferred focus; five application cases and all 15 Windows suites pass. Native
acceptance and physical-input/visual dogfood remain pending in
`../results/2026-10-03-new-folder/README.md`. The historical state below is not
the latest checkout/release description.

This file is the durable handoff for ongoing implementation. Update it before
and after every material repair so conversation compaction cannot erase why a
change exists, what has actually been proved, or what remains unresolved.

It does not promote the program to 1.0. The owner correction in
[`OWNER_CORRECTION_2026-08-11.md`](OWNER_CORRECTION_2026-08-11.md) remains the
promotion authority. No Git staging, commit, push, or publication is authorized
by this ledger.

## Operating rules

- The Neo checkout is authoritative. Mirror and build with `m4build`; never
  treat the Mini mirror as primary source.
- Browser and native visual inspection happen on the M4 through Screen Sharing,
  never in a browser on the Neo.
- A visible control must perform its admitted operation, open an honest bounded
  surface, or be absent.
- Home and Volumes are the only ordinary daily navigation roots. Do not restore
  Places, Recent Locations, Quick Access, Libraries, or other invented aliases.
- Daily Home/Volumes browsing is read-only unless a later accepted mutation ADR
  explicitly widens ADR-017. Mutations remain confined to an explicit protected
  root plus separate same-volume quarantine.
- Dynamic Open With remains absent until `ORC-HND-001` passes. Plugin commands,
  previews, and application registry entries remain absent until their named
  contracts pass.
- Do not stage, commit, push, or open a PR unless the owner explicitly asks.

## Current authoritative product state

- Visible product version: `0.001-alpha`.
- The current local worktree contains uncommitted frontend repair work and one
  diagnostic-only GUI.Forms demoboard-test edit. This task performed no Git
  write; external automation commit `2fb2520` captured earlier GUI.Forms work
  and is documented at the ledger tail.
- Real read-only Home and `/Volumes` navigation, direct path entry, Back,
  Forward, Up, object activation, tree selection, icon/details view, factual
  sorts, selection/properties, bounded preview, native Open, Terminal Here,
  Copy Path, settings, and About have been exercised on the M4.
- Protected-root New Folder, Rename, staged Copy/Move/Paste, recoverable Delete,
  internal drag, and one-step Undo exist under ADR-017. Their availability must
  not be widened to Home by presentation code.
- The newest post-paint-state M4 gate passed GUI.Forms 62/62, reinstalled that
  exact package, rebuilt File Manager against it, and passed frontend 11/11 in
  1.48 seconds. Installed Engine and Orchestrator filter-only
  probes pass end-to-end on catalogue generation 2. The production Application
  also passes installed-service Criteria controls and installed-machine real
  Home/descendant/Volumes navigation headlessly. FM-R008, FM-R009, FM-R011,
  FM-R013, and Criteria retain native Screen Sharing dogfood gaps; automated
  suites do not close those rows.
- The production Web.Forms HTML and the authoritative concept atlas were both
  opened on the M4. The static HTML is design/build input and is not claimed as
  an interactive runtime.

## Defect and repair register

| ID | Status | Authority | Observed defect | Required repair | Proof required |
|---|---|---|---|---|---|
| FM-R001 | **DONE / reverify after later edits** | Owner correction; DNA-N08 | Prototype used invented Places/Recent navigation. | Remove the concepts and their authored/generated controls; expose honest Home and Volumes. | Source search plus M4 browser/native inspection. |
| FM-R002 | **DONE / reverify after later edits** | Goal; DNA-F01/N05/N08 | Program could not navigate ordinary real folders. | Admit read-only Home and Volumes, keep explicit launch root, implement direct path resolution and history without widening mutation authority. | Real M4 Home/Documents/Volumes navigation plus focused tests. |
| FM-R003 | **DONE / reverify after later edits** | Design DNA; icon verdict | Native shell lacked the approved object/command material vocabulary. | Supply original programmatic House art at native 1x/2x sizes and bind it to object, tree, shelf, navigation, and title identities. | Raster/draw tests plus native M4 inspection. |
| FM-R004 | **DONE / reverify after later edits** | House Composite | Native title/shelf/navigation/status read as generic flat controls. | Apply Watercolor title material, Office Pearl shelf, graphite navigation/status, compact metrics, and correct body/control typography roles. | Browser/native side-by-side inspection and relevant GUI.Forms tests. |
| FM-R005 | **DONE** | Owner correction | Prior work falsely called the product 1.0. | Present `0.001-alpha`; retain picker package 1.0 only as its independent package contract. | About dialog, bundle metadata, source audit. |
| FM-R006 | **DONE / reverify after later edits** | DNA-N08; concept atlas | The live folder tree began as a flat duplicate, and the first hierarchy patch still combined full Home children with a buried Volumes peer. | Match the atlas's explicit root-mode instrument: one honest Home-rooted, Volumes-rooted, or explicit protected-root tree at a time; the header switch keeps the other admitted roots immediately available. Within the selected mode, retained expansion shows real depth and tree selection navigates on one click. | Interaction tests for depth/expansion/mode switch plus real M4 comparison. |
| FM-R007 | **DONE / reverify after later edits** | DNA-P07/P08; concept atlas | The inspector used two competing vertical layout owners and lacked an editable factual Name row. | Preview is PropertyList header content so preview and properties share one scroll owner. Name is inset-editable only for a single protected-scope object and commits through ADR-017 rename identity checks. | PropertyList ownership test, collision/success rename test, and native M4 selection inspection. |
| FM-R008 | **IMPLEMENTED / NATIVE REVERIFY** | Total plan §4.1; DNA-N03/N05/N06 | The old build used flat rectangular buttons and the first retained BreadcrumbTrail painted its chevron joints before overlapping successor faces, erasing them. | Reusable GUI.Forms BreadcrumbTrail now owns stable segments, shared chevron joints painted above faces, bounded overflow, canonical preview, autocomplete/Tab, inline edit, and rollback without changing identity or row geometry. | Headless focus/edit/commit/cancel/overflow/paint-order tests pass; current M4 native comparison remains. |
| FM-R009 | **OPEN** | Total plan §4.2; concept atlas | Shelf anatomy is functional but visually too faint and generic; button labels/icons and group geography do not yet match the prototype's compact Office Pearl structure. | Improve public GUI.Forms command presentation, pressed/disabled states, group captions, and responsive priority collapse without adding unauthorized commands. | Native screenshots at ordinary and narrow sizes; command-by-command interaction test. |
| FM-R010 | **DONE / exact pointer-drag reverify** | Total plan §5.2; concept atlas | macOS drew an ordinary native title strip above the authored Watercolor title, producing two visual title regions. | Added a default-off reusable GUI.Forms full-size-content titlebar mode and exact retained drag-backdrop identity. File Manager reserves traffic-light space inside its authored title; native titled-window identity and controls remain. | GUI.Forms native-property/action tests; isolated M4 visual, retained-input, and physical close dogfood. Screen Sharing could not serve as a physical drag oracle in this session. |
| FM-R011 | **IMPLEMENTED / NATIVE REVERIFY** | DNA-P03 | Splitters originally lacked the required quiet rest grip and pre-activation proximity/focus enlargement. | Reusable private grip now paints a 7×28 rest actuator over a ≤3 px seam, enlarges to 12×42 on bounded proximity/keyboard focus/press, owns complete damage outsets, and File Manager supplies a stable 12 px invisible hit strip. | GUI.Forms paint/input/keyboard/damage tests and installed consumer are green; current M4 Screen Sharing appearance/pointer pass remains. |
| FM-R012 | **OPEN** | DNA-O03/O04; icon audit | House objects exist, but size-specific 16/24/32/48 redrawing and full required taxonomy/unavailable states are incomplete. | Finish size-specific artwork and manifest the original-art license/provenance before distribution. | Pixel/raster tests at each size and source manifest. |
| FM-R013 | **OPEN** | DNA-O05/O09 | Object labels and selection now work, but two-line icon labels, focus/selection distinction, full-name disclosure, and prototype-equivalent grounding need direct evidence. | Close ObjectView wrapping/truncation/focus states and compare sparse/dense folders. | Unicode/long-name tests and native M4 screenshots. |
| FM-R014 | **OPEN / authority-gated** | ADR-008/019; F6 | Daily Home search is honestly unavailable because the installed Engine profile indexes only the protected root. | Do not implement a frontend tree-walk substitute. Add daily roots only through Engine root admission and Orchestrator policy, then expose exact source/currentness/partial truth. | Installed M4 Engine/Orchestrator evidence for an explicitly admitted daily root. |
| FM-R015 | **OPEN / authority-gated** | ADR-020; `ORC-HND-001` | Open uses the native default handler, but Open With is absent. | Freeze and implement the internal handler snapshot/resolution contract before adding the context command. | Canonical contract fixtures, native handler selection, stale/unavailable tests. |
| FM-R016 | **OPEN** | DDV-007-06; GUI.Forms FM-D01–D09 | Only in-window pointer drag to a visible folder is implemented. External inbound/outbound drag, other File Manager windows, preview drag, tree drop, autoscroll, and host-conventional operation negotiation are missing. | Complete reusable typed transfer source/destination support in GUI.Forms and bind it to identity-checked File Manager operations. | Headless typed-drag tests plus real M4 Finder ↔ File Manager and File Manager ↔ File Manager dogfood in protected roots. |
| FM-R017 | **OPEN** | F4/F10; ADR-017 unresolved edges | Protected operations have no crash/restart journal; copy/move collision and complex merge flows are intentionally fail-closed. | Design/approve journal and conflict workflow before widening. Keep destructive work in disposable protected roots. | Fault/crash recovery corpus, collision dialogs/drawer, identity evidence. |
| FM-R018 | **OPEN** | F10 | App is ad-hoc dogfood only, not signed/notarized distribution with setup, update, uninstall, recovery, or rollback. | Build the macOS distribution path only after product controls and daily workflow stabilize. | Clean-machine install/update/uninstall and offline launch evidence. |
| FM-R019 | **OPEN** | F6/F10; verification matrix | No complete performance acceptance: cold/warm launch, first-folder paint, wide folders, input stalls, idle CPU/RSS, damage, service restart. | Define workloads/budgets with owner where missing and measure on the M4. | p50/p95/p99/max and resource records; correctness oracle first. |
| FM-R020 | **OPEN** | F10/F11; DNA adaptation | VoiceOver promotion, host/local high contrast, IME, scale/large text, reduced motion, Windows, and Linux are not proved. | Close macOS daily-use accessibility/adaptation first, then physical Windows/Linux ports against the same product model. | Native assistive-tech/platform campaigns, not Wine-only evidence. |
| FM-R021 | **DONE / historical record corrected** | Owner correction | The existing 2026-08-11 dogfood record said the protected profile “passed 1.0” and claimed About 1.0.0; that evidence is superseded by the owner correction and current 0.001-alpha reality. | The record now identifies itself as superseded protected-slice evidence, links the owner correction and this ledger, preserves observed historical bytes/hashes/version as rejected-build facts, and explicitly denies 1.0/visual/control/daily-use promotion. | Text audit: no present-tense protected-profile 1.0 proof remains in that record. Active About is `0.001-alpha`; active CMake/export versioning is tracked separately. |
| FM-R022 | **OPEN** | Goal; visible-controls rule | Full control-by-control runtime audit must be repeated after each major surface repair; automated tests currently cover only a subset. | Maintain a matrix of every menu, shelf button, context item, path/search control, tree/object action, pane command, property editor, settings row/action, and dialog. Remove or fix every inert/misleading item. | Native M4 pointer and keyboard run paired with focused tests. |

## FM-R009 command-shelf repair notes — 2026-08-13

- **AUTHORITY / SOURCE AUDIT BEFORE EDIT:** The concept atlas and checked-in
  Web.Forms board agree on exactly two permanent shelf groups. “Selection” owns
  `Move / copy` and `Delete`; “Arrange & inspect” owns `View`, current
  `Sort: name`, and `Properties`. The board requires a 66 px shelf, 60 px group
  bodies, 43 px command faces, visible group seams, compact 8 px bold captions,
  vertically stacked 22–28 px House command art and labels, and distinct normal,
  hot, pressed, disabled, and drop-down-open presentation. Production retains
  the exact members and captions and does not contain the rejected Places or
  Recent concepts.
- **OBSERVED AUTOMATED EVIDENCE GAP BEFORE EDIT:** The installed Application
  interaction campaign proves exact membership/labels, command authority,
  menu opening and Escape, mutation refusal, view/sort state, Properties focus,
  medium/narrow overflow membership, and full-width restoration. It does not
  assert the authored 43 px face/caption/seam geometry or record distinct
  normal/hot/pressed/disabled/drop-down-open paint recipes. Therefore its green
  state cannot close FM-R009 or prove visual equivalence. Inspect the compiler
  output and reusable GUI.Forms button/theme recipes, add focused geometry and
  paint-state oracles, then repair the public presentation layer if those
  oracles expose a mismatch. Current native M4 visual comparison remains
  blocked by the Neo Computer Use lock gate.
- **FM-R009 ROOT CAUSE BEFORE EDIT / AUTHORED GROUP WIDTHS WERE ERASED:**
  Generated C++ faithfully carries the Web.Forms geometry: the shelf is 66 px,
  groups are 60 px with authored minimum widths 176 and 306 px, action rows are
  44 px, command faces are 43 px with 72/90 px member floors, and captions are
  13 px. Production then explicitly replaces both group minimum widths with
  zero. At ordinary full-shelf width this compresses the two Office Pearl groups
  to their bare child content, weakening the prototype geography and seams.
  The existing 530 px full/medium collapse boundary already accommodates the
  authored 176 + 306 px groups plus shelf padding/gap, while medium mode retains
  only the 176 px Selection group and narrow mode retains only Commands.
  Restore those two width floors in Application rather than changing generated
  output. Add installed live-layout assertions for 66/60/44/43/13 px anatomy,
  176/306 px group floors, caption-below-actions order, and full geometry after
  narrow-to-wide restoration. Keep all overflow authority tests unchanged.
- **FM-R009 FIRST LIVE-GEOMETRY CAMPAIGN / FAILED:** Restoring the authored
  176/306 px group floors compiled, but the focused M4 Application interaction
  test failed at the new compound Office Pearl geometry assertion in 0.61
  seconds. Existing behavior assertions were not reached, and no shelf pass is
  claimed. Emitted requested/minimum values are insufficient to identify which
  live arranged dimension differs. Temporarily report the exact arranged
  rectangles for shelf, both groups, both action rows, all five command faces,
  and both captions; then repair the specific layout mapping or correct only an
  oracle that confused CSS box geometry with native committed geometry.
- **FM-R009 ARRANGED-RECT DIAGNOSTIC / REAL GROUP OVERLAP FOUND:** The M4
  reported shelf `0,66,1304,66`; Selection `8,3,176,60`; Arrange
  `169,3,306,60`; Selection actions `5,2,147,43`; Arrange actions
  `5,2,240,43`; command faces 72/72 and 72/90 px by 43 px; captions 147/240 px
  by 13 px at y=45. Thus the restored group widths are committed, but Arrange
  starts 15 px before Selection ends. Production still sets each group requested
  width to zero, so the parent flow advances by shrunken content measurement
  before the child clamps to its minimum. Restore authored requested widths
  176/306 as well as minimum widths so measure and arrange use one geometry.
  The 43 px action-row result is not a defect: native auto-sizing collapses its
  44 px CSS container request to the exact 43 px command-face content, and the
  captions begin exactly at that content bottom. Correct that oracle to require
  43 px and nonoverlapping groups with the authored 4 px shelf gap.
- **FM-R009 REQUESTED-WIDTH HYPOTHESIS / REJECTED:** Setting requested group
  widths to 176/306 while retaining `grow_and_shrink` produced the identical
  live rectangles and identical 15 px overlap. The parent cursor still advanced
  from child-content measurement, proving requested width is not authoritative
  in that auto-size mode. Do not add arbitrary padding or alter the collapse
  threshold to mask it. Inspect the reusable FlowLayoutPanel measure contract
  and restore the generated `grow_only` mode if that is the mode that respects
  the authored minimum/requested width while still allowing the explicit
  product projection to hide whole groups at medium/narrow sizes.
- **FM-R009 LAYOUT CONTRACT / MEASURED FIX SELECTED:** GUI.Forms explicitly
  defines `grow_only` as retaining the authored requested extent and
  `grow_and_shrink` as deriving only child/margin/padding content. The generic
  core test already proves that distinction. Web.Forms generated both shelf
  groups as `grow_only`; File Manager had changed both to `grow_and_shrink`,
  creating the parent-measure/child-minimum disagreement and overlap. Restore
  `grow_only` together with the authored requested/minimum widths. Whole-group
  responsive removal remains owned by `update_command_shelf_projection()`, so
  the groups do not need to self-shrink at medium or narrow widths.
- **FM-R009 DOCUMENTED MODE RESTORE / STILL FAILED; REUSABLE OVERRIDE DEFECT:**
  Rebuilding with `grow_only` restored returned the exact same rectangles and
  overlap. Source inspection explains why: base `Control::measure()` preserves
  requested extent in `grow_only`, but `FlowLayoutPanel::measure()` overrides
  that logic and, whenever AutoSize is true, unconditionally returns the
  child-derived layout extent. The later child arrangement still clamps to the
  group minimum, splitting parent cursor measurement from committed geometry.
  This is a reusable GUI.Forms contract defect exposed by valid Web.Forms
  output, not a reason for File Manager-specific spacer padding. Add a focused
  FlowLayoutPanel test proving GrowOnly retains requested extent while
  GrowAndShrink derives child content, repair the override to match the public
  AutoSizeMode contract, then rerun both the GUI.Forms layout test and installed
  File Manager geometry campaign.
- **FM-R009 FOCUSED GUI.FORMS AUTOSIZE CONTRACT / PASSED ON M4:** The public
  FlowLayoutPanel measure override now applies the same AutoSizeMode semantics
  as base Control: GrowOnly preserves requested extent; GrowAndShrink derives
  content; both honor minimum and maximum bounds. The focused layout-panel
  target rebuilt with warnings-as-errors and passed 1/1 in 0.39 seconds on the
  M4. Its new regression proves a 176×60 authored flow containing 72×43 content
  measures 176×60 in GrowOnly and 100×50 in GrowAndShrink when that is the
  explicit minimum. Install this exact GUI.Forms package and rerun the installed
  File Manager geometry/overflow campaign before claiming the shelf repair.
- **FM-R009 FIRST INSTALLED-CONSUMER RETRY / STALE APPLICATION DYLIB:** The
  focused layout build updated `libgui_forms_controls.a`, but did not relink
  `libgui_forms_application.dylib`; install explicitly reported that dylib
  up-to-date. File Manager links through the application runtime carrier, so
  its geometry remained byte-for-byte identical and the same overlap assertion
  failed in 0.40 seconds. This is another package coherence failure, not
  evidence against the focused FlowLayoutPanel repair. Rebuild the GUI.Forms
  application target (or full tree), install the relinked dylib, then rebuild
  and rerun the unchanged installed Application campaign.
- **FM-R009 COHERENT DYLIB DIAGNOSTIC / OVERLAP FIXED; LIVE-STRETCH ORACLE
  CORRECTED:** After relinking and installing `libgui_forms_application.dylib`,
  the rectangles changed: Selection is `8,3,176,63`, Arrange is
  `188,3,306,63`, giving the authored 4 px gap with no overlap. Action rows are
  44 px, all command faces stretch to 44 px, and 13 px captions begin at y=46.
  The reusable fix therefore resolved the real parent-measure/child-arrange
  defect. The test incorrectly required requested/minimum inputs (60 px group,
  43 px face) as final committed sizes even though the authored shelf and both
  nested flow panels use cross-axis `stretch`. Keep requested/minimum mapping
  documented from generated C++, but make the live oracle require shelf 66,
  group 63, row/face 44, caption 13, authored width floors, 4 px nonoverlap,
  and caption-after-actions ordering. Rerun all command/overflow behavior.
- **FM-R009 POST-GEOMETRY CAMPAIGN / MINIMUM COMMANDS FALLBACK FAILED:** With
  the corrected committed-geometry oracle, the ordinary shelf passed and the
  campaign advanced through full/medium/narrow command authority, then failed
  at the 150×150 fallback assertion requiring the sole Commands target to stay
  within the client. This is a real post-repair layout regression until exact
  bounds prove otherwise. Report client size plus the Commands absolute and
  committed rectangles; determine whether the shelf's GrowOnly authored extent,
  the dynamically added overflow button, or parent layout invalidation owns
  the escape. Do not relax the minimum usable-target requirement.
- **FM-R009 MINIMUM FALLBACK DIAGNOSTIC / STANDALONE STRETCH FOUND:** At a
  150×150 client the Commands control was absolute `29,96.4,72,57` and local
  `11,6,72,57`, ending 3.4 px below the client. Its construction owns requested
  and minimum 72×43 but no maximum height, so the shelf's cross-axis stretch
  expands this dynamically added standalone fallback across its full inner
  height. Unlike the permanent buttons, it has no 44 px action-row owner. Cap
  the overflow control at authored command-face height 43 px with maximum
  height 43; preserve its 72 px width and all responsive command membership.
  Rerun the 150 px containment and ordinary/medium/narrow restoration checks.
- **FM-R009 FOCUSED INSTALLED SHELF GEOMETRY + BEHAVIOR / PASSED ON M4:** With
  coherent GUI.Forms GrowOnly measurement, authored 176/306 px group extents,
  and the standalone overflow height capped to its 43 px face, the complete
  Application interaction campaign passed 1/1 in 1.97 seconds. It now proves a
  66 px ordinary shelf; nonoverlapping 176/306 px groups separated by at least
  the authored 4 px gap; stretched 44 px action/button faces; 13 px captions
  below their actions; exact permanent membership and labels; disabled
  mutation refusal; View/Sort/Properties actions; medium and narrow overflow;
  a usable Commands target wholly inside 150×150; and full labelled geometry
  restored after widening. Run all 62 GUI.Forms and all 11 frontend tests before
  staging. Native paint/state comparison remains open.
- **FM-R009 FULL M4 REGRESSION GATES / PASSED:** `git diff --check` was clean,
  the complete GUI.Forms tree rebuilt, and all 62 GUI.Forms CTests passed. The
  exact application dylib/package was installed; the complete File Manager app
  and tests rebuilt against it; and all 11 frontend CTests passed in 2.36
  seconds. The Application interaction campaign passed in 1.60 seconds and the
  dogfood launcher contract passed in 0.20 seconds. This closes automated
  structure, layout, command, and responsive evidence for the current FM-R009
  slice. Stage a new hash-pinned candidate; native ordinary/narrow paint and
  input comparison remains required before FM-R009 can close.
- **FM-R009 CURRENT SHELF-REPAIR CANDIDATE / STAGED + VERIFIED:** The exact
  62/62 + 11/11 green app was staged without overwriting prior candidates as
  `file-manager-dogfood-20260813T121938Z-94f1810857b2.app`. The manifest pins
  executable SHA-256
  `94f1810857b2a98903cf0e610aeb806d1ad234d24a621b4b5fdd9cb83137b0cd`;
  independent hashing matched, deep strict signature verification passed, and
  the executable is arm64. Using the correct launcher contract, daily dry-run
  reported zero arguments and protected read-only reported exactly the bounded
  four root/engine arguments. The exact process is absent. Do not launch it
  through SSH; launch and compare through the M4 Screen Sharing Aqua session
  after the controlling Neo Computer Use lock gate clears.
- **FM-R009 CONSUMER PAINT-STATE ACCEPTANCE BEFORE EDIT:** Generic GUI.Forms
  tests prove authored button-state machinery, but File Manager replaces three
  Web.Forms Buttons with DropDownButtons at runtime. Add installed-consumer
  proof that all five permanent shelf members retain authored recipe overrides;
  normal, hot, pressed, and disabled Office Pearl recipes remain structurally
  distinct after replacement; and opening View adds expanded semantics plus the
  DropDownButton's bounded bottom-edge cue, while closing removes both. This is
  renderer-neutral paint/state evidence only and must not be represented as a
  native screenshot or closure of FM-R009.
- **FM-R009 GENERATED RECIPE AUDIT / MISSING PRODUCT PRESSED RULE:** Generated
  Office Pearl recipes show normal and hot materials differ, and disabled
  differs through its authored text/glyph color, but pressed is byte-for-byte
  equal to hot. The authoritative concept atlas defines a darker reversed
  active gradient, stronger border/inset depth, and one-pixel depression; the
  product Web.Forms CSS omitted `.shelf-command:active`. Web.Forms supports a
  bounded translate and maps it to retained visual offset, already used by its
  command specimens. Add the missing active rule to the authoritative product
  CSS with the atlas colors/depth and `translateY(1px)`. Regenerate/rebuild, then
  require normal != hot, hot != pressed, normal != disabled for all five shelf
  members after runtime replacement. Preserve the open-edge cue assertion.
- **FM-R009 FIRST ACTIVE-RULE COMPILE / WEB.FORMS FAILED CLOSED:** Regeneration
  rejected all five shelf buttons with WFGM086: pressed projection lost CSS
  cascade order for background-color, background-image, and border-color. The
  attempted rule combined the desired active material, inset shadow, and
  translate, but no retained C++ was accepted and no test ran. Inspect the
  compiler's pressed-state cascade restriction and use the admitted selector/
  declaration structure; do not bypass Web.Forms or inject a C++ recipe.
- **FM-R009 PRESSED PROFILE BOUNDARY / BOUNDED PRODUCT RULE SELECTED:** The IR
  retains per-node active/hover material pool references but not enough CSS
  source-order provenance for Stage 2 to prove which overlapping declaration
  wins. The accepted compiler model intentionally composes hover plus
  nonconflicting active deltas. Preserve fail-closed WFGM086: remove active
  background/border declarations that overlap hover, retain `translateY(1px)`
  and the distinct inset active shadow. Pressed then remains visibly and
  structurally distinct through depression/offset while retaining the hot face.
  The atlas's fully reversed pressed gradient is still a named compiler-profile
  fidelity gap; do not hide it or claim native equivalence.
- **FM-R009 FOCUSED CONSUMER PAINT/STATE CAMPAIGN / PASSED ON M4:** Web.Forms
  regenerated the product source with the bounded active rule, the installed
  Application rebuilt, and its interaction campaign passed 1/1 in 2.16 seconds.
  The test proves all five permanent shelf members retain generated Office
  Pearl recipe arrays; normal differs from hot, hot differs from pressed via
  active inset depth/one-pixel visual offset, and disabled differs from normal;
  the three runtime Button-to-DropDownButton replacements preserve those
  arrays. Opening View also adds expanded semantics and exactly one bounded
  bottom-edge cue. This is renderer-neutral proof, not native screenshot
  equivalence. Run the full GUI.Forms/frontend gates and stage a superseding
  candidate from the regenerated app.
- **FM-R009 CLOSE-STATE ORACLE GAP / RECORDED BEFORE EDIT:** The focused
  consumer campaign proves that opening View adds expanded semantics and one
  bounded bottom-edge cue, and its existing action assertions prove the popup
  subsequently closes. It does not repaint the closed control or explicitly
  require expanded semantics to be absent after that close. Add that final
  close-state paint/semantic oracle before treating the open/close acceptance
  as complete; do not infer disappearance from the internal boolean alone.
- **FM-R009 POST-PAINT FULL-GATE OUTPUT LOST TO COMPACTION / STATUS
  UNPROVEN:** A complete GUI.Forms build/test, exact-package install, frontend
  rebuild/test command was issued after the focused paint/state pass, but
  conversation compaction retained neither its final exit code nor its test
  summary. Partial or likely completion is not evidence. Treat the regenerated
  CSS/app as focused-only until the close-state oracle is added and the full
  62-test GUI.Forms plus 11-test frontend gates are rerun with captured final
  summaries. The previously staged `94f1810857b2` candidate predates this CSS
  regeneration and is superseded regardless of that lost command.
- **FM-R009 FIRST CLOSE-STATE FOCUSED COMMAND / ZERO TESTS EXECUTED:** The
  updated interaction target compiled and linked on the M4, but the attempted
  CTest display-name regex matched no registered tests. CTest exited zero while
  explicitly reporting “No tests were found”; that exit is not a product pass.
  Enumerate the frontend registry, rerun the exact Application interaction test
  by its registered name, and retain the new close-state assertions unchanged.
- **FM-R009 CLOSE-STATE FOCUSED CONSUMER PROOF / PASSED ON M4:** The exact
  registered `file_manager_application_interaction_tests` test then ran and
  passed 1/1 in 1.78 seconds on the M4. After View opens, paints its one extra
  bottom-edge cue, and selects Small icons, the installed consumer now proves
  the control is no longer semantically expanded and its repaint returns to the
  exact closed line count. This completes the renderer-neutral open/close state
  oracle; the complete coherent package and frontend regression gates remain
  required before staging.
- **FM-R009 POST-PAINT COHERENT FULL M4 GATES / PASSED:** `git diff --check`
  was clean; GUI.Forms rebuilt from current source and all 62 reusable tests
  passed in 3.04 seconds. That coherent build was installed, the frontend was
  explicitly reconfigured against its package and completely rebuilt, and all
  11 frontend tests passed in 1.48 seconds. The installed Application campaign
  passed in 1.14 seconds and the dogfood launcher contract in 0.21 seconds.
  This supersedes the compaction-lost proof run and the pre-active-CSS
  `94f1810857b2` candidate. Stage and independently verify a fresh candidate;
  native ordinary/narrow/pressed comparison through M4 Screen Sharing remains
  open and the reversed-gradient compiler-profile fidelity gap remains named.
- **FM-R009 FIRST INDEPENDENT CANDIDATE CHECK / WRONG INNER EXECUTABLE NAME:**
  A fresh bundle staged as
  `file-manager-dogfood-20260813T122814Z-574d8c1f60a6.app` with manifest hash
  `574d8c1f60a6fb8bdb5906659b98c5c20435da06d6794c2bad409a514a9317d1`.
  Deep strict signature verification passed and the correct dry-run wrapper
  proved daily zero arguments plus protected four bounded arguments. However,
  the independent direct hash, architecture, and process-absence checks guessed
  `Contents/MacOS/file_manager`; that path does not exist, so those three checks
  produced no evidence. Read `CFBundleExecutable` from the staged Info.plist and
  rerun all three against the actual executable before approving or launching
  the candidate.
- **FM-R009 POST-PAINT CANDIDATE IDENTITY / FULLY VERIFIED, NOT LAUNCHED:**
  `CFBundleExecutable` is `File Manager`. Direct hashing of that actual binary
  exactly matched the manifest hash
  `574d8c1f60a6fb8bdb5906659b98c5c20435da06d6794c2bad409a514a9317d1`;
  it is a Mach-O 64-bit arm64 executable, deep strict signature verification
  passed, daily dry-run has zero arguments, protected dry-run has exactly
  `--root $HOME/Developer/CodexRuns/fmsandbox --engine-root-id fm1-contained`,
  and the exact candidate process is absent. The immutable bundle is
  `file-manager-dogfood-20260813T122814Z-574d8c1f60a6.app`. It has not been
  launched. Launch only from an M4 Aqua Terminal/Desktop through Screen Sharing;
  never use SSH as the GUI launch route.
- **FM-R009 POST-PAINT SCREEN SHARING RETRY / STILL BLOCKED BY NEO GATE:**
  The required Computer Use inspection of `Screen Sharing` again returned “The
  Mac is locked and automatic unlock could not unlock it.” This is the
  controlling Neo automation gate established earlier, not evidence that the
  M4 desktop or candidate is locked. No M4 screenshot, launch, pointer action,
  or keyboard action occurred. Keep the verified candidate absent/unlaunched;
  continue source and automated repair work and retry only after the Neo is
  manually unlocked.

## FM-R012 House-art audit notes — 2026-08-13

- **SOURCE/DECISION AUDIT / IMPLEMENTATION BOUNDARY RECORDED:** The active
  House renderer is original repo-native raster construction and imports no
  external artwork. It currently owns 17 bounded object/command identities and
  independently rasterizes the requested display extent plus a separately
  generated 2× density extent; the green test proves key and density resource
  resolution, not visual redrawing quality. However, DNA-O04 still labels the
  exact 16/24/32/48 ladder **CANDIDATE**, and the icon audit still requires an
  owner decision on whether original File Manager art is licensed under MIT or
  a separate asset license. The proposed open-folder, unavailable, and generic
  plugin kinds are also audit comparison subjects rather than admitted product
  taxonomy. Do not silently add them or claim the provenance gate closed. A
  legitimate next FM-R012 slice needs either owner approval of the size/license
  decision or can remain limited to testing the already-consumed 17/22/42 px
  extents. No House source changed in this audit.

## FM-R013 object-label/focus repair notes — 2026-08-13

- **AUTHORITY / OBSERVED BEFORE EDIT:** DNA-O05 requires a centered object label
  of at most two lines with the full name available on focus/inspection.
  DNA-O07/O08 require selection to use a semantic pair plus explicit boundary
  and keyboard focus to remain distinct from selection and visible without
  color alone. DNA-O09 permits materially richer hover inspection only if it
  does not change selection/opening/authority and has a keyboard equivalent.
  `MATERIAL_VERDICTS_001.md` accepts centered two-line labels and a precise
  dotted keyboard-focus rectangle; the concept atlas uses 78 px cells, a 42 px
  object, 13 px label lines, a selection boundary, and a separate dotted focus
  cue.
- **EXACT IMPLEMENTATION GAP / RECORDED BEFORE EDIT:** Public `ObjectView`
  currently paints one width-elided icon-label line, paints unbounded full text
  in details rows, and draws one solid accent rectangle for focus. It does
  retain independent stable selection and focus IDs and publishes distinct
  semantic selected/focused states, but its painting does not visibly express
  that distinction in the accepted non-color dotted grammar. Hover only changes
  the cell background; neither hover nor keyboard focus discloses a truncated
  full name. The existing `ToolTip` maps static text to one retained control and
  cannot identify/anchor a virtual ObjectView item, so mapping one tooltip to
  the whole collection would be false item-level disclosure.
- **BOUNDED REPAIR SURFACE / RECORDED BEFORE EDIT:** Add UTF-8-safe two-line
  icon-label layout with ellipsis only on the second line when content remains;
  keep details labels single-line and width-bounded. Paint selection with its
  existing semantic fill plus an explicit boundary, and paint independent
  keyboard focus with a precise dotted rectangle gated by the Window keyboard
  focus cue so pointer focus does not masquerade as keyboard focus. Add a
  virtual-item inspection surface that discloses the complete name for a
  truncated hovered item and for the keyboard-focused item, without selecting,
  activating, or granting filesystem authority. Rich metadata beyond exact
  stored item fields remains a separate unresolved product surface.
- **ACCEPTANCE BEFORE EDIT:** Paint tests must prove one/two-line UTF-8-safe
  output, second-line ellipsis, no adjacent-cell or details-column overrun,
  explicit selection boundary, dotted focus independent of selection, pointer
  focus-cue suppression, and full-name inspection for both pointer hover and
  keyboard focus. Existing virtualization, stable-ID multiselection, semantics,
  image-list, and application interaction suites must remain green. Sparse and
  dense native M4 comparison remains required before FM-R013 can close.
- **FM-R013 STARTED / NO SOURCE EDIT YET.**
- **FM-R013 FIRST SOURCE PASS / IMPLEMENTED, UNVERIFIED:** The reusable public
  `ObjectView` now computes UTF-8 scalar-safe icon labels with at most two
  centered lines, prefers balanced word breaks when the complete name fits,
  and appends an ellipsis only to the second line when content remains. Details
  names and secondary fields are width-elided within their owned columns.
  Selection keeps its semantic fill and gains an explicit accent boundary;
  independent focus uses the shared renderer-neutral dotted focus painter only
  while the Window keyboard focus cue is visible. A truncated item paints a
  bounded complete-name inspection bubble when hovered or keyboard-focused;
  this virtual-item surface does not change selection, activation, or domain
  authority. The first exact paint/input campaign has been added but has not
  compiled or run. **Do not call FM-R013 passed.**
- **2026-08-13 FM-R013 FIRST M4 COMPILE / FAILED:** The focused target stopped
  while compiling `object_view.cpp`: the new anonymous-namespace inspection
  painter referenced `with_alpha` before the later
  `using namespace collection_detail` declaration. This is a source-order
  qualification error, not test evidence. Qualify the helper explicitly and
  rerun the unchanged focused campaign.
- **2026-08-13 FM-R013 SECOND M4 COMPILE / PASSED; FOCUSED TEST / FAILED:** The
  qualified source compiled, but the collection test failed at its older
  `icon labels must elide` assertion before reaching the new campaign. Its
  English fixture fits in full across two admitted lines, so an ellipsis is no
  longer the correct result. Keep that fixture as a complete two-line
  reconstruction proof; retain the new deliberately overlong Unicode name as
  the actual second-line-ellipsis oracle. No product pass is claimed.
- **2026-08-13 FM-R013 THIRD FOCUSED TEST / FAILED (FIXTURE SCOPE):** The revised
  complete-name assertion still failed because the shared image-list fixture
  records the adjacent TreeView's `Folder` label before ObjectView's two label
  lines. The prior assertion searched any text and concealed that mixed-control
  trace. Scope reconstruction to the ObjectView suffix while retaining the
  exact two-line/content requirement; product source is unchanged by this
  correction.
- **2026-08-13 FM-R013 FOURTH FOCUSED TEST / FAILED (WRAP ORACLE):** The
  campaign reached the new hover-inspection assertion, which incorrectly
  required wrapping to isolate `identity.txt` as one exact draw command.
  Available width may legally group complete words differently. Replace this
  with the actual invariant: the ordered inspection lines reconstruct the
  complete UTF-8 filename, remain inside a bounded bubble, and leave selection
  unchanged. No source behavior is being relaxed.
- **2026-08-13 FM-R013 FIFTH FOCUSED TEST / FAILED (TRACE OFFSET):** The
  reconstruction oracle still began after two draw commands, but the paint
  trace first contains two label lines for each of two visible objects. The
  inspection bubble begins after four commands. Correct that scope boundary for
  both hover and keyboard checks; the bounded-bubble assertion remains intact.
- **2026-08-13 FM-R013 SIXTH FOCUSED TEST / FAILED; DIAGNOSTIC NEEDED:** The
  corrected four-command offset still failed the compound hover invariant.
  Stop guessing: report the reconstructed name, text-command count, stroke
  count, and first stroke geometry from the exact fixture, then repair the
  specific violated condition. Source behavior remains unpromoted.
- **2026-08-13 FM-R013 HOVER DIAGNOSTIC / SOURCE BEHAVIOR CORRECT:** The exact
  trace reconstructed `Résumé資料 archive evidence package with immutable
  identity.txt`, with seven text draws. It recorded three strokes because both
  document glyphs already own a material outline and the inspection border is
  the final stroke; the assertion had incorrectly classified every stroke as
  inspection chrome. Assert the final stroke's bounded geometry, and require
  the selected-state campaign to add exactly one boundary beyond the two glyph
  outlines. Preserve the diagnostic detail in this ledger, not in normal test
  output.
- **2026-08-13 FM-R013 FOCUSED GUI.FORMS CAMPAIGN / PASSED:** After correcting
  the trace classification without relaxing source behavior,
  `gui_forms_collection_controls_tests` passed 1/1 in 1.09 seconds on the M4.
  The campaign now proves complete two-line use before elision, a deliberately
  overlong mixed Unicode filename with scalar-safe second-line ellipsis,
  complete-name reconstruction on pointer hover and keyboard focus, bounded
  inspection geometry, selection nonmutation, pointer focus-cue suppression,
  explicit selection boundary, dotted keyboard focus, and details-column
  elision. Installed-consumer integration, full regressions, and native sparse/
  dense visual inspection remain; FM-R013 is still OPEN.
- **2026-08-13 FM-R013 POST-PASS DAMAGE/EQUIVALENCE AUDIT / THREE GAPS:** The
  first green slice chose pointer state whenever any item was hovered, so a
  non-truncated hovered item could suppress disclosure for a distinct
  keyboard-focused truncated item. The inspection bubble also requested an
  outer blur shadow without declaring visual outsets; although the collection
  clip bounded damage, the shadow could be visibly cut at an edge. Finally, the
  new paint campaign's keyboard target was also selected, so it did not prove
  the accepted independent focus-versus-selection geometry. Correct the
  inspection priority to hover only when hover actually needs disclosure,
  replace the undeclared outer shadow with bounded inset depth, use compact
  inspection typography to protect ordinary filename lengths, and drive
  Control+Arrow so selection and keyboard focus occupy different cells. These
  post-pass corrections are unverified.
- **2026-08-13 FM-R013 POST-PASS CORRECTIONS / FOCUSED PASSED:** The corrected
  collection target rebuilt and passed 1/1 in 1.10 seconds on the M4. The test
  now places the explicit selected boundary and dotted keyboard focus on
  different cells, proves a non-truncated hovered item cannot suppress the
  keyboard-focused item's full-name disclosure, and keeps all inspection depth
  inside the bounded bubble so no additional visual damage outsets are needed.
  Installed-consumer and full-suite proof remain.
- **2026-08-13 FM-R013 INSTALLED-CONSUMER CONFIGURE / FIRST ATTEMPT FAILED:**
  The complete GUI.Forms source build and install to
  `gui_forms/.build/fm0-install` succeeded. The following frontend configure
  failed because the invocation supplied a relative `GUIForms_DIR`; CMake
  resolved it from the frontend build context and could not find
  `GUIFormsConfig.cmake`. Rerun with the documented absolute remote
  `$PWD/gui_forms/.build/fm0-install/lib/cmake/GUIForms` path. This is command
  construction failure, not product evidence.
- **2026-08-13 FM-R013 INSTALLED-CONSUMER CAMPAIGN / PASSED:** With the
  documented absolute package path, File Manager reconfigured and rebuilt
  against the exact installed GUI.Forms dylib, and
  `file_manager_application_interaction_tests` passed 1/1 in 1.96 seconds on
  the M4. This preserves real navigation, selection, command, tree/inspector,
  mutation-gating, drag, responsive shelf, breadcrumb, and splitter behavior
  through the repaired ObjectView package seam. Full suites and native visual
  dogfood remain.
- **2026-08-13 FM-R013 DURABLE GUI.FORMS RECORD / UPDATED:** ObjectView's
  Markdown and HTML library pages now state the two-line UTF-8 label,
  column-bounded details text, explicit selection boundary, independent dotted
  keyboard focus, complete-name inspection behavior, focused/installed M4
  evidence, and pending current Screen Sharing proof. The existing capture is
  retained as historical visual evidence and is not represented as a current
  FM-R013 screenshot.
- **2026-08-13 FM-R013 FINAL FULL M4 SUITES / PASSED:** Against the exact
  installed corrected public package, the frontend suite passed 10/10 in 1.21
  seconds and the complete GUI.Forms suite passed 62/62 in 15.75 seconds on the
  M4. The runs include File Manager application and picker interactions,
  ObjectView virtualization/Unicode/paint/input evidence, public demoboard,
  damage, focus, semantics, popup, native macOS hosts, renderer boundaries, and
  package policy. `git diff --check` was clean before the campaign. This is
  complete automated proof for the source slice; sparse/dense current native
  visuals and physical hover/keyboard inspection still remain. FM-R013 stays
  OPEN.
- **2026-08-13 FM-R013 FINAL DOGFOOD STAGING / PASSED:** The freshly rebuilt
  arm64 source bundle retained the known strict-signature failure. A new,
  non-overwriting isolated copy was staged at
  `$HOME/Developer/CodexRuns/fmrepair-objectview-final-20260813.app`, ad-hoc
  re-signed with its embedded GUI.Forms dylibs/resources, and passed deep strict
  verification; its executable is arm64. The guard used `/bin/test` as required.
  This bundle contains the final FM-R008/FM-R009/FM-R011/FM-R013 source state
  but has not yet been launched in the M4 Aqua session.
- **2026-08-13 FM-R013 SCREEN SHARING RETRY / NEO GATE STILL REJECTS:** After
  rereading the mandatory Computer Use skill, a fresh
  `sky.get_app_state({app:"Screen Sharing", disableDiff:true})` call was denied
  before AX/framebuffer access with `The Mac is locked and automatic unlock
  could not unlock it`. The earlier bundle-ID-shaped retry had also failed
  earlier in argument approval because the API requires the skill's plain
  `app` property; it performed no UI action. No launch was attempted through
  SSH because that would not enter the M4 logged-in Aqua session. The exact
  repaired bundle remains staged and unlaunched; FM-R008/FM-R009/FM-R011/
  FM-R013 retain their named native dogfood gaps.
- **2026-08-13 FM-R013 POST-SUITE UNICODE AUDIT / GRAPHEME GAP:** The green
  wrapping implementation retreats only to UTF-8 scalar boundaries. That
  preserves valid encoding but can still split an extended grapheme such as a
  decomposed accent, emoji ZWJ family, flag pair, or Indic cluster. GUI.Forms
  already has a Unicode 17.0-conformant public `TextStore` grapheme map, so
  ObjectView must use that boundary authority rather than duplicating a weaker
  scalar loop. Add a ZWJ/combining fixture that proves both line breaks and the
  ellipsis cut are original-name grapheme boundaries, then rerun focused,
  installed-consumer, and complete suites. The prior 10/10 and 62/62 runs are
  retained but no longer final for FM-R013.
- **2026-08-13 FM-R013 GRAPHEME CORRECTION / FOCUSED PASSED:** ObjectView now
  uses the public Unicode 17.0-conformant `TextStore` grapheme map for both
  fitting and ellipsis retreat instead of scanning UTF-8 scalars itself. The
  expanded collection campaign passed 1/1 in 0.87 seconds on the M4, including
  a deliberately overwide emoji ZWJ family cluster that must be omitted whole
  before the ellipsis. Installed-package and complete-suite reruns remain.
- **2026-08-13 FM-R013 GRAPHEME INSTALLED-CONSUMER / PASSED:** The corrected
  GUI.Forms dylib was rebuilt and installed, File Manager relinked against that
  exact package, and `file_manager_application_interaction_tests` passed 1/1 in
  1.20 seconds on the M4. Complete suites must still supersede the earlier
  pre-grapheme counts.
- **2026-08-13 FM-R013 GRAPHEME FINAL FULL SUITES / PASSED:** After installing
  the grapheme-safe public control and relinking consumers, the frontend suite
  passed 10/10 in 1.17 seconds and the complete GUI.Forms suite passed 62/62 in
  2.95 seconds on the M4. `git diff --check` was clean before the runs. These
  counts supersede the earlier pre-grapheme 1.21/15.75-second campaign. Native
  M4 visual dogfood remains the only FM-R013 source-slice gate; the product is
  still 0.001-alpha and FM-R013 stays OPEN.
- **2026-08-13 FM-R013 GRAPHEME FINAL DOGFOOD STAGING / PASSED:** The prior
  `fmrepair-objectview-final-20260813.app` predates the grapheme correction and
  is explicitly stale. A new non-overwriting exact bundle was staged at
  `$HOME/Developer/CodexRuns/fmrepair-objectview-grapheme-final-20260813.app`,
  ad-hoc deep-signed, strict-verified, and confirmed arm64. Use only this newer
  path for the pending Screen Sharing campaign. It remains unlaunched because
  the Neo capture gate is still closed.

## Active edit notes — 2026-08-12

### Tree/inspector repair started

The implementation slice for FM-R006/FM-R007 is syntactically closed but is
intentionally unverified until it compiles and passes tests. It introduces:

- a tree-directory cache and expansion tracking;
- asynchronous enumeration when a tree row expands;
- PropertyList header ownership for the preview block;
- a Name property editor and commit event route;
- single-selection/protected-scope enablement for the Name editor;
- asynchronous rename through `FileOperationService::rename_object`, including
  no-follow identity revalidation, disabled in-flight mutation controls, and
  editor rollback when the operation is refused;
- selected-tree identity threaded through the recursive retained builder.

The earlier compile blockers (missing recursive selected-ID parameter and
missing `commit_property_name`) are resolved in source. Remaining risks before
proof: cache invalidation across rename/navigation, empty directories presented
as expandable, and async expansion ordering. Do not describe this as working
until the focused tests and M4 build pass.

Focused application regressions now pass on the M4. They assert root,
child, and grandchild depth; lazy expansion of a collapsed sibling without
navigating; current-ancestry selection; preview ownership by the PropertyList;
Name-editor protected-scope availability; collision rollback; and successful
identity-checked rename followed by directory refresh.

FM-R006 root cause found after the focused failure: `apply_directory` moved
`snapshot.location` and `snapshot.root` into application state before passing
the same snapshot to `rebuild_tree`. The moved-from location became the tree
cache key, so the expanded active root could not find its children. The code now
copies those two small paths and preserves the snapshot as authoritative cache
input. The focused M4 rerun passed after this repair.

The first green hierarchy still failed live visual comparison because it joined
all admitted roots into one long tree. The concept atlas's actual construction
uses a `Home-rooted ▾` header switch. A second FM-R006 repair is now implemented
but unverified: the tree shows one admitted root hierarchy at a time, while the
retained header menu exposes Home, Volumes, and any explicit protected root.
Switching modes does not navigate the object field; it lazily reads that honest
root for the tree. Focused tests have been expanded for menu availability,
single-root presentation, Home mode, and return to the explicit root.

### Next execution order

1. Repair FM-R008: replace the boxed path buttons with a reusable continuous
   breadcrumb/path instrument that preserves real canonical identity, explicit
   overflow, keyboard focus, and the existing direct-entry mode.
2. Add focused GUI.Forms/application tests for segment activation, edit entry,
   commit/cancel, invalid-path rollback, overflow, and non-stolen focus; run the
   exact M4 suites through the installed package seam.
3. Stage a separately named dogfood bundle and compare the breadcrumb with the
   atlas on the M4 while navigating real Home descendants.
4. Continue writing every failure, partial result, and proof here before moving
   to the next OPEN row.

### FM-R010 custom-chrome repair started

Source inspection confirms the duplicate-title cause: both GUI.Forms macOS
launch paths unconditionally create an ordinary `NSWindowStyleMaskTitled`
window and set a visible native title, while File Manager draws its own 40 px
Watercolor title inside the retained content. There is currently no host option
for a full-size content view or an authored drag band. The repair must keep the
titled mask and its native window buttons; a borderless window is not an
acceptable substitute. This source slice is **STARTED / UNVERIFIED**.

The first implementation slice now adds a default-off
`MacTitlebarPresentation::transparent_full_size_content` option and a retained
stable-ID drag backdrop. Both GUI.Forms launch paths retain the titled window
mask and add only AppKit's full-size-content bit, transparent titlebar, and
hidden visual title. The system title string remains set. Pointer down begins a
native window drag only when the configured retained backdrop itself—not an
interactive descendant—is the hit-test winner. File Manager opts in with its
authored 40 pt title control and reserves 76 pt of leading content space for
the traffic lights. Host snapshots and both macOS host tests now contain
assertions for native flags, resolved drag identity, and default-mode isolation.
This is still **IMPLEMENTED / UNVERIFIED** until the M4 compiles it.

Because Screen Sharing could not synthesize reliable drag/minimize/zoom input,
the native close test now also discovers the actual hosted `NSWindow` by its
preserved system title, moves and resizes its frame, toggles zoom, minimizes,
and restores it on the AppKit main queue before the existing close-veto/close
sequence. This augmentation is **IMPLEMENTED / UNVERIFIED** until its M4 run.

### FM-R008 breadcrumb/path repair started

The final FM-R010 M4 surface visibly confirms the remaining FM-R008 defect:
the location instrument presents separate rectangular `Home` and `./` buttons,
not the prototype's continuous breadcrumb with chevron joints, bounded
overflow, and coherent transition into direct path editing. Source/authority
inspection and reusable primitive selection are now **STARTED / UNVERIFIED**.

Authority inspection rejects another frontend-only button composition. The
chosen reusable boundary is a public retained GUI.Forms `BreadcrumbTrail`:

- one stable outer control identity and one invariant row geometry;
- caller-owned stable segment IDs/text/accessible descriptions;
- continuous shared-edge chevron paint and direct segment hit testing;
- automatic preservation of the first and current tail segments with an
  explicit middle-overflow actuator when width is constrained;
- pointer, keyboard, and virtual semantic activation for visible segments;
- an owned inline `TextBox` that replaces only the trail's presentation, not
  its retained identity or surrounding layout;
- File Manager retains filesystem admissibility, canonical resolution,
  suggestion generation, explicit navigation, and history authority.

This contract is **DECIDED FOR IMPLEMENTATION / UNVERIFIED**. Autocomplete and
canonical-preview presentation will be layered through public popup/label
primitives after the base trail is compiling; they may not be faked inside the
renderer.

Source inspection has now fixed the first implementation seam precisely. The
existing frontend owns one `FlowLayoutPanel`, one unrelated `TextBox`, a fresh
set of index-named flat `Button` children, and separator `Label` children on
every location change. That construction loses path identity when depth/order
changes and makes edit mode a second layout participant. The replacement base
slice will therefore:

- add `BreadcrumbTrail` to GUI.Forms' public collection-control surface and
  installed target, rather than hiding it inside File Manager;
- use path-derived segment identities supplied by File Manager, never display
  indexes;
- retain exactly one owned editor child inside the trail and keep the trail as
  the path host's only layout participant;
- expose segment activation, overflow activation, edit commit, and edit cancel
  as typed events; File Manager maps those events back to admitted paths;
- make the overflow actuator operational by building an owned menu from the
  exact hidden segment identities, not merely painting an ellipsis;
- keep canonical resolution and invalid-path refusal in
  `resolve_navigation_target`; GUI.Forms receives no filesystem authority.

This narrower source contract is **READY TO EDIT / UNVERIFIED**. The first
compile is expected to expose any retained child-layout, accessibility, or
installed-package omissions; record those failures rather than bypassing the
public seam.

The reusable base primitive is now **IMPLEMENTED / UNVERIFIED** in source.
`BreadcrumbTrail` is exported through `collection_controls.hpp`, built in the
GUI.Forms controls target, retains one same-bounds `TextBox`, validates unique
caller identities, realizes segments as virtual semantic buttons, preserves
the first and deepest fitting tail around an explicit middle-overflow actuator,
and routes pointer/Left/Right/Home/End/Enter/Space activation through typed
events. It paints one connected row with shared chevron edges instead of child
button rectangles. This entry deliberately makes no compile, paint, input, or
frontend-integration claim yet. Focused GUI.Forms tests are next; only after
they expose and close base defects will File Manager replace its old controls.

Focused collection-control coverage is now **WRITTEN / UNRUN**. It requires
one retained editor rather than segment controls; first/current preservation
and named hidden identities under width pressure; semantic and keyboard
segment/overflow/edit activation; invariant editor geometry; caller-owned
commit authority; Escape rollback; connected-chevron paint commands; and
duplicate-ID refusal. These assertions are test intent only until the M4 build
and focused run below report exact results.

- **2026-08-13 FM-R008 M4 BASE COMPILE / PASSED:** The Neo source was mirrored
  with `m4build`; CMake regenerated the GUI.Forms target with the new public
  source, compiled `breadcrumb_trail.cpp`, relinked `gui_forms_controls`, and
  built `gui_forms_collection_controls_tests` on arm64 without error. This is
  compile evidence only; no assertion or frontend behavior is promoted yet.
- **2026-08-13 FM-R008 FOCUSED BASE TEST / FAILED:** The focused M4 test ran
  and stopped after 0.67 seconds at `breadcrumb editing must preserve outer
  identity and row geometry`. Earlier assertions in the same test had already
  proved one retained editor plus explicit root/current/overflow/edit semantic
  identities and typed semantic activation. The failure is a retained-layout
  timing defect or an over-specific local-coordinate expectation after the
  editor is revealed; inspect committed outer/editor bounds before changing
  either implementation or oracle. Do not weaken same-row ownership.
- **2026-08-13 FM-R008 FIRST FAILURE DIAGNOSIS / TEST TIMING:** Revealing the
  formerly hidden retained editor correctly invalidates layout; its committed
  bounds remain the prior zero/unarranged value until the next retained layout
  pass. The test sampled between those two events. It now explicitly performs
  that layout pass before comparing the editor's 1 px inset with the unchanged
  trail extent. This timing correction is **UNVERIFIED** and does not relax the
  geometry assertion.
- **2026-08-13 FM-R008 FOCUSED BASE RERUN / PASSED:** After the timing-oracle
  correction, the rebuilt `gui_forms_collection_controls_tests` passed on the
  M4 in 0.75 seconds. This proves the reusable primitive's tested base contract
  in headless GUI.Forms: stable virtual path identities, bounded overflow,
  keyboard/semantic activation, same-row edit commit/cancel, chevron paint
  commands, and invalid duplicate refusal. Frontend replacement, operational
  overflow menu contents, canonical preview, suggestions, full suites, and
  native appearance remain open.

The frontend replacement is now **IMPLEMENTED / UNVERIFIED**. The old
`FlowLayoutPanel`/indexed `Button`/separator `Label` composition and separate
path-host `TextBox` have been removed from application state. File Manager now
installs one `BreadcrumbTrail` with its owned `fm.path.editor`; supplies
path-derived FNV identities mapped to exact admitted paths; rebuilds the model
without rebuilding controls; routes segment activation into existing
navigation/history; creates a real `ContextMenu` of exact hidden paths only
when overflow is invoked; and rolls an edit back to the current location when
the editor loses focus. Commit still enters `resolve_navigation_target`, and
the trail returns to presentation mode before asynchronous enumeration. This
is source intent only until the public GUI.Forms package is installed and the
frontend compiles/tests against that package.

One integration gap was caught before compilation: `focus_observed(false)`
implements click-away only when the clicked destination takes focus. A click
on a non-focusable backdrop would leave the editor active, which does not meet
the owner contract. The repair will add a public observation event for Control
preview-pointer routing (the same retained route already executed by Window),
subscribe File Manager's root, and cancel only a primary down whose target lies
outside the breadcrumb. This is **IDENTIFIED / UNIMPLEMENTED**; focus loss stays
as a secondary keyboard/focus-scope route, not the sole click-away proof.

The click-away gap is now **IMPLEMENTED / UNVERIFIED** through a small public
GUI.Forms observation seam: every control can subscribe to its own participation
in the existing preview-pointer route, while Window remains the sole hit-test
and routing authority. File Manager subscribes only its root and rolls back an
active path edit on primary-down outside the trail's exact absolute bounds.
Clicks inside the trail/editor are excluded; focus loss remains the keyboard or
focus-scope fallback. No behavior claim follows until focused tests exercise a
non-focusable outside target.

- **2026-08-13 FM-R008 GUI.FORMS FULL COMPILE / PASSED:** The complete M4
  GUI.Forms build regenerated and rebuilt all affected core, controls,
  application, ABI, native host, test, and demoboard targets successfully after
  adding the breadcrumb and preview-pointer observation event. The only output
  was the macOS host's already-recorded seven macOS 15 `CVDisplayLink`
  deprecation warnings and the existing duplicate-static-library linker note.
  This is compile evidence; tests after the pointer-route change and installed
  frontend integration remain pending.
- **2026-08-13 FM-R008 INSTALLED PACKAGE REFRESH / PASSED:** CMake installed
  the rebuilt M4 GUI.Forms libraries, updated collection/control headers
  (including `BreadcrumbTrail` and `pointer_preview_observed`), and refreshed
  the application dylib in the existing persistent
  `gui_forms/.build/fm0-install` prefix. File Manager can now be compiled
  against the supported installed seam; that compile has not yet been run.
- **2026-08-13 FM-R008 INSTALLED-CONSUMER COMPILE / PASSED:** The M4
  reconfigured `frontend/build` against the refreshed installed GUI.Forms
  prefix and rebuilt the frontend model, application, document picker, all
  test executables, and the arm64 `File Manager.app` successfully. This proves
  the new public control and observation seam are consumable through the
  installed package. It does not yet prove the new frontend behavior; focused
  assertions are being added before execution.

Focused application coverage is now **WRITTEN / UNRUN** for the integrated
path instrument. It requires one trail-owned editor; path-derived root segment
activation; invalid absolute-path rollback with explicit status; click-away on
a non-focusable status label; relative path entry; real navigation to a
three-segment directory; constrained explicit overflow; a concrete retained
menu row for the hidden exact path; hidden-path navigation; and stable ordinary
object/Up/Back/Forward/tree behavior afterward. No application test claim has
been made yet.

- **2026-08-13 FM-R008 FOCUSED FRONTEND TEST / FAILED:** The integrated test
  compiled, then failed after 1.77 seconds at `primary click on a non-focusable
  outside surface must roll back path editing`. The preceding assertions
  passed: one trail-owned editor, stable root segment navigation, invalid-path
  refusal/status, and editor reopening. The root preview observer did not see
  the synthetic status-label down. Diagnose the retained preview route and the
  clicked geometry; do not replace this with a focusable-target-only test.

- **2026-08-13 FM-R008 CLICK-AWAY FAILURE DIAGNOSIS / INVALID TARGET:** The
  diagnostic rerun showed the status label at absolute y=1005 while this
  headless Window fixture's viewport is 850 pt high. The synthetic click was
  therefore outside the retained root and no preview route existed; editing
  correctly remained active with the uncommitted text. The oracle now targets
  the visible, non-focusable authored title surface at the top of the same
  Window. This preserves the non-focusable click-away requirement and is
  **UNVERIFIED** until rerun.
- **2026-08-13 FM-R008 CLICK-AWAY RERUN / PASSED, OVERFLOW FAILED:** With the
  valid visible non-focusable title target, execution advanced through
  click-away rollback, relative direct entry, and real navigation into
  Documents/Nested. It then failed at `constrained application breadcrumb must
  expose exact hidden path identities`. Setting `maximum_size` after the trail
  had already received its flex layout did not force the parent to reallocate
  the existing slot in this fixture. The behavior under test is overflow, not
  the maximum-size property; resize the Window narrowly enough for the actual
  path host to constrain the trail, then assert committed width and hidden
  identities before invoking overflow.

The focused oracle now applies a 160 pt arranged width directly to the public
trail after preserving its actual parent-relative origin/height, asserts that
exact committed width, exercises the operational overflow path, then lets the
ordinary parent layout restore it. This isolates responsive projection from
the unrelated flex allocator and is **UNVERIFIED**. Native narrow-window
dogfood remains required to prove the parent allocator supplies equivalent
pressure in the product.

- **2026-08-13 FM-R008 FOCUSED FRONTEND RERUN / PASSED:** The rebuilt
  `file_manager_application_interaction_tests` passed on the M4. The run covers
  the integrated trail-owned editor, stable real-path activation, invalid-path
  status/rollback, valid non-focusable click-away rollback, relative direct
  path resolution, deep real-folder navigation, exact hidden identities,
  concrete overflow menu row invocation, hidden-path navigation, and the
  pre-existing object/Up/Back/Forward/tree/property-rename interactions. Full
  frontend and GUI.Forms suites remain pending after the new preview-route
  event; native appearance and parent-driven narrow overflow remain unproved.
- **2026-08-13 FM-R008 EXACT FULL SUITES / PASSED:** On the M4, the installed-
  package frontend suite passed 10/10 in 0.85 seconds and the source GUI.Forms
  suite passed 62/62 in 21.33 seconds under parallel load. The latter includes
  core dispatch, lifecycle, focus-scope, popup, semantics, collection controls,
  both native macOS host tests, and the host-boundary audit after adding
  `pointer_preview_observed`. This is the current exact automated baseline.
  It does not close FM-R008: canonical preview, generation-cancelled
  suggestions/Tab acceptance, native appearance/input, and parent-driven
  narrow overflow still require implementation or evidence.

### FM-R008 direct-entry completion started

The remaining direct-entry work is now **STARTED / UNVERIFIED**. It must not
change the already-proved trail identity or move filesystem authority into
GUI.Forms. File Manager will own a monotonically increasing suggestion
generation, resolve the typed candidate against the current location/Home and
admitted roots, enumerate only the resolved candidate's immediate local parent
on the existing worker, and drop stale/cancelled results before UI publication.
A public anchored popup will present one factual canonical/admissibility label
and a retained `ListBox` of bounded directory suggestions. Tab accepts the
active suggestion into the same editor without navigating; Enter still commits
through `resolve_navigation_target`; Escape/click-away restore the last
location. Absence, invalid roots, read errors, and no matches remain explicit
preview states rather than empty-success autocomplete.

This implementation contract is recorded before source edits. It does not
authorize a tree walk, index substitute, cloud lookup, JavaScript, or a second
path identity.

The reusable input seams for this layer are now **IMPLEMENTED / UNVERIFIED**:
`BreadcrumbTrail` has a caller-controlled `tab_completion_available` state and
an `edit_completion_requested` event; while its owned editor has focus, the
trail's preview-key route consumes Tab only when a real suggestion exists.
`AnchoredPopupLayer` now excludes its exact anchor bounds from click-away so a
user may reposition/select inside the editor while its preview is open. Focused
tests require both behaviors. No compile or runtime claim has been made.

Inspection of Window popup hit testing found that merely returning from the
layer's pointer callback would still leave the full-client overlay as the hit
target and block the editor beneath it. The implementation now makes the
layer's own hit test transparent only over the live exact anchor bounds;
popup-content children remain first-class targets and the rest of the overlay
still reports click-away. The focused popup test now requires the anchor click
to reach the underlying retained control with no dismissal. This correction is
**IMPLEMENTED / UNVERIFIED**.

The File Manager completion layer is now **IMPLEMENTED / UNVERIFIED** in
source. Entering edit mode requests a generation-tagged bounded lookup. The UI
shows an anchored factual resolution/admissibility line for every non-stale
request and up to twelve immediate directory suggestions from the admitted
parent; stale generations are dropped both during enumeration and before UI
publication. Suggestions have path-derived semantic identities, the first real
match is active, Tab fills it without navigation, activation shares that fill
path, and the edit remains inline. Enter/invalid/Escape/click-away keep their
existing authority. No compile/test claim has been made; likely first risks are
popup lifetime while replacing results and focus interaction while the popup
is open.

Pre-compile review found and repaired three popup-state hazards. The suggestion
`ListBox` is deliberately non-focusable because keyboard ownership remains in
the inline editor; otherwise a pointer press would fire editor focus-loss and
tear down the popup before row activation. Popup click-away and Escape now both
perform the required full edit rollback rather than merely hiding suggestions.
Closing clears the popup-owned subscriptions and disables Tab completion, and
suggestion acceptance relies on the editor's one text-change request instead of
starting a duplicate generation. These are **IMPLEMENTED / UNVERIFIED** fixes.

- **2026-08-13 FM-R008 COMPLETION-SEAM COMPILE / PASSED:** The M4 rebuilt the
  GUI.Forms controls library plus the focused collection and popup test
  executables with Tab completion and anchor passthrough. No compile error was
  reported. Focused assertions have not yet run, and the installed package is
  still the prior green base slice.
- **2026-08-13 FM-R008 COMPLETION-SEAM FOCUSED TESTS / PASSED:** The M4
  collection-control test passed in 0.99 seconds and the anchored-popup test
  passed in 0.60 seconds. This proves conditional Tab interception/event
  routing and exact anchor passthrough without popup dismissal. It does not
  prove File Manager generation cancellation, preview content, or suggestion
  acceptance; install and integrated tests remain.
- **2026-08-13 FM-R008 INSTALLED FRONTEND COMPILE / FAILED:** The refreshed
  GUI.Forms install completed, but File Manager compilation stopped in
  `application.cpp`: `FlowLayoutPanel` is a `ContainerControl` and has neither
  `set_background` nor `set_border_style`. The popup content chose the wrong
  public composition type for a painted frame. Repair with a `Panel` frame
  owning a docked/top preview label and fill/bottom suggestion list, or another
  public painted layout owner; do not add unrelated styling methods to
  `FlowLayoutPanel`.

The compile failure is repaired in source by using a painted `Panel` as the
popup frame. Its factual preview label docks top at 28 pt and its non-focusable
suggestion list fills the remaining bounded area; the existing Panel border and
paper background APIs now own appearance. This is **IMPLEMENTED / UNVERIFIED**.

- **2026-08-13 FM-R008 FRONTEND RELINK / FAILED AT INSTALLED DYLIB:** The
  corrected frontend source compiled, but both application consumers failed to
  link `BreadcrumbTrail::set_tab_completion_available(bool)`. The focused
  GUI.Forms build had rebuilt `libgui_forms_controls.a`, while the installed
  `GUIForms::Application` dylib still came from the prior full build and had not
  absorbed that new control symbol. This is installed-artifact skew, not a
  missing implementation. Rebuild the complete GUI.Forms application dylib,
  reinstall it, then relink File Manager; do not bypass the supported dynamic
  package with source/static paths.
- **2026-08-13 FM-R008 ARTIFACT-SKEW REPAIR / COMPILE PASSED:** The complete
  M4 `gui_forms_application` dylib was relinked, installed into the supported
  prefix, and then File Manager plus all frontend consumers rebuilt
  successfully. The linker emitted only the already-recorded duplicate-static-
  archive note while producing GUI.Forms. Completion behavior remains untested.

Integrated completion coverage is now **WRITTEN / UNRUN**. Before the existing
invalid/click-away/overflow campaign, it requires an exact `Resolved:` preview,
a bounded suggestion list, retained editor focus, a `Doc` prefix converging on
`Documents`, Tab filling the canonical directory without navigation, preview
refresh for the filled value, and Escape removing the popup while restoring
the original current location. This test also creates back-to-back generations
through select-all typing; a stale result may not satisfy the latest-prefix
assertion.

- **2026-08-13 FM-R008 INTEGRATED COMPLETION TEST / FAILED:** The test compiled
  and then timed out at `direct path mode must publish a factual canonical
  preview and bounded local suggestions`. Entering edit mode first calls
  `begin_edit(location)`, whose programmatic `set_text` emits `text_changed`
  synchronously and starts a request; `set_path_editing` then started a second
  identical request. Both post work, but the later generation cancels the
  earlier and may leave the popup state missing if the second lifecycle is
  disrupted. Remove the explicit second request and let the one editor change
  event own initial generation; rerun before changing the assertion.

The duplicate initial generation is removed in source. `TextBox::set_text`
remains the single initial request owner via `text_changed`, matching later
typing and suggestion acceptance. This is **IMPLEMENTED / UNVERIFIED**.

- **2026-08-13 FM-R008 INITIAL-PREVIEW RERUN / FAILED AGAIN:** The same
  assertion failed after removing the duplicate request. The actual retained
  state explains why: normal breadcrumb rebuild already keeps the hidden editor
  text synchronized to `location_`, so `begin_edit(location_)` often assigns an
  identical value and `TextBox::set_text` correctly emits no `text_changed`.
  Initial preview therefore needs exactly one request whether the assigned text
  changed or not. Capture equality before `begin_edit`; rely on `text_changed`
  when it changes, and issue the request explicitly only for the equal-value
  case. This preserves one generation in both branches.

- **2026-08-13 FM-R008 INITIAL-PREVIEW THIRD RUN / FAILED:** Even with one
  request in both equal/different branches, the preview/list assertion timed
  out. Add bounded diagnostics for editor state/text, suggestion generation,
  preview presence/text, list presence/count, and popup-layer presence before
  further lifecycle changes. The next repair must follow that observed state;
  do not guess by weakening the required preview/list.

The bounded diagnostic reports: editing remained true, editor text was the
exact current root, and preview/list/layer were all absent. The request passed
its synchronous guards but never published a visible result. To make the async
boundary testable without exposing private state, the worker submission is now
factored into a named `enumerate_path_suggestions` member; the generation and
UI guards remain unchanged. Next add a failure-safe publication path around
enumeration (including exceptions) and record whether the background work ran,
rather than silently relying on the generic worker exception status path.

The suggestion worker now catches its own exceptions and publishes a factual
`Suggestion lookup failed:` preview through the same generation/text guards;
successful enumeration also moves the preview explicitly into the UI closure.
This makes background execution observable without converting an error into an
empty success. It is **IMPLEMENTED / UNVERIFIED**.

The repeated diagnostic is now resolved to an ordering fact in
`BreadcrumbTrail::begin_edit`: it assigns editor text before setting
`editing_ = true`. A changed value can emit `text_changed`, but File Manager's
guard correctly ignores it because edit mode is not active yet; an equal value
emits nothing. Therefore neither branch owns the initial request. The earlier
“duplicate generation” diagnosis was wrong and is preserved above as a failed
hypothesis. `set_path_editing(true)` now always issues exactly one explicit
request after `begin_edit` returns; subsequent user/programmatic changes remain
owned by `text_changed`. This ordering repair is **IMPLEMENTED / UNVERIFIED**.

The preview is now published synchronously as `Reading matching local
folders…` after admissibility/parent resolution and before worker submission.
This is both a UI repair (canonical/admissibility truth does not wait behind
enumeration) and a diagnostic seam: the next run can distinguish synchronous
resolution from asynchronous suggestion publication without private test
access. The generation remains unchanged, so the worker result may replace
only that exact loading state. This is **IMPLEMENTED / UNVERIFIED**.

The synchronous preview still remained absent, exposing the actual integration
root cause: the trail's `./` semantic/pointer activation calls
`BreadcrumbTrail::begin_edit` internally, while File Manager previously heard
only commit/cancel/completion events. Therefore the application suggestion
request never started on the real edit actuator; only direct private calls to
`set_path_editing(true)` would have done so. GUI.Forms now publishes a typed
`edit_started(text)` event after the owned editor is visible/focused. File
Manager subscribes to that one route and the explicit request is removed from
`set_path_editing`, so pointer, keyboard, semantic, and application entry all
start exactly one generation. Focused GUI.Forms coverage now requires that
event. This is **IMPLEMENTED / UNVERIFIED**.

- **2026-08-13 FM-R008 EDIT-START API BUILD / FOCUSED BASE PASSED:** The M4
  rebuilt the GUI.Forms controls/application dylib, and the collection-control
  test passed in 0.52 seconds with one edit-start event. The updated package was
  installed and the integrated File Manager interaction test compiled. The
  integrated executable has not yet run against this repair.
- **2026-08-13 FM-R008 INTEGRATED COMPLETION RERUN / PASSED:** The exact M4
  application interaction executable passed. Its path campaign now includes
  canonical loading/resolved preview, bounded immediate-folder suggestions,
  retained inline-editor focus, newest-prefix convergence on `Documents`, Tab
  fill without navigation, preview refresh, popup Escape rollback, invalid-path
  refusal, non-focusable click-away rollback, relative entry, real deep
  navigation, overflow-menu navigation, and downstream ordinary navigation.
  Full suites and native dogfood remain before FM-R008 can close.

Pre-suite review found two **OPEN / UNVERIFIED** real-world defects in that
green slice. First, `request_path_suggestions` calls
`std::filesystem::is_directory` on the UI thread to decide whether to enumerate
the candidate or its parent; mounted/unresponsive local volumes could stall
input. All existence/type/canonical checks must move into the already
generation-cancelled worker, while the UI may publish only lexical
`Checking:`/admissibility state. Second, the non-focusable suggestion list was
initially suspected to require double-click/default activation. Source
inspection immediately corrected that hypothesis: `ListBox::on_pointer`
already emits `item_activated` on the matching ordinary primary release, so its
existing activation subscription is the required one-click fill path even when
the list declines focus. No extra pointer observer remains, avoiding a double
accept. Moving filesystem type probing off the UI thread remains the material
repair required before final suite/dogfood proof.

- **2026-08-13 FM-R008 ASYNC-PROBE REPAIR / FOCUSED PASSED:** Source audit now
  finds the per-keystroke candidate `is_directory` only inside
  `enumerate_path_suggestions`' worker closure (the remaining constructor-time
  `/Volumes` admission probe is unrelated startup work). The rebuilt integrated
  application test passed on the M4 with lexical `Checking:` followed by the
  same resolved/latest-prefix/Tab/Escape/invalid/click-away/overflow campaign.
  Full suites and native dogfood remain.

The application test now has a real retained pointer campaign for suggestions:
it reopens editing, types `Doc`, waits for the concrete `Documents` virtual row,
dispatches one primary down/up at that row's exact bounds, and requires a single
canonical editor fill with focus still in the inline editor. Escape must then
roll that whole pointer-filled transaction back. This coverage is **WRITTEN /
UNRUN**.

- **2026-08-13 FM-R008 POINTER SUGGESTION RERUN / PASSED:** The rebuilt M4
  interaction test passed the exact one-click row campaign. `ListBox`'s existing
  ordinary pointer activation filled the canonical Documents path once,
  preserved inline editor focus, and Escape rolled it back. No extra pointer
  observer or double-accept path was required.

- **2026-08-13 COMPACTION RECOVERY / BUILD RESULT LOST, NOT INFERRED:** A final
  full GUI.Forms install plus frontend build had been started in tool cell 942,
  but conversation compaction destroyed that cell before its result was
  returned. The cell no longer exists, so this invocation has no knowable
  outcome and is neither a pass nor a failure. Rebuild the exact authoritative
  Neo source on the M4 before running final suites; do not infer freshness from
  surviving build products.
- **2026-08-13 FM-R008 PRE-SUITE AUDIT / UI-THREAD FILESYSTEM PROBE STILL
  OPEN:** Moving `is_directory` into the worker was incomplete. The
  per-keystroke `request_path_suggestions` path still calls
  `resolve_navigation_target` on the UI thread. A lexically out-of-root spelling
  can make that resolver call `std::filesystem::equivalent` repeatedly while it
  walks parents, so an unavailable local mount can still stall input. Move the
  whole admission/equivalent-root rebase into the generation-cancelled worker;
  the UI may only expand and normalize text lexically and publish a factual
  `Checking:` state. This repair is **OPEN / UNVERIFIED**.
- **2026-08-13 FM-R008 ADMISSION-PROBE REPAIR / IMPLEMENTED, UNVERIFIED:** The
  UI path now performs only `~`/relative expansion, lexical normalization, and
  immediate `Checking:` publication. It snapshots admitted roots/current/Home
  into the generation-tagged worker, where `resolve_navigation_target`,
  equivalent-root rebasing, type probing, and bounded directory enumeration now
  run together. Stale work is checked immediately after resolution and again by
  `read_directory`. The integrated campaign now waits for an asynchronous
  `Not admitted:` preview before committing its refusal case. Build and test
  proof remain outstanding.
- **2026-08-13 FM-R008 FINAL SOURCE-SLICE BUILD / PASSED:** The authoritative
  Neo tree was freshly mirrored after compaction. The M4 rebuilt all GUI.Forms
  targets, installed that exact public package into
  `gui_forms/.build/fm0-install`, and rebuilt the installed-package File Manager
  app plus interaction test with exit status 0. This proves compilation and the
  package seam only; complete suite results and native dogfood remain.
- **2026-08-13 FM-R008 FINAL EXACT FULL SUITES / PASSED:** Against that fresh
  installed-package source slice, the M4 frontend suite passed 10/10 in 2.50
  seconds, including the 1.07-second full application interaction campaign.
  The M4 GUI.Forms suite passed 62/62 in 20.82 seconds, including collection,
  popup, focus/semantic, host-boundary, and both native macOS host campaigns.
  These are automated proofs only. A separately staged/signature-verified arm64
  bundle and Screen Sharing visual/input dogfood still gate FM-R008 closure.
- **2026-08-13 FM-R008 FINAL DOGFOOD STAGING / STARTED:** Copy the freshly built
  `frontend/build/File Manager.app` to a new, non-overwriting bundle below the
  M4's `Developer/CodexRuns`, ad-hoc re-sign it after copy, require deep strict
  verification, and confirm its executable is arm64 before launch. No native
  behavior or appearance claim exists yet.
- **2026-08-13 FM-R008 FINAL DOGFOOD STAGING / PASSED:** A new bundle now exists
  at `~/Developer/CodexRuns/fmrepair-breadcrumb-final-20260813.app` on the M4.
  It was copied from the just-tested build, ad-hoc re-signed with embedded
  GUI.Forms dylibs/resources, passed `codesign --verify --deep --strict`, and
  its `File Manager` executable is a 1,910,688-byte arm64 Mach-O. Launch and
  native Screen Sharing verification remain; staging alone proves no controls.
- **2026-08-13 FM-R008 SCREEN SHARING PRELAUNCH / FALSE-LOCK WARNING:** Local
  Screen Sharing's Computer Use accessibility gate again reports that the Mac
  is locked, but direct read-only checks show the M4 console user logged in,
  `screensharingd` running, the existing `5901` AWDL relay listening, and the
  Screen Sharing app still running. This matches the previously observed false
  lock/Neo-denial condition and is not evidence that the M4 itself locked.
  Native framebuffer inspection remains blocked until that local gate yields.
- **2026-08-13 FM-R008 ISOLATION PRECHECK / TWO STALE APP PROCESSES:** Exact M4
  process inspection found prior dogfood PIDs 32315 and 38647 running from the
  separately named custom-chrome bundles. They must be terminated by exact PID
  before launching the new breadcrumb bundle so the visible window and process
  path cannot be misattributed. No broad process kill is authorized or needed.
- **2026-08-13 FM-R008 FINAL ISOLATED LAUNCH / PROCESS PASSED, UI UNSEEN:** The
  two exact stale PIDs exited after TERM. LaunchServices then opened only the
  new verified bundle on the M4, yielding PID 58066 at the exact
  `fmrepair-breadcrumb-final-20260813.app/Contents/MacOS/File Manager` path.
  This proves isolated launch/process identity, not visible layout or input;
  Screen Sharing's local false-lock gate still withholds the framebuffer.
- **2026-08-13 FM-R008 SCREEN SHARING RETARGET / BLOCKED BY NEO GATE:** Both a
  fresh full-state capture addressed by bundle ID and a second capture addressed
  by app name were rejected before returning AX or framebuffer state. An
  attempted `Command-Q` through the mandated Computer Use channel was rejected
  by the same gate before reaching Screen Sharing, so Codex cannot cleanly
  reconnect it itself. Direct checks still show the M4 console live and only the
  exact final File Manager PID 58066 running. The user must release/unlock the
  Neo-side Screen Sharing capture/permission gate; do not misreport this as an
  M4 lock or a File Manager failure. FM-R008 remains OPEN solely for native
  visual/input/parent-driven overflow dogfood.
- **2026-08-13 FM-R008 FINAL SOURCE INTEGRITY / PASSED:** `git diff --check`
  reports no whitespace/error diagnostics. Source inspection confirms the
  per-keystroke path route calls `resolve_navigation_target` only inside the
  worker closure; UI-side resolution calls that remain are startup or explicit
  navigation/tree actions, not typing preview. No Git staging, commit, push,
  PR, or other Git write was performed.
- **2026-08-13 HOTEL NETWORK RESUME / M4 PATH AND APP STILL LIVE:** After the
  network disruption and hotel change, `m4mini-awdl` again reaches macOS 26.5
  arm64. The M4 console user remains logged in, `screensharingd` is running, the
  Neo's existing AWDL `5901` relay is listening, and the sole File Manager
  process remains PID 58066 from the exact final breadcrumb bundle. Thus the
  private relay and app survived; this is connectivity/process evidence only.
- **2026-08-13 HOTEL NETWORK RESUME / NEO CAPTURE GATE STILL REJECTS:** A fresh
  mandatory Computer Use capture of `com.apple.ScreenSharing` was again denied
  before AX or framebuffer state with the local "Mac is locked" message. This
  is the same Neo-side gate previously distinguished from the live M4 console.
  FM-R008 native appearance/input remains unproved. Continue source/test work
  that does not depend on framebuffer access, and retry native dogfood after the
  Neo capture gate is released; do not call the product or the repair complete.
- **2026-08-13 FM-R009 PROTOTYPE/SOURCE AUDIT / EXACT GAP:** The accepted Folder
  atlas and Design DNA require a compact 64-unit Office Pearl shelf whose
  permanent membership is Move/Copy, Delete, View, Sort, and Properties; New
  Folder stays background context and Rename/Open With stay object context.
  Current authored HTML has the correct two groups and five members, and the
  application binds all five to admitted commands. The remaining defect is not
  command invention: the live C++ surface still relies on five ordinary Button
  instances. Move/Copy, View, and Sort open menus but do not expose reusable
  split/dropdown command anatomy; group caption/geography depends on generic
  generated flow layout; pressed/disabled optical states and content-aware
  priority collapse have no focused proof. FM-R009 must repair those exact
  presentation/interaction seams while retaining every admitted command and
  keeping hidden context commands off the permanent shelf.
- **2026-08-13 FM-R009 ACCEPTANCE SURFACE / RECORDED BEFORE EDIT:** Ordinary
  width must show both labelled groups and all five visible text+icon commands.
  Move/Copy, View, and Sort must have an explicit dropdown actuator and keyboard
  menu route; their current state/default action must remain legible. Delete
  must remain visibly discoverable when disabled and destructive only on
  invocation. Properties must reveal/focus the existing factual pane. Narrow
  width must reflow before collapsing lower-priority commands into one honest
  overflow menu, preserve command stable identity/state, and restore the full
  shelf when widened. Proof requires reusable GUI.Forms paint/input/layout
  tests, command-by-command frontend interaction coverage, exact M4 suites, and
  native ordinary/narrow dogfood when Screen Sharing becomes available.
- **2026-08-13 FM-R009 RESPONSIVE AUDIT / ROOT MINIMUM CONTRADICTS GIVEN:** The
  generated native layout still applies a 1080-unit minimum width to the outer
  File Manager form, while DNA-C12 explicitly requires content-aware reflow and
  priority collapse down to a 150 × 150 logical-unit product minimum. The two
  generated shelf groups also retain 176- and 306-unit minimum widths even when
  descendants could collapse. Therefore merely hiding buttons would leave the
  root/group layout unable to shrink honestly. The frontend repair must override
  those product-level generated minima after construction (without changing the
  Web.Forms compiler globally), retain the existing 700/900 pane-collapse
  thresholds, and prove widening restores all shelf members and captions.
- **2026-08-13 FM-R009 REUSABLE CONTROL ROUTE / STARTED:** Extend public
  GUI.Forms `Button` with explicit menu/split dropdown anatomy rather than
  frontend-only arrow text. The control must paint a bounded disclosure region,
  distinguish dropdown versus primary activation, support Enter/Space and
  Alt+Down, publish menu/expand semantics, and reuse normal theme
  pressed/disabled recipes. File Manager will use menu mode for the three
  state/choice commands because their whole action is selection from a menu;
  no ambiguous default Move/Copy operation will be invented. Source is not yet
  edited or tested.
- **2026-08-13 FM-R009 GUI.FORMS DROPDOWN CONTROL / IMPLEMENTED, UNVERIFIED:**
  Added public `DropDownButton` beneath the existing Button hierarchy, exported
  through `basic_controls.hpp` and built as part of `gui_forms_controls`.
  Retained state includes menu/split mode, bounded trailing width, popup-open
  projection, menu request, and close request. It paints its chevron and split
  separator over ordinary authored button recipes, so hot/pressed/disabled/high
  contrast stay theme-owned. Pointer routing distinguishes the split region;
  Enter/Space reuse activation, Alt+Down requests the menu, and semantics expose
  show-menu/expand/collapse. A focused test covers menu and split pointer paths,
  Alt+Down, semantic state/actions, open-edge paint, disabled refusal, and
  invalid geometry. Nothing has compiled or run yet.
- **2026-08-13 FM-R009 GUI.FORMS DROPDOWN CONTROL / FOCUSED PASSED:** The M4
  configured and rebuilt the affected public control library and focused basic
  controls executable. `gui_forms_basic_controls_tests` passed 1/1 in 0.80
  seconds with the new menu/split pointer, Alt+Down, semantic, paint, disabled,
  and validation campaign. This proves the reusable primitive in isolation;
  installed-package integration, the File Manager shelf, full suites, and
  native dogfood remain.
- **2026-08-13 FM-R009 DROPDOWN POST-PASS REVIEW / SPLIT RELEASE-DRIFT BUG:**
  The first green control still reassigned split-region ownership on primary
  release. Because Window qualifies activation by control identity, a press in
  the primary region followed by a release over the disclosure region could
  incorrectly request the menu. Preserve ownership from primary down instead.
  Source now does so, and the focused test adds the cross-region release case;
  this correction is **IMPLEMENTED / UNVERIFIED** until rerun.
- **2026-08-13 FM-R009 FRONTEND SHELF COMPOSITION / IMPLEMENTED, UNVERIFIED:**
  The generated shelf's Move/Copy, View, and Sort buttons are now replaced in
  their existing owners with public GUI.Forms `DropDownButton` controls while
  preserving stable IDs, authored layout/style data, and command ownership.
  Delete and Properties remain direct buttons. The two authored labels remain
  `SELECTION` and `ARRANGE & INSPECT`; current state is published as
  `View: details` and `Sort: Name` without fake arrow glyphs. A new `More` /
  `Commands` dropdown projects hidden permanent commands through the same five
  command authorities. Width policy is full shelf at 530+, Selection plus More
  at 280--529, and one Commands menu below 280. The frontend overrides the
  generated 1080-wide root and fixed group minima locally and the native host
  advertises the DNA-C12 150 x 150 logical minimum. The application test now
  campaigns full, medium, narrow, and restored-wide states, real menu
  invocation, retained sort state, captions, and exact overflow membership.
  This is source/test intent only: neither the corrected primitive nor this
  composition has compiled or run after these edits. Vertical usability at the
  150-unit extreme and native pressed/collapse appearance remain explicit
  dogfood questions, not implied by the minimum-size declaration.
- **2026-08-13 FM-R009 SPLIT RELEASE-DRIFT CORRECTION / FOCUSED PASSED:** The
  corrected public control rebuilt on the M4 and
  `gui_forms_basic_controls_tests` passed 1/1 in 1.00 second. The regression
  now proves a primary-region press remains a primary activation even if the
  pointer releases over the split disclosure region. This supersedes the
  unverified correction status, but does not yet prove installed-package or
  File Manager integration.
- **2026-08-13 FM-R009 INSTALLED-CONSUMER COMPILE / PASSED:** The complete
  GUI.Forms source tree rebuilt on the M4, installed its updated public headers,
  static control library, and application dylib into the existing
  `gui_forms/.build/fm0-install` package prefix, and File Manager reconfigured
  and rebuilt against that exact package. The application bundle and
  `file_manager_application_interaction_tests` linked successfully. This proves
  the generated-control replacement crosses the supported installed seam; it
  is not yet a behavioral or layout pass.
- **2026-08-13 FM-R009 FOCUSED APPLICATION CAMPAIGN / PASSED:** The installed-
  package `file_manager_application_interaction_tests` passed 1/1 in 2.31
  seconds on the M4. It proved the ordinary 1340-unit shelf has the exact two
  labelled groups and real Move/Copy, View, and Sort dropdown controls; View,
  Sort, and Properties invoke retained command authorities; 400 units preserves
  Selection while projecting Arrange/Inspect through More; 220 units projects
  all five permanent members through Commands; Escape closes the popup; and
  widening restores both groups while retaining `Sort: Kind`. This is focused
  retained-tree/input evidence, not native visual proof. Move/Copy menu content,
  disabled mutation refusal, Alt+Down in the composed application, the 150-unit
  vertical extreme, and physical native appearance remain to be audited before
  FM-R009 can close.
- **2026-08-13 FM-R009 POST-GREEN SCOPE AUDIT / PASTE LEAK:** The pre-existing
  Move/Copy popup and the new narrow overflow both include Paste beneath the
  permanent Move/Copy member. DDV-007-03 names the permanent shelf vocabulary
  exactly as Move/Copy, Delete, View, Sort, and Properties; Paste remains in the
  complete menu vocabulary and folder/background context. Keeping Paste inside
  the shelf dropdown makes a sixth operation reachable from permanent chrome
  and weakens the accepted context boundary. Remove Paste from both shelf
  projections while retaining it in Edit/background surfaces. Add a negative
  row assertion so future refactors cannot silently reintroduce it. **OPEN.**
- **2026-08-13 FM-R009 POST-GREEN CONTROL AUDIT / DISCLOSURE DOUBLE RESERVE:**
  `DropDownButton` reserves 22 units in trailing content padding, so inherited
  Button measurement already includes disclosure space. Its auto-size override
  then adds `drop_down_width_` a second time whenever no explicit width is
  authored. Fixed-width File Manager controls hid this defect. Remove the
  second addition and add an auto-size comparison proving dropdown disclosure
  costs only its reserved padding. **OPEN.**
- **2026-08-13 FM-R009 PROOF GAPS / TEST SOURCE NEXT:** Extend the focused
  application campaign to prove the read-only Move/Copy and Delete controls stay
  visible but refuse semantic activation, Alt+Down opens the focused View
  dropdown through the composed Window input route, and a 150 x 150 resize
  retains a usable Commands target inside the client rather than merely
  accepting the host size. These are unrun acceptance additions.
- **2026-08-13 FM-R009 POST-GREEN CORRECTIONS / IMPLEMENTED, UNVERIFIED:**
  Removed Paste from the ordinary Move/Copy shelf popup and the narrow Commands
  projection while retaining its existing Edit/background routes. Removed the
  dropdown measure override that double-counted disclosure space; inherited
  Button measurement now owns the explicit 22-unit trailing padding. The public
  control test compares ordinary and dropdown auto-size widths, and the
  application campaign now covers read-only visible/disabled refusal,
  composed Alt+Down, exact Move/Copy submenu membership in ordinary and narrow
  states, Escape closure, and an in-client usable Commands target at 150 x 150.
  None of these post-green corrections has compiled or run yet.
- **2026-08-13 FM-R009 DROPDOWN AUTO-SIZE CORRECTION / FOCUSED PASSED:** The
  affected GUI.Forms control library rebuilt on the M4 and
  `gui_forms_basic_controls_tests` passed 1/1 in 1.11 seconds. The reusable
  control now proves its disclosure width is reserved exactly once alongside
  the earlier pointer, keyboard, semantic, disabled, and paint campaign. The
  File Manager scope and minimum-size additions remain unrun.
- **2026-08-13 FM-R009 POST-GREEN APPLICATION RERUN / FAILED:** GUI.Forms
  reinstalled and the File Manager application plus interaction executable
  rebuilt successfully, but the focused CTest failed after 0.90 seconds at
  `narrow shelf Move/Copy submenu must not leak Paste into permanent chrome`.
  The assertion currently combines root popup opening, submenu expansion,
  Copy/Move row presence, and Paste absence, so this run does not identify which
  condition failed. Split the proof before changing implementation. Preserve
  this failure even if the next diagnostic run advances.
- **2026-08-13 FM-R009 SPLIT DIAGNOSTIC / TEST ID ASSUMPTION WRONG:** The
  rebuilt focused test again failed, now after 1.06 seconds specifically at
  `narrow shelf Move/Copy submenu must retain Copy`. Root Commands opening,
  Move/Copy row presence, and submenu expansion all passed. Source inspection
  shows ContextMenu gives submenu rows owner-wide identities
  `fm.shelf.more-menu.popup.row.copy` / `.move`; it does not embed the parent
  row ID. The test expected a nonexistent hierarchical ID. Correct only those
  three row lookups and retain the negative Paste assertion. No product change
  follows from this diagnostic.
- **2026-08-13 FM-R009 EXPANDED APPLICATION CAMPAIGN / PASSED:** After correcting
  only the submenu-row identity expectation, the rebuilt installed-package
  `file_manager_application_interaction_tests` passed 1/1 in 1.07 seconds on
  the M4. In addition to the earlier full/medium/narrow/restore campaign, this
  run proves read-only Move/Copy and Delete remain visible but reject semantic
  invocation; composed Alt+Down opens View; ordinary and narrow Move/Copy
  projections contain Copy and Move but no Paste; Escape closes them; and the
  exact 150 x 150 retained client keeps a >=44 x 24 Commands target fully inside
  its bounds. Native host minimum enforcement and visual dogfood remain separate
  evidence requirements.
- **2026-08-13 FM-R009 FULL M4 REGRESSION SUITES / PASSED:** With the corrected
  public dropdown control installed and the final shelf scope test in place,
  the installed-package frontend suite passed 10/10 in 1.09 seconds and the
  GUI.Forms source suite passed 62/62 in 21.01 seconds on the M4. `git diff
  --check` also reported no whitespace/error diagnostics before the run. This
  is complete automated regression evidence for the source slice. Native
  ordinary/narrow visuals, physical pressed feedback, host minimum enforcement,
  and real pointer/menu dogfood still gate FM-R009 closure.
- **2026-08-13 FM-R009 DOGFOOD STAGING / SOURCE SIGNATURE FAILED, ISOLATED COPY
  PASSED:** The freshly rebuilt arm64 `frontend/build/File Manager.app` again
  failed strict signature verification with `code has no resources but
  signature indicates they must be present`. A newly named, non-overwriting
  copy was staged at
  `$HOME/Developer/CodexRuns/fmrepair-shelf-final-20260813.app`, ad-hoc re-signed
  with its resources, and then passed deep strict verification; its executable
  is arm64. The staging shell attempted `/usr/bin/test` (not present on this
  macOS) inside an `if`; because the target was absent, the guarded copy still
  proceeded to a new path and no existing bundle was overwritten. Use `/bin/test`
  or shell `[` in future staging checks. This is staging evidence only; do not
  infer that the bundle launched in Aqua or looks correct.
- **2026-08-13 FM-R009 COMPUTER-USE RETRY / API FALSE START, NO UI ACTION:** The
  first mandatory Screen Sharing inspection called `getAppState` on the
  imported `@oai/sky` module rather than `sky.get_app_state`; the current package
  exposes no such module-level function. The call failed before any screenshot,
  click, launch, or other UI mutation. The Computer Use skill was reread in
  full and the retry used its documented `sky` object API.
- **2026-08-13 FM-R009 COMPUTER-USE RETRY / NEO CAPTURE GATE STILL REJECTS:**
  The corrected fresh call to `sky.get_app_state` for
  `com.apple.ScreenSharing` was denied before framebuffer or AX state with
  `The Mac is locked and automatic unlock could not unlock it`. As established
  by direct M4 console/process checks, this is the Neo-side Computer Use gate,
  not proof that the M4 console is locked. The staged shelf bundle was not
  launched through SSH because an SSH process is not the M4's logged-in Aqua
  session. FM-R009 remains OPEN pending ordinary/narrow/pressed/disabled/menu
  dogfood after this local capture gate clears.
- **2026-08-13 FM-R011 SOURCE/ATLAS AUDIT / EXACT GAP:** Public GUI.Forms
  `SplitContainer` already separates the 3-unit visible seam from a 9-unit hit
  strip, retains pointer capture and constrained drag, leaves the seam
  keyboard-focusable, supports orientation arrows plus Shift increments, owns
  Enter/Space pane collapse, and projects expand/collapse semantics. The
  source-private `SplitterGrip`, however, has no pointer-proximity state. It
  always paints a full-hit-width 9 x 34 tab and keyboard focus adds an accent
  rectangle around the entire pane-height hit strip. That contradicts
  DDV-007-04's small flat mechanical rest grip and pre-activation enlargement.
  The accepted atlas already authors 7 x 28 at rest and 12 x 42 on pointer
  proximity or keyboard focus; DNA-P02 keeps the structural seam <=3 and
  permits a wider invisible target. Use those existing specimen dimensions,
  declare paint outsets for the 12-unit engaged grip, and do not freeze a new
  animation duration in this slice.
- **2026-08-13 FM-R011 ACCEPTANCE SURFACE / RECORDED BEFORE EDIT:** The reusable
  source-private grip must paint a centered 3-unit seam plus a visible 7 x 28
  actuator at rest; hover or keyboard focus must enlarge it to 12 x 42 and
  increase contrast/depth before activation; press may retain that engaged
  treatment; leaving and blurring must restore rest geometry. The 9-unit hit
  target, drag distances, collapse directions/origins, Enter/Space, arrow keys,
  semantic expand/collapse, and horizontal orientation must not regress. Proof
  requires renderer-neutral paint/state/input tests, frontend application seam
  geometry checks, complete M4 suites, and later native pointer/focus dogfood.
  **FM-R011 STARTED / NO SOURCE EDIT YET.**
- **2026-08-13 FM-R011 FIRST SOURCE PASS / REJECTED BEFORE BUILD:** The first
  private-grip edit added 7 x 28 rest and 12 x 42 engaged painting, bounded
  outsets, focus-cue gating, depth, and active press state, but drove hover from
  generic enter/leave on the entire pane-height splitter control. Because the
  retained hit strip spans the full seam, a pointer far above or below the
  center actuator would incorrectly enlarge it. Keep the paint/outset/active
  work, but replace generic seam hover with position-aware proximity against
  the engaged 12 x 42 actuator box. This false start was not compiled or run.
- **2026-08-13 FM-R011 PRIVATE GRIP REPAIR / IMPLEMENTED, UNVERIFIED:** The
  source-private `SplitterGrip` now retains bounded proximity, active-press,
  and keyboard-focus-cue state. It paints a 7 x 28 flat actuator over the
  centered <=3 seam at rest and a 12 x 42 higher-contrast/deeper actuator on
  center proximity, keyboard focus, or press, with maximum visual outsets
  declared independently from the 9-unit hit strip. SplitContainer still owns
  drag/collapse policy and uses the expanded actuator box for collapse input.
  Renderer-neutral tests inspect exact vertical rest/hover/leave/focus/press
  geometry, horizontal transposition, and existing collapse result; the real
  frontend test asserts both application seams consume 3/9 geometry and the
  declared outsets. No source has compiled or run yet.
- **2026-08-13 FM-R011 FIRST M4 COMPILE / FAILED:** The focused target stopped
  while compiling `splitter_grip.cpp`: `member access into incomplete type
  'Window'` at the keyboard focus-cue query. The private implementation relied
  on Control's forward declaration but now needs the public Window definition.
  Add `gui_forms/window.hpp` to that `.cpp` only and rerun the identical target;
  no public API or behavior change is indicated by this failure.
- **2026-08-13 FM-R011 FOCUSED GUI.FORMS CAMPAIGN / PASSED:** After adding the
  implementation-only Window include, the control library and focused split
  executable rebuilt on the M4 and `gui_forms_split_container_tests` passed
  1/1 in 1.09 seconds. The run covers the new exact rest/proximity/leave/focus/
  press/horizontal paint geometry and all pre-existing constrained drag,
  keyboard resize, collapse focus, automatic/user origin, and semantic
  behavior. Installed-consumer integration and native appearance remain.
- **2026-08-13 FM-R011 INSTALLED-CONSUMER INTEGRATION / PASSED:** The complete
  GUI.Forms tree rebuilt on the M4, installed the refreshed controls and
  application dylib into `gui_forms/.build/fm0-install`, and the File Manager
  app/tests relinked against that exact package. The focused installed-package
  `file_manager_application_interaction_tests` passed 1/1 in 2.03 seconds,
  including both real application splitter identities with 9-unit hit geometry
  and 1.5-unit horizontal visual outsets around the 3-unit seam. This is not
  yet the complete regression suite or native visual proof.
- **2026-08-13 FM-R011 POST-PASS DAMAGE AUDIT / SHADOW OUTSETS TOO SMALL:** The
  first green implementation declares only the 1.5-unit body overhang of a
  12-unit actuator over a 9-unit control. Its engaged box shadow uses 3-unit
  blur plus 2-unit downward offset and therefore reaches farther than the
  declared damage/replay bounds. Compute outsets from the union of the expanded
  actuator and its exact shadow envelope, and assert the asymmetric horizontal
  case too. **CORRECTION OPEN.**
- **2026-08-13 FM-R011 POST-PASS HIT AUDIT / ENGAGED EDGE NOT STABLE:** The
  reusable 9-unit test correctly proves out-of-bounds decoration support, but
  File Manager itself still authors a 9-unit hit strip under a 12-unit engaged
  actuator. Moving onto the outer 1.5-unit visual edge changes the hit target
  back to an adjacent pane and can drop hover emphasis; a direct press there
  would grow only after SplitContainer preview activation. Exact hit geometry
  was explicitly a CANDIDATE, not a frozen 9-unit decision. File Manager should
  author a 12-unit invisible strip so its entire engaged grip is a stable
  pre-activation target; the structural seam stays 3 and the rest actuator 7.
  Keep the 9-unit reusable fixture to prove bounded outsets. **CORRECTION OPEN.**
- **2026-08-13 FM-R011 POST-PASS CORRECTIONS / IMPLEMENTED, UNVERIFIED:** The
  grip now derives maximum outsets from the exact expanded actuator plus the
  renderer/theme conservative `blur x 3` shadow envelope and uses the current
  style's dark-border color rather than a fixed charcoal. The real frontend and
  public File Manager demoboard now author a 12-unit invisible splitter strip,
  matching the engaged actuator body while retaining a 3-unit visible seam and
  7 x 28 rest grip. The reusable 9-unit fixture now expects 10.5-unit horizontal
  shadow outsets; its horizontal transpose expects 8.5 top / 12.5 bottom due to
  the 2-unit downward shadow offset. The frontend expects 12-unit hit geometry
  and 9-unit shadow outsets. These corrections have not compiled or run.
- **2026-08-13 FM-R011 POST-PASS CORRECTIONS / FOCUSED PASSED:** The corrected
  control library rebuilt on the M4 and the expanded
  `gui_forms_split_container_tests` passed 1/1 in 0.67 seconds. This supersedes
  the unverified correction status and proves the full shadow envelope in both
  orientations while preserving every earlier grip and splitter behavior.
  The widened File Manager/demoboard consumer geometry still needs installed-
  package compilation and application proof.
- **2026-08-13 FM-R011 CORRECTED INSTALLED-CONSUMER CAMPAIGN / PASSED:** The
  complete GUI.Forms tree and public demoboard rebuilt, the corrected dylib was
  installed, File Manager relinked, and the focused installed-package
  application interaction test passed 1/1 in 2.11 seconds on the M4. Both real
  application seams now prove 12-unit invisible hit strips, 3-unit visible
  policy, and complete 9-unit horizontal shadow outsets. Complete regression
  suites and native pointer/focus appearance still remain.
- **2026-08-13 FM-R011 DURABLE GUI.FORMS RECORD / UPDATED:** The Markdown and
  checked-in HTML library pages no longer describe the older full-strip focus
  cue as current truth. They now record the 7 x 28 rest / 12 x 42 engaged state
  machine, bounded center proximity, active projection, keyboard-visible focus
  gating, visual-outset damage contract, installed-consumer proof, and pending
  current native Screen Sharing evidence. Documentation edits do not add
  behavioral proof.
- **2026-08-13 FM-R011 FINAL FULL M4 SUITES / PASSED:** After the shadow-damage
  and product hit-strip corrections, the installed-package frontend suite
  passed 10/10 in 1.31 seconds and the complete GUI.Forms suite passed 62/62 in
  18.05 seconds on the M4. The GUI.Forms run includes the public File Manager
  demoboard, split-container campaign, invalidation/damage, both native macOS
  host tests, renderer-boundary audits, and package policy. `git diff --check`
  was clean before the run. FM-R011 remains OPEN only for current native
  rest/proximity/focus/press/collapse appearance and physical pointer dogfood;
  automation is not substituted for that observation.
- **2026-08-13 FM-R011 FINAL DOGFOOD STAGING / PASSED:** The freshly rebuilt
  source bundle reproduced the known strict signature failure (`code has no
  resources but signature indicates they must be present`). A new target was
  guarded with `/bin/test`, copied without overwriting prior runs to
  `$HOME/Developer/CodexRuns/fmrepair-grip-final-20260813.app`, ad-hoc re-signed,
  and passed deep strict verification. Its executable is arm64. This combined
  bundle includes the final FM-R008, FM-R009, and FM-R011 source state but has
  not been launched or viewed in the M4 Aqua session.
- **2026-08-13 FM-R011 SCREEN SHARING RETRY / NEO GATE STILL REJECTS:** A fresh
  mandatory `sky.get_app_state` call for `com.apple.ScreenSharing` was again
  denied before framebuffer or AX state with the local automatic-unlock error.
  No coordinate, key, launch, or remote UI action followed. The combined bundle
  remains staged but unlaunched; FM-R008, FM-R009, and FM-R011 all retain their
  explicitly recorded native dogfood gaps. Continue independently testable
  source work without converting the repeated local gate into product proof.

## Evidence log

Append dated, exact evidence here. Do not replace failures with later success.

- **2026-08-12 OBSERVED, M4 Screen Sharing:** production Web.Forms HTML shows
  the intended dense House Composite shell but static placeholders only.
- **2026-08-12 OBSERVED, M4 Screen Sharing:** concept atlas Folder state shows a
  compact hierarchical Home/current-ancestry tree and a one-scroll inspector
  with editable Name/Open-with region.
- **2026-08-12 OBSERVED, M4 Screen Sharing:** current native app navigates the
  real M4 Home but its tree is a long flat duplicate of Home directories; this
  materially diverges from the atlas. The native host also presents a second
  macOS title strip above the authored Watercolor identity.
- **2026-08-12 SOURCE STATE:** no Git staging, commit, push, or publication was
  performed or authorized.
- **2026-08-12 IMPLEMENTED / UNVERIFIED:** FM-R006 recursive hierarchy and
  lazy-expansion source path plus FM-R007 one-scroll inspector and protected
  Name-property rename source path are complete enough to compile. No build or
  runtime claim has been made yet.
- **2026-08-12 TEST SOURCE / UNRUN:** `application_interaction_tests.cpp` now
  contains focused hierarchy, one-scroll ownership, collision-refusal, and
  successful property-rename checks. This is test intent only until the M4 run
  below records a result.
- **2026-08-12 M4 BUILD / FAILED TEST:** Release configuration and compilation
  completed successfully with the installed GUI.Forms package. CTest passed
  9/10; `file_manager_application_interaction_tests` failed after 0.80 seconds
  at `tree must retain an expanded root with real first-level depth`. Preserve
  this failure while determining whether the retained tree state or the test's
  asynchronous readiness condition is wrong. No native behavior is promoted by
  this result.
- **2026-08-12 M4 FOCUSED RERUN / FAILED TEST:** After splitting the compound
  assertion, the active explicit root was present at depth zero and expanded,
  but its first-level `Documents`/`Pictures` rows were absent. The focused
  executable failed at `expanded admitted tree root must retain first-level
  folders`. Treat this as an FM-R006 implementation defect until the exact
  retained model/cache state is identified.
- **2026-08-12 M4 FOCUSED RERUN / ADVANCED, THEN FAILED SETUP:** Preserving the
  snapshot cache key fixed the missing tree children; hierarchy, lazy expansion,
  navigation, and one-scroll assertions advanced. The later protected rename
  fixture then failed during `FileOperationService` construction with
  `operation roots must not traverse symbolic links` because macOS reports the
  temporary directory through lexical `/var` while its canonical path begins
  `/private/var`. Keep the product's no-symlink rule; canonicalize the test
  fixture roots before the next rerun.
- **2026-08-12 M4 FOCUSED RERUN / PASSED:** After canonicalizing only the
  disposable fixture paths, `file_manager_application_interaction_tests`
  compiled and passed. The run exercised retained root/child/grandchild depth,
  lazy sibling expansion, single-click tree navigation, PropertyList ownership
  of preview content, protected Name editing, destination-collision rollback,
  and successful filesystem rename/refresh. Full suites and live M4 visual
  inspection remain pending.
- **2026-08-12 M4 FULL SUITES / PASSED:** Release frontend build completed;
  frontend CTest passed 10/10 in 0.59 seconds, including the expanded
  application interaction suite. GUI.Forms CTest passed 62/62 in 2.00 seconds.
  Automated proof now covers FM-R006/FM-R007's focused behavior; live M4 visual
  and pointer/keyboard dogfood are still required before closing either row.
- **2026-08-12 M4 DOGFOOD STAGING / SIGNATURE FAILED:** The new, separate
  `CodexRuns/fmrepair-tree-inspector-20260812.app` copied from the green build,
  but `codesign --verify --deep --strict` reported `code has no resources but
  signature indicates they must be present`. Existing running and rollback
  bundles were not overwritten. Re-sign only this dogfood copy ad hoc, then
  verify again before GUI launch.
- **2026-08-12 M4 DOGFOOD STAGING / READY:** The separate dogfood bundle was
  re-signed ad hoc with a resource seal and then passed deep strict code-sign
  verification. It is an arm64 bundle at
  `CodexRuns/fmrepair-tree-inspector-20260812.app`; no prior bundle was replaced.
- **2026-08-12 SCREEN SHARING LAUNCH ATTEMPT / NO EFFECT:** The M4 framebuffer
  remained live and interactive-looking, but coordinate clicks on the visible
  remote Terminal/Dock did not transfer application focus; typed launch text
  did not reach a shell, and process inspection still showed only the older
  `fmverify3.app` PID 20187. Use M4 LaunchServices over the M4 SSH session only
  to start this native bundle, then perform inspection/interaction through
  Screen Sharing. Do not infer that the repaired bundle was already visible.
- **2026-08-12 M4 LIVE DOGFOOD / FM-R006 FAILED VISUALLY:** M4 LaunchServices
  started the separate repaired arm64 bundle as foreground PID 24595 and Screen
  Sharing showed it. The tree rows now have real retained hierarchy in tests,
  but the Home root still reveals every direct Home directory before the
  Volumes peer root. The sidebar remains a long duplicate of the current Home
  object field, and Volumes remains off-screen. Do not close FM-R006. Tighten
  root presentation to match the atlas's `Home-rooted ▾` mode switch: one
  honest root tree at a time, with the other admitted roots immediately
  available from that retained header control.
- **2026-08-12 M4 ROOT-MODE FOCUSED TEST / PASSED:** The revised application
  interaction executable compiled and passed after adding the retained tree
  root-mode header/menu. Its assertions now cover a single-root model, immediate
  Home and Volumes menu rows, Home-rooted switching, return to the explicit
  protected-root mode, lazy expansion, navigation, inspector ownership, and
  protected property rename. Full suites and a second live comparison remain.
- **2026-08-12 M4 ROOT-MODE FULL SUITES / PASSED:** The rebuilt root-mode slice
  passed frontend CTest 10/10 in 0.40 seconds and GUI.Forms CTest 62/62 in 2.03
  seconds. This supersedes no earlier negative visual evidence; the second live
  comparison is still required.
- **2026-08-12 M4 SECOND LIVE DOGFOOD / FM-R006 STILL FAILED:** The separately
  staged, verified bundle launched as foreground arm64 PID 27355. Screen Sharing
  visibly confirmed FM-R007's new Name row, but the sidebar still showed the old
  static `FOLDERS · HOME / VOLUMES` caption and the full Home child list; the
  root-mode button was not visible. This conflicts with the headless semantic
  test, so FM-R006 remains open. Inspect generated layout/retained child
  ownership and do not treat semantic findability as visual presence.
- **2026-08-12 M4 PROCESS ISOLATION / LAYOUT ROOT CAUSE:** After terminating the
  two older File Manager processes, the new PID's surface was unambiguous: the
  old static caption was absent and FM-R007's Name row was visible, but the new
  root-mode header had collapsed to zero height. The retained model was present;
  visual layout was not. Added explicit 27 px header/caption/button minima and a
  committed-arranged-bounds assertion so future headless proof requires actual
  visible geometry.
- **2026-08-12 M4 MINIMUM-GEOMETRY RERUN / STILL CLIPPED:** The focused test
  passed with explicit arranged geometry, but the fresh isolated native bundle
  still clipped the nested header above the sidebar viewport. The control was
  arranged outside the visible clip. Simplified the composition: preserve the
  authored `FOLDERS` caption and insert the retained mode button as the first
  top-down child of the already-visible tree host, immediately above TreeView.
  This latest composition is unverified.
- **2026-08-12 M4 ISOLATED LIVE DOGFOOD / FM-R006 + FM-R007 PASSED:** The latest
  separately staged and verified arm64 bundle launched as the only File Manager
  process (PID 28671). Screen Sharing showed the authored `FOLDERS` caption, a
  visible `Home-rooted ▾` control, and the real hierarchy beneath it. Pointer
  activation opened a retained menu containing Home-rooted and Volumes-rooted;
  choosing Volumes replaced the tree with the honest `/Volumes` hierarchy while
  the center remained at its current Home location. Selecting a real Home folder
  populated the one-owner inspector with Name, kind, exact path, size, and
  modified rows. The Name editor was visibly disabled in daily read-only Home,
  consistent with ADR-017. Focused tests separately proved enabled protected
  rename, collision rollback, and success. FM-R006/FM-R007 are closed subject
  to normal revalidation after later surface changes.
- **2026-08-12 M4 EXACT-LAYOUT FULL SUITES / PASSED:** After the final visible
  tree-host composition, frontend CTest passed 10/10 in 0.38 seconds and
  GUI.Forms CTest passed 62/62 in 1.99 seconds. This is the automated result for
  the same source slice visually exercised above.
- **2026-08-12 FM-R010 SOURCE INSPECTION / STARTED:** `MacHostOptions` has no
  titlebar presentation or drag-region field. `run_macos` and
  `run_macos_application` both create titled native windows, set a visible
  native title, and install the retained view below it. The retained view sends
  every primary mouse-down/drag into the portable pointer path, so any custom
  drag handling must be explicitly bounded and must not silently convert the
  entire content surface into a window drag target. No implementation or test
  claim exists yet.
- **2026-08-12 FM-R010 IMPLEMENTED / UNVERIFIED:** Added the default-off host
  presentation option and exact retained drag-backdrop identity to public
  GUI.Forms macOS options; applied them in single- and multi-window hosts;
  extended host diagnostics; opted File Manager into its authored title
  backdrop; and added single-/multi-window assertions. The M4 has not compiled
  or run this source yet, so it is not evidence that duplicate chrome is fixed.
- **2026-08-12 FM-R010 M4 COMPILE / PASSED:** `cmake --build
  gui_forms/build --parallel 10` completed on the M4, including the macOS host,
  application dylib, single-window test, and multi-window test. The compiler
  emitted only the host's pre-existing macOS 15 `CVDisplayLink` deprecation
  warnings. No test or visible-window claim follows from compilation alone.
- **2026-08-12 FM-R010 M4 FOCUSED HOST TESTS / PASSED:** CTest ran
  `gui_forms_macos_host_close_tests` and
  `gui_forms_macos_multi_window_tests`; both passed (2/2 in 3.41 seconds).
  These tests inspected the live native window configuration through the final
  host snapshot and proved opt-in/default isolation. They do not prove visual
  placement or physical pointer dragging; Screen Sharing dogfood remains.
- **2026-08-12 FM-R010 INSTALLED-CONSUMER BUILD / PASSED:** Installed the
  rebuilt GUI.Forms artifacts into the existing remote
  `gui_forms/.build/fm0-install` prefix, reconfigured `frontend/build` against
  that exact installed package, and rebuilt File Manager plus its test targets
  successfully. This proves the new public option crosses the supported
  package seam; runtime visual proof remains pending.
- **2026-08-12 FM-R010 FULL M4 SUITES / PASSED:** The installed-package
  frontend passed 10/10 tests in 4.30 seconds, including application
  interactions; the source GUI.Forms build passed 62/62 in 4.35 seconds,
  including both native macOS host tests and the host-boundary audit. Live
  window-control, drag, resize, and visual comparison remain before FM-R010 can
  close.
- **2026-08-12 FM-R010 DOGFOOD STAGING / SOURCE SIGNATURE FAILED, COPY READY:**
  The freshly built app bundle again failed strict verification before staging
  with `code has no resources but signature indicates they must be present`.
  A separately named copy at
  `CodexRuns/fmrepair-custom-chrome-20260812.app` was created without replacing
  prior bundles, ad-hoc re-signed with its resources, and then passed deep
  strict verification. Its executable is arm64. This is staging evidence only.
- **2026-08-13 FM-R010 FIRST LIVE COMPARISON / VISUAL PASS, DRAG UNPROVED:**
  After closing the prior dogfood window, the separately staged bundle launched
  as the sole File Manager process (PID 32315). Screen Sharing shows one
  continuous Watercolor title region with native red/yellow/green controls
  embedded at the left and the House mark/title clear of them; the prior gray
  strip and duplicated `File Manager` title are absent. One synthesized drag
  from a blank point in the authored title did not move the window. Do not call
  movement passed until the same relay is controlled against an ordinary native
  titlebar and the host path is corrected or independently observed.
- **2026-08-13 SCREEN SHARING DRAG CONTROL / RELAY-LIMITED:** Repeated the same
  synthesized drag against the visible ordinary Terminal titlebar in the M4
  remote desktop. Terminal also did not move. This invalidates Screen Sharing
  coordinate-drag as a movement oracle in this session; it neither condemns nor
  proves GUI.Forms dragging. Keep the host's exact-target drag code and native
  configuration tests, but record physical movement as unproved until a relay
  capable of press/hold/move or direct owner observation is available.
- **2026-08-13 SCREEN SHARING WINDOW-CONTROL INPUT / RELAY-LIMITED:** The native
  red/yellow/green controls are visibly present and respond to pointer hover,
  but synthesized traffic-light clicks and `Command-M` did not complete a
  window action through this Screen Sharing session. Automated host diagnostics
  are being strengthened to assert that all three standard AppKit buttons and
  the system title still exist. Do not claim physical minimize/zoom from this
  relay observation.
- **2026-08-13 M4 NATIVE CLOSE / PASSED:** A fresh exact click on the embedded
  red AppKit control produced the native closing animation and removed the File
  Manager window/process cleanly. This physically proves the close button and
  existing close lifecycle survived the full-size-content presentation. It
  does not upgrade the unproved relay drag/minimize/zoom observations.
- **2026-08-13 FM-R010 NATIVE ACTION PROBE / IMPLEMENTED, UNVERIFIED:** Extended
  the focused single-window test to find the real titled host window and
  programmatically exercise frame movement, frame resize, zoom toggle,
  miniaturize, and deminiaturize on AppKit's main queue before the existing
  close test. No action is claimed until this test compiles and passes on M4.
- **2026-08-13 FM-R010 NATIVE ACTION PROBE / FAILED:** The augmented focused
  test compiled, then failed on M4 after 3.42 seconds (CTest 0/1). Its initial
  combined exit code did not identify which native transition was missing.
  Preserve this failure; add per-action diagnostics and repair the probe or host
  behavior based on the exact result, without dropping any required action.
- **2026-08-13 FM-R010 NATIVE ACTION PROBE / DIAGNOSED:** The diagnostic rerun
  again failed, reporting `found=1 moved=1 resized=1 zoomed=1 minimized=0
  restored=1`. The host operations are not the source of this failure:
  `miniaturize:` commits asynchronously, while the first probe sampled
  `isMiniaturized` in the same main-queue turn. The probe now waits one bounded
  main-queue interval before sampling minimize, then restores and samples after
  a second interval. This timing repair is unverified.
- **2026-08-13 FM-R010 NATIVE ACTION PROBE / PASSED:** After respecting AppKit's
  asynchronous miniaturization commit, the focused test passed on M4 in 3.40
  seconds. The actual hosted full-size-content `NSWindow` was found through its
  preserved system title, moved, resized, zoomed, minimized, restored, then
  exercised through the existing vetoed-first/accepted-second close lifecycle.
  This supplies deterministic native action evidence where the Screen Sharing
  relay could not synthesize reliable press/hold or minimize/zoom input.
- **2026-08-13 FM-R010 FULL SUITES / GUI.FORMS FAILED 61/62:** The final
  installed frontend suite passed 10/10 in 3.98 seconds. The parallel GUI.Forms
  suite passed 61/62; `gui_forms_macos_host_close_tests` failed because the new
  native action probe minimized the window before the pre-existing scheduler
  campaign painted (`paints=0`) under parallel load. Its eight scheduler cycles
  all advanced, and the native action probe had passed alone. Treat this as
  test interference, not a green product result. The probe is now sequenced
  after all eight scheduler cycles and owns the final close; rerun focused and
  parallel campaigns.
- **2026-08-13 FM-R010 ORDERING REPAIR / FOCUSED + FULL PASSED:** After moving
  the native action sequence behind the eight existing scheduler cycles, the
  focused macOS host test passed in 3.51 seconds and the parallel GUI.Forms
  suite passed 62/62 in 3.39 seconds. The corresponding installed frontend
  suite had already passed 10/10 in 3.98 seconds. This is the final automated
  result for the source slice; restage and visually relaunch that exact build.
- **2026-08-13 FINAL DOGFOOD LAUNCH / SCREEN SHARING INPUT FAILED:** The final
  arm64 bundle was copied separately to
  `CodexRuns/fmrepair-custom-chrome-final-20260813.app`, ad-hoc re-signed, and
  passed deep strict verification. The first in-session Terminal launch attempt
  lost/mutated a quote through Screen Sharing and stopped at a `dquote>`
  continuation prompt; it did not launch the app. Cancel and retry the same
  no-space path without quotes. This is relay evidence, not a product failure.
- **2026-08-13 FM-R010 FINAL ISOLATED DOGFOOD / PASSED WITH NAMED LIMIT:** The
  exact final arm64 bundle passed deep strict signature verification and
  launched as the sole File Manager process. Screen Sharing again showed one
  continuous Watercolor title region, embedded native traffic lights with no
  overlap, no gray duplicate strip, and intact Home hierarchy/inspector
  geography. The Home-rooted retained control accepted keyboard focus and
  opened its Home/Volumes menu, proving ordinary retained input remained routed
  below the custom chrome. A live red-button click had already closed the prior
  behavior-equivalent slice cleanly. The native M4 test deterministically proved
  preserved close/minimize/zoom buttons, system title, move, resize, zoom,
  minimize, restore, close veto, and accepted close. Only physical pointer
  dragging remains a named reverify because the Screen Sharing drag primitive
  also failed against Terminal. FM-R010 is closed subject to that narrow
  reverify and normal later regression checks.
- **2026-08-13 FM-R008 LIVE BASELINE / STARTED:** On the same exact final M4
  surface, the location row still presents isolated boxed `Home` and `./`
  controls. This visibly diverges from the atlas's continuous chevron path
  instrument and is now the active repair. No breadcrumb implementation claim
  has been made.

## FM-R022 visible-control audit — 2026-08-13

This matrix is the durable working inventory. `SOURCE` means the retained
command is bound to a concrete handler and its state source has been inspected;
it is not a substitute for interaction proof. `AUTO` names current focused
automation. `NATIVE PENDING` means that the current exact build still needs an
M4 pointer/keyboard pass. Keep a row here even when a control is removed.

| Visible surface | Admitted operation | Current evidence | Open edge |
|---|---|---|---|
| File → Open | Open the one selected nonsymlink object through the host default handler. | SOURCE: shared `file.open`; disabled without a valid selection. | Full menu-row AUTO and current native pass pending. |
| File → New folder | Collision-safe create inside the explicit protected root only. | SOURCE: shared `file.new-folder`; mutation gate and availability reason. AUTO covers protected creation through the permanent shelf/application flow. | Full menu-row and native protected-root pass pending. |
| File → Settings… | Replace the file surface with the owned settings surface. | SOURCE + AUTO: File menu row opens settings and Back restores files. | Native pointer/keyboard pass pending. |
| File → Close File Manager | Request the native window close lifecycle. | SOURCE + AUTO: real menu row invokes the bound close callback exactly once; FM-R010 owns automated/native AppKit close evidence. | Recheck exact current bundle physically. |
| Home → Open | Same authority as File → Open. | SOURCE: shared command. | Menu-row AUTO/native pending. The top-level name `Home` is retained as authored prototype vocabulary, not a navigation alias; do not reinterpret it as Places/Recent. |
| Home → Move / copy → Copy, Move, Paste | Stage or publish one protected-root transfer with identity/collision checks. | SOURCE: three shared mutation commands with live state. | Submenu-row AUTO/native pending. |
| Home → Delete | Recoverable quarantine inside protected scope. | SOURCE: shared destructive command with live state. | Menu-row AUTO/native pending. |
| Home → Properties | Reveal/focus the factual owned inspector. | SOURCE: shared `view.properties`. AUTO covers shelf and overflow projections. | Home-row/native pending. |
| Edit → Select all | Select every visible folder object. | SOURCE + AUTO: enabled only for a nonempty visible ObjectView; disabled semantic/menu/shortcut behavior is proved for empty folders and single-selection correspondence search, then re-enabled on return. | Current native menu/shortcut pass pending. Do not silently add correspondence multiselection. |
| Edit → Rename, Copy, Move, Paste, Delete, Undo | Protected mutation commands only; Undo is one recoverable operation. | SOURCE: shared commands, mutation/transfer/undo state and availability reason. Existing AUTO covers protected rename/collision and shelf mutation gating. | Every Edit menu projection plus native pass pending. |
| View → Small icons / Details | Switch one retained ObjectView between the two admitted presentations. | SOURCE + AUTO: shelf View menu switches to icons; state is radio-checked. | Top-menu Details and current native comparison pending. |
| View → Sort → Name / Kind / Size / Modified | Reorder by factual local fields only. | SOURCE + AUTO: shelf Kind action/state. | Remaining modes, top-menu projection, stable-selection proof, and native pass pending. |
| View → Refresh | Re-read the current admitted local folder without adding history. | SOURCE: shared navigation request. | AUTO for visible retained result and native pass pending. |
| View → Folder tree / Selection and Properties | Collapse/restore each owned split pane; checked state follows the retained split. | SOURCE; splitter and shelf/property AUTO cover parts of the behavior. | Top-menu pointer/keyboard campaign pending. |
| Go → Back / Forward / Up / Home | Navigate bounded admitted roots/history; Home means the actual user Home folder. | SOURCE: live state derives from history/root. Navigation AUTO covers real paths and history. | Every menu row and exact native pass pending. |
| Commands → SHA-256… | Revision-validated digest for one selected regular file, optional expected-digest comparison, and cancel while running. | SOURCE + AUTO + NATIVE: exact PID 17317 accepted 64 hex characters in the live PropertyList, computed a real file digest, reported MISMATCH, then cleared/disabled the editor on folder selection. | Large-file cancel and invalid-expected native cases remain. |
| Commands → Terminal here | Open host Terminal at selected directory or current location. | SOURCE: setting visibility and directory state; prior native dogfood observed. | Menu-row AUTO and current bundle pass pending. |
| Commands → Copy path | Copy the exact selected or current path. | SOURCE: shared command. | Clipboard AUTO/native proof pending. |
| Help → About File Manager | Owned host dialog with exact `0.001-alpha` and authority statement. | SOURCE + AUTO: real menu row publishes exact version/local-authority status in headless mode; version/source audit recorded under FM-R005. | Owned dialog native recheck pending. |
| Permanent shelf → Move / copy, Delete, View, Sort, Properties | Compact Office Pearl projection of the same command objects; responsive collapse must not change authority. | SOURCE + AUTO at ordinary, medium, narrow, and 150×150 sizes; popup contents and several actions proved. | FM-R009 current native visual/pointer/keyboard pass pending. |
| Shelf → More / Commands overflow | Project exactly every shelf member hidden at the current width. | SOURCE + AUTO for medium/narrow membership, action sharing, Escape, and restore. | Current native pass pending. |
| Location → breadcrumb segments / overflow / inline editor | Navigate admitted ancestors, reveal hidden ancestors, or commit an admitted canonical path with completion/cancel behavior. | SOURCE + broad AUTO from FM-R008, including real directories and retained identities. | FM-R008 continuous native construction and current dogfood remain open. |
| Search field / clear / More results | Query only an admitted installed Engine root, clear back to folder, and request the next cursor page only when available. | SOURCE: unavailable roots are disabled and honestly labelled; next-page state follows cursor/loading. | Installed Engine interaction campaign, error/retry, pagination, and native pass pending; FM-R014 still authority-gated for daily roots. |
| View → Criteria / retained instrument rack / More results | Enter an exact catalogue-only virtual folder over the current admitted subtree; edit/toggle/remove Kind, Modified, and Size predicates; page only through a catalogue cursor; ordinary navigation or text search exits. | SOURCE + AUTO + INSTALLED APPLICATION/PROVIDER: retained grammar, local validation, truthful clearing, catalogue-source refusal, generation/cursor projection, real Kind Files→Folders, enable/remove/add, invalid-date recovery, Selection/PropertyList, Engine query, and C++ → Orchestrator → Engine forwarding all pass. | Native M4 pointer/keyboard/visual comparison of the exact current bundle remains. Content/plugin/fuzzy/staged Apply remain absent until admitted. |
| Folder-tree root mode / rows / expansion | Choose exactly one Home-, Volumes-, or explicit-root hierarchy; expand and navigate real retained directory rows. | SOURCE + AUTO for mode membership, depth, expansion, selection, and one-click navigation. | Current native pointer/keyboard pass pending. No Places or Recent row is admitted. |
| Object field / correspondence results | Select, focus, activate, context-click, and inspect real objects; correspondence is a separate single-selection factual search projection. | SOURCE + navigation/ObjectView AUTO; FM-R013 label/focus proof; FM-R022 proves distinct tree/object IDs, canonical activation/symlink refusal, empty-result-space pointer behavior, synchronized selection clearing, and real background menu construction. | Current sparse/dense native pass; remaining object-menu rows require exhaustive AUTO/native proof. |
| Object context menu | Open; protected Copy/Move/Rename/Delete; SHA-256; Terminal; Copy path; Properties. | SOURCE + AUTO: exact admitted membership; protected Copy/Move staging, Rename/cancel, first-stage recoverable Delete with nonmutation; canonical SHA-256 row/source through real worker completion; canonical Copy path row/source with honest unavailable backend state; Properties focus/status; disabled symlink Open semantics/refusal. | Paste/confirmed Delete/committed Rename are covered through other projections; Terminal planner/fallback is proved but its direct row intentionally awaits native launch; real clipboard transport and native pass pending. |
| Background context menu | Protected New folder/Paste; Terminal at current location; current path; Properties pane. | SOURCE + AUTO: every item shares canonical state; real empty search-space right-click clears both presentation selections and constructs the folder menu with New folder/Properties and no Open. | Remaining row actions and native pass pending. |
| Selection and Properties pane | One scroll owner for preview and factual properties; editable Name only for a single protected object. | SOURCE + AUTO for ownership and protected collision-safe property rename. | Remaining property rows, preview kinds/failure states, and current native pass pending. |
| Settings tabs and rows | General, Appearance & Access, Navigation & Views, Search & Indexing, Services, Handlers & Commands, Previews & Extensions, Applications, Privacy & Data, Advanced; show only Orchestrator-schema-backed controls or an honest empty/unavailable state. | SOURCE + AUTO: schema/snapshot driven; unavailable fields read-only; exactly one tab owns retained selected semantics and generated selected visuals; empty Applications remains explicit. Services owns explicit actions. | Build remaining per-row matrix from an installed snapshot; stale revision, failure, destructive confirmation, focus, and current native proof pending. |
| Settings → Apply / Cancel / Reset / Back | Commit pending schema values, discard edits, reset current page to defaults, or restore file surface. | SOURCE + AUTO: Back restores files; synthetic typed transaction proves meaningful Reset, staged defaults, Apply/Cancel recomputation, committed restoration, disabled empty/default pages, and no-op honesty. | Actual Orchestrator commit/conflict and native pass pending. |
| Operation failure / service confirmation / About dialogs | Owned host surfaces that explain failure or request explicit bounded confirmation. | SOURCE. | Dialog-by-dialog semantic/default/cancel/native proof pending. |

- **FM-R022 OBJECT-CONTEXT DIRECT-ACTION GAP / RECORDED BEFORE EDIT:** The
  retained object menu defines exactly Open, Copy, Move, Rename, Delete,
  SHA-256, Terminal, Copy path, and Properties, all backed by shared Commands,
  but the installed Application interaction campaign never opens this menu or
  presses any of its rows. Source binding alone does not prove the visible
  surface works. Add a non-destructive first campaign on a selected regular
  file that opens the real menu, requires exact admitted membership/state, and
  invokes Copy path and Properties through their menu-row semantic actions.
  Then open it for the real symlink fixture and require Open to retain its
  symbolic-link availability reason while exposing no press action. Do not
  launch an external application or execute mutation in this first menu proof;
  those remain separate protected/native campaigns.
- **FM-R022 ABOUT/CLOSE DIRECT MENU ACCEPTANCE / RECORDED BEFORE EDIT:** At the
  end of the complete interaction campaign, press the real Help → About row and
  require exact `File Manager · 0.001-alpha development build` plus its local-
  authority summary even when the headless fixture has no dialog backend. Then
  press File → Close File Manager and require the callback bound by the host to
  run exactly once. Close must be last so it cannot create false downstream
  control evidence. Native About dialog presentation and physical AppKit close
  remain separate Screen Sharing/host evidence.
- **FM-R022 ABOUT/CLOSE DIRECT MENU ROWS / PASSED ON M4:** The complete exact
  Application interaction campaign passed 1/1 in 2.04 seconds. Help → About
  published `File Manager · 0.001-alpha development build` and `local
  filesystem authority · GUI.Forms + Web.Forms` through the real retained row
  without pretending a dialog backend existed. File → Close, intentionally
  invoked last, called the host-bound close request exactly once. Native owned-
  dialog presentation and physical AppKit closure remain current-bundle
  Screen Sharing evidence; run the complete frontend gate.
- **FM-R022 ABOUT/CLOSE-ADJUSTED FULL FRONTEND GATE / PASSED:** All 11
  frontend CTests passed in 1.77 seconds on the M4. The complete Application
  interaction campaign passed in 1.47 seconds and the launcher contract in
  0.26 seconds. This terminal checkpoint includes every direct object-menu row
  that is safe without spawning another application, plus direct Help/About
  and File/Close rows. Product code and Mach-O UUID remain unchanged; the
  existing signed `574d8c1f60a6` candidate is still the current unlaunched
  product image pending Neo unlock and M4 Screen Sharing dogfood.
- **FM-R022 FIRST OBJECT-CONTEXT CAMPAIGN / RUNTIME SEGFAULT:** The updated
  Application interaction target compiled and linked on the M4, but the exact
  registered test crashed with a segmentation fault after 1.05 seconds before
  emitting an assertion message. No object-menu action pass is claimed. Take a
  symbolized M4 backtrace before changing behavior; distinguish a menu snapshot
  lifetime defect, command/popup teardown defect, clipboard/focus defect, or
  test-owned dangling pointer. Preserve the exact admitted-membership,
  Copy-path, Properties, and disabled-symlink acceptance conditions.
- **FM-R022 OBJECT-CONTEXT LLDB ATTEMPT / DEBUG PERMISSION DENIED:** A
  noninteractive M4 LLDB run loaded the exact arm64 interaction executable but
  macOS refused to start it under the debugger with “cannot get permission to
  debug processes.” No backtrace or additional product evidence resulted, and
  no system security setting was changed. Read the matching DiagnosticReports
  crash record or use a separately instrumented build rather than weakening
  host security or guessing at the fault.
- **FM-R022 OBJECT-CONTEXT CRASH REPORT / TEST-OWNED NULL SERVICE:** The M4
  `.ips` report identifies `EXC_BAD_ACCESS` at address `0x8` inside
  `gui_forms::HostServices::read_clipboard_text()`, called directly by the new
  test. This headless Application fixture has no installed HostServices;
  production `copy_current_path()` already guards that condition and reports
  `Clipboard unavailable`, but the test unconditionally dereferenced the null
  service pointer after invoking the row. Replace the invalid clipboard read
  with a subscription proving one canonical `commands.copy-path` invocation
  from the exact context-menu row plus the honest unavailable status. Keep
  real clipboard contents as native/host-service evidence rather than faking a
  backend in this fixture.
- **FM-R022 OBJECT-CONTEXT FIRST DIRECT CAMPAIGN / PASSED ON M4:** After
  replacing only the invalid headless clipboard dereference, the exact
  `file_manager_application_interaction_tests` target passed 1/1 in 1.45
  seconds on the M4. It opens the object menu through the selected file's
  public semantic route; requires exactly all twelve admitted command/separator
  rows and no extras; invokes Copy path as `commands.copy-path` from
  `fm.context.object.popup.row.copy-path` and observes the truthful headless
  `Clipboard unavailable` result; invokes Properties and proves PropertyList
  focus plus factual status; then opens the same surface for a real symlink and
  proves Open is disabled, exposes the symbolic-link reason, publishes no press
  action, and refuses invocation. Run the full frontend gate. Protected
  mutation/SHA/Terminal rows and native clipboard/pointer behavior remain open.
- **FM-R022 OBJECT-CONTEXT PROTECTED ROW ACCEPTANCE / RECORDED BEFORE EDIT:**
  Extend the disposable protected-root fixture through the real menu rows for
  Copy, Move, Rename, and Delete without publishing a filesystem mutation.
  Copy and Move must each stage the selected exact identity and report their
  captured state; Rename must reveal/focus the existing bounded editor and
  cancel through Escape; Delete must enter only its first recoverable armed
  state. Invoke Copy once more after arming Delete because transfer capture
  deliberately clears that arm, ensuring the later shortcut campaign cannot
  become an unintended second-delete/quarantine action. Do not press Paste,
  confirm Delete, or commit Rename in this slice.
- **FM-R022 OBJECT-CONTEXT PROTECTED ROWS / PASSED ON M4:** The expanded exact
  Application interaction test passed 1/1 in 2.42 seconds. Through real object-
  menu semantic rows, Copy staged `root.txt`, Move restaged the same exact
  object, Rename exposed/focused the bounded editor and cancelled through
  Escape, and Delete entered only `Delete armed · press Delete again for
  root.txt`. A final Copy capture cleared that arm, and the fixture asserted
  `root.txt` still exists before continuing. No Paste, Rename commit,
  quarantine, external launch, or other filesystem mutation occurred in this
  direct-row slice. Run the complete frontend gate.
- **FM-R022 OBJECT-CONTEXT SHA-256 ROW ACCEPTANCE / RECORDED BEFORE EDIT:**
  Invoke the selected regular file's real `sha256` context row, require one
  canonical `commands.sha256` invocation carrying that row's stable source ID,
  and drain the existing worker/UI route until it reports completion. The
  fixture already supplies a 64-character nonmatching expected digest, so the
  completion detail must say the expected digest did not match. This is a
  bounded read-only operation. Do not invoke Terminal in headless automation;
  that row deliberately launches another host application and remains a native
  Screen Sharing campaign.
- **FM-R022 OBJECT-CONTEXT SHA-256 ROW / PASSED ON M4:** The expanded
  Application interaction test passed 1/1 in 1.78 seconds. Pressing the real
  `fm.context.object.popup.row.sha256` row emitted one canonical
  `commands.sha256` invocation with that exact row as source, then completed
  through the application's worker/UI queues against the selected stable file.
  The status reached `SHA-256 complete` and its detail truthfully reported the
  fixture's expected digest did not match. This was read-only. Terminal remains
  intentionally unpressed outside the Aqua dogfood session because it launches
  another application. Run the complete frontend gate.
- **FM-R022 OBJECT-CONTEXT SHA-ADJUSTED FULL FRONTEND GATE / PASSED:** All 11
  frontend CTests passed in 1.81 seconds on the M4 after adding the direct
  SHA-256 context row. The complete interaction campaign passed in 1.57 seconds
  and the launcher contract in 0.20 seconds. The object-menu automated slice is
  now complete except for deliberate host-launch/real-clipboard native actions;
  no product binary change or new staging occurred.
- **FM-R022 OBJECT-CONTEXT PROTECTED-ROW FULL FRONTEND GATE / PASSED:** All 11
  frontend CTests passed in 1.89 seconds on the M4 after the direct Copy/Move/
  Rename/Delete expansion. The complete Application interaction campaign
  passed in 1.64 seconds and the launcher contract in 0.21 seconds. This remains
  test/ledger-only evidence over the same product Mach-O UUID; no new staging
  or GUI launch is required. Direct SHA-256 and Terminal menu-row proof plus
  native clipboard/pointer evidence remain.
- **FM-R022 OBJECT-CONTEXT COMPLETE FRONTEND GATE / PASSED:** The complete
  frontend build then passed all 11 CTests in 1.72 seconds on the M4; the
  expanded Application interaction test passed in 1.48 seconds and the launcher
  contract in 0.20 seconds. This slice changes only interaction-test evidence
  and this ledger, not product source or GUI.Forms, so the already verified
  `574d8c1f60a6` product candidate remains current only if a direct build-versus-
  candidate executable hash comparison matches.
- **FM-R022 BUILD/CANDIDATE RAW-HASH COMPARISON / INVALID ORACLE:** The rebuilt
  source bundle executable hashed `a04e7f6c…`, while the staged candidate
  remains `574d8c1f…`. This does not prove different product code: staging
  recursively ad-hoc signs the copy, and the Mach-O signature blob changes the
  executable byte hash. Do not restage merely to make those values equal.
  Compare the Mach-O build UUIDs (or another signature-independent image
  identity) to determine whether the linked product image is unchanged.
- **FM-R022 SIGNATURE-INDEPENDENT PRODUCT IDENTITY / MATCHED:** The rebuilt
  source executable and staged `574d8c1f60a6` candidate both report Mach-O UUID
  `1163691A-7BD7-307A-83FA-7EF02F8BF67C` for arm64. The linked product image is
  therefore unchanged; only the candidate's ad-hoc signature bytes account for
  the raw-hash difference. No restaging is required for this test-only object-
  context expansion, and the existing signed/verified candidate remains the
  current unlaunched product bundle.

- **FM-R022 FIRST REPAIR ACCEPTANCE / RECORDED BEFORE EDIT:** `Select all`
  must be enabled only while the ordinary ObjectView is the active surface and
  it owns at least one visible object. During correspondence search it must be
  disabled with the reason that search results support one factual selection at
  a time; in an empty ordinary folder it must be disabled with an empty-folder
  reason. A disabled menu row and `Cmd+A`/semantic invocation must not mutate
  selection or status as if work occurred. Returning to a nonempty folder must
  re-enable the same retained command. The repair must reuse current
  correspondence single-selection authority and must not add a second selection
  model merely to make the label true.
- **FM-R022 FIRST REPAIR STARTED / NO SOURCE EDIT YET.**
- **FM-R022 ADVERTISED-SHORTCUT AUDIT / CONFIRMED BEFORE EDIT:** Command state
  prints `Enter`, `Cmd+V`, `Delete`, `F2`, `Cmd+A`, and the three `Alt+Arrow`
  navigation gestures in menu rows, but File Manager registers no Window
  accelerators. Focused ObjectView/CorrespondenceView happen to implement
  Enter, and ObjectView happens to implement primary-modifier+A, but those
  routes bypass the shared command identity; Paste, Rename, Delete, and
  navigation have no application keyboard route at all. Use GUI.Forms' existing
  tokenized accelerator API, host-conventional primary modifier text, and the
  same `Command::execute` gate used by menus. Text editors retain first refusal
  for editing chords; app-navigation gestures may preempt collection arrow
  movement but must not steal word navigation from a focused TextBox. Exact
  disabled command state must reject the matching shortcut. Do not add a second
  ad-hoc key switch to the frontend.
- **FM-R022 SURFACE-CONTEXT AUDIT / CONFIRMED BEFORE EDIT:** The persistent
  menu remains visible while Settings replaces the files workspace. Its file,
  selection, View, Go, and Commands rows retain their ordinary enabled state,
  so they can operate on hidden state. `Settings…` itself toggles back to Files
  while still claiming it opens Settings. In correspondence search, Small
  icons/Details and every sort stay enabled even though they mutate only the
  hidden ObjectView; the permanent View and Sort buttons therefore open menus
  that cannot affect the visible result surface. `Refresh` exits search and
  clears the query despite claiming to refresh the current local content.
  Disable file-workspace commands while Settings is active, retitle the one
  retained escape command to `Back to files`, disable folder-presentation
  commands and their permanent buttons during correspondence search, and make
  Refresh repeat the current Engine query rather than silently leave it.
  Properties/tree pane controls may remain available in search because they
  still change the visible owned surface. Search mutations remain governed by
  exact protected identity and are not widened by this repair.
- **FM-R022 DISABLED-MENU AUDIT / CONFIRMED BEFORE EDIT:** A disabled
  ContextMenu row is nonfocusable and refuses activation, but still installs a
  hand cursor and publishes `focus`/`press` semantic actions. That advertises
  interactivity which the row correctly refuses. Public GUI.Forms menu rows
  must expose no action and no hand cursor while their command is disabled,
  while retaining the availability reason in their semantic description.
- **FM-R022 FIRST REPAIR EXPANDED ACCEPTANCE / RECORDED BEFORE EDIT:** Focused
  tests must prove: disabled menu rows have no enabled state, focus/press action,
  or hand cursor and still explain why; empty-folder and correspondence-search
  Select All rows refuse semantic activation; returning to a nonempty folder
  reenables one shared menu/primary-modifier+A command; every advertised
  application shortcut has a real command-backed route with text-field
  precedence and disabled-state refusal; search disables folder-only View/Sort
  commands and re-runs rather than clears its query on Refresh; Settings
  disables hidden-workspace commands and exposes an accurately named Back to
  files row. M4 focused tests, installed-consumer tests, full suites, and current
  native pointer/keyboard proof remain separately required.
- **FM-R022 MENU-TRUTH FIRST SOURCE PASS / IMPLEMENTED, UNVERIFIED:** A public
  GUI.Forms popup row now projects disabled command state into the retained
  control itself, removes its hand cursor, makes it nonfocusable, and publishes
  no semantic focus/press action while preserving description and availability
  reason. The existing menu test now asserts all of those properties and exact
  refusal. The Windows host adapter also fills its previously missing portable
  Delete/F1/F2/F4 USB-HID mappings so command accelerators can remain portable.
  No compile or test claim has been made yet.
- **FM-R022 FRONTEND COMMAND-TRUTH FIRST SOURCE PASS / IMPLEMENTED,
  UNVERIFIED:** File Manager now registers the already advertised Open, Paste,
  Delete, Rename, Select All, Back, Forward, and Up gestures through Window's
  tokenized accelerator path and executes the same retained Command objects as
  menus. The displayed primary-modifier name is selected per host. TextBox
  retains normal editing precedence; collection navigation gets only the
  bounded preemptive routes needed to avoid ObjectView consuming Alt+Arrow or
  primary-modifier+A first. Command state now distinguishes the visible Files,
  correspondence-search, and Settings surfaces: Select All and folder-only
  View/Sort are unavailable where their label cannot be fulfilled, Settings
  retitles its toggle to `Back to files`, hidden-workspace actions disable,
  and Refresh routes to the visible Engine query while search is showing. The
  shelf View/Sort actuators mirror the same surface availability. This pass has
  not compiled or run; the exact focused campaign still needs to be added.
- **FM-R022 FIRST FOCUSED CAMPAIGN / ADDED, UNVERIFIED:** The File Manager
  application interaction target now drives shared command behavior through
  real menu rows and Window key dispatch. It covers primary-modifier+A,
  TextBox first refusal, F2 Rename/cancel, Delete arming, nested Home Copy,
  Enter Open, primary-modifier+V Paste, Alt+Left/Right/Up, empty-folder Select
  All semantics/refusal, synthetic exact-identity correspondence search,
  disabled shelf View/Sort, search Select All refusal, Refresh preserving and
  repeating the query, Settings hidden-workspace state and `Back to files`,
  and re-enabling Select All on return to a nonempty folder. Synthetic search
  enters through the same private apply seam under a narrow friend probe; it
  does not fake filesystem identity or add a production test switch. No compile
  or pass claim has been made.
- **FM-R022 FIRST M4 COMPILE / GUI.FORMS TEST FAILED:** The repaired
  `context_menu.cpp` compiled, but `gui_forms_menu_controls_tests` stopped at
  compile time because the new assertion called a `has_action` helper that this
  test translation unit did not yet define. Add the same small action-membership
  oracle used by semantic tests and rerun the unchanged source behavior. This
  is not a product pass or runtime failure.
- **FM-R022 INSTALLED-CONSUMER FOCUSED / PASSED:** GUI.Forms was rebuilt and
  installed to the persistent `gui_forms/.build/fm0-install` package, File
  Manager reconfigured against that exact absolute package path, and the
  expanded `file_manager_application_interaction_tests` passed 1/1 in 2.79
  seconds on the M4. The run performed real protected staged Copy/Paste and
  real navigation alongside the shortcut, disabled-state, search, Settings,
  and semantic campaigns named above. This is focused evidence only; a
  post-pass audit, complete suites, and native current-bundle dogfood remain.
- **FM-R022 POST-PASS FOCUS AUDIT / CONFIRMED AND CORRECTED, UNVERIFIED:** The
  green focused campaign did not assert focus after returning from Settings.
  `hide_settings`, `cancel_rename`, and the asynchronous rename handoff always
  requested focus for ObjectView even if correspondence search was the visible
  object surface. A shared helper now focuses the effectively visible
  ObjectView or CorrespondenceView. Extend the campaign to prove Settings
  return in search focuses correspondence and ordinary Files return still
  focuses ObjectView. `apply_directory` now also recognizes replacement of a
  visible correspondence surface and transfers focus after ObjectView becomes
  visible, so Escape/activation cannot strand focus in the cleared search box
  or detached result projection. Rerun before accepting the prior count as
  final.
- **FM-R022 POST-PASS FOCUS RERUN / FAILED WITH REUSABLE CAUSE:** The expanded
  File Manager target rebuilt, then failed 0/1 in 1.74 seconds at the new
  Settings-return focus assertion. The prior name/state/return conditions had
  already passed in the previous campaign; the new focus condition exposed
  ContextMenu ordering: it executed application code while its containing
  focus scope was active, then popup teardown restored the menu invoker over
  any destination the command tried to focus. GUI.Forms now ends only the menu
  focus scope before executing the still-snapshotted shared Command, emits the
  invocation while MenuStrip identity remains available, and then tears down
  the popup. Add an exact framework test whose command focuses another control;
  do not weaken the File Manager assertion. This correction is unverified.
- **FM-R022 MENU FOCUS-DESTINATION CORRECTION / FOCUSED PASSED:** The updated
  GUI.Forms menu target passed 1/1 in 1.09 seconds on the M4. Its new command
  explicitly focuses a different retained control and proves popup teardown no
  longer overwrites that destination; the earlier disabled-row campaign remains
  in the same target. Reinstall the corrected package and rerun File Manager's
  exact focus assertion before claiming consumer success.
- **FM-R022 MENU FOCUS CORRECTED INSTALLED CONSUMER / PASSED:** After
  reinstalling the corrected GUI.Forms application library, the expanded File
  Manager interaction target passed 1/1 in 2.55 seconds on the M4. The current
  campaign now proves returning from Settings during correspondence search
  keeps focus on CorrespondenceView and clearing search transfers it to the
  newly visible ObjectView. This supersedes the earlier 2.79-second focused
  count but still precedes the remaining post-pass command audit and full
  suites.
- **FM-R022 SHARED-COMMAND PATH AUDIT / CONFIRMED BEFORE EDIT:** ObjectView and
  CorrespondenceView activation currently call `activate` directly rather than
  executing `file.open`. The shared command correctly disables Open for a
  symbolic link, but direct activation falls through the directory/file split
  and can call the host launcher anyway. Visible Back/Forward/Up, Delete, and
  Properties buttons also call implementation methods directly instead of the
  canonical Command object, duplicating action authority despite mirroring
  enabled state. Route visible object activation and these permanent buttons
  through `Command::execute` with their stable source identities, and retain a
  defensive symlink refusal at the central activation seam. Prove a selected
  symlink cannot launch through semantic item activation and that button/menu/
  shortcut paths emit the canonical command identity.
- **FM-R022 SHARED-COMMAND PATH SOURCE PASS / IMPLEMENTED, UNVERIFIED:** Object
  and correspondence activation now select the exact identity then execute
  `file.open`; Back/Forward/Up, shelf Delete/Properties, and inspector Open,
  SHA-256, Terminal, and Copy Path buttons execute their retained canonical
  commands with stable source IDs. The central `activate` seam also refuses a
  symlink defensively. The focused fixture now creates a real symlink, proves
  canonical Open and Back invocation/source records for Enter and Alt+Left,
  and proves semantic symlink press cannot emit or bypass disabled Open. No
  compile or runtime result has been claimed for this pass.
- **FM-R022 SHARED-COMMAND FOCUSED RERUN / FAILED AT SOURCE ORACLE:** The
  target rebuilt, then failed 0/1 in 1.08 seconds after Enter successfully
  navigated into Documents but before accepting the recorded command source as
  `fm.accelerator.file.open`. ObjectView already owns Enter and now forwards
  its activation through the canonical Open command, so the valid source may
  be `fm.objects.activation`; do not guess or weaken canonical identity. Print
  the exact invocation trace and then either remove a redundant accelerator or
  correct the source oracle while continuing to require one `file.open`
  invocation.
- **FM-R022 OPEN TRACE DIAGNOSTIC / GLOBAL SEMANTIC-ID COLLISION:** The
  diagnostic rerun again failed 0/1 in 1.14 seconds and reported `count=0`.
  Navigation had occurred before the assertion because directory rows in the
  visible TreeView and ObjectView reused the same filesystem stable ID.
  Semantic selection could therefore resolve the tree row, whose one-click
  selection navigates immediately; the following Enter was not the successful
  operation the fixture appeared to prove. Namespace TreeView presentation IDs
  while retaining their map to exact filesystem paths, assert tree/object IDs
  differ, explicitly focus the ObjectView before Enter, and keep the one
  canonical `file.open` invocation requirement. This is both a test-oracle
  correction and a real accessibility/automation identity repair.
- **FM-R022 IDENTITY RERUN / TEST COMPILE FAILED:** Product source compiled,
  but the application test stopped because the new `folder_tree` lookup was
  inserted in the earlier navigation fixture rather than the command-truth
  fixture that used it. Move only that lookup and rerun unchanged product
  behavior. No runtime or product pass follows from this attempt.
- **FM-R022 SHARED-COMMAND AND GLOBAL-ID FOCUSED / PASSED:** After moving the
  test lookup to its intended fixture, the application interaction target
  passed 1/1 in 1.69 seconds on the M4. The campaign now proves distinct
  TreeView/ObjectView semantic IDs for the same directory, one canonical Open
  invocation/source for Enter, one canonical Back invocation/source for
  Alt+Left, and exact refusal of semantic activation on a real symlink while
  `file.open` is disabled. This focused result supersedes the earlier 2.55
  second count. Complete suites and current native dogfood remain.
- **FM-R022 GUI.FORMS MENU-TRUTH FOCUSED / PASSED:** After adding only the
  missing test helper, `gui_forms_menu_controls_tests` rebuilt and passed 1/1
  in 1.02 seconds on the M4. This proves the disabled row's retained enabled,
  focus, cursor, availability-description, semantic-action, and invocation
  behavior. It does not yet prove the installed File Manager consumer or full
  GUI.Forms regressions.
- **FM-R022 FULL-SUITE RESULT LOST AT COMPACTION / RERUN REQUIRED:** A complete
  M4 campaign (File Manager build and suite followed by the GUI.Forms suite)
  was still attached to execution cell `1348` when conversation compaction
  occurred. The post-compaction retrieval attempt reported that the cell no
  longer existed, so no pass or failure can honestly be inferred from that
  run. Rerun the identical complete campaign and record its terminal result;
  the missing tool cell is an evidence-loss event, not a product-test result.
- **FM-R022 COMPLETE M4 REGRESSION RERUN / PASSED:** The replacement campaign
  rebuilt File Manager, then passed all File Manager tests 10/10 in 0.63
  seconds and all GUI.Forms tests 62/62 in 3.02 seconds on the arm64 M4 mirror.
  This validates the current command-state, accelerator, shared-command,
  semantic-identity, disabled-menu, and popup-focus repairs against the full
  automated surface. It is automated evidence only: native visual/control
  dogfood of a freshly staged bundle remains required, and FM-R022 stays open
  while the remaining visible-control matrix is audited.
- **FM-R022 SEARCH BACKGROUND CONTEXT / CONFIRMED BEFORE EDIT:** A secondary
  click on empty CorrespondenceView space is marked handled on pointer-down but
  discarded on pointer-up: unlike ObjectView, the control neither clears its
  selected row nor emits an empty-target `context_requested` event. File
  Manager's nominal correspondence-background handler is therefore unreachable
  through a real pointer; even if called, it clears only the hidden ObjectView
  selection and leaves the visible correspondence highlight intact. Repair the
  GUI.Forms contract so empty primary space clears selection and empty secondary
  space clears selection plus emits exactly one empty-target context request.
  File Manager must explicitly synchronize both presentations and open its
  folder/background menu for the current directory. Prove the generic pointer
  behavior in GUI.Forms and the real menu/selection behavior in the application
  campaign before accepting the repair.
- **FM-R022 SEARCH BACKGROUND CONTEXT / IMPLEMENTED, UNVERIFIED:**
  CorrespondenceView now clears selection when a complete primary or secondary
  click lands on empty space, and the secondary path emits one empty-target
  context request at the released pointer position. File Manager's handler now
  clears correspondence and ObjectView explicitly before showing the retained
  background menu. A GUI.Forms test exercises both pointer paths; the
  application campaign starts with synchronized selection, right-clicks real
  empty correspondence geometry, requires both selection models empty, and
  requires New folder/Properties present with Open absent. Compile and focused
  runtime results are not yet claimed.
- **FM-R022 SEARCH BACKGROUND FOCUSED CONSUMER / FAILED ON STALE INSTALLED
  LIBRARY:** The generic GUI.Forms collection target rebuilt and passed 1/1 in
  0.57 seconds on the M4, proving the new pointer contract in its statically
  linked test. The same invocation then installed without first rebuilding the
  GUI.Forms application dylib; the install log explicitly reported
  `libgui_forms_application.dylib` up-to-date. File Manager consequently ran
  against the pre-repair consumer library and failed 0/1 in 0.91 seconds at the
  real empty-search-background assertion. Rebuild the application library,
  reinstall it, and rerun the unchanged consumer test. This is a packaging
  verification failure, not evidence that the new pointer code failed its
  focused framework test.
- **FM-R022 SEARCH BACKGROUND DYLIB REBUILD RERUN / STILL FAILED:** Rebuilding
  the real `gui_forms_application` dylib, reinstalling it, and relinking File
  Manager did not clear the consumer failure: the exact application assertion
  still failed 0/1 in 2.07 seconds. The stale-install diagnosis therefore did
  not explain the whole result. Preserve the acceptance condition and report
  correspondence/control geometry, both selected identities, and actual popup
  row presence on the next run to distinguish hit routing, item geometry, and
  application menu-state causes.
- **FM-R022 SEARCH BACKGROUND DIAGNOSTIC / TEST POINT OUTSIDE WINDOW:** The
  diagnostic rerun failed 0/1 in 1.30 seconds with both pointer dispatches
  false, unchanged selections, and no popup. CorrespondenceView reported
  absolute geometry `(245,219,784,780)`, the single row local geometry
  `(2,2,780,103)`, and the test point `(637,991)`. The layout extends below the
  850-high headless window, so choosing `control.bottom - 8` put the oracle
  outside host hit-test bounds. Place the pointer eight pixels below the real
  row instead; that point is empty correspondence content and remains within
  the actual window. Do not change product code for this false test coordinate.
- **FM-R022 SEARCH BACKGROUND CONTEXT / FOCUSED PASSED:** With the application
  test point derived from the realized row rather than the off-window arranged
  bottom, the installed-consumer target passed 1/1 in 1.54 seconds on the M4.
  Together with the GUI.Forms collection target's 1/1 pass in 0.57 seconds,
  this proves empty primary/secondary correspondence pointer behavior and the
  File Manager result that both selection models clear while the retained
  current-folder menu exposes New folder and Properties but not Open. Complete
  suites must be rerun after the remaining FM-R022 audit changes.
- **FM-R022 ENABLED-STATE DESCRIPTION AND EMPTY DROPDOWN / CONFIRMED BEFORE
  EDIT:** `update_command_state` assigns the correspondence-only availability
  reason to every View/Sort command even when ordinary folder presentation has
  enabled it; enabled SHA-256 similarly retains `Select one regular file`.
  Those stale descriptions contradict actionable semantic rows. Separately,
  the permanent Move/Copy dropdown is enabled when `paste_ready` is true even
  with no selected object, but its admitted popup deliberately contains only
  Copy and Move (Paste lives in Edit, shortcut, and background context). The
  result is an enabled visible dropdown with two disabled children. Clear
  availability reasons whenever these commands are enabled, and enable the
  Move/Copy actuator only when one selection makes at least one contained
  command actionable. Prove ordinary View/Sort/SHA command descriptions and
  the staged-transfer/no-selection shelf state without adding Paste to the
  permanent popup.
- **FM-R022 EMPTY SUBMENU CONTRACT / CONFIRMED BEFORE EDIT:** GUI.Forms treats
  every nonempty submenu as enabled regardless of child command state. At
  narrow File Manager widths this leaves the projected Move/Copy submenu
  focusable, expandable, and hand-cursor active while both contained commands
  are disabled. Derive submenu enabled state recursively from at least one
  actionable descendant, expose a generic no-commands-available description,
  and refuse pointer/keyboard/semantic expansion when none exists. Keep the
  disabled children visible so their exact command-specific reasons remain
  inspectable when a sibling eventually makes the submenu actionable. Add a
  GUI.Forms contract test and an application narrow-overflow assertion.
- **FM-R022 ENABLED DESCRIPTION / EMPTY DROPDOWN AND SUBMENU / IMPLEMENTED,
  UNVERIFIED:** Enabled folder View/Sort and enabled SHA-256 commands now clear
  availability reasons. The wide Move/Copy dropdown now requires one selected
  object rather than unrelated staged Paste readiness. GUI.Forms recursively
  derives submenu enabled state from actionable descendants, paints and exposes
  an all-disabled submenu as disabled with no actions/cursor and a generic
  explanation, and refuses its expansion. Focused framework and File Manager
  assertions cover these states; no compile or runtime result is yet claimed.
- **FM-R022 ENABLED DESCRIPTION / EMPTY DROPDOWN AND SUBMENU / FOCUSED
  PASSED:** After rebuilding the GUI.Forms application library and installed
  consumer, the GUI.Forms menu target passed 1/1 in 1.09 seconds and the File
  Manager application interaction target passed 1/1 in 1.71 seconds on the M4.
  The proof covers recursive all-disabled submenu semantics/refusal, cleared
  enabled-command reasons, a disabled wide Move/Copy actuator with staged Paste
  but no selection, and the same honest disabled state in the narrow Commands
  projection. Complete suites and current native dogfood remain.
- **FM-R022 SETTINGS TAB/RESET TRUTH / CONFIRMED BEFORE EDIT:** Settings tabs
  are ordinary Buttons, and the authored General button alone permanently owns
  the `.selected` Web.Forms recipe. `select_settings_tab` changes title/body but
  never changes visual or semantic selection, so General remains painted
  selected after another tab opens. Separately, Reset page is enabled for every
  loaded non-Service page, including pages with no available field and pages
  whose effective values already equal every declared default; its handler then
  announces `Page defaults staged` even when nothing changed. Add a retained
  Button selected property that participates in theme resolution and semantics,
  move the generated selected/ordinary recipes with the active settings tab,
  and enable Reset only when one available field on the active page has an
  effective value different from its default. A defensive no-op reset must say
  values already match defaults. Prove General, a nondefault Appearance field,
  Reset staging, Apply/Cancel state, and an empty Applications page from a
  synthetic typed schema/snapshot without requiring a live Orchestrator.
- **FM-R022 SETTINGS TAB/RESET TRUTH / IMPLEMENTED, UNVERIFIED:** GUI.Forms
  Button now retains a selected property, resolves native theme/image state with
  it, publishes semantic selected state, and includes it in visual outsets.
  File Manager captures the generated selected and ordinary settings-tab
  recipes, moves them with exactly one active category, and publishes current/
  open descriptions. Reset now scans available fields on the active page and
  compares the effective pending-or-committed value with the declared default;
  it disables with exact no-fields/already-default/not-ready descriptions and
  refuses to claim staged work on a defensive no-op. A standalone synthetic
  typed Settings campaign covers tab state/recipes, nondefault Appearance,
  Reset staging, Apply/Cancel recomputation, committed-value restoration, and
  empty Applications. Compile and runtime results are not yet claimed.
- **FM-R022 SETTINGS TAB/RESET TRUTH / FOCUSED PASSED:** After rebuilding and
  reinstalling the GUI.Forms application library, the basic-control target
  passed 1/1 in 0.80 seconds and the File Manager interaction target passed 1/1
  in 1.71 seconds on the M4. The framework proof covers retained Button selected
  semantics; the consumer proof covers moving generated selected/ordinary
  recipes, exactly one selected Settings category, a nondefault Appearance
  value, meaningful Reset staging, Apply/Cancel action recomputation, committed
  value restoration, empty Applications, and defensive no-op status. Complete
  suites and current native dogfood remain.
- **FM-R022 INSPECTOR DIRECT-CONTROL AUDIT / CLEARED FALSE ALARM:** The first
  mechanical grep suggested inspector Open/SHA-256 remained at their initial
  disabled state and Terminal/Copy Path at generated defaults because
  `update_command_state` does not write those Buttons. Full call-chain
  inspection showed `update_selection` always enters
  `update_mutation_controls`, which computes all four inspector enabled states
  (including checksum in-flight and command visibility) and immediately calls
  `update_command_state`; checksum transitions also re-enter that seam. No
  product edit is warranted from the grep alone. Keep native and interaction
  coverage open, but remove this suspected defect from the repair queue.
- **FM-R022 COMPLETE POST-REPAIR M4 REGRESSION / PASSED:** The application and
  all dependent File Manager targets rebuilt on the arm64 M4, then the complete
  File Manager suite passed 10/10 in 1.08 seconds. The complete GUI.Forms suite
  passed 62/62 in 3.06 seconds. This campaign includes search-background,
  recursive submenu, enabled-description, Button selected-state, and Settings
  transaction repairs. It is automated evidence only; a freshly isolated and
  signed current bundle still requires native Screen Sharing dogfood.
- **FM-R022 CURRENT DOGFOOD STAGING / PASSED:** The exact post-regression source
  bundle was copied to a new non-overwriting M4 target at
  `$HOME/Developer/CodexRuns/fmrepair-controls-truth-final-20260813.app`, then
  ad-hoc signed recursively. `codesign --verify --deep --strict` reports valid
  on disk and satisfies its designated requirement. The executable is a
  1,986,320-byte arm64 Mach-O. It has not been launched through SSH; staging
  proves package identity only. Inspect and operate it through Screen Sharing's
  Aqua session.
- **FM-R022 CURRENT SCREEN SHARING PRELAUNCH / NEO GATE DENIED:** The mandatory
  Computer Use `sky.get_app_state({app:"Screen Sharing", disableDiff:true})`
  call was rejected before returning framebuffer or accessibility state with
  `The Mac is locked and automatic unlock could not unlock it`. Prior direct
  evidence and user correction identify this as the Neo-side Screen Sharing
  access gate, not proof that the M4 console is locked. No launch, key, click,
  or coordinate action followed. The signed current bundle remains unlaunched;
  native control/visual dogfood is still open.
- **FM-R022 CORRESPONDENCE BACKGROUND FOCUS / CONFIRMED BEFORE EDIT:**
  ObjectView requests focus on every primary/secondary background press, but
  CorrespondenceView requests focus only when `index_at` finds a row. Its newly
  repaired empty primary click therefore clears selection yet leaves keyboard
  focus on whichever control previously owned it; the empty secondary context
  path can likewise restore the wrong focus owner after Escape. Make every
  admitted pointer-down inside CorrespondenceView focus the collection before
  row/background branching. Prove primary background focus generically and
  require File Manager's search background menu to restore correspondence focus
  after Escape.
- **FM-R022 CORRESPONDENCE BACKGROUND FOCUS / FOCUSED PASSED:** With pointer
  focus established before correspondence row/background branching, the
  GUI.Forms collection target passed 1/1 in 0.94 seconds and the installed File
  Manager interaction target passed 1/1 in 2.00 seconds on the M4. The generic
  proof covers primary background focus/clear; the consumer proof covers
  secondary background menu Escape restoring focus to the still-visible
  CorrespondenceView. Complete suites must be rerun, and the previously staged
  controls-truth bundle is now stale relative to this source correction.
- **FM-R022 FINAL FOCUS-ADJUSTED COMPLETE M4 REGRESSION / PASSED:** The final
  application source rebuilt and the complete File Manager suite passed 10/10
  in 1.25 seconds; the complete GUI.Forms suite passed 62/62 in 3.03 seconds on
  the arm64 M4. This supersedes the prior complete counts for the current source
  slice and includes background correspondence focus. Restage to a new bundle;
  do not overwrite or present the earlier controls-truth bundle as current.
- **FM-R022 FINAL FOCUS-ADJUSTED DOGFOOD STAGING / PASSED:** A new
  non-overwriting bundle now exists at
  `$HOME/Developer/CodexRuns/fmrepair-controls-focus-final-20260813.app` on the
  M4. It was copied from the final green build, recursively ad-hoc signed, and
  passes deep strict verification; its executable is a 1,986,320-byte arm64
  Mach-O. The earlier `fmrepair-controls-truth-final-20260813.app` remains
  untouched but stale. The current bundle is unlaunched because the Neo-side
  Screen Sharing access gate still blocks GUI inspection.
- **FM-R022 GLOBAL VIRTUAL-SEMANTIC IDENTITY / PROOF HARDENING BEFORE EDIT:**
  Window already rejects duplicate retained Control IDs, but virtual tree,
  object, correspondence, PropertyList, and popup rows are resolved through a
  global semantic action walk. The earlier tree/object collision proved that
  local model uniqueness is insufficient. Add a recursive full-snapshot oracle
  that rejects any duplicate nonempty stable ID, and run it in ordinary Files,
  with a popup open, during correspondence search, and in Settings. This is
  regression hardening, not a newly claimed product defect.
- **FM-R022 GLOBAL IDENTITY ORACLE / TEST COMPILE FAILED:** The first focused
  build stopped before runtime because the convenience overload accepted
  `const Window&`, while `semantic_snapshot()` deliberately increments snapshot
  generation and is non-const. Change only the helper signature to `Window&`
  and rerun the same ordinary/popup/search/Settings uniqueness assertions. No
  product result follows from this compile failure.
- **FM-R022 GLOBAL IDENTITY ORACLE RERUN / AWDL TRANSPORT RESET:** The corrected
  test did not reach synchronization or compilation: SSH key exchange to the
  M4 relay was reset by peer on local port 2222. This is transport loss, not a
  build or test outcome. Confirm the `m4mini-awdl` endpoint and rerun the same
  target without changing the oracle.
- **FM-R022 AWDL RELAY READ-ONLY DIAGNOSTIC:** The SSH alias resolves to
  `127.0.0.1:2222`, but a direct orientation command is immediately closed.
  Process inspection shows duplicate `awdl-forward --client` processes for both
  ports 2222 and 5901, while the port-2222 listener query returns no socket.
  Determine their parent/start authority before restarting exact relay
  processes; do not edit the M4 mirror or treat this as product failure.
- **FM-R022 RELAY STRUCTURE / NO RESTART WARRANTED:** The apparent duplicate
  forwarders are expected supervisor/child pairs owned by the system launchd
  jobs `com.ultimus.awdl-client-ssh` and `com.ultimus.awdl-client-vnc`. The
  underlying `awdl0` interface reports `status: inactive`; killing or restarting
  the healthy supervisors would not restore the missing radio link, so no
  destructive relay action was taken.
- **FM-R022 WIFI FALLBACK / CURRENTLY UNDISCOVERABLE:** The existing `m4mini`
  alias targets `joshuahs-mac-mini.local`, but a bounded SSH orientation attempt
  failed at name resolution. Thus neither private AWDL nor ordinary Wi-Fi can
  currently reach the M4. The only source change after the last green full
  suites and current signed bundle is test-only global semantic-ID proof
  hardening; no product change is awaiting compilation.
- **CURRENT RECOVERY CHECKPOINT — 2026-08-13:** Product truth remains
  `0.001-alpha`; do not describe it as 1.0. The last complete M4 regressions for
  the current product binary are File Manager 10/10 in 1.25 seconds and
  GUI.Forms 62/62 in 3.03 seconds. The exact current, signed, unlaunched arm64
  dogfood bundle is
  `$HOME/Developer/CodexRuns/fmrepair-controls-focus-final-20260813.app`; the
  earlier controls-truth bundle is stale. The sole later source change is a
  test-only recursive semantic-ID uniqueness oracle in
  `frontend/tests/application_interaction_tests.cpp`. Its initial constness
  compile error was corrected, but the rerun has not synchronized or compiled
  because AWDL is inactive and the Wi-Fi host alias does not currently resolve.
  Screen Sharing remains blocked before framebuffer access by the Neo-side
  automatic-unlock denial; this is not evidence that the M4 is locked. Native
  M4 interaction/dogfood therefore remains open. No file has been staged,
  committed, pushed, or published, and the user's explicit no-Git-write rule
  remains controlling. On transport recovery, first compile and run the focused
  uniqueness oracle; record any duplicate identity exactly before repair. Use
  Computer Use against Screen Sharing for GUI launch and inspection—never SSH
  and never a browser on the Neo.
- **EXTERNAL GIT MUTATION DISCOVERED — 2026-08-13 03:12:56 -0600:** Although
  this task performed no Git write, an external repository automation advanced
  `HEAD` to `2fb25209fca5f858caf1c62ae53bc9752c38ab21`, subject
  `chore(gui_forms): automated four-hour backup`. The commit captured 32
  GUI.Forms files (1,328 insertions, 108 deletions), including this campaign's
  selected Button state, recursive menu enablement, correspondence background
  behavior, and their tests, plus earlier GUI.Forms work. The frontend repair
  files remain modified/untracked in the working tree. Do not claim that the
  user requested this commit; do not stage, commit, push, reset, revert, or
  otherwise rewrite Git state without explicit authority. The existing commit
  is an externally observed fact and is left untouched.
- **FM-R022 BUTTON SELECTED-STATE DOCUMENTATION / CONFIRMED BEFORE EDIT:** The
  external backup captured the new public `Button::selected()` and
  `Button::set_selected(bool)` API and its implementation/tests, but the
  generated GUI.Forms Button atlas page still omits both methods. The atlas is
  declaration-generated from checked-in sources; regenerate it through the
  project tool, inspect the exact delta, and retain only truthful generated
  documentation. This is contract documentation repair, not a binary change.
- **FM-R022 LIBRARY ATLAS REGENERATION / FAILED, NO PASS CLAIM:** The first
  generator invocation yielded no visible result and was initially—but
  incorrectly—treated as zero-delta success. An explicit parser probe confirms
  the current `Button` declaration does expose `selected`, `set_selected`, and
  `semantic_descriptor`; an explicit generator run then exited 1 during final
  validation. The validator reports a broad pre-existing/manual-review backlog
  introduced by current GUI.Forms declarations, including `DropDownButton`,
  `BreadcrumbTrail`, and many callback/helper types. The generator deletes and
  rewrites output directories before that final validation, so inspect and
  contain any unvalidated generated delta. Do not hand-edit the Button page or
  claim documentation passed. This is a documentation-pipeline blocker, not a
  File Manager binary failure.
- **FM-R022 FAILED ATLAS SIDE EFFECT / CONTAINED:** The failed generator had
  rewritten 524 tracked atlas files and created 184 untracked generated pages
  before validation rejected the output. Those tracked changes were reversed
  from their exact generated diff with the system `patch` utility (no Git
  checkout/reset), the 184 enumerated new atlas pages were removed, and 259
  `.orig` backups created by `patch` were removed. A final scoped status shows
  zero changes below `gui_forms/docs/library`; repository-wide `git diff
  --check` passes. Only the intended frontend working tree remains dirty. The
  Button documentation omission stays recorded but deferred behind the broader
  manual-review/generator backlog; do not regenerate until the validator can
  pass atomically.
- **FM-R022 TRANSPORT RETRY / STILL BLOCKED:** AWDL still resets before SSH key
  exchange and `joshuahs-mac-mini.local` still does not resolve. The Neo's ARP
  table exposed six non-router LAN peers; bounded TCP/22 probes found no SSH
  listener on any of them, so none was assumed to be the M4. A fresh mandatory
  Computer Use read of Screen Sharing again failed before framebuffer/AX state
  with the Neo-side automatic-unlock denial. No GUI coordinate, launch, remote
  edit, install, or Git action followed. Continue source/control audit locally;
  retry the focused semantic-ID oracle and Aqua dogfood when either transport
  or Screen Sharing access returns.
- **FM-R022 TERMINAL-HERE SELECTION FALLBACK / CONFIRMED BEFORE EDIT:** The
  admitted command description and `CONTEXTUAL_BUILTIN_COMMANDS.md` define the
  target as the exact selected/current directory. A selected regular file is
  not a directory, so the containing current location remains the honest
  target. Current runtime state instead disables Terminal Here whenever a
  regular file is selected, advertises “Select a folder or clear the
  selection,” and `request_terminal()` silently returns if reached. This makes
  the Commands and object-context projections needlessly inert during ordinary
  file selection. Keep Terminal enabled throughout the files surface; use an
  exact nonsymlink selected directory when present and otherwise synthesize the
  already-observed current location. Prove a regular-file selection launches
  the fixed-argv terminal plan at the current directory, while a selected
  directory still targets itself and no shell/elevation path is introduced.
- **FM-R022 TERMINAL-HERE SELECTION FALLBACK / IMPLEMENTED, UNVERIFIED:**
  Terminal Here is now enabled throughout the visible Files surface. One exact
  selected nonsymlink directory remains the target; a regular file, symlink,
  multiple selection, or no selection falls back to a freshly observed current
  location. The execution path still enters the existing native planner with
  `navigation_root_` as the admission boundary. The interaction test now
  requires a selected regular file to produce the exact `/usr/bin/open -a
  Terminal <current-directory>` argv-only plan and a selected folder to remain
  its own target. It deliberately does not spawn Terminal. Compile and run the
  focused target on the M4 when transport returns; do not claim this repair
  passed before then.
- **FM-R022 TERMINAL-HERE FOCUSED M4 RUN / FAILED:** AWDL recovered and the M4
  rebuilt the application plus interaction target successfully. Runtime then
  failed 0/1 in 0.91 seconds at the new regular-file/current-directory plan
  assertion. The global semantic-ID oracle reached and passed its first
  ordinary-folder checkpoint, but later popup/search/Settings checkpoints did
  not all execute before this failure. Add exact target path, directory flag,
  planner result code, executable, and argv diagnostics; rerun without changing
  product behavior so the mismatch is evidence rather than speculation.
- **FM-R022 TERMINAL-HERE FAILURE DIAGNOSIS / TEST ORACLE TOO STRICT:** The
  diagnostic rerun again failed 0/1 (0.78 seconds), but proved the target equals
  the fixture root, is a directory, planner result is `ok`, executable is
  `/usr/bin/open`, and argv has four entries. Only the final directory spelling
  differed: the planner's already-established canonical directory path retains
  a trailing `/`, while the fixture's `path.string()` does not. This is the same
  directory and not a product behavior regression. Require the final argv entry
  to equal the planner's validated `selected_path.string()` and require that
  selected path to be filesystem-equivalent to the current location; keep the
  fixed `/usr/bin/open`, `-a`, `Terminal` prefix exact.
- **FM-R022 TERMINAL-HERE + GLOBAL IDENTITY FOCUSED M4 / PASSED:** After
  correcting only the path-spelling oracle, the M4 rebuilt the focused target
  and passed 1/1 in 0.99 seconds. This proves Terminal Here remains enabled for
  a selected regular file and resolves to the current directory through the
  exact argv-only planner, while selected directories remain their own target.
  Since the complete interaction test reached its end, the recursive global
  semantic-ID oracle also passed in ordinary Files, with a popup open, during
  correspondence search, and in Settings. Complete File Manager and GUI.Forms
  suites are still required before this source slice replaces the current
  signed dogfood bundle.
- **FM-R022 POST-TERMINAL COMPLETE M4 REGRESSION / PARTIAL RESULT:** The full
  File Manager build and suite passed 10/10 in 0.57 seconds. During the ensuing
  GUI.Forms run, `gui_forms_file_manager_demoboard_tests` failed with
  `keyboard context command did not execute and restore collection focus` at
  1.41 seconds. The remaining output stream was cut off when the SSH transport
  closed, so no final GUI.Forms pass/fail count is claimed. This observed
  demoboard failure is outside the Terminal product path but belongs to the
  current independently committed GUI.Forms slice. Reproduce that exact test
  alone with verbose output before classifying it as deterministic, load/order
  sensitive, or stale expectation; do not stage a new dogfood bundle yet.
- **FM-R022 DEMOBOARD KEYBOARD CONTEXT FAILURE / DIAGNOSTIC BEFORE EDIT:** The
  failing assertion combines End handling, Enter handling, popup removal, focus
  scope closure, object-field focus restoration, and Properties status into one
  Boolean expression. It therefore does not identify the failed contract. Split
  only the test diagnostics so End and Enter failures are named separately and
  any post-command failure reports popup presence, focus-scope depth, focused
  stable ID, and authority text. Do not change ContextMenu, ObjectView, or
  demoboard product behavior before an isolated reproduction identifies the
  predicate.
- **FM-R022 DEMOBOARD CONTEXT FAILURE / STATIC INFERENCE, NOT YET MEASURED:**
  End selects the last object-menu row, `properties`. Its admitted
  `ShowPropertiesHandler` explicitly focuses the retained PropertyList. The
  current ContextMenu activation order deliberately ends the popup focus scope
  before executing the command, so destination focus survives; the old test
  still requires focus to return to ObjectView, which would overwrite the
  Properties command's focus result. This strongly indicates a stale test
  expectation. Do not change it on inference alone: the isolated diagnostic
  run must report `fm.selection.properties` as the actual focus while popup,
  scope, and authority predicates pass.
- **CURRENT RECOVERY CHECKPOINT — POST-TERMINAL REPAIR, 2026-08-13:** Product
  truth remains `0.001-alpha`. The newest product source enables Terminal Here
  for ordinary file selection by falling back to the current directory; its
  focused M4 interaction target passed 1/1 in 0.99 seconds and the complete File
  Manager suite passed 10/10 in 0.57 seconds. The same completed interaction
  run proves recursive global semantic-ID uniqueness in ordinary Files, popup,
  correspondence-search, and Settings states. No bundle containing this newest
  product source has been staged: the signed
  `$HOME/Developer/CodexRuns/fmrepair-controls-focus-final-20260813.app` is now
  stale and must not be presented as current. The subsequent GUI.Forms suite
  observed one demoboard keyboard-context failure before SSH loss; final suite
  count is unknown. A diagnostic-only demoboard test edit awaits isolated M4
  execution. Static evidence predicts the actual post-Properties focus is the
  PropertyList and the old ObjectView-focus assertion is stale, but no repair
  is accepted until runtime confirms it. AWDL is again resetting before key
  exchange; Screen Sharing's most recent read remains denied by the Neo-side
  auto-unlock gate. The atlas regeneration remains deferred after its failed
  output was fully contained. This task has performed no stage, commit, push,
  PR, branch, reset, or revert; external automation commit `2fb2520` remains
  untouched.
- **FM-R021 HISTORICAL 1.0 CLAIM / CORRECTED:** The 2026-08-11 dogfood README
  now begins with an explicit supersession banner, links the owner correction
  and this ledger, calls itself protected-slice prototype evidence, preserves
  the old `1.0.0` bundle/hash as rejected-build observations, and explicitly
  denies 1.0, visual-fidelity, complete-control, or daily-use proof. A targeted
  text audit finds no remaining present-tense “protected profile proves 1.0” or
  “About reports 1.0.0” claim in that record. Active About remains
  `0.001-alpha`; the bundle post-build metadata is `0.0.1`.
- **FM-R021 CMAKE `1.0.0` SUSPECT / CLEARED FALSE ALARM:** `project(... VERSION
  1.0.0)` feeds the independently installed `FileManagerDocumentPicker` package
  and its `SameMajorVersion` compatibility file. ADR-020 records a measured
  out-of-tree consumer of that 1.0 package, and the ledger's FM-R005 decision
  explicitly retains it as an independent package contract. The app target does
  not surface that value: compile-time About is `0.001-alpha` and the bundle
  marketing version is rewritten to `0.0.1`. Do not change the picker package
  major as an app-version cleanup.
- **FM-R022 EXPECTED SHA-256 INPUT / CONFIRMED BEFORE EDIT:** The inspector
  rebuild clears the authored inspector children and installs a PropertyList as
  the only body. The old inspector command stack—including the sole
  `Expected SHA-256 (optional)` host—is detached and hidden, while
  `request_checksum()` still reads its retained but unreachable TextBox. Thus
  the visible SHA-256 command can compute a digest, but the admitted optional
  expected-digest comparison is not actually enterable despite README/ADR
  claims. Do not resurrect the obsolete Open/Terminal/Copy Path button stack.
  Read the contextual checksum surface authority, then put one bounded visible
  expected-digest editor into the factual Selection/Properties surface with
  exact selection/checksum availability and interaction proof.
- **FM-R022 EXPECTED SHA-256 INPUT / IMPLEMENTED, UNVERIFIED:** The live
  Selection PropertyList now owns a `VERIFICATION` group with one editable
  `Expected SHA-256` row. Its TextBox is the exact object read by the hashing
  path, is limited to 64 characters, and is enabled only when the canonical
  checksum context is one selected regular nonsymlink file, visible Files, and
  no hash in flight. Any selection change clears the value before recomputing
  eligibility, preventing comparison carryover between objects. The detached
  legacy inspector button stack remains absent; it was not resurrected. The
  interaction test requires live PropertyList ownership, bounded/enabled file
  state, exact value entry, and clear/disable on folder selection. Compile and
  run on M4 before claiming pass.
- **FM-R022 M4 VERIFICATION RETRY / TRANSPORT BLOCKED, 2026-08-13:** A fresh
  passwordless `m4mini-awdl` probe reset during SSH key exchange
  (`kex_exchange_identification: read: Connection reset by peer`) before any
  remote command or build ran. This is neither product-test evidence nor proof
  that the Mini is offline. Keep the expected-digest repair explicitly
  unverified, do not launch the GUI on the Neo, and continue bounded relay
  recovery checks while using local compilation only for earlier feedback.
- **FM-R022 SCREEN SHARING RETRY / NEO-SIDE GATE STILL CLOSED, 2026-08-13:**
  Computer-control inspection of the existing Screen Sharing app again stopped
  before returning either the remote framebuffer or an accessibility tree:
  `The Mac is locked and automatic unlock could not unlock it.` Per the earlier
  owner correction, this is the Neo/Codex Screen Sharing unlock gate and must
  not be reported as evidence that the M4 itself is locked. No browser or File
  Manager process was opened on the Neo. Native visual dogfood remains pending.
- **FM-R022 LOCAL FOCUSED BUILD FALSE START / ENVIRONMENT, 2026-08-13:** The
  first Neo-side feedback attempt did not compile any source: the shell returned
  `cmake: command not found` before the build started. This is not a test
  failure. Prefer the repository's M4 build route; use an absolute local CMake
  path only if one is actually installed and do not install new software for
  this retry.
- **FM-R022 FOCUSED M4 EVIDENCE / EXPECTED DIGEST PASSED, 2026-08-13:** After
  the AWDL relay recovered, the authoritative tree was resynchronized and the
  focused File Manager interaction target built and passed 1/1 in 1.80 seconds
  on macOS 26.5 arm64. The passing target includes proof that the only live
  Selection PropertyList owns the bounded expected-SHA-256 editor, that one
  selected regular file enables it and retains an exact 64-character value,
  and that selecting a directory clears and disables it. This closes the
  structural/interaction verification for the expected-digest repair; complete
  File Manager and GUI.Forms regressions and native visual proof remain.
- **FM-R022 DEMOBOARD CONTEXT FAILURE / MEASURED ROOT CAUSE BEFORE TEST EDIT:**
  The isolated M4 diagnostic reproduced in 0.38 seconds and reported exactly:
  popup absent, focus-scope depth zero, focused stable ID
  `fm.selection.properties`, and authority text
  `Selection properties active · retained fixture projection`. Therefore the
  command executed, the popup closed, and its focus scope ended; the admitted
  Properties handler then correctly moved focus to its PropertyList. The old
  ObjectView-focus expectation is stale. Update only that oracle to require the
  command's destination focus, retain the other predicates, and rerun before
  removing the diagnostic.
- **FM-R022 DEMOBOARD CONTEXT ORACLE / CORRECTED AND FOCUSED PASS:** The
  diagnostic oracle now requires the retained PropertyList—the explicit
  destination of the Properties command—rather than incorrectly demanding that
  ContextMenu overwrite it by restoring ObjectView focus after execution. No
  product control behavior changed. The isolated M4 demoboard target then
  passed 1/1 in 0.49 seconds with its detailed failure diagnostic retained for
  future regressions. Run the complete GUI.Forms suite before closing this
  regression.
- **FM-R022 COMPLETE M4 REGRESSION / GREEN, 2026-08-13:** The current
  authoritative source was resynchronized to macOS 26.5 arm64, fully rebuilt,
  and passed the complete File Manager suite 10/10 in 0.70 seconds and the
  complete GUI.Forms suite 62/62 in 3.01 seconds. This closes the automated
  regression gate for the visible expected-digest repair, regular-file
  Terminal Here fallback, global semantic-ID oracle, and corrected demoboard
  Properties focus contract. It does not prove native visual fidelity or daily
  usability. Stage a fresh uniquely named signed bundle, then launch and
  dogfood it only through the M4 Aqua session when Screen Sharing is available.
- **FM-R022 BUNDLE PRE-STAGE SIGNATURE CHECK / FAILED BEFORE COPY:** The newly
  linked build-tree executable is a 1,979,856-byte arm64 Mach-O, but strict deep
  verification of its enclosing build-tree app failed with `code has no
  resources but signature indicates they must be present`. No candidate was
  copied into `CodexRuns` and no GUI was launched. Inspect the CMake resource /
  signing order, copy to a unique nonexisting destination only after the build
  is otherwise accepted, ad-hoc re-sign the complete staged bundle, and require
  strict deep verification there before calling it a dogfood candidate.
- **FM-R022 FRESH M4 DOGFOOD CANDIDATE / STAGED AND VERIFIED, NOT LAUNCHED:**
  The complete-suite build was copied without overwriting any prior bundle to
  `$HOME/Developer/CodexRuns/fmrepair-checksum-terminal-final-20260813.app`,
  then ad-hoc signed recursively. Deep strict verification passes; its signed
  executable is a 1,986,480-byte arm64 Mach-O with SHA-256
  `525f42ab5b692261f8d66ab1d1a0dffae7fdf106d0d988a4447388dc063791e5`.
  `CFBundleShortVersionString` is truthfully `0.0.1` and bundle build is `1`.
  This proves candidate identity and integrity only. It has not been launched;
  launch it from Terminal inside the M4 Screen Sharing session, never through
  SSH, and visually/input dogfood that exact path before making native claims.
- **FM-R022 SCREEN-SHARING LAUNCH FALSE START / KEY NAME REJECTED:** The exact
  verified bundle command was typed into a Terminal opened inside the M4 Aqua
  desktop, but the computer-control API rejected key name `ENTER`
  (`keyNotFound`) before sending a keystroke. This is not a product launch or
  failure. Inspect the fresh framebuffer to confirm the pending command, then
  send the supported `RETURN` key without duplicating the text.
- **FM-R022 SCREEN-SHARING LAUNCH FALSE START / CASE MUTATION CAUGHT BEFORE
  EXECUTION:** Fresh framebuffer inspection showed that synthesized typing
  lowercased the case-sensitive `$HOME/Developer/CodexRuns` spelling. Return
  was not sent, so the malformed command did not execute. Create a temporary,
  lowercase `runfm.app` symlink inside the M4's existing `CodexRuns`, clear the
  pending Terminal line, and launch through a lowercase `~/developer/codexruns`
  spelling (valid on this case-insensitive system). Remove only that temporary
  symlink after launch identity is established; do not rename or mutate the
  candidate bundle or mirrored source.
- **FM-R022 SCREEN-SHARING LAUNCH FALSE START / TILDE MUTATED:** From a fresh
  M4 Terminal prompt, `type_text` changed `~` into a backtick and Return left
  zsh at its `bquote>` continuation prompt. The File Manager did not launch.
  Leave that incomplete shell window alone, open another clean Terminal window,
  and use the shift-free path relative to the login home:
  `open developer/codexruns/runfm.app`. Inspect the typed line/result and exact
  process identity before attributing any visible File Manager window to the
  candidate.
- **FM-R022 EXACT M4 CANDIDATE / LAUNCHED AND VISIBLY IDENTIFIED:** A clean M4
  Terminal accepted the shift-free `open developer/codexruns/runfm.app`
  command. Screen Sharing then showed the foreground File Manager with the new
  `VERIFICATION / Expected SHA-256` PropertyList group, which the stale bundle
  did not contain. Read-only process inspection identifies PID 17317, started
  at 06:15:52, at the exact verified
  `fmrepair-checksum-terminal-final-20260813.app/Contents/MacOS/File Manager`
  path. Older breadcrumb PID 58066 remains separately alive; no inference from
  its window is used. Remove only the temporary `runfm.app` symlink, preserve
  both real bundles/processes, and continue native input dogfood on the visibly
  identified foreground candidate.
- **FM-R023 NATIVE SHA-256 COMMAND AVAILABILITY / INITIAL SUSPECT, DOWNGRADED
  AFTER CONTROL:** In
  exact PID 17317 on the M4, selecting real regular file `README.md` updates the
  preview and exact Selection facts; the new expected-digest TextBox is enabled
  and accepts/retains 64 lowercase hex characters through native input. Yet the
  visible Commands menu appeared to render `SHA-256…` disabled and the first
  pointer click did not execute it. However, the same Screen Sharing click on
  known-enabled `Copy path` merely highlighted the row and also left the menu
  open, so disabled-product behavior is not established; the relay may be
  dropping menu button-up/activation. Do not edit product state on this
  ambiguous observation. Exercise the already-open menu through keyboard
  navigation or another known-valid activation path, compare with the safe
  control row, and classify only from the resulting status/dialog behavior.
- **FM-R023 NATIVE SHA-256 COMMAND / PRODUCT CLEARED, RELAY POINTER DEFECT:**
  With the Commands menu still open, keyboard `Home` + `Return` invoked the
  canonical SHA-256 command. The M4 displayed its owned result dialog for
  `README.md`, including the exact object path, six bytes read, algorithm, a
  64-character digest, and `MISMATCH · expected digest is different.` The
  expected-value path is therefore natively reachable and functional; the two
  nonactivating pointer attempts were Screen Sharing menu input loss, not a
  disabled File Manager command. Dismiss with `No` to avoid changing the
  clipboard, then verify a real folder selection clears and disables the live
  expected-digest editor.
- **FM-R023 EXPECTED-DIGEST NATIVE END-TO-END / PASSED WITH RELAY NOTE:** In
  exact candidate PID 17317, selecting `README.md` enabled the visible
  PropertyList editor; native pointer focus plus synthesized text entered and
  retained exactly 64 `a` characters. Commands-menu keyboard activation
  computed the real six-byte file's SHA-256 and presented the owned
  `MISMATCH · expected digest is different` result. Single pointer clicks and
  several synthesized keys were swallowed by Screen Sharing at the native host
  dialog, but a bounded double-click on `No` dismissed it without copying.
  Selecting the real `Pictures` directory then changed preview/facts to folder
  state and visibly cleared the editor back to its disabled placeholder. The
  product feature passes end-to-end; the named relay limitation is not a File
  Manager failure.
- **FM-R024 AUTHORITATIVE BOARD COMPARISON / OPEN PRODUCT GAPS BEFORE EDIT:**
  Direct comparison of exact PID 17317 with the four canonical local renders
  confirms that the current app now owns the correct one-location geography,
  Watercolor title, classic menu, compact task shelf, graphite path/search
  chassis, one Home-rooted tree, white object field, and factual Selection
  pane. It is still visibly flatter/denser than the Folder board and does not
  yet present a Criteria mode. Criteria is not optional decoration:
  `DNA-S09`, `DDV-007-10`, and `DDV-007-11` make an instrument rack above an
  ordinary source-derived virtual folder GIVEN. But predicate cost
  classification and the staged-Apply threshold are explicitly unresolved.
  Do not fabricate a generic chip filter or claim Criteria complete. First map
  the existing Engine/Orchestrator query contract and demoboard criteria model
  to the minimum honest admitted slice; record any missing registry edge before
  implementation. Separately audit physical-depth tokens against the Folder
  board so visual tuning does not erase functional state distinctions.
- **FM-R024 CRITERIA CONTRACT MAPPING / ADAPTER COMPLETION GAP:** Frozen
  `ORC-ENG-001` already admits zero or more bounded exact metadata filters, and
  the Go Engine implements `name`, `path`, `kind`, `size_min`, `size_max`,
  `modified_after`, and `modified_before` with explicit invalid-query rules.
  The current Orchestrator `EngineSearchRequest` exposes only nonempty text and
  its JSONL adapter hard-codes `filters: {name: request.text}`; the C++ client
  mirrors that narrower shape. Therefore a truthful first Criteria slice is an
  incomplete projection of existing frozen semantics, not a new architectural
  contract. Extend the provider-neutral request and C++ client with a bounded
  exact-filter representation, permit filter-only catalogue queries, and keep
  the live-filesystem fallback ineligible when it cannot express those filters.
  Frontend Criteria may then expose only known-cheap intrinsic predicates and
  label source/generation. Do not expose content, plugin, fuzzy, or an expensive
  staged Apply module until their cost/channel contracts are admitted.
- **FM-R024 RUST SEAM FIRST PASS / IMPLEMENTED, LOCAL TOOLCHAIN ABSENT:** The
  provider-neutral request now carries a default-empty, ordered map bounded to
  the seven frozen exact metadata fields; a request is valid with text or
  filters, never both text and a duplicate `name` field, and filtered cursors
  must remain catalogue-sourced. The JSONL adapter merges ordinary text as the
  exact `name` predicate. The broker forces every filtered request to the
  catalogue lane even under `LiveOnly`, with a regression asserting no live
  call occurs. All request construction sites were updated and `git diff
  --check` passes. The Neo has no `cargo` executable, so local formatting/tests
  did not run; this is an environment limitation, not a pass. Run the complete
  M4 Orchestrator verification after the C++ projection is added.
- **FM-R024 FIRST M4 SEAM CHECKPOINT / FORMAT-ONLY FAILURE:** The M4 received
  the exact source and `cargo fmt --check` stopped before compilation on four
  purely mechanical rustfmt differences (one expression wrap, one import wrap,
  one assertion wrap, and import ordering). No Rust test or C++ build ran, so
  there is no behavioral result. Apply only those printed formatting changes
  on the authoritative Neo tree and rerun the identical checkpoint.
- **FM-R024 SECOND M4 SEAM CHECKPOINT / RESULT LOST TO COMPACTION:** The
  formatting corrections were applied and the focused M4 command was issued,
  but its output exceeded the conversation context and no pass/fail result
  survived. The Codex desktop terminal reader subsequently hung without
  returning content and was terminated. This is unproven work, not a pass.
  Rerun the focused Rust formatting/tests and C++ client build verbatim before
  beginning frontend Criteria integration; retain the new result here.
- **FM-R024 REPEATED M4 SEAM CHECKPOINT / PASSED:** The authoritative Neo tree
  was mirrored again and the complete focused checkpoint passed on the M4:
  `cargo fmt --check`; two `engine_contract` unit tests; the filter-only JSONL
  projection unit test; all six `engine_port` integration tests, including the
  no-live-widening regression; and configure/build of the C++ Orchestrator
  client. The only output of note was three pre-existing Rust deprecation
  warnings for `Atomic::fetch_update`; they are outside this Criteria seam and
  did not affect the pass. Frontend work may now consume the bounded exact
  filters, but the complete canonical Orchestrator gate remains required before
  final handoff.
- **FM-R025 CRITERIA ENGINE REALITY CHECK / METADATA-ONLY QUERY BLOCKED:** A
  second read of the executable Go M1 exact path found a material constraint
  that the provider-neutral request shape alone did not reveal:
  `exact.QueryIndex` rejects a query unless `filters.name` or `filters.path` is
  present. A visible Kind/Size/Modified Criteria rack would therefore compile
  through the repaired Orchestrator seam but fail against the actual Engine
  whenever it contains only the intrinsic predicates shown in the prototype.
  Do not publish or dogfood that false surface. Determine whether a bounded
  catalogue-wide candidate scan already exists or can be admitted inside the
  frozen exact-filter semantics with explicit resource-budget behavior and
  control tests. If not, keep Criteria visibly unavailable and record the
  missing Engine capability instead of synthesizing results.
- **FM-R025 DISPOSITION / BOUNDED INTERNAL M1 COMPLETION ADMITTED:** Required
  Engine reading confirms this is not a new query family or storage decision.
  The canonical frozen ORC-ENG semantic request admits zero or more exact
  literals plus bounded intrinsic metadata filters; Architect Handoff 001 and
  the M1 delivery plan require exact basic-metadata queries over a committed
  generation; `API_CONTRACT.md` labels the name/path anchor as the currently
  implemented subset, not an exclusion. Complete that subset with a
  representation-neutral `CandidateAll(maximum)` exact-reader operation. It
  must return every generation-local live binding ordinal only when the whole
  candidate set is within the existing 100,000 ceiling; otherwise return the
  existing resource-budget failure without partial success. Prove reference
  pagination, metadata evidence, out-of-scope exclusion, budget refusal, and
  durable/reference equality. Do not change storage format or expose reader
  ordinals across the public API.
- **FM-R025 PRE-BUILD DIFF REVIEW / MISSING IMPORT AND TERMINAL MAPPING:** The
  first bounded-scan patch is structurally small and `git diff --check` passes,
  but inspection caught that the durable reader's new `context.Context`
  signature needs a `context` import. It also exposed that the common query
  path would wrap a cancellation raised during candidate enumeration as an
  integrity fault. Correct both before the first M4 build and add a regression
  requiring `context.Canceled` to survive a metadata-only scan. This is a
  source-review catch, not a compiler failure or passing Engine result.
- **FM-R025 FIRST M4 ENGINE CHECKPOINT / FORMAT GATE FAILED:** The focused M4
  command exited 1 at `test -z "$(gofmt -l …)"`; the chained Go tests did not
  run. That guard suppressed the formatter's filename list, so the exact
  mechanical differences are not yet known. Run `gofmt -d` on the six touched
  Go files, apply only its output to the authoritative Neo tree, then repeat
  the formatter gate and focused exact/generation tests.
- **FM-R025 REPEATED M4 ENGINE CHECKPOINT / PASSED:** After applying only the
  `gofmt -d` alignment changes and the pre-build cancellation/import fixes, the
  M4 formatter gate passed. `go test ./internal/exact ./internal/generation`
  passed (`internal/exact` 0.323 s; `internal/generation` 2.797 s). The tests
  now cover metadata-only paging, metadata evidence, scope, generation-bound
  cursor expiry, cancellation, budget refusal, and reference/durable reader
  equality. Full Engine test/race/vet gates and installed-provider dogfood are
  still outstanding; this checkpoint proves only the bounded exact-query seam.
- **FM-R026 RETAINED CRITERIA FIRST SLICE / IMPLEMENTATION CONTRACT BEFORE
  EDIT:** Add Criteria as a checkable content mode under View, independent of
  the Small Icons/Details presentation radio pair. Its shallow retained
  `InstrumentRack` sits above the existing ordinary `ObjectView`; Selection and
  Properties stays visible. Open with Kind (`is`) and Modified
  (`after`/`before`, `YYYY-MM-DD`) modules and offer the third admitted Size
  (`at least`/`at most`, bytes) module through a bounded `+ module` action. Each
  module must expose the InstrumentRack's enable and remove controls, validate
  locally, and issue a new filter-only catalogue request only on a committed
  edit/toggle/remove/add. Results must show exact catalogue source and
  generation; paging must remain on the catalogue cursor. Navigation or a text
  search exits Criteria. Do not expose Content/Similarity/plugins, call these
  predicates cheap, or add a staged Apply until those unresolved contracts are
  admitted.
- **FM-R026 COMMAND-SURFACE PATCH FALSE START / NO SOURCE CHANGE:** The first
  combined patch for View-menu projections included a duplicate expected
  `view_menu_->set_items` context that does not exist. `apply_patch` rejected
  the entire patch before changing `application.cpp`. Re-locate the actual
  shelf-overflow, persistent menu-strip, and shelf View popup blocks and patch
  each exact site once; do not infer any command wiring from this failed call.
- **FM-R026 TRANSITION REVIEW / PREMATURE CONSOLE HIDE:** The first navigation
  transition draft set `criteria_showing_ = false` and hid the console before
  `apply_directory` could observe that focus was returning from a virtual
  surface. Keep the old retained console visible during the asynchronous read
  (while the mode flag and generation are already cancelled), then hide it when
  the successful directory snapshot is applied. This preserves deterministic
  ObjectView focus without leaving Criteria logically active.
- **FM-R026 STATIC INTEGRATION REVIEW / THREE PRE-BUILD CORRECTIONS:** The
  complete source audit found (1) the criteria worker lambda used the C++23
  shorthand `[captures] mutable` while this target is C++20 and therefore needs
  `[captures]() mutable`; (2) `request_navigation` cancelled search/Criteria
  state before validating the requested path, so a refused path could leave a
  logically inactive but still visible virtual surface; and (3) the add button
  was permanently named “Add Size” even after another module was removed.
  Correct all three before the first frontend compile. None is a compiler/test
  result yet.
- **FM-R026 FIRST M4 FRONTEND COMPILE / PASSED:** After the static corrections,
  the M4 built `file_manager_application` and
  `file_manager_application_interaction_tests`, including the expanded C++
  Orchestrator client, with warnings-as-errors and no compiler/linker failure.
  This is compile evidence only. No Criteria behavioral assertion or native
  GUI interaction has passed yet.
- **FM-R026 CRITERIA TEST REVIEW / ORACLE CORRECTIONS BEFORE RUN:** The new
  focused scenario initially queried the existing text-search loading probe
  after live-source refusal instead of Criteria loading, and its narrow-layout
  assertion assumed Size would follow Kind even though removing/re-adding Kind
  correctly appends it. Add the Criteria-loading probe and assert that either
  module has a greater wrapped-row `y`, independent of module ordering. These
  are test-oracle defects caught before execution, not product failures.
- **FM-R026 FIRST FOCUSED INTERACTION CHECKPOINT / TEST COMPILE FAILURE:** The
  M4 rebuilt the already-compiling application library, then stopped compiling
  `application_interaction_tests.cpp` because `require(modified_value, …)`
  cannot use `shared_ptr`'s explicit boolean conversion for a by-value `bool`
  parameter. No interaction test ran. Change only that assertion to
  `static_cast<bool>(modified_value)` and rerun the same build/ctest command.
- **FM-R026 REPEATED FOCUSED INTERACTION CHECKPOINT / PASSED:** The M4 rebuilt
  the interaction executable and its single registered test passed in 2.07 s.
  The scenario proves retained Criteria mode/menu truth; default Kind and
  Modified modules; field/operator/value/enable/remove controls; UTC date
  conversion; local invalid-date state; disabled-predicate omission; bounded
  Size add/remove/re-add; ordinary ObjectView catalogue results with generation
  label; refusal of a filtered `live` page without replacing prior objects;
  narrow-width wrapping/growth; unique semantic IDs; and exit back to a real
  directory. This does not yet prove the installed M4 Engine binary accepts a
  metadata-only query or that the native rendered surface matches the board.
- **FM-R027 INSTALLED METADATA-QUERY PROBE / LOCAL QUOTING FALSE START:** The
  first attempt to combine a fixture search, remote process listing, runtime
  listing, and authenticated JSON request into one shell command failed locally
  with `zsh: unmatched` before SSH executed. This says nothing about the M4
  Engine. Split the read-only orientation and query into simpler quoted calls,
  then retain the exact installed provider response.
- **FM-R027 CONTAINED M4 ENGINE / INSTALLED BINARY CONFIRMED STALE:** The
  authenticated read-only query against the exact contained runtime
  `$HOME/Library/Caches/com.filemanager.engine.fm1`, root `fm1-contained`,
  reached the running provider and returned
  `INVALID_QUERY: the M1 exact engine requires filters.name or filters.path`.
  Therefore the source and focused tests are ahead of the installed service;
  native Criteria cannot work until that binary is upgraded. Run the full Go
  quality gate first, preserve the current installed executable as a
  content-hash-named rollback copy, ad-hoc sign and verify the replacement as
  this host-bound profile requires, restart only
  `com.filemanager.engine.fm1-contained`, and repeat this exact query before
  touching the native frontend bundle.
- **FM-R027 FIRST FULL M4 ENGINE GATE / FORMAT GUARD INVALID AND ATLAS STALE:**
  The combined gate exited 1 at the final generated-library-doc check. The
  opening format guard also did not prove formatting: the relay shell could
  not find `gofmt`, its failed command substitution produced no stdout, and
  `test -z` therefore continued. Between those two points, `go test ./...`,
  `go test -race ./...`, `go vet ./...`, and both Linux-amd64 and
  Windows-amd64 cross-builds passed. The race builds emitted the existing
  Apple-linker `malformed LC_DYSYMTAB` warnings but no test failed. The docs
  checker reported the service atlas stale in `README.md`, `manifest.js`,
  `manifest.json`, and the exact/generation/service Markdown and HTML pages.
  Repeat the format check with `/opt/homebrew/bin/gofmt`, regenerate the atlas
  from the authoritative source, inspect those generated changes, then rerun
  the full gate; do not treat this first invocation as a pass.
- **FM-R027 FORMATTER-PATH FOLLOW-UP / ASSUMED HOMEBREW SHIM ABSENT:** A
  local follow-up attempted `/opt/homebrew/bin/gofmt` and failed with “no such
  file or directory.” The prior M4 output proves only the `go` shim exists at
  that location, not `gofmt`. Resolve `GOROOT` through the Mini's absolute Go
  binary and invoke `$GOROOT/bin/gofmt` explicitly in the repeated gate.
- **FM-R027 REPEATED FULL M4 ENGINE GATE / PASSED:** After regenerating and
  inspecting the nine deterministic service-atlas artifacts, the corrected
  gate resolved `gofmt` through the Mini's Go 1.25.5 `GOROOT` and passed with
  no formatted-file output. `go test ./...`, `go test -race ./...`,
  `go vet ./...`, Linux-amd64 and Windows-amd64 cross-builds, and
  `generate_library_docs.py --check` all passed. The race-linked packages
  again emitted Apple `malformed LC_DYSYMTAB` linker warnings, but no package
  or assertion failed. The atlas reports 20 packages and 1,096 production
  declarations. Source is now eligible for the contained-provider replacement;
  installed behavior remains unproven until the signed binary is replaced and
  the authenticated metadata-only query succeeds.
- **FM-R027 CONTAINED DEPLOYMENT ORIENTATION / SHARED EXECUTABLE:** The
  contained LaunchAgent is running as PID 1304 from
  `$HOME/.local/bin/fileman-engine`, SHA-256
  `7c3a99ef52f1b3bbdf865c96387f53938505b33c17897cdc590120245c4e32b3`,
  with a valid ad-hoc arm64 signature. Its manifest is the expected
  `fm1-contained` deployment rooted only at `CodexRuns/fmsandbox`. The separate
  `com.filemanager.engine.m4-dogfood` LaunchAgent references the same installed
  executable. Replacing the file therefore updates the next executable image
  for both profiles, although only the contained label will be restarted for
  this repair. Preserve the old bytes under that exact content hash before an
  atomic signed replacement; do not restart or otherwise disturb the old
  dogfood profile.
- **FM-R027 CONTAINED ENGINE REPLACEMENT / SIGNED AND RESTARTED:** The full-gate
  arm64 candidate was installed through a temporary sibling and atomic rename.
  The previous bytes remain at
  `$HOME/.local/bin/fileman-engine.rollback-7c3a99ef52f1b3bbdf865c96387f53938505b33c17897cdc590120245c4e32b3`
  and re-hashed correctly before replacement. Ad-hoc signing and strict
  verification passed before and after the rename. The installed signed
  executable is SHA-256
  `a0077a7280bf31211270bbbba28bf25d329aa7e05d96900ee6272577be0d9418`;
  only `com.filemanager.engine.fm1-contained` was kicked, respawning as PID
  23650. The separate dogfood job was not restarted. Query behavior and service
  stability are the next proof; installation alone is not the acceptance.
- **FM-R027 FIRST POST-RESTART QUERY / JSON QUOTING FAILED BEFORE TRANSPORT:**
  The combined status/query invocation reached the new `call-local` CLI but
  nested SSH shell quoting removed the JSON key quotes. The CLI stopped at
  `decode request: invalid character 'i' looking for beginning of value`; no
  authenticated request reached the service. Reissue the same static payloads
  through base64 decoding and do not infer provider behavior from this call.
- **FM-R027 BASE64 PROBE CONSTRUCTION / ORCHESTRATOR RUNTIME FALSE START:** The
  first attempt to encode the fixed payloads inside the tool's JavaScript
  isolate failed because that runtime does not provide `btoa`. It failed before
  invoking a shell or SSH and supplies no Engine evidence. Use the host's
  `/usr/bin/base64` to construct the static nonsecret payloads instead.
- **FM-R027 INSTALLED CONTAINED ENGINE / METADATA-ONLY ACCEPTANCE PASSED:** A
  fixed authenticated call against the restarted contained runtime reported
  `ready=true`, catalogue generation 2, ten indexed records, and no active
  work. The same request that the prior binary rejected—descendant scope
  `fm1-contained`, exact filter `kind=file`, limit 3—now returned three exact
  catalogue records (`field-notes.txt`, `ledger.csv`, and `read-me.txt`) plus a
  generation-bound next cursor, with `partial=false` and only the `exact`
  channel in the plan. This proves the installed Engine filter-only seam on a
  real contained folder. It does not yet prove the installed Orchestrator
  forwards filters or that the native frontend renders/operates Criteria.
- **FM-R028 ORCHESTRATOR FILTER PROBE / OBSERVABILITY CONTRACT BEFORE EDIT:**
  The reusable C++ client supports exact filters, but its executable exposes
  only text search, so the installed service cannot yet be discriminated from
  an older build that ignores unknown `filters`. Add one conformance-only
  `criteria ROOT_ID FIELD VALUE [MAX_RESULTS]` command. It must call
  `search_subtree` with empty text and one exact filter and print terminal,
  source, completeness, optional generation, result count/first name, and
  cursor. This adds no product feature or service mutation; it makes the
  already-frozen request field observable for authenticated end-to-end proof.
- **FM-R028 FIRST LIVE ORCHESTRATOR FILTER PROBE / INSTALLED BROKER STALE:** The
  expanded C++ conformance client configured and built successfully on the M4,
  authenticated to the installed default Orchestrator runtime, and issued
  `criteria fm1-contained kind file 3`. The live service returned
  `invalid: search request is not well formed`. Because the separately
  authenticated contained Engine now accepts the identical filter-only
  semantic request, this isolates the remaining deployed break to the older
  installed Orchestrator validation/forwarding path. Run the complete
  Orchestrator verification gate, preserve and replace its exact signed release
  artifact using the recorded persistent LaunchAgent layout, restart only
  `com.filemanager.orchestrator`, then repeat this C++ probe before building the
  frontend bundle.
- **FM-R028 FIRST FULL M4 ORCHESTRATOR GATE / SERVICE ATLAS STALE:** The native
  atomic-worker library, benchmark target, and test executable built; its one
  CTest passed in 0.54 s. Rust formatting then passed silently, but the next
  generated-doc check stopped the script before Rust tests, Clippy, or release
  build. It reported exactly eight stale generated artifacts: both manifests,
  Markdown for `engine_contract`, `engine_jsonl`, and `engine_port`, and those
  three HTML pages. Regenerate and inspect that deterministic atlas from the
  authoritative Neo source, then repeat the complete gate from the top; this
  invocation is not a release pass.
- **FM-R028 SECOND FULL M4 ORCHESTRATOR GATE / TESTS PASS, CLIPPY BLOCKS
  RELEASE:** The worker CTest passed, formatting and the regenerated 56-file
  atlas check passed, and every printed locked Rust/C++/hostile suite passed:
  68 library tests, 9 CLI tests, 1 independent C++ client test, 4 Engine
  contract tests, 6 Engine-port tests, 2 fixture tests, and one each for live
  search, hostile local frames, and local-wire fixtures. Clippy with warnings
  denied then rejected `EngineSearchRequest::is_well_formed` for a
  nonminimal boolean expression and one `Default::default()` filter fixture in
  `engine_jsonl.rs` for unclear trait access. The script stopped before the
  locked release build. Rewrite the validation as explicit early returns and
  name `BTreeMap::default()` in the fixture, regenerate any line-indexed atlas
  changes, then repeat the complete gate; do not deploy this partial result.
- **FM-R028 CLIPPY-FIX PATCH / WRONG CROSS-FILE CONTEXT REJECTED:** The first
  combined patch mixed the `engine_jsonl` test-import anchor into the
  `engine_contract.rs` file section. `apply_patch` rejected the entire patch;
  neither lint correction was applied. Patch the validation and fixture files
  independently against their exact contexts.
- **FM-R028 LOCAL FORMATTER INVOCATION / TOOLCHAIN NOT ON NEO PATH:** The two
  lint corrections applied, but the immediate local format command did not:
  `rustfmt` was absent and the fallback `rustup` command was also not on this
  shell's PATH. The atlas generator still ran afterward, so those generated
  files reflect current semantics but not yet a formatter-verified source
  layout. Resolve the Neo's absolute Rust toolchain path, format the two source
  files, regenerate the line-indexed atlas again, and only then rerun the M4
  gate.
- **FM-R028 THIRD FULL M4 ORCHESTRATOR GATE / ONE RUSTFMT JOIN REMAINS:** The
  worker CTest passed in 0.02 s, then the Mini's authoritative
  `cargo fmt --check` stopped the script. Rustfmt requires the closing
  `self.filters.iter().all(...)` expression and the following
  `&& self.budget.is_well_formed()` to share one line. No docs check, Rust test,
  Clippy, or release build ran in this invocation. Apply exactly that printed
  formatting diff, regenerate the atlas, and repeat the full gate.
- **FM-R028 FOURTH FULL M4 ORCHESTRATOR GATE / ALL TESTS PASS, TWO ALL-TARGET
  CLIPPY FINDINGS:** Formatting, the 56-file atlas check, worker CTest, and the
  complete locked test suite all passed again. Clippy then reached additional
  targets and rejected (1) the `POLICIES` test constant in
  `tests/engine_port.rs` because it was declared after executable statements,
  and (2) `Default::default()` for filters in
  `examples/engine_jsonl_probe.rs`, which must name `BTreeMap::default()`.
  The release build did not run. Move only the test constant to the start of
  its function and name/import `BTreeMap` in the example, then repeat the full
  gate.
- **FM-R028 FIFTH FULL M4 ORCHESTRATOR GATE / PASSED:** One uninterrupted run
  passed the atomic-worker CTest, `cargo fmt --check`, the current 56-file
  service atlas, every locked Rust/C++/hostile suite, Clippy across all targets
  and features with warnings denied, and the optimized locked release build.
  The printed executable suites again comprised 68 library, 9 CLI, 1 C++
  client, 4 Engine-contract, 6 Engine-port, 2 fixture, and one each live-search,
  hostile-frame, and local-wire tests, all passing. The release artifact is now
  eligible for rollback-safe installation; the live filtered route remains
  unproven until replacement/restart and a repeated C++ Criteria probe.
- **FM-R028 ORCHESTRATOR DEPLOYMENT ORIENTATION / OLD PROCESS LOST ENGINE
  CONNECTION:** The installed persistent artifact is a valid ad-hoc arm64
  binary, SHA-256
  `f7dbc8060cc2b7cf743a164efaad56b9ad9f3ea5cce1fff4c390bf4e8894f79a`,
  running as PID 978 from the accepted Application Support path. The gated
  release candidate is arm64 SHA-256
  `b0ca6d0c08c0d95d0a1662d53070dfe7d7df569aa3a26dccd42177b40a52843b`
  before explicit replacement signing. The pre-restart C++ bootstrap remained
  Core-ready but reported Engine unavailable, consistent with its process-held
  provider connection being severed by the earlier Engine restart. The current
  launchd adapter discovers the exact contained cache runtime on startup.
  Preserve the installed bytes under the full old hash, sign/verify an atomic
  replacement, restart only `com.filemanager.orchestrator`, then require both
  Engine-ready bootstrap and filtered catalogue success.
- **FM-R028 ORCHESTRATOR REPLACEMENT / SIGNED, AWAITING SOCKET ACTIVATION:** The
  old artifact was preserved and hash-verified at
  `orchestrator.rollback-f7dbc8060cc2b7cf743a164efaad56b9ad9f3ea5cce1fff4c390bf4e8894f79a`.
  The replacement was installed through a temporary sibling, ad-hoc signed,
  strictly verified before and after atomic rename, and now has SHA-256
  `57e8a0a7ae93ceacfb493e00c9acb6ee1a4d810bbaf53d922e3cf811f38ebf4f`.
  Only `com.filemanager.orchestrator` was kicked. One second later launchd
  reported the socket objects active, run count 2, and process `not running`;
  this accepted socket-activated service must now be activated by an
  authenticated client before judging health. Require a new running PID and
  both bootstrap/provider and filtered-query proof.
- **FM-R028 INSTALLED ORCHESTRATOR FILTER ROUTE / END-TO-END PASSED:** The
  authenticated C++ client activated the replacement as PID 27743 and received
  a Core-ready bootstrap with the contained Engine `ready`, currentness
  `manual_reconcile`, and direct fallback available. Its filter-only request
  `criteria fm1-contained kind file 3` returned terminal `success`, source
  `catalogue`, `complete=false`, generation 2, three results beginning with
  `field-notes.txt`, and a catalogue cursor. This proves the installed C++ →
  Orchestrator → authenticated Engine path preserves exact filters and paging.
  Native frontend rendering and user interaction remain the next acceptance.
- **FM-R029 CRITERIA ENTRY TRUTHFULNESS / REVIEW GAP BEFORE EDIT:** The product
  `show_criteria()` currently switches the rack/title and begins an async
  filtered request without clearing the ordinary directory snapshot already
  in ObjectView. Until the first response arrives—or indefinitely on an
  unavailable provider—the prior directory can be misread as Criteria matches.
  On initial entry, clear ObjectView and Selection/Properties immediately,
  retain an explicit catalogue-loading status, and add an interaction
  regression proving old directory objects disappear before the response.
  Subsequent committed-filter refreshes may retain the last proven Criteria
  page while the new page is in flight, but initial entry must never relabel
  unrelated objects.
- **FM-R029 CRITERIA ENTRY TRUTHFULNESS / FOCUSED M4 PASS:** Production now
  routes initial Criteria entry through one retained-surface transition that
  clears the directory entries, ObjectView items, selection, and inspector
  before showing the rack and catalogue-waiting state. The interaction probe
  uses that same transition and asserts the old `root.txt` is absent before a
  response is injected. The warnings-as-errors application and interaction
  targets rebuilt, and the focused CTest passed in 2.36 s. Full frontend and
  GUI.Forms gates plus native rendered interaction remain outstanding.
- **FM-R030 FULL M4 FRONTEND AND GUI.FORMS GATE / PASSED:** The Release
  frontend was reconfigured against the named installed GUI.Forms package and
  rebuilt, including regenerated Web.Forms output, application, document
  picker, all tests, and `File Manager.app`. All 10 File Manager CTests passed
  (the interaction executable in 1.81 s), and all 62 GUI.Forms CTests passed in
  3.07 s, including instrument, semantic, menu, macOS host, Skia, drawing, and
  demoboard coverage. This is the full automated pre-staging gate for the
  current Criteria route. It does not replace a fresh signed-bundle identity
  check or native Screen Sharing dogfood on the M4.
- **FM-R030 FRESH CRITERIA DOGFOOD BUNDLE / STAGED AND VERIFIED:** The green
  build-tree app was copied without overwriting any prior candidate to
  `$HOME/Developer/CodexRuns/fmrepair-criteria-final-20260813.app`. Both
  embedded GUI.Forms dylibs and the app envelope were ad-hoc re-signed; deep
  strict verification passed. Its executable is native arm64 with SHA-256
  `830d8f1e3d44d8357366c6b4589dfcd8dde6ed74ba070861dadea1c3b00f9ae2`,
  and bundle metadata remains truthfully `0.0.1`, build 1. This proves staged
  candidate identity only. Launch this exact bundle from Terminal inside the
  M4 Screen Sharing session, confirm its exact PID/path, and exercise the
  retained Criteria controls and real catalogue results before making native
  claims.
- **FM-R030 SCREEN SHARING COMPUTER-USE ENTRY / NEO DENIED FRAMEBUFFER:** The
  first required Computer Use inspection of `Screen Sharing` returned no
  screenshot and said “The Mac is locked and automatic unlock could not unlock
  it.” Per the already-established host behavior and architect correction,
  this lock report concerns the controlling MacBook Neo / automation access;
  it is not evidence that the M4 desktop is locked and says nothing about the
  staged app. Retry the app by its actual bundle identifier and inspect the
  Screen Sharing relay/process read-only. Do not work around this by launching
  the GUI through SSH or by opening Chrome on the Neo.
- **FM-R030 SCREEN SHARING BUNDLE-ID RETRY / SAME NEO-WIDE DENIAL:** Computer
  Use could not enumerate applications while the Neo lock gate was active, and
  a direct retry against `com.apple.ScreenSharing` returned the identical
  automatic-unlock denial. This confirms an automation-host gate, not an app
  naming error. The current candidate remains staged and unlaunched. Native
  acceptance must resume after the Neo is manually unlocked; no SSH GUI launch
  is an acceptable substitute.
- **FM-R030 LEDGER/RELAY COMBINED PATCH / WRONG WRAP ANCHOR REJECTED:** A
  combined attempt to append the bundle-ID retry and then inspect relay state
  used a context line whose Markdown wrapping did not match the ledger.
  `apply_patch` rejected the whole tool script before the read-only inspection
  ran. The bundle and all services were untouched. Append against the exact
  tail, then run relay/process inspection separately.
- **FM-R030 RELAY ORIENTATION / M4 AQUA ALIVE, NEO TUNNEL NOT LISTENING:**
  Read-only inspection found the M4 console `WindowServer`, `loginwindow`, and
  `screensharingd` running; the repaired Orchestrator also remains running as
  PID 27743. On the Neo, no process was listening on TCP 5901 despite a transient
  `pgrep` match. Thus the native gate currently has both a missing SSH relay and
  the separate Neo Computer Use lock denial. This is not an M4 or File Manager
  crash. Re-establish the documented 5901 relay after Neo unlock, open Screen
  Sharing there, and launch the exact staged bundle only from the remote Aqua
  Terminal.
- **FM-R030 NATIVE LAUNCH CONTRACT / REQUIRED ARGUMENTS RECORDED:** The current
  executable admits Engine search/Criteria only when an explicit Engine root ID
  accompanies an admitted filesystem root. The eventual Aqua Terminal launch
  must therefore open the exact staged bundle with
  `--root $HOME/Developer/CodexRuns/fmsandbox --engine-root-id fm1-contained`.
  Launching the bundle without those arguments and then observing Criteria
  disabled would be a test-setup error, not a product regression. Do not add
  mutation/quarantine flags to this read-only Criteria run.
- **FM-R030 RELAY RESTART / PORT-STATE RACE:** The documented SSH forward
  command immediately reported `bind [127.0.0.1]:5901: Address already in use`,
  despite the prior `lsof` check showing no listener. Do not kill an unknown
  process or assume the relay is bad. Resolve the current 5901 owner and test
  whether it already reaches the M4 VNC endpoint before opening Screen Sharing.
- **FM-R030 RELAY RESOLUTION / ACTIVE OVER IPV6 LOOPBACK:** The new SSH process
  owns `[::1]:5901` as PID 33413, and the existing Screen Sharing process has an
  established loopback connection to that port. The earlier bind diagnostic
  concerned the competing IPv4 address; the IPv6 relay is live and must be
  left intact for this run. Native inspection can retry through the already
  connected Screen Sharing app.
- **FM-R031 INSTALLED-SERVICE APPLICATION PROBE / ACCEPTANCE BEFORE EDIT:** Add
  an opt-in scenario to the existing application interaction executable,
  enabled only by explicit root/Engine-ID environment variables so ordinary
  hermetic CTest remains service-independent. The scenario must instantiate
  the production Application over the real contained root, wait for direct
  folder enumeration, execute the same retained `view.criteria` command as the
  menu, drain the actual worker/UI handoff, and require: Criteria checked and
  no longer loading, the retained rack/ObjectView visible, at least one real
  file result, a `CATALOGUE GENERATION` derivation label, and no
  CorrespondenceView substitution. Run it explicitly on the M4 against
  `fmsandbox` / `fm1-contained`; failure is a frontend integration failure, not
  equivalent to the lower-level C++ probe.
- **FM-R031 INSTALLED-SERVICE APPLICATION PROBE / PASSED ON M4:** The
  warnings-as-errors interaction executable rebuilt and ran with the explicit
  contained root and `fm1-contained` ID. Its production Application enumerated
  the real directory, executed retained command `view.criteria`, traversed the
  real worker/UI and authenticated service route, and rendered five catalogue
  files in ordinary ObjectView; the first was `blue-hour.png`. The retained
  derivation reads `CURRENT SUBTREE · EXACT VIRTUAL FOLDER · CATALOGUE
  GENERATION 2`, Criteria was checked and no longer loading, and
  CorrespondenceView remained hidden. Every existing interaction scenario also
  passed in that executable. This final edit is test-only, so the signed app
  candidate remains product-source current. Native pixels and physical input
  are still separately pending behind the Neo lock gate.
- **FM-R032 INSTALLED CRITERION EDIT CONTROL / ACCEPTANCE BEFORE EDIT:** Extend
  the opt-in installed Application scenario beyond initial load. Obtain the
  production Kind value ComboBox from the retained InstrumentRack, change it
  through its real selected-index API from Files to Folders, let the subscribed
  committed-change handler issue the new async request, and require the
  generation-labelled ObjectView to replace file results with actual directory
  results. Record concrete before/after names. This proves one visible editor's
  complete control → Application → Orchestrator → Engine → ObjectView loop;
  native pointer/keyboard paint remains separately pending.
- **FM-R032 INSTALLED KIND CONTROL / END-TO-END PASSED:** On the M4, the opt-in
  production Application first rendered five file results led by
  `blue-hour.png`. Changing the retained Kind value ComboBox from `Files` to
  `Folders` through its real selected-index event caused the subscribed
  Application handler to issue a new filtered request; ObjectView then rendered
  four directory results led by `Documents`, still labelled catalogue
  generation 2. All interaction scenarios passed. This proves one visible
  criterion editor's complete control-to-result loop; physical pointer,
  keyboard, and paint comparison remain native-only gaps.
- **FM-R033 INSTALLED ENABLE CONTROL / ACCEPTANCE BEFORE EDIT:** In the opt-in
  installed scenario, activate the production Modified module CheckBox through
  `on_activate`, not through the rack's test/model setter. Require its checked
  state to become false, the Application search generation to advance, the
  retained module to report `disabled · no filter emitted`, loading to settle,
  and the real catalogue ObjectView to remain valid under the remaining Kind
  predicate. This proves the visible enable control dispatches a new filtered
  request and does not merely repaint locally.
- **FM-R033 INSTALLED MODIFIED ENABLE CONTROL / END-TO-END PASSED:** Activating
  the production Modified CheckBox through `on_activate` toggled it off,
  advanced the Application's actual request generation, retained disabled state
  and status `disabled · no filter emitted`, and settled another successful
  catalogue page under the remaining Kind predicate without losing the four
  real folder results. The full interaction executable passed. The visible
  enable control is therefore operational through the installed service path;
  native pointer/keyboard paint remains unproved.
- **FM-R034 INSTALLED REMOVE/ADD CONTROLS / ACCEPTANCE BEFORE EDIT:** Continue
  the installed Application scenario by activating the production Modified
  remove button and requiring that module to disappear, `+ module` to remain
  enabled, request generation to advance, and the Kind-only catalogue page to
  settle. Then activate the production `+ module` button, require Modified to
  be restored exactly once with its default retained controls, require another
  generation advance and settled real page, and retain unique semantic IDs.
  This proves both visible buttons alter the installed query, not only local
  module geometry.
- **FM-R034 FIRST REMOVE/ADD PROBE BUILD / PRIVATE BUTTON HOOK REJECTED:** The
  interaction target stopped at compile time because `Button::on_activate()` is
  private; neither installed control scenario ran. CheckBox exposes that method
  publicly, but ordinary Button intentionally requires its public input or
  semantic route. Replace only the two direct calls with Window semantic
  `press` actions against `fm.criteria.modified.remove` and `fm.criteria.add`,
  then rerun the same installed assertions.
- **FM-R034 INSTALLED REMOVE/ADD CONTROLS / END-TO-END PASSED:** Window semantic
  `press` on the production Modified remove button removed exactly that module,
  left Kind, kept `+ module` available, advanced request generation, and
  settled the installed catalogue page. Semantic `press` on `+ module`
  restored exactly one Modified module with its retained value editor, advanced
  generation again, and settled another real catalogue page. Global retained
  and virtual semantic IDs remained unique, and the complete interaction
  executable passed. These visible controls are operational through the
  installed service path; native pointer/keyboard paint remains pending.
- **FM-R035 INSTALLED TEXT VALIDATION/RECOVERY / ACCEPTANCE BEFORE EDIT:** In
  the installed Application scenario, use the restored production Modified
  TextBox and its public Return-key commit path. Commit impossible date
  `2026-02-30`; require local invalid module/field state, no loading, and an
  empty ObjectView so prior results cannot masquerade as matches. Then commit
  `2026-01-01`; require the module to return live, its validation message to
  clear, and the real generation-labelled catalogue folder results to recover.
  This proves the visible free-text field fails locally and recovers through
  the installed query route.
- **FM-R035 INSTALLED TEXT VALIDATION/RECOVERY / END-TO-END PASSED:** The
  restored production Modified TextBox accepted Return-key commit of
  `2026-02-30`, marked the module invalid with a field validation message,
  stayed out of loading, and cleared ObjectView so prior matches could not be
  mislabeled. Recommitting `2026-01-01` through Return cleared validation,
  restored the module to live, and recovered the four real catalogue folder
  results on generation 2. The complete interaction executable passed. Local
  validation and installed-service recovery are functional; native edit paint
  and physical typing remain pending.
- **FM-R036 POST-INSTALLED-CAMPAIGN HERMETIC FRONTEND GATE / PASSED:** With no
  installed-probe environment variables, the ordinary M4 frontend CTest suite
  still passed 10/10 in 1.43 s. The opt-in live scenario therefore does not
  introduce a daemon/filesystem dependency into normal tests. Product source
  has not changed since the already staged signed Criteria bundle; all changes
  after staging are interaction-test and ledger evidence only.
- **FM-R036 FINAL SCREEN SHARING RETRY / NEO STILL LOCKED:** With the IPv6 5901
  relay live and Screen Sharing connected, a final Computer Use inspection
  still returned the same Neo automatic-unlock denial and no framebuffer. The
  exact signed Criteria bundle remains intentionally unlaunched; no native
  visual/input claim is made. Manual Neo unlock is the remaining prerequisite
  for that campaign, not an M4 service or product repair.
- **FM-R037 INSTALLED CRITERIA SELECTION/INSPECTOR / ACCEPTANCE BEFORE EDIT:**
  The canonical board requires Criteria results to remain ordinary virtual
  folder objects with Selection visible. Extend the opt-in installed scenario
  to select one actual catalogue ObjectView item through the production
  selection model, drain the normal selection/preview handoff, and require the
  retained PropertyList to expose that exact item's factual Name, Kind, and
  Location rows. This proves Criteria did not substitute an isolated result
  surface or disconnect the inspector.
- **FM-R037 INSTALLED CRITERIA SELECTION/INSPECTOR / PASSED:** After valid-date
  recovery, the opt-in Application scenario selected the first actual catalogue
  directory through ObjectView's production selection model. The retained
  PropertyList populated the exact result name, Kind `Folder`, and its exact
  path below the contained `fmsandbox` root. The complete interaction executable
  passed. This proves the canonical Criteria requirement that results remain
  ordinary selectable/inspectable objects with Selection visible; native
  pointer focus and visual comparison remain pending.
- **FM-R038 CURRENT RESUME CHECKPOINT / SERVICES READY, CANDIDATE UNLAUNCHED:**
  Final read-only inspection reverified the exact staged bundle deeply and
  strictly; its executable remains SHA-256
  `830d8f1e3d44d8357366c6b4589dfcd8dde6ed74ba070861dadea1c3b00f9ae2`.
  No process is running from that exact executable. The contained Engine is
  running as PID 23650 and the repaired Orchestrator as PID 27743. The newest
  hermetic frontend suite passed 10/10 in 1.57 s, while the opt-in installed
  Application campaign passes mode entry, real generation-2 results, Kind
  Files→Folders editing, Modified enable/disable, remove/add, invalid-date
  clearing, valid-date recovery, and ordinary Selection/PropertyList
  inspection. Resume after manually unlocking the Neo: keep the live 5901
  relay, inspect Screen Sharing, open a clean Terminal inside the M4 Aqua
  desktop, and launch only
  `fmrepair-criteria-final-20260813.app` with read-only root `fmsandbox` and
  Engine ID `fm1-contained`. Confirm exact PID/path/hash, then exercise those
  same controls by pointer and keyboard and compare against
  `frontend-001-criteria.jpg`. Do not launch through SSH, do not open Chrome on
  the Neo, and do not make any Git write.
- **FM-R039 RESUMED NATIVE GATE / SAME CONTROLLING-NEO LOCK DENIAL:** On the
  next goal continuation, read-only checks reconfirmed the exact signed
  candidate hash, no exact candidate process, Engine PID 23650, Orchestrator
  PID 27743, M4 `WindowServer`/console/loginwindow/screensharingd, and an
  established Screen Sharing connection over the IPv6 5901 relay. Computer Use
  nevertheless returned the identical automatic-unlock denial before exposing
  a framebuffer. This remains a controlling-Neo automation gate, not an M4 or
  File Manager failure. No SSH GUI launch was attempted. Continue source and
  headless installed-machine work; retry native Aqua only after the Neo is
  manually unlocked.
- **FM-R040 DAILY M4 LAUNCHER / DEFECT AND ACCEPTANCE BEFORE EDIT:** The
  checked-in `frontend/tools/launch_m4_daily.command` and protected companion
  both blindly open `$HOME/Developer/CodexRuns/fmnew.app`. The verified current
  candidate is `fmrepair-criteria-final-20260813.app`, so those launchers can
  silently start an older or unrelated bundle and provide no executable-path
  or hash evidence. The protected companion also silently adds
  `--allow-mutations` and a quarantine, contradicting the documented
  read-only-default contract for ordinary contained-root/Criteria dogfood.
  Repair the launch contract without hard-coding an ephemeral repair name:
  atomically select a deliberate versioned dogfood bundle in a small manifest,
  refuse a missing/non-bundle/hash-mismatched/invalid-signature target with a
  visible Terminal diagnostic, preserve daily no-argument Home startup, make
  protected `fmsandbox`/`fm1-contained` startup read-only, and put mutation
  flags behind a separately and unmistakably named launcher. Automated shell
  checks must prove selection, argument construction, hash refusal, and mode
  separation; actual Aqua launch and PID/path/hash proof remain Screen Sharing
  work and must never be substituted with an SSH GUI launch.
- **FM-R040 FIRST LAUNCHER TEST / INVALID SYNTHETIC APP FIXTURE:** Shell syntax
  checks passed, but the focused launcher test stopped before exercising the
  launch contract because its temporary `.app` contained an executable without
  an `Info.plist`; macOS `codesign` correctly reported “bundle format
  unrecognized, invalid, or unsuitable.” No product or staged bundle was
  touched. Give the disposable test app the minimal bundle identifier,
  executable, name, and package-type metadata, sign it again, and rerun the
  unchanged mode/hash/refusal assertions.
- **FM-R041 INSTALLED DAILY NAVIGATION / ACCEPTANCE BEFORE EDIT:** Existing
  hermetic interaction coverage uses a disposable launch root and proves the
  retained navigation mechanics, but it does not instantiate the production
  Application over the M4 user's actual Home or exercise the actual `/Volumes`
  root. Add an explicit opt-in, read-only, service-independent scenario that
  requires caller-supplied Home and descendant paths. It must enumerate Home,
  commit the real descendant through the retained inline exact-path editor,
  prove Back and Forward history, invoke the real Home command, switch the
  folder-tree mode to `/Volumes`, activate that real root, and return Home.
  Assert canonical locations/root labels and globally unique semantic IDs;
  never print directory contents or request mutation authority. Run it on the
  M4 with a known existing descendant below Home. Together with FM-R040's
  zero-argument daily launcher proof, this closes the headless portion of
  ordinary real-root navigation while native Aqua pixels/input remain pending.
- **FM-R041 FIRST INSTALLED DAILY RUN / UNLAID-OUT VIRTUAL ACTUATOR:** The
  warnings-as-errors M4 build passed and the ordinary hermetic interaction
  executable passed, but the opt-in daily scenario stopped at its first
  breadcrumb edit action. The new scenario had never called `perform_layout()`
  before asking Window to resolve the breadcrumb's virtual edit actuator;
  existing production interaction coverage lays out the retained window before
  geometry-backed virtual actions. This run therefore reached no path commit
  and is not evidence of a Home-navigation failure. Establish initial layout
  after Home settles, then rerun the same direct-path/history/Volumes
  assertions.
- **FM-R040 FOCUSED LAUNCHER CONTRACT / PASSED LOCALLY:** After adding a valid
  minimal `Info.plist` to the disposable app fixture, all shell syntax checks
  passed and the focused launcher test passed. It proved zero arguments for
  daily Home startup, exactly the contained root and Engine ID for protected
  read-only startup, mutation/quarantine flags only in the separately named
  mutation mode, and hard refusal of a mismatched executable hash or missing
  manifest. The fixture is deleted after the test. Full M4 CTest integration,
  stable candidate staging, and Aqua launch remain pending.
- **FM-R041 INSTALLED M4 DAILY NAVIGATION / PASSED HEADLESS:** The repaired
  warnings-as-errors interaction target rebuilt on the M4. With explicit
  environment admission matching the process's actual Home, the production
  Application settled at real Home, committed the real
  `$HOME/Developer/CodexBuilds` descendant through the retained exact-path
  editor, traversed Back and Forward, returned through the shared Home command,
  switched the single-root tree instrument to real `/Volumes`, selected that
  actual root, and returned Home. It retained unique semantic IDs and never
  requested mutation authority or printed folder contents. The full ordinary
  hermetic interaction executable also passed separately. Native pointer and
  pixel verification remains pending behind the Screen Sharing gate.
- **FM-R040/FM-R041 FULL M4 FRONTEND GATE / PASSED:** A fresh authoritative
  sync reconfigured the Release frontend against the named installed GUI.Forms
  package, rebuilt the app and every test target, and passed 11/11 CTests in
  1.57 seconds. The suite now includes the dogfood launcher contract as test 11;
  the complete application interaction campaign passed in 1.34 seconds. This
  is automated source/build proof only. Next stage the exact green app through
  the hash-pinned current-candidate contract, dry-run the installed launchers,
  and defer the actual GUI launch to the M4 Aqua Terminal in Screen Sharing.
- **FM-R040 VERSIONED DOGFOOD PROMOTION / PASSED:** The staging helper copied
  the exact 11/11-green build-tree app into the non-repository M4 run area as
  `file-manager-dogfood-20260813T114328Z-371560781c7d.app`, re-signed both
  embedded GUI.Forms dylibs and the app envelope, passed deep strict signature
  verification, and atomically published a current-candidate manifest whose
  executable SHA-256 is
  `371560781c7d9ec382ebc64332d774b6010f54463772f4727f2cee943a0d272f`.
  It also installed separately named daily, protected-read-only, and mutation-
  sandbox `.command` files. No GUI launch occurred. Installed-launcher dry runs,
  independent identity checks, and native Aqua use remain next.
- **FM-R040 FIRST INDEPENDENT REMOTE CHECK / QUOTING FALSE START:** The combined
  read-only SSH check stopped on line 6 before signature, launcher, or process
  inspection because an over-escaped `awk $1` was interpreted by `set -u` as
  the remote shell's unset positional parameter. It did not alter or launch the
  candidate. Replace that extraction with `cut -d " " -f 1` and rerun the
  identical independent checks.
- **FM-R040 INSTALLED LAUNCH CONTRACT / INDEPENDENT M4 PASS:** The corrected
  read-only SSH inspection independently matched the manifest and staged
  executable hash, passed deep strict signature verification, and identified
  the executable as native arm64. Installed dry runs reported: daily zero
  arguments; protected exactly `--root <CodexRuns/fmsandbox>
  --engine-root-id fm1-contained`; mutation exactly those four arguments plus
  `--allow-mutations --quarantine <CodexRuns/fmquarantine>`. The exact current
  candidate process was absent. Thus the stable entry point can no longer
  silently choose `fmnew.app`, and routine protected dogfood no longer silently
  enables mutation. Actual launch remains Aqua-only.
- **FM-R042 CURRENT-CANDIDATE SCREEN SHARING RETRY / SAME NEO DENIAL:** After
  staging and independently verifying the new current candidate, Computer Use
  again returned “The Mac is locked and automatic unlock could not unlock it”
  before exposing the `Screen Sharing` framebuffer. This is the same
  controlling-Neo automation denial documented in FM-R030/R036/R039, not M4
  lock or candidate evidence. No GUI was launched through SSH. Continue the
  source/test prototype-fidelity repair and retry the installed daily/protected
  `.command` files only from the M4 Aqua Terminal after manual Neo unlock.
- **FM-R043 BREADCRUMB CHEVRON PAINT / CONCRETE DEFECT BEFORE EDIT:** The
  retained BreadcrumbTrail already computes eight-pixel overlapping faces and
  emits two shared-edge chevron lines, but `on_paint` fills and edges each item
  in one loop. Because the following face begins inside the preceding face's
  overlap, its later rectangular fill paints over the just-drawn chevron. The
  existing test merely observes that some line commands were issued and cannot
  detect this ordering failure. Split paint into deterministic passes: paint
  every face/text first, then paint all shared chevron joints and focus rings
  above every face. Extend the recording test to require that no face fill
  occurs after the first chevron line and that the expected two-line joints
  exist between every visible item. Preserve geometry, hit testing, overflow,
  editor ownership, color states, and public API. Rebuild and run GUI.Forms plus
  frontend interaction gates on the M4 before staging another candidate;
  framebuffer comparison remains separately required.
- **FM-R043 FIRST FOCUSED PAINT TEST / PANEL-BORDER LINES MISCLASSIFIED:** The
  repaired GUI.Forms control and strengthened test compiled on the M4, but the
  focused collection test failed because the recorder's first line commands
  belong to `Panel::on_paint`'s sunken outer border, before any breadcrumb face.
  The assertion incorrectly treated the first line in the entire control as a
  chevron and then objected to the legitimate face fills. This is a test-
  instrument classification error, not proof that the two-pass paint failed.
  Locate the final face fill, require exactly two line commands per visible
  shared joint after that point, and require no later face fill; rerun unchanged
  production paint.
- **FM-R043 FOCUSED CHEVRON PAINT CONTRACT / PASSED ON M4:** With the recorder
  correctly separating the Panel border from BreadcrumbTrail foreground, the
  focused warnings-as-errors collection target rebuilt and passed. The test now
  proves every visible adjacent item owns exactly two joint-line commands after
  the final overlapping face fill, with no face fill after those joints. Thus
  later segment faces cannot erase the continuous chevrons. Full GUI.Forms
  suite, installed-package rebuild, frontend suite, fresh staging, and native
  comparison remain pending.
- **FM-R043 FULL CROSS-COMPONENT M4 GATE / PASSED:** The M4 rebuilt all
  GUI.Forms targets and passed 62/62 tests in 13.42 seconds, including the
  strengthened BreadcrumbTrail paint-order regression and native host suites.
  That exact GUI.Forms package was reinstalled, File Manager was reconfigured
  and rebuilt against it, and all 11 frontend tests passed in 3.38 seconds; the
  complete application interaction campaign took 1.68 seconds and the installed
  launcher contract 0.20 seconds. Stage a fresh versioned dogfood candidate
  from this exact build next. Native framebuffer comparison is still required
  before claiming the visible chevron mismatch closed.
- **FM-R043 POST-REPAIR CURRENT CANDIDATE / STAGED AND VERIFIED:** The exact
  62/62 + 11/11 green build was staged without overwriting its predecessor as
  `file-manager-dogfood-20260813T114736Z-59e6b9fd209a.app`. Its atomically
  published manifest pins executable SHA-256
  `59e6b9fd209a5904a07d5ea061f478f9b83dcf1ce918c82a44ef73f328f0e43f`.
  Independent inspection matched that hash, passed deep strict signature
  verification, identified native arm64, dry-ran daily with zero arguments and
  protected with only the contained root/Engine ID, and found no exact
  candidate process. The old candidate remains preserved. Launch and visible
  board comparison remain Aqua Screen Sharing work.
- **FM-R044 TOP-LEVEL `Home` AUDIT / NEGATIVE RESULT, KEEP IT:** A review of the
  rendered folder board alone made the top row look like the conventional
  `File · Edit · View · Go · Commands · Help` sequence, raising concern that
  production's extra `Home` menu might be another invented navigation concept.
  Direct inspection of the authoritative HTML at the folder surface instead
  shows `File · Home · Edit · View · Go · Commands · Help`, with `Home` marked
  as the active command-category tab and the Move/copy shelf directly beneath
  it. Therefore production `fm.menu.home` is intentional prototype vocabulary,
  not Places/Recent-style navigation, and must remain. No source control was
  removed. Future audits must distinguish this command category from the real
  Home folder command under Go and from the Home-rooted tree mode.
- **FM-R045 SHELF VIEW LABEL / DEFECT AND ACCEPTANCE BEFORE EDIT:** The
  authoritative HTML and folder board label the permanent Arrange command
  simply `View`, while production replaces it with `View: details` and later
  `View: small icons`. This is not only a visual divergence: ObjectView is
  initially created in icons mode while the shelf is initially hard-coded to
  `View: details`, so the visible label can contradict the actual presentation
  until an Orchestrator setting is applied. Keep the permanent prototype label
  `View`; expose the actual mode through the existing checked Small icons /
  Details popup commands and a current-mode accessible description. Focused
  interaction must prove label stability, checked-state transition, accessible
  current-mode truth, and unchanged action/overflow authority at ordinary and
  narrow widths. Rebuild on M4; native shelf comparison remains pending.
- **FM-R045 PRE-BUILD TEST IDENTITY CHECK / POPUP ID REJECTED:** The first new
  Details-transition assertion targeted `fm.shelf.view`, which is the retained
  ContextMenu model ID, not the DropDownButton. No build or run occurred with
  that mistake. Use the actual authored button stable ID already obtained from
  `view_button`; this preserves the test's purpose of exercising the real
  permanent control rather than an invented alias.
- **FM-R045 FIRST FOCUSED M4 RUN / INITIAL MODE DESCRIPTION ABSENT:** The
  warnings-as-errors target rebuilt, then the interaction test failed at the
  new initial truth assertion. The visible permanent label was correctly
  `View`, but its accessible description did not yet report `Small icons`:
  production assigned that description only inside `set_view_mode()`, while
  initial ObjectView construction starts in icons mode without calling that
  transition. Initialize the description beside the stable `View` label, keep
  `set_view_mode()` responsible for later transitions, and rerun the unchanged
  popup/state assertions.
- **FM-R045 SECOND FOCUSED M4 RUN / STATE RECOMPUTE OVERWRITE FOUND:** The
  rebuilt test still failed the initial description assertion. Source tracing
  found the actual override: `update_command_state()` replaces the initialized
  mode-aware description with generic “Choose the visible folder object
  presentation” whenever the folder surface is active. Make that authoritative
  recomputation report `Current presentation: Small icons` or `Details`, while
  retaining its existing unavailable reasons for correspondence and Settings;
  rerun the same interaction test.
- **FM-R045 THIRD FOCUSED M4 RUN / LIVE DEFAULT-VIEW ORACLE TOO RIGID:** The
  mode-aware state recomputation compiled, but the same assertion still failed
  because it required `Small icons` unconditionally. Product Application
  negotiates the installed settings service during startup; by the assertion,
  its configured default may legitimately have changed ObjectView to Details.
  The product invariant is that the stable visible label is `View` and the
  accessible description names the ObjectView's actual current mode. Derive the
  expected phrase from `objects->view_mode()`, then keep the explicit Small
  icons and Details transitions as deterministic follow-up assertions.
- **FM-R045 DIAGNOSTIC RUN / ROOT CAUSE IS SPLIT INITIAL MODE:** The diagnostic
  rerun reported `text="View"`, description `Current presentation: Details`,
  and actual ObjectView mode `Small icons`. Source inspection confirms
  `details_mode_` defaults to `true` in `application.hpp` while
  `install_dynamic_controls()` explicitly initializes ObjectView to icons.
  Before settings reconciliation, that contradictory cache drives the shelf
  description and checked View commands. Initialize `details_mode_` to false so
  it matches the retained control, keep every later transition through
  `set_view_mode()`, and require both accessible description and the Icons /
  Details command checks to agree with the actual ObjectView mode.
- **FM-R045 FOCUSED M4 VIEW-TRUTH CAMPAIGN / PASSED:** After aligning the
  `details_mode_` default with the actual initial icons ObjectView, the
  warnings-as-errors Application and interaction target rebuilt and the focused
  test passed in 1.94 seconds. It now proves the permanent shelf label remains
  prototype-exact `View`; the accessible description and Small icons/Details
  radio checks agree with the actual configured presentation; and explicit
  transitions to Small icons and Details update that truth without renaming the
  permanent command. Existing dropdown, overflow, and command-authority checks
  in the same campaign still pass. Full frontend gate, staging, and native
  Office Pearl comparison remain.
- **FM-R045 FULL M4 FRONTEND GATE / PASSED:** The M4 rebuilt the complete
  frontend app and all test targets after the View-mode invariant repair. All
  11 CTests passed in 1.79 seconds, including the full application interaction
  campaign in 1.56 seconds and installed launcher contract in 0.20 seconds.
  GUI.Forms source did not change after its preceding 62/62 gate. Stage a fresh
  versioned current candidate from this exact app, then retain native shelf
  visual/input comparison as the open FM-R009 evidence.
- **FM-R045 CURRENT VIEW-TRUTH CANDIDATE / STAGED AND VERIFIED:** The exact
  11/11 green app was staged without overwriting prior candidates as
  `file-manager-dogfood-20260813T115529Z-55dbd69c0486.app`; the current manifest
  pins executable SHA-256
  `55dbd69c0486b59ae57c356f2bd701d39dfacb3a1b03b3f7d222bbbe6c067b65`.
  Independent inspection matched the hash, passed deep strict signature
  verification, identified native arm64, dry-ran the installed daily launcher
  with zero arguments, and found no exact candidate process. No GUI was launched
  through SSH. Use this identity for the next Aqua Screen Sharing comparison.
- **FM-R046 ACTIVE `Home` COMMAND CATEGORY / DEFECT AND ACCEPTANCE BEFORE
  EDIT:** The authoritative HTML marks top-level `Home` as the selected command
  category above the permanent Move/copy, Delete, View, Sort, and Properties
  shelf. Production's public MenuStrip currently has only a transient
  `active_index_`, meaning “this popup is open”; it clears on popup close and
  cannot represent the persistent shelf category independently. Extend the
  reusable MenuStrip with one optional selected top-level stable ID. Selected
  must paint and publish `SemanticState::selected` while closed; opening File,
  Edit, or another popup must publish `expanded` only on that transient item
  without clearing or falsely expanding the selected Home category. Item model
  replacement must retain selection only when the stable ID still exists and
  remains visible. File Manager binds `fm.menu.home` once after setting items.
  Focused GUI.Forms tests must prove selection, transient expansion, close, and
  model-replacement behavior; frontend tests must prove Home selected with no
  popup and still selected while File is expanded. No shelf membership or
  command authority changes. Full M4 gates and native comparison remain.
- **FM-R046 PRE-BUILD INSPECTION / OVERLAPPING `sed` FALSE ALARM:** Two adjacent
  source slices both printed the same boundary `catch` line, which looked like
  a duplicated exception clause. A cleanup patch correctly refused to apply;
  numbered source inspection shows exactly one valid `catch` at line 410. No
  file changed in that false alarm. Proceed to the focused build with the actual
  source as written.
- **FM-R046 FOCUSED GUI.FORMS MENU CONTRACT / PASSED ON M4:** The public
  MenuStrip and warnings-as-errors menu test rebuilt on the M4 and passed 1/1 in
  0.58 seconds. The test proves a selected category remains selected while
  closed, transient File expansion does not select File or expand the selected
  View category, compatible item replacement retains the selected stable ID,
  removal clears it, and hidden selection is rejected. This is reusable-control
  proof only; install the exact package and run File Manager's Home binding next.
- **FM-R046 COMPACTED BUILD OUTPUT / STATUS REJECTED AS UNPROVEN:** The first
  exact-package install, frontend interaction rebuild, and focused-test command
  outlived the captured tool session; compaction preserved neither a final exit
  code nor a test summary. Do not infer a pass from partial build output. A
  direct focused CTest rerun then failed before execution because
  `frontend/build/file_manager_application_interaction_tests` did not exist,
  proving the lost command had not produced the requested target. This is a
  build/proof failure rather than a product-test failure. Reinstall GUI.Forms,
  reconfigure the frontend against that exact package, rebuild the missing
  target, and rerun the unchanged Home-category interaction assertions.
- **FM-R046 EXACT-PACKAGE FRONTEND LINK / STALE LIBRARY FOUND:** Reinstalling
  without first rebuilding GUI.Forms copied the changed public header but left
  the previously built `libgui_forms_controls.a` in place. Frontend compilation
  therefore accepted `MenuStrip::set_selected_item_id(std::string_view)`, but
  its arm64 link failed with that exact symbol undefined. This is a package
  coherence failure in the proof sequence, not a passing implementation and
  not an interaction-test failure. Rebuild the GUI.Forms libraries from the
  current source, reinstall that coherent header/library pair, then rebuild and
  run the focused frontend interaction target unchanged.
- **FM-R046 COHERENT PACKAGE + FOCUSED FILE MANAGER PROOF / PASSED ON M4:**
  GUI.Forms was rebuilt from current source before installation, producing a
  coherent header/library package. The frontend interaction target then linked
  successfully and passed 1/1 in 2.87 seconds on the M4. The installed
  Application proof now confirms `fm.menu.home` is selected while no menu is
  expanded, opening File expands only File while Home remains selected and not
  expanded, and Escape closes the transient popup without destroying the
  command-category selection. Run the full 62-test GUI.Forms gate and full
  11-test frontend gate before staging a replacement native candidate.
- **FM-R046 FULL M4 REGRESSION GATES / PASSED:** The M4 rebuilt GUI.Forms from
  current source and all 62 GUI.Forms CTests passed. That exact package was
  installed, the complete File Manager frontend and app were rebuilt against
  it, and all 11 frontend CTests passed in 2.82 seconds; the Application
  interaction campaign passed in 1.57 seconds and the dogfood launcher contract
  passed in 0.20 seconds. The persistent Home category repair is therefore
  regression-green in both its reusable control and installed consumer. Stage
  a new versioned candidate from this exact build and retain Aqua visual/input
  comparison as the remaining native evidence.
- **FM-R046 REPLACEMENT NATIVE CANDIDATE / STAGED + IDENTITY VERIFIED:** The
  exact 62/62 + 11/11 green app was staged without overwriting earlier builds as
  `file-manager-dogfood-20260813T120415Z-8a3a4ee0738a.app`. The manifest pins
  executable SHA-256
  `8a3a4ee0738a81b8b461b3a08c82182fd9c29208ed00d59909d0e0f74ae05ea9`;
  an independent read matched that hash, deep strict signature verification
  passed, and the executable is native arm64. Daily dry-run resolved this exact
  bundle with zero product arguments; protected read-only dry-run resolved it
  with the bounded root arguments. The final prelaunch absence guard found the
  exact candidate process already running on the M4. Do not kill, duplicate, or
  relaunch it blindly: inspect its PID/parent/start/command and use the existing
  Aqua instance for Screen Sharing comparison if it is the correct GUI process.
- **FM-R046 VERIFICATION OPERATOR ERROR / WRONG DRY-RUN VARIABLE CAUSED SSH
  LAUNCH:** Independent verification mistakenly set
  `FILE_MANAGER_DOGFOOD_DRY_RUN=1`; the checked-in launcher contract actually
  uses `FILE_MANAGER_LAUNCH_DRY_RUN=1`. The unrecognized variable correctly had
  no effect, so each wrapper fell through to `open -n`; the daily invocation
  launched the exact staged File Manager app through SSH at 08:04:26, contrary
  to the required Screen Sharing launch sequence. PID 47034 is the exact
  hash-pinned executable with launchd as parent. Do not count the preceding
  wrapper output as dry-run argument proof, do not pretend the launch route was
  valid, and do not duplicate or kill the correct app blindly. Rerun both
  wrappers with the actual `FILE_MANAGER_LAUNCH_DRY_RUN=1` contract, record
  their argument counts, then inspect the already-running app only through the
  M4 Screen Sharing session. The launcher implementation itself is unchanged.
- **FM-R046 CORRECTED LAUNCHER CONTRACT PROOF / PASSED:** With the actual
  `FILE_MANAGER_LAUNCH_DRY_RUN=1` variable, the installed daily wrapper verified
  the exact staged hash and reported `argument_count=0`; the protected read-only
  wrapper verified the same app and reported exactly four arguments:
  `--root $HOME/Developer/CodexRuns/fmsandbox --engine-root-id fm1-contained`,
  with no mutation flag. Exact PID inspection remained only 47034, so the
  corrected proof created no duplicate process. Native visual/input evidence
  still must come from Screen Sharing; the accidental SSH launch itself is not
  acceptable launch-route proof.
- **FM-R046 SCREEN SHARING NATIVE PROOF / BLOCKED BY NEO COMPUTER-USE GATE:**
  The required Computer Use inspection of the already-running M4 candidate
  returned “The Mac is locked and automatic unlock could not unlock it.” Prior
  direct evidence establishes that this gate belongs to the controlling Neo,
  not the M4 desktop visible through Screen Sharing. No screenshot or native
  interaction was obtained, and the accidental SSH launch is not substituted
  for that missing evidence. Keep FM-R009/native comparison open. Continue
  read-only prototype/source auditing and automated implementation work; retry
  Screen Sharing only after the Neo Computer Use gate is manually unlocked.
- **FM-R047 ADVANCED MASK/BLEND AUDIT / NOT REQUIRED:** The accepted File
  Manager HTML/CSS and the generated GUI.Forms material graph require ordered
  clipped solid/linear/radial/repeating fills, borders, keylines and
  inset/outset shadows, but no arbitrary mask, blend mode, isolated group,
  backdrop filter or color matrix. The pinned fidelity oracle reports zero
  material-mismatch nodes. Do not add speculative compositing vocabulary; a
  future accepted specimen must supply a bounded source grammar, renderer-
  neutral semantics and cross-renderer evidence before that decision reopens.
- **FM-R048 COMMAND OVERFLOW / GENERATED RETAINED CONTRACT:** GUI.Forms now
  owns explicit whole-group priority collapse and publishes a stable-ID
  `CommandOverflowSnapshot`; Web.Forms emits the panel, group extents,
  priorities, actuator and real drop-down controls. Frontend removed its
  duplicate shelf projection and only composes the authorized live overflow
  menu from collapsed group IDs. Focus protection and atomic invalid metadata
  are covered in focused tests; full M4 and MinGW gates are green.
- **FM-R049 PHYSICAL SEAM INTERPOLATION / ACCEPTED GAP CLOSED:** The collapse
  actuator now interpolates quiet `7 x 28`, near `9 x 34` and engaged `12 x
  42` geometry over an authored 90 ms ease-out transition. Seam and hit bounds
  remain fixed; only an in-flight transition owns a frame lease; reduced motion
  and zero duration complete immediately. Web.Forms projects duration with the
  pane geometry. Focused half-duration/completion/reduced-motion tests and the
  complete 86-test GUI.Forms suite pass.
- **FM-R050 STAGE 3 STATIC TOPOLOGY / POST-GENERATION REBUILD REMOVED:** The
  authored source now contains the outer responsive tracks, three retained
  split containers, fixed/collapse/min/max/hit/transition policy, tree header
  slots, command presentation and title drag region. Frontend aliases the two
  generated workspace splits and no longer clears/rebuilds the workspace or
  inspector composition. Remaining replacement calls target explicit live
  hosts for menu, breadcrumb, search, tree, object view, preview, property list
  and settings/services content; those remain application state, not a visual
  compiler gap. M4 frontend tests pass after the topology migration. Windows
  native visual review remains separate from the green full MinGW build.
- **FM-R051 STAGE 3 SOURCE CLOSURE / CONNECTED STOCK, KEYLINES, AND LOCAL ART:**
  Web.Forms now validates and lowers explicit connected button topology and
  ordered per-edge keylines. Its new PNG-only local-resource lane bounds source
  containment, logical/intrinsic dimensions, density, count and aggregate
  bytes; image bytes participate in the source digest; generated code embeds
  and registers the 1x/2x variants and attaches the retained `ImageList`.
  File Manager's eight permanent shelf/navigation icons now come from this
  source lane, and application C++ no longer installs their image list. Dynamic
  object/tree/preview art remains product model work. The navigation group is
  source-authored as one connected three-button stock. Web.Forms passes 37/37;
  GUI.Forms 86/86; frontend 11/11; complete MinGW/Skia builds and the generated
  File Manager tree compiles under MinGW warnings-as-errors.
- **FM-R052 FIDELITY MATRIX / MEASURED, NOT LAUNDERED:** The stable-ID oracle
  now compares source digest, parent/kind/topology/resource identity, bounds,
  clips, state, resolved text/baselines, semantic materials, keylines, and
  eight actual RGBA probes per visible node. Eleven M4 profiles cover ordinary,
  720-wide, 150x150, 125/150/200-percent text, inactive, hover, pressed,
  focused and disabled. All profiles preserve exact source identity and
  structure. The ordinary profile has zero state/material mismatches but 69
  strict geometry, 23 typography and 32 raster mismatch nodes. At 150x150 the
  native projection collapses both command groups and exposes the overflow
  actuator; raw HTML accepts the viewport but does not execute the retained
  priority solver, producing 31 state mismatches and five material mismatches
  at clipped descendants. This is the remaining Stage 3 Web.Forms
  preview-adapter question, not evidence for masks/blending or another
  GUI.Forms primitive.
- **FM-R053 GENERATED RESOURCE BIND ORDER / REGRESSION CAUGHT AND CLOSED:** A
  final cleanup moved publication of `Application::window_` after generated
  `ImageList` attachment. The full interaction campaign then segfaulted
  consistently because attachment invalidation can synchronously enter already
  installed application callbacks. Restoring window publication before
  `bind_native_resources` made the focused campaign pass three consecutive
  runs, after which the complete 11-test frontend gate passed. The ordering is
  documented beside the call; no resource or control ownership was weakened.
