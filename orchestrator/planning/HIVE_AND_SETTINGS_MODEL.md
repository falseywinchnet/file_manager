# Oracle hives, registries, and settings

Status: **authority classes accepted; physical stores unresolved**.

## Do not build one universal database

The Oracle coordinates several stores because their authority and lifecycle
differ. They may share a storage library after measurement; they do not share an
undifferentiated schema or corruption boundary by convenience.

## Semantic-memory hive

Purpose: durable interoperable memory for concepts, temporal interpretations,
explicit corrections, relationships, and exact file-object anchors.

Required record classes:

- `UserFact`: explicitly authored/confirmed; durable authority with audit;
- `ProposedFact`: AI/plugin proposal, not yet equivalent to user authority;
- `DerivedFact`: reproducible model/extractor result with source/version;
- `Relation`: typed directional or symmetric link with provenance;
- `Interpretation`: one of possibly several meanings, with confidence/context;
- `Tombstone`: deletion or unavailable-source lineage without fabricated fact.

The hive stores memory; it does not run the model that creates an interpretation.
Conflicting facts remain representable. “Winter of 22” may identify several
intervals until context or a user decision resolves it.

## Provider hives

Each provider receives an independent namespace keyed by package/plugin,
extractor/model version, schema, root policy, and exact source generation.
Typical records include OCR, extracted text, captions, media fields,
fingerprints, and provider-local search structures.

Deposit lifecycle:

1. declare schema and required source/privacy classes;
2. obtain a scoped quota and generation lease;
3. emit bounded batches keyed by exact file-object anchor;
4. validate type, provenance, source generation, and limits;
5. publish the complete generation atomically;
6. retire obsolete generations and propagate source/root erasure.

Plugins never receive storage paths or page handles. A malformed or oversized
generation is rejected/quarantined without affecting engine or other hives.

## Operational registries

Separate authoritative registries cover:

- packages and plugin identity;
- grants and capability decisions;
- File Manager-local handlers/type associations;
- declarative context commands;
- provider schemas and quotas;
- platform integration desired/observed state.

These are backed up/migrated as operational configuration, not semantic memory.

## Settings

Core and plugin settings use declarative schemas with stable field IDs, types,
bounds, defaults, localization keys, sensitivity, restart effect, and migration.
Values are namespace-owned by the Oracle; File Manager and CLI render/edit them
through the same transactions.

- no arbitrary settings file access for plugins;
- secret fields use opaque platform-secret handles if admitted;
- schema migration is pure and capability-free;
- invalid values do not widen grants;
- reset distinguishes core registry, plugins/packages, provider data, semantic
  memory, view state, and integration state.

## Quotas and anti-war policy

Every provider has limits for total bytes, bytes/file, records/file, schema
fields, ingestion rate, publication frequency, query CPU/wall time, result count,
and retained generations. Global and per-root ceilings prevent many individually
legal providers from exhausting the machine.

The Oracle exposes usage and rejection counters. A plugin crossing a hard limit
loses the current generation or job, not the integrity of a shared critical
store.

## Sync boundary

Future synchronization may include explicit user facts and approved coarse
catalogues. Provider-derived data and settings do not become synchronizable by
default. Every synced class needs encryption, conflict, erasure, device identity,
and newer-schema behavior before admission.
