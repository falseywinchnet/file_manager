# M12-P5 popup ownership and native-callback containment

Status: **MEASURED PARTIAL**. Date: 2026-08-06.

## Reported failure

The Complete Showcase could abort after range-control interaction followed by
navigation to Animation. The manual gesture was intermittent. The supplied
crash report established a C++ exception escaping an AppKit main-queue callback
into `std::terminate`; it did not identify the throwing retained callback.

## Deterministic reproduction

A 48-cycle renderer-free sequence now performs range mutations, navigation to
Animation, scheduled frames, and paint. LLDB stopped on the actual exception:
`Window::register_subtree` rejected a duplicate stable ID such as
`tooltip.5.894.layer` during `ToolTip::show_now`.

The sequence is therefore a regression fixture, not a claim that one human
gesture deterministically caused the crash.

## Root cause and correction

`ToolTip::close_overlay` released its only strong overlay references before
disconnecting the `PopupToken`. `PopupAttachment` deliberately retains weak
control references, so the popup could expire before `Window::close_popup`
detached and unregistered its subtree. Reopening the same target tooltip then
encountered its stale stable IDs.

The provider now disconnects the popup token while its layer and bubble remain
strongly retained, then releases those objects. A separate 32-cycle keyboard
focus tooltip gate proves repeated attach, close, unregister, and reuse.

The portable host session now also contains application exceptions at its
foreign dispatch boundary, clears pointer/drag state, reports
`HostDispatchError::callback_fault`, and continues with later input. AppKit
damage collection, posted-work drainage, and retained drawing callbacks contain
residual C++ exceptions rather than allowing one through an Objective-C block.
Containment is a crash boundary; it does not excuse or hide a retained-model
fault, which remains separately tested and counted.

## Evidence

- `gui_forms_tooltip_tests`: repeated focus tooltip lifecycle passes.
- `gui_forms_showcase_interaction_tests`: all 48 slider-to-Animation cycles
  pass with scheduled frames and paint.
- `gui_forms_host_protocol_tests`: an intentional throwing pointer handler is
  counted and later dispatch succeeds.
- `gui_forms_macos_host_close_tests`: native close/lifetime path passes.
- Address/undefined sanitizer runs of tooltip and host protocol tests pass with
  `ASAN_OPTIONS=detect_leaks=0`.

## Remaining edge

Physical native stress and long wall-clock focus/tooltip churn remain useful.
The deterministic regression now owns the known failure, so another manual
reproduction or user-captured trace is not required for this defect.
