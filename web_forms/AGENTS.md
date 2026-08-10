# Web.Forms planning guardrails

This subtree is the planning-stage **Web.Forms** project. It defines a bounded,
browser-valid HTML/CSS authoring profile and a build-time compiler into retained
GUI.Forms construction. It is not a browser engine, a web application, or a
second GUI runtime.

Before changing this subtree, read in order:

1. `README.md`
2. `planning/CHARTER.md`
3. `planning/MOCKUP_DECOMPOSITION_001.md`
4. `planning/NATIVE_TRANSLATION_NEGATIVE_001.md`
5. `planning/LANGUAGE_PROFILE_001.md`
6. `planning/NESTED_LANDSCAPE_001.md`
7. `planning/GENERATED_CPP_PROFILE_001.md`
8. `planning/DATA_PROJECTION_001.md`
9. `planning/COMPILER_BOUNDARY_001.md`
10. `planning/INTERVIEW_LEDGER_001.md`
11. `decisions/README.md`
12. `planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`
13. parent `../AGENTS.md`
14. `../planning/SURFACE_PIPELINE.md`
15. `../gui_forms/AGENTS.md`

## Current permission

Research, language specification, conformance-corpus design, and interface
negotiation are open. Parser, generator, designer, runtime, and build-system
implementation remain closed until the language profile, GUI.Forms metadata
edge, fidelity oracle, and first experiment gate are approved.

## Hard boundaries

- The approved profile contains no JavaScript, inline event handlers,
  executable URLs, DOM mutation, network fetch, or general-purpose expression
  evaluator. Do not implement an escape hatch.
- A browser is a design-time preview host only. No browser, DOM, CSS engine,
  JavaScript engine, or web runtime is linked or shipped with generated
  applications.
- Generated applications use public GUI.Forms construction and behavior. They
  may not reach into private GUI.Forms, Skia, AppKit, Win32, or GTK types.
- Skia remains GUI.Forms' private CPU renderer. Web.Forms produces no renderer
  objects and adds no GPU capability.
- The compiler is fail-closed: an unknown element, selector, property, unit,
  resource, state, or compiler directive is an error, never an ignored hint.
- The source language is versioned and bounded. No accepted feature enters 0.1
  without parse, memory, expansion, layout, paint, and diagnostic limits.
- Stable IDs belong to retained controls, commands, popups, model hosts, and
  semantic nodes. Decorative fragments lower into their owning paint/style
  record and do not become addressable controls merely because HTML used a
  `div` or pseudo-element to draw them.
- Repeated/virtualized items receive runtime identities from model keys; static
  source IDs are never cloned into duplicates.
- Generated code is disposable. Source and compiler version are authoritative;
  hand edits to generated C++ are not a supported workflow.
- Product output is C++17-compatible source in the accepted orthodox generated
  profile. Rust is permitted only in the build-time compiler and contributes no
  runtime dependency.
- Preserve GUI.Forms' accepted deterministic event and initialization-order
  contract. A Web.Forms compiler may coalesce work but may not invent a second
  runtime ordering mode.
- Preserve unrelated work in the dirty parent tree. Do not change GUI.Forms
  implementation while defining this project.

## Epistemic discipline

Use the repository labels GIVEN, OBSERVED, MEASURED, HYPOTHESIS, CANDIDATE,
REJECTED, and DECIDED. Browser similarity is OBSERVED evidence until a named
native comparison workload measures it. “WYSIWYG” always names its font pack,
browser/profile, scale, state, geometry tolerance, and pixel-difference rule.
