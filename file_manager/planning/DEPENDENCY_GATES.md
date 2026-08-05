# Frontend dependency gates

Status: **mandatory waiting contract**.

## GUI.Forms consumption snapshot

Required before frontend implementation:

- named experimental C ABI and C++ wrapper release;
- retained object lifetime and UI-thread rules;
- application/window/host lifecycle;
- text storage, grapheme behavior, shaping seam and IME events;
- invalidation/damage and rendering-host behavior;
- core control/container/range/menu/tree/list primitives needed by the first
  File Manager window;
- theme/resource/PNG/language seams;
- headless deterministic traces plus macOS host demonstration;
- clean dependency-facing build/install target without demo/build-tree coupling.

Semicomplete means the first File Manager slice does not have to invent or reach
inside these foundations. It does not mean every WinForms-compatible control is
finished.

## Engine consumption snapshot

Required before frontend implementation:

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

## Oracle consumption snapshot

Required before frontend implementation:

- master registry entries used by the first frontend are `frozen-v0`;
- authenticated session/lifecycle client;
- immutable handler and context-command snapshots;
- settings schema/value transaction;
- engine query broker or registered direct-engine relationship;
- semantic/provider hive unavailable and empty behaviors;
- plugin preview/thumbnail request and failure envelope for the first macOS
  slice;
- CLI and structured output over the same semantic operations;
- fake Oracle/engine/plugin peers for deterministic frontend testing;
- restart, timeout, cancellation and partial-failure fixtures.

## Owner gate

Passing technical gates does not start implementation automatically. The grand
architect explicitly transitions direction from Oracle to frontend and opens
Gate F0.
