# ObjectView Details development seam 001

## Consumer shortcut reconciliation — 2026-10-02

The integration revision uses Alt+Shift+Left/Right for column width and body
horizontal pan. Plain Alt chords pass through the new Details control so
File Manager retains Back/Forward even when a header is focused. Earlier
Alt+Left/Right proposals below are superseded by this revision. File Manager
routes Enter after the focused control, allowing header sort before file open.
The renderer-neutral header regression checks that plain Alt+Right changes
neither width nor horizontal offset and remains unhandled, then checks the
distinct resizing chord. Native keyboard behavior remains to be verified.

Source review of this revision covers the explicit modifier comparison,
early pass-through and named test observations under the complete house style.
No new callback storage or borrowed state is introduced. Collection and
allocation-failure targets pass locally (2/2, 0.44 seconds).

Status: authorized source development, not frozen ABI, SDK availability or native table accessibility acceptance.

## Model and ownership

`ObjectColumnId` names a column independently of its position. A complete Details replacement owns its columns, items and each item's ID-keyed cells. Column IDs and row IDs are unique within their respective namespaces. Cells are normalized to column order during replacement; callers never coordinate positional cell indices. Text is owned UTF-8. Availability is explicitly available, unavailable or not-applicable; nonavailable text is supplied by the caller (including localization), never guessed from an empty string or zero.

Columns are bounded to 64, rows to 1,000,000 and each text field to 65,536 bytes. Widths are finite logical units with `40 <= minimum <= width <= maximum <= 4096`. Alignment is left or right. Toolkit has no file, metadata, numeric comparison or service policy. Column labels/default order come from the consumer. The first column receives the existing item icon; all its text still comes from its explicit cell. Empty columns retain historical icon/details name-plus-secondary rendering and accept historical items without cells.

**Development resource guard:** the sum of all incoming column IDs/labels, item IDs/names/secondary text/descriptions/image keys, and cell IDs/text must not exceed **64 MiB (67,108,864 bytes)**. A nonallocating, checked-addition pass runs before provider identity-index allocation or cell normalization. An excessive replacement throws `std::invalid_argument` and preserves the old model. This is logical UTF-8/string byte admission, not measured capacity, a product limit, or a hard total heap bound: by-value caller storage already exists, vector/map overhead and spare caller capacity are additional, and atomic publication temporarily retains both models. Row/column/per-field caps still apply independently. Nonavailable cells require nonempty caller-authored disclosure text.

**Build compatibility:** this additive C++ source seam changes public class/item layout. Rebuild GUI.Forms and every consuming binary against matching headers and library artifacts. Do not mix these headers or objects with an existing installed SDK. No SDK export or ABI compatibility claim is made by this stage; the historical FM0 artifact is unchanged.

`set_details_model` validates a full replacement and prepares identity/selection state before committing. `set_items` uses the current columns and the same transaction. Invalid input/allocation failure before commit leaves the model and selection unchanged. After commit, invalidation/event delivery may fail according to existing Control/Event rules; the committed model remains valid. Borrowed spans are invalidated by replacement, disposal or subsequent column mutation. There is no retained borrow into caller storage. Peak replacement storage includes old and new models and an identity index. New model publication cancels pending header interaction.

## State and geometry

One shared column geometry drives header/body paint, clipping and hit testing. Header height follows row/text scale. Horizontal offset is logical units; it is clamped on resize/model/width changes. The header stays above vertically scrolling body rows. Width adjustment cannot change file selection or activate an item. Column reorder/chooser is absent in this development slice; caller replacement defines order.

Replacement preserves selected identities, primary, independent focus, range anchor and top-visible identity if they survive. Missing primary falls back to the first remaining selected row; removed focus clears; removed range anchor falls back to primary; removed top anchor clamps the prior numeric top row. View changes preserve the top object by mapping it to the containing icon row. These fallbacks are toolkit mechanics, not per-folder persistence.

## Requests and interaction

Sort request carries an owned column ID and proposed ascending/descending direction. First activation requests ascending; activation of the currently sorted column toggles direction. The consumer owns comparison/order and must separately publish accepted sort state. A request alone never moves rows or changes the indicator. Non-sortable headers do not issue requests. An empty sort column clears the indicator. Header replacement clears an indicator whose column disappears.

