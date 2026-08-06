# DEMO-D6 Criteria evidence

Date: 2026-08-06

Base revision: `28b7b220434d01180ac1781f2ce60c87532bc18f`
(working-tree implementation under review)

Environment: macOS AppKit host, CPU Skia retained renderer, product-only native
application, catalogue `file-manager-demoboard-001`, initial generation 86.

Font pack:

- Portsmouth Rapids regular:
  `88988bea222852c30e08a3629d9f929baacfec4844c85bb978dcc68e6f4d4add`
- Portsmouth Rapids bold:
  `f97d702778f5933b4ae138064a348499a6dba90d9e2e6bf75a6ab81a6730c03a`

## Claims

- **MEASURED** — the complete native suite passes 56/56, including the new
  renderer-free `InstrumentRack` tests and File Manager Criteria contract.
- **MEASURED** — a clean build-local install exports
  `instrument_controls.hpp`; the separate `find_package(GUIForms CONFIG)`
  consumer builds, instantiates choice/text rack fields through
  `GUIForms::Controls`, and exits successfully.
- **MEASURED** — the same public model and controls cross-build as an x86-64
  PE32+ `File Manager Demoboard.exe` with SHA-256
  `bbd6bed2db75b1d19e9126d6ed901dd5f2ac4a294594bf05c3dbeb250ae336de`.
- **MEASURED** — Computer Use can identify and operate every default module
  field, Apply, Add, the bounded Add menu, result objects, and retained
  Selection properties through published native accessibility elements.
- **MEASURED** — Apply advances through the GUI.Forms UI scheduler, rejects
  duplicate activation while pending, completes at generation 87, reduces the
  deterministic result projection from 31 to 5 objects, and preserves
  `fm.object.obj-facade-study` selection.
- **MEASURED** — the narrow physical resize collapses Selection automatically,
  withdraws the ribbon, wraps the third module to a second rack line, and keeps
  the result field reachable.
- **OBSERVED** — the first physical Add-menu run used parent-local coordinates
  and opened near the title. The corrected run uses the owner's absolute
  bounds, opens beneath `+ module`, and has a headless geometry regression
  assertion.

## Captures

| File | SHA-256 | Meaning |
|---|---|---|
| `native-criteria-default.jpeg` | `9a2099692796ee9d99ac30f3bc84a0ebd62a12f8157a5626f036d2a1d560c501` | isolated default Criteria surface |
| `native-criteria-add-menu.jpeg` | `08cf595dcad4be8c935e7e3105955dfd3cbb7b268c3623da4464d0b59fcd3dbd` | corrected bounded Add-menu anchor |
| `native-criteria-compact.jpeg` | `009c886f079467609617558f28c29cff031a3c2cab75aa4f7a5d28d4823a24b3` | physical compact resize and rack wrap |

## Inference boundary

This evidence does not close provider-scale evaluation, native text-range
publication, high-contrast captures, external asynchronous criterion
producers, exact prototype raster matching, or the review-board visual
superset. D6 therefore remains **measured partial**.
