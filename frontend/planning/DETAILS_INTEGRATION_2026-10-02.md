# Details integration development receipt

Status: **published portable dogfood checkpoint**
[`v0.001-alpha.6e74434`](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.6e74434).
Both complete native workflows 36989014139 and 36989019470 pass on Windows,
macOS and Linux. Source `6e74434792081df124efd374cb0679ddaa380515` was rebase-
merged by PR #6 as `47e24cd70109075ee327abcdeaa50a5bdba1cc30`; both have tree
`aba13e40b6a8a4fc10a1d065f39732a905792b25`. Independently downloaded archives
passed source, sidecar and 40/42/44 Linux/Mac/Windows file-hash checks; uploaded
archive digests match. The final Mac screenshot visibly shows all three fixture
rows and all four headers. Earlier pending entries below retain stage history;
physical-input smoothness, accessibility and full daily-use acceptance stay open.

## Implemented consumer policy

**CANDIDATE defaults:** Name, Type, Size, Date modified. The interviews require
factual sortable headers but do not settle their initial order. Preferred widths
are 260/120/104/172 logical units, interpolated toward 120/90/72/132 minima to
fit the current pane. Extra width goes to Name. Text-scale changes refit logical
widths; panes below the minimum total retain horizontal access. A manual resize
stops automatic fitting for the session, preserving user widths across later
resizes and directory changes. Column selection, persistent widths/order and
native table/header accessibility remain separate unfinished work.

The same retained model serves local directories and exact Criteria results;
the hidden object projection behind Correspondence uses the same facts.
Name is the observed pathname leaf, Type follows the no-follow native object
classification, Size is regular-file logical bytes, and Date modified is the
signed observed timestamp. A present zero differs from absent metadata.
Directories and links have no synthetic content-size value. Indexed allocated
folder aggregates remain unimplemented. Engine generation/provenance remains
separate from the current observed file fields; stale catalogue type, size or
name hints cannot overwrite them.

Folders remain first. Available typed size/time/type values precede unavailable
values in both directions. Modification ordering compares signed seconds then
fraction, independently of formatted calendar text and legacy revision bits.
Type retains the existing EntryKind group order; this is not an extension-based
file-association taxonomy. Equal non-name values use the existing folded name
fallback. Name sort reverses within the folder/nonfolder groups.

## Input, ownership and failure

F6 enters/leaves header navigation; Enter/Space requests sort. The application
Open accelerator runs after the focused control so header Enter does not open
the selected object. Body activation still executes the canonical Open command
through `fm.objects.activation`. Alt+Left/Right retains history; Alt+Shift+Left/
Right adjusts a focused column or pans the body. Wheel/pointer resizing remain
available. These bindings have renderer-neutral tests, not physical-input
latency evidence.

Each incoming snapshot now builds cells, sorts once and publishes one model.
The previous install-then-copy-and-rebuild sequence was removed. Cell storage
reserves four elements and moves owned display values directly; no initializer-
list string copies remain in that repeated row path. This is source evidence,
not a measured end-to-end speed improvement.

The explicit-sort provider overload validates and commits row order and its
accepted indicator together before notification. Application sort intent is
swapped into owned storage before publication; precommit refusal restores it
without allocation, while a postcommit callback exception keeps the accepted
new intent. Existing directory snapshot publication as a whole is not claimed
to be an exception-atomic filesystem transaction.

## Rejected intermediate results

The assembled application caught a valid header Enter being discarded: the
provider captured its cache revision before calling `absolute_bounds`, which
can flush pending layout and change that revision. It now resolves layout
before reading the accepted sort and snapshotting context. Post-release context
validation also checks revision after any geometry-triggered flush. The new
pending-layout control regression passes; existing release/disposal/reentrant
sort suppression remains tested.

The first size assertion assumed five bytes for a text-mode `root\n` fixture.
Windows CRLF storage made that assumption wrong. The test now independently
reads the generated fixture's actual byte length. Production size decoding was
not changed to satisfy the incorrect expectation.