Pointer header click requests sort; dragging a header's right edge adjusts its bounded width with pointer capture. Horizontal wheel and Shift+vertical wheel pan. F6 enters/leaves header keyboard focus; Left/Right visits/reveals headers, Home/End selects first/last, Enter/Space requests sort, Alt+Left/Right adjusts width by 8 logical units. Escape returns to the body/cancels active resize. Body Alt+Left/Right pans horizontally. Ordinary body keys preserve existing collection behavior. Header focus is independent of item focus/selection. No filesystem action is implied.

Events use existing synchronous UI-thread Event/SubscriptionToken revocation semantics and named consumer targets. Sort requests own their strings across reentrant model replacement. The control clears pending press before publishing; subscribers may replace the model. No toolkit callback retains a consumer pointer outside the existing subscribed-owner lifetime mechanism. Width state is queryable; persistent storage remains consumer-owned and is not implemented here.

## Work and accessibility

**OBSERVED header-entry reentrancy correction (2026-10-02):** Details pointer delivery retains the control while focus and capture callbacks run. Before using the previously computed header hit, it rechecks lifecycle, attachment, effective availability, focus ownership, Details revision and geometry after focus delivery. Capture acquisition receives the same checks plus capture ownership and the pending resize identity. A changed context retires that press; cleanup releases capture only when this control still owns it. The revision advances conservatively with Details cache invalidation, including same-shape model replacement. Synchronous window lifetime remains the caller's responsibility.

**MEASURED regression coverage:** `test_details_header_entry_reentrancy` exercises header-edge down with focus callbacks that clear or replace the model, dispose or detach the control, or change column width. Capture-publication callbacks repeat those mutations and additionally revoke capture or transfer it back to the prior owner. Follow-up movement cannot resume the retired resize, and cleanup preserves another owner's capture. The renderer-neutral collection-control target passes after this correction. Source review against `planning/PROGRAMMING_HOUSE_STYLE.md` additionally covers the context record/check, retained control lifetime, early-return ordering, conditional capture cleanup, revision update and named regression listener with explicit borrowed state.

Paint visits only visible body rows and horizontally intersecting columns, plus the bounded header. A bounded visible-text cache reuses owned elided strings until model/font/width/viewport changes; cache misses perform grapheme-safe elision. Counters report visited rows/cells and prepared text, not speed. Full model replacement is linear in model size plus bounded columns and owns all rows; this is not a streaming provider or a million-row performance claim.

Native table/header/cell relationships require a coordinated portable semantic and host-adapter extension. This slice retains list/list-item semantics with explicit factual cell descriptions and keyboard header access; native table parity is **degraded/unimplemented**. No host files or semantic role enums are changed here. Header sort/resize accessibility relations remain an acceptance gate, not a claim inferred from drawing.

## Verification and source review

**OBSERVED implemented source:** `ObjectView` exposes `set_details_model`, `details_columns`, `set_details_column_width`, `set_details_sort`, `details_sort`, `sort_requested`, `set_horizontal_offset`, `horizontal_offset` and `details_paint_work`. The item record adds trailing owned `cells`; existing aggregate callers with no cells remain source-compatible. `maximum_details_text_bytes` publishes the development logical-text guard. No default product column set is embedded in the control.

**MEASURED renderer-neutral tests (2026-10-02, Shadow Windows, GNU C++ 16.2.0, Release, two build jobs):** the complete `gui_forms_collection_controls_tests` executable passes in `.build/object-details-stage1`. It includes existing icon/legacy Details/selection regressions and new complete-model validation, primary/focus/anchor/top identity, header/body separation, sort-request/accepted-indicator separation, resize/cancel/capture revocation, empty model, reentrant replacement/revoked listeners, horizontal keyboard reachability, Unicode grapheme elision and repeated-text-cache checks. No host, Skia or HarfBuzz renderer was enabled for this target; these are not native typography or input results.

| Workload | Result |
|---|---|
| 1,000 rows, all selected, 230×190 viewport, 3 columns | 6 visible rows, 12 painted cells, 14 first-paint text preparations |
| 100,000 rows, all selected, same viewport/columns | Same 6/12/14 counts; scrolling retains 6/12 work |
| Cold 65,536-byte cell, 80-unit first column (40-unit text extent), estimated renderer-neutral metrics | 21 width queries submitting 131,152 bytes total |
| Same cold cell with deliberately nonmonotonic prefix-width fixture | 21 queries submitting 131,144 bytes; exact returned ellipsis text fits the 40-unit extent |
| Repeated direct Details paint with unchanged inputs | 0 width queries and 0 text preparations in the cold-cell fixture |
| 1,024 rows carrying 65,536-byte cells plus IDs/labels/other text | Aggregate 64 MiB guard rejects; previous model/primary/top identity remain intact |

