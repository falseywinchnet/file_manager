# Windows DIB candidate/front transaction — A1 development

Date: 2026-10-01. Status: **OBSERVED implemented development option; measured
focused fixtures; coordinator integration review pending.** No SDK was installed.

## Scope and ownership

The coordinator authorized the Windows frame-transaction prerequisite before
prepared text enters a production Painter. `GUI_FORMS_WINDOWS_TRANSACTIONAL_DIB`
is OFF by default. It does not select a program-wide renderer or provide the
public prepared-text service, wrapped text masks, or editing geometry.

The private `DibFrameStore` owns a committed front and a candidate, on the
constructing UI thread. CPU access to the front is read-only and exposes no HDC.
Candidate views expire at begin/commit/abort/close. The host's drawing aliases
point to candidate storage only during painting; an independent compatible DC
supports metrics before the first frame and between frames. Save-BMP reads the
committed front, including its actual old dimensions after a failed resize.

**GIVEN development bound:** 16,777,216 pixels / 64 MiB per 32-bit DIB. Two
surfaces occupy at most 128 MiB of controlled pixel payload; resize replacement
may temporarily retain three, at most 192 MiB. Native DC/bitmap bookkeeping and
allocator overhead are separate and not bounded by that number. This is not a
process hard quota. Dimension, pixel-product and byte-product conversions are
checked before allocation. Zero size does not allocate or relabel the old front.

Begin initializes the full candidate and copies the clipped old front for
partial-damage preservation. This incurs full-surface memory work; its cost has
not been benchmarked and no performance improvement is claimed. Failure before
commit preserves front bytes, actual dimensions, revision, epoch and scale.

The host accepts a candidate only after a valid Window paint receipt and native
flush/presentation success. It acknowledges only a newly presented receipt;
same-revision recovery clears damage without duplicate acknowledgment. A failed
frame retains damage and requests one exposure-only recovery pass. Later native
or application paint requests retry; permanent allocation failure does not
create an unconditional repaint loop. Exceptions from the candidate paint path
are contained and reported at the host boundary.

