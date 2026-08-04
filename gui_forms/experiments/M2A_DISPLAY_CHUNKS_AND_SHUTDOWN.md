# M2a display chunks, paint planes, and shutdown evidence

Date: 2026-08-04

Status: **OBSERVED implementation evidence for the bounded M2a proving slice**.
This record does not select the M2 renderer, scheduler, raster-cache policy, or
public serialization format.

## Scope and evidence labels

- **GIVEN**: GUI.Forms remains retained-mode. Controls retain runtime state and
  invalidation requests work; the host does not run a perpetual redraw loop.
- **GIVEN**: the public core remains renderer-free. No Skia or AppKit type enters
  the display-chunk interface.
- **CANDIDATE**: one immutable command chunk per visible control is the current
  proving representation. It is not a selected scene graph or wire format.
- **CANDIDATE**: backplane, control, and overlay are the bounded three-plane
  paint contract for this slice. Later composition policy remains unresolved.

## Implemented substrate

- A renderer-neutral recording painter captures complete local control output
  as immutable commands: save/restore, translation, clip, fill, stroke, line,
  text, and image operations.
- Each chunk records a monotonic window-local generation, paint plane, logical
  bounds, and command count. Bounds or plane incompatibility forces rebuild.
- Paint invalidation rebuilds only the affected control chunk. Exposure can
  replay compatible chunks without invoking control paint callbacks.
- Damage is stored independently for backplane, control, and overlay. Painting
  replays those planes in that order, independent of visual insertion order.
- Plane migration damages the control's old and new planes and rejects the old
  cached chunk. Detach and disposal remove affected retained cache entries.
- Metrics distinguish chunks rebuilt, chunks reused, commands replayed, cache
  entries, and current display generation. The Gallery diagnostics surface the
  cache and rebuild/reuse values.

## Executable observations

The focused display-chunk fixture proves:

- three controls inserted in reverse plane order still replay backplane,
  control, then overlay;
- a full exposure reuses three compatible chunks and invokes zero control paint
  callbacks;
- localized invalidation advances exactly one generation and rebuilds one of
  three chunks while reusing the other two;
- plane-specific damage can be taken without consuming damage on other planes;
- forged out-of-range paint-plane values are rejected without mutating cache
  state or consuming valid pending damage;
- plane migration damages both the former and destination planes and clears the
  incompatible cache;
- child disposal reduces cache occupancy and the completed window returns to
  zero requested frame, wake, layout, paint, present, or callback work.

The 44-control Gallery interaction fixture observes a localized checkbox
mutation rebuilding fewer chunks than the retained control count, with every
consumed paint invalidation accounted for by a rebuilt chunk and nonzero replay
work.

## Close-crash reproduction and correction

- **OBSERVED negative result**: the supplied macOS crash report terminates in
  `objc_release` while the autorelease pool is drained after `run_macos`.
- **OBSERVED reproduction**: a new host-close test that performs an AppKit
  `performClose:` after launch produced the same `SIGSEGV` before the fix.
- **HYPOTHESIS confirmed by the reproduction**: ARC retained a strong local
  `NSWindow *` while AppKit's default `releasedWhenClosed` ownership also
  released the window on close.
- **OBSERVED correction**: the host sets `releasedWhenClosed` to `NO`, then
  explicitly clears the window delegate and content view after the application
  run loop returns.
- The corrected native close test passed ten consecutive Debug executions and
  one AddressSanitizer-instrumented execution.

## Verification

Environment: macOS arm64, AppleClang 16.0.0.16000026.

- Full Gallery/Skia/AppKit Debug: 12/12 tests passed.
- Renderer-free Debug/Werror: 8/8 tests passed.
- Renderer-free ASan+UBSan/Werror: 8/8 tests passed with leak detection disabled
  because this Apple AddressSanitizer does not support it.
- Renderer-free TSan/Werror: 8/8 tests passed.
- Renderer-free Release/Werror: 8/8 tests passed.
- AppKit/Skia host close under ASan: 1/1 test passed.
- Canonical lifecycle trace remained byte-identical to M1a: 42 lines, 1,959
  bytes, SHA-256
  `7fd93035ae50971f7f8fa04f8a0dc3e6164fc8a9bd23978d36ca78ad55d0f99a`.

## Retained negative results

- Applying global `-Werror` to the host-sanitizer build promotes GNU variadic
  macro warnings in pinned third-party Skia headers. Project code remains under
  Werror in the renderer-free matrices; the host ASan gate omits global Werror.
- Combining UBSan with the prebuilt no-RTTI Skia archives introduces unresolved
  `typeinfo` symbols. The host close gate therefore uses ASan alone; UBSan still
  covers the renderer-free core.

## Unresolved edges

- No throughput, latency, memory, or frame-budget claim is made. M2a supplies
  truthful counters and deterministic conformance fixtures, not a benchmark.
- Paint currently traverses the retained tree once per plane during full
  exposure. Plane masks, scheduler coalescing, and cache admission/eviction are
  later M2 work.
- Chunks retain high-level text and image requests; resource identity,
  shaped-text runs, raster cache generations, and GPU upload policy are not
  defined here.
- PNG golden comparison, renderer alternatives, backend selection, and the M2
  renderer decision record remain open gates.