Cold Details preparation uses its own `details_text` path, not legacy `elide_object_name`: one full-string width check, an explicit ellipsis check, a logarithmic candidate interval over grapheme boundaries, and a final exact width measurement. The accepted lower endpoint is always a measured-fitting string; the algorithm does not promise maximal retained prefix when shaping is nonmonotonic. Its scratch destination string is reused between probes and retained for subsequent paint. TextStore construction/segmentation remains cold preparation work; query/byte counts do not establish real shaping latency or total memory. The existing legacy icon/no-column elider still uses successive-prefix probes and is not claimed repaired here.

Reproduce from the workspace root:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake -S gui_forms -B gui_forms/.build/object-details-stage1 -G Ninja -DCMAKE_BUILD_TYPE=Release -DGUI_FORMS_ENABLE_SKIA=OFF -DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=OFF -DGUI_FORMS_ENABLE_WINDOWS_HOST=OFF -DGUI_FORMS_ENABLE_MACOS_HOST=OFF -DGUI_FORMS_BUILD_GALLERY=OFF -DGUI_FORMS_BUILD_TESTS=ON -DGUI_FORMS_BUILD_FILE_MANAGER_DEMOBOARD=OFF
cmake --build gui_forms/.build/object-details-stage1 --target gui_forms_collection_controls_tests --parallel 2
ctest --test-dir gui_forms/.build/object-details-stage1 -R '^gui_forms_collection_controls_tests$' --output-on-failure -V
```

Preserved failed checks: the initial repaint-cache assertion inspected Window's display-list replay instead of an invalidated control paint; the corrected check invalidates paint before observing preparations. The first cold-cell test omitted the retained layout flush and consequently painted no cells; the corrected test flushes layout before direct control paint. Both failures were fixture mistakes and neither is reported as successful measurement.

**Source review against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`:** reviewed the public added records/API/state; budget helpers; full model transaction; item/selection lookup and replacement; modified mode/geometry/arrange methods; all new Details header input/keyboard/sort/width/cache/paint methods; added semantic disclosure; and the new named fixture builders/listeners/tests plus counting-painter override. Types are explicit, callback state has named owners/borrows, model strings are owned, per-row selected membership uses a retained hash set, cell geometry uses fixed bounded arrays, cache storage is sized before visible paint traversal, normalization/index allocation precedes model commit, selection payload preparation precedes selection commit, and offscreen row subtraction converts operands before subtraction. Capture state is retired before capture-release notification; sort requests own IDs through reentrant model changes. The initial implementation added no CMake/source unit. The subsequent allocation campaign adds one dedicated test unit/target as recorded below; no anonymous callback, generated source or vendored modification was needed.

**Remaining style/performance scope:** the legacy icon/name wrapping, successive-prefix elider and type-prefix search retain temporary strings and repeated measurement/folding work; these bodies were not rewritten or certified. The existing body pointer/key dispatcher received a Details-routing guard; its remaining legacy body was not wholesale restyled. The complete toolkit, Event implementation, native adapters and application callbacks are outside this source-review claim. Visible-text cache capacity retains its high-water allocation; its storage and container/index/string capacities are additional to the logical model-byte guard. No total heap bound, exhaustive allocator-failure guarantee, native performance claim or full repository house-style compliance is asserted. The bounded allocation campaign below supplies evidence for its named operations and fixtures.

**Gate status:** the initial source-order-only allocation assessment is superseded by the bounded allocation-fault campaign below. Independent consumer evidence remains required before SDK acceptance; the campaign does not establish universal out-of-memory safety. Capture/event/invalidation failures after successful publication do not roll back the model. Native table/header/cell accessibility, host keyboard behavior, large selected-set memory/timings, column persistence/chooser and frontend shortcut conflict resolution remain open.

**Canonical draft review:** `orchestrator/spec/contracts/GUI_OBJECT_DETAILS_DEVELOPMENT.md` revision `object-details-draft-1` is consistent with the source meanings above. Provider reply adds the exact 64 MiB accounting proposal and matching-artifact rebuild requirement. F6 and Alt+Left/Right remain candidate consumer integration bindings. Bounded allocation-fault evidence is now recorded below; native/independent-consumer evidence remains unavailable; no requirement is weakened by the renderer-neutral pass.

