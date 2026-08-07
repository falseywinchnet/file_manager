# M12-P3 presentation pressure and frame-callback faults

Status: **MEASURED PARTIAL**. Date: 2026-08-06.

## Trigger

**OBSERVED:** a supplied macOS 14.8.7 crash report for Complete Showcase
process 83557 records `SIGABRT`/`std::terminate` on the main dispatch queue from
the AppKit wake dispatch source. The report contains no application throwing
frame or exception text, so the exact callback that threw is not established.
The user recalled adjusting controls on Ranges and then selecting Animation.

**MEASURED:** the same native Ranges → three slider mutations → Animation
sequence does not reproduce against the rebuilt application. Normal animation
and showcase interaction gates also pass. The incident nevertheless exposed a
real contract defect: a frame or UI-timer callback exception was allowed to
cross a native timer callback and terminate the process.

## Implemented contract

- `Window::poll_frame_schedule` isolates each UI-timer or retained-surface
  callback. A fault disconnects that request, increments
  `FramePollResult::callback_faults` and `frame_callback_faults`, and continues
  healthy peers. A faulted active surface cannot retry on every native tick.
- AppKit and Win32 scheduler timer callbacks contain any remaining C++
  exception at the native callback boundary, disarm that wake, and emit an
  explicit `scheduled-wake-fault` diagnostic. AppKit also publishes
  `native_callback_faults` in host JSON.
- `Window::paint` returns an exact `PaintReceipt` only after complete backend
  replay and a second surface-epoch/owner check. Resize, scale replacement, or
  retirement entered from backend replay withholds the receipt.
- macOS and Windows hosts copy/present only a valid receipt and acknowledge the
  exact revision/epoch after the native copy succeeds. Duplicate, backward, or
  replaced-epoch acknowledgements are rejected and counted.

## Deterministic pressure results

The renderer-neutral gate injects 100 state mutations, one pointer event, and
eight nested paint requests from the first backend replay command.

- one coherent old receipt completed;
- one later render consumed value 100 directly;
- one paint wake and one input drain were pending at any observation;
- eight nested requests became eight deferred facts, not nested paints;
- the newer receipt presented and the later-arriving older receipt was rejected;
- paint, input, and dispatcher work returned to zero.

Additional gates prove:

- resize during backend replay abandons the candidate and preserves the prior
  presentation revision until one replacement-epoch render;
- 100 mutations while occluded create no wake or paint, exposure creates one
  wake, and one latest-state render returns idle;
- a throwing backend painter issues no receipt and a clean retry succeeds;
- owner retirement at replay abandons its queued key input and issues no
  receipt; and
- one throwing active surface disconnects while a healthy peer continues on
  the same and later scheduler turns.

Affected normal, renderer-free, strict, macOS compile, showcase interaction,
host protocol, and Win64 core gates pass.

## Honest boundary

This closes callback-fault containment and exact synchronous presentation
receipts. It does not yet implement the M12-P2 second candidate raster/swap, an
asynchronous compositor fence, useful native multi-rectangle copies, or a
forced physical input callback inside `drawRect:`/`WM_PAINT`. The supplied
crash report establishes the escaped-exception class but cannot identify the
historical throwing callback without matching symbols or exception text.
