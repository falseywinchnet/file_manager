# M11h — form focus, dialog keys, and owned-form lifecycle

Status: **MEASURED PARTIAL**, 2026-08-05. This bounded slice processes retained
form semantics and dialog-key routing. It does not claim independent modeless
native top-level windows, complete validation, mnemonics, or popup focus scopes.

## Implemented behavior

- **OBSERVED / corrected:** generated `BeginInvoke` and control routing already
  use the retained dispatcher, but forms had no preview stage for dialog keys.
  Experimental Forms ABI 0.18 appends a tokenized form key-preview callback.
  Preview runs before focused-descendant routing and a handled callback stops
  further delivery.
- **IMPLEMENTED:** `ContainerControl.ActiveControl` is scoped to its descendant
  tree. Recursive tab traversal is stable by `TabIndex` and insertion order,
  skips disabled, invisible, and noninteractive controls, and supports reverse
  traversal.
- **IMPLEMENTED:** generated `Button` implements `IButtonControl` with
  `DialogResult`, `NotifyDefault`, and `PerformClick`. Accept and cancel
  activation raises Click before applying the dialog result. Setting a dialog
  result only requests closure for a currently modal form.
- **IMPLEMENTED:** owned-form relationships are bidirectional and reject cycles.
  Modal entry disables its owner, clears owner focus, and restores the prior
  enabled and focus state at modal exit. Retained-hosted modeless forms support
  cancellable close reasons, a single closed notification, deterministic detach,
  and focus restoration.

## Measured gates

- Renderer-free native ABI tests pass 24/24; the Skia-enabled suite passes
  28/28; all 22 cross-built PE64 executables pass under Wine.
- The generated facade verifies `checked=1104 missing=0` and builds with zero
  warnings.
- Host and Wine headless form fixtures report:
  `form-semantics=tab:nested|accept:ordered|cancel:ordered|active-control:scoped|owner:acyclic|modal:focus-restored|label-focus:rejected`.
- Host and Wine secondary-form fixtures report:
  `secondary-form=attached:true|owned:true|clamped:true|reopened:true|cancelled:true|detached:true|focus-restored:true|loads:1|closed:1`.
- A visible Wine fixture received physical HID Enter and Escape through the
  Win32 host. It reported
  `dialog-key-live=enter:1|escape:1|focused-field:true|result:Cancel`.

## Open edge

The modeless secondary form fixture is an honest retained-hosted composition,
not yet a second native top-level window. The next compatible closure should
give modeless forms independent native hosts and explicit owner activation,
then reuse the same focus/close contracts for popup and dropdown focus scopes.

## ABI 0.19 cursor and dock-padding follow-through

- **OBSERVED / corrected:** retired compatibility specimen telemetry reached `Cursors.HSplit`,
  `Cursors.VSplit`, `Cursors.Hand`, and `Control.Cursor`. The generated cursor
  values were stable objects but carried no native role. ABI 0.19 now provides
  bounded cursor set/get projection, including an explicit inherited state, over
  the core's existing parent-inherited cursor model and Win32 cursor mapping.
- **MEASURED:** the C11 ABI fixture round-trips horizontal resize and inherited
  states and rejects invalid values. Managed host and Wine fixtures report
  `cursor=identity:stable|projection:roundtrip|inherit:restored`.
- **OBSERVED / corrected:** `DockPaddingEdges` retained four integers without
  affecting its `ScrollableControl` owner. The wrapper is now owner-backed;
  mutation updates retained `Padding`, triggers layout, and changes fill-child
  geometry. Host and Wine report
  `dock-padding=projection:owned|fill:inset|relayout:updated`.
- **MEASURED gate hygiene:** the repeatable Wine smoke now uses a title-scoped
  automation target, builds and loads the owned GUI.Drawing ABI plus Skia raster,
  and never overwrites the generated Drawing facade with Microsoft's assembly.
  Native and custom-painted buttons each promote one pointer activation into
  exactly one managed Click.
- **OBSERVED / corrected:** Skia's `N32` surface alias produced RGBA bytes in
  the macOS archive and BGRA bytes in the MinGW archive. The host-facing raster
  contract is now explicitly premultiplied RGBA8. The same byte-order smoke
  passes natively and under Wine, bringing the PE executable gate to 22/22.
