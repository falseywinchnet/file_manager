# GUI.Forms implementation guardrails

This subtree is the independent GUI.Forms proving project. It may be built from
this directory without File Manager. The repository root may consume it; this
subtree may never include, link, inspect, or depend on sources outside
`gui_forms/`, except pinned third-party dependencies explicitly declared by its
own build.

## Scope of the first proving slice

Build a native macOS control-gallery demonstration and automated tests proving:

- retained controls and explicit ownership;
- stateful imperative mutation;
- typed layout/paint/hit-test dirtiness;
- nested update scopes and bounded flushes;
- damage-limited CPU painting;
- Forms-like focus, pointer, keyboard, and activation behavior;
- a DML-like compiled/static form description with stable IDs;
- instrumentation visible in logs and an optional on-screen diagnostics panel;
- CPU-only Skia hidden behind GUI.Forms drawing vocabulary;
- and a native AppKit host which can be launched, inspected, and closed.

This is a control gallery, not File Manager and not a file browser.

## Hard boundaries

- C++20 is admitted. Objective-C++ is admitted only in the macOS host adapter.
- No .NET, Java, browser engine, or Godot dependency.
- GUI.Forms has no GPU capability. Do not compile, link, initialize, probe, or
  expose Ganesh, Graphite, GL, Vulkan, Metal, Direct3D, Dawn, or WebGPU.
- Skia may be used only through a private CPU renderer adapter. No Skia type may
  appear in public headers, DML, demo-facing control APIs, C ABI, or tests of the
  renderer-neutral core.
- **GIVEN:** the GUI.Forms core and host protocol target Windows, macOS, and
  Linux equally. No portable semantic, event order, path/dialog model, or
  scheduler rule may encode AppKit, Win32, Wayland, or X11 assumptions. Honest
  platform differences are capability-reported adapter behavior, not hidden
  core policy. Private Skia extensions may use platform-specific CPU details but
  may not change or leak through the portable contract.
- PNG is the only image decoder admitted to the renderer/resource core. Other
  codecs are excluded.
- AppKit types remain inside `src/host/macos/`.
- The core is retained; do not introduce an immediate-mode control API or a
  perpetual redraw loop.
- Accessibility/help metadata is optional secondary hook data, not required to
  instantiate a control.
- Portsmouth Rapids 1.0 evaluation faces are supplied for the Gallery. Use
  them only for titles and control chrome; retain Lucida Grande for editable,
  collection, metric, and other field content. This demo role split does not
  settle M4 shaping/fallback or M9 production typography and packaging.
- Preserve user files and unrelated working-tree changes.

## Public seam

- Portable C++ headers live under `include/gui_forms/`.
- Renderer-neutral implementation lives under `src/core/` and
  `src/controls/`.
- Skia implementation lives under `src/render/skia/`.
- Native macOS code lives under `src/host/macos/`.
- Demonstrations and their static/DML descriptions live under `demo/`.
- Tests live under `tests/`.
- Instrumentation must be queryable as structured counters/snapshots, not only
  prose logs.

The public ABI may begin as disciplined portable C++ for the spike, but no
compiler-specific type may be assumed to be the eventual stable C ABI.

## Agent ownership during the initial parallel build

- Core agent: `include/gui_forms/`, `src/core/`, core instrumentation, and core
  tests.
- Control-gallery agent: `src/controls/`, `demo/`, demo resources, and gallery
  interaction tests.
- Skia/host agent: `third_party/`, `src/render/skia/`, `src/host/macos/`, and
  dependency/build integration specific to those paths.
- Root agent: guardrails, cross-cutting build integration, final review,
  execution, visual inspection, and closure.

Do not edit another agent's owned files without first messaging that agent and
root. Communicate interface needs early. Preserve negative results and report
exact commands and environment.

## Build contract

The lower build must eventually support commands equivalent to:

```text
cmake -S gui_forms -B gui_forms/build
cmake --build gui_forms/build
ctest --test-dir gui_forms/build --output-on-failure
gui_forms/build/.../gui_forms_gallery
```

No benchmark result may be labelled fast or lightweight without recording the
workload and measurement. A working demonstration is evidence only for the
behaviors it exercises.
