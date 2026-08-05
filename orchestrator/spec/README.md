# Canonical Orchestrator specifications

This directory is the source of truth for cross-project semantics. The registry
assigns ownership and routes readers to the individual contracts.

The provider-independent bootstrap has an executable handwritten projection of
its fixture-draft families. ADR-007 additionally freezes the Engine semantic-v0
query and administration meanings and provides a Rust fixture projection while
the runtime adapter remains unavailable. A later accepted decision may select
an IDL and generators per negotiated interface, but generated files will live
outside this directory and will not replace these semantics.

Read [`CONTRACT_REGISTRY.md`](CONTRACT_REGISTRY.md) first.
