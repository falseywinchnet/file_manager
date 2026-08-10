# GUI.Forms library atlas

Status: **OBSERVED complete 470-type/147-enum declaration inventory with 617
HTML and 617 Markdown pages, reviewed type narratives, declaration-derived
method explanations, zero grouped migration records, and verified Screen
Sharing evidence for every visual surface**.

The completion audit classifies 146 types as isolated per type, 321 as
intentionally header-only or state-machine-distributed contracts, and three as
demo composition. Among visual declarations, only the foundational `Control`
(deliberately split by retained state machine) and demo-only `GalleryControl`
are not exact per-type translation units.

Open `index.html` to navigate the iframe reference. Every generated HTML page
has an AI-readable Markdown mirror under `markdown/`. The root navigation is
generated from checked-in declarations. It reports isolation independently from
API status: an atlas page is evidence of source inventory and documentation,
not an architecture decision.

Run:

```sh
python3 gui_forms/tools/generate_library_docs.py
```

The generator reads `manual.json` for reviewed type explanations, selected
method-specific narratives, visual classification overrides, and capture paths.
Every discovered method still receives a signature-grounded explanation when a
more specific hand review is unnecessary. Captures belong in `captures/` and
are admitted only after the corresponding native surface has been viewed
through Screen Sharing.
The reversible bundle sequence and the evidence required to complete each
bundle are recorded in `MIGRATION_FRONTIER.md`.

## Per-type source layout

New and migrated visual types use one directory per type, mirrored between the
public and implementation trees. A derived type nests below its reusable base
when that relationship is stable and informative. Compatibility family headers
remain thin umbrellas during migration.

Bundles 001 and 002 establish:

```text
include/gui_forms/controls/
├── panel/panel.hpp
│   ├── card/card.hpp
│   │   └── review_card/review_card.hpp
│   ├── group_box/group_box.hpp
│   └── picture_box/picture_box.hpp
├── label/label.hpp
├── button_base/button_base.hpp
│   ├── button/button.hpp
│   ├── check_box/check_box.hpp
│   ├── radio_button/radio_button.hpp
│   └── link_label/link_label.hpp
└── container/master_detail_view/master_detail_view.hpp

src/controls/
├── panel/panel.cpp
│   ├── card/card.cpp
│   │   └── review_card/review_card.cpp
│   ├── group_box/group_box.cpp
│   └── picture_box/picture_box.cpp
├── label/label.cpp
├── button_base/button_base.cpp
│   ├── button/button.cpp
│   ├── check_box/check_box.cpp
│   ├── radio_button/radio_button.cpp
│   └── link_label/link_label.cpp
├── basic/basic_control_rendering.cpp
└── container/master_detail_view/master_detail_view.cpp
```

The nested drawing above is conceptual: a base type's `.cpp` sits beside its
derived-type directories. `basic_controls.hpp` and `composition_controls.hpp`
remain source-compatible umbrellas.
