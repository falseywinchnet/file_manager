# Search source records — 2026-10-03 UTC

Status: **MEASURED Windows source-client/application repair; native CI pending.**
This is a prerequisite for trustworthy cached search and indexed-thumbnail
binding, not thumbnail implementation or measured search acceleration.

## Change and authority

The existing C++ client now preserves Engine object root/ID/incarnation/platform
keys, record generation, stored mode and modification time. Stored size now
uses the producer's signed 64-bit domain. Runtime and semantic identity aliases
must agree if both report values. Missing/null optional values stay unreported;
reported empties and zero are preserved. Integer overflow and wrong types fail
the whole response. No path-derived identity or page-derived record generation.

Worker preparation moves accepted provider records into rows alongside the
independent current filesystem observation. One immutable shared page owner
retains lane, scan ID and page generation. UI search/Criteria publication retains
these records for displayed rows, keeps original pairs for skipped duplicates,
and retires them on replacement or successful directory publication. Cancelled
or obsolete completions do not alter the displayed records. A failed navigation
retains the old displayed rows and their corresponding source records.

Canonical reconciliation is
`../../../orchestrator/spec/FRONTEND_SEARCH_SOURCE_RECORDS.md`; frontend dialogue
is in `../../planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`. Rebuild C++ source
consumers together. There is no new wire operation, provider grant or stable ABI.
Cached facts never replace current display/operation facts. Windows Engine and
frontend identities still have different representations and are not compared.
Rank/certainty/evidence, eligibility lookup and exact binding remain open.

## Validation and retained failures

**MEASURED:** Shadow Windows, MinGW GCC 16.2 Release, one compiler, existing
development GUI.Forms SDK. Final all 15 CTest suites pass in 5.01 seconds;
`WindowsLastTest.log` retains their output. Tests cover:

- Owned identity after response retirement; exact int64 endpoints, uint32/uint64
  maxima and zero; row/page generation distinction; optional/empty values.
- Runtime/semantic aliases, conflicts, malformed strings/maps, out-of-range and
  fractional/exponential stored integers, and whole-response refusal.
- Deliberately different current/provider name, kind, size, identity and mtime;
  cancellation and rejected paths; original observation after fixture removal.
- Shared 64 KiB scan token across two rows, surviving one-row retirement and
  releasing with the final row; search/Criteria append and duplicates;
  replacement, obsolete/cancelled deliveries and directory retirement.

`cargo fmt --check`, full `cargo test --locked --jobs 1 -- --test-threads=1`
and `cargo clippy --all-targets --all-features --locked --jobs 1 -- -D warnings`
pass. The Windows-selected Rust suites total 76 tests; Unix-only integration
modules run zero tests on this host. See `OrchestratorTests.log` and `Clippy.log`.

The first default-parallel Rust run failed one existing Windows host test while
reading newly published discovery: OS error 32, sharing violation, at
`src/service/windows.rs:422` (`private record`). 58/59 unit tests passed. The
same test passed alone and the complete serial suite subsequently passed. No
Rust host code or test assertion was changed; the underlying intermittent
publication/read failure is not claimed repaired.

The first serial run then correctly failed the release golden fixture because
the amended canonical registry participates in provenance. The two golden
response digests were updated from `509080821c915474c5c3d564c12948bd57cce3fa1a384939a1220816747b5bf5`
to `506543a9c12fef0724da6e64d23fc03d523aab644075c8b1f25ca55f36154a5e`.
No other fixture facts or test rules changed. `BeforeGoldenUpdate.log` retains
that failure; the final complete serial suite passes the exact fixtures.

## Source review

Reviewed the new identity type and SearchResultInfo fields; signed parser,
alias/identity/revision helpers and result projection; paired preparation and
shared-page owner; both Application publication loops and all entry-clear
paths; complete new/changed tests. Applied the full
`planning/PROGRAMMING_HOUSE_STYLE.md`: explicit initialized types, named behavior,
read-only inputs, exact conversions, no retained response/native borrows,
independent source/current authority, cancellation order, repeated allocation
and ownership retirement. Eight touched C++ files report zero spelling findings.
Untouched parser/Application code and dependencies are not certified.

The visible Details sibling independently found per-row copies of page-level
strings in the first implementation. Root replaced them with the shared owner
and added the lifetime/large-token assertions. Its read-only re-review found the
issue resolved and no further actionable defect; it did not run tests. Aggregate
source retention still grows with displayed results; no global query-cache cap
or latency/RSS improvement is claimed. Native compilation and real interaction
remain separate acceptance evidence.
