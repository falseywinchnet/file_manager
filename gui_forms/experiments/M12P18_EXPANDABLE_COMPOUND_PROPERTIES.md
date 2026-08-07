# M12-P18: expandable compound properties and reset affordance

Date: 2026-08-07

Status: **MEASURED PARTIAL** continuation of B02 and control-matrix row 41.

## Claim and boundary

M12-P18 turns the typed compound values admitted by M12-P16 from read-only
diagnostic text into retained editable content. Native `PropertyGrid` now
projects stable property paths and expandable child rows for:

- Point: X and Y;
- Size: Width and Height;
- Rect: X, Y, Width, and Height;
- Insets: Left, Top, Right, and Bottom;
- Color: Red, Green, Blue, and Alpha; and
- FontSpec: Role, Size, Weight, Italic, and LetterSpacing.

The path syntax is an inspection/edit API (`Bounds.X`, `Padding.Left`), not a
DML serialization grammar. A field edit reads the live typed parent, converts
the field through its bounded kind, reconstructs the parent value, and calls
the parent's registered setter. The parent descriptor therefore remains the
authority for whole-value validation, invalidation effects, change events,
origin, and reset. GUI.Forms never parses the diagnostic parent string back
into authored state.

Invalid conversion or whole-value rejection restores every visible field from
the actual getter, preserves the complete target value, and reports the exact
field path. Programmatic parent refresh updates the summary and all fields.

## Retained hierarchy and Reset

`PropertyList` now supports validated row hierarchy metadata: an earlier
expandable parent, bounded depth, retained expanded state, descendant
visibility/layout, disclosure hit testing, and expand/collapse semantic
actions. Hidden descendants remain owned controls with stable IDs; expansion
does not destroy and recreate editor state.

Reset is a real stock `Button` per resettable parent row, not painted text or a
demo callback. Its enabled state follows the property's actual
`ShouldSerialize` result, while value origin remains independently reported.
Activation calls the registered whole-property reset path, synchronizes parent
and descendants, publishes one typed reset event, and disables when the value
returns to its declared default/inherited state.

## Native dogfood

The Complete Showcase Values page expands the inspected NumericUpDown Bounds
property through public APIs. On the native macOS host, accessibility exposed
the parent disclosure, X/Y/Width/Height text editors, and enabled/disabled
Reset buttons. Computer-use dogfood performed these exact operations:

1. activated Reset Value, changing the inspected value from 3.75 to 0 and
   publishing `PropertyGrid committed Value · origin default · reset`;
2. edited Bounds.X to 25, which updated the typed parent summary, changed its
   origin to local, enabled Reset Bounds, and published the precise path; and
3. activated Reset Bounds, restoring the parent and X field to zero, origin
   default, and disabled reset state.

The dense layout remained clipped to its one scroll plane, with visible
Windows-inspired disclosure and Reset controls and no overlap outside the
PropertyGrid edge.

## Measured gates

On the macOS arm64 development host:

```text
ctest --test-dir build --output-on-failure \
  -R 'gui_forms_(inspection_controls|showcase_interaction)_tests'
  2/2 passed

ctest --test-dir build-renderer-free-polish --output-on-failure \
  -R gui_forms_inspection_controls_tests
  1/1 passed

ctest --test-dir build-m11e-sanitize --output-on-failure \
  -R gui_forms_inspection_controls_tests
  1/1 passed under ASan/UBSan with warnings as errors

cmake --build build-win64 --target gui_forms_core gui_forms_controls -j 8
  passed
```

The focused oracle covers stable collapsed editors, expansion visibility,
path-to-parent typed mutation, exact path events, negative rectangle-width
rejection without partial mutation, spacing-field editing, Reset enablement and
activation, parent/child resynchronization, and expanded semantic state plus
pointer/semantic collapse/expand actions. A projection revision gate also proves
that a setter callback may replace the selected object without the retiring edit
touching stale row or descriptor maps.

## Open edge

This is compound content breadth, not arbitrary object reflection. General
nested objects, nullable-specialized/date/duration/path/command values,
collections and collection editors, multi-selection/common-value state,
property-level custom editor/type-converter factories, modal editors, inline
color/font swatches, flags checklists, context-menu commands beyond visible
Reset, localization, atomic multi-property transactions, DML typed IR/round
trip, managed `System.ComponentModel` descriptors, and the managed WinForms
PropertyGrid projection remain open.
