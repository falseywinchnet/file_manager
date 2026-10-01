# Prepared text retention and Windows adapter development checkpoint

**OBSERVED:** root authorized this exact slice after independent acceptance of
the service corrections at `6992bcb`. The historical shutdown hold was withdrawn.
This checkpoint extends the independent service/raster; it does not enable a
new installed SDK, change Window ownership, or satisfy document editing.

## Ownership and build boundary

Renderer-neutral storage moved from `src/render/text/prepared_storage.*` to
`src/core/text/prepared/prepared_storage.*`. Plain glyph/run/geometry records
now live in `src/core/text/shaping/shaped_text_geometry.hpp`; the private HB
header consumes those records. No FT/HB/SheenBidi/native header or call enters
core. The session factory moved into `prepared_service.cpp`. Core owns value
lifetimes, ledger and authority operations; the optional prepared component
owns the worker and native raster. This avoids a core/prepared link cycle.

The development Painter virtual and command fields are guarded by
`GUI_FORMS_PREPARED_TEXT`, supplied consistently through core's PUBLIC
BUILD_INTERFACE definition when the option is enabled. Painter forward-declares
the needed types and does not include an excluded prepared header in the OFF
SDK. Standalone Painter-header syntax checks pass both OFF and ON. Root owns
all CMake changes, including refusal of prepared-ON installation before copies.
Ordinary OFF packaging is being checked independently by the coordinator.

The Windows override additionally requires `GUI_FORMS_WINDOWS_PREPARED_TEXT`
and the transactional DIB profile. The optional component stays OFF by default.
Window, HostServices, HostSession and cursor lifecycle behavior are unchanged.
The caller owns its per-view service and shutdown/wake lifetimes in this slice.

## Recording, replay and transaction rules

- Painter returns a typed result distinguishing `recorded`, `staged` and refusal.
  The base implementation refuses with `incompatible_backend`. Neither recording
  nor candidate pixels mean a frame was presented.
- Recording validates authority and finite placement, then retains private
  `shared_ptr<const PreparedTextStorage>`. It never borrows the caller's wrapper.
  Ignored refusal poisons the candidate recorder; `finish` throws a typed paint
  failure rather than publishing an incomplete chunk.
- Replay rechecks authority and makes a temporary movable wrapper over the
  retained storage. Unsupported/stale/failed drawing throws a typed paint
  failure. The existing Window exception path abandons candidate chunks and
  denies a paint receipt; no text operation silently falls back to GDI shaping.
- The Windows painter requires the service's opening executor and exact scale.
  It completes native raster before any text pixel write, flushes outstanding
  GDI writes, then blends into the candidate DIB under the authority lock.
  Any refused prepared operation makes the candidate uncommittable, even if a
  direct caller ignores the return value.
- Each frame has a fixed table of at most **64 distinct session authorities**.
  Duplicate authority references share an entry; mixed epochs of one session
  refuse. A 65th authority returns budget-exceeded. This is a finite Windows
  development profile, not a new global service quota or installed capability.
- At frame commit, retained authority mutexes are acquired in process-unique
  session order and validated before DIB commit. Local strong owners outlive
  these guards even when abort clears the painter's member owners. Stale or
  failed commit retains the previous committed pixels, frame key and presented
  receipt. Failed OS presentation retains the existing DIB profile's best-effort
  exposure recovery; it is not a claim that the screen never transiently changes.

Absolute baseline DIPs include the painter translation once. Device placement
rounds to the nearest integer with signed half ties away from zero; prepared
glyph positions and mask pixels are not scaled again. Rectangular and rounded
clips test pixel centers. Gray coverage combines with color alpha and blends
into opaque BGRA candidate pixels. Target stride/extent, finite geometry and
scale are validated before the nonallocating compositor loop. Target and mask
storage are disjoint by the DIB/mask ownership boundary.

The 64-authority limit, placement law and recorded/staged distinction were sent
to the coordinator for canonical-record reconciliation before promotion.

## Validation and review

