# File Manager MIT icon audit 002

Status: **GIVEN owner-approved direction: Fluent Color coverage plus an original
House Material depth layer. No external dependency or product asset is packaged;
revision, provenance, derivative, and original-art license gates remain**.

Date inspected: 2026-08-05.

## Target

**GIVEN:** icon options must be MIT licensed. The desired voice is professional,
industrial, serious, classically deep, high-color where object identity matters,
and compatible with an unplaceable 1998–2007 “Encarta '00 ultra HD deluxe”
surface.

**OBSERVED:** the MIT ecosystem is rich in flat command glyphs and contemporary
file-type themes, but the audit found no maintained, full desktop object theme
that already satisfies the target depth grammar. Selecting a flat set as the
whole answer would solve licensing while missing the art direction.

## Round 003 depth correction

The target is now explicit: **classic-iOS depth**, meaning rendered volume rather
than merely two colors or a gradient fill. A qualifying object needs a coherent
upper-left light, localized material gradients, occlusion where planes overlap,
a restrained cast shadow, a specular edge, and redrawing for its small sizes.

| Source | Status | Depth value | Limit |
|---|---|---|---|
| Fluent UI System **Color** Icons | **CANDIDATE lead** | approximately 890 MIT multicolor icons; preserved gradients and multiple fills; many assets span 16–48 px | richer than Filled, but several objects still need more occlusion and larger-size detail |
| Fluent Emoji 3D | **CANDIDATE benchmark** | actual rendered folders, open folders, file cabinets, card files, and index objects | literal emoji geometry is too soft/playful for the house family |
| Meteocons Fill | **CANDIDATE technique reference** | rich-color filled SVGs with compact atmospheric layering; 475+ icons in four styles | weather-only semantics; not a file theme |
| AppIcon Forge | **CANDIDATE construction study** | MIT tool exposes repeatable background, border, gradient, and shadow controls | tool license does not license user-supplied input art; contributes no file taxonomy |
| Original House Material 48 | **CANDIDATE likely requirement** | exact control over industrial geometry, material, size redrawing, degraded state, and rare types | illustration workload and separate original-art license decision |

### Fluent Color evidence

- Source: <https://github.com/microsoft/fluentui-system-icons>
- Inspected `main`: `9e9a1766ae48f4a138fed896b25a59a5f6619230`.
- **OBSERVED:** the repository contains `icons_color.md` and color SVG assets.
  The published SVG package describes `color` as preserved multicolor artwork
  using gradients and multiple fills. Document, Image, Apps, Document Folder,
  and many other relevant semantics are present.
- **CANDIDATE role:** primary external base for command color, small objects,
  and semantic coverage; augment or redraw the visibly important 32/48 px file,
  folder, archive, application, and volume objects.

### Meteocons evidence

- Source: <https://github.com/basmilius/meteocons>
- Inspected `main`: `70dfb1d6e30dc9e791cfb0e4c5b5e5e60e972aa0`.
- **OBSERVED:** MIT repository with 475+ icons in Fill, Flat, Line, and
  Monochrome styles; Fill is explicitly the rich-color family.
- **CANDIDATE role:** study how layered atmosphere and readable color survive at
  icon scale. No weather animation or weather semantics are imported.

### AppIcon Forge evidence

- Source: <https://github.com/zhangyu1818/appicon-forge>
- Inspected `main`: `09cda1d50d6fd3891df4de6d3f573c3c15f7ae85`.
- **OBSERVED:** MIT tool supporting icon position, color, background, border,
  gradient, and shadow adjustments.
- **CANDIDATE role:** construction-reference only. It is not a selected build
  dependency, and its Iconify inputs retain their individual licenses.

## Prior command and taxonomy shortlist

These remain useful for coverage and small control verbs, but the new depth
target removes them from leadership for visible object art.

### A — Fluent UI System Icons, filled

- Source: <https://github.com/microsoft/fluentui-system-icons>
- Inspected `main`: `9e9a1766ae48f4a138fed896b25a59a5f6619230`.
- **OBSERVED:** repository license is MIT; it publishes regular and filled icon
  lists and plain SVG assets.
- **CANDIDATE role:** primary 16/20/24 px command and status skeleton.
- **Fit:** the most professional, familiar, and operationally complete option.
  Filled glyphs should remain legible when seated on raised physical plates.
- **Failure mode:** if used as large object art, the product becomes another
  contemporary Fluent utility and loses its material identity.

### B — Phosphor Icons, duotone

- Source: <https://github.com/phosphor-icons/core>
- Inspected `main`: `2b75f3ad12b420c9504ef05df8d2564a28f8500e`.
- **OBSERVED:** repository license is MIT and the core contains multiple weights,
  including duotone.
