# M2 tiered cohort compaction control 003

Status: **MEASURED promising write schedule; CANDIDATE only. Current heap
overlay is REJECTED for the 10,000-change form; streaming compactor, disk
indexes, manifest publication, and long-horizon levels remain open**.

Date: 2026-08-05. Source manifest (Go/C files, `go.mod`, and `Makefile`):
`311049d844f565f92c953533ca1a5cd2b1524ef363376398239927cb2d2f4c05`.

## Question and mechanism

The rejected cumulative policy rewrote every old change whenever the active
chain reached eight. This control instead compacts only each completed cohort
of eight fresh runs. At 22 epochs the checked chain is:

```text
cohort(1..8) -> cohort(9..16) -> fresh(17) ... fresh(22)
```

That is eight visible runs: two immutable cohort runs plus six fresh runs. Each
old operation is written as a fresh record and at most once more into its
cohort. No prior cohort is rewritten.

The experiment uses full checkpoint segments solely as exact diff oracles for
cohort boundaries. Their bytes are excluded. A production implementation must
stream the same net cohort from checked composite readers without writing that
checkpoint. The emitted cohort files themselves are real checked delta runs,
form one generation/digest chain, and are reopened through the existing exact
overlay.

## Environment and workload

- macOS 14.8.7/Darwin 23.6.0, Apple M3 arm64, 8 GiB RAM;
- Go 1.26.5 darwin/arm64; warm internal APFS state;
- `scale-v1`, 250,000 files plus 25,000 directories;
- 22 disjoint mixed epochs at 4,096 or 10,000 exact operations per epoch;
- metadata update, delete, rename, create, and hard-link distribution shared
  with the previous M2 controls;
- exact digest/generation validation after every fresh run and cohort;
- visible run count asserted at no more than eight.

Command:

```sh
FILEMAN_ENGINE_M2_MEASURE=1 go test ./benchmarks \
  -run '^TestTieredCohortCompactionDistribution$' -count=1 -v
```

## Result

| Batch | Epochs | Cohorts + fresh | Visible runs | Changed paths | Fresh bytes | Cohort bytes | Logical bytes | Cumulative WA | Live run bytes | Retained heap growth |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 4,096 | 22 | 2 + 6 | 8 | 90,112 | 15,422,968 | 11,207,912 | 13,336,576 | **1.9968x** | 15,414,176 | 37,051,104 B |
| 10,000 | 22 | 2 + 6 | 8 | 220,000 | 37,633,816 | 27,361,256 | 32,560,000 | **1.9962x** | 37,625,024 | **79,217,560 B** |

Each complete eight-run cohort measured about 2.31x at its boundary because all
eight fresh runs and their one replacement are charged against those eight
epochs. Six later fresh epochs amortize the cumulative 22-epoch schedule to
about 2.00x. This remains below 3x before manifest/filesystem metadata. Adding
indexes and manifest publication must fit the remaining approximately 1.00x;
the margin is a budget, not permission to spend it by intuition.

## Disposition

- **CANDIDATE retained:** size-tiered cohort compaction avoids the cumulative
  rewrite defect and keeps the 22-epoch visible chain at eight runs.
- **REJECTED representation:** materializing the complete changed-path union in
  the current Go heap overlay. The 10,000-change form retained 79.2 MB before
  the rest of the service; even the 37.1 MB form leaves insufficient evidence
  for the 48 MiB complete-service gate.
- **REQUIRED next:** checked on-disk path/name/identity indexes and a query view
  that retains only bounded block/fence/hash metadata, with exact record
  verification after every index hit. Approximate hashes may accelerate probes
  but never decide identity.
- **REQUIRED next:** a streaming cohort compactor that consumes the previous
  checked composite directly, without an oracle checkpoint or a full changed
  union.
- **OPEN long horizon:** after enough cohorts, another level or base replacement
  is necessary. It must measure the same cumulative denominator; this result
  covers 22 epochs, not a 24-hour steady state.

The schedule is shared across exact, lexical, and content/fragment descriptor
components. It neither depends on nor privileges a Kolmogrov channel.

Implementation/measurement locator:
`benchmarks/m2_tiered_cohort_test.go`. Existing checked mechanics:
`internal/generation/delta_candidate.go` and
`internal/generation/overlay_candidate.go`.

Verification after measurement: **PASS** macOS `go test ./...`, race, `go vet`,
Linux/amd64 and Windows/amd64 cross-builds; **PASS compatibility oracle** all
13 test-bearing Windows/amd64 packages under Wine 11.10 and all 13 Linux/arm64
packages under Lima with guest `/var/tmp`. These are not native NTFS/ext4
durability or service-installation results.
