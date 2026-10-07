# ADR-021: Component repositories and source compilation reuse

Status: **accepted for implementation**.
Date: 2026-10-07.
Owner approval: the owner specified three repositories, a combined Rust/Go
backend, source-consuming C++ applications, reusable GUI.Forms compilation,
and no binary-size increase caused by caching, then directed "lets do it."

## GIVEN constraints

- GUI.Forms, backend, and File Manager have separate repositories.
- Backend groups Engine and Orchestrator builds without merging their runtime
  authority, languages, protocols or executable lifetimes.
- File Manager consumes pinned GUI.Forms source and compiler-cache artifacts.
- Frontend changes must not cause Go/Rust service recompilation.
- Compiler caching must not add runtime code or relax correctness checks.
- Changes are reviewed through PRs directly against each repository's main.

## Workloads and failure modes

Cold and relocated warm toolkit builds, an implementation edit, a public header
edit, changed compiler options, unavailable caches, wrong-platform backend
archives, corrupt executables, and package startup. Cached compilation is not
cached test acceptance. Public ABI changes still require matching consumers.

## Candidates

1. Keep one repository and improve caches only.
2. Require prebuilt shared SDKs for all C++ consumption.
3. Separate ownership, consume GUI.Forms source with portable compiler caches,
   and consume tested backend executables through a pinned bundle.

## Evidence and measurements

**OBSERVED:** GUI.Forms is independently buildable and its native Application
target is already shared; most internal targets are static. This migration
does not alter those linking choices. Ccache stores compilation-unit outputs,
not arbitrary isolated functions. **MEASURED:** the initial M4 retained-control
fixture reused 176/176 compilations after relocation and produced identical
1,672,184-byte executable hashes; cold 70.94 s, warm 1.35 s. These are single
fixture observations, not whole-application or cross-platform speed claims.
Raw proof receipts and platform CI provide the expanded acceptance evidence.

## Decision

Choose 3. `falseywinchnet/gui_forms` owns the toolkit. `falseywinchnet/backend`
owns `engine/` and `orchestrator/`, preserving Orchestrator contract authority.
File Manager pins source revisions through submodules and backend/cache release
archives through `dependencies.lock.json`. The backend submodule supplies the
C++ wire client and contract fixtures; frontend CI does not invoke Go or Cargo
for the service bundle. Explicit research workflows may build their own isolated
laboratory fixtures when their inputs change.

Provider caches are expendable accelerators. Exact pinned backend bundles are
required inputs, verified by archive digest, provider revision, platform and
individual executable hashes. Every consumer still compiles/links and tests.
Caches have bounded storage; normal misses rebuild. Release assets referenced
by dependency locks are durable inputs, not cleanup targets.

## Why the other candidates lost

One repository can reuse builds, but does not express the owner's selected
ownership/rebuild boundaries. Shared-only consumption would unnecessarily
restrict C++ source dogfooding. Neither a new ABI nor a new per-function cache
is needed to establish the requested compilation reuse.

## Consequences

Provider and consumer PRs are separate. Consumers update explicit dependency
pins after provider validation. Shared/static policy and ThinLTO remain separate
measured changes. Existing runtime capabilities and service-installation gates
remain unchanged. Historical documents retain their original source paths;
the current program map and repository READMEs provide migration routing.

## Reversal and migration path

Component source history is preserved through path-filtered extraction from
File Manager `0dc39f989f2d077d5485027dba9046f884e43bc7`. Reverting the integration
PR restores the original source ownership. A cache can be disabled or deleted
without changing build semantics. Pins allow backend rollback without rebuilding
it; unavailable required bundles fail explicitly rather than silently rebuilding.

## Unresolved edges

Static Application linking, ThinLTO, additional architectures and service
installation remain separate work. This decision does not promote them.
