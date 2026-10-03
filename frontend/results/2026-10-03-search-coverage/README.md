# Search coverage projection and presentation

2026-10-03 UTC; Windows Shadow session. This finishes the paused source checkpoint
`9e25b05`, rebased onto the verified PR 9 merge `8f20cca` without changing that
release's source. Canonical reconciliation:
`../../../orchestrator/spec/FRONTEND_SEARCH_COVERAGE.md`.

**OBSERVED defect:** the C++ search client discarded coverage fields already
forwarded by Orchestrator. Empty displayed result sets used unconditional
no-match wording, and a later page had no retained representation of earlier
stale/unavailable/warning coverage. Cached criteria matches acquired fresh path
metadata without enough disclosure of the cached match's limitations.

## Implementation and scope

The source client now owns optional stale roots, unavailable roots/paths,
warnings and scan identity. Absent/null stays unreported, explicit empty stays
reported empty, and malformed non-null types fail before page return. The
existing result shape is otherwise unchanged. A private projection function is
shared by the real transport call and focused tests. Native strings/vectors are
not a new wire ABI. Consumers must rebuild with the changed C++ source library.

Frontend preparation carries the page annotations into the UI. UI-owned flags
retain gaps across appended pages and reset on replacement. The primary status
states how many matches are shown, plus the most consequential coverage state;
the secondary description retains the other states. Cached matches say they
may have changed. An incomplete page without a cursor offers no continuation.
Provider warning text remains in the owned page but is summarized in the UI;
the accumulated UI model stores flags, not a full browsable evidence history.

No row identity equivalence, fresh criteria revalidation, signed size repair,
new indexed substring route, provider currentness or performance improvement
is claimed. Those are independent open requirements. Existing source cursor,
generation/shutdown checks, worker scheduling and filesystem authority remain.

## Verification

**MEASURED:** matching GNU/MinGW Release build and all 13 local CTest suites pass
in 2.94 seconds. `WindowsLastTest.log` preserves the test output. The suite
includes the new independent C++ projection target and the application coverage
scenario. Nine changed C++ files produce zero spelling-regression candidates.
Tests cover:

- Owned ordered coverage strings after input retirement; catalogue/live shape;
  absent/null/empty optionality; malformed list/scalar/member rejection.
- Empty partial results, More results eligibility, final-page transition,
  previous gaps retained across append, and reset on a replacement live query.
- Missing coverage, incomplete responses without continuations, cached criteria
  disclosure, and source paths disappearing before display.
- Actual retained application status controls and primary-status layout presence.
  This is not native pixel or physical-input acceptance.

The first new application case used an incorrect continuation-control ID and
failed its control lookup. Correcting the fixture to the authored ID and
rebuilding passed the focused suite and full suite; no product behavior was
changed to satisfy that lookup failure. Windows error 1314 still skips existing
symlink-fixture assertions; native Mac/Linux checks remain necessary.

Native Windows/macOS/Linux CI for this follow-up is pending at commit time.
The previous PR 9 native matrices passed, but are not proof of this new source.

Orchestrator `cargo fmt --check`, full Windows `cargo test --jobs 2 --
--test-threads=2`, and `cargo clippy --all-targets --all-features --jobs 2 -- -D
warnings` pass. Unix-only tests compile out on this Windows host. Changing the
embedded registry intentionally changed the Core provenance digest. The
repository fixture generator refreshed release/bootstrap responses; independent
JSON comparison confirmed only the expected digest changed, with no readiness
or protocol changes. `CargoStaleFixture.log` retains the initial failure.

An intervening default-thread test run hit Windows sharing error 32 while the
existing host test read its just-published discovery file. The earlier run had
passed that same test; the bounded two-thread rerun passed the full suite.
`CargoDiscoveryRace.log` retains the failure, and `CargoTest.log`/`Clippy.log`
retain the final gates. This intermittent startup/publication edge is unresolved;
no test was disabled and this patch does not claim to fix it. A focused follow-up
should distinguish the test's exists-before-close race from production discovery
behavior before changing the provider.

## House-style review

Reviewed against `planning/PROGRAMMING_HOUSE_STYLE.md`: new coverage records and
private projection header/tests; `optional_string_list` and the moved/extended
search-page projection; the changed `Client::search_subtree` delegation;
`SearchCoverageSummary` and its Application call sites/member; the new
interaction probe/scenario; both CMake additions. Explicit initialized types,
named behavior, source-owned strings/vectors, response-local borrows, move
ownership, reserve-before-loop behavior, optional states, exception cleanup and
publication order were checked. Strings necessarily allocate when preserving
provider text; there is no claim of an allocation-free projection.

The inherited JSON parser/transport, whole Application, unchanged interaction
tests, generated UI source and vendored code are not certified by this review.
The moved row projection still treats stored size as unsigned, drops per-row
identity/evidence and constructs filesystem paths under its existing rules;
these known compatibility issues remain explicit rather than silently repaired.
