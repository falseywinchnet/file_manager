# Prepared text: Linux X11 host integration checkpoint

Date: 2026-10-01. **OBSERVED authored source; native Linux CI pending.**

Coordinator assignment transfers only the prepared-ON integration in
`src/host/linux/application/linux_host.cpp`, a focused test and this evidence.
Mac, CMake, CI, public API and the previously tested Skia adapter remain outside
this provider's edit scope. The complete applicable GUI.Forms guardrails and
`planning/PROGRAMMING_HOUSE_STYLE.md` govern this change. Canonical A2 authority,
source/revision and transaction semantics remain unchanged.

## Authored behavior

The guarded `NativeWindow::update` route calls `update_prepared_window` after
existing dispatch/model work and visibility/damage collection. The ordinary OFF
resize/paint/presentation route is retained unchanged inside `#else`.

The ON route synchronizes images, admits a candidate through
`begin_prepared_frame`, and passes the expanded damage to `Window::paint`.
`end_frame` is cleanup only. Only a returned model `PaintReceipt` can commit the
candidate. Image refusal, admission refusal, null receipt, commit refusal and
exceptions abort; exceptions retain host damage and the prior coherent front
instead of escaping the event loop. No fabricated live-only receipt exists.

Reviewed `Window::paint`: it returns no receipt for occlusion, reentrancy,
retirement, empty coverage or changed surface epoch; replay exceptions abandon
the candidate and rethrow. Reviewed `notify_presented`: it checks current
surface epoch, completed rendered revision and monotonically new presentation.
The host does not replace those checks with its own source identity.

`present_prepared_front` reads only the committed front, whose actual dimensions,
scale and receipt remain unchanged on failed resize. The Linux route currently
uses scale 1; other front scales explicitly refuse presentation. Copy bounds
intersect requested damage, actual front extent and current drawable extent
before narrowing to integer pixel coordinates. Nonfinite geometry refuses.
Native image shape, pitch and format are validated before conversion; output
storage is bounded to 64 MiB. The XImage owns its native metadata and borrows the
window's conversion vector only during presentation. RAII clears that borrow
before XDestroyImage, including on exceptions.

Channel masks, shifts, maxima and conversion mode are selected before the pixel
loops. Named native-32 and generic XPutPixel kernels preserve existing channel
conversion behavior. Color arithmetic widens to uint64 before multiplication;
source and destination storage are disjoint. Pixel loops neither allocate nor
discover format. No performance improvement is claimed.

**GIVEN coordinator scope:** retain the existing X11 submission boundary:
`XPutImage`, then `XFlush`, then model notification. Xlib asynchronous server
errors and physical display completion are not newly certified. This slice adds
no process-global X error handler. A failed/null candidate may expose the actual
old front but receives no model acknowledgement. Successful repaint of an
already acknowledged exposure can clear host damage only if exact front epoch,
revision and extent still match the current presented model surface.

## Focused native fixture and build needs

New `tests/linux_prepared_text_host_tests.cpp` uses real `Runtime`/`NativeWindow`
under Xvfb, an A2 prepared control and the exact bundled font directory. It does
not use timer guesses for readiness: it synchronizes the X connection and drains
the initial map/configure events. Server captures use XGetImage after XSync.

Authored checks cover:

- initial prepared ink, coherent front and actual model acknowledgement;
- callback exception and revoked prepared layout preserving front pixels,
  receipt, presented revision and server-visible old pixels;
- oversized admission refusal while current requested size differs from the
  old front, then a real drawable resize whose stale layout also refuses;
- valid replacement recovering with a newly sized front and receipt;
- model occlusion returning no receipt while previous exposure receives no new
  acknowledgement;
- partial repaint changing its requested area while preserving unaffected rows.

Root CMake needs target `gui_forms_linux_prepared_text_host_tests` from that
source only when Linux host and prepared text are enabled; link
`gui_forms_host_linux`, `gui_forms_prepared_text` and `X11::X11`. The test accepts
one absolute `gui_forms/assets/fonts` path and sets `GUI_FORMS_FONT_DIR` itself.
Run under the native Linux CI Xvfb environment, for example:

```text
xvfb-run -a <build>/gui_forms_linux_prepared_text_host_tests <checkout>/gui_forms/assets/fonts
```

No local Linux compile/link/runtime result is claimed on Shadow Windows.
`git diff --check` passes; that is formatting evidence only. Native CI and
independent coordinator review are required before this integration is accepted.
No installed SDK, document-view availability, latency or CPU improvement follows
from the source checkpoint.

## Exact manual house-style review

Reviewed every authored guarded helper and update-routing hunk in
`linux_host.cpp`, and the complete new test, against all sections of
`planning/PROGRAMMING_HOUSE_STYLE.md`: explicit initialized types; named callback
execution; native metadata ownership and vector/pixel borrow lifetimes; model
receipt versus front identity; operation and cleanup order; checked/narrowed
geometry, pitch, capacity and color arithmetic; failure states; and invariant
mode/channel selection before repeated pixel work. Test actions are separate
from assertions and use generated fixture state. No known violation remains in
this exact authored scope. Existing OFF host methods, accessibility, public/core
code, test support, Skia, Xlib and root-owned changes are not newly certified.

The two source hashes in `PREPARED_TEXT_LINUX_HOST_HASHES_2026-10-01.csv` freeze
this provider checkpoint for review and native CI.
