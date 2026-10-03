# ORC-GUI-001 — ObjectView Details development stage 001

Date: 2026-10-02. Revision: `object-details-draft-1`.

Status: **reconciled semantic draft; named provider changes reviewed,
independent consumer and native accessibility acceptance pending.** This is not
a stable ABI, matching SDK receipt or advertised runtime capability.

## Authority and scope

**GIVEN:** the owner rejected File Manager's two-region Details representation
and directed real headers, usable metadata, responsiveness and interview
alignment. **OBSERVED:** the source audit is
[`../../../frontend/planning/DETAILS_ACCEPTANCE_AUDIT_2026-10-02.md`](../../../frontend/planning/DETAILS_ACCEPTANCE_AUDIT_2026-10-02.md).
It recovered sortable factual Details intent, but no approved default column
order. The provider proposal is
[`../../../gui_forms/docs/OBJECT_VIEW_DETAILS_DEVELOPMENT_001.md`](../../../gui_forms/docs/OBJECT_VIEW_DETAILS_DEVELOPMENT_001.md).

GUI.Forms owns retained collection mechanics, geometry, input and semantic
projection. File Manager owns filesystem facts, their availability, labels,
comparison policy, result ordering and view preferences. Orchestrator registers
these meanings; it neither draws columns nor receives row data through a new
daemon operation. This addition stays within the existing in-process C++
GUI.Forms consumer edge. No plugin execution, filesystem authority or wire
operation is introduced.

## Required meanings

| Record | Meaning |
|---|---|
| Column identity | Nonempty owned stable ID, independent of position and label |
| Column definition | Owned label; finite logical-unit width and minimum/maximum; left/right alignment; sort eligibility |
| Row | Existing exact consumer identity, enabled state and owned cell facts; identity never derives from visible row number |
| Cell | Column identity, owned UTF-8 display text, and available/unavailable/not-applicable state |
| Accepted sort | Optional column identity and ascending/descending direction, published by consumer |
| Sort request | Column identity and proposed direction; no implicit data mutation or accepted indicator |
| Paint work | Rows/cells visited and text prepared in a named paint; work counts do not assert native latency |

Column and row identities are unique in their own namespaces. A cell may appear
once for each declared column; missing/duplicate/unknown cells are rejected for
the explicit Details model. Unavailable facts are never synthesized as zero,
empty successful content or a different filesystem object's metadata. The
consumer supplies localized unavailable/not-applicable text.

Empty column definitions retain the historical icon/name-plus-secondary
projection, including existing picker consumers. New explicit Details cells
do not change the meaning of legacy item fields. Column labels, order and
meaning remain consumer policy; GUI.Forms contains no file-type or time sort.

## Replacement and lifetime

All mutation and input occurs on the owning UI thread. Replacement receives
owned records, validates a complete model and prepares derived lookup and
selection state before publication. Invalid input or allocation failure before
commit preserves the previous model, selection and viewport. The provider must
state the failure mechanism in its C++ projection. Event delivery/invalidation
after successful publication cannot retroactively promise rollback.

No borrow into caller-owned model storage survives publication. Borrowed public
views expire on model/column replacement or disposal. An event payload owns its
column ID across reentrant replacement. Named subscriptions use the existing
Event/SubscriptionToken ordering and revocation law. Header press/resize state
is retired before dispatching a sort request; disposed owners do not receive
later callbacks.

The reconciled explicit-sort replacement overload accepts incoming columns,
rows and their accepted sort together. Validate the sort identity/direction
against incoming sortable columns before commit, then publish order and
indicator before callbacks. The existing two-argument replacement retains a
surviving accepted sort. Neither form performs domain sorting. Application
intent recovery must distinguish a precommit refusal from a callback exception
after this complete publication; it must not restore an old indicator onto a
new row order. The revised bounded allocation campaign records 409 failures
with complete old/new states, including this overload; native acceptance for
the revised source remains separate from the earlier SDK checkpoint.

Replacement preserves surviving selected identities, primary, independent
focus, range anchor and top-visible identity. If primary disappears, choose the
first surviving selected item; if none survive, clear primary. Removed focus
clears. Removed range anchor falls back to the surviving primary. A removed
top-visible identity clamps the previous numeric top row. Preserving an identity
does not leave avoidable blank rows at the end: replacement, view-mode changes,
viewport growth and explicit scrolling clamp to the last full page. A partial
bottom row remains reachable in full. View-mode changes map the top object to
its containing icon row before that clamp. These are transient toolkit
rules, not permission to persist per-folder state.

## Geometry and input

One committed column geometry drives header paint, body paint, clipping and
hit testing. Header remains above the scrolling body. Horizontal offset is
finite and clamped after viewport, width and model changes. Resize cannot
select or activate a filesystem row. Keyboard traversal must reveal every
header even when the total width exceeds the viewport.

