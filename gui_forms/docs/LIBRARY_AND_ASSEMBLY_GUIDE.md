# GUI.Forms library and assembly guide

Status: **public design guide over an implementation spike**. Names marked
**CANDIDATE** are working package/source shapes, not an ABI or NuGet promise.
The implemented C++ surface remains pre-1.0.

GUI.Forms is a retained, CPU-rendered, cross-platform forms library. It is built
for native applications first and now exercises an experimental thin C# facade
for Forms-shaped consumers such as the admitted retired compatibility specimen compatibility corpus. The
native runtime owns identity, state, layout, painting, input, accessibility,
and disposal. The managed facade projects those objects; it does not become a
second UI engine.

## 1. What exists today

| Build object | Current role | Public-boundary status |
|---|---|---|
| `gui_forms_core` | retained tree, lifetime, events, invalidation, display chunks, scheduling, resources, portable host protocol | renderer-free C++ proving API; not frozen |
| `gui_forms_controls` | reusable basic, container, lifecycle, range, and raster-canvas controls | renderer-neutral C++ proving API; incomplete |
| `gui_forms_host_headless` | deterministic host/service oracle | test and automation adapter |
| `gui_forms_host_macos` | AppKit translation and native window host | private platform adapter |
| `gui_forms_host_windows` | Win32 translation and CPU DIB window host | private bounded platform adapter |
| `gui_forms_skia` | private CPU raster adapter | never crosses the control or ABI seam |
| `gui_forms_c_api` | opaque-handle control/tree, callback, dispatcher, raster, input, edit, cursor, scrolling, layout/property transactions, and top-level-host experiment | ABI 0.24; not the eventual 1.0 table |
| `gui_drawing_core` | portable geometry/color, drawing resources, state stack, paths, logical image references, and typed recording | renderer-free C++20 proving API; not frozen |
| `gui_drawing_c_api` | generational drawing handles, transactional bitmap edits, damage, and command submission | independent experimental ABI 0.2 with a negotiable 0.1 prefix; one exported negotiation symbol |
| Gallery model/application | visible dogfood and instrumentation | example, not library authority |

The native core does not link .NET, AppKit, Win32, Wayland, X11, Skia types, or
platform control objects into its public contract.

## 2. Intended delivery layers

The eventual consumer stack has one ownership system and several projections:

```text
application or trusted compatibility extension
        |
        +-- C++ RAII wrapper -------------------+
        +-- generated C# facade (M11 experiment) --+--> ABI 0.24 --> native retained engine
        +-- compiled DML handles ---------------+
                                                     |
                                                     +--> GUI.Drawing ABI 0.2 command/image/edit core
                                                     +--> selected private host adapter
                                                     +--> selected private CPU renderer
```

The stable C ABI is the authoritative future binary seam. C++ wrappers provide
RAII and typed convenience. The generated C# assembly uses deterministic
handles, P/Invoke, callback trampolines, and explicit UI-thread dispatch. DML
compiles into ordered construction/property/event operations; generated source
is disposable and never becomes runtime state truth.

**GIVEN subsystem boundary, CANDIDATE package/namespace shape:**

- `GUI.Drawing` — portable geometry, color, drawing resources, paths, images,
  recording graphics contexts, and safe native handles;
- `GUI.Forms` — Forms-shaped controls, components, events, layout, input, and
  owner-draw integration over GUI.Drawing;
- a generated `System.Drawing` compatibility facade for the admitted captured
  surface; this preserves managed names without making .NET drawing the native
  implementation;
- a generated compatibility/deviation manifest shipped beside the facade;
- optional developer analyzers and DML generators in a tools package, not the
  runtime assembly.

Assembly identity, namespaces, strong naming, target frameworks, and NuGet
layout remain M11 decisions. No current binary should bind to these candidate
names.

**OBSERVED current limitation:** M11d managed owner paint still delegates to
.NET 10 `System.Drawing`, rasterizes to an intermediate PNG, and then uses ABI
0.6 to present that raster. This is explicitly `passthrough`, not GUI.Drawing
support. The 353-row drawing owner set, 307-row required floor, broader File
Manager oversight families, and cutover gates are defined in
`../planning/GUI_DRAWING_REVISION_PLAN.md`.

**MEASURED M11e experiment:** GUI.Drawing now has an independent renderer-free
core and C ABI for value/resource/state semantics, paths, logical image
references, image attributes, and typed command recording. C++ and C consumers
produce a byte-identical golden trace. Logical images have no pixel storage and
no Forms/managed paint path calls this ABI yet; raster execution, bitmap
storage, facade mapping, and cutover are M11f/M11g work. See
`../experiments/M11E_GUI_DRAWING_CORE_AND_ABI.md`.

