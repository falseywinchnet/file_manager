# A2 prepared text: private CPU Skia transaction adapter

Date: 2026-10-01. **OBSERVED source checkpoint; native execution pending.**

The coordinator assigned this adapter after mask checkpoint `7b260cf`, toward
three-platform read-only document-view consumption. Canonical semantics remain
`orchestrator/spec/contracts/GUI_PREPARED_TEXT_DEVELOPMENT.md`; earlier D2/D3
proposal language does not supersede accepted A2 source/revision, authority,
reservation or transaction laws. No public Painter declaration changed.

## Exact implementation and host integration

Provider source scope is the five files in
`PREPARED_TEXT_SKIA_HASHES_2026-10-01.csv`: existing private
`src/render/skia/raster/skia_raster.hpp/.cpp`, new private
`prepared_skia_frame.hpp/.cpp` in that directory, and
`tests/prepared_text_skia_tests.cpp`. Root owns CMake, CI and native hosts.
No text-mask, native-host, D1, public prepared API or worker code changed here.

**OBSERVED:** the existing typed Painter operation suffices. The adapter calls
`rasterize_prepared_text` with the exact immutable A2 storage/authority and
retained font bytes. It does not call HarfBuzz or Skia font lookup to reshape or
reinterpret glyph IDs. A2 device-pixel positions and FT raster profile remain
authoritative. Placement converts baseline DIPs once, includes canvas translation,
rounds nearest with signed half ties away from zero, and does not rescale the
mask. Only scale plus translation is admitted. The CPU canvas draws an Alpha8
image with the requested color and source-over blending, preserving device clip.

The existing charged `GrayTextMask` moves into `MaskImageOwner`. A named release
callback owns a shared reference through Skia's pixmap-release context. Any
native image retention therefore retains pixels and their original ledger charge;
there is no uncharged pixel copy or third mask. The pinned factory at
`third_party/skia/src/image/SkImage_RasterFactories.cpp` validates arguments
before installing the release callback in SkData, then transfers that owner into
the image. The local shared owner and callback flag distinguish pre-registration
rejection from synchronous release on failure. The callback retires the context
and mask after the last native image reference releases it. Fixed context/control
allocation overhead is not asserted to be part of the A2 mask-pixel byte quota.

The guarded private host seam is:

```cpp
PreparedTextStatus begin_prepared_frame(Size, double, DamageRegion&);
PreparedTextStatus commit_prepared_frame(PaintReceipt);
void abort_prepared_frame() noexcept;
PaintReceipt prepared_front_receipt() const noexcept;
double prepared_front_scale() const noexcept;
bool prepared_front_matches(Size, double) const noexcept;
```

`begin` validates executor before state access or mutation. Invalid initial
admission preserves any ordinary surface. Successful admission selects the
transactional route; future ordinary `resize` cannot replace its coherent front.
All frame work and pixel borrows remain on the renderer's construction executor.
Wrong-executor draw/begin/commit refuses without mutating an active transaction;
wrong-executor abort does nothing.

The frame owner retains at most two RGBA8 CPU surfaces, each at most 16,777,216
pixels, axis at most 16,384 pixels, scale 0.5..4: at most 128 MiB of requested
pixel storage combined. Obsolete scratch is released before replacement
allocation. Native Skia object/allocator overhead is separately opaque, not a
process-memory quota. Allocation and size refusal preserve the coherent front.

`begin` clips supplied damage to the admitted extent and expands it to a full
repaint for initial/changed extent or scale, empty damage, or revoked front
authority. A matching partial candidate copies the entire previous front and
inherits its authority owners. Full repaint starts with an empty authority table.
The sorted fixed table admits at most 64 distinct sessions, deduplicates equal
authority, refuses mixed epochs and refuses the 65th session. Commit retains
strong owners before lock guards, takes locks in session order, rechecks every
authority, and holds the locks through candidate/front publication. Old front
owners retire only when the front is replaced. Failed draw poisons candidate
commit. Revocation after staging or inherited partial-frame revocation refuses
commit and preserves the actual previous pixels and receipt.

Host integration points, assigned to root:

- Mac `drawRetainedRect`: replace the guarded ordinary resize/begin route with
  `begin_prepared_frame`, then paint its updated damage. Commit only a nonempty
  successful model receipt; abort for absent receipt and exception cleanup.
  `end_frame` only restores canvas state and never commits. Prior-frame exposure
  uses actual front dimensions, scale and receipt and does not acknowledge new
  model content. Live-only updates cannot substitute an invented model receipt.
- Linux `NativeWindow::update`: the same guarded begin/paint/commit sequence
  precedes X11 presentation. Refusal/exception/null receipt aborts and must not
  expose candidate pixels or acknowledge presentation.
- Both hosts acknowledge the model only after their native presentation succeeds.
  Surface publication is not a claim of physical-display atomicity if a later
  native presentation operation fails.

Ordinary OFF code keeps its historical route. The only shared accessor refactor
selects the same ordinary surface when the guarded transaction is unavailable.
Prepared ON package export remains a concrete gate pending three-platform
adapter and independent installed-consumer checks, not an indefinite prohibition.

## Validation and retained failures

**MEASURED:** local Shadow Windows compile-only checks used borrowed Plan Paint
MinGW GCC 16.2 via `tools/Enter-WindowsToolchain.ps1`, C++20, and exact pinned
Skia `2a9b593bab4b2fd019fa494c8d401ff1fab0b883` fetched by root with repository
patches. New helper and test compile with `-Wall -Wextra -Wconversion
-Wsign-conversion -Werror`; existing raster compiles ON and OFF with
`-Wall -Wextra`. Public/vendor include roots are system includes. These are
syntax/type checks, not linked native execution or a blanket legacy style audit.

