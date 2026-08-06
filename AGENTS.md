# File Manager operating instructions

This repository has accepted several program-spine decisions but remains
pre-architecture in unresolved areas. Do not silently turn research candidates
into architecture decisions, and do not reopen an accepted ADR through older
candidate language.

Before planning or implementation, read:

1. `planning/README.md`
2. `planning/PROGRAM_MAP.md`
3. `planning/PREPLAN_CHARTER.md`
4. `planning/EVIDENCE_REGISTER.md`
5. `planning/DECISION_PROTOCOL.md`
6. `decisions/README.md`
7. the relevant component `AGENTS.md` and contract registry entries

## Scope boundaries

- The core product is local-machine software. Web search, web stores, cloud
  accounts, and remote content discovery are not core capabilities. If ever
  admitted, they enter through explicit plugins and user-controlled policy.
- Treat Zeta only as a source for indexing and retrieval research. Do not import
  its zeta mathematics or unrelated algorithms into this project.
- Treat BFFT only as a model for research discipline: label claims, keep failed
  experiments, measure against controls, and distinguish reasoned proposals
  from measured results.
- File Manager may not rely on .NET, Java, Godot, or a bundled web engine.
  GUI.Forms and the frontend are disciplined C++; the indexing/search engine is
  Go; Orchestrator and hostile plugin supervision are Rust. C# bindings may target a
  native UI engine without making .NET part of File Manager.
- Modern.Forms is fetched source material and a possible compatibility/API
  frontend over a new native retained engine. Its present .NET implementation is
  not an admitted File Manager runtime dependency.
- Do not expand features merely because an operating system file manager has
  them. Absence is a first-class design decision here.

## Program boundaries

- `orchestrator/` is the active Rust Orchestrator repository and canonical
  integration authority for cross-project API/ABI semantics, capability needs,
  and availability. New edges enter its registry and project-local negotiation
  process before adapters freeze.
- `frontend/` is the end-user application project and Orchestrator's GUI for
  settings/service controls. Frontend 001 opens only after Orchestrator Core 1.0
  is available, the named GUI.Forms go-ahead is recorded, and the architect
  explicitly directs implementation to begin.
- `engine/`, `gui_forms/`, and `kolmogrov/` remain independently buildable and
  own their private implementations.
- `malkuth/` is the planning-only suite release, installer, documentation, and
  website program. It does not absorb component versions or open publishing
  implementation before its release gates.
- `paint/` and `text_editor/` are interview-first planning subprojects. Their
  planning gates are open; source/build implementation remains closed until
  their local dependency gates and explicit architect start directions pass.
- `games/` and `lexicon/` are planning-only future projects. Research/interview
  work is open; source/build/provider implementation remains closed until their
  local gates and explicit architect start directions pass.
- `plugin_runtime/` is frozen legacy research input. New runtime implementation
  belongs to Orchestrator's plugin-supervisor subsystem after its gate opens.
- Kolmogrov similarity is a gated core engine candidate channel. It is distinct
  from AI interpretation and personal semantic memory, which belong in
  Orchestrator-managed hives beyond the engine's critical boundary.
- Persistent left/right panels are reserved to File Manager and its embedded
  picker/browser projection. Other first-party applications use owned popup
  dialogs for secondary tools.

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

Architecture decisions live in numbered records under `decisions/`.
Each must state the question, constraints, candidates, measurements, failure
modes, chosen option, rejected options, reversal path, and owner approval.

Performance proposals require a baseline and workload. Preserve negative
results. Optimize the measured bottleneck. A clever data structure is not a
relevance model, and a relevance model is not an identity store.

## Voice

Be direct, curious, and exact. Name the object, its status, its evidence, and
its unresolved edge. Prefer a precise exclusion over speculative feature creep.
