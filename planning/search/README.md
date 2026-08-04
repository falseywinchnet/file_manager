# Search and indexing architecture interview

Status: **pre-architecture decision workbook**.

This directory turns the Zeta search extraction and the grand architect's
requirements into choices, questions, and falsifiable experiments. It does not
select a database, service language, index format, sharding policy, ranking
model, semantic representation, or synchronization design.

Read in order:

1. `search-architecture-board.html` — interactive decision board containing
   every question below plus the original search-related question atlas.
   Answers persist locally in the browser and can be exported as Markdown or
   JSON. Rebuild it with `python3 planning/search/build_search_board.py`.
2. `SEARCH_SYSTEM_OPTIONS.md` — algorithms, stores, update models, shards, and
   semantic-hive boundaries.
3. `SEARCH_RECONCILIATION_001.md` — explicit constraints recovered from the
   first completed decision-board export and follow-up clarification.
4. `SEARCH_ARCHITECT_INTERVIEW.md` — numbered decisions with gains, losses,
   alternatives, and follow-up questions.
5. `../../research/zeta_search/ALGORITHM_INVENTORY.md` — observed Zeta
   mechanisms and negative results.
6. `../../research/zeta_search/EXPERIMENT_BACKLOG.md` — correctness and
   performance gates.

The filesystem remains authoritative unless the architect explicitly admits a
second class of user-authored knowledge whose survival and synchronization
rules differ from the disposable search projection.
