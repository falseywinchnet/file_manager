# Search match evidence and submitted context — 2026-10-03

Status: **MEASURED Windows integration; native macOS/Linux checks pending.**
This repairs lost result context and misleading correspondence provenance. It
does not activate indexed substring search or establish a speed improvement.

## Behavior and authority

The rebuilt C++ source client retains optional rank, certainty and ordered
evidence, including unknown claims and owned inert details JSON. Its numeric,
optional-state and whole-page failure contract is
`../../../orchestrator/spec/FRONTEND_SEARCH_MATCH_CONTEXT.md`; focused client
evidence is in
`../../../orchestrator/conformance/evidence/SEARCH_MATCH_PROJECTION_2026-10-03.md`.
No Engine storage, wire operation, capability or stable binary ABI changed.

SearchWork and CriteriaWork create one immutable submitted-request owner before
the call. The call borrows those exact fields. Accepted rows share that owner
through their page provenance; query/filter/scope/cursor strings are not copied
per row. Cancellation and rejected rows retain no published partial page.
Duplicate rows keep their original source/request pair. The final retained row
releases page and request storage. An absent request remains explicitly absent
for older fixtures rather than being synthesized from current controls.

Correspondence rebuilding now reads each row's own page and record generation.
Appending generation 28 no longer relabels an existing generation-27 row or its
generation-26 record. The brief match text describes the first evidence record
in provider order. Unknown kind/channel combinations remain generic and reported
inference is visible; an unknown source is not labeled live. The existing live
provider's exact_name evidence covers name/path substring matching, so its text
does not promise an exact filename match. No score comparison, rank-driven
reordering, cached-predicate revalidation or identity-equivalence claim is added.

## Validation

**MEASURED:** Shadow Windows, GCC 16.2, Release frontend, existing matching
development SDK at `.build/details-sdk`, one C++ compiler job:

```text
cmake --build .build/details-consumer --parallel 1
ctest --test-dir .build/details-consumer --output-on-failure
```

All 15 suites passed, 5.32 seconds. `WindowsLastTest.log` preserves final output.
The first integration pass also passed 15/15, 5.40 seconds; source review then
tightened unknown-channel wording, used named request fields and added a focused
unknown/inferred/ordered-claim regression before the final rebuild above.

Added/extended application checks exercise original submitted fields after the
caller's copy is mutated and retired; one shared owner across accepted rows;
last-row retirement; search and Criteria publication; per-page continuation;
duplicates; mixed-generation append; unreported and unknown source/evidence;
provider order and inference disclosure. Existing cancellation, obsolete reply,
replacement, navigation, operations, preview, picker and platform tests pass.
The combined query/filter ownership fixture is private storage test data, not
admission of a combined public search route.

Orchestrator `cargo fmt --check`, full locked serial tests and all-target/
all-feature warning-free Clippy passed during integration:

```text
cargo fmt --manifest-path orchestrator/Cargo.toml --check
cargo test --manifest-path orchestrator/Cargo.toml --locked --jobs 1 -- --test-threads=1
cargo clippy --manifest-path orchestrator/Cargo.toml --all-targets --all-features --locked --jobs 1 -- -D warnings
```

80 Windows-selected tests passed (63 unit, five CLI, four contract, six broker,
two fixture); Unix-only integration modules select zero tests on this host.
No Rust source or golden fixture changed. These results do not replace native
Unix consumer validation, especially floating-point from_chars availability.

## Source review and limits

Reviewed against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`: new request
and presentation records; prepare_search_page owner construction/transfer;
describe_search_match; SearchWork/CriteriaWork call and completion changes;
Criteria signature cleanup; correspondence publication changes; new/changed
test probe and assertions. Review covered explicit initialized types, named
callbacks and retained state, const borrows, shared lifetime, cancellation order,
failure boundaries, provider/current authority and repeated storage. The nine
touched C++ files have zero spelling findings; this is supplemental evidence.
The sibling's exact source-client semantic review is recorded separately and
root reviewed its added projection helpers and all four added test functions.
Untouched Application/parser/JSON escaping code is not newly certified.

Request construction copies job arguments once, then moves the owned record into
immutable shared storage. Correspondence still rebuilds accumulated cards and
allocates display strings, including the generation conversion. Details JSON
encoding also allocates. No allocation-free, throughput, latency or global
retained-result memory bound is claimed. Native input/pixel acceptance, current
filesystem identity comparison, thumbnails and indexed substring activation
remain open. Existing downloadable packages do not yet contain this change.
