# Charter for planning the plan

## Objective

Produce enough disciplined knowledge to write an implementable architecture and
delivery plan for File Manager without prematurely choosing a fashionable stack,
copying incumbent feature bloat, or mistaking an interesting algorithm for an
appropriate system boundary.

This charter governs the preparation of that plan. It is not the plan itself.

## Governing question

What is the smallest coherent system that can become the grand architect's
daily file manager while preserving a credible path to the intended indexing,
semantic, preview, handler, styling, CLI, API, and plugin ambitions?

“Smallest” refers to conceptual and operational surface, not lack of rigor.

## Workstreams to resolve before project planning

### 0. Negative product definition

Build the approved catalogue of:

- features the core must never contain;
- interactions that feel slow, patronizing, hidden, or dangerous;
- platform conventions we deliberately refuse;
- styling tropes and motion behaviors we reject;
- data collection, networking, AI, and plugin powers we prohibit;
- compatibility we will not pay for;
- failure modes severe enough to control the architecture.

Exit: `PRODUCT_NEGATIVE.md` is approved and open contradictions are named.

### 1. Invariants and budgets

Translate aesthetic and systems language into observable laws. Examples to
quantify with the architect include launch time, first folder paint, keyboard
latency, memory at rest, index write amplification, idle CPU, directory scale,
search latency, preview isolation, undo guarantees, and offline behavior.

Exit: each important adjective—native, fast, snappy, economical, local,
simple—has a workload and an acceptance bound or is explicitly marked poetic.

### 2. Capability ownership

For each capability, decide whether it belongs to:

- the portable core;
- a platform adapter;
- a privileged helper;
- the GUI process;
- the indexing service;
- a sandboxed plugin;
- an optional local AI service;
- nowhere.

Exit: every requested capability has one authoritative owner and a failure
containment boundary.

### 3. Platform reality map

Document filesystem identities, event APIs, metadata, case behavior, links,
trash/recycle semantics, mount discovery, file-handler integration, icons,
previews, sandboxing, signing, packaging, and accessibility on all three target
families. Separate portable semantics from honest platform differences.

Exit: no core abstraction depends on a false lowest common denominator.

### 4. Candidate architecture dossier

Admit candidates for core language/runtime, process topology, storage engine,
index design, GUI framework, renderer, IPC, public API, plugin ABI, and local
semantic models. A candidate is admitted only with a reason it might satisfy the
invariants and a test that can reject it.

Exit: every material choice has at least one fair alternative, unless a GIVEN
constraint makes it forced.

### 5. Spikes and falsification

Run the experiments in `EXPERIMENT_MAP.md` against declared fixtures and
hardware. Preserve code, environment, outputs, and negative results. Use the
same workloads across candidates whenever their semantics permit.

Exit: critical decisions have measurements proportional to their reversal cost.

### 6. Architecture decisions

Create explicit decision records. Select boundaries before implementation
libraries: identity before schema, capability before process, plugin authority
before plugin SDK, retrieval semantics before vector database, visual grammar
before widget theme.

Exit: accepted ADRs cover the irreversible core, and each includes a reversal
or migration path.

### 7. The actual plan

Only now write `PLAN.md`: vertical slices, dependency order, validation gates,
dogfood threshold, packaging milestones, migration policy, and deferred ideas.

Exit: the first usable slice can replace a real daily workflow without requiring
the final semantic system, complete plugin ecosystem, or finished aesthetic.

## Pre-plan exit gate

The project plan may be written when all are true:

- the negative definition and core invariants are approved;
- “usable” has a concrete daily-workflow definition;
- process and trust boundaries are decided;
- the GUI and indexing candidates have passed representative spikes;
- identity, mutation, recovery, and index-consistency semantics are explicit;
- the CLI/API grammar is sketched from real tasks;
- plugin capabilities and denials are modeled;
- semantic interpretation has a staged Oracle/provider path that cannot corrupt
  exact search, while Kolmogrov similarity remains a separately gated engine
  candidate channel;
- at least one packaging path per target OS is demonstrated;
- remaining unknowns are bounded and assigned to later gates.

## Things this charter refuses to decide

Subsequent grand-architect direction has resolved the Go engine's language,
custom-backend mandate, reference model, and target storage spine in ADR-001,
and the Oracle contract authority, repository topology, and delivery order in
ADR-002. The historical candidate list below remains accurate only for choices
not superseded by an accepted decision record.

- private implementation details and exact process routing inside the accepted
  C++ GUI/frontend, Go engine, and Rust Oracle boundaries;
- the implementation of the native retained renderer and whether Modern.Forms
  becomes a compatibility/API frontend over it;
- SQLite/FTS, a purpose-built store, or a hybrid;
- exact process routing and transport choice per registered contract;
- local model family or inference runtime;
- exact control-shelf composition within the accepted single-window pane model;
- per-platform mechanisms for the accepted internal handlers and isolated
  preview-provider boundary.

Those are interview-and-experiment outputs, not assumptions.
