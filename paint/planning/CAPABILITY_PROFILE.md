# Paint capability profile

Status: **CANDIDATE profile for interview and dependency negotiation**.

## Paint-owned minimum

- canvas/document dimensions, background and color mode;
- pixel-safe zoom/pan and cursor-coordinate/status feedback;
- selection, move, copy, cut, paste, transparent placement and commit/bake;
- architect-approved mark/shape/text tools;
- foreground/background palette, eyedropper and transparent color behavior;
- bounded undo/redo if admitted;
- PNG open/save/export at minimum, with decoder/import policy explicit;
- native clipart/composite save/open if admitted;
- external-modification and overwrite handling;
- print/export only if separately admitted;
- local contextual help topics and greaseboard overview anchors.
- detailed Color popup with RGB/hex/OKLCH and admitted CMYK conversion under
  the explicit color/profile rules in `COLOR_DIALOG.md`;

## GUI.Forms requirements

- named stable public C++/C ABI consumption snapshot and clean package;
- owned top-level/modal windows, menus, tool strips/palette composition, status,
  task dialogs, modeless owned utility dialogs and custom retained controls;
- pointer capture, pressure capability reporting, keyboard/mnemonic/focus,
  high-DPI transforms and accessibility publication;
- bounded editable text for the text tool;
- CPU GUI.Drawing bitmap surface, alpha composition, clipping, paths, shapes,
  fills, stroke, text, scaling/interpolation choices and efficient local damage;
- timers only where needed for cursor/tool feedback; no perpetual redraw;
- native/custom file/color dialogs and deterministic headless result adapters;
- clipboard and completed inbound/outbound typed drag, lazy/promised payloads,
  expiry/cancellation and cross-window participation;
- `HelpProvider`, F1/help command routing, tooltips and greaseboard anchors;
- PNG resources/decoder and explicit unsupported codec state;
- headless input/drawing/semantic/damage traces and native macOS/Windows/Linux
  host evidence.

GUI.Forms panel controls are not a Paint topology. Paint's detailed Color,
Attributes, Resize, Help and future secondary tools remain owned popup dialogs.

## Orchestrator requirements

- `ORC-APP-001`: stable Paint identity/profile and settings namespace;
- `ORC-PCK-001`: open/save-as/import/export selection sessions;
- `ORC-HLP-001`: local Paint help registration/resolution;
- `ORC-XFR-001`: file, PNG and Paint clipart/composite transfer declarations;
- `ORC-HND-001`: Paint file-type/handler declarations;
- `ORC-SET-001`: bounded application settings transaction;
- truthful availability, restart and incompatible-version behavior.

Paint does not require semantic hives, Engine indexing, plugin AI, federation or
LAN availability for its first slice.

## File Manager/shared-surface requirements

- independently consumable Document Picker package—not File Manager executable
  linkage;
- live browse/open/save without Engine;
- Paint-scoped hidden visibility and type filters;
- File Manager file/preview drag source with exact file/object snapshot;
- native fallback and unavailable route fixtures;
- no injected File Manager commands, panes, plugins or settings into Paint.

## Transfer flavors

**CANDIDATE minimum ordered offer:**

1. ordinary file reference;
2. `image/png` alpha-bearing representation;
3. a versioned Paint composite/clipart flavor if the native object model wins;
4. bounded plain text only for the Paint text tool if explicitly accepted.

Large data uses lazy/promised transfer. Every flavor names size, lifetime,
producer, cancellation and fallback. Orchestrator registers meaning but never
relays pointer motion or canvas bytes.
