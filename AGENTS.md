# File Manager operating instructions

This repository is in pre-architecture until the grand architect closes the
questions required by `planning/PREPLAN_CHARTER.md`. Do not silently turn
research candidates into architecture decisions.

Before planning or implementation, read:

1. `planning/README.md`
2. `planning/PREPLAN_CHARTER.md`
3. `planning/EVIDENCE_REGISTER.md`
4. `planning/DECISION_PROTOCOL.md`
5. the relevant sections of `planning/QUESTION_ATLAS.md`

## Scope boundaries

- The core product is local-machine software. Web search, web stores, cloud
  accounts, and remote content discovery are not core capabilities. If ever
  admitted, they enter through explicit plugins and user-controlled policy.
- Treat Zeta only as a source for indexing and retrieval research. Do not import
  its zeta mathematics or unrelated algorithms into this project.
- Treat BFFT only as a model for research discipline: label claims, keep failed
  experiments, measure against controls, and distinguish reasoned proposals
  from measured results.
- File Manager may not rely on .NET, Java, Godot, or a bundled web engine. C++
  and Rust are the admitted native core/surface languages. Go is admitted as a
  candidate for the indexing/search service. C# bindings may target a native UI
  engine without making .NET part of File Manager.
- Modern.Forms is fetched source material and a possible compatibility/API
  frontend over a new native retained engine. Its present .NET implementation is
  not an admitted File Manager runtime dependency.
- Do not expand features merely because an operating system file manager has
  them. Absence is a first-class design decision here.

## Epistemic labels

Use these labels in planning and design records:

- **GIVEN** — directly required or excluded by the grand architect.
- **OBSERVED** — directly present in a named source or code path.
- **MEASURED** — reproduced by a named benchmark with environment and data.
- **HYPOTHESIS** — falsifiable proposed explanation or mechanism.
- **CANDIDATE** — an option admitted for comparison, not selected.
- **REJECTED** — failed a declared gate; retain the reason and evidence.
- **DECIDED** — accepted by an explicit decision record with reversal cost.

Never describe a candidate as revolutionary, fast, native, lightweight, or
semantic without naming the measurement or the deliberately unverified status.
Approximate indexes may propose candidates; exact filesystem identity and exact
stored records remain authoritative.

The GUI must not be immediate-mode. Keep authoring style, serialized description,
runtime state retention, layout, native-control wrapping, and rendering strategy
as separate axes. The live surface hypothesis is imperative authoring compiled
through a PTP-like declarative schema into a retained, custom-rendered native
core; it remains a hypothesis until `planning/SURFACE_PIPELINE.md` is resolved.

## Decision hygiene

Architecture decisions live in future numbered records under `decisions/`.
Each must state the question, constraints, candidates, measurements, failure
modes, chosen option, rejected options, reversal path, and owner approval.

Performance proposals require a baseline and workload. Preserve negative
results. Optimize the measured bottleneck. A clever data structure is not a
relevance model, and a relevance model is not an identity store.

## Voice

Be direct, curious, and exact. Name the object, its status, its evidence, and
its unresolved edge. Prefer a precise exclusion over speculative feature creep.
