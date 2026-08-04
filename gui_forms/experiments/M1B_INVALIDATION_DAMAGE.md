# M1b typed invalidation and bounded damage evidence

Date: 2026-08-04

Status: **OBSERVED implementation evidence for the bounded M1b proving slice**.
This record does not select the unresolved layout observability contract, display
chunk representation, renderer architecture, or public ABI.

## Scope

- **GIVEN**: GUI.Forms remains a retained, renderer-neutral C++20 core with no
  perpetual redraw loop.
- **CANDIDATE**: synchronous geometry read barriers and the four-pass layout
  guard are proving-slice behavior. They are not a compatibility promise.
- **CANDIDATE**: display chunks remain M2 work. M1b reports exactly zero rebuilt
  chunks and separately counts controls visited, callbacks invoked, and paint
  invalidations consumed.

## Implemented substrate

- Typed effects distinguish measure, arrange, paint, hit-test, text, style,
  resource, semantics, and accessibility dirtiness.
- Each retained control caches aggregate subtree dirtiness. Layout walks only
  affected root-to-leaf paths unless a mutation explicitly declares whole-
  subtree impact.
- Re-entrant layout is bounded to four passes and records a structured
  `bounded_pass_limit_hits` diagnostic when it does not quiesce.
- Damage rectangles compact only when their union remains exactly rectangular.
  More than 64 rectangles collapse deterministically to one bounding rectangle;
  compactions, collapses, and maximum rectangle count are metrics.
- Development builds reject undeclared mutation effects. Production builds
  record the diagnostic and conservatively invalidate the subtree.

## Executable observations

The six-control locality fixture observes:

- leaf bounds mutation: three measure callbacks and three arrange callbacks on
  the root-to-leaf path;
- unaffected sibling subtrees: zero additional layout callbacks;
- explicit window resize: six measure callbacks and six arrange callbacks;
- non-quiescing re-entrant arrange: four passes and one pass-limit hit;
- paint-only mutation: zero layout passes, a nonzero consumed-invalidation
  count, and zero display chunks rebuilt;
- exact adjacent damage: two rectangles compact to one without area growth;
- nonrectangular overlap: rectangles remain separate below the complexity cap;
- 65 disjoint rectangles: one deterministic complexity collapse;
- completed paint: no requested frame, wake, layout, paint, present, or callback
  work while idle.

## Verification

Environment: AppleClang 16.0.0.16000026, macOS, arm64 host.

- Renderer-free Debug/Werror configuration (`GUI_FORMS_ENABLE_SKIA=OFF`,
  `GUI_FORMS_ENABLE_MACOS_HOST=OFF`, gallery off): 4/4 tests passed.
- Full Gallery/Skia/AppKit configuration: 7/7 tests passed and
  `GUI.Forms Gallery.app` linked.
- Renderer-free ASan+UBSan Debug/Werror: 4/4 tests passed with
  `ASAN_OPTIONS=abort_on_error=1` and `UBSAN_OPTIONS=print_stacktrace=1`.
- Renderer-free TSan Debug/Werror: 4/4 tests passed with
  `TSAN_OPTIONS=halt_on_error=1`.
- Renderer-free Release/Werror: 4/4 tests passed, including the production
  conservative fallback for undeclared mutation.
- Canonical lifecycle trace replay remained byte-identical to M1a: 42 lines,
  1,959 bytes, SHA-256
  `7fd93035ae50971f7f8fa04f8a0dc3e6164fc8a9bd23978d36ca78ad55d0f99a`.

## Retained negative result

Apple's AddressSanitizer in this environment aborts before test execution when
`ASAN_OPTIONS=detect_leaks=1` is requested: `detect_leaks is not supported on
this platform`. The same ASan+UBSan binaries pass without that unsupported
option. This is an environment limitation, not a passing leak-sanitizer result.

## Unresolved edges

- No throughput or latency claim is made; this slice establishes exact work
  counters and conformance fixtures, not a performance baseline.
- Paint traversal is damage-clipped but does not yet use retained display
  chunks. Chunk construction, reuse, and rebuild accounting remain M2.
- The final inside-update geometry visibility contract remains unresolved.
- Hit-test cache structure and semantic/accessibility projection remain later
  milestones; M1b only carries their typed invalidation effects.
