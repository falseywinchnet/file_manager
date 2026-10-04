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
