# M12P30 — Deterministic typography and scale

Date: 2026-08-13  
Status: **MEASURED PARTIAL** — portable resolution/metric contract, native
macOS dogfood, native Windows build, pinned-pack policy, and headless scale
corpus are green. A retained 2× native raster corpus and authoritative profile
goldens remain open.

## Question and boundary

Can a retained GUI.Forms control distinguish its committed effective
`FontSpec` from the primary bundled face and fallback runs that the terminal
text engine actually selected, use the same logical metrics for measurement
and painting, and snap only final baselines at device scale?

This tranche is limited to deterministic typography and text scale. It does
not select Carlito as File Manager's final body face, admit a new raster
profile, implement the rest of FM-R07, or claim browser/native pixel equality.
The Stack 1 House Composite uses Portsmouth Rapids for title/control, Carlito
for content, Cousine for terminal/path, then bounded Noto Sans CJK JP and Noto
Emoji fallback. Carlito remains the consumer profile's lead **CANDIDATE**.

## Public contract

**OBSERVED:** GUI.Forms previously retained a renderer-neutral `FontSpec` but
recording controls generally measured through a scalar-width estimate while
the Skia terminal shaped a different, private set of runs. Inspection could
truthfully show the request, but not the selected face or logical line metrics.

The new renderer-neutral contract is:

| Record | Meaning |
|---|---|
| `FontSpec` | effective role, logical size, weight, italic, and tracking request after window text scale |
| `ResolvedTextLayout::primary_family` | family read from the bundled primary face actually admitted by the provider |
| `ResolvedFontRun` | UTF-8 source range plus actual bundled face family, registered weight/style, and fallback flag |
| logical metrics | shaped width, line box, ascent, descent, and line gap, independent of display scale |
| `snap_text_baseline` | final logical-to-device placement rounding only; it never mutates metrics or identity |

`TextMetricsProvider` is non-owning and portable. Public headers contain no
HarfBuzz, FreeType, Skia, AppKit, or Win32 identity type. A host installs the
provider for its raster lifetime and detaches it before destroying either
object. Headless tests may inject an exact deterministic provider. No provider
returns an explicit `estimated` record with no fabricated family or runs.
Invalid UTF-8/`FontSpec`, a missing primary face, and missing cluster coverage
have separate statuses.

The Skia provider maps actual HarfBuzz run face IDs back to metadata recorded
at bundled-face registration. It does not infer fallback from a requested
role. The Windows provider is enabled only after all twelve private font files
load and it verifies the family returned by `GetTextFaceW`; otherwise the
Window provider remains unavailable.

## Demoboard corpus

`gui_forms_typography_scale_lab` and its Windows counterpart contain:

- Portsmouth title (17/700) and compact control (10/700) specimens;
- Carlito body (12/400), caption (8/700), italic, and tracking specimens;
- Cousine terminal/path content (10/400);
- `Report 日本語 · launch 🚀 · Журнал` for bounded per-cluster fallback;
- a long essential label that wraps rather than clips;
- visible baselines, actual line metrics, resolution state, primary family,
  run count, and logical/device baseline placement;
- 100%, 125%, 150%, and 200% text-scale controls. At 150% and 200% the
  specimen region reflows to two columns while every font role remains fixed.

The right-hand `VisualInspectorView` targets the fallback specimen. Its public
snapshot keeps text redacted by default while preserving byte count, effective
`FontSpec`, exact resolved families/runs, and bounds.

## Reference hierarchy comparison

**OBSERVED:** the accepted HTML/CSS specimens use a 12 px Carlito body, 16–17
px high-emphasis titles, 10 px commands/body metadata, and 8–9 px tracked
captions. `VISUAL_CONSTRUCTION_SPECIFICATION.md` defines Portsmouth title and
control roles, selected bundled humanist body/object and metadata roles, and a
bundled monospace terminal/path role. The lab represents the same hierarchy:
17/700 title, 10/700 compact control, 12/400 body, 8/700 tracked caption, and
10/400 monospace. Unlike the browser reference's permissive CSS family list,
native product resolution never consults an arbitrary host-family fallback.

## Measurements

Environment: M4 Mac mini, arm64 Release build, pinned HarfBuzz 14.2.1,
FreeType 2.14.2, and the exact-hash Gallery font pack. Corpus was the 31-byte
mixed-script string `Report 日本語 · launch 🚀` at Carlito content 12/400.

| Text scale | logical size | primary | shaped runs | missing clusters |
|---:|---:|---|---:|---:|
| 100% | 130.797 × 18.000 | Carlito | 4 | 0 |
| 125% | 160.734 × 23.000 | Carlito | 4 | 0 |
| 150% | 195.688 × 27.000 | Carlito | 4 | 0 |
| 200% | 261.578 × 35.000 | Carlito | 4 | 0 |

