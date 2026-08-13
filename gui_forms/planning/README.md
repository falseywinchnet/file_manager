# GUI.Forms planning index

Status: **component planning and evidence; individual records retain their own
epistemic status**.

This directory contains GUI.Forms planning records. It does not make a
candidate architecture accepted merely by describing it. Accepted cross-project
decisions remain in the repository `decisions/` directory, and Web.Forms owns
the exact bounded HTML/CSS source profile and compiler semantics.

## Design-language system records

The following records capture the grand-architect discussion of 2026-08-13.
Together they replace the inadequate mental model of a stock control plus a
collection of theme callbacks.

| Record | Purpose |
|---|---|
| [`DESIGN_LANGUAGE_SYSTEM_MODEL.md`](DESIGN_LANGUAGE_SYSTEM_MODEL.md) | States the recursively nested system model, the HTML/CSS/C++ separation, and the shared failure mode of non-composable theme addressability. |
| [`CONTROL_DNA_AND_STYLE_ADDRESSABILITY.md`](CONTROL_DNA_AND_STYLE_ADDRESSABILITY.md) | Proposes the superset vocabulary of regions, parts, relationships, states, and projections that a design language must be able to address. |
| [`AUTHORITATIVE_CONTROL_REGIONS.md`](AUTHORITATIVE_CONTROL_REGIONS.md) | Records the first authority axiom and derives parent-owned visual order, interactive order, overlap, clipping, projection, and transition rules. |
| [`MICROFEATURE_CONSERVATION_PROTOCOL.md`](MICROFEATURE_CONSERVATION_PROTOCOL.md) | Defines an interview-to-source-to-native conservation ledger and uses the broken breadcrumb as the first corrective case. |

These records are inputs to both GUI.Forms and Web.Forms. GUI.Forms owns the
retained execution, input, layout, rendering, accessibility, and inspection
contracts. Web.Forms owns the admitted source grammar, validation, selector
profile, lowering, and generated C++ contract. Neither project may silently
absorb the other's authority.

## Existing program records

- [`MASTER_IMPLEMENTATION_PLAN.md`](MASTER_IMPLEMENTATION_PLAN.md)
- [`CONTROL_COMPLETENESS_MATRIX.md`](CONTROL_COMPLETENESS_MATRIX.md)
- [`FILE_MANAGER_CONSUMER_CAPABILITY_PROFILE.md`](FILE_MANAGER_CONSUMER_CAPABILITY_PROFILE.md)
- [`FUTURE_APPLICATION_CONSUMER_PROFILE.md`](FUTURE_APPLICATION_CONSUMER_PROFILE.md)
- [`FUTURE_PROGRAM_GUI_FORMS_CAPABILITY_AUDIT.md`](FUTURE_PROGRAM_GUI_FORMS_CAPABILITY_AUDIT.md)
- [`GUI_DRAWING_REVISION_PLAN.md`](GUI_DRAWING_REVISION_PLAN.md)
- [`PAINT_PIPELINE_AVAILABILITY.md`](PAINT_PIPELINE_AVAILABILITY.md)
- [`WINFORMS_API_CATALOGUE.md`](WINFORMS_API_CATALOGUE.md)
- [`WINFORMS_BEHAVIOR_GAP_CATALOGUE.md`](WINFORMS_BEHAVIOR_GAP_CATALOGUE.md)
