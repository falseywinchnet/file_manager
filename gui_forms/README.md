# GUI.Forms proving project

Status: implementation spike. This subtree builds a native, retained,
custom-rendered control framework demonstration backed by CPU-only Skia. It is
not the File Manager application and does not yet constitute a stable ABI.

The first deliverable is a Modern.Forms-style control gallery with structured
instrumentation, automated tests, and an AppKit host. See `AGENTS.md` for the
non-negotiable dependency and source boundaries.

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

## Current proving-slice limits

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
  contract is platform-neutral; Windows and Linux production adapters are not
  implemented yet. AppKit capture is scoped to its native implicit drag
  sequence, not a global event tap. Clipboard commands are not wired into the
  provisional text probe; M4 owns the editor, selection, copy/paste, and undo
  contract.
- The Gallery bundles Portsmouth Rapids 1.0 evaluation faces for titles and
  control chrome while retaining Lucida Grande for field content. This is a
  demo-only role split, not the final shaping or redistribution contract.
- The C++ ownership and API surface are a spike, not the final cross-language
  ABI or complete Forms-compatible control set. The Gallery is deliberately a
  proving consumer, not evidence that the 51-family completeness matrix is
  filled out.
