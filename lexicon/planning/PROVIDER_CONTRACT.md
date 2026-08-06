# Lexicon provider contract

Status: **CANDIDATE semantic contract; no runtime operation admitted**.

## Lookup request

The smallest request contains:

```text
request_id
provider/corpus preference or default
literal input term
normalization policy identifier
locale/language
maximum headwords / senses / bytes
deadline / cancellation
accepted result schema version
```

The caller does not supply SQL, paths, arbitrary predicates, UI overrides or a
request to mutate the corpus.

## Lookup result

Each result distinguishes:

- exact display-form match;
- normalized/case-folded match;
- explicitly recorded spelling/inflection variant;
- suggestion, which is a different and later candidate channel.

It returns bounded headwords and senses with stable package-local IDs, display
form, part of speech, concise definition, usage/domain labels if available,
source/edition/license attribution, attestation locator, provider/package
version and confidence/status appropriate to the source. Missing, unavailable,
budget-exhausted, cancelled, corrupt-package and incompatible-version are named.

## Search merge law

- Lexicon is invoked only when enabled and the query shape admits lexical
  lookup. Exact trigger grammar is open.
- Its result is a typed lexical result/card, never a fake path or file row.
- File similarity/ranking does not automatically demote or promote it. The host
  composes result classes deliberately.
- Provider failure cannot delay or erase Engine file results past a bounded
  deadline.
- Lexicon may not mutate a query, record a personal semantic fact, or deposit
  into Engine through this contract.

## Corpus enumeration

A separate bounded operation accepts a versioned `CorpusProfileId` and returns
sorted/paged word records with normalization, display form, length/shape fields,
eligibility flags and provenance. Consumers pin a generation for the whole job.
Changing the installed corpus produces a new generation; a Crossword generator
never observes half of each.

## Placement candidates

### Trusted first-party provider process/library

Gains small startup/IPC cost and controlled signed data. Loses some hostile-data
containment and creates a privileged parser/update path.

### Supervised first-party plugin worker

Gains reef containment and exercises provider capabilities. Loses process
startup/memory and must not receive generic plugin powers merely because it is
first-party.

### Engine search extension

Gains reuse of index primitives and perhaps query speed. Loses authority
clarity, burdens the systemwide file engine with non-file schemas, and risks
mixing result identities. Retain only as an internal acceleration adapter after
the semantic provider boundary is proven; reject Engine ownership of senses.

**CANDIDATE recommendation:** Orchestrator-visible provider semantics with an
independently built Lexicon store; decide trusted process versus supervised
worker after source/update threat analysis. Engine is not the authority.

## Bounds and conformance

- fixed maximum input length, normalized forms, senses, definition bytes and
  relations per response;
- deterministic normalization under pinned Unicode/data versions;
- cancellation and no unbounded result streaming;
- corrupted index/table and adversarial source fixtures;
- exact fixtures for homographs, inflections, punctuation, case, archaic forms,
  missing terms and conflicting sources;
- cold/warm lookup latency, RSS, page reads and startup measured separately.
