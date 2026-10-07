# Component extraction and compiler reuse evidence

Date: 2026-10-07. Implements accepted ADR-021. This record distinguishes
source preservation, measured compiler reuse, and native application validation.

## Source preservation

**OBSERVED:** the extracted repository main trees match the corresponding
File Manager `0dc39f989f2d077d5485027dba9046f884e43bc7` subtrees exactly:

| Component | Git tree SHA |
| --- | --- |
| GUI.Forms | `031a4de28a346f8bcbbb9194d1cede204b0f5fed` |
| Engine | `94ae13efbff512cafc8f736270117c110d0aafb1` |
| Orchestrator | `44d280f9314e894231d30394f19fb6ab574d720d` |

Filtered histories retain commit mappings in GUI.Forms
`docs/FILE_MANAGER_COMMIT_MAP.txt` and backend `SOURCE_COMMIT_MAP.txt`.
File Manager replaces these owned directories with explicit source submodules.
Games/PlaySuite was not edited.

## Relocated compiler-cache workload

**MEASURED:** clean producer and consumer source copies, different absolute
checkout locations, Clang Release, the real `gui_forms_owned_event_tests`
target, four compile jobs. Hosts/renderers are disabled for this bounded proof.
Both builds execute the same event test. All 176 cacheable compilations hit in
each relocated consumer, with zero misses and identical executable SHA-256
and size within that platform.

| Environment | Cold compile/link | Relocated warm compile/link | Executable bytes |
| --- | ---: | ---: | ---: |
| Local M4 macOS | 67.63 s | 1.25 s | 1,672,184 |
| GitHub macOS arm64 | 146.46 s | 2.87 s | 1,672,184 |
| GitHub Linux x64 | 170.12 s | 0.44 s | 1,589,544 |
| GitHub Windows CLANG64 x64 | 179.15 s | 4.37 s | 1,697,792 |

Hosted proof receipts come from GUI.Forms run
[37594728588](https://github.com/falseywinchnet/gui_forms/actions/runs/37594728588),
whose proof steps completed before the broader run was superseded. They do not
claim that superseded workflow completed native validation. The local accepted
receipt is retained in GUI.Forms `experiments/compiler-cache/macos-m4.json`.
These are single observations, excluding configure, cache transfer, dependency
fetch and packaging time; they are not full-application speed measurements.

Implementation edits, public-header edits and a changed optimization option
caused cache misses. The local implementation edit missed once, the public-header
edit missed 125 times, and the option edit missed 176 times.

**REJECTED:** an initial invalidation test changed an unused `-D` definition.
Ccache correctly reused identical preprocessed source. The proof now changes
`-fno-inline-functions`. The rejected raw receipt remains available. Windows
proof linking also disables the PE timestamp so time alone cannot alter its
binary comparison. That flag applies to the proof, not application link policy.

## Build and runtime boundaries

The ordinary C++ compilation and linking remain intact. Ccache applies to CMake
and Skia's GN compile commands. Cache entries add no runtime payload, shared
library dependency or new ABI. Existing static/shared target decisions remain.
This fixture proves byte equivalence for its executable; it is not a measured
whole-application/package size comparison.

Backend archives contain the separately built Go Engine and Rust Orchestrator.
Archive hashes, source revision, platform and executable hashes must agree before
copying services. File Manager does not invoke Go or Cargo to build those services.
The frontend JPEG laboratory still builds its own research conformance fixture;
backend-only research workflows move with their sources.

Compiler seeds are optional; source compilation handles a missing seed. Required
backend bundles fail explicitly when unavailable. Tested provider main builds
publish durable `build-<full-source-SHA>` prerelease assets. Consumer locks bind
all source pins and platform archive digests. A cache hit never authorizes skipping
consumer tests or claiming platform capability availability.

## Local consumer finding

**OBSERVED:** the first local native consumer compile selected old GUI.Forms
headers from `/usr/local/include` before its explicit SDK's `-isystem` path.
A preprocessor trace established the selected files. The consumer now configures
`CMAKE_NO_SYSTEM_FROM_IMPORTED=ON`, giving its chosen imported SDK ordinary
include-path priority. The global installation is preserved. This prevents a
verified SDK manifest from silently accompanying compilation against other headers.
The local host SDK (26.5) differs from the hosted producer SDK (26.6), so the
initial local native compilation exercised cache misses rather than reuse.
After the include-priority correction, the local M4 build passed 86 toolkit
CTests and 16 frontend CTests, then produced a signed development archive whose
package startup check passed. This was a pre-commit working-tree validation;
the receipt correctly records a dirty source tree.

## Reviewed source scope

House-style source review covered the new provider Python build/cache/proof
tools, new consumer `backend_bundle.py`, `fetch_build_inputs.py`, their focused
tests, and the changed portions of `build_native.py`, `package_native.py` and
`export_native_sdks.py`.
Review included explicit types, named execution, failure propagation, scoped
temporary files, bounded artifact members and source/provenance ordering.
New workflows pass actionlint; the existing C++ spelling regression checks remain.

Backend review also covered the new saturating atomic compare/exchange helpers,
the authenticated shutdown reconnect in the live-search test, empty-result
assertion updates, and the platform-gated test import. Atomic memory ordering and
saturation semantics are preserved; the helpers retain the declared Rust API
baseline without using the deprecated `fetch_update` spelling. Golden fixture
updates change only an obsolete contract-provenance digest. Canonical LF text
keeps that digest consistent between native checkouts.

This is not a compliance claim for all imported code. Legacy Rust inference and
closures, historical tooling, and previously unreviewed C++ remain outside this
change's style acceptance. Native provider and consumer results are attached to
their PRs and run artifacts; package startup remains distinct from interactive
desktop, accessibility and daily-root acceptance.
