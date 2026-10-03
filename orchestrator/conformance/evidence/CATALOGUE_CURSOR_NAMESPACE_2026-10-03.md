# Catalogue substring cursor namespace guard

**OBSERVED development implementation; substring capability remains unavailable.**
The canonical development profile is
`../../spec/contracts/ENGINE_CATALOGUE_SUBSTRING_DEVELOPMENT.md`. Existing exact
catalogue cursors are base64url. The new `.fm-substring-` family is reserved
before legacy text-as-name dispatch: v1 with a nonempty bounded opaque token
returns Unsupported while its provider is absent; an unknown revision, empty
token or over-4096-byte wrapper returns Invalid. Neither issues an exact query
or falls back to live. The Engine retains all token authentication and query
binding responsibility; classification alone is not token validation.

Legacy values are passed to their existing validator without new restrictions.
The helper borrows a string for one call, scans only the fixed prefix and byte
length, allocates nothing and retains nothing. No token issuance, capability
advertisement, new method or public source-client field is added. The installed
local adapter already shares this development adapter, so the same guard applies
without duplicating a second routing policy.

**MEASURED Windows:** `cargo fmt --check`, full locked serial Rust tests and
all-target/all-feature warning-free Clippy pass. There are 80 Windows-selected
tests: 63 unit, five CLI, four live-contract, six broker and two fixture tests.
Unix-only suites run zero tests on this host. `CursorTests.log` and
`CatalogueCursorClippy.log`
retain output. New tests cover known/unknown/empty wrappers, exact and over-limit
UTF-8 byte counts, legacy dispatch, and the complete broker's same-lane refusal
without either legacy exact calls or live fallback. No performance or native
Unix runtime claim follows.

House-style source review covers the complete new `engine_catalogue_cursor.rs`,
the module registration, the added adapter match/return block, and the new named
broker assertion/test functions. Explicit types and states, immutable borrowed
input, no anonymous behavior, separated allocations in fixture construction,
failure-before-dispatch and loop-free allocation-free classification were
checked against `planning/PROGRAMMING_HOUSE_STYLE.md`. Existing Rust closures and
other legacy code remain outside this certification.
