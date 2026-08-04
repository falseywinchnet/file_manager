# M1c experimental ABI and corrected Gallery visual direction

Date: 2026-08-04

Status: **OBSERVED implementation evidence for the bounded M1c proving slice**.
The ABI is explicitly 0.1 experimental scaffolding. The Gallery palette is a
demo-default **CANDIDATE**, not the frozen M9 style/theme contract.

## Scope and evidence labels

- **GIVEN**: the eventual authoritative binary seam is C, with opaque
  generational handles; no C++ object, exception, STL type, renderer type, or
  platform type crosses it.
- **GIVEN**: Windows 7/10-inspired professional styling is the requested default
  direction for the demonstration, informed by Modern.Forms and the parent
  program's recorded visual verdicts.
- **OBSERVED**: Modern.Forms presents compact labelled commands, white work
  canvases, blue selection/title identity, explicit pane boundaries, and dense
  conventional control hierarchy in its ControlGallery, Explorer, and Outlaw
  Windows screenshots.
- **OBSERVED**: the parent program accepts precise square edges, shallow raised
  actions, inset edit wells, etched grouping, blue semantic selection, compact
  professional density, restrained watercolor structure, and deeper technical
  instruments.
- **CANDIDATE**: the exact palette values and drawing recipes in the Gallery are
  local visual evidence. M9 still owns relational OKLCH roles, material records,
  theme packs, contrast transforms, and theme-generation replacement.

## Modern.Forms reference set

Repository: `modern-forms/Modern.Forms`, locally pinned at
`b1babc26283c24c6f963ac396b0565db9c737f98`.

- `docs/controlgallery-windows.png` — SHA-256
  `50a37a6346e74d45633c4bec17808811427bfc5e252c949761976c590ce319a8`
- `docs/explorer-windows.png` — SHA-256
  `31475c3eb8416232e6a61c43842a61a8ab31bc37c6623ba86c927b9d70024bbc`
- `docs/outlaw-windows.png` — SHA-256
  `53f29f8703bb97b05e6c6594f650f51ab5459e901bb096d53950b552a72d50dc`

The framework repository itself describes these as its ControlGallery,
Windows Explorer clone, and Outlook clone samples. The screenshots are used as
layout/style evidence only; no .NET implementation becomes a runtime
dependency.

## ABI 0.1 substrate

- C11-compatible `c_api.h` with fixed-width values and length-delimited UTF-8.
- One exported symbol, `gf_get_api_v0`, negotiating a size-prefixed function
  table for version `0x00000001`.
- Slot-plus-generation handles for controls and event tokens; slot reuse bumps
  generation and zero is never a valid identity.
- Explicit retain/release, synchronous disposal, and subtree-wide handle/token
  staling before owned visual teardown.
- Stable ID, bounds, visibility, parent/child, component-state, structured
  last-error, and state-change subscription operations.
- UI-thread rejection at the ABI boundary and exception-to-result translation.
- Thin C++20 RAII wrapper proving copy-retain, move, release, property access,
  and disposal without exposing the C++ retained object.
- macOS export allowlist constraining the dynamic library to the negotiation
  symbol.

## Visual correction and Gallery default

Visual inspection exposed a renderer/host contract defect: the Skia raster is
RGBA on this build, while the AppKit host described it to CoreGraphics as BGRA.
Red and blue were swapped, turning intended blue selection brown and cyan live
data yellow. The host now declares premultiplied RGBA and the Skia smoke test
asserts exact first-pixel bytes `(241, 238, 226, 255)`.

With truthful color restored, the Gallery default now uses:

- white work/list/edit planes and cool pearl-grey structural chrome;
- deliberate Windows-blue selection, focus, progress, and command identity;
- shallow two-step raised actions and inset edit wells;
- etched square group boundaries and compact 12 px control rhythm;
- a restrained blue/lavender backplane field rather than flat card UI;
- a deep navy technical instrument with cyan/gold live-data pairing;
- an ABI 0.1 status label and a two-line development-font disclosure.

## Executable observations

- C11 consumer: version negotiation, truncated table prefix, unsupported
  version, stable-ID buffer sizing, bounds round trip, reparent/detach, errors,
  retain/release, wrong-thread rejection, token disconnect, handler-driven
  disposal, and subtree handle staling.
- C++20 consumer: RAII copy/move/release, property mutation, stable ID, disposal.
- Export policy: `nm -gjU` reports only `_gf_get_api_v0`.
- Dynamic dependencies: `libc++.1.dylib` and `libSystem.B.dylib`; no Skia or
  AppKit dependency enters the ABI library.
- Canonical M1a lifecycle trace remains byte-identical, SHA-256
  `7fd93035ae50971f7f8fa04f8a0dc3e6164fc8a9bd23978d36ca78ad55d0f99a`.

## Verification

Environment: macOS arm64, AppleClang 16.0.0.16000026.

- Renderer-free Debug/Werror: 7/7 tests passed.
- Full Gallery/Skia/AppKit Debug: 10/10 tests passed.
- Renderer-free ASan+UBSan/Werror: 7/7 tests passed.
- Renderer-free TSan/Werror: 7/7 tests passed.
- Renderer-free Release/Werror: 7/7 tests passed.
- Updated Gallery launched, Diagnostics toggled through native input, and both
  primary and diagnostics layouts were visually inspected.

## Unresolved edges

- ABI 0.1 is process-global experimental scaffolding. Runtime/context
  partitioning, allocators, weak handles, bulk spans, async dispatch, complete
  properties/events, ABI migration, Windows/Linux export policy, and two
  independent external consumers remain future gates.
- ABI 1.0 is not frozen and no source/binary compatibility promise is made.
- Style values are not theme tokens. High contrast, DPI matrix, theme switching,
  OKLCH compilation, icons, and Portmouth Rapids asset loading remain later
  milestones.
- Corrected color is a conformance fix, not renderer selection evidence. Skia
  remains replaceable pending the M2 comparison gate.
