# Paint color dialog

Status: **GIVEN popup utility; exact color-management depth open**.

## Surface law

Paint has no right or left panel. Its detailed color selector and converter are
an ordinary Paint-owned popup dialog. The compact everyday palette may remain
on the primary Paint toolbar/surface; opening detailed color work never docks or
reduces the canvas permanently.

## Invocation modes

- **Choose Color:** owned modal dialog opened when editing a palette slot or a
  command requires one committed color. OK returns one color; Cancel changes
  nothing.
- **Color Workbench:** candidate modeless form for repeated conversion and
  foreground/background adjustment while painting. It remains owned by the
  document window, closes with it, and never becomes a persistent panel.

One implementation may serve both modes through an explicit session profile;
modal/modeless lifecycle cannot be inferred from control reuse.

## Minimum controls

- foreground/background target and swap;
- visual color field plus hue/lightness/chroma/value controls as admitted;
- alpha;
- numeric RGB and hex;
- numeric OKLCH;
- CMYK conversion fields with an explicit conversion/profile statement;
- current/new swatches and recent/application palette entries;
- eyedropper sample import from the Paint canvas;
- copy/paste textual color representation;
- validation, gamut and out-of-range indication.

## Color truth

Paint's document color representation is authoritative. Conversions are
explicit transformations under named color space, transfer function, white
point and ICC/profile policy. CMYK without a selected/output profile is an
informational conversion, not a press-proof claim. Out-of-gamut behavior must be
visible before commit; changing textual representation alone must not compound
rounding repeatedly.

The Malkuth relational OKLCH theme palette does not force Paint documents to use
theme colors or transform user artwork.

## GUI.Forms requirements

- owned modal and modeless dialog lifecycle, focus return and owner close;
- numeric fields, spin controls, sliders, color field, swatches and keyboard
  operation;
- deterministic headless input/result fixtures;
- high-contrast/accessibility labels independent of swatch appearance;
- local damage while dragging controls and no idle redraw;
- tooltip/F1 and local Help dialog integration.

## Evidence

- round trips for exact representable RGB/hex/OKLCH values;
- declared tolerance for non-exact conversions;
- alpha, gamut, invalid numeric and locale-decimal cases;
- modal cancel/no mutation and modeless document-close behavior;
- keyboard/screen-reader workflow and scaling;
- rapid slider drag damage, allocation and latency.

## Deferred questions

ICC loading/embedding, wide-gamut canvas modes, spot colors, palette file
formats, perceptual gamut mapping, sampled-color history and which CMYK profiles
may be bundled remain Paint interview decisions.
