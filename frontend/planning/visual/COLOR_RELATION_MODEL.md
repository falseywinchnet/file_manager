# Relational color model

Status: **GIVEN principle; numerical token system not yet frozen**.

## 1. Color belongs to a pair

Foreground color is not selected independently of its background. GUI.Forms
themes should define a foreground/background relationship as the primitive:

```text
ColorPair = {
    background: OKLCH,
    foreground: OKLCH,
    relation: phase family,
    contrast class: body | large | instrument | disabled | decorative,
    gamut policy: sRGB | Display-P3 | platform profile
}
```

“Purple text” and “blue background” are incomplete tokens. A useful token names
the relation: violet-on-white, green-on-black, teal-on-parchment, or
gold-on-blue.

## 2. Supplied phase-diamond examples

The architect supplied four example relationships. Pair notation below is
`background → foreground`:

| Family | Background | Foreground |
|---|---|---|
| A | white | violet `#9261B3` |
| B | black | green `#82B361` |
| A-prime | parchment | teal `#356B62` |
| B-prime | blue | gold `#FFD27A` |

The diamond describes offset relationships between background phase and text
phase in OKLCH space. It is evidence for a family generator, not a claim that
the four hexadecimal samples are adequate for every size, weight, display, or
accessibility mode.

## 3. Seed palette

The classic Microsoft Office Word ribbon font-color gallery is the preferred
source family for base hues, tints, and shades. It supplies a useful set of
professional primaries and pastels that can seed:

- semantic selection and focus;
- live-data instruments;
- file/object icon families;
- title frescoes;
- rare ceremonial patterns;
- warnings, success, and destructive state;
- and authored visual schemes.

Before values become normative, the specific Office revision, default theme,
color-management path, and screenshot/source must be pinned. “Office ribbon
colors” has varied across releases.

## 4. Compiler behavior

The theme compiler should:

1. select a named Office-derived anchor hue or supplied custom anchor;
2. derive the paired background and foreground in OKLCH using a declared phase
   relation;
3. adjust lightness and chroma jointly for the target contrast class;
4. gamut-map the pair into the destination profile without independently
   clipping one member into a different relationship;
5. generate hover, pressed, selected, disabled, active-window, and
   inactive-window variants by transforming the pair together;
6. verify that meaning survives monochrome/high-contrast presentation and is
   never conveyed by color alone.

Small body text may require a darker/lighter derivative than the supplied large
display sample. The compiler should preserve hue relationship while satisfying
the chosen legibility gate; it must not silently approve a decorative pair for
body text.

## 5. DML implication

DML should normally reference pair roles:

```text
TextStyle {
    color_pair: "title.violet_on_paper";
    contrast_class: large;
}
```

Raw foreground colors remain available for low-level custom drawing, but common
controls should consume resolved pair roles. This makes color behavior
inspectable, themeable, accessible, and consistent across custom controls.

## 6. Relation to material depth

Color and depth are orthogonal:

- relief communicates physical role;
- color pairing communicates semantic phase and hierarchy;
- illustration supplies identity or ceremony;
- object icons carry recognition;
- spacing and geometry establish structure.

A deeper control does not automatically receive more chroma. A selected row
does not become flatter merely because it receives a semantic color. This
separation prevents both grey machinery everywhere and rainbow skeuomorphism.

