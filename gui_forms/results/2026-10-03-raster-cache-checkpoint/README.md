# Windows raster-cache checkpoint — paused 2026-10-03

## Status and scope

**GIVEN:** The owner requested end-of-day pause, commit, and report. The File
Manager goal remains paused. Separately authorized GUI.Forms performance work
is also stopped at this checkpoint. No final build/timing interval was started;
the Games sibling's quiet hold was released. This is unfinished source, not an
approved toolkit revision for adoption or a dogfood release.

Branch: `codex/gui-forms-raster-cache`; base:
`568d7fbde2cc173cb5f857431aec53400ee1b550`.
The branch intentionally excludes paused File Manager PR 33.

## Saved implementation

**OBSERVED:** The private Windows DIB painter increases the gradient cache from
32 to 128 entries while retaining its 16 MiB pixel limit. A new shadow cache
retains up to 64 entries / 4 MiB of uint16 sample arrays. Metadata and allocator
overhead are outside those byte claims. Combined pixel/sample allowance is
20 MiB per painter. Shadow samples distinguish skipped coverage (256) from a
real blend whose rounded alpha is zero. Geometry, scale, and alpha are keyed;
RGB, current background, and clipping are applied during replay. Shadow
allocation failure falls back to the existing scalar path.

No public headers or ABI changed in this patch. No image-sampling or Skia
optimization is included. This Windows result does not establish a macOS
performance improvement.

## Measurements and validation already performed

**MEASURED:** Local Shadow Windows Release build, GCC 16.2, Skia off,
transactional DIB lifecycle fixture; 800 x 500 brush scene, 40 timed warm
iterations. Background reset and pixel comparison are outside the timer.
Games confirmed a quiet interval. These are component brush times, not app
frame rates or end-to-end responsiveness measurements.

| Scene | Baseline 1 p50 / p95 ms | Candidate p50 / p95 ms | Baseline 2 p50 / p95 ms |
|---|---:|---:|---:|
| 42 gradients | 25.3698 / 27.8402 | 0.8455 / 0.8758 | 25.3117 / 26.2521 |
| Shadows | 5.1889 / 6.3015 | 0.9233 / 0.9941 | 5.2356 / 5.7728 |

Baseline gradient runs built all 42 brushes every iteration (1,680 builds).
Candidate warm iterations built none. Raw logs are beside this record.
Their transaction-fault messages are deliberate existing lifecycle injections.

The lifecycle fixture passed earlier candidate builds, including a later
expanded correctness/entry-limit version (about 1.57 s). The final source adds
allocation-failure guards and cold benchmarks that have NOT been rebuilt or
run. Do not describe this checkpoint's complete source as test-passing.

The preserved local baseline executable is
`.build/brush-cache-baseline/gui_forms_windows_dib_lifecycle_tests.exe`, SHA-256
`2A70E3D499E567F52115B64C47E6D1F5859B8689F1B7991F1A0159DE5C57E33A`.
Its instrumented host and initial fixture snapshots remain in that ignored
directory. Timed variant-zero scenes are unchanged; later correctness guards
and cold benchmarks are not in that executable. Export a reproducible baseline
patch from these snapshots before removing local build data.

## Independent source review and known gaps

The visible sibling **Audit File Manager Details against interviews** reviewed
the new ShadowRaster, lookup/build/replay, integration, gradient-cap change,
test-only wiring, and the complete new fixture against
`planning/PROGRAMMING_HOUSE_STYLE.md`. It reported no production defect or
additional house-style violation in that scope. Legacy surroundings were read
for context, not blanket certified. The spelling scanner also reported zero
findings for the host and a temporary .cpp copy of the fixture; scanning alone
does not establish compliance.

Reviewed source SHA-256 values at this checkpoint:

- windows_host.cpp: `F222BC45F1E8CD79C28C7CDBC687E4A5187BF11762033AF13AB708907C4756D6`
- windows_raster_cache_fixture.inc: `D880AC6CDA8F55B808D7A58A03070D915D72E1E35CDC2DE2CC403D1C77735948`

**OPEN P2:** The correctness fixture draws a full-surface gradient after the
shadows before comparing. That overwrites destination alpha and can conceal
an incorrect alpha-zero skip. Add shadow-only comparison on a transparent
backdrop using alpha-one input, both on cache construction and a cache hit.
The allocation-failure case compares scalar fallback with scalar reference
and does not close this gap. This finding is deliberately retained unresolved
at the owner's pause.

Other remaining validation: accepted large brushes forcing sample-byte
eviction, reuse of one populated cache through scale changes, final cold-path
cost measurements, ordinary production host compilation, and native CI.
The inherited gradient path does not catch allocation failure and increments
its pixel accounting before vector insertion; the new shadow failure guarantee
must not be generalized to gradients.

## Resume sequence

1. Obtain owner direction to resume; keep broader File Manager work paused
   unless separately resumed.
2. Close the P2 fixture gap and resource-bound coverage, then review changes.
3. Add explicit Windows CI coverage for DIB lifecycle/frame-store targets.
   They currently require GUI_FORMS_WINDOWS_TRANSACTIONAL_DIB=ON, while the
   ordinary exported SDK uses OFF. Keep export settings intact.
4. Coordinate a maximum of two compiler jobs globally with Games. Build the
   lifecycle target in gui_forms/.build/windows-prepared-dev and ordinary
   gui_forms_application in gui_forms/.build/shadow-windows; run focused tests.
5. Reserve a fresh quiet interval for final warm/cold and reverse-baseline
   measurements; release the hold immediately afterward.
6. Publish a reviewed immutable revision only after validation. Games prefers
   a separately tested narrow backport onto 7b260cf. Current main has seven
   changed public headers versus that pin: a full-revision adoption requires
   a matching SDK and complete Games rebuild, not a DLL swap.

## File Manager delivery position

PR 33 remains open and unmerged at
`eccf4922992471b387c6eab5b959bda3fb35a3d4`. Its push and PR native matrices
(37123511971 and 37123513808) were observed passing before this checkpoint.
The latest published portable release remains `v0.001-alpha.0372733`.
No new installer or File Manager release results from this cache checkpoint.
Thumbnail work, broader preview completion, private prepared-preview activation,
and the larger interview-driven acceptance backlog remain unfinished.
