# Integrated file-manager dogfood readiness

Status: **MEASURED/OBSERVED engineering rubric; current score 69/100; ready
for installed controlled M4 dogfood, not yet a solid always-current daily
search backend**.

Date: 2026-08-10. This score measures readiness as a separately built File
Manager search component, not completion of the ultimate OS-parity program.
Points are awarded only for observed code or named measurements. Candidate
designs and compatibility environments do not receive native-production
credit.

## Current score

| Area | Weight | Earned | Evidence and missing edge |
|---|---:|---:|---|
| Exact catalogue, identity, pinned projections | 15 | 13 | Exact reference, durable checked generations, object/binding identity, bounded queries, and digest equivalence exist. Native APFS/NTFS/ext4 identity mutation oracles remain incomplete. |
| Durability, recovery, corruption | 15 | 11 | Live full-generation and isolated tiered-candidate dual-slot publication, fallback, rebuild, abrupt-exit, short-write, schema, and logical fault tests exist. Candidate quarantine integration, physical disk-full, and native power-loss remain open. |
| Incremental storage and write economy | 15 | 13 | Indexed tiered cohort measures 2.42x final and 2.80x cohort-boundary application bytes with 12.2-16.8 MB retained growth. Warm query and isolated atomic-manifest mechanics now pass. Live service admission, manifest-byte accounting, long-horizon tiers, and overlap pacing remain open. |
| Background ingestion and currentness | 15 | 9 | Bounded chained-cursor coalescing, gap/overflow recovery, scan-overlap convergence, backlog status, restart baseline, native macOS FSEvents, Windows `ReadDirectoryChangesW`, a measured 4,096-operation storm, and a ten-minute 0.006654%-CPU/zero-write idle run exist. macOS final-hard-link and Windows root-replacement/journal coverage fail closed; the watermark remains volatile; Linux, live deltas, battery/device-power, and live-plus-committed merge remain. |
| Query usefulness and relevance | 15 | 6 | Exact path/name/identity and metadata filters are useful, and the tiered warm relative p99 gate now passes with separately reported cold costs. Lexical text/phrase search is absent; Kolmogrov/fuzzy remains a disabled bounded candidate channel; fusion and judged relevance are not admitted. |
| Service lifecycle, configuration, API | 10 | 9 | Development lifecycle plus M4 installed readiness/drain/restart, stale-safe configuration, host-bound root admission, same-uid `ENG1` query/admin separation, rotated credentials, and status/capability projection pass. Status subscription and general production transport conformance remain open. |
| Cross-platform install and native validation | 10 | 6 | The M4 per-user LaunchAgent passed install, reconcile, exact query, integrity, credential/instance rotation, bootout/rebootstrap, SIGKILL restart, and private-mode checks. General launchd/system-daemon packaging, update/rollback, SCM/systemd, and native NTFS/ext4 campaigns remain open. Wine/Lima receive no native durability credit. |
| Parent integration and external conformance | 5 | 2 | Orchestrator declarations and a narrow versioned API direction exist. A separately built File Manager client conformance suite and integrated build/install/update exercise do not. |
| **Total** | **100** | **69** | **The named M4 installed dogfood path is useful now; always-current, useful-relevance, cross-platform daily dogfood is not yet earned.** |

## Interpretation

- **50-69:** controlled integration fixture. It may run against disposable or
  explicitly authorized roots, and failures must degrade to navigation without
  search. Manual reconciliation and development transport are acceptable.
- **70-89:** solid daily dogfood. Background currentness, useful lexical search,
  live durable deltas, service supervision, and at least one native packaged
  platform must pass. No critical area may remain below half credit.
- **90-100:** release-candidate evidence across macOS, Windows, and Linux,
  including native identity/durability campaigns and separately built client
  conformance.

The score is not an average-percent-complete claim. In particular, background
currentness is a blocking dependency: improving an already-fast fuzzy method
cannot compensate for an index that is stale or cannot be supervised.

## Shortest evidence-bearing route to 70

1. Reconcile the now-observed isolated tiered manifest through an architect ADR,
   charge its bytes in the write workload, integrate live service leases/status,
   and extend its recovery evidence into the live quarantine workflow.
2. Reconcile and implement the manifest schema that commits the source
   watermark with root policy and exact components; close macOS hard-link and
   Windows root-replacement/journal coverage, then add Linux without a polling
   fallback.
3. Admit a minimum lexical filename/path channel with exact evidence fallback
   and deterministic pagination. Keep Kolmogrov as an independently gated
   candidate source, not the query/storage spine.
4. Extend the now-passing M4 authenticated query/admin and native LaunchAgent
   evidence into a separately built client conformance fixture, update/rollback
   exercise, and the eventual general macOS service boundary.
5. Extend the now-passing warm M2 relative p99 result to diverse paths, cold
   queries, and concurrent background compaction; preserve absolute CPU and
   latency alongside the ratio.

The current engine can therefore run as the opt-in installed M4 dogfood backend
now, but manual reconciliation and exact-only usefulness mean it must not be
presented as the user's always-current general search backend yet. See
[`../results/M4_LAUNCHD_DOGFOOD_001.md`](../results/M4_LAUNCHD_DOGFOOD_001.md).