**MEASURED:** Windows Shadow, Plan Paint MinGW GCC 16.2 toolchain borrowed
read-only, C++20 Release and pinned text dependencies. Commands:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake -S gui_forms -B gui_forms/.build/house-style-text -DGUI_FORMS_BUILD_PREPARED_TEXT=ON -DGUI_FORMS_ENABLE_WINDOWS_HOST=ON -DGUI_FORMS_WINDOWS_TRANSACTIONAL_DIB=ON
cmake --build gui_forms/.build/house-style-text --target gui_forms_host_windows gui_forms_prepared_text_service_tests gui_forms_prepared_text_raster_tests gui_forms_prepared_display_tests gui_forms_windows_prepared_text_tests gui_forms_windows_dib_lifecycle_tests gui_forms_text_store_tests gui_forms_bounded_grapheme_tests gui_forms_bounded_shape_tests gui_forms_harfbuzz_font_engine_tests --parallel 2
ctest --test-dir gui_forms/.build/house-style-text --output-on-failure -R 'prepared_text|prepared_display|dib_lifecycle|bounded|harfbuzz|text_store'
```

Production Windows host and all selected test targets build. Nine focused
suites pass; final raw output is in
`PREPARED_TEXT_A2_INTEGRATION_TESTS_2026-10-01.txt`. This is correctness evidence,
not a latency benchmark. The new tests prove command retention after wrapper
release, retirement across copied chunks, fourth-generation refusal while old
chunks live, stale replay, unsupported-backend refusal without ordinary-text
fallback, and recorder failure despite an ignored refusal.

The native test uses only its own hidden HWND, with no activation or global
input. It proves prepared text reaches a committed DIB, stale cached replay
leaves old pixels/key/presented receipt intact, injected commit-flush failure
preserves them, later valid content recovers, and cancellation after staging
denies commit. A bounded generated-record fixture checks the 64-entry authority
table, ordering, deduplication and mixed-epoch rejection. Compositor tests check
known alpha values, rectangular and rounded clipping, row padding, invalid
extent/scale preservation and signed half-device placement. The prior real-Window
DIB lifecycle test remains green. No visible typography inspection is claimed.

Strict `-Wall -Wextra -Wconversion -Wsign-conversion -Werror` syntax checks pass
for the neutral storage, new compositor and new test translation units, with
existing public headers treated as system includes. The complete guarded
Windows translation unit and fixture pass ordinary syntax/build checks;
unchanged legacy Windows code is not claimed strict-warning-clean.

Source review against the complete `planning/PROGRAMMING_HOUSE_STYLE.md` covers
the neutral storage move/factory split and new geometry header; authored public
Painter/result declarations; authored base Painter, display command,
recording/replay hunks; all guarded prepared additions in `windows_host.cpp`;
the complete compositor pair, hidden native fixture and two new test files;
and the changed service/HB/test include paths and explicit include additions.
Root reviews CMake separately. No unrelated legacy or vendor code is certified.

Reviewed explicit types and unit conversions, named behavior, initialization,
executor confinement, retained ownership and borrow expiry, mutex ordering,
checked extents before pixel access, failure/rollback order, and stable
allocation-free compositor storage. The callback and worker contracts from the
previous service checkpoint remain in force. No remaining house-style violation
was identified in this authored scope. A final review added native class cleanup
on fixture exceptions and explicit standard-library includes.

Independent integration review is complete for this development slice, as
recorded below. Public backend-capability negotiation,
Window/document-view attachment, consumer-ready lifecycle binding and SDK export
remain separate gates. macOS/Linux prepared painting, wrapping/newlines/tabs,
monochrome, caret/hit/selection/editing, sanitizer/resource-exhaustion testing
and responsiveness measurements are not established by this checkpoint.

## Final read-only parameter correction

The coordinator's final house-style audit found missing explicit `const` on
read-only value parameters. Corrected the authored prepared Painter declarations
and definitions; compositor helpers and entry point; prepared storage authority,
validation, metrics and reservation inputs; service/raster definition inputs;
and prepared service/display/Windows test helpers and fixture inputs. Seventeen
files changed within the existing 21-file manifest. Geometry-only records,
replay, display-command storage and the shaper-header extraction needed no new
parameter correction. Unrelated legacy Painter/Windows methods were not changed.

Moving shared owners, move constructors/assignment, output references, mutable
working variables and native objects remain mutable. A const span/target
descriptor does not make its destination pixels const; the compositor retains
exclusive mutable pixel access. No behavior or function-type/ABI change is
intended by these top-level parameter qualifiers. Public declarations outside
the 21-file authored integration scope were not rewritten.

Reviewed the corrected executable inputs against the complete house style,
including ownership transfer and pointee mutation rather than applying textual
const replacement to working state. No remaining read-only value-parameter
violation was identified in this reviewed authored scope. Earlier blanket
scope statements must be read with this correction; passing tests alone had not
established parameter spelling compliance.

Rebuilt service, raster, display and both Windows fixture targets with two jobs.
Final focused run after correction: **9/9 passed in 1.16 seconds**. The existing
test receipt and all 21 source hashes were refreshed. Source is frozen again
for the coordinator's final verification and commit.

## Independent coordinator acceptance

The coordinator verified all 21 final source hashes and reviewed the authored
integration scope and root CMake wiring. This includes immutable display storage
retention, poisoned-recording refusal, replay without ordinary-text fallback,
executor and scale checks, checked compositor extents, and frame commit authority.
The fixed authority table sorts distinct session identities; strong local owners
are declared before the lock guards, so abort cannot destroy a locked mutex.
Authority-to-ledger ordering agrees with the reviewed cancellation/retirement
paths. Duplicate provider instances remain outside the admitted identity model.

The final qualifier corrections were reviewed for ownership transfer, mutable
pointees and outputs. No remaining violation was found in the exact authored
scope. Unchanged legacy code is not certified. The independently rebuilt native
host and nine focused test targets passed **9/9 in 0.99 seconds** after the final
source freeze. The hidden native fixture validates committed DIB bytes and
failure preservation; it does not establish visible typography or latency.

The coordinator also exercised packaging boundaries in dedicated build outputs:

- ON: `cmake --install gui_forms/.build/house-style-text --prefix
  .build/a2-on-install-check` refused before any files were copied.
- OFF: the native application rebuilt in `gui_forms/.build/shadow-windows` and
  installed into `.build/a2-off-install-check`. The separate existing installed
  Controls consumer configured, linked and ran successfully in
  `.build/a2-off-consumer`; prepared headers and compile definitions were absent
  from that SDK. These OFF checks preceded the final top-level-const correction;
  they establish extraction/export separation, not a new distributable artifact.

Accepted as Windows development integration only. Public SDK publication,
consumer lifecycle binding, wrapping, monochrome masks and other platform
backends remain separate work. No additional product availability is implied.