At 100%, ascent was 14.000 and descent 4.000. The four actual runs included
Carlito, Noto Sans CJK JP, and Noto Emoji. Integer FreeType hinting means line
height is not assumed to be a mathematically exact multiple; each scale is
shaped and measured.

Headless geometry tests prove the logical result is identical at device scales
1× and 2×, while a logical baseline of 17.25 snaps to 17.0 at 1× and 17.5 at
2×. This is a geometry/metric claim, not a pixel-raster claim.

## Commands and results

Mac native/model targets:

```text
m4build -- cmake --build gui_forms/build --target
  gui_forms_typography_scale_lab
  gui_forms_typography_scale_lab_tests
  gui_forms_harfbuzz_font_engine_tests
  gui_forms_visual_inspection_tests --parallel 10
```

**MEASURED:** all targets built. Focused tests passed:

- `gui_forms_typography_scale_lab_tests`
- `gui_forms_harfbuzz_font_engine_tests`
- `gui_forms_text_shaping_tests`
- `gui_forms_visual_inspection_tests`
- `gui_forms_gallery_font_policy`
- `gui_forms_typography_scale_lab_font_policy`
- `gui_forms_host_boundary_audit`
- `gui_drawing_renderer_boundary_audit`

Windows cross build:

```text
configure_windows_mingw_m4.sh gui_forms/.build/windows-x64-skia-make \
  gui_forms/.build/skia-windows-mingw
cmake --build gui_forms/.build/windows-x64-skia-make \
  --target gui_forms_typography_scale_lab_windows --parallel 10
```

**MEASURED:** `GUI.Forms Typography + Scale Lab.exe` linked successfully.
This is build evidence, not Windows raster dogfood.

Both native lab bundles pass the existing exact SHA-256 font-policy manifest,
including Portsmouth Rapids regular/bold, Carlito four styles, Cousine four
styles, Noto Sans CJK JP regular, and Noto Emoji regular.

## Native dogfood

**MEASURED:** Screen Sharing dogfood ran on the arm64 M4 Mac mini with macOS
26.5 (25F71), Skia CPU m152, HarfBuzz 14.2.1, FreeType 2.14.2, and the native
lab's reported display scale 1.000000×. The bundle was launched with
`/usr/bin/open` from Terminal inside the logged-in remote desktop. The unique
temporary `CodexRuns/typography-scale-item2-20260813.app` symlink was removed
after the lab was closed.

- At 100% and 125%, the retained specimen column kept the title/control/body/
  caption/italic/mono/fallback hierarchy and every row reported `exact`.
- At 150% and 200%, specimens reflowed to two columns. The italic specimen
  wrapped to two lines and the essential long label to three lines at 200%; no
  authored specimen content or caption clipped.
- Actual-size Screen Sharing review made the small diagnostic text legible.
  Portsmouth Rapids, Carlito, and Cousine were visibly distinct; Japanese,
  monochrome rocket emoji, and Cyrillic clusters were present in the mixed
  line. Its diagnostic reported `exact Carlito 5r A28/D8 B52.0>52.0 @1.0x`.
- The representative inspector row reported content 12/400, 15/400, 18/400,
  and 24/400 as text scale changed, with `exact` Carlito resolution. The first
  pass incorrectly chose the specimen caption's control 8/700 operation;
  dogfood rejected that behavior and the inspector now deterministically picks
  the largest committed text operation. A focused test preserves the fix.
- Shrinking the native window to its 720-logical-pixel minimum height at 200%
  preserved every specimen and the inspector footer. Active/inactive switching
  dimmed and restored native chrome without changing text role or geometry.

The Mini exposed only a 1× device profile during this pass. Actual-size visual
review qualifies the observations above for that named profile, but no PNG was
promoted to an authoritative golden and no 2× native pixel claim is made.
Unrelated macOS Login Items notifications obscured the far-right portion of
the inspector in some captures; the row was visible before the overlay and its
complete value is covered by the headless inspection test.

## Remaining limits

- Portsmouth distribution remains evaluation-rights constrained.
- Carlito is the accepted House Composite specimen but remains a File Manager
  body-face **CANDIDATE**, not a final program decision.
- Noto Emoji is the bounded monochrome fallback already admitted for this
  showcase proof; color emoji raster policy remains open.
- Windows uses its existing Uniscribe/GDI terminal rather than the shared
  HarfBuzz raster path. The new diagnostic verifies the complete private pack
  and actual selected family, but Windows native visual parity remains open.
- This does not complete decoration, selection, caret, ellipsis, or
  cluster-hit geometry from FM-R07.
- No browser screenshot is a native text golden. A retained 2× native raster
  corpus and authoritative profile goldens remain open.
