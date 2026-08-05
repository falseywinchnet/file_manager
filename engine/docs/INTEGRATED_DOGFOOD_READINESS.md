# Integrated file-manager dogfood readiness

Status: **MEASURED/OBSERVED engineering rubric; current score 64/100; ready
for controlled sandbox integration, not solid daily dogfood**.

Date: 2026-08-05. This score measures readiness as a separately built File
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
| Service lifecycle, configuration, API | 10 | 7 | Development start/readiness/drain/stop, restart identity, stale-safe configuration apply, status/capability projection, and JSONL service operation exist. Authenticated framed query/admin separation and supervisor policy do not. |
| Cross-platform install and native validation | 10 | 3 | macOS host tests, cross-builds, Wine Windows compatibility, and Lima Linux compatibility exist. launchd/SCM/systemd packages and native NTFS/ext4 campaigns do not. Wine/Lima receive no native durability credit. |
| Parent integration and external conformance | 5 | 2 | Orchestrator declarations and a narrow versioned API direction exist. A separately built File Manager client conformance suite and integrated build/install/update exercise do not. |
| **Total** | **100** | **64** | **Controlled native macOS sandbox integration is useful now; solid daily cross-platform dogfood is not yet earned.** |

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
4. Freeze authenticated local query/admin transport semantics, build the
   separate client conformance fixture, and exercise install/start/status/
   restart/stop/uninstall on one native supervisor.
5. Extend the now-passing warm M2 relative p99 result to diverse paths, cold
   queries, and concurrent background compaction; preserve absolute CPU and
   latency alongside the ratio.

The current engine can therefore enter an opt-in File Manager integration
sandbox now, but it should not be presented as the user's always-current search
backend yet.