## Allocation-failure campaign, 2026-10-02

**Acceptance status:** parent review accepted this campaign's bounded evidence on 2026-10-02, explicitly without full SDK acceptance. **OBSERVED consumer conflict:** `frontend/src/application.cpp` assigns Alt+Left and Alt+Right to Back and Forward. The toolkit width/pan chords remain candidates and are not accepted File Manager bindings. No frontend shortcut changes are included here.

**OBSERVED mechanism:** `tests/object_details_failure_tests.cpp` is a separate renderer-neutral executable registered as `gui_forms_object_details_failure_tests`. Its replacement scalar/array and aligned C++ allocation functions are confined to that executable. A named, noncopyable `FaultScope` enables a thread-local, zero-based single-allocation failure only across the tested operation and disables it during unwinding. Each attempt rebuilds a fresh fixture. Inputs, prior/expected snapshots, observer subscriptions and gesture expectations are constructed before injection. Snapshot comparisons, logging and fixture destruction run with injection disabled. No production allocator hook or public API was added.

The fixture has twelve rows, two columns and strings long enough to allocate beyond small-string storage. Replacement reverses the surviving rows, removes the primary/focused/anchor object, replaces a column identity, changes column width and clears the now-invalid accepted sort. It must preserve the surviving top object's identity. Snapshots cover every public model field and ordered selection, primary/focus/anchor/top, horizontal offset, accepted sort, observable focused header, capture ownership and visible semantic selection states. Active-resize cases additionally move/release the pointer after comparison to verify that pending resize behavior matches the old or new snapshot. The expected replacement is generated on an independent fixture before injection; it is not constructed by reading the faulted object.

**MEASURED:** Shadow Windows, borrowed MinGW GNU C++ 16.2.0, Release, renderer-neutral configuration above, at most two build jobs. The campaign advances through each allocation ordinal until a completion with no injected failure. All 286 injected exceptions escaped from the selected allocation and matched a complete old or complete published state; no partial snapshot or swallowed failure passed. Precommit failures emitted no selection notification, and every unfailed completion emitted exactly one.

| Operation | Active resize | Allocation ordinals exercised | Exact old state after exception | Complete published state after exception |
|---|---|---:|---:|---:|
| `set_details_model` | No | 63 | 53 | 10 |
| `set_details_model` | Yes | 64 | 53 | 11 |
| `set_selected_ids` | No | 28 | 18 | 10 |
| `set_selected_ids` | Yes | 28 | 18 | 10 |
| `select_all` | No | 92 | 72 | 20 |
| `clear_selection` | No | 11 | 5 | 6 |

Postcommit allocation failure is permitted to prevent notification delivery; it does not roll back the model. Eight separate observer cases (each of the four operations with and without reentry) verify that a throwing observer sees the complete published state, that a distinct nested selection survives an outer observer exception, and that a subsequent selection notification still works after unwinding. These observer cases run without allocation injection to separate the failure mechanisms.

**REJECTED prior operation order:** the first expanded campaign reproduced mixed `select_all` state at zero-based allocation ordinal 72: selection and focus had committed, while notification preparation threw before the later `ensure_visible` call could update `top_row`. The correction calculates the next top row in `apply_selection`, commits it with selection/focus/anchor, and removes the late `select_all` scroll mutation. This also prevents an outer `select_all` continuation from overriding scroll state after reentrant notification. Both the fault executable and existing collection-control executable pass after correction. An earlier test compile incorrectly treated `PhysicalKey` constants as an enum type; the fixture now uses their declared `std::uint32_t` representation. The failed compile is not runtime evidence.

Reproduce after the renderer-neutral configure shown above:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake --build gui_forms/.build/object-details-stage1 --target gui_forms_object_details_failure_tests gui_forms_collection_controls_tests --parallel 2
ctest --test-dir gui_forms/.build/object-details-stage1 -R '^gui_forms_(object_details_failure|collection_controls)_tests$' --output-on-failure -V
```

**House-style and failure-order source review:** reviewed the complete new test file, its seven-line CMake registration, and the `select_all`/`apply_selection` correction against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`. Local types and conversions are explicit; executable callbacks are named and retain explicit borrowed references. Each subscription dies or disconnects before its referenced fixture/state. The Window is destroyed before its owning control handle. Inputs move once into the tested transaction; comparison strings and snapshots remain independently owned. The only raw allocation addresses are the isolated replacement-new/delete boundary with matching ordinary/aligned frees. Fault scope cleanup is automatic on exceptions. Fixture rebuilding per allocation ordinal is intentional test isolation, outside repeated production work. The production correction prepares scalar scroll state without allocation, completes allocating event preparation before swaps, commits top/focus/selection together, and performs no later scroll write after notification. No new violation was found in this reviewed scope; the legacy unreviewed/repeated-work scope identified above remains unchanged.

