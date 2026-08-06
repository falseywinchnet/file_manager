# M11h dialogs and host-services closure

Status: **MEASURED PARTIAL M11h** on 2026-08-05.

## Scope

This slice promotes the existing portable `HostServices` contract into the
Win32 host and dogfoods the same contract from the independent Complete
Showcase. It does not introduce native handles into the retained core and does
not claim Linux host completion.

## Implemented behavior

- The Win32 `HostSession` now attaches one owned `HostServices` instance to its
  retained `Window`, matching the headless and AppKit host boundary.
- Win32 reports monitor geometry, clipboard, dialogs, and semantic sound cues
  as protocol-v5 capabilities and publishes their structured counters in the
  final host snapshot.
- Native Win32 adapters cover open, multi-open, save, folder, `MessageBoxW`, and
  `ChooseColorW`. Message buttons/icons/default choices, filters, default
  extension, overwrite prompt, multi-selection, cancellation, custom color
  swatches, and bounded typed results remain behind the portable request/result
  vocabulary.
- Native cursor/capture acknowledgements, UTF-16 clipboard conversion, monitor
  enumeration/scale, and semantic `MessageBeep` routing use no renderer or
  control-specific shortcut.
- AppKit message dialogs now translate physical Escape to the declared cancel
  response inside the nested modal loop and remove the temporary event monitor
  immediately on exit.
- The fourteenth Complete Showcase page exercises all five dialog request
  families, cancellation preservation, clipboard round-trip, monitor query,
  `operation_complete`, and warning cues through `Window::host_services()`.

## Evidence

- Headless showcase interaction drives four typed dialog results, one preserved
  cancellation, UTF-8 clipboard write/read, monitor inspection, and the
  operation-complete cue. The final snapshot is exact: four requests and
  completions, one cancellation, maximum depth one, current depth zero, one
  clipboard write/read, one monitor query, and one sound playback.
- Live AppKit dogfood opens the warning alert, cancels it with physical Escape,
  restores the retained owner, and publishes the cancelled result. The native
  color dialog also cancels with Escape; monitor inspection reports
  `appkit.display.1` at scale 2, and the completion cue reports success.
- `gui_forms_windows_showcase_services_smoke` cross-builds the complete Win32
  showcase, runs it under Wine, activates the host-services page, inspects the
  native monitor, plays a completion cue, opens owned `MessageBoxW` and
  `ChooseColorW` dialogs, cancels both through their real native commands, and
  closes cleanly. The final service snapshot records two requests, two
  completions, two cancellations, maximum modal depth one, current depth zero,
  one monitor query, one sound request/playback, zero rejected host events, and
  shutdown true.

## Open edges

- Linux host services and physical Windows dogfood remain open.
- Win32 `ChooseColorW` does not edit alpha; when alpha is admitted, the adapter
  preserves the caller's initial alpha while editing RGB. A future custom alpha
  affordance requires a separately tested host extension.
- Accepted native file/folder paths need broader Unicode, long-path, device,
  unavailable-volume, and overwrite/error corpora.
- Public Forms-shaped component wrappers, validation/error providers, nested
  application-owned dialogs, DML projection, and the wider modal-window family
  remain separate work.
