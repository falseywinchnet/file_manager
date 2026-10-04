# Native preview paint measurement

**CANDIDATE measurement, native pixel control rejected.** This standalone macOS
consumer links the normal installed `GUIForms::Application` SDK. It changes no
provider source, application behavior, preview format admission or deployment.
It uses public GUI.Forms APIs and AppKit display/snapshot calls, with no private
host selectors or renderer access.

The question is whether the admission-only comparison survives the additional
work of synchronizing image resources, decoding or copying into the renderer,
and painting through the native host. Source observations place that work in
the host's `drawRetainedRect` path; the experiment times the complete forced
display call rather than assigning uninstrumented time to individual internals.

## Declared workload

- The same generated solid-color controls as `../preview_admission`: 1024 by
  64/256/1024, opaque RGB `(12,226,198)`, raw BGRA versus stored and deflated PNG.
  All nine source buffers are prepared before host startup and remain owned by
  the experiment. This is not a photo corpus or a source-I/O benchmark.
- One retained PictureBox fills a 640 by 480 DIP native client area and uses
  stretch mode for equal destination work. The actual backing scale and renderer
  name are recorded. The initial callback rejects an occluded window or a client
  extent other than 640 by 480; this is not continuous geometry monitoring.
- Each format/geometry is warmed once. Warmups independently inspect nine
  interior native snapshot samples for the expected sRGB color, then clear the
  image and require those samples to stop matching. The tolerance is 0.02 per
  color channel and opacity must exceed 0.98. This is sampled fixture evidence,
  not a complete image-equivalence test.
  The compatible AppKit destination is tagged sRGB before drawing into it;
  captured pixels are never retagged afterward to satisfy the predicate.
  After capture the probe verifies that profile and packed 8-bit, four-channel,
  integer RGBA layout, then normalizes `getPixel` samples by 255. The opaque
  fixture does not distinguish premultiplied from straight-alpha storage.
- Thirty samples per format/geometry cover all six format orders five times.
  A second fresh process reverses geometry order. This balances that declared
  order effect; it is not a collection of independent statistical trials.

Five intervals are reported separately:

1. The synchronous public image admission call.
2. Admission-result validation, PictureBox binding and forced AppKit display.
3. The combined admission-through-display interval, measured directly rather
   than adding separately calculated percentiles.
4. A forced display of the same retained image, without a resource mutation.
5. PictureBox clear, registry removal and a forced blank display.

Each display must advance public retained-paint and host-presentation counts
by exactly one. After removal, the ID must be stale and logical registry
resource/byte counters must return to zero. Samples and percentile scratch
arrays are fixed storage; source preparation, snapshots, metric reads,
assertions after each display, sorting and reporting are outside timed spans.
Each warmup and sample has its own autorelease pool, drained after its timed
spans; pool drainage time is excluded and physical memory release is unproven.
The harmless admission success check is inside the bind/display and combined
intervals. Nearest-rank p50/p95, maximum and every raw sample are retained.
No speed threshold is asserted.

## Interpretation limits

`displayIfNeeded` is deliberately forced in one owning-thread callback. The
interval includes host image synchronization and native drawing, but excludes
normal event-loop waiting, file selection, source reads, provider IPC, desktop
composition and physical screen scan-out. It is not end-to-end latency or a
claim that ordinary application input stays responsive during the experiment.

Retirement includes a renderer synchronization opportunity, but neither logical
registry counters nor cleared pixels prove native allocator release, physical
memory residency or peak overlap. This workload retires an image before
admitting the next; replacement overlap needs a separate experiment. Native
snapshotting may itself draw; its control draws are outside the measured spans.
The solid fixture is highly compressible. A favorable result here does not
select a transport, provider or raw-versus-encoded product strategy.

## Running

Generate a fresh fixture directory with `../preview_admission/fixture.py`.
Configure this project with an installed macOS Application SDK and absolute
`PREVIEW_PAINT_FIXTURES` directory, then build and run CTest. The two tests have
60-second process limits. CI runs this consumer against the normal SDK before
enabling source-only provider development options, and preserves the verbose
log. No install rules or application integration are introduced.

The complete programming house style governs this source, CMake and workflow
scope. Scanner, independent source review and native results will be recorded
with their exact scope; they do not certify unchanged GUI.Forms or dependencies.

## First native result

At source `e898c35c`, both native processes compiled and reached the Skia CPU
host at scale 1, but the warmup pixel check failed before collecting timing
samples. The original error did not distinguish image presence from retirement,
format or sampled color. This is a rejected measurement, not evidence of faster
previews or a diagnosed application bug. The next run records the exact warmup,
expected state, sampled color, control/image bounds and a failure PNG without
changing the pixel tolerance or accepting missing output. Rejected logs are
retained in `../../results/2026-10-04-preview-paint/`.

## Display-profile diagnostic and next control

**OBSERVED:** at `5050e675`, both processes failed during their first BGRA
warmup, with correct 640 by 480 control/image bounds. The first sample converted
to sRGB was `(0, 0.898072, 0.818158, 1)`. The retained failure PNG carries the
runner's `Display` / `Apple Virtual` ICC profile. Reading its stored sample
through Windows System.Drawing without enabling embedded color management gave
RGBA `(13,226,198,255)`, close to the fixture bytes. The log and forward PNG are
retained as `MacDisplayProfileRejectedLastTest.log` and
`MacDisplayProfileRejected.png` in the results directory.

**HYPOTHESIS:** the display-dependent snapshot destination makes this control
compare values from different color spaces. The next control gives the compatible
AppKit bitmap an explicit sRGB profile **before** `cacheDisplayInRect` draws into
it and verifies that destination profile. Apple documents that callers initialize
the destination configuration before this
[snapshot operation](https://developer.apple.com/documentation/appkit/nsview/cachedisplay%28in%3Ato%3A%29?language=objc).
No captured samples, fixture colors, tolerance or renderer implementation change.
This does not yet diagnose the renderer or establish color fidelity in the normal
desktop presentation path. Native execution must still pass the unchanged
presence and retirement checks before any timings are accepted.

**REJECTED control:** at `f5d1ca9e`, pre-draw sRGB tagging alone still failed
the NSColor-based check with `(0,0.898066,0.818159,1)`. The saved PNG now has an
explicit sRGB chunk, no ICC chunk, and stored RGBA `(12,226,198,255)` at the same
sample. Primary inspection used System.Drawing without embedded color management
and a standard-library PNG chunk listing; these are independent reads of the
retained file, not evidence about screen scan-out. The failure log and PNG are
preserved as `MacSrgbColorObjectRejectedLastTest.log` and
`MacSrgbColorObjectRejected.png`.

The next control therefore reads the verified sRGB bitmap's numeric samples
directly through AppKit's documented
[`getPixel:atX:y:`](https://developer.apple.com/documentation/appkit/nsbitmapimagerep/getpixel%28_%3Aatx%3Ay%3A%29?language=objc)
interface. The expected color, tolerance, opacity threshold and nine-point
presence/retirement checks remain the same. It rejects unsupported sample
formats before reading a four-component array. Raw channels and the independent
NSColor conversion/profile are both logged for the first sample of each capture,
outside timing. **HYPOTHESIS:** NSColor's intermediate representation introduces
the observed discrepancy. That mechanism is still unconfirmed; the validated
bitmap samples, rather than a relabeled capture or a relaxed threshold, are the
new oracle. This changes the sample-reading method and must pass native execution
before measurements are accepted.
