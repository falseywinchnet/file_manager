# Search and indexing architecture interview

Status: **pre-architecture decision workbook**.

This directory turns the Zeta search extraction and the grand architect's
requirements into choices, decisions, questions, and falsifiable experiments.
The service language and engine architecture spine are now accepted; low-level
codecs, later rank fusion, Kolmogrov transfer details, semantic representation,
and synchronization remain bounded open work.

Read in order:

1. `search-architecture-board.html` — interactive decision board containing
   every question below plus the original search-related question atlas.
   Answers persist locally in the browser and can be exported as Markdown or
   JSON. Rebuild it with `python3 planning/search/build_search_board.py`.
2. `SEARCH_SYSTEM_OPTIONS.md` — algorithms, stores, update models, shards, and
   semantic-hive boundaries.
3. `SEARCH_RECONCILIATION_001.md` — explicit constraints recovered from the
   first completed decision-board export and follow-up clarification.
4. `../../decisions/ADR-001-ENGINE-REFERENCE-AND-STORAGE-SPINE.md` — accepted
   object model, root/volume topology, storage spine, update, ranking, API,
   placement, scale, and performance direction.
5. `../../engine/docs/ARCHITECT_HANDOFF_001.md` — mandatory worker-local shim of
   that accepted direction.
6. `../../engine/docs/ARCHITECT_HANDOFF_002.md` — mandatory distinction between
   core Kolmogrov candidates and Orchestrator-managed semantic memory.
7. `../../orchestrator/spec/CONTRACT_REGISTRY.md` — canonical cross-project
   engine, provider, hive, and interrogation contract inventory.
8. `SEARCH_ARCHITECT_INTERVIEW.md` — numbered decisions with gains, losses,
   alternatives, and follow-up questions.
9. `../../research/zeta_search/ALGORITHM_INVENTORY.md` — observed Zeta
   mechanisms and negative results.
10. `../../research/zeta_search/EXPERIMENT_BACKLOG.md` — correctness and
   performance gates.

The filesystem remains authoritative unless the architect explicitly admits a
second class of user-authored knowledge whose survival and synchronization
rules differ from the disposable search projection.
