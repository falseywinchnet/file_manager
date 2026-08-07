# GUI.Forms documentation

- [`LIFECYCLE_CONTRACT.md`](LIFECYCLE_CONTRACT.md) — authoritative portable
  initialization, presentation, close, shutdown, retained attachment, and
  compatibility-handle ordering under ADR-014.

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
- [`../planning/WINFORMS_API_CATALOGUE.md`](../planning/WINFORMS_API_CATALOGUE.md)
  — pinned LibreWinForms/.NET 10 member-level oversight totals and honest
  native/partial/missing/excluded states; the generated TSV contains every row.
- [`../planning/WINFORMS_BEHAVIOR_GAP_CATALOGUE.md`](../planning/WINFORMS_BEHAVIOR_GAP_CATALOGUE.md)
  — cross-cutting behavior and visual-superset gaps that API identities alone
  cannot prove.
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
- [`../experiments/M12P17_METADATA_DRIVEN_PROPERTY_GRID.md`](../experiments/M12P17_METADATA_DRIVEN_PROPERTY_GRID.md)
  — exact property value origins plus the native metadata-driven PropertyGrid,
  typed stock editors, reset/validation, hostile callback disposal, and native
  showcase dogfood.
- [`../experiments/M12P18_EXPANDABLE_COMPOUND_PROPERTIES.md`](../experiments/M12P18_EXPANDABLE_COMPOUND_PROPERTIES.md)
  — expandable typed geometry/spacing/color/font fields, precise property-path
  mutation, real retained Reset controls, semantics, and native dogfood.
- [`../experiments/M12P19_NESTED_PROPERTY_VALUES_AND_COLLECTIONS.md`](../experiments/M12P19_NESTED_PROPERTY_VALUES_AND_COLLECTIONS.md)
  — bounded immutable object/collection values, recursive member/index editing,
  atomic collection mutations, stock ComboBox.Items, and native dogfood.
- [`../experiments/M12P20_PROPERTY_CONVERTER_AND_EDITOR_SERVICES.md`](../experiments/M12P20_PROPERTY_CONVERTER_AND_EDITOR_SERVICES.md)
  — instance-owned TypeConverter/editor analogues, retained custom-editor
  ownership, default NumericUpDown inspection, and typed commit dogfood.
- [`../experiments/M12P21_SPECIALIZED_FLAGS_AND_COLOR_EDITORS.md`](../experiments/M12P21_SPECIALIZED_FLAGS_AND_COLOR_EDITORS.md)
  — retained multi-select flags popup, alpha-aware color field/swatch,
  bounded failure routing, and native-neutral portability gates.
- [`../experiments/M11B_MANAGED_HOST_SURFACE_AND_ABI_0_3.md`](../experiments/M11B_MANAGED_HOST_SURFACE_AND_ABI_0_3.md)
  — managed `Application.Run(Form)`, additive ABI 0.3 host projection,
  deterministic headless evidence, and a captured Win32/Wine surface.
- [`../experiments/M11C_MANAGED_CALLBACKS_AND_LOOP_ABI_0_4.md`](../experiments/M11C_MANAGED_CALLBACKS_AND_LOOP_ABI_0_4.md)
  — native-to-managed events, queued live mutation, exception containment,
  cancelable form close, and application-context lifecycle in ABI 0.4.
- [`../experiments/M11E_GUI_DRAWING_CORE_AND_ABI.md`](../experiments/M11E_GUI_DRAWING_CORE_AND_ABI.md)
  — renderer-free GUI.Drawing value/resource/state semantics, independent C
  ABI, deterministic cross-language traces, and the precise M11f boundary.
- [`../experiments/M11H_COMPATIBILITY_CATALOGUE_AND_PAINT_LEASES.md`](../experiments/M11H_COMPATIBILITY_CATALOGUE_AND_PAINT_LEASES.md)
  — the 18,097-row nominal-base catalogue and the first revisioned,
  non-reentrant coherent paint-lease implementation.
- [`../experiments/M12P5_POPUP_CALLBACK_CONTAINMENT.md`](../experiments/M12P5_POPUP_CALLBACK_CONTAINMENT.md)
  — deterministic reproduction and correction of stale popup stable IDs plus
  portable/native foreign-callback exception containment.
- [`../experiments/M12P5_BINDING_CURRENCY_KERNEL.md`](../experiments/M12P5_BINDING_CURRENCY_KERNEL.md)
  — retained record currency, explicit control-property descriptors, two-way
  update modes, format/parse/completion, manager transfer, and lifetime gates.
- [`../experiments/M12P6_VALIDATION_AND_BOUND_ERRORS.md`](../experiments/M12P6_VALIDATION_AND_BOUND_ERRORS.md)
  — focus-driven cancellable validation, AutoValidate policy, automatic
  OnValidation binding, and binding-aware ErrorProvider projection.
- [`../experiments/M12P7_DIALOG_KEYS_AND_MNEMONICS.md`](../experiments/M12P7_DIALOG_KEYS_AND_MNEMONICS.md)
  — renderer-free mnemonic text/routing, validated programmatic clicks, and
  retained accept/cancel dialog commands.
- [`../experiments/M12P8_COMMAND_ARBITRATION_AND_NATIVE_CALLBACKS.md`](../experiments/M12P8_COMMAND_ARBITRATION_AND_NATIVE_CALLBACKS.md)
  — stable duplicate mnemonic arbitration, retained DialogResult/menu behavior,
  and the AppKit dispatch-source exception boundary.
- [`../experiments/M12P9_CONTROL_GEOMETRY_AND_ORDER.md`](../experiments/M12P9_CONTROL_GEOMETRY_AND_ORDER.md)
  — exact geometry flags, masked bounds, preferred/AutoSize sizing,
  direct-child filtering, nested traversal, and coherent topmost-first ordering
  across the retained core and generated facade.
- [`../experiments/M12P15_PROPERTY_METADATA_CENTER.md`](../experiments/M12P15_PROPERTY_METADATA_CENTER.md)
  — native renderer-neutral property descriptors, typed access, defaults,
  reset/serialization policy, tokenized changes, declared effects, and
  initialization/thread/lifetime gates.
- [`../experiments/M12P16_COMPOUND_PROPERTY_VALUES.md`](../experiments/M12P16_COMPOUND_PROPERTY_VALUES.md)
  — geometry/spacing/color/font/image/enum property values, finite shared enum
  schemas, descriptor-aware conversion, and real layout/visual registrations.
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
