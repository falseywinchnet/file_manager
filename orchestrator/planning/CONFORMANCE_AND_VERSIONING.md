# Orchestrator conformance and versioning constitution

Status: **macOS Core 1.0 bootstrap and independent C++ source-client conformant;
other platform projections and later-family codec/IDL work remain open**.

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
manifest reports `ready` only because every requirement in
[`CORE_1_0_RELEASE.md`](CORE_1_0_RELEASE.md) now passes.

ADR-010 starts the stable Core bootstrap, structured CLI, and installed local
wire namespaces at 1.0 on the first macOS artifact. Pre-release 0.1 peers are
negative fixtures, not supported consumers. Later Core 1.x minors preserve 1.0
bootstrap meanings and add only optional fields or new methods; incompatible or
new must-understand semantics require another major.

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

**OBSERVED:** `conformance/clients/cpp/` is independently configured and built
as C++17, then authenticates to the Rust daemon, materializes the single-read
Core bootstrap snapshot including contracts, availability, routes and service
controls, verifies the golden release digest, shuts down, and reconnects to a
second daemon instance at the same runtime location. Together with the Rust
hostile/bootstrap fixtures and ADR-010 horizon this satisfies the macOS Core 1.0
consumer gate. Windows named pipes remain a separate platform projection.

ADR-007 freezes the transport-neutral `ORC-ENG-001/002` semantic-v0 subset for
experimental implementation. Its Rust fixture projection proves that
Orchestrator preserves cached-stale, authoritative no-match, provider-absent,
and unsupported-live-query outcomes as distinct states. ADR-008 requires the
separate `ORC-ENG-004` lane. Its fixtures prove source-explicit progressive
pages, partial traversal, and non-bypass of authority errors.

**OBSERVED:** the Rust development JSONL adapter performs real cross-process
exact and catalogue-independent live queries against a separately built Go
Engine. The live route passes through authenticated `orchestrator.search` in
both Rust and independent C++ consumers without exposing lane selection. This
is implementation evidence for `ORC-ENG-001` and the fixture-draft
`ORC-ENG-004`; it does not promote JSONL to the production wire or close the
installed discovery/authentication, native NTFS/ext4, or million-entry gates.

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

**OBSERVED:** the Core release manifest uses length-delimited fields and
embedded inputs under a domain-separated SHA-256 digest rather than relying on
JSON object byte order. Its golden response pins the result. The digest is
content identity, not a signature or a packaged-binary attestation.

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
