# M12-P22 nullable, culture, nested-service, and atomic properties

Status: **MEASURED PARTIAL**. Date: 2026-08-07.

## Question

Can GUI.Forms retain a property's non-null payload schema while its current
value is null, format two inspectors with different cultures without mutating a
process locale, give nested members their own converter/editor identities, and
commit one edit across multiple selected controls without leaving partial
state?

## Implemented contract

- `PropertyDescriptor::kind` remains the payload kind. `nullable` separately
  admits `std::monostate`; null no longer collapses a row's declared editor or
  converter schema.
- Descriptors admit at most 256 unique typed `standard_values`, an exclusive
  finite-set policy, and a bounded inert dynamic-provider service name. Core
  conversion—not only PropertyGrid—enforces exclusive values.
- `PropertyConversionContext` is instance-owned by a converter registry. Its
  culture identity and distinct UTF-8 decimal/group separators are bounded.
  The default numeric converter localizes and parses through that context
  without `setlocale`, a global `std::locale`, or host-dependent punctuation.
- Object members can declare payload kind, nullability, enum schema, finite
  values, and converter/editor services. PropertyGrid carries the effective
  descriptor on every edit path instead of clearing nested services or using
  the current runtime variant as schema.
- `set_selected_objects` projects the common compatible schema. Commit first
  preflights every owner and immutable path; setters then run as one guarded
  transaction. If an owner rejects, changed owners are restored before the
  error is published. A successful transaction publishes one logical change.
- Property getters are checked against their declared schema so an invalid
  executable registration cannot leak malformed state into inspection or DML.

## Deterministic evidence

- A `de-DE`-like registry formats `1234.5` as `1.234,5` and parses the same
  punctuation back to the typed double while the invariant registry remains
  unchanged.
- A nullable exclusive number begins as `(none)`, commits a typed standard
  value, returns to null, and rejects a value outside its finite set.
- A nested `Settings.Threshold` member retains and uses its own `percent`
  converter identity.
- With two owners, the second rejects `5.0`; the first is restored to `1.0`,
  the second stays `2.0`, and no change event is published. `3.0` then commits
  to both and publishes exactly once.

## Gates

```text
normal:        complete configured suite                   63/63 passed
renderer-free: binding + inspection                         2/2 passed
diff hygiene:  git diff --check                             passed
```

Sanitizer and Win64/Wine closure remain for the combined P22/P23 managed
projection round; no portability result is inferred from the host build.

## Honest remaining edge

The managed PropertyGrid/editor-service projection, dynamic standard-value
provider registry, mixed-value visual/editor state, hostile setters that both
mutate-before-throw and reject rollback, modal editors, DML schema, and
localized boolean/enum display remain open. The native rollback contract is
strong for conforming GUI.Forms setters; rollback failure is reported rather
than disguised.