**MEASURED M11f continuation:** GUI.Drawing now owns bounded raster storage and
installs a private CPU-only Skia service lazily. The generated
`System.Drawing.Common` laboratory facade resolves all 307 required captured
rows; the combined surface is 1,104/1,104. Win32 HBITMAP/HDC/HWND leases pass
native and generated-.NET probes under Wine. The assembly name remains a
compatibility identity, not a dependency on Microsoft's implementation. The
Windows x64 private Skia DLL and unchanged zero-passthrough run remain open.
See `../experiments/M11F_RENDERING_RASTER_STORAGE_AND_DRAWING_FACADE.md`.

**MEASURED M12-P26 continuation:** the public `GUIForms::Drawing` target is now
an exported dependency of `GUIForms::Controls`. GUI.Drawing bitmap edits are
rectangular, cancellable, generation-safe, and damage-queryable through C++ and
ABI 0.2. Public `RasterCanvas` consumes that contract with zoom/pan,
nearest/linear retained sampling, transparency presentation, RGBA/BGRA
normalization, resource retirement, and localized Window damage. The same
renderer-free color core owns linear sRGB, XYZ D65, OKLab/OKLCH, gamut status,
and chroma-only mapping. This is the Paint substrate, not a Paint document
model or completed detailed Color dialog. See
`../experiments/M12P26_EDITABLE_RASTER_CANVAS_AND_COLOR_TRUTH.md`.

**MEASURED M11a experiment:** replacement assemblies named
`System.Windows.Forms` and `System.Windows.Forms.Primitives` now contain all 797
required captured facade identities. Their `Control` hot path projects a
bounded set of operations through experimental ABI 0.2. These laboratory
identities are not the package-name or strong-name decision and are not ABI 1.0.

**MEASURED M11b experiment:** additive ABI 0.3 projects a generated managed
`Form` and its retained descendants into the selected native host.
`Application.Run(Form)` completes the deterministic headless lifecycle on host
.NET and a real Win32 DIB create/show/paint/close cycle under Wine. This is the
first end-to-end managed surface; managed input callbacks, live run-loop
mutation, and most generated member behavior remain open. See
`../experiments/M11B_MANAGED_HOST_SURFACE_AND_ABI_0_3.md`.

**MEASURED M11c experiment:** additive ABI 0.4 projects retained button input
into managed `Click`, live property mutation, queued UI dispatch, cancelable
`Form.Close`, `FormClosed`, and `ApplicationContext.ThreadExit`. Managed
exceptions are contained at the trampoline, reported through
`Application.ThreadException`, and counted natively. Deterministic headless and
Win32/Wine runs produce the same one-per-stage lifecycle counts. See
`../experiments/M11C_MANAGED_CALLBACKS_AND_LOOP_ABI_0_4.md`.

