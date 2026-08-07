# M12-P21 specialized flags and color editors

Status: **MEASURED PARTIAL**. Date: 2026-08-07.

## Question

Can the P20 converter/editor service center support genuine drop-down and
compound value editors without adding type-specific branches to PropertyGrid,
losing popup ownership, or treating invalid authored text as retained state?

## Implemented contract

- `PropertyEnumDescriptor` now enforces valid UTF-8, 256-byte type/choice
  names, and at most 256 choices in addition to trimmed and unique names.
- Public `FlagsValueEditor` renders the normalized flags name and opens a
  Window-owned `CheckedListBox` popup. Zero and positive single-bit choices are
  projected; toggles commit a typed `PropertyEnumValue` immediately while the
  popup remains open for additional choices.
- The flags popup uses `PopupToken`, an explicit focus scope, owner revocation,
  outside/Escape dismissal, F4/Alt+Down/Space input, tokenized subscriptions,
  and synchronous unregister-before-release cleanup.
- Public `ColorValueEditor` composes an ordinary retained `TextBox` with an
  alpha-visible checker/swatch. It emits canonical `#RRGGBBAA`, accepts
  `#RRGGBB` or `#RRGGBBAA`, preserves the last typed Color on failure, exposes
  invalid visual/semantic state, and restores canonical text on Escape.
- `PropertyEditorBinding` optionally connects typed input failures. PropertyGrid
  owns that token and routes failures into its existing exact-path inline error
  channel; programmatic synchronization clears stale editor-invalid state
  without emitting a user commit.
- The default converter registry maps Color to the bounded `color-hex`
  converter. The default editor registry maps flags enums and Color to the two
  retained controls; non-flags enums continue through the stock ComboBox.
- The Complete Showcase Values page adds a Style target that inspects a real
  Label and therefore dogfoods both `Anchor` flags and `ForeColor` alpha-aware
  editing through their registered setters.

## Deterministic evidence

- Flags popup checks initially match `Top | Left`; checking `Right` commits
  `Top, Left, Right`, and close removes the popup subtree from Window lookup.
- Valid `#2A6FB4CC` edits mutate Label ForeColor with alpha intact. Invalid
  `not-a-color` preserves that Color, publishes an exact ForeColor error, and
  Escape restores the canonical text and normal state.
- The showcase Style command selects its retained status Label and commits
  `#245A92FF` through the same public editor service.
- The complete showcase interaction suite still passes its 48-cycle captured
  slider-to-Animation/frame/paint regression. The supplied historical crash
  report was from the older `dfcc…` binary; the repaired tree remains
  non-reproducible under that deterministic gate.

## Gates

```text
normal:        binding + inspection + showcase interaction  3/3 passed
renderer-free: binding + inspection                         2/2 passed
ASan/UBSan:    inspection                                   1/1 passed
Win64:         controls + inspection PE32+                   built
Wine:          Win64 inspection                             passed
native macOS:  rebuilt application launched and remained responsive
```

Native interaction was not automated over the user's concurrent Animation
pass; no claim is made that the Style popup received an independent physical
mouse pass in that session.

## Honest remaining edge

Open specialized families include image/resource selection, nullable values,
date/time and duration, paths, commands, modal editor services, standard-values
and culture contexts, nested-member service identities, multiple-owner atomic
commit, DML schema, and managed PropertyGrid/editor-service projection.
