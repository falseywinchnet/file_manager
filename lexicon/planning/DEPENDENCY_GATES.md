# Lexicon dependency gates

Status: **mandatory; research open, implementation closed**.

## LX0 — architect interview closure

- runtime classification criteria and first platform selected;
- first lookup trigger/presentation and enabled-by-default policy;
- initial dictionary source fields and language scope;
- Shakespeare edition/profile rules;
- normalization, homograph, inflection and suggestion behavior;
- package/update/uninstall/privacy model;
- Crossword clue policy and later corpus selection boundaries.

## LX1 — source and license gate

Exact source artifacts, digests, rights, licenses, attribution, redistribution
and importer replay are reviewed. No source is packaged merely because its text
appears old or accessible online.

## LX2 — reference data model gate

Source, attestation, headword, sense, form relation, corpus membership and clue
records have deterministic fixtures and migrations. Normalization results are
reproducible under pinned data versions.

## LX3 — Orchestrator contract gate

The lexical provider lookup, availability, bounds, cancellation, result typing,
settings/audit and corpus-generation semantics are reconciled. Provider UI and
Engine fact mutation are forbidden in executable conformance fixtures.

## LX4 — placement/threat gate

Trusted process/library versus supervised first-party worker is selected through
source/update/parser threat analysis and measured startup/query costs. Engine
acceleration, if any, remains behind the same semantics and does not own senses.

## LX5 — owner start gate

The grand architect explicitly opens Lexicon implementation for a named source
profile/platform. Passing research gates does not start code.

## LX6 — Crossword consumption gate

A pinned Shakespeare corpus generation supports bounded enumeration, prefix/
shape candidates and clue provenance. Games validates it against fixed grid
fixtures before Crossword claims availability.

## LX7 — release inclusion gate

Requires reproducible packages, licenses/attribution, lookup and corpus
conformance, performance/security evidence, installer/uninstaller behavior,
local docs, migrations, platform support and accepted known issues.
