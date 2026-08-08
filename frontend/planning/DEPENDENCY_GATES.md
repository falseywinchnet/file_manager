# Frontend gates

Status: **DECIDED split gates under ADR-004 and ADR-006**.

The earlier global Gate F0 is superseded. Frontend 001 has two independent
foundation gates—Orchestrator Core 1.0 and GUI.Forms FM0—plus explicit architect
start direction. The architect direction was recorded on 2026-08-07 in
[`OWNER_DIRECTION_2026-08-07.md`](OWNER_DIRECTION_2026-08-07.md); the two
technical foundation gates remain open. Other provider adapters retain their
own later gates.

## Orchestrator Core 1.0 bootstrap gate

Required before Frontend 001 product implementation:

- a named Core 1.0 release manifest reports the bootstrap profile ready;
- `ORC-COM-001`, `ORC-LIF-001`, `ORC-FE-001`, and `ORC-CLI-001` have the
  accepted Core 1.0 compatibility horizon;
- user-scoped discovery, authentication, session, restart, and shutdown are
  available on the first target platform;
- the C++-suitable client projection passes independent conformance fixtures;
- contract and availability snapshots truthfully distinguish available,
  degraded, unavailable, deferred, and stubbed providers;
- malformed, oversized, incompatible, timed-out, and cancelled requests have
  bounded terminal behavior.

Engine, Kolmogrov, plugin execution, and semantic facts need not be functional
for this gate. The live Orchestrator must report their real state.

ADR-008's catalogue-independent Engine fallback therefore does not block the
Frontend 001 start gate. It does block a claim that File Manager search is
ready: until `ORC-ENG-004` is available, the frontend presents search as
provider-limited while retaining live folder navigation.

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

Passing Core 1.0 and the GUI.Forms snapshot does not start implementation
automatically. The grand architect explicitly directs File Manager frontend
work to begin. Engine, Kolmogrov, plugin execution, and semantic facts cannot
veto Frontend 001 when the live Orchestrator reports their reduced state.

## Engine consumption snapshot

Required before replacing the Frontend 001 Engine fixture port:

- exact object/binding model and approved-root ownership;
- immutable reader generations and sandbox scanner;
- exact name/path/basic-metadata query, inspect, pagination and status;
- JSONL golden fixtures and registered private local protocol direction;
- integrity/staleness/unavailable behavior;
- bounded `ORC-ENG-004` catalogue-independent name/path fallback, with explicit
  live source, partial state, containment, budgets, cancellation, and no hidden
  catalogue side effect;
- representative query/resource baseline;
- Kolmogrov configuration/candidate seam reserved as a core contract, with
  conventional controls permitted during research completion;
- no frontend dependency on engine private Go packages or store layout.

“Mostly running” means useful exact search, the required reduced live fallback,
and stable consumption semantics, not final compaction, federation, semantic
providers, or every performance target.

## Later Orchestrator capability gates

Settings transactions, immutable handler/context-command snapshots, the Engine
query broker, plugin supervision, hives, and semantic facts enter when their own
contract families pass. They are not silently included in Core 1.0 merely
because the daemon itself is available.

## Kolmogrov transfer gate

Kolmogrov yields admitted results to Engine when its own proof and measurement
records permit. Frontend consumes only the Engine contract and does not wait for
or directly bind Kolmogrov during 001.

## Fixture law

Frontend tests may use deterministic ports and Core 1.0 fixture replay. Product
bootstrap uses the live Orchestrator. Other simulated provider data is marked
`simulated` and may not be cited as evidence that its provider works.
