# M12-P17: metadata-driven property grid and value origins

Date: 2026-08-06

Status: **MEASURED PARTIAL** continuation of B02 and row 41.

## Claim and boundary

GUI.Forms now has a renderer-neutral `PropertyGrid` that projects the public
M12-P15/P16 property descriptors of an arbitrary retained `Control`. It is a
real metadata consumer, not a hand-authored diagnostics panel and not a
reflection facade.

The property center also publishes exact runtime value authorship as
`defaulted`, `local`, `inherited`, `ambient`, or `computed`. A registration may
provide the origin explicitly. Otherwise a readable property with a declared
default compares its current typed value to that default; a readable property
without a default is computed. Origin is deliberately independent of
`ShouldSerialize`: a consumer's serialization policy cannot rewrite where a
live value came from. Label Font and ForeColor prove inherited/local/reset
transitions against the retained theme.

`PropertyGrid` holds its selected object weakly, uses stable property and group
identities, offers categorized or alphabetical projection, and routes bounded
types to stock retained editors:

- Boolean values use `CheckBox`;
- scalar text and numeric values use `TextBox`;
- finite non-flags enums use `ComboBox`; and
- compound, resource, flags, and null values remain honestly read-only until a
  suitable typed editor exists.

Edits call the registered setter, normalize the displayed value from the live
getter, publish typed previous/current/origin/reset facts, and surface invalid
conversion inline without mutating the target. Reset calls the registration's
actual reset/default contract. Properties with truthful change notifications
refresh live; others require explicit `refresh_properties()`.

## Callback and lifetime boundary

A hostile test found that a target change callback could synchronously dispose
its `PropertyGrid` while an edit was committing. The edit path now revalidates
both identities before touching retained editors. `PropertyList` and
`PropertyGrid` also complete teardown through `Control::on_dispose`, whose
mutation-free base path owns child detachment; disposal no longer calls public
child mutation APIs after the component has entered its disposing state.

## Dogfood

The Complete Showcase Values page uses only the public `PropertyGrid` API to
inspect a live `NumericUpDown`. On the native macOS host the grid rendered as a
dense categorized Windows-inspired panel, exposed its categories and editors
through the semantic/accessibility tree, accepted `4.5` through the accessible
Value editor, committed it to the inspected control, and displayed
`PropertyGrid committed Value · origin local`.

## Measured gates

On the macOS arm64 development host:

```text
ctest --test-dir build --output-on-failure \
  -R 'gui_forms_(inspection_controls|binding|showcase_interaction)_tests'
  3/3 passed

ctest --test-dir build-renderer-free-polish --output-on-failure \
  -R 'gui_forms_(inspection_controls|binding)_tests'
  2/2 passed

ctest --test-dir build-m11e-sanitize --output-on-failure \
  -R 'gui_forms_(inspection_controls|binding)_tests'
  2/2 passed under ASan/UBSan with warnings as errors

cmake --build build-win64 --target gui_forms_core gui_forms_controls -j 8
  passed
```

The focused oracle covers deterministic projection and sorting, stock typed
editors, keyboard text commit, immediate Boolean commit, programmatic refresh,
invalid-edit preservation and diagnostics, exact reset, default/local/
inherited/computed origins, origin/serialization independence, selected-object
retirement, and callback-time inspector disposal.

## Open edge

This does not implement the complete WinForms designer `PropertyGrid` family.
Collection/content and nested expandable values, nullable-specialized/date/
duration/path/command values, multiple selection, arbitrary editor and type-
converter factories, modal collection/component editors, a visible reset/
context-menu affordance, per-property localization, atomic multi-property
rollback, DML parser/typed IR/round trip, managed `System.ComponentModel` and
`System.Windows.Forms.PropertyGrid` projection, complete stock registrations,
and truthful change events for every property remain open. Ambient is a real
origin vocabulary member but no stock ambient-property provider is claimed by
this slice.