**MEASURED M12-P15/P16/P17/P18/P19/P20/P21/P22 native property substrate:** GUI.Forms now separates
inert public property descriptors from private executable registrations. The
typed value domain includes scalars, geometry, spacing, color, font,
generational images, finite named/flags enums, bounded immutable objects, and
homogeneous collections; defaults, reset/
ShouldSerialize, exact dirty scope, deterministic inspection, and tokenized
changes use real retained setters. Base layout properties and visible
Label/PictureBox/Button appearance properties are registered. Exact defaulted,
local, inherited, ambient, and computed origins are independent of serialization
policy. Native `PropertyGrid` dogfoods the descriptors through stock typed
editors, live refresh, validation, weak selection, expandable geometry/spacing/
color/font fields, recursive member/index paths, atomic collection insert/remove/
move, real retained Reset controls, and instance-owned converter/editor
registries. Numeric values use a retained NumericUpDown factory; inert
descriptor service names can select reusable custom formatting and editors
without placing callbacks in metadata. FlagsValueEditor owns a tokenized
multi-select CheckedListBox popup; ColorValueEditor owns canonical alpha-aware
text, swatch, invalid-state, cancel, and typed failure behavior. Nullable payload
schema, finite standard values, per-registry numeric culture, nested member
services, and rollback-safe multiple-owner commits are native contracts rather
than PropertyGrid exceptions. Field edits reconstruct every immutable
ancestor and never parse its diagnostic display string. Stock `ComboBox.Items`
dogfoods the content-collection contract. This is the native
schema/tooling foundation for compiled DML and managed descriptors, not a DML
parser; diagnostic value strings are not serialized source. Experimental ABI
0.22 and the generated façade expose native PropertyGrid selection/sort/
refresh for managed GUI.Forms Controls. ABI 0.23 adds owned nonvisual proxies
for ordinary `TypeDescriptor`/`ICustomTypeDescriptor` objects with nullable
typed values, finite standards, current-culture TypeConverter formatting and
parsing, reset/serialization/change callbacks, selection-failure preservation,
and rollback-safe multiple-owner editing. Managed `UITypeEditor` drop-down/
modal hosting is implemented by ABI 0.24: the native row owns a real retained
editor button, while the generated adapter supplies `ITypeDescriptorContext`,
`IWindowsFormsEditorService`, blocking drop-down Control hosting, owned modal
Forms, typed return, and atomic commit. It is not emitted as a nominal no-op.
See `../experiments/M12P15_PROPERTY_METADATA_CENTER.md` and
`../experiments/M12P16_COMPOUND_PROPERTY_VALUES.md` and
`../experiments/M12P17_METADATA_DRIVEN_PROPERTY_GRID.md` and
`../experiments/M12P18_EXPANDABLE_COMPOUND_PROPERTIES.md` and
`../experiments/M12P19_NESTED_PROPERTY_VALUES_AND_COLLECTIONS.md` and
`../experiments/M12P20_PROPERTY_CONVERTER_AND_EDITOR_SERVICES.md` and
`../experiments/M12P21_SPECIALIZED_FLAGS_AND_COLOR_EDITORS.md` and
`../experiments/M12P22_NULLABLE_CULTURE_NESTED_AND_ATOMIC_PROPERTIES.md` and
`../experiments/M12P23_MANAGED_PROPERTY_GRID_ABI_0_22.md` and
`../experiments/M12P24_MANAGED_TYPE_DESCRIPTOR_PROXY_ABI_0_23.md` and
`../experiments/M12P25_MANAGED_UI_TYPE_EDITOR_ABI_0_24.md`.

**MEASURED compatibility-laboratory evidence:** a consumer compiled against the
authentic strong-named .NET 10 `System.Windows.Forms` reference can resolve in a
private load context to an unsigned experimental same-name assembly on host
.NET and Wine. This establishes one feasible interception mechanism; it does
not select the package name, strong-name policy, default-context startup, or
unchanged-binary launch design. See
`../experiments/CAPTURE_1_DISPOSITION_AND_LOADER_LAB.md`.

## 3. Three different extension objects

Do not conflate these:

| Object | Code-bearing? | Authority |
|---|---:|---|
| Trusted GUI.Forms application extension | yes | may compose/subclass admitted portable controls inside a host-owned container |
| File Manager third-party plugin | isolated by default | receives only separately granted capabilities; never a raw control, renderer, native window procedure, or theme override |
| Theme/language assembly | no | immutable data-only roles, resources, messages, and coverage metadata |

The [retired compatibility specimen compatibility inventory](../../planning/gui_forms/retired compatibility specimen_COMPATIBILITY_INVENTORY.md)
is evidence for useful retained-subtree lifecycle. It is not permission to make
arbitrary File Manager plugins in-process or visually sovereign.

## 4. Retained-subtree extension shape

For trusted applications and the managed compatibility laboratory, GUI.Forms
intends a small extension contract in the lineage of retired compatibility specimen rather than a large
framework service locator.

The following is **CANDIDATE source shape**, not compilable API today:

```csharp
public interface IGuiFormsExtension
{
    string DisplayName { get; }
    void Initialize(IGuiFormsHost host);
    UserControl? Gui { get; }
    void Close();
}

public interface ILazyGuiFormsExtension
{
    UserControl? CreateGui();
}
```

Optional facets should remain narrow: lazy GUI creation, settings load/save,
status, category/menu identity, and preferred host region. A capability is not
added to the base interface merely because one plugin might want it.

### Normative lifecycle direction

1. Discover and validate extension identity and compatibility.
2. Construct the extension object without requiring a GUI subtree.
3. Call `Initialize` with a bounded host interface.
4. Create `Gui` immediately or lazily at the declared extension point.
5. Validate that the returned root is live, detached, and from the same runtime.
6. Attach it to a host-owned `ContainerControl`/`UserControl` boundary.
7. Apply host layout, semantic metadata, and style roles inside one bounded
   initialization/update transaction.
8. Publish visibility only after successful attachment.
9. On unload, hide and detach the subtree, revoke subscriptions/timers/queued
   work, call `Close`, dispose owned components and controls, then invalidate
   handles.
10. Report residual callbacks, handles, resources, or threads as unload faults.

