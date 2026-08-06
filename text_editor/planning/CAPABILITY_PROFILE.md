# Text Editor capability profile

Status: **CANDIDATE profile for interview and dependency negotiation**.

## Text Editor-owned minimum

- bounded byte ingestion and text/binary classification;
- explicit encoding/BOM and malformed-input policy;
- newline observation/preservation and optional explicit conversion;
- grapheme-safe text editing, selection, clipboard, undo/redo and dirty state;
- line/column/status, wrap and optional line-number behavior;
- literal find/replace and an accepted non-regex wildcard mode if admitted;
- exact document revision anchoring for color-hint spans;
- external modification, permission, conflict and safe-write behavior;
- one-document-per-window unsaved-close and reopen behavior;
- application-local help/context IDs.
- owned Characters dialog with pinned Unicode/font evidence and encoding-safe
  insertion under `CHARACTERS_DIALOG.md`;

## GUI.Forms requirements

- named stable public package/C++ or C ABI snapshot;
- mature multiline editor with large-document policy, UTF positions, grapheme
  selection, word/line navigation, scroll/viewport, caret, clipboard, undo,
  composition/IME, bidi, shaping, font fallback and accessibility text ranges;
- bounded decoration/span rendering independent of text truth;
- find/replace UI composition, menus, status, task dialogs, modal ownership,
  modeless owned dialogs, focus restore and keyboard command routing;
- local HelpProvider/F1, tooltips and greaseboard anchors;
- typed inbound/outbound file/text drag and clipboard where admitted;
- headless document/input/semantic/layout/damage traces and native host evidence;
- no mandatory RichTextBox/RTF package for plain-text editing.

GUI.Forms panel controls are not a Text Editor topology. Find/Replace,
Characters, Help and file-information utilities remain owned popup dialogs.

## Orchestrator requirements

- `ORC-APP-001`: stable Text Editor identity/profile and settings namespace;
- `ORC-PCK-001`: open/open-many/save-as selection sessions with app-scoped hidden
  visibility;
- `ORC-HLP-001`: local Text Editor help registration/resolution;
- `ORC-HND-001`: plain/config file-type and open-with association declarations;
- `ORC-SET-001`: bounded editor preferences;
- optional provider identity/availability for color hints, without admitting a
  general plugin UI or semantic-fact dependency.

Text Editor does not require Engine indexing, Kolmogrov, semantic hives,
federation, plugin AI or LAN availability for basic editing.

## File Manager/shared picker requirements

- independently consumable open/open-many/save-as picker;
- live navigation without Engine;
- Text Editor-specific hidden-file default/remembered setting;
- filters do not hide extensionless or unknown configuration files by default;
- exact selection/access grant and native fallback behavior;
- open-with/handler declaration remains Orchestrator-owned.

## Color-hint boundary

The first implementation may embed a small first-party lexer set or use a
versioned provider; selection is open. Every result identifies document digest/
revision, provider/version, range coordinate system and category. Stale spans
drop immediately. Invalid or slow providers cannot delay typing, saving, or
plain rendering. No hint engine receives filesystem authority merely because it
parses text.
