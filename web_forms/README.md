# Web.Forms

Status: **grand-architect intake and language planning; implementation closed**.

Web.Forms is the proposed design-time authoring and compilation companion to
GUI.Forms. An author writes ordinary browser-valid HTML and CSS inside a strict,
versioned subset. A compiler validates the document, resolves its bounded
cascade and layout/style vocabulary, and emits dense C++ construction plus
static GUI.Forms schema, style, resource, and typed-handle records.

The generated application is an ordinary retained GUI.Forms program:

```text
*.wf.html + *.wf.css
        |
        v
Web.Forms validator/compiler (build time only)
        |
        +--> generated C++ construction and typed ID handles
        +--> immutable style/layout/resource records
        +--> inspection metadata and source map
        |
        v
GUI.Forms retained controls + public behavior + private CPU Skia renderer
```

There is no browser or HTML/CSS parser in the product runtime. Application code
attaches ordinary typed GUI.Forms handlers to generated handles; it does not run
or translate JavaScript.

## Accepted authoring boundary

Use HTML and CSS only as authored design source, with no JavaScript. Do not omit
interaction *states*: `hover`, `pressed`, keyboard focus, disabled, selected,
checked, expanded, and popup-visible need declarative visual specimens because
they are part of WYSIWYG. The preview tool selects those states externally; the
source does not simulate application logic.

Keep responsibilities distinct:

- `id` is stable retained identity and generates the C++ handle tree;
- ordinary `class` tokens select reusable visual recipes;
- reserved `wf-*` class tokens may carry Boolean compiler traits;
- typed `data-wf-*` attributes carry values and references that do not belong
  in CSS;
- C++ owns domain behavior, commands, models, validation, and side effects.

The nested source tree is also semantic input. Each child receives a compiled
ambient layout, surface, typography, theme, accommodation, and effective-state
context from its parent. The native result must preserve that landscape rather
than placing independently painted controls over an approximate background.

The compiler is a build-time Rust tool. It emits C++17-compatible source in the
orthodox generated profile; Rust is absent from the product runtime.

## Planning artifacts

- [`planning/CHARTER.md`](planning/CHARTER.md) — mission, scope, and opening gates.
- [`planning/MOCKUP_DECOMPOSITION_001.md`](planning/MOCKUP_DECOMPOSITION_001.md) —
  evidence from the latest File Manager concept atlas.
- [`planning/NATIVE_TRANSLATION_NEGATIVE_001.md`](planning/NATIVE_TRANSLATION_NEGATIVE_001.md)
  — owner verdict and retained negative evidence from the manual native dogfood.
- [`planning/LANGUAGE_PROFILE_001.md`](planning/LANGUAGE_PROFILE_001.md) — first
  bounded HTML/CSS profile candidate.
- [`planning/NESTED_LANDSCAPE_001.md`](planning/NESTED_LANDSCAPE_001.md) — the
  compiled parent context, surface ownership, state, and layout model.
- [`planning/GENERATED_CPP_PROFILE_001.md`](planning/GENERATED_CPP_PROFILE_001.md)
  — C++17-compatible generated-code house profile.
- [`planning/DATA_PROJECTION_001.md`](planning/DATA_PROJECTION_001.md) — narrow
  one-way item projection proposal for virtual collections.
- [`planning/COMPILER_BOUNDARY_001.md`](planning/COMPILER_BOUNDARY_001.md) —
  lowering, generated output, composition, and fidelity contract.
- [`planning/INTERVIEW_LEDGER_001.md`](planning/INTERVIEW_LEDGER_001.md) — GIVEN
  direction, recommendations, and unresolved owner choices.
- [`planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md)
  — proposed GUI.Forms metadata/code-generation edge.
- [`decisions/README.md`](decisions/README.md) — accepted local decisions.

The latest source specimen remains
[`../frontend/planning/visual/frontend-concept-atlas.html`](../frontend/planning/visual/frontend-concept-atlas.html).
Web.Forms does not copy or execute it at runtime.
