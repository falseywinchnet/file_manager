# Text Editor Characters dialog

Status: **GIVEN popup utility; dataset/font depth open**.

## Surface law

Text Editor has no right or left panel. Character selection is an ordinary
owned modeless popup dialog, with a modal fallback if platform focus behavior
requires it. It is not a separate Character Map application and does not remain
after its owning editor window closes.

The primary command is `Insert -> Character...`. `Help -> Characters...` may be
an alias because the utility also explains character identity, but Help does not
become a utility panel.

## Minimum behavior

- display characters supported by the selected/fallback font scope;
- search locally by exact code point, Unicode name or admitted keyword;
- browse by bounded block/category filters;
- show glyph, code point, Unicode name, UTF-8 bytes and admitted escape forms;
- show selected font/fallback and missing-glyph state honestly;
- insert one or more selected characters at the current selection;
- copy without inserting;
- keep a bounded session-local recent list if admitted;
- never rewrite encoding/newline state silently.

Insertion is an ordinary Text Editor edit and undo unit. If the document's
selected encoding cannot represent the character losslessly, the dialog must
not silently insert and corrupt on save: offer cancel, explicit encoding change,
or another architect-approved path.

## Data boundary

Unicode names/categories and script/block data come from a pinned local Unicode
data version or the selected platform API, with version/provenance recorded.
Font coverage is observed for the chosen rendering path and may differ by
platform. No online glyph search, emoji store, sticker system or AI description
service.

## GUI.Forms requirements

- owned modeless dialog, owner focus tracking and document-close cleanup;
- virtualized glyph grid/list rather than eager control-per-code-point layout;
- search field, filters, font selector if admitted, detail fields and insert/
  copy commands;
- keyboard grid navigation, screen-reader name/codepoint publication, IME-safe
  insertion and high-DPI rendering;
- deterministic headless fixtures using a bundled test font/data subset;
- no perpetual redraw or background scanning of every system font at startup.

## Evidence

- BMP and supplementary-plane insertion;
- combining marks, variation selectors, joiners and multi-code-point sequences;
- missing glyph, invalid code point and unsupported document encoding;
- search/category/version behavior;
- insertion undo/redo and external document change while dialog is open;
- keyboard/screen-reader workflow, scale and large virtualized dataset latency.

## Deferred questions

Emoji sequences, named sequences, private-use characters, system-versus-bundled
fonts, favorites/recents persistence, escape syntax list and whether multi-
character compositions belong in this utility remain Text Editor interview
items.
