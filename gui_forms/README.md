# GUI.Forms proving project

Status: implementation spike. This subtree builds a native, retained,
custom-rendered control framework demonstration backed by CPU-only Skia. It is
not the File Manager application and does not yet constitute a stable ABI.

The first deliverable is a Modern.Forms-style control gallery with structured
instrumentation, automated tests, and private AppKit and Win32 hosts. The same
retained Gallery now runs natively on macOS and as a PE64 program under Wine.
See `AGENTS.md` for the non-negotiable dependency and source boundaries.

## Build and run on macOS

The dependency scripts fetch the pinned Skia/libpng/zlib revisions and apply
the recorded PNG-only policy patch. CMake invokes them automatically when the
private archives are absent.

```sh
cmake -S gui_forms -B gui_forms/build -DCMAKE_BUILD_TYPE=Release
cmake --build gui_forms/build --parallel
ctest --test-dir gui_forms/build --output-on-failure
open "gui_forms/build/GUI.Forms Gallery.app"
```

The lower project does not consume sources from the File Manager parent. The
public retained core is renderer-neutral; Skia and AppKit remain private
adapters. See `third_party/SKIA_BUILD_EVIDENCE.md` for the exact measured pin,
archive sizes, link audit, and the non-PNG decoder negative result.

## First-party consumer specification

[`file_manager_demoboard/README.md`](file_manager_demoboard/README.md) defines
the standalone Native File Manager Demoboard: a fully interactive,
fixture-backed reproduction of the accepted File Manager prototype built only
through public GUI.Forms. It is currently a detailed worker specification, not
an implemented frontend or authorization to add missing GUI.Forms capabilities
inside the consumer directory.

## Build and run the Windows Gallery under Wine

```sh
cmake -S gui_forms -B gui_forms/build-windows-x64 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/x86_64-w64-mingw32.cmake \
  -DGUI_FORMS_ENABLE_SKIA=OFF \
  -DGUI_FORMS_ENABLE_MACOS_HOST=OFF \
  -DGUI_FORMS_ENABLE_WINDOWS_HOST=ON \
  -DGUI_FORMS_BUILD_TESTS=OFF \
  -DCMAKE_BUILD_TYPE=Release
cmake --build gui_forms/build-windows-x64 --parallel
wine "gui_forms/build-windows-x64/GUI.Forms Gallery.exe"
```

The deterministic Wine interaction, capture, and close gate is:

```sh
gui_forms/tools/run_windows_wine_smoke.sh \
  /Users/quentinkuttenkuler/file_manager/gui_forms/build-windows-x64
```

See [`docs/WINDOWS_WINE_HOST.md`](docs/WINDOWS_WINE_HOST.md) for implemented
host calls, measured evidence, the stable-ID automation channel, and the honest
MSAA/UIA boundary.

## Library and extension documentation

- [`docs/LIBRARY_AND_ASSEMBLY_GUIDE.md`](docs/LIBRARY_AND_ASSEMBLY_GUIDE.md)
  describes current native targets, the experimental managed facade, trusted
  retained-subtree extensions, lifecycle/ownership rules, and host-owned house
  aesthetics.
- [`docs/CURRENT_API_REFERENCE.md`](docs/CURRENT_API_REFERENCE.md) maps the
  implemented C++ and experimental C ABI 0.x surface without implying 1.0
  stability.
- [`experiments/WINDOWS_ORACLE_PREFLIGHT.md`](experiments/WINDOWS_ORACLE_PREFLIGHT.md)
  records the installed Wine/.NET/MinGW environment and the original bounded
  backend gates; [`docs/WINDOWS_WINE_HOST.md`](docs/WINDOWS_WINE_HOST.md)
  records their implementation and result.
- [`experiments/M11A_GENERATED_SURFACE_AND_ABI_0_2.md`](experiments/M11A_GENERATED_SURFACE_AND_ABI_0_2.md)
  records the generated 796-row replacement surface and its first ABI-backed
  managed control tree on host .NET and Wine.
- [`experiments/M11B_MANAGED_HOST_SURFACE_AND_ABI_0_3.md`](experiments/M11B_MANAGED_HOST_SURFACE_AND_ABI_0_3.md)
  records `Application.Run(Form)` crossing ABI 0.3 into the deterministic
  headless host and the real Win32 DIB host under Wine, including the captured
  managed demonstration surface.
- [`experiments/M11C_MANAGED_CALLBACKS_AND_LOOP_ABI_0_4.md`](experiments/M11C_MANAGED_CALLBACKS_AND_LOOP_ABI_0_4.md)
- [`experiments/M11D_BEHAVIORAL_FACADE_AND_PRIVATE_LOAD.md`](experiments/M11D_BEHAVIORAL_FACADE_AND_PRIVATE_LOAD.md)
  records the unchanged-specimen Wine load, visible retained surface, behavior
  widening, and the remaining managed drawing passthrough.
- [`experiments/M11E_GUI_DRAWING_CORE_AND_ABI.md`](experiments/M11E_GUI_DRAWING_CORE_AND_ABI.md)
  records the renderer-free GUI.Drawing semantic core, independent drawing ABI,
  deterministic C++/C traces, and exact raster/facade work still left to M11f.
