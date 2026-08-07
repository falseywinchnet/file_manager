# M12-P16: compound and enumerated property values

Date: 2026-08-06

Status: **MEASURED PARTIAL** continuation of B02 and property-schema
foundation for B25.

## Claim and boundary

The scalar-only M12-P15 registry could describe text and simple state but could
not faithfully author ordinary retained layout or appearance. M12-P16 widens
the renderer-neutral `BindingValue`/property value domain with:

- `Point`, `Size`, `Rect`, and `Insets`;
- `Color`, `FontSpec`, and generational `ImageId`;
- finite named/flags `PropertyEnumValue`; and
- shared immutable `PropertyEnumDescriptor` schemas with type identity,
  choices, values, and flags policy.

This is typed runtime/schema state, not a DML source grammar. Diagnostic string
rendering exists for inspection, but its spelling is explicitly not a frozen
serialization format.

Descriptor-aware conversion validates finite geometry and fonts, preserves
typed resources, resolves enum names case-insensitively, parses bounded comma
or vertical-bar flag lists, rejects unknown bits/types/names, and normalizes
back to one canonical authored name. Registration rejects a missing, malformed,
or misplaced enum schema and normalizes defaults through the same conversion.
Binding source-to-control transfer now uses descriptor-aware conversion.

## Real stock registrations

Base `Control` now exposes non-facade-shell property registrations for:

- Bounds, MinimumSize, MaximumSize, Margin, Padding, AutoScrollOffset;
- Dock, Anchor flags, and AutoSizeMode;
- TabIndex, TabStop, AllowDrop, and HitTestTransparent; and
- AccessibleName and AccessibleDescription.

These call the existing retained setters, preserve their validation and exact
dirty declarations, support reset/ShouldSerialize, and remain deliberately
non-bindable where no truthful change event exists.

Visible stock use also proves Font/Color/Image rather than leaving the kinds
theoretical:

- Label Font and ForeColor expose effective inherited values, serialize only a
  local override, and reset back to live theme inheritance;
- PictureBox Image, SizeMode, and ImageOpacity use real resource, enum, and
  scalar behavior, including tokenized Image change observation; and
- Button Font and direct Image use the same default/reset center.

## Measured gates

On the macOS arm64 development host:

```text
ctest --test-dir build --output-on-failure \
  -R 'gui_forms_(core|basic_controls|binding|layout_panel|showcase_interaction)_tests'
  5/5 passed

ctest --test-dir build-renderer-free-polish --output-on-failure \
  -R 'gui_forms_(binding|basic_controls|core)_tests'
  3/3 passed

ctest --test-dir build-m11e-sanitize --output-on-failure \
  -R 'gui_forms_(binding|basic_controls|core)_tests'
  3/3 passed under ASan/UBSan with warnings as errors

cmake --build build-win64 --target gui_forms_core gui_forms_controls -j 8
  passed
```

The focused oracle covers typed kind identity, invalid finite values, canonical
enum/flags normalization, unknown-name rejection without mutation,
compound/enum default reset, inherited visual override reset, image conversion,
and tokenized image changes.

## Open edge

B02 and B25 remain partial. Property collections/content values, nullable
typed values beyond `null`, duration/date/path/command values, ambient/local/
inherited origin metadata, framework-wide stock registration, truthful change
events for every registered property, atomic rollback, localization, DML
parser/IR/round trip/migrations, managed `System.ComponentModel` projection,
and undeclared-effect diagnostics remain. Enum choice schemas are shared, but
the executable registration map is still per control; class-level immutable
descriptor storage remains a memory-layout optimization requiring measurement.