**Limits:** this measures allocation ordinals reached by the named deterministic fixture under one standard library/compiler/configuration, not every possible model, platform or source allocation site. Injection intercepts this executable's C++ new boundary; direct C allocations, OS/host allocations, physical memory exhaustion, concurrent work, BeginInit/EndInit deferred publication, and unrelated selection-mode or body pointer/key paths are not covered. No native renderer/host, sanitizer or leak campaign was run. Internal index/selection coherence is checked through public state and visible semantic behavior, not inspection of every private bucket. Private dirty/cache capacities are not required to equal the prior snapshot. The matching-SDK, independent-consumer and native-accessibility gates remain open.

## Parent-review corrections, 2026-10-02

**OBSERVED source-review corrections:** parent review identified allocating index insertions inside compound validation predicates in `set_details_model`. Column and item handling now perform pure validation first, assign an explicitly typed insertion result second, and reject duplicates third. The replacement-new/delete test boundary now marks read-only size/alignment parameters and pointer values `const`; pointee storage remains mutable as required by allocation/free. The earlier source review missed these spelling/operation-order issues; passing tests did not establish their compliance.

**REJECTED mode-change order:** `test_details_mode_release_reentrancy` reproduced all four capture-release callback cases before correction. Observers saw the previous mode; disposal caused a later outer-setter `logic_error`, and a nested mode/model change could be overwritten by the outer continuation. The fixture reports mutation 0 (dispose) with `committed=0 threw=1`; detach, nested mode and nested model report `committed=0 threw=0`. The outer setter had called `cancel_header_interaction` before updating mode/top/cache.

**OBSERVED correction:** `set_view_mode` retains an owning control handle when available, commits mode/top/header/cache and invalidation first, then retires the header interaction and releases capture as its final action. Capture-release listeners therefore see the completed mode, and no outer setter mutation follows their callbacks. Focused regressions verify disposal, detachment, a nested change back to Details, and replacement with a smaller model plus an explicitly chosen top row. They assert capture retirement, completed state at notification, and preservation of nested results.

**MEASURED:** both `gui_forms_collection_controls_tests` and `gui_forms_object_details_failure_tests` pass after these changes with two build jobs in the existing renderer-neutral Release configuration. All six allocation counts and the 219-old/67-published totals above are unchanged. No mode-setter allocation-failure campaign is implied by the callback regressions.

**House-style review scope:** re-reviewed the two validation/insertion blocks, the complete `set_view_mode` body, replacement-new/delete parameter declarations, and the named mode-release listener/test against `planning/PROGRAMMING_HOUSE_STYLE.md`. Insertion side effects now have separate statements and named results. The setter's lifetime handle spans the synchronous release; its callback-facing commit precedes notification and has no later writes. Test callback references outlive their subscription, enum alternatives and local types are explicit, and the four-case loop rebuilds independently owned fixtures. Remaining legacy/unreviewed scope and SDK/native gates above are unchanged.

## Sort retirement and paint-order review, 2026-10-02

**REJECTED prior sort entry:** the added `test_details_sort_retires_capture` failed before correction because Enter during an active header resize emitted sort while capture remained active. Its first assertion reported `sort must own its identity and follow resize capture retirement`. `request_header_sort` now owns the request and retains the control, snapshots the Details context, retires the pending press/resize and releases capture, then validates the context before emitting. Disposal, detachment, model/revision/mode/geometry or focus changes during release suppress the stale request. A newly established press/resize/capture also prevents the old request from proceeding. No column borrow crosses release notification.

**MEASURED focused regression:** Enter during resize releases capture before exactly one valid sort in the unchanged case. Separate release observers dispose, detach, replace the model with the same column identities, or switch to icons; each suppresses sort. Follow-up movement cannot resume the retired resize. Existing reentrant sort-listener ownership tests continue to pass.