- [`experiments/M11F_RENDERING_RASTER_STORAGE_AND_DRAWING_FACADE.md`](experiments/M11F_RENDERING_RASTER_STORAGE_AND_DRAWING_FACADE.md)
  records owned raster storage, the private Skia service, the generated 307-row
  Drawing facade, and measured Win32 handle leases under Wine.
- [`planning/GUI_DRAWING_REVISION_PLAN.md`](planning/GUI_DRAWING_REVISION_PLAN.md)
  owns the 307 required captured drawing rows, separates native/facade/
  passthrough/missing status, and keeps broader File Manager drawing needs in a
  distinct consumer-promotion ledger.
- [`planning/CONTROL_COMPLETENESS_MATRIX.md`](planning/CONTROL_COMPLETENESS_MATRIX.md)
  remains authoritative for supported, deferred, packaged, and excluded
  control families.

## Current proving-slice limits

- M11f supplies owned GUI.Drawing pixels, a private macOS CPU-only Skia raster
  module, a generated 307/307 Drawing facade, and measured D8 handle leases
  under Wine. The Windows x64 Skia raster DLL is not packaged yet, so the
  unchanged Wine specimen still uses the earlier managed paint path. M11g owns
  the visible zero-passthrough cutover; the prior retired compatibility specimen result is not native
  drawing closure.
- The checked-in C++ header is the compiled form of the DML gallery schema; a
  DML compiler/designer is not implemented yet.
- Text input proves committed UTF-8 and the AppKit IME bridge, not a finished
  shaping, selection, or editing engine.
- The M3e host seam supplies monitor/work-area records and change events,
  inherited portable cursors, bounded UTF-8 clipboard text, synchronous pointer
  capture, occlusion-aware frame scheduling, five typed common-dialog families,
  bounded nested-modal state, and typed inbound drag destinations in headless
  and AppKit adapters. Drag payloads are portable text, file-list, and
  media-typed byte variants; outbound drag initiation remains open. The
  contract is platform-neutral; the Windows proving adapter now implements the
  W0-W3 window/input/presentation subset, while Linux and the remaining Windows
  services are open. AppKit capture is scoped to its native implicit drag
  sequence, not a global event tap. Clipboard commands are not wired into the
  provisional text probe; M4 owns the editor, selection, copy/paste, and undo
  contract.
- The Gallery bundles Portsmouth Rapids 1.0 evaluation faces for titles and
  control chrome while retaining Lucida Grande for field content. This is a
  demo-only role split, not the final shaping or redistribution contract.
- The first M6 extraction provides reusable panel/group/label/link and
  button/check/radio controls with deterministic events and normalized-key
  activation. The Gallery now consumes those public types and visibly exercises
  indeterminate and visited-link state. Text, range, list, container, provider,
  and command families remain incomplete; see
  `experiments/M6A_REUSABLE_BASIC_CONTROLS.md`.
- M6b adds focus-aware `ContainerControl`/`UserControl` identities and a reusable
  custom `RangeControl` substrate with concrete `TrackBar` and determinate
  `ProgressBar`. The Gallery consumes the latter two. Real scrolling/extents,
  container load/validation/dialog routing, and complete stock range facades
  remain open; see `experiments/M6B_CONTAINER_RANGE_CONTROLS.md`.
- M6c adds nested `begin_init`/`end_init` invalidation batching, deterministic
  attach/detach hooks with failed-attach rollback, and one-shot lifetime
  `UserControl::loaded` with committed attachment counts. The DML Gallery now
  dogfoods a composed `UserControl` with reusable label children and visible
  lifecycle counts. Typed property metadata, validation, scrolling, and
  designer serialization remain open; see
  `experiments/M6C_INITIALIZATION_LIFECYCLE.md`.
- M4a adds the renderer-neutral Unicode text-store foundation: strict UTF-8,
  typed UTF-8/UTF-16/scalar/line positions, bounded atomic replacement, Unicode
  separator line indexing, deterministic style-span transformation, and
  structured rebuild/rejection counters. Contiguous UTF-8 remains a candidate;
  grapheme segmentation, shaping, editing, and IME remain open. See
  `experiments/M4A_UNICODE_TEXT_STORE.md`.
- M11a generates and verifies the complete captured 796-row managed facade
  surface, but most rows remain behavioral stubs. ABI 0.2 implements only the
  high-frequency construction/property/tree/disposal spine. M11b appends the
  ABI 0.3 top-level host projection: the managed 17-control demonstration now
  completes real Win32 create/show/paint/close under Wine and the equivalent
  deterministic headless lifecycle. M11c appends ABI 0.4 typed click/form
  callbacks and managed-loop dispatch. M11d widens the generated behavioral
  facade and privately loads the pinned unchanged retired compatibility specimen specimen to a live
  46-control Win32/Wine surface; this is measured compatibility evidence, not
  parity or redistribution.
  callbacks, UI dispatch, live mutation, contained callback faults, requested
  close, and a bounded `ApplicationContext`. Broader input/member behavior,
  cross-thread dispatcher stress, accessibility, and packaging remain open.
  This is not the final cross-language ABI or evidence that the 51-family
  matrix is filled out.
