# Orchestrator conformance and versioning constitution

Status: **bootstrap fixtures active; codec/IDL and cross-language peers unresolved**.

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
