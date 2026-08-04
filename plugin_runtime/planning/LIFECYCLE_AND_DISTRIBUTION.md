# Package lifecycle, distribution, SDK, and operations

Status: **candidate plan; catalogue and updater are not admitted features**.

## Package shape candidate

A package is an immutable archive containing:

```text
manifest, payloads by target, assets, schemas, licenses,
SBOM, file-digest tree, publisher signature(s), optional transparency proof
```

No installer script, post-install executable, symlink escape, device node,
absolute path, or package-time network fetch. Extraction occurs into a new
staging directory with count/size/path-depth quotas. Verification precedes an
atomic activation pointer change. Installed versions are content-addressed and
read-only to the guest.

Manifest requests extension contracts, supported protocol ranges, target
triples, entrypoint kind, resource maxima, and settings schema. Requests do not
grant capabilities.

## Discovery

Candidates:

1. explicit user install/import into one per-user managed root;
2. system/package-manager roots imported read-only;
3. development roots enabled only in developer mode;
4. future signed catalogue metadata.

The supervisor never recursively scans arbitrary home directories for plugins.
Duplicate plugin IDs, version conflicts, case collisions, replaced files, and
untrusted ownership/permissions fail verification.

## Install and grants

Proposed transaction:

1. stream archive into bounded staging while hashing;
2. verify archive structure, digests, manifest schema, target, and signatures;
3. render requested contracts and risks using core UI;
4. obtain user decision for install and separately unresolved capabilities;
5. commit immutable package and grant record atomically;
6. run a no-user-data self-test in the actual sandbox;
7. enable only on success; preserve a local audit record.

Install-time approval cannot authorize a broader first-use scope than the user
has seen. Folder/file grants, if admitted, should be selected by host UI and
represented as scoped handles/bookmarks, not plugin-entered paths.

## Signing and trust

Signatures establish package identity and integrity. They do not justify wider
sandbox permissions.

Candidate policies:

- File Manager release key signs built-in adapters.
- Publisher keys are explicitly admitted/revoked; rotation is cross-signed or
  user-confirmed.
- Package digest is the immutable execution identity; version strings alone are
  not trusted.
- Downgrade and rollback require explicit policy; emergency revocation data must
  have an offline/manual import path if network checking is absent.
- Reproducible build metadata and SBOM are verified when available but are not
  treated as proof of source safety.

Unsigned packages are a possible developer-mode feature only. The UI must keep
developer status conspicuous, scope it to test roots by default, and make exit
from developer mode disable—not bless—those packages.

## Update candidates

| Model | Gain | Loss/risk |
|---|---|---|
| Manual signed import | offline, explicit, smallest core | user carries update burden |
| OS/package manager | delegates transport and policy | inconsistent cross-platform semantics |
| Core-managed catalogue | coherent UX and revocation | introduces network/store security surface forbidden from core without approval |
| Plugin self-update | publisher flexibility | **REJECTED candidate:** grants network/write authority to the subject being updated |

Any accepted updater downloads to staging, verifies before activation, keeps a
known-good rollback, reruns compatibility/self-tests, and never migrates grants
to broader meanings. A plugin cannot update itself or its grant record.

## Disable, quarantine, and uninstall

- Disable prevents new jobs and drains/kills running workers.
- Quarantine is supervisor policy after protocol/security faults; thresholds and
  user-visible behavior remain an architect decision.
- Uninstall first disables, then removes activation, worker caches, settings,
  derived data, grants, and content-addressed payloads according to explicit
  retention policy.
- Derived records are located by plugin ID plus version/provenance, not by a
  best-effort directory search.
- User may need export/retain choices for derived data that has independent
  value. No silent orphaned caches.
- Audit/history retention is separately bounded and redacted.

## Observability

Local structured events should record:

- package/plugin digest, contract, job ID, state transition, result status;
- sandbox profile hash and enforcement primitives actually active;
- wall/CPU time, peak memory, bytes read/written, output dimensions, cache use;
- timeout/cancel/crash/protocol violation and quarantine reason;
- no contents, query text, secrets, or raw paths by default.

Development builds may enable detailed traces into an explicit test directory.
Production export is manual and previews the redacted bundle. Metrics are not
sent automatically.

## SDK and tooling program

Future workspace candidates:

```text
crates/protocol-model       semantic messages and state machine
crates/protocol-codec-*     independently fuzzed codec candidates
crates/supervisor-core      policy, jobs, validation, lifecycle
crates/sandbox-macos        App Sandbox/XPC adapter
crates/sandbox-windows      AppContainer/LPAC/Job adapter
crates/sandbox-linux        Landlock/seccomp/namespace adapter
crates/host-c               opaque stable C client boundary
crates/guest-rust           Rust guest SDK
crates/contracts-*          preview/thumbnail/search/virtual schemas
tools/plugin-pack           deterministic pack/sign/inspect tool
tools/plugin-doctor         compatibility and sandbox self-test
tools/protocol-trace        redacted trace decoder
fixtures/hostile-*          crash/hang/bomb/spoof/stale/leak guests
```

Required developer experience:

- scaffold a minimal plugin for exactly one contract;
- run it against an in-memory conformance host and real sandbox host;
- lint requested capabilities and reject unused/broader ones;
- deterministic package creation, SBOM, digest, local signing, inspection;
- protocol compatibility matrix and golden fixtures;
- clear errors when a platform cannot enforce a requested contract.

## Cross-platform delivery candidates

- Rust target artifacts built per supported triple; universal/fat packaging is a
  package-layer choice, not an ABI promise.
- macOS helpers embedded/signed/notarized with entitlement profiles.
- Windows helpers and native guests Authenticode-signed; MSIX and unpackaged
  paths tested separately until distribution is decided.
- Linux helpers packaged for chosen baseline libc or musl where compatible;
  distro sandbox/kernel feature probes are part of support declaration.
- Plugin packages may contain several target payloads selected only after digest
  and manifest verification. A portable WASM payload is an additional candidate,
  not an excuse to omit native isolation.

