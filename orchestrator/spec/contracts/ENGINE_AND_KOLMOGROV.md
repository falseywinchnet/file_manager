# ORC-ENG / ORC-KOL: engine and Kolmogrov contracts

Status: **engine v0 import plus accepted direction**.

The Go engine remains the provider of exact file-object catalogue, root-scoped
query, intrinsic metadata, lexical retrieval, and core fuzzy candidates.

Kolmogrov is a planned first-class core fuzzy-candidate mechanism, not a
semantic plugin. The complete engine must expose:

- configuration/family/version identity;
- exact source-feature policy;
- fixed hash widths and independently addressable channels;
- candidate limits and deterministic continuation;
- raw per-channel distance/evidence;
- exact catalogue verification;
- fallback/control identity during development;
- rebuild and migration behavior when the hash family changes.

The early engine may use conventional transposition, character-gram, and bounded
edit verification controls while Kolmogrov is formalized. The storage and query
planner must not encode assumptions that make later Kolmogrov integration an
external provider or schema-breaking retrofit.

The Oracle registers both query and administrative engine contracts. Query
consumers cannot gain root admission, integrity repair, or rebuild authority by
changing method names or transport.

Source imports:

- `../../engine/api/`
- `../../engine/docs/API_CONTRACT.md`
- `../../engine/docs/ARCHITECT_HANDOFF_001.md`
- `../../engine/docs/ARCHITECT_HANDOFF_002.md`
- `../../kolmogrov/docs/FORMAL_PROGRAM.md`
