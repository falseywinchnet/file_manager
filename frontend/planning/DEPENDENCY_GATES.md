# Frontend gates

Status: **DECIDED split gates under ADR-004**.

The earlier global Gate F0 is superseded. Frontend 001 has one foundation gate;
each real service adapter has its own later integration gate.

## GUI.Forms consumption snapshot

Required before Frontend 001 implementation:

- named experimental C ABI and C++ wrapper release;
- retained object lifetime and UI-thread rules;
- application/window/host lifecycle;
- text storage, grapheme behavior, shaping seam and IME events;
- invalidation/damage and rendering-host behavior;
- core control/container/range/menu/tree/list primitives needed by the first
  File Manager window;
- the FM0 subset in
  `../../gui_forms/planning/FILE_MANAGER_CONSUMER_CAPABILITY_PROFILE.md`,
  including priority-responsive panes, command shelf/ribbon composition,
  breadcrumb/editor/suggestion primitives, bundled HarfBuzz/FreeType typography,
  default semantic adapters, and the admitted House Material drawing vocabulary;
- theme/resource/PNG/language seams;
- headless deterministic traces plus macOS host demonstration;
- clean dependency-facing build/install target without demo/build-tree coupling.

The snapshot may explicitly narrow the 001 feature set. Semicomplete means the
first slice does not have to invent or reach inside its admitted foundations;
it does not mean every WinForms-compatible control is finished.

## Owner start direction

Passing the GUI.Forms snapshot does not start implementation automatically. The
grand architect explicitly directs File Manager frontend work to begin. Once
both events are recorded, Engine, Orchestrator, plugin, semantic-fact, and
Kolmogrov readiness cannot veto Frontend 001.

## Engine consumption snapshot

Required before replacing the Frontend 001 Engine fixture port:

- exact object/binding model and approved-root ownership;
- immutable reader generations and sandbox scanner;
- exact name/path/basic-metadata query, inspect, pagination and status;
- JSONL golden fixtures and registered private local protocol direction;
- integrity/staleness/unavailable behavior;
- representative query/resource baseline;
- Kolmogrov configuration/candidate seam reserved as a core contract, with
  conventional controls permitted during research completion;
- no frontend dependency on engine private Go packages or store layout.

“Mostly running” means useful exact search and stable consumption semantics, not
final compaction, federation, semantic providers, or every performance target.

## Orchestrator integration gate

Required before replacing the Frontend 001 Orchestrator fixture port:

- master registry entries used by the first frontend are `frozen-v0`;
- authenticated session/lifecycle client;
- immutable handler and context-command snapshots;
- settings schema/value transaction;
- engine query broker or registered direct-engine relationship;
- semantic/provider and plugin systems explicitly report `stubbed` or
  `unavailable`; empty success is forbidden;
- real plugin preview/thumbnail and semantic-fact operations are not required by
  the first frontend slice unless a later approved workflow adds them;
- CLI and structured output over the same semantic operations;
- fake Orchestrator/engine/plugin peers for deterministic frontend testing;
- restart, timeout, cancellation and partial-failure fixtures.

## Kolmogrov transfer gate

Kolmogrov yields admitted results to Engine when its own proof and measurement
records permit. Frontend consumes only the Engine contract and does not wait for
or directly bind Kolmogrov during 001.

## Fixture law

Before a real adapter gate passes, the frontend uses deterministic ports and
marks their data `simulated`. A fixture may exercise UI state but may not be
cited as evidence that a provider contract exists or works.
