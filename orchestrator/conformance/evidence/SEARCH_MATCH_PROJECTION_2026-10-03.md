# Owned search match projection — 2026-10-03

**OBSERVED:** the C++ source client now preserves rank, certainty and ordered
evidence under `spec/FRONTEND_SEARCH_MATCH_CONTEXT.md`. This repairs projection
of existing runtime fields. No transport, storage, security, wire version,
capability or relevance model changed; no stable C++ ABI is claimed. Consumers
must rebuild together. Root owns frontend retention/presentation and canonical
documentation; this receipt covers only the source-client slice.

## Behavior and ownership

SearchResultInfo appends optional uint64 rank, double certainty and an ordered
vector of SearchEvidenceInfo. Missing/null remains unreported; explicit zero,
empty strings, empty evidence and empty details objects remain reported. Unknown
kind/channel strings retain their exact values and order, including an unknown
`live_exact_name` if supplied. No rank synthesis or certainty clamping occurs.
Each evidence entry requires kind/channel strings, finite score and exact/inferred
Booleans; its four optional string projections own their storage.

Rank uses the existing exact unsigned integer conversion. Negative, fractional,
exponential and overflowing rank representations fail. Certainty/score use
locale-independent floating `std::from_chars` with full-token, conversion-status
and finite-value checks. Nonrepresentable underflow also fails the conversion;
representable subnormals and signed zero survive. No score is synthesized for
missing evidence fields. Compact semantic fixtures that omit score, required
metadata or the local response envelope are not complete runtime-wire examples;
runtime api.Evidence emits score/exact/inferred.

The existing private JsonValue representation remains unchanged. A private
recursive serializer copies validated details objects into owned inert JSON text.
It copies JsonNumber's original lexeme, without floating conversion; array order
and string values survive. Object key order and insignificant whitespace may
change. No private parse-tree borrow or dynamic public JSON type escapes.
The serializer grows an owned output string and allocates temporary escaped
strings for keys/values; it is not allocation-free and has no performance claim.
Existing parser depth/value and transport frame limits remain in place.

Evidence is built on the unpublished row, and the page returns only after every
row succeeds. A malformed later evidence entry or row therefore fails the whole
projection and cannot publish an earlier valid row.

## Local validation

**MEASURED:** Windows GNU 16.2.0, adjacent Plan Paint toolchain read-only, Debug,
explicit C++17, `-Wall -Wextra -Werror -Wpedantic`, one compiler job. Commands:

```text
cmake -S orchestrator/conformance/clients/cpp -B orchestrator/.build/search-match-projection -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DFILEMAN_ORCHESTRATOR_BUILD_PROBE=OFF -DFILEMAN_ORCHESTRATOR_BUILD_SEARCH_CHECKS=ON
cmake --build orchestrator/.build/search-match-projection --target orchestrator-cpp-search-checks --parallel 1
ctest --test-dir orchestrator/.build/search-match-projection --output-on-failure
```

Build passed without warnings; the focused CTest target passed, 1/1, 0.10 s
test time. It includes earlier coverage/source-record checks and four new named
match checks: response retirement/owned nested details, optional/empty values,
numeric/type boundaries, and malformed later evidence/row atomicity.
Tests preserve number spelling beyond binary64, escaped controls/NUL and a
surrogate-pair string through serialization and reparse. They exercise uint64
maximum, both finite double extrema, minimum subnormal, signed zero, overflow,
underflow, invalid types, required fields and late failure retaining prior output.

Initial test compilation exposed a `u8` literal becoming char8_t under GCC's
newer default mode. The expected UTF-8 scalar is now expressed as explicit byte
escapes, avoiding a char8_t-to-string conversion. The corrected target passed
both the compiler default mode and the subsequent explicit C++17 build.

**Limits:** no Rust/provider/transport change was made or full Rust gate run in
this scoped assignment. Windows floating from_chars compilation is established;
macOS/libc++ and Linux native CI remain unverified here. C++17 language mode
alone does not prove floating-from_chars library availability. Existing
GUI.Forms classic-locale stream conversion is source evidence of a possible
older-toolchain limitation, not proof of this client's native support. No
speculative platform branch was added. Full frontend integration and native CI
remain root-owned acceptance work.

## Exact house-style review and handoff

Reviewed against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`: the new
SearchEvidenceInfo and three appended result fields; finite_number,
append_inert_json, project_search_evidence, project_search_match and their call
site/include addition; all four new test functions, includes and main calls.
This was semantic source review, not merely a spelling scan: explicit initialized
types, named behavior, const borrows, parser/response retirement, optional-state
guards, binary64 versus lexeme semantics, allocation order, vector reservation,
recursive traversal, owned output and failure-before-publication were checked.
No remaining violation was identified in this authored scope. Untouched parser,
escaping helpers, earlier tests and transport code are dependencies, not newly
certified house-style-compliant code. No authored tooling file was added.

Only client.hpp, client.cpp, search_projection_checks.cpp and this receipt were
edited; build outputs remain in the component's ignored .build directory. No
Git operation or worker/subagent was used. Header, implementation and tests are
frozen for root integration; the one-job compiler slot is released.
