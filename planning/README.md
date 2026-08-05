# Pre-plan index

Status: **grand-architect interview / evidence collection with accepted program
spine**.

This directory does not contain the project plan. It defines how we will earn
one. The immediate task is to discover exclusions, convert the surviving intent
into measurable invariants, and design fair experiments before choosing a core,
GUI framework, index store, semantic pipeline, or plugin ABI.

## Artifacts

| File | Purpose |
|---|---|
| `PREPLAN_CHARTER.md` | Stages and exit gates for creating the actual plan |
| `PROGRAM_MAP.md` | Accepted component ownership, repository permissions, contract authority, and delivery order |
| `PRODUCT_NEGATIVE.md` | Confirmed exclusions and anti-trajectories |
| `ARCHITECTURE_INPUTS.md` | Confirmed positive constraints and deferred missions |
| `EVIDENCE_REGISTER.md` | What the supplied references do and do not establish |
| `DECISION_PROTOCOL.md` | Evidence labels, decision records, benchmark rules |
| `EXPERIMENT_MAP.md` | Spikes and measurements required before architecture selection |
| `SURFACE_PIPELINE.md` | Native stateful retained UI hypothesis and open design axes |
| `INTERVIEW_ROUND_002.md` | Concrete Finder/Explorer anti-model and richness questions |
| `QUESTION_ATLAS.md` | Numbered questions, beginning with what the product must not become |
| `visual/VISUAL_CUE_INVENTORY.md` | Atomic 2000–2012 accept/adapt/reject visual ledger |
| `visual/VERDICT_ROUND_001.md` | First atomic structure/material verdict batch |
| `visual/VERDICTS_001.md` | Architect's first visual verdicts and constrained synthesis |
| `visual/visual-atlas.html` | Visible, interactive reference atlas with locally saved verdicts |
| `visual/CEREMONIAL_SOFTWARE_ROUND_001.md` | Scope and guardrails for installer, OOBE, and professional-tool aesthetics |
| `visual/ceremonial-software-atlas.html` | 17-object, 68-cue interactive installer/motion/Visual Studio verdict board |
| `visual/CEREMONIAL_VERDICTS_001.md` | Architect's installer, OOBE, progress, and professional-tool verdict synthesis |
| `visual/MATERIAL_DEPTH_ROUND_001.md` | Depth loci and scope for skeuomorphic/material/pattern research |
| `visual/material-depth-atlas.html` | Static, visible accept/distill/reject board for 3D structure and pattern families |
| `visual/MATERIAL_VERDICTS_001.md` | Architect's material, depth, configuration, and frozen-pattern synthesis |
| `visual/COLOR_RELATION_MODEL.md` | OKLCH foreground/background pair doctrine and Office-derived seed palette |
| `visual/FRONTEND_DESIGN_BRIEF_001.md` | Pooled frontend/dependency knowledge, first coherent daily-work visual grammar, icon shortlist, and open design questions |
| `visual/frontend-concept-atlas.html` | Deterministic HTML program mockups for folder, search, criteria, theme, and icon-direction review |
| `gui_forms/WINFORMS_CONTROL_INVENTORY.md` | Exhaustive compatibility catalogue (research in progress) |
| `gui_forms/retired compatibility specimen_COMPATIBILITY_INVENTORY.md` | Observed serious-consumer compatibility envelope (research in progress) |
| `gui_forms/GUI_FORMS_BACKEND_DECISION_MAP.md` | Renderer, host, text, state, event, DML, ABI, and demonstration choices |
| `gui_forms/GUI_FORMS_INTERVIEW_LEDGER.md` | Architect's GIVEN GUI.Forms constraints plus CANDIDATE tradeoffs and remaining subquestions; not an ADR |
| `gui_forms/GUI_FORMS_RESOURCES_AND_CONFIGURATION.md` | Theme/language assemblies, PNG boundary, safe fallback, and retired compatibility specimen-informed mutable configuration candidates |
| `search/SEARCH_RECONCILIATION_001.md` | Search identity, root ownership, offline-catalogue, result-motion, mutation-boundary, ConeDAG, and semantic-deferral constraints from the first completed board export |
| `../decisions/ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md` | Accepted engine object model, root/volume topology, immutable-generation spine, update/ranking/API policy, external placement, scale behavior, and initial performance constitution |
| `../decisions/ADR-002-ORACLE-CONTRACT-AUTHORITY-AND-PROGRAM-SEQUENCE.md` | Accepted Oracle contract authority, repository topology, and phased GUI.Forms → engine → Oracle → frontend program sequence |
| `../engine/` | Standalone Go search/index engine workstream; implementation may proceed behind its API and evidence gates without selecting parent integration architecture |
| `../kolmogrov/` | Independent ConeDAG, complexity, exhaustive-breakdown, and fixed-width perceptual-hashing research program |
| `../orchestrator/` | Paper-first Oracle specification repository and canonical cross-project contract registry |
| `../file_manager/` | Waiting end-user frontend repository; implementation is gated on GUI.Forms, engine, and Oracle snapshots |

The following parent-level artifacts remain deliberately absent until the
interview supplies their inputs:

- `INVARIANTS.md` — measurable behavior, performance, privacy, and UX laws;
- `CAPABILITY_MODEL.md` — core/platform/plugin ownership of every capability;
- `PLAN.md` — only after the pre-plan exit gate passes.

Accepted partial-spine decisions already live under `../decisions/`; they do
not imply that the complete parent plan or every component choice is closed.

## Current facts

- Product name: **File Manager**.
- Target systems: Windows, macOS, and Linux.
- Product stance: local-first, fast, quiet, simple, inspectable, scriptable.
- Visual lineage: classic Windows Explorer (roughly XP/7 through restrained
  NT 7–10 idioms), NeXTSTEP, Windows Watercolor, and turn-of-the-millennium
  Encarta—rendered crisply rather than imitated as a low-resolution skin.
- Core surfaces: GUI, CLI, and a small AI-interrogatable local API.
- Core search: persistent economical indexing plus exact, lexical, structural,
  and eventually local semantic retrieval.
- Core does not search the web or a web store.
- File handlers, icons, previews, sorting, and plugins require explicit product
  models rather than uncontrolled inheritance from host-shell behavior.
- File Manager itself will not rely on .NET, Java, Godot, or a bundled browser
  engine. GUI.Forms and the end-user surface are disciplined C++; Oracle and
  hostile plugin supervision are Rust; the opt-in indexing/search service is a
  standalone Go engine with purpose-built storage and retrieval machinery.
  Mature databases remain mandatory measurement controls rather than its
  production backend.
- The main surface is a single location-oriented window: collapsible tree,
  content, and collapsible preview/properties sections. No tabs and no dual pane.

Anything not recorded as GIVEN or DECIDED remains an open question or candidate.

## Interview loop

1. Answer questions by ID, in any order or batch size.
2. Answers are recorded as GIVEN constraints or explicit deferrals.
3. Contradictions are surfaced, never silently resolved.
4. New questions are added where an answer exposes a hidden tradeoff.
5. Only closed constraints become experiment gates or decision criteria.

The most useful answer is often “absolutely not,” followed by the failure mode
that makes it unacceptable.
