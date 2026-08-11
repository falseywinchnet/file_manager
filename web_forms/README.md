# Web.Forms

Status: **Python two-stage dogfood plus fail-closed native-tree experiment open
under ADR-003 and ADR-004**.

Web.Forms is the proposed design-time authoring and compilation companion to
GUI.Forms. An author writes ordinary browser-valid HTML and CSS inside a strict,
versioned subset. A compiler validates the document, resolves its bounded
cascade and layout/style vocabulary, and emits dense C++ construction plus
static GUI.Forms schema, style, resource, and typed-handle records.

The target application is an ordinary retained GUI.Forms program. The current
dogfood emits a compile-checked descriptor, separate manifest-backed component
projections, and a complete experimental typed control tree. The whole tree is
refused if any required record lacks an exact native projection:

```text
*.wf.html + *.wf.css
        |
        v
Stage 1: accept/reject -> typed, versioned *.wfir.json
        |
        v
Stage 2: IR only -> orthodox C++17 descriptor + typed handles
        |          + geometry / typography / material / state / decoration
        |          + explicit GUI.Forms capability report
        +
        +--> experimental exact material projection
        |      -> GUI.Forms retained fills/borders/shadows
        |      -> retained hot/pressed/disabled/focus recipes
        |      -> per-style refusal report for inexact geometry
        |
        +--> exact native component projections
        |      -> surface/state/inset-shadow recipes
        |      -> flex + grid tracks/cells + bounded boxes
        |      -> retained typography + owned pseudo-decoration
        |
        v
experimental typed NativeForm -> GUI.Forms retained controls + private CPU Skia
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

SVG is visual-resource input, not a second structure language. Inline SVG trees
are rejected in the 0.1 dogfood profile. A later closed local SVG asset may
paint one HTML/CSS-laid image box, but its descendants cannot become controls,
layout nodes, IDs, states, or hit targets. Relational decoration such as a
breadcrumb chevron remains CSS geometry owned by its box.

The compiler is a build-time Python tool. Stage 1 accepts/rejects source and
emits versioned IR; Stage 2 consumes only that IR and emits C++17-compatible
source in the orthodox generated profile. Python is absent from the product
runtime.

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

## Dogfood commands

```sh
python3 web_forms/tools/webforms.py check \
  web_forms/boards/widgets/breadcrumb/breadcrumb.wf.html --quiet

python3 web_forms/tools/webforms.py build \
  web_forms/boards/apps/standard_shell/standard_shell.wf.html \
  --emit-ir /tmp/standard-shell.wfir.json \
  --output-dir /tmp/standard-shell-generated

python3 web_forms/tools/webforms.py report \
  /tmp/standard-shell.wfir.json \
  --manifest web_forms/capabilities/gui_forms_observed_001.json

python3 web_forms/tools/webforms.py generate-gui-materials \
  /tmp/standard-shell.wfir.json \
  --manifest web_forms/capabilities/gui_forms_observed_001.json \
  --output-dir /tmp/standard-shell-native-materials

python3 web_forms/tools/webforms.py generate-gui-layouts \
  /tmp/standard-shell.wfir.json \
  --manifest web_forms/capabilities/gui_forms_observed_001.json \
  --output-dir /tmp/standard-shell-native-layouts

python3 web_forms/tools/webforms.py generate-gui-tree \
  /tmp/standard-shell.wfir.json \
  --manifest web_forms/capabilities/gui_forms_observed_001.json \
  --output-dir /tmp/standard-shell-native-tree \
  --unit standard_shell_sapphire

python3 web_forms/tools/webforms.py profile

python3 -m unittest discover -s web_forms/tests -v
python3 web_forms/tools/measure_dogfood.py --iterations 50
```

[`experiments/DOGFOOD_001.md`](experiments/DOGFOOD_001.md) records the first
acceptance, geometry, visual, C++ compilation, and GUI.Forms capability results.
