# Architect handoff 002 — Kolmogrov core and Oracle boundary

Status: **DECIDED direction; implementation details remain gated**.

This handoff corrects boundary language that predates the grand architect's
separation of fuzzy hashing from semantic memory. It supplements
`ARCHITECT_HANDOFF_001.md`; where the two appear to conflict, this handoff
controls.

## ENG-K01 — Kolmogrov is a core candidate channel

**GIVEN:** the production engine is to be designed around a rigorously extended
Kolmogrov/ConeDAG-derived fixed-width similarity mechanism as a core fuzzy and
structural candidate channel. It is not an AI-semantic provider and is not to be
relegated to the general plugin runtime.

This does not waive transfer gates. The independent `../kolmogrov/` program must
supply a pinned specification, proof obligations, reference vectors, adversarial
fixtures, quality measurements, and a versioned encoder/distance contract. Until
then, character grams, deletion dictionaries, edit distance, and exhaustive
scans are controls and valid interim implementations—not the declared final
production target.

The exact catalogue remains authoritative. Kolmogrov hashes propose bounded
candidates and attach evidence; they never replace platform identity, exact
metadata, exact stored records, or post-candidate verification.

## ENG-K02 — Semantic memory is outside the engine

**GIVEN:** requests such as “the pecan pie recipe from the winter of 22” require
interpretation and remembered context. That semantic-memory system is not part
of the Go catalogue engine and is not supplied by Kolmogrov hashing.

Semantic facts, plugin-derived annotations, model provenance, and personal
memory belong beyond the critical engine boundary in Oracle-managed hives. The
engine may expose exact records and query primitives and may accept explicitly
versioned read-only candidate/evidence inputs through an Oracle-owned contract.
It must not absorb an unbounded semantic document store or allow plugin writers
to mutate its authoritative catalogue.

## ENG-K03 — The Oracle owns the cross-project contract registry

**DECIDED:** cross-project API and ABI semantics are registered under
`../orchestrator/spec/CONTRACT_REGISTRY.md`. The engine owns its implementation
and may keep transport-neutral local types, but any surface consumed by Oracle,
the File Manager frontend, plugins, or generated bindings must map to a registry
contract and version.

Engine work that needs a new cross-project call must submit a proposal under
`../orchestrator/proposals/`; it must not make an implementation accident into a
program-wide ABI. Oracle contract ownership does not require hot query traffic
to be proxied through the Oracle process. Routing and process placement remain
explicit contract decisions.

## Required delivery effect

1. Preserve the current exact and lexical vertical slices.
2. Keep the similarity seam independently rebuildable and evidence-preserving.
3. Treat Kolmogrov transfer as a named production-readiness dependency, not an
   optional plugin enhancement.
4. Keep semantic interpretation, hives, handlers, settings, and plugin lifecycle
   out of this engine.
5. Publish conformance fixtures and versioned contracts for every exported
   engine interaction before Oracle integration begins.
