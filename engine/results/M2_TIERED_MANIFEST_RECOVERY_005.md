# M2 tiered candidate manifest and recovery 005

Status: **OBSERVED isolated atomic publication/recovery mechanics; candidate
only; live `MANIFEST.*` and service query path unchanged**.

Date: 2026-08-05. Source manifest (Go/C files, `go.mod`, and `Makefile`):
`2237d809e89e2f4c303fb98ee19d0f8637179424e4cc1b70e42a5bff0e3cea97`.

## Question and boundary

Can a checked base plus digest-chained delta/index runs be named atomically,
recovered after interruption, and reclaimed without exposing a partial exact
generation?

The experiment uses separate `TIERED.0`/`TIERED.1` slots. It does not modify or
supersede the live v1 `MANIFEST.*` schema. The checksummed manifest binds:

- root identity and clean absolute root path;
- authenticated base segment name, generation, segment digest, and catalogue
  digest;
- ordered delta/index artifact names;
- final generation, catalogue digest, live length, change count, and index
  bytes.

Opening a manifest fully checks the base, every delta payload, every disposable
sidecar, the generation/digest chain, and the final projection dimensions
before returning a lease. Sidecars can be rebuilt to a new immutable name and
the same exact generation can be republished with a later manifest sequence.

## Observed campaign

Tests cover:

- logical interruption and actual subprocess exit after artifact-directory
  sync, manifest-file sync, manifest rename, and manifest-directory sync;
- previous-generation selection before the manifest rename and new-generation
  selection after it;
- corrupt newest sidecar fallback to the previous complete generation;
- rebuilt-sidecar same-generation repair;
- corrupt newest manifest fallback without permitting publication to overwrite
  its evidence;
- one-byte/partial manifest write fallback;
- cancellation before publication;
- authenticated newer-schema rejection for both recovery and publication;
- pin-aware reclamation: an overwritten artifact remains while a reader lease
  is open and is removed after the final pin closes;
- strict store-local, regular-file, safe-name admission.

Test locators:
`internal/generation/tiered_manifest_candidate.go` and
`internal/generation/tiered_manifest_candidate_test.go`.

## Disposition

- **PASS logical atomicity:** checked recovery exposes the old or new exact
  state, never a mixed chain, at every tested boundary.
- **PASS process-exit oracle:** the same old/new rule holds after the publisher
  exits with no cleanup.
- **PASS bounded namespace/reclamation:** two bounded manifest slots and
  pin-aware deletion cover engine-owned tier artifacts without scanning into
  indexed roots.
- **PASS rebuildable index behavior:** a damaged sidecar does not damage exact
  source records and can be replaced under a later checked manifest sequence.
- **OPEN live admission:** the service still publishes full v1 generations.
  Promotion requires the architect-owned ADR, manifest-byte accounting in the
  write workload, service-level generation leases, and startup/status policy.
- **OPEN native durability:** Wine and Lima are compatibility oracles only.
  Native APFS/NTFS/ext4 disk-full, device-flush, and power-loss campaigns remain
  mandatory.
- **OPEN evidence quarantine:** corrupt candidate artifacts are preserved by
  refusing reclamation while a recovery problem exists, but they do not yet
  use the full live-store quarantine metadata workflow.

## Verification matrix

- **PASS:** macOS/arm64 full tests, race detector, and `go vet`;
- **PASS:** Linux/amd64 and Windows/amd64 cross-builds;
- **PASS compatibility oracle:** all 13 test-bearing Windows/amd64 packages
  under Wine on host APFS;
- **PASS compatibility oracle:** all 13 test-bearing Linux/arm64 packages under
  Lima with test temporary files on guest `/var/tmp` ext4.

No native filesystem or physical power-loss claim is made.
