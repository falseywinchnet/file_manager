# GUI.Forms documentation

GUI.Forms is a portable retained GUI library under active construction. These
documents describe the intended consumer boundary without promoting incomplete
spike APIs to a frozen compatibility promise.

## Start here

- [`LIBRARY_AND_ASSEMBLY_GUIDE.md`](LIBRARY_AND_ASSEMBLY_GUIDE.md) — package
  roles, the managed facade direction, trusted retained-subtree extensions,
  ownership, lifecycle, and the host-owned visual contract.
- [`CURRENT_API_REFERENCE.md`](CURRENT_API_REFERENCE.md) — implemented C++ and
  experimental C ABI 0.x types, behavior, event order, and explicit gaps.
- [`WINDOWS_WINE_HOST.md`](WINDOWS_WINE_HOST.md) — PE64 build, Win32/Wine host,
  stable-ID instrumentation, measured smoke result, and accessibility boundary.
- [`../planning/CONTROL_COMPLETENESS_MATRIX.md`](../planning/CONTROL_COMPLETENESS_MATRIX.md)
  — the authoritative support/defer/exclude ledger for control families.
- [`../planning/MASTER_IMPLEMENTATION_PLAN.md`](../planning/MASTER_IMPLEMENTATION_PLAN.md)
  — dependency order and release gates.
- [`../planning/GUI_DRAWING_REVISION_PLAN.md`](../planning/GUI_DRAWING_REVISION_PLAN.md)
  — captured `System.Drawing` floor, GUI.Drawing ownership/cutover stages, and
  broader File Manager drawing oversight.
- [`../experiments/WINDOWS_ORACLE_PREFLIGHT.md`](../experiments/WINDOWS_ORACLE_PREFLIGHT.md)
  — measured Wine/.NET/MinGW environment and the original Windows-host handoff.
- [`../experiments/CAPTURE_0_STATIC_COMPATIBILITY_MANIFEST.md`](../experiments/CAPTURE_0_STATIC_COMPATIBILITY_MANIFEST.md)
  — deterministic non-executing retired compatibility specimen metadata/IL-operand capture, privacy
  boundary, and measured authoritative manifest.
- [`../experiments/CAPTURE_1_DISPOSITION_AND_LOADER_LAB.md`](../experiments/CAPTURE_1_DISPOSITION_AND_LOADER_LAB.md)
  — exhaustive disposition closure, generated facade catalogue, and measured
  strong-reference interception under host .NET and Wine.
- [`../experiments/M11A_GENERATED_SURFACE_AND_ABI_0_2.md`](../experiments/M11A_GENERATED_SURFACE_AND_ABI_0_2.md)
  — 796/796 compiled facade identities and the first native ABI-backed managed
  control/property/tree/disposal spine on host .NET and Wine.
- [`../experiments/M11B_MANAGED_HOST_SURFACE_AND_ABI_0_3.md`](../experiments/M11B_MANAGED_HOST_SURFACE_AND_ABI_0_3.md)
  — managed `Application.Run(Form)`, additive ABI 0.3 host projection,
  deterministic headless evidence, and a captured Win32/Wine surface.
- [`../experiments/M11C_MANAGED_CALLBACKS_AND_LOOP_ABI_0_4.md`](../experiments/M11C_MANAGED_CALLBACKS_AND_LOOP_ABI_0_4.md)
  — native-to-managed events, queued live mutation, exception containment,
  cancelable form close, and application-context lifecycle in ABI 0.4.
- [`../experiments/M11E_GUI_DRAWING_CORE_AND_ABI.md`](../experiments/M11E_GUI_DRAWING_CORE_AND_ABI.md)
  — renderer-free GUI.Drawing value/resource/state semantics, independent C
  ABI, deterministic cross-language traces, and the precise M11f boundary.
- [`../experiments/M4A_UNICODE_TEXT_STORE.md`](../experiments/M4A_UNICODE_TEXT_STORE.md)
  — strict UTF-8, typed positions, atomic edits, Unicode line indexing, style
  spans, and deterministic corpus evidence.
- [`../experiments/M4B_GRAPHEME_SHAPING_SEAM.md`](../experiments/M4B_GRAPHEME_SHAPING_SEAM.md)
  — Unicode 17 extended grapheme conformance, typed cluster navigation, and the
  renderer-neutral shaping/fallback contract.

## Vocabulary

In this project, “assembly” can mean either a future managed facade assembly or
a compiled data-only theme/language assembly. It never means that the native
GUI.Forms runtime depends on .NET. The library guide keeps those meanings
separate.