Live placement sampling requires the front's revision/epoch/scale/extent to
match the current model and no pending retained damage. Live candidate failure
preserves the front. GDI source consumption finishes while the LiveSurfaceFrame
read lease remains owned. Native batching is flushed before live CPU copying
and before releasing native source borrows. GdiFlush reports failure if an
operation in the flushed batch failed; it does not mean the batch remains
pending. See [Microsoft's GdiFlush contract](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-gdiflush).

**Limit:** BitBlt failure cannot prove that physical screen pixels remained
unchanged. Recovery reexposes the retained front best-effort. This change does
not audit every legacy GDI primitive's return checking or CPU/GDI ordering;
errors from automatically flushed batches are not all reported by a later
GdiFlush. The guarantee is the tested transaction boundary and committed backing
preservation, not universal detection of every native drawing fault.

## Validation

**MEASURED:** Shadow Windows, borrowed read-only Plan Paint MinGW GCC 16.2.0,
C++20 Release, native Win32 host, Skia OFF, HarfBuzz text ON. New build projection
`gui_forms/.build/windows-prepared-dev`; frozen SDKs and app stages unchanged.

Final focused CTest: **2/2 passed**, store 0.03 s, hidden Window fixture 0.18 s,
total reported 0.22 s. These are functional run durations, not latency benchmarks.

- Store: allocation/begin-flush/commit-flush/present refusal, real null target,
  byte-exact old-front exposure, subsequent partial update, resize copy/clear,
  old scale/identity preservation, invalid/oversized/zero dimensions, invalid
  receipt/scale, foreign-executor refusal, repeated close, and GDI object count
  returning to baseline over 32 allocation/close iterations.
- Native host: real retained Window and hidden owned HWND; metrics DC available
  before first paint; successful receipt; failed commit without acknowledgment;
  exposure without a new paint lease; retry; callback exception containment;
  callback-triggered epoch revocation; live pixels and unchanged receipt;
  two-buffer producer reuse after native read-lease release; failed live update
  and same-revision recovery displaying the latest published `0x77` coverage
  bytes without another producer publish; stale placement guard; failed resize and recovery;
  zero-size preservation; native close and owner destruction.
- Production ON host library builds. Production archive symbol inspection found
  no `DibHostFixture`, `DibFixtureControl`, or fixture entry. Test access is
  compiled only with `GUI_FORMS_DIB_LIFECYCLE_TEST` in the separate test target.
- Default OFF host source passes a separate C++20 syntax check. This is not a
  fresh default-profile full application build or GUI dogfood run.
- Helper and the two test entry/source files pass `-Wall -Wextra -Wconversion
  -Wsign-conversion -Werror` syntax checking. Legacy host code is not certified
  warning-free by that command. `git diff --check` passed.

Reproduction (after dot-sourcing `tools/Enter-WindowsToolchain.ps1`):

```powershell
cmake -S gui_forms -B gui_forms/.build/windows-prepared-dev -G Ninja `
  -DCMAKE_BUILD_TYPE=Release -DGUI_FORMS_ENABLE_WINDOWS_HOST=ON `
  -DGUI_FORMS_ENABLE_SKIA=OFF -DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=ON `
  -DGUI_FORMS_BUILD_TESTS=ON -DGUI_FORMS_WINDOWS_TRANSACTIONAL_DIB=ON
cmake --build gui_forms/.build/windows-prepared-dev --parallel 2 `
  --target gui_forms_host_windows gui_forms_windows_dib_frame_store_tests `
  gui_forms_windows_dib_lifecycle_tests
ctest --test-dir gui_forms/.build/windows-prepared-dev `
  -R '^gui_forms_windows_dib_' --output-on-failure
```

No desktop input or activation was used. A hidden native-window correctness
fixture does not prove visible compositing, perceived responsiveness, OS fault
atomicity, or prepared-text availability.

## Failed checks and review corrections

The initial configure used the nonexistent `GUI_FORMS_ENABLE_HARFBUZZ` variable;
CMake warned that it was unused. It was removed and the correct
`GUI_FORMS_ENABLE_HARFBUZZ_TEXT=ON` option configured. The first native fixture
compile assumed an `active_read_leases` snapshot field that does not exist;
the check now exercises actual two-buffer recycling instead. Strict checking
rejected MinGW's HGDI_ERROR macro's signed narrowing; a named pointer-width
sentinel replaced it. These were setup/source defects, not passing evidence.

Independent coordinator review identified effectful test calls nested in
`require`. All 27 such store operations and eight native-fixture operations now
execute into explicit typed, named results before assertion. Final tests passed
after that correction. Review also corrected initial metrics-DC alias setup,
same-revision recovery damage clearing, and live source-lease flushing.

The coordinator's final functional review found that the synthetic control
registered live presentation but did not retain/draw the surface in normal
painting. Clearing pending damage alone did not establish newest-frame recovery.
The corrected fixture owns the LiveSurface and records its draw command before
fast presentation. It then checks exact old-front preservation after failure and
exposure, followed by latest published pixel recovery without a new publish.
The final 2/2 result above includes this stronger assertion. No production change
was required for that fixture correction: retained live commands sample the
latest owned frame during replay.

Semantic review against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`
covers the complete new helper pair, store test, lifecycle test entry and
included fixture, plus only authored host hunks: resize/begin/commit/exposure,
live presentation/borrow completion, saved-front access, bitmap cleanup, metrics
DC ownership, failed-frame recovery and guarded test seam. Explicit types,
named behavior, operation order, bounds/conversions, initialized storage,
executor ownership, borrowed-view invalidation, cleanup, failure publication
and repeated copying were reviewed. No remaining violation was found in that
authored scope after the coordinator correction. Unchanged host primitives,
existing core/controls, unrelated tests and vendors are not globally certified.
Root owns CMake changes and their separate review.

The parent reviewed the authored helper, host, test and CMake scopes, verified
the final lifecycle fixture hash, confirmed all three selected targets were
up to date, and independently reran both focused CTests: 2/2 passed in 0.06
seconds total. The test includes recovery of the latest already-published live
frame after a failed presentation, without another producer publication.

SHA-256 of reviewed provider files:

| File | SHA-256 |
|---|---|
| dib_frame_store.hpp | `47ab9f4a36c7f8448fd53a5c759a034a007bc45cba7d2d8e9c6ee02334f2edf1` |
| dib_frame_store.cpp | `57a7d779e35c41650949a56e905aca6d501bf36e53837142f995cf0e1683e9c4` |
| windows_host.cpp | `9e81aa7c91a13397a0d549240ee78669ab6d4246b5ec114fa6f44ce17d9a082e` |
| windows_dib_lifecycle_fixture.inc | `34c0f38d20ab3c164687266c931313206dadff6a6f406f948e62be0794dc7113` |
| windows_dib_frame_store_tests.cpp | `a4ca07c56ec3fa6d79c32a7abc8efb7a047b5e833417dbf7555d0085346b0e50` |
| windows_dib_lifecycle_tests.cpp | `fd6a5fb7e20869d0fd1c9abccbcbfe845d540541d48e40f0c0d7c1825f51d54e9` |