Review found and corrected missing factual metadata in Criteria/search,
unavailable Type sorting ahead of known types in descending order, per-row
initializer-list copies and separate model/indicator publication. The former
Open trace expectation was updated from the preemptive accelerator source to
the actual focused-object activation source; it still requires the same
canonical command and successful folder navigation.

## Reviewed scope and pending evidence

Source review against the complete `planning/PROGRAMMING_HOUSE_STYLE.md` covers
`details_projection.hpp`, changed `ObjectOrder` comparisons, application sort/
publication handlers, observed search-entry construction, their three call
sites, named subscription and Enter ordering, plus added ordering/interaction/
macOS tests. The metadata adapter's precise review and native coverage gaps are
in `DETAILS_METADATA_DEVELOPMENT_001.md`; provider replacement/fault scope is
in `../../gui_forms/docs/OBJECT_VIEW_DETAILS_DEVELOPMENT_001.md`.

Inputs and stored types are explicit; owned event identities and row values
outlive synchronous callbacks; no column borrow crosses model replacement;
preparation and publication are separate. Native resource ownership remains in
the observation adapter. No generated-profile restrictions are applied to
ordinary implementation containers. Untouched application commands, complete
directory publication rollback, general worker scheduling and legacy control
paths are outside this compliance claim.

**MEASURED:** the independently configured consumer at `.build/details-consumer`
passes all 12 frontend tests in 3.60 seconds against the newly installed
`.build/details-sdk`, on Shadow Windows GNU C++ 16.2.0 Release. The original
frozen shadow-sdk was not replaced. Header input covers every column, both
directions for Size/Date modified, preserved selected identity and width,
body activation, precommit refusal and a throwing postcommit selection observer.
Criteria fixtures deliberately supply stale catalogue name/type/size hints.
Provider tests include 409 allocation-failure states with a coherent indicator.

The GUI consumer initially exposed a Windows header-macro collision when it
included the native handle adapter. `NativeObjectObservation` and its portable
function declaration now live in private `native_observation.hpp`; GUI code
does not import the adapter's Windows headers. This new value-only header and
the include changes are part of the reviewed source scope.

**MEASURED:** source `17a748f` passed both three-platform native workflows
36985684151 and 36985688166, including macOS/Linux metadata branches. The Mac
header probe found 24 dark Name-header pixels and passed synthetic sort with
selection retained; the earlier TXT/PNG/unsupported preview pixel checks also
passed. These are source-specific results, not acceptance of subsequent edits.

**REJECTED visual result:** the saved 1024x674 Mac screenshot showed only the
last of three fixture rows after sorting and clipped almost all of Date
modified. Model/pixel checks had not caught either ergonomic defect. The new
viewport regression clamps replacement, mode change, explicit scrolling and
viewport growth to the last full page, preserving the former top identity
only where that does not leave avoidable empty rows. A partial final row stays
reachable in full. Automatic column fitting above addresses the clipped header.
The Mac harness now requires all three rows to remain at top zero and all four
column widths to fit before preserving the screenshot.

House-style review additionally covers the bounded four-column fitting loop,
named layout/presentation callbacks, their owned width history, the nonallocating
top-offset clamp, and focused viewport/native regressions. Committed geometry
avoids a layout flush inside the fitting callback; borrowed columns survive the
width setters, which publish no callbacks and do not replace the model. No
general layout/scrollbar or untouched legacy-control compliance claim is made.

Source review also found that damage allocation can throw after a width setter
commits. The fitting catch records the four actually committed widths without
allocation before rethrowing; a later attempt therefore does not confuse an
interrupted automatic update with manual ownership. This recovery has source
review, not a frontend allocation-injection result. Text-scale changes and
manual-width preservation across both resize and scale have interaction tests.

**MEASURED local follow-up:** the final matching Windows consumer passes 12/12
tests in 2.96 seconds. Independent sibling source review found no remaining
actionable issue in the added callbacks/state, fitting recovery, row-count and
clamp helpers or focused regressions. This is bounded source/local evidence;
it does not replace the pending native screenshot and package checks.

Pending: native acceptance of these visual corrections, matching package
verification, physical input/scrolling feel and accessibility refinement.
Snapshot rendering can force display and does not measure compositor latency.
