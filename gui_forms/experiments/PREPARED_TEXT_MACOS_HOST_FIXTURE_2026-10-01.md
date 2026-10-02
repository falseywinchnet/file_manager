# Native macOS prepared-glyph host fixture

Date: 2026-10-01. **OBSERVED authored fixture; native compilation/execution pending.**

Coordinator assignment is restricted to the new
`tests/macos_prepared_text_host_tests.mm` and this receipt. Root owns Mac host
integration, CMake and workflow changes. The profiling sibling owns the existing
idle fixture. None of those files or public APIs changed in this provider slice.

## What the fixture exercises

The executable uses public A2 preparation APIs and `run_macos` with a retained
control that records actual prepared glyph commands. Host typography loads from
the fixture's bundled NSBundle resources. The separate encoded A2 font bank
copies Carlito from the single supplied font-directory argument. Both font
dependencies must therefore be available; an encoded font path alone does not
satisfy the host's bundled-font startup requirements.

The native sequence is bounded and evidence-driven:

1. Wait for the named visible NSWindow, actual application painting and a
   nonzero model presented revision. This initial readiness path does not call
   snapshot, display or forced paint. Capture `backingScaleFactor` from that
   window; the fixture never assumes scale 1.
2. Submit complete bounded A2 text with that exact scale, including a combining
   sequence. Poll the service's completion slot from main-queue continuations,
   adopt only the expected authority, invalidate the retained control and let
   the native host paint. No synchronous wait/sleep for shaping runs on UI.
3. Wait for a later actual model presented revision and a prepared recording.
   Only then request an AppKit bitmap exposure while the model is occluded.
   Check opaque dark ink against the light background. This is a native glyph
   presence check, not an exact font golden or a typography-quality claim.
4. Inject a control callback failure after its background command, invalidate,
   and wait until the existing private native fault counter records the attempt.
   Verify the presented revision did not advance. Occluded AppKit exposure must
   exactly preserve the prior prepared bitmap; it cannot expose the failed red
   background or an incomplete replacement.
5. Clear the failure, resize content from 280x120 to 360x180, submit a valid
   replacement at the observed backing scale, and wait for a later native
   receipt. Verify prepared ink, changed content and enlarged captured extent.

Every occluded snapshot checks that application paint count, presented revision
and accepted presentation-receipt count remain unchanged. This directly covers
the null-model-receipt exposure path. A backing-scale change during the bounded
fixture explicitly fails rather than silently preparing at the wrong scale.

The only private Objective-C selectors used are existing `hostJSON`,
`collectDamage` and `notifyOcclusion:`. No new native diagnostic or public ABI is
introduced. `collectDamage` schedules normal host work after valid adoption;
the initial readiness predicate is independently observed before any snapshot.

## Continuation, ownership and failure review

Each `dispatch_after_f` context owns a `shared_ptr<State>` through a named
`DeferredStep`. There is at most one scheduled continuation. The executing
callback copies that owner before queuing its successor; no reference to a local
holder or anonymous capture survives the call. Each state transition sets a
ten-second deadline, with 50 ms readiness checks. CTest should use a 90-second
timeout to cover all stages and native startup.

Native view/window borrows are resolved afresh and last only for the main-queue
invocation. The model borrow is valid only while the native host owns the model.
The named closed target clears it and marks shutdown; delayed callbacks check
that state before any borrow. Close invocation retains a local callable copy so
a synchronous closed notification may clear the stored target without destroying
the executing target. The session is closed and joined after the native run
returns. No worker touches AppKit or Window.

C++ exceptions and Objective-C exceptions are contained in the continuation,
record a failure and close the fixture. Capture is capped to 2048 per axis and
one million pixels before allocating RGBA comparison storage. Channel values
are finite-checked, clamped and rounded explicitly. Captures perform no source
mutation and use generated fixture text only.

## Exact root build and native-test needs

- Target: `gui_forms_macos_prepared_text_host_tests`, source
  `tests/macos_prepared_text_host_tests.mm`, Objective-C++20 with the same AppKit
  and ownership settings as existing native fixtures.
- Link: `gui_forms_host_macos`, `gui_forms_prepared_text`, AppKit/framework
  dependencies inherited or explicitly supplied by root CMake.
- Build only with the native macOS host and prepared-text option enabled.
- Make a bundled fixture and copy the ordinary host font resources using the
  same root-owned bundle helper as the other Mac native tests.
- Supply an absolute `gui_forms/assets/fonts` argument for the encoded A2 bank.
- Run the bundle executable on the native Mac CI runner with a 90-second CTest
  timeout. No display simulation or headless substitute establishes this result.

**OBSERVED local checks:** `git diff --check` passes. There is no local AppKit
compiler/runtime on Shadow Windows, so no syntax, link or native pass is claimed.
Native CI, independent coordinator review and the actual output remain required.
This fixture is not a CPU benchmark, installed-consumer proof or public capability
promotion. It does not claim stale-authority coverage independently of the
already tested private adapter; this host fixture deliberately exercises the
callback-exception alternative and valid replacement.

## Exact house-style review

Reviewed the complete new Objective-C++ fixture against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`: explicit initialized types; named retained
callbacks/state; visible owner and borrow lifetimes; readiness and operation
order; real source/layout authority; checked snapshot capacity/conversions;
typed refusal versus presentation; repeated storage/work; and shutdown. Objective-C
message syntax remains at the native test boundary; C++ pointer access uses the
house spelling. Test operations are separate from assertions. No known
house-style violation remains in this exact authored file. Existing test support,
public/core/native host code, AppKit and root-owned build/workflow changes are
not certified by this review.

The single source hash in
`PREPARED_TEXT_MACOS_HOST_FIXTURE_HASHES_2026-10-01.csv` freezes this checkpoint
for coordinator review and native CI.

## First native compile failure and correction

**OBSERVED:** `edfacf3`, workflow run 36960787657, Mac job 110693829397,
failed compiling this new fixture because unqualified `Rect` collided with
Apple's global MacTypes declaration. The consequent override/conversion errors
refer to that same parameter. The signature now spells `gui_forms::Rect`;
its source manifest was refreshed. Native rerun remains required.

The original job log is retained at
`.build/provider-edfacf3-evidence/macos-job.log`. Windows and Linux passed that
run; the Mac profiling steps were skipped after the compilation failure, so
there is no profile result from it. The coordinator separately changed workflow
conditions to attempt the dedicated profiling build even after an unrelated
test failure, and to capture only if that dedicated build succeeds. This cannot
turn a failed native correctness job into a passing one.