GUI.Forms currently proves deterministic attach/detach order, one-shot
`UserControl::loaded`, successful attachment counts, initialization batching,
coalesced post-commit state events, event tokens, initialization-time
focus/capture/input cleanup, disposal, UI timers, and renderer-free posted
dispatch with shutdown revocation. Complete managed-handle unload remains a
future gate.

## 5. Ownership and thread rules

- The application/extension owns its nonvisual component container.
- The visual parent owns attached children strongly; children refer to parents
  weakly.
- Detaching a root does not destroy it while an admitted external strong handle
  exists.
- Event subscriptions are tokenized. Unload revokes tokens before user teardown
  can observe half-live controls.
- Attached control mutation occurs on the owning UI thread. Public C++
  Window/Control `BeginInvoke` queues one FIFO snapshot per host turn; the
  generated ABI retains a separate compatibility queue with the same nested
  next-turn rule. The facade's blocking managed `Invoke` is not yet the native
  C++ synchronous-invoke contract; cross-thread stress and deadlock policy
  remain explicit pre-1.0 gates.
- Exceptions never cross the stable C ABI. Managed callback exceptions are
  caught at the facade boundary and become structured diagnostics/fault state.
- A plugin reference leak cannot keep native callbacks, timers, or window
  resources active after host revocation.

## 6. Host-owned appearance

Extensions provide content and semantic intent. The host owns the visual
grammar.

- Use named roles for control title, field/content text, emphasis, warning,
  selection, surface, edge, and instrument planes.
- Do not hard-code platform theme handles, Windows system colors, AppKit
  appearances, Mica, rounded cards, or third-party chrome.
- Ordinary actions are visibly raised; inputs are inset; groups are etched;
  instrument surfaces may be deeper. Depth communicates function.
- The default direction is professional Windows 7/10 structure interpreted
  through GUI.Forms, not a pixel skin and not whatever the current OS happens
  to prefer.
- Portsmouth Rapids is the current Gallery control/title face. Lucida Grande
  or the content/system role is used for field text. This specific font pairing
  remains a demo policy until font licensing, fallback, and M9 packs close.
- A trusted custom control may paint through renderer-neutral GUI.Forms
  primitives and publish semantic children. It may not acquire the private
  renderer or replace host controls.

An extension should normally set text, values, semantic roles, and layout intent
and let the host resolve appearance. If it requires a new visual role, that role
is reviewed as host vocabulary rather than smuggled in as arbitrary colors.

## 7. Discovery and manifest direction

Executable extension discovery remains **CANDIDATE** pending the File Manager
plugin authority ADR. A future manifest needs, at minimum:

```text
stable extension ID
display name and publisher
extension and contract versions
minimum/maximum compatible GUI.Forms ABI
declared extension points and capabilities
entry assembly/type or native entry table
content hashes and optional signature chain
settings schema identity
localization/resource coverage
```

Discovery never scans arbitrary working-directory assemblies. Compatibility
failure is explicit and does not fall through to executing unknown code.
Theme/language assemblies use a separate data-pack manifest and can never name
an executable entry point.

## 8. Compatibility promise

The authoritative compatibility ledger is the
[`CONTROL_COMPLETENESS_MATRIX.md`](../planning/CONTROL_COMPLETENESS_MATRIX.md).
An API is supported only when its construction, behavior, event order,
rendering, input, accessibility, disposal, ABI, documentation, and platform
lanes meet the applicable conformance requirements.

GUI.Forms does not promise all of WinForms. ActiveX, browser hosting, printing,
MDI, obsolete families, arbitrary `WndProc`/HWND behavior, and undocumented
implementation accidents remain excluded or separately packaged. retired compatibility specimen custom
controls that depend on native messages are ported to portable GUI.Forms
behavior under ADR-001; they do not enlarge the base facade with general Win32
emulation.

The unchanged retired compatibility specimen binary under Wine is a late best-effort dogfood specimen,
not the source of new unbounded requirements. Reflection supplies coverage;
normative GUI.Forms traces supply behavior.

## 9. Versioning gates

Before a 1.0 native or managed package freezes:

- the stable C function tables and error model pass a breaking-change audit;
- two independent native consumers pass ownership/thread/callback tests;
- the generated C# Gallery and one bounded SDR-style sample repeatedly
  load/unload without residual handles or callbacks;
- assembly resolution, architecture, runtime, and deployment are reproducible;
- supported/deferred/excluded APIs ship as a machine-readable manifest;
- Windows, macOS, and selected Linux hosts pass the same normative traces; and
- package licenses, integrity metadata, and SBOM are complete.

Until then, code should depend on the smallest demonstrated surface and cite
the milestone evidence that proves it.
