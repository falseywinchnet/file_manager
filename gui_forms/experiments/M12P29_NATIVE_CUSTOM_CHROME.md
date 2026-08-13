# M12-P29 — native custom-chrome composition

Status: **MEASURED PARTIAL**. Date: 2026-08-13.

## Question

Can a retained GUI.Forms surface own the File Manager's Watercolor identity
plane continuously through a genuine native title region, while leaving native
caption controls, window identity, drag, resize, activation, and lifecycle
semantics with the host?

## Implemented contract

- `resolve_window_chrome_hit(Window&, Point, span<string>)` returns a portable
  `WindowChromeHit`: the exact retained hit target, its client/drag role, and
  the matching authored drag-region identity. Only the configured backdrop
  itself begins native movement. An interactive descendant remains ordinary
  client input; a hit-test-transparent decorative descendant exposes the
  backdrop naturally.
- `validate_window_chrome_drag_regions` accepts multiple disjoint stable-ID
  regions and reports empty, duplicate, and unresolved IDs distinctly. The
  AppKit host validates before creating a native window.
- `MacHostOptions::window_drag_region_ids` is the reusable plural contract. The
  existing singular option remains source-compatible and is merged into the
  same validated set.
- AppKit's `transparent_full_size_content` path keeps an `NSWindow` with its
  genuine traffic lights and native style mask. GUI.Forms paints from client
  `y=0`; a primary press whose public chrome hit resolves to `drag_region` is
  handed to `performWindowDragWithEvent:`. No caption button is painted and no
  AppKit type enters a public header.
- `HostActivationEvent` now updates `Window::active`, so retained material can
  distinguish key/inactive presentation without disabling content.
- The Custom Chrome Lab is an ordinary consumer. Its 40-logical-pixel title
  uses the House Composite Sapphire Dusk ramp (`#17347f` through `#3a68cb` to
  `#d96872`), cyan/violet/apricot blooms, an outer lowlight plus ordered
  highlight keylines, a pointer-transparent glass mark, retained title text,
  and an interactive exclusion. Its public `VisualInspectorView` reports the
  title identity, bounds, material counts, display chunk, and window state.

The public API is renderer-neutral and the rendering path remains CPU-only.
It encodes host mechanics, not File Manager policy.

## Deterministic evidence

- `gui_forms_custom_chrome_lab_tests` proves the exact 1120 x 40 logical title
  bounds, four active fills/two keylines, a distinct two-fill inactive state,
  stable geometry at 1x and 2x, exact open-backdrop drag resolution,
  transparent decoration pass-through, interactive descendant exclusion and
  pointer delivery, ordinary body behavior, semantic retention, and distinct
  empty/duplicate/unresolved validation failures.
- `gui_forms_host_protocol_tests` pairs the activation material with the
  portable host lifecycle and input boundary.
- `gui_forms_macos_host_close_tests` exercises native attach, resize, scale,
  activation, occlusion, move, zoom, minimize/restore, close authorization,
  and shutdown. Its host JSON also reports one configured and one resolved drag
  region.
- `gui_forms_macos_multi_window_tests` proves the same resolved chrome-region
  configuration survives the multi-window application path.
- `gui_forms_custom_chrome_lab_font_policy` prevents the native lab from
  silently losing GUI.Forms' pinned fonts.

The release targets are `gui_forms_custom_chrome_lab` (AppKit),
`gui_forms_custom_chrome_lab_windows` (Win32), and
`gui_forms_custom_chrome_lab_tests`.

Configured and built through the authoritative M4 mirror:

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/cmake -S gui_forms -B gui_forms/build \
  -DCMAKE_BUILD_TYPE=Release -DGUI_FORMS_BUILD_GALLERY=ON \
  -DGUI_FORMS_SKIA_PREBUILT=ON \
  -DGUI_FORMS_SKIA_OUT=gui_forms/build/skia-cpu-release
/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/cmake --build gui_forms/build \
  --target gui_forms_custom_chrome_lab \
           gui_forms_custom_chrome_lab_tests \
           gui_forms_host_protocol_tests \
           gui_forms_macos_host_close_tests \
           gui_forms_macos_multi_window_tests --parallel 10
/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/ctest --test-dir gui_forms/build \
  --output-on-failure -R \
  'gui_forms_(custom_chrome_lab|host_protocol|macos_host_close|macos_multi_window)_tests|gui_forms_custom_chrome_lab_font_policy'
/Users/ultimussecundai/.local/bin/m4build -- \
  /usr/bin/env PATH=/opt/homebrew/bin:/usr/bin:/bin \
  /opt/homebrew/bin/cmake --build \
  gui_forms/.build/windows-x64-skia-make \
  --target gui_forms_custom_chrome_lab_windows --parallel 10
```

The final focused native run passed 5/5 tests. The MinGW target also compiled
and linked the standard-titlebar Windows lab successfully; that build result is
not evidence for the still-missing Win32 full-client composition path.

## Native dogfood

The AppKit lab was opened from Terminal in the M4 Mac mini's logged-in Aqua
session and controlled only through the Computer Use Screen Sharing workflow.
Fresh captures showed a continuous saturated sapphire-to-coral identity plane
behind three unobscured genuine traffic lights, with no generic blank title
bar. The title began at client `y=0`, remained 40 logical pixels high, and the
inspector reported the intended retained identity and current display chunk.

The open title backdrop moved the native window. A second press on the
excluded title button incremented the lab's retained counter, demonstrating
that it was client input rather than a painted platform rectangle. Activating a
background Terminal changed the title to its restrained gray-violet two-layer
state while the body and button remained enabled; reactivating restored the
four-layer Watercolor state. The lab closed through its native traffic light
and returned its launch Terminal to the prompt without affecting neighboring
dogfood windows.

Screen Sharing synthesized coordinates did not align reliably with the remote
frame edge or narrow traffic-light targets: one intended interactive press
started a drag and one intended resize moved the window. Neither is classified
as a host defect. Every meaningful action used a fresh capture, and exact hit
semantics plus resize/zoom/minimize/restore/close are independently paired with
the deterministic tests above. The visual pass also exposed that the seam was
still represented only as a border; it was moved to the new ordered keyline
contract, with the unchanged one-pixel appearance now asserted at 1x/2x.

Reference judgment: this reaches the House Composite's required native/title
seam and Watercolor identity structure. It is a focused construction board,
not a claim that the rest of the File Manager prototype has reached visual
parity.

## Honest remaining edge

This is macOS completion, not universal custom chrome. The Win32 lab composes
the same retained specimen below a standard `WS_OVERLAPPEDWINDOW` title bar;
Win32 full-client composition, DWM caption-zone mapping, system menu/snap
behavior under that mode, and physical Windows dogfood remain open. The lab
does not fake those missing semantics with painted caption controls. Linux
host support is also outside this evidence.
