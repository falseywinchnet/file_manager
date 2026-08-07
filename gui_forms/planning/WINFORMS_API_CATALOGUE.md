# WinForms and drawing API oversight catalogue

Status: **OBSERVED upstream inventory; conservative implementation ledger**.

LibreWinForms source: `wieslawsoltes/LibreWinForms` branch `librewinforms-progpu-port`, commit `1457ed5beef24a7d7e3688a4725c7487da09791e`. Forms rows come from its `src/System.Windows.Forms/PublicAPI.Shipped.txt`. Drawing rows come from the pinned local .NET 10.0.3 reference XML. The generated TSV is the member-level authority; this summary is navigation.

A `partial_native` row means only that a corresponding retained GUI.Forms/GUI.Drawing type exists. `measured_partial` identifies an exact behavioral center with named evidence while retaining its stated gaps. Neither status claims that every overload, exception, event order, accessibility projection, or raster result is complete. `missing` is the default for admitted calls without direct evidence.

## Totals

| Surface | Rows | Admitted | Excluded |
|---|---:|---:|---:|
| `System.Drawing` | 3522 | 3050 | 472 |
| `System.Windows.Forms` | 14575 | 13310 | 1265 |

## Honest current states

| Surface | State | Rows |
|---|---|---:|
| `System.Drawing` | `excluded` | 472 |
| `System.Drawing` | `measured_partial` | 1 |
| `System.Drawing` | `missing` | 1875 |
| `System.Drawing` | `package_review` | 19 |
| `System.Drawing` | `partial_native` | 1138 |
| `System.Drawing` | `platform_extension_partial` | 17 |
| `System.Windows.Forms` | `excluded` | 1265 |
| `System.Windows.Forms` | `measured_partial` | 494 |
| `System.Windows.Forms` | `missing` | 9850 |
| `System.Windows.Forms` | `partial_native` | 2924 |
| `System.Windows.Forms` | `platform_extension_review` | 42 |

## Families

| Surface | Family | Rows |
|---|---|---:|
| `System.Drawing` | `drawing_misc` | 581 |
| `System.Drawing` | `fonts_text_glyphs` | 198 |
| `System.Drawing` | `graphics_state_commands_targets` | 305 |
| `System.Drawing` | `images_pixels_color_adjustment` | 1036 |
| `System.Drawing` | `paint_gradients_textures` | 480 |
| `System.Drawing` | `paths_transforms_regions_strokes` | 386 |
| `System.Drawing` | `values_geometry_color` | 536 |
| `System.Windows.Forms` | `accessibility_semantics` | 416 |
| `System.Windows.Forms` | `application_window_control_kernel` | 821 |
| `System.Windows.Forms` | `collections_binding_virtualization` | 3733 |
| `System.Windows.Forms` | `commands_popups_guidance` | 1699 |
| `System.Windows.Forms` | `controls_components_misc` | 5032 |
| `System.Windows.Forms` | `dialogs_host_services` | 595 |
| `System.Windows.Forms` | `input_transfer` | 479 |
| `System.Windows.Forms` | `layout_scrolling` | 779 |
| `System.Windows.Forms` | `style_resources_owner_draw` | 326 |
| `System.Windows.Forms` | `text_editing` | 695 |

## Policy boundary

The base excludes ActiveX/browser hosting, printing, MDI, design-time services, classic `DataGrid`, obsolete menu/toolbar/status families, metafile playback, and arbitrary desktop capture. Raw HWND/HDC members are platform-extension work behind opaque leases, never portable public types. PNG is the renderer-core codec; other codec/metadata families remain package review.

Everything else is an admitted oversight row, not an implementation promise. Promotion to `native` requires a named test or runtime trace for the exact behavioral center; generated identity resolution alone is facade evidence.

## Regeneration

See `tools/winforms_catalogue/README.md`. Regeneration is deterministic for the pinned inputs and writes `planning/generated/WINFORMS_API_CATALOGUE.tsv` plus this summary.
