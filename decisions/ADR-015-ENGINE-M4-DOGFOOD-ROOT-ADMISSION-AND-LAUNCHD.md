# ADR-015: Engine M4 dogfood root admission and launchd service

Status: **accepted and installed evidence passed for the named M4 deployment;
other production hosts remain gated**.

Date: 2026-08-10.

Owner approval: the grand architect explicitly directed the Engine to be
ungated for dogfood on the separate M4 Mac mini, added under launchd, and
unsandboxed on that host only.

## Question

How can the Engine index a useful non-disposable source tree and remain
resident on the M4 without creating a general sandbox bypass or claiming that
the cross-platform production-service gates are closed?

## Constraints

- **GIVEN:** the deployment is restricted to the named M4 and its logged-in
  user. The Neo checkout remains authoritative source.
- **GIVEN:** no `--force`, environment bypass, whole-home root, filesystem
  root, or implicit root discovery is admitted.
- **GIVEN:** Engine state remains outside every indexed source root.
- **GIVEN:** query and administration have different authority.
- **OBSERVED:** the M4 route has no passwordless administrative authority. A
  root `/Library/LaunchDaemons` installation is therefore unavailable.
- **OBSERVED:** a per-user LaunchAgent is an accepted native supervisor model
  elsewhere in this program (ADR-011) and is sufficient for single-user
  dogfood.
- **OBSERVED:** background FSEvents coverage remains incomplete. This decision
  does not mislabel manual reconciliation as always-current observation.

## Candidates

### A. Disable the sandbox with a flag

This makes every path reachable by any invocation and has no durable statement
of user approval, host identity, exclusions, or reversal scope.

### B. Install a root LaunchDaemon

This expands privilege and requires administrative installation authority that
is not available through the M4 build route. The first dogfood workload does
not require cross-user indexing.

### C. Install a host-bound per-user LaunchAgent with manifest-limited roots

Use a `0600`, same-uid manifest bound to Darwin `kern.uuid`, uid, exact root
object identity, canonical paths, relative exclusions, private runtime/store
directories, and one or more immutable root id/path pairs. Expose separate
same-uid Unix sockets with per-instance credentials and bounded framing.

## Decision

Choose C for deployment id `m4-dogfood` only.

The initial root is the deterministic M4 mirror of the File Manager repository.
The whole home directory is not authorized. Build products, repository
metadata, and named raw-result trees are excluded. The store, endpoint tokens,
discovery record, sockets, and logs live under the effective user's private
`~/Library/Application Support/FileManager/Engine/m4-dogfood` leaf.

The manifest is checked on every process start. It must be a non-symlink
regular file, owned by the service uid, mode `0600`, schema-compatible, and
bound to the current host UUID and uid. Each approved root must still resolve
to the exact recorded platform object identity. Runtime administrative calls
may apply only exact manifest id/path pairs and therefore cannot widen
authority. Manifest exclusions prune traversal before metadata observation.

The native projection is a per-user LaunchAgent named
`com.filemanager.engine.m4-dogfood`, with `RunAtLoad`, `KeepAlive`, background
process classification, private umask, and a one-second restart throttle. The
process owns two `0600` Unix sockets inside its `0700` runtime directory. Both
require the OS peer uid to equal the service uid. Query and admin endpoints use
distinct random 256-bit credentials rotated at every process start. Messages
use bounded `ENG1 || uint32-be length || JSON` frames. Query authority cannot
invoke root, reconcile, integrity, rebuild, or shutdown methods; admin
authority cannot invoke query or inspect methods.

This closes the installed-supervisor and authenticated-local-wire gates only
for controlled M4 dogfood. It does not close native NTFS/ext4, systemwide
multi-user installation, always-current observation, lexical relevance, update
packaging, or general release gates.

## Failure modes and tests

- A copied manifest on another Mac or uid fails before the store opens.
- A replaced root directory fails its exact object-identity check.
- A loose-mode, wrong-owner, symlink, unknown-field, or trailing-data manifest
  fails closed.
- A root-policy request outside the manifest cannot be planned or applied.
- An excluded subtree is pruned during authoritative and live traversal.
- A wrong endpoint credential, wrong authority, wrong peer uid, malformed
  magic, truncated frame, or oversized frame is rejected.
- A stale socket is replaced only when it is a socket and no listener accepts;
  a non-socket path is never unlinked.
- Graceful shutdown exits the process; launchd `KeepAlive` creates a new
  instance and new credentials while preserving the checked generation.
- Bootout removes supervision and volatile endpoints. Uninstall preserves the
  durable store and logs unless they are separately and explicitly purged.

## Rejected options

- A is **REJECTED** because it violates the Engine safety boundary and has no
  auditable authorization object.
- B is **REJECTED for this dogfood deployment** because it adds unnecessary
  privilege and unavailable installation authority. It remains a candidate for
  a future multi-user system service under a separate decision.

## Reversal path

Run the checked-in uninstaller or `launchctl bootout` the exact label, remove
the exact LaunchAgent plist and volatile runtime objects, and leave the
manifest/store/log evidence intact. Purging those persistent objects is a
separate explicit action. No source-root file is modified by reversal.

## Installed evidence — 2026-08-10

**MEASURED:** the M4 LaunchAgent installed without administrative privilege,
published a 28,542-record checked generation, returned an exact APFS-identity
query result, passed integrity, rotated credentials and instance identity after
graceful shutdown, became unreachable after bootout, recovered after
rebootstrap, and recovered again after SIGKILL. All authority objects had the
declared private owner and modes. The detailed record is
`engine/results/M4_LAUNCHD_DOGFOOD_001.md`.