The provider proposes: pointer click for sort; right-edge drag for width with
pointer capture; horizontal or Shift+vertical wheel for horizontal movement;
F6 to enter/leave header focus; Left/Right and Home/End for header navigation;
Enter/Space for sort; Alt+Shift+Left/Right for bounded width changes; Escape to
cancel/leave header interaction. Body Alt+Shift+Left/Right proposes horizontal pan.
These bindings are **CANDIDATE consumer integration choices** until checked
against File Manager's existing shortcuts and native keyboard behavior. Their
existence does not establish screen-reader access to columns.

Explicit semantic actions addressing an item (focus, select, press or show-menu)
move internal keyboard focus from the header to that item, matching pointer
entry into the body. Re-selecting the same item must still retire header focus
and update focused semantics; an unchanged selection is not an unchanged focus.
Keys the body does not handle remain available to consumer accelerators.

First sort activation proposes ascending; repeated activation of the accepted
column proposes the opposite direction. Only the consumer can accept the
request by publishing actual order and indicator state. Disabled/non-sortable
headers never issue a request. Clearing accepted sort removes its indicator.

## Resource discipline

The provider proposes development guards of 64 columns, 1,000,000 rows, 65,536
bytes per text field, and widths `40 <= minimum <= width <= maximum <= 4096`.
These are **CANDIDATE guards**, not measured practical capacities or an owner
file-count limit. The provider reply proposes a 64 MiB (67,108,864 byte)
aggregate logical-text ceiling. Count column IDs/labels, item IDs/names/
secondary text/descriptions/image keys, and cell IDs/text before derived
storage allocation. Reject overflow/excess transactionally. This accounting
excludes container overhead and spare capacity and is not a total heap bound.

Peak replacement includes old and incoming models, normalized cells, identity
lookup and retained selection/viewport state. Capacity and container overhead
are distinct from text byte length and must be reported when claiming memory
bounds. Caller-side fixture/model construction is not hidden toolkit storage.

Paint traversal is restricted to visible rows and intersecting columns plus
the bounded header. Repeated paint reuses prepared visible text until an input
that affects it changes. Model replacement may remain linear in the admitted
model; this stage does not claim a streaming million-row provider or constant
replacement cost. A name lookup or selection membership scan over the whole
collection must not occur once per visible cell.

## Independent consumer and acceptance cases

Before File Manager consumes a published SDK, require:

1. Duplicate/unknown/missing IDs/cells, invalid enum/width/UTF-8, aggregate
   limits and replacement allocation failure preserve valid old state.
2. Real header/body alignment under resize, horizontal scroll and scale;
   right-aligned facts; clipping prevents overlap with adjacent columns.
3. Sorting emits one owned request, permits reentrant replacement and revokes
   disposed subscriptions. Indicator follows accepted state only.
4. Width drag/cancel and keyboard header navigation preserve file selection;
   every header remains reachable. Column changes clamp offset.
5. Selection, primary, focus, range anchor and top object survive reorder,
   ordinary replacement and icon/Details transitions where identities survive.
6. Named 1k/100k fixtures show bounded visible-row/cell/text-preparation work,
   including repeated paint, scrolling and a large selected set. Report memory
   and timings separately from counters; preserve failed measurements.
7. Independent frontend tests exercise metadata availability, all column
   requests/directions, retained state and the legacy picker projection.
8. Native macOS, Windows and Linux interaction/semantic evidence and coherent
   provider-consumer builds precede portability or release claims.

The first source slice retains list/list-item semantics with factual row
descriptions. Native table/header/cell relations and accessible sorting/resizing
remain **degraded/unimplemented** and require a coordinated portable semantic
and host-adapter stage. Drawing headers and keyboard access alone do not close
this gap. A column chooser, persisted widths/order and final default columns
are subsequent consumer work.

## Compatibility and reversal

Parent source review covered model validation/commit, selection publication,
geometry/input, visible-text preparation and allocation-failure tests. The
provider receipt records 286 injected exceptions, complete old/new state
checks, and bounded visible work for 1k/100k rows. Review corrections retired
capture before sort publication, moved viewport commit before selection events,
and revalidated interaction state across focus/capture callbacks. These local
results do not establish native latency or accessibility. File Manager already
uses Alt+Left/Right for history. Integration review therefore revised the new
width/pan binding to Alt+Shift+Left/Right; plain Alt chords pass through
the Details control to the application. File Manager's Enter command runs
after the focused control, allowing a header to sort before object activation.

This is a C++ source development projection under ORC-GUI-001. Added item fields
and control state change C++ layout: provider, frontend and picker consumers
must be rebuilt against the same headers/libraries. Never copy a new library
over an older application's SDK or infer compatibility from its filename.
Existing frozen FM0 manifests and Core 1.0 availability remain unchanged.
No new C ABI table entry is frozen here.

The empty-column legacy path is the reversible fallback while the explicit
model is developed. Removing the development API before publication affects
only named source consumers. After a coherent SDK release, migration requires
an explicit source/package receipt and rebuilt consumers.
