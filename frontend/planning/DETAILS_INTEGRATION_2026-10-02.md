# Details integration development receipt

Status: matching local consumer validation passed (12/12); source development.
Native acceptance and a downloadable application containing these changes
remain pending. The earlier preview release does not include this work.

## Implemented consumer policy

**CANDIDATE defaults:** Name, Type, Size, Date modified. The interviews require
factual sortable headers but do not settle their initial order. Widths are
260/120/104/172 logical units with bounded resizing, retained across directory
changes during the session. Column selection, persistent widths/order and
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

Pending: native macOS/Linux metadata branches,
native header pixel/synthetic-route evidence, matching package verification,
physical input/scrolling feel and accessibility refinement. A Mac harness now
preserves a generated-fixture Details screenshot for visual review; snapshot
rendering can force display and does not measure compositor latency.