**Fixture correction retained:** the allocation snapshot previously used Enter to probe header focus. After sort retirement was corrected, that probe consumed captured resize state and caused the allocation test's capture case to report a mismatch at failure ordinal zero. The snapshot now avoids Enter while capture is active; the separate move/release comparison verifies the pending resize column and behavior. This was a snapshot side effect, not a new allocation-transaction failure. Uncaptured header focus remains probed through the public sort event.

**Source review against `planning/PROGRAMMING_HOUSE_STYLE.md`:** reviewed the new sort-entry ownership, context checks and release-before-notification order, the named release/sort observer fixtures, and the allocation snapshot adjustment. In `paint_details`, visible-column indexing, header-cache calls and body-cache slot assignment now use separate increment statements; none embeds a slot/count update in indexing or a function argument. This corrects operation-order issues missed by the earlier review. The review remains confined to the new Details paths and named test changes; declared legacy exclusions remain in force.

**MEASURED final checks:** both scoped renderer-neutral targets pass after correction with at most two build jobs. Allocation counts remain 63/64 for replacement, 28/28 for selected-ID publication, 92 for select-all and 11 for clear: 286 injected failures total, 219 exact old states and 67 complete published states. Visible-work counts remain 6 rows/12 cells/14 preparations for both 1,000 and 100,000 selected rows. Cold long-cell counts remain 21 queries with 131,152/131,144 submitted bytes for the two metric fixtures. No frontend, native-host, SDK export or Git operation was part of this follow-up; independent-consumer/native gates remain open.
## Atomic model and accepted-sort publication, 2026-10-02

**OBSERVED:** the public three-argument `set_details_model(columns, items, accepted_sort)` overload validates the explicit direction and nonempty column identity against the incoming columns, including their sortable flag, before changing control state. An empty identity clears the indicator; an invalid direction still rejects. The shared private replacement helper prepares owned model, selection and sort values, then publishes them before invalidation or synchronous notifications. The two-argument overload retains its prior behavior: preserve the accepted sort when its column survives and remains sortable, otherwise clear it. The control does not sort the supplied rows; the consumer supplies their actual order and matching accepted indicator.

**MEASURED:** both `gui_forms_collection_controls_tests` and `gui_forms_object_details_failure_tests` pass in `.build/object-details-stage1` on Shadow Windows with MinGW GNU C++ 16.2.0, Release, renderer-neutral configuration and two build jobs. Focused coverage rejects missing/unsortable IDs and invalid directions (including an empty ID), preserving model, indicator, selection, focus, anchor and top. A throwing selection observer sees reversed replacement rows and the replacement-column descending indicator together. Separate assertions cover two-argument retention and explicit clearing.

The allocation campaign now includes the explicit-sort overload both without and with an active resize. Current counts supersede the historical operation counts above for this source state:

| Operation | Active resize | Injected failures | Exact old state | Complete published state |
|---|---|---:|---:|---:|
| Two-argument replacement | No | 62 | 52 | 10 |
| Two-argument replacement | Yes | 63 | 52 | 11 |
| Explicit-sort replacement | No | 62 | 52 | 10 |
| Explicit-sort replacement | Yes | 63 | 52 | 11 |
| Selected-ID publication | No | 28 | 18 | 10 |
| Selected-ID publication | Yes | 28 | 18 | 10 |
| Select all | No | 92 | 72 | 20 |
| Clear selection | No | 11 | 5 | 6 |

All 409 injected failures match complete old or published snapshots (321 old, 88 published). Snapshot coverage includes the accepted indicator. Fixture input strings are prepared before injection. The existing eight throwing/reentrant observer cases also pass. Visible-work and cold-text measurements remain 6/12/14 at both row counts and 21 queries submitting 131,152/131,144 bytes. This is bounded exception-state evidence, not universal allocation safety or native-host verification.

**Source review:** reviewed the added public/private declarations, both forwarding overloads, shared replacement preparation and commit order, named `DetailsCommitObserver` and focused test, plus the added failure-campaign operation/input/dispatch against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`. Types and conversions are explicit; callback borrows end at disconnection before their local state is destroyed; accepted-sort ownership moves into prepared state; all rejection precedes commit; no new per-row allocation or traversal was introduced by the sort validation. No remaining house-style violation was identified in this added scope. Previously recorded legacy exclusions remain. Concurrent root-owned keyboard/context changes are outside this follow-up's source-review claim. Application recovery, canonical contract integration, SDK export, native verification and final acceptance remain with their owners.
