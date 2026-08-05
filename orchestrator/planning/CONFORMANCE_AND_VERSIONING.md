# Orchestrator conformance and versioning constitution

Status: **bootstrap and Engine semantic-v0 fixtures active; codec/IDL and
cross-language runtime peers unresolved**.

## Independent version namespaces

Never use one global “File Manager API version.” Track independently:

- semantic contract family;
- wire framing;
- payload/IDL schema;
- C client ABI;
- GUI.Forms ABI;
- engine query and administration APIs;
- Kolmogrov family/configuration;
- plugin package manifest;
- worker protocol and each extension contract;
- hive schema and exported interchange;
- settings schema;
- CLI grammar/structured output;
- platform integration contract.

Compatibility negotiates the intersection. A newer endpoint never unilaterally
selects semantics unknown to the peer.

Orchestrator Core 1.0 is a release/readiness profile over named compatible
contract versions, not an exception to this rule. Its executable release
manifest remains `development` until every requirement in
[`CORE_1_0_RELEASE.md`](CORE_1_0_RELEASE.md) passes.

## Contract artifact ladder

Every contract advances through:

1. `outline` — operation and authority named;
2. `semantic-draft` — types, lifecycle, errors, limits, cancellation complete;
3. `fixture-draft` — canonical examples and adversarial cases;
4. `frozen-v0` — owner approval for experimental implementation;
5. `implemented` — at least one provider and one independent consumer;
6. `conformant` — differential and cross-version suites pass;
7. `stable` — explicit compatibility horizon accepted.

Code existence cannot skip a stage.

A `fixture-draft` may have a deliberately disposable executable laboratory
projection, as the bootstrap does. That does not promote the contract to
`implemented`; that stage still requires an independently built provider and
consumer against the accepted fixtures.

ADR-007 freezes the transport-neutral `ORC-ENG-001/002` semantic-v0 subset for
experimental implementation. Its Rust fixture projection proves that
Orchestrator preserves cached-stale, authoritative no-match, provider-absent,
and currently unsupported-live-query outcomes as distinct states. ADR-008 now
requires that missing lane through the separate `ORC-ENG-004` negotiation. Its
draft fixtures prove source-explicit progressive pages, partial traversal, and
non-bypass of authority errors.

**OBSERVED:** the Rust development JSONL adapter now performs a real
cross-process exact query against a separately built Go Engine after canonical
root plan/apply and reconciliation. This is implementation evidence for
`ORC-ENG-001`; it does not promote JSONL to the production wire or close the
installed discovery/authentication gate. `ORC-ENG-004` still lacks its Engine
provider implementation and remains negotiating.

## Required conformance corpus

For each boundary preserve:

- minimal valid request/reply/event;
- every terminal error family;
- unknown optional and unknown critical fields;
- version downgrade and incompatible-major behavior;
- fragmented/coalesced frames and abrupt EOF;
- cancellation before, during, and after terminal response;
- timeout, backpressure, quota, and oversized payload;
- stale/wrong generation, nonce, identity, and capability;
- partial multi-provider/shard failure;
- deterministic replay digest.

Cross-language consumers decode and re-encode the same canonical semantic
fixtures. Where encoding is intentionally noncanonical, semantic equality rather
than bytes is tested and signing/digest rules use a separately canonical form.

## ABI rules

- C ABI functions are version-prefixed or retrieved from a sized function table;
- structs carry size/version and fixed-width fields;
- ownership and thread affinity are explicit;
- no allocator mismatch, exception, panic, RTTI, STL, Rust layout, or Go pointer
  crosses the boundary;
- asynchronous work uses pollable queues/futures/events, not uncontrolled
  callbacks;
- symbol and layout tests run on every target architecture.

## Dependency locks

Every generator, schema compiler, and runtime codec is version-locked and
reproducible offline. Checked-in generated artifacts name
their source contract revision and generator digest. Hand-edited generated files
fail CI.