- **CANDIDATE role:** alternate command skeleton where a second tone improves
  instrument hierarchy.
- **Fit:** more dimensional and adaptable than outline-only libraries.
- **Failure mode:** its soft geometry can read friendly or lifestyle-oriented
  rather than industrial. Mixing it freely with Fluent would also create two
  drawing grammars.

### C — VSCode Symbols

- Source: <https://github.com/miguelsolorio/vscode-symbols>
- Inspected `main`: `296ef1b62287fb2315cb5651e552e09e8c8e1de8`.
- **OBSERVED:** the repository's root license is MIT and its source separates
  file and folder icon inventories.
- **CANDIDATE role:** file-type coverage and color-taxonomy reference.
- **Fit:** serious, compact, and well adapted to developer-facing file identity.
- **Failure mode:** the art is flat, contemporary, and optimized for a narrow
  code tree. It is not a 32/48 px material-object solution.

### D — Fluent Emoji

- Source: <https://github.com/microsoft/fluentui-emoji>
- Inspected `main`: `62ecdc0d7ca5c6df32148c169556bc8d3782fca4`.
- **OBSERVED:** the repository license is MIT. It includes explicit `3D` assets
  for `File folder`, `Open file folder`, `File cabinet`, `Card file box`, and
  `Card index` among its broader art inventory.
- **CANDIDATE role:** material, light, volume, edge, and color-separation study.
- **Fit:** this is the only audited MIT art source with immediately relevant 3D
  file-storage objects and production-level rendered finish.
- **Failure mode:** literal emoji are too playful, rounded, and semantically
  overdetermined for direct use. Treat the work as a material reference, not a
  product icon theme.

### E — Tabler Icons, filled subset

- Source: <https://github.com/tabler/tabler-icons>
- Inspected `main`: `7007ad520bf85301bc464501d03619487369559f`.
- **OBSERVED:** the project describes 6,146 MIT-licensed icons, with 1,053 filled
  variants, on a disciplined 24 px grid.
- **CANDIDATE role:** semantic coverage reserve after the primary grammar is
  fixed.
- **Fit:** excellent breadth, consistent geometry, and easy provenance.
- **Failure mode:** its generic web-product character and outline ancestry make
  it the least distinctive option in this shortlist.

## License traps rejected

### vscode-icons/vscode-icons

- Source: <https://github.com/vscode-icons/vscode-icons>
- **REJECTED:** source code is MIT, but the icons are CC BY-SA and branded icons
  retain their own copyright. It does not meet an MIT-only artwork constraint.

### microsoft/vscode-icons

- Source: <https://github.com/microsoft/vscode-icons>
- **REJECTED:** code is MIT, but documentation and other repository content use
  CC BY 4.0. The repository is explicitly mixed-license, not an MIT-only icon
  source.

### La Capitaine

- Source: <https://github.com/keeferrourke/la-capitaine-icon-theme>
- **REJECTED:** the theme has the gradients and shadowing relevant to the visual
  target and describes dual MIT/GPL licensing, but its own license section says
  much of the artwork derives from Numix Circle and El General/Antu and must be
  treated as GPLv3. The configuration script is MIT; the desired artwork is not
  an MIT-only source.

## Approved direction and remaining comparison

**GIVEN:** Fluent Color is the external semantic/coverage base; the visually
important file, folder, volume, archive, application, media, and degraded-state
objects receive an original House Material treatment. Fluent Emoji 3D and
Meteocons remain technique references rather than literal product art.

The following production details remain **CANDIDATE**, not architecture or
packaging decisions:

1. Compare Fluent Color against an original House Material 48 treatment on ten
   high-frequency objects at 16, 24, 32, and 48 px.
2. Retain Fluent System filled only for monochrome command/status cases; use
   VSCode Symbols only to enumerate taxonomy coverage.
3. Study Fluent Emoji's file-storage objects for material segmentation, global
   light, edge finish, and controlled color—not for literal shape copying.
4. Study Meteocons Fill for compact atmospheric layering, then draw one original
   material family at 32 and 48 px: closed folder, open folder,
   document, image, audio, archive, application, volume, unavailable item, and
   one generic plugin-contributed kind.
5. License original File Manager art under MIT if owner policy accepts that
   downstream reuse model; otherwise record the separate asset-license decision
   before production packaging.

The unresolved implementation comparison is therefore no longer “which flat
glyph family?” It is:

- **Fluent Color, deepened selectively** — lower illustration cost and broad
  MIT coverage;
- **original House Material 48 over Fluent semantics** — highest fit and highest
  illustration cost.

Fluent Emoji 3D and Meteocons are reference sources. Fluent Filled, Phosphor,
VSCode Symbols, and Tabler are reserves for commands or coverage, not competing
object themes.
