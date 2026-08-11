# ADR-003: Python compiler and two-stage dogfood

Date: 2026-08-10

Status: **DECIDED — approved and directed for implementation by the grand
architect on 2026-08-10**.

Owner approval: “do it in python. dogfood, experiment, test.”

## Question

How should Web.Forms be implemented while the admissible HTML/CSS language and
GUI.Forms lowering vocabulary are still being discovered?

## GIVEN constraints

- The compiler is Python and remains a build-time tool only.
- Authored source is bounded browser-valid HTML/CSS without JavaScript.
- Stage 1 accepts or rejects source against the closed language and emits a
  versioned, deterministic intermediate representation.
- Stage 2 consumes only accepted IR and emits orthodox C++17-compatible source.
- Dogfood begins with single controls, small widgets, and attractive application
  demoboards derived from the File Manager vision board.
- Content, geometry/layout, typography, and color/effect material remain
  distinguishable through parsing and generation.
- CSS-attached extensive client styles bake into generated style generations.
- The experiment may identify and add missing public GUI.Forms capabilities.
- SVG may be a build-time visual resource but not a second structural/layout
  tree. The HTML/CSS host owns geometry, identity, semantics, and state.

## Workloads and failure modes

The progression is button/control specimens, breadcrumb and compound-widget
specimens, then a standard application shell. Each must pass Stage 1, produce
stable IR, produce compile-valid C++17 in Stage 2, and retain browser-preview
evidence.

Failures include accepting executable/unknown source, a Stage 2 path that
re-parses HTML/CSS, loss of nested surface ownership, mixing application content
into reusable style records, nondeterministic output, use of forbidden C++
constructs, or claiming GUI.Forms support where only descriptor generation has
been proved.

## Candidates

1. Finish the language on paper, then implement a Rust compiler.
2. Implement one Python pass directly from HTML/CSS to GUI.Forms calls.
3. Implement Python Stage 1 validation/IR and an independent Stage 2 C++
   generator, refining both through bounded dogfood.

## Evidence and measurements

**OBSERVED:** manual native reconstruction of the vision board was visually
weak and slow to reconcile; the retained negative is recorded in
`NATIVE_TRANSLATION_NEGATIVE_001.md`.

**HYPOTHESIS:** Python shortens the parse/inspect/refine loop without weakening
the product boundary because neither Python nor source parsing ships in the
application. Determinism, bounds, rejection behavior, and generated output are
fixture-tested rather than inferred from implementation language.

## Decision

Select candidate 3. Python owns both build-time stages, but Stage 2 accepts only
serialized, schema-versioned IR. The first Stage 2 target is a standalone,
compile-checked C++ descriptor plus capability report. A GUI.Forms construction
adapter is promoted only as manifest-backed capabilities become proven.

Style lowering has separate geometry, typography, and material records.
Unsupported source syntax is rejected. A known target capability that is absent
may degrade only through an explicit, reported fallback chain. Geometry,
identity, behavior, ownership, and accessibility never silently degrade;
decorative effects may fall back to an earlier supported CSS declaration or a
declared neutral effect.

## Why the other candidates lost

Candidate 1 delays the visual experiments that must discover the real language.
Candidate 2 couples parser evolution directly to GUI.Forms APIs and makes it
difficult to test acceptance separately from native lowering.

## Consequences

- The Python package is hermetic, standard-library-first, network-free, and
  forbidden from evaluating source or loading source-selected code.
- Golden acceptance/rejection, IR, C++ snapshot, and C++ compile fixtures are
  mandatory.
- Browser fidelity and native fidelity remain separate measurements.
- ADR-002 is superseded only for compiler language/sequence; its orthodox C++
  product profile remains binding through `GENERATED_CPP_PROFILE_001.md`.

## Reversal and migration path

Another compiler may consume and emit the same versioned IR and fixture corpus.
Generated applications and Web.Forms source need not change. Python is replaced
only after a measured compiler, packaging, or hardening failure.

## Unresolved edges

The complete CSS profile, fallback constitution, GUI.Forms capability manifest,
native construction adapter, typography oracle, visual tolerances, and 0.1
resource ceilings remain experiment outputs.