The focused executable takes one `assets/fonts` directory argument. Eight
groups are authored for native CI execution:

1. A2 mask parity at scale 0.5, 1, 1.25, 1.5, 2, 3 and 4, combining composition,
   fractional translated placement, RGBA channels and opaque destination alpha.
2. Hidden candidate, cleanup without commit, null receipt, oversized resize,
   aborted resize, successful resize and old front/receipt preservation.
3. Partial-frame inherited authority, cancellation before commit, forced full
   repaint after revocation, and revocation after prepared draw stages.
4. Actual typed record/replay after unique wrapper release, incompatible-scale
   refusal and poisoned candidate commit.
5. Clip retention across device-matrix reset, space-only mask and nonfinite
   placement refusal.
6. Sorted 64-session boundary, duplicate/mixed epoch/65th-session behavior and
   deterministic candidate-allocation refusal preserving front and receipt.
7. Wrong executor on ordinary and active paths; initial invalid geometry keeps
   ordinary pixels.
8. Retained native image keeps its transferred mask pixels and budget charge;
   last image release retires the charge; factory rejection without callback
   leaves no mask charge.

No local Skia archives are available, so these eight groups have not been run
locally. Root owns native CI execution and host integration verification.
Pixel comparison allows one channel level for Skia integer compositing rounding;
this is not a visual golden, typography judgement, responsiveness benchmark,
sanitizer run or independent consumer result.

Retained development failures: first compile exposed the presentation header's
RuntimeId include prerequisite; the adapter now includes the defining control
header. Skia `peekPixels` requires a mutable native surface handle even for a
const pixel borrow; the private accessor now preserves that native requirement.
Test setup corrected the wake pointer and existing PaintPlane enumerator.
Damage intersection uses the existing static two-argument operation. Coordinator
review caught executor checks after mutable frame access and an insufficient
shared-pixmap lifetime assumption; both were corrected and focused fixtures added.

## Performance limits and remaining contract delta

**OBSERVED:** each matching partial candidate currently copies the whole front.
Every prepared draw calls the A2 rasterizer, which opens native FT faces and
renders glyphs for measurement and fill again. It does not reshape, but there is
no warm-mask reuse. This slice makes no speed, idle-CPU or latency-improvement
claim. Before installed editor adoption, measure small-damage and warm replay
against the current path; add bounded reuse only if that cost is the measured
bottleneck. No new broad performance suite was started in this adapter slice.

**CANDIDATE smallest next D2/D3 delta**, separate from adapter acceptance:

- D2a-v0 needs a retained read-only control/controller that binds exact D1
  revision/page and A2 authority, owns one active result/replacement intent,
  emits bounded named page requests, and reports current/pending/previous/
  unavailable coverage. Service/wake attachment, revocation before detach,
  reentrant notification rules and shutdown/join ownership need the concrete
  matched-source binding. Painting consumes the existing prepared command.
- Visible control-character labels can already enter A2 as D1-owned expanded
  display text with exact source/token mapping. The minimum delta is the
  consumer's explicit display projection setting and its identity/invalidation,
  plus retained-view presentation of that mapped text. No source rewriting or
  glyph-cluster-derived source identity is needed. The original tab/newline
  layout limitation remains unless those bytes are projected as atomic labels.
- Actual source hit testing/single-selection requires D3 immutable visual stops
  and line extents tied to the existing full key and certified source endpoint
  pairs, with explicit bidi affinity and atomic expanded-token endpoints. A2
  glyph clusters alone cannot certify source grapheme navigation. Publish this
  as a separate interaction capability; never estimate missing geometry silently.
  Visual-stop count and bytes must join the existing payload budget before
  allocation. The earlier proposal's comparison ceilings are not newly selected.

Multiple selection, editing, wrapping/long-line continuation and printing are
outside this adapter slice. The visual-only retained view can be consumed before
interaction geometry, provided it reports that distinction explicitly.

## Exact house-style review

Reviewed the complete new `prepared_skia_frame.hpp/.cpp` and
`prepared_text_skia_tests.cpp`, and every authored declaration/hunk in existing
`skia_raster.hpp/.cpp`, against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`. Review covered explicit initialized types,
named execution and release context, shared owner/borrow lifetimes, executor
admission, lock-owner destruction order, operation order and publication,
scale/placement conversions, typed failure states, allocation admission and
repeated-loop work. Named callback context and static operation declarations
retain no anonymous executable behavior. Whole damage/surface validation occurs
before drawing; per-authority/per-mask checks remain where values can vary.
No known house-style violation remains in that exact authored scope.

Unchanged raster methods, A2 service/raster, test support, public/core headers,
Skia/vendor code and root-owned CMake/CI/host changes are not certified by this
review. Functional test success, when available, does not replace source review.

## Coordinator integration review

**MEASURED:** the coordinator independently matched all five final source hashes,
reran the strict helper/test syntax check on Shadow with the pinned Skia headers,
and ran the spelling checker over the three new C++ files (zero findings).
Manual review included the final image-release ownership and rejection paths,
damage clipping/full-repaint admission, executor guards, and the eight-group
test source. This confirms the reviewed source scope, not native execution.

**OBSERVED:** CMake now links the private adapter only when the prepared-text
target exists, and supplies the test's private Skia include root. POSIX CI enables
prepared text after the ordinary SDK export, then builds and runs its four
focused tests alongside the four mask tests. This does not enable the feature
in the exported SDK or integrate the macOS/Linux native presentation loops.
The authored CMake/CI changes were reviewed for dependency order and scope.
Native adapter execution remains pending the next CI run.
