# Rejected exhaustive exact-name first-page plan 001

Status: **REJECTED as the default simple-name plan; retained when semantics
require exhaustive filtering or ordering**.

At one million files, 100,000 bindings share the exact basename `repeated`.
The generic M2 pipeline materialized and read every candidate before returning
the canonical first 100. Across 20 samples it measured p50 33.506 ms, p95
44.719 ms, p99/max 45.273 ms. This violates the accepted 8 ms p95 exact-query
gate even though the underlying ordered range can retrieve the page directly.

The replacement uses a generation-bound range window only when all of these are
true: exactly one approved root, full-root descendant scope, exact name only,
canonical path order, and no metadata predicates. Cursors retain the same query
fingerprint and global ranks. Across 2,000 one-million-record samples the exact
pipeline measured p50 47.375 us, p95 56.209 us, p99 125.250 us, and max 176.209
us with the same `a69b9113a32327fd` correctness digest.

Nested-root exclusions, narrower scopes, metadata filters, and alternate sorts
still use exhaustive bounded validation. Reversal requires a plan that preserves
those semantics with less work, not a silent skip.
