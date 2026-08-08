# M1L catalogue-independent live query — implementation and 10k APFS control

Status: **OBSERVED implementation; MEASURED local APFS scale point; native
NTFS/ext4 and million-entry promotion gates remain open**.

Date: 2026-08-07.

Environment:

- macOS Darwin 23.6.0, arm64 Apple M3, 8 GiB RAM;
- APFS on solid-state storage;
- Go 1.26.5;
- disposable `t.TempDir` fixture, 10,000 flat zero-byte files;
- command:
  `FILEMAN_ENGINE_LIVE_MEASURE=1 go test -v -run '^TestLiveQueryDistribution$' -count=1 ./benchmarks`.

## OBSERVED implementation

`internal/live.Manager` performs metadata-only `os.Root` traversal with
streaming `ReadDir(1)`, an explicit open-directory depth ceiling, no directory
symlink traversal, bounded result/work/time/response budgets, a fixed
32-session process cap, and 30-second process-local cursor expiry. State is
`O(open depth + current page + fixed session table)`; it does not collect or
globally sort the tree. Root-policy changes, completion, expiry, and shutdown
close retained handles. Live query neither consults nor publishes the
catalogue.

The persistent-service conformance test fingerprints every store path, mode,
size, and modification timestamp before and after a live query and observes no
change. This is **OBSERVED** filesystem-state evidence, not block-device write
telemetry or a physical power-loss campaign.

The authenticated Orchestrator local daemon development projection spawned a
separately built Engine, advertised live availability from `engine.version`,
and returned a zero-catalogue match through the single `orchestrator.search`
operation. The independent C++17 client passed the same route. These are
cross-process development-transport results, not installed Engine transport
admission.

## MEASURED 10k result

One retained run reported:

- first result page: 82.875 µs;
- complete 10,000-result traversal: 40.802 ms across 80 pages;
- visited entries / metadata calls: 10,000 / 10,000;
- Go total allocation: 30,251,504 bytes and 325,847 allocations;
- retained heap delta after GC: 9,992 bytes;
- observed descriptor count: 6 baseline, 9 maximum, 6 after completion;
- catalogue generation unchanged and root remained unindexed.

These values are **MEASURED** for this one warm local fixture only. Total
allocation is high enough to keep per-result allocation reduction as an
optimization target; the retained-state and descriptor results pass this
small control.

## Red evidence

The following remain explicitly unmeasured or incomplete:

- million-entry wide and deep trees;
- p50/p95/p99 distributions, CPU time, RSS, and filesystem bytes read;
- native NTFS and ext4 containment/identity/resource campaigns;
- permission-denied execution under a distinct identity, mount-boundary
  policy, root replacement, and blocked-output cancellation;
- exact encoded-byte accounting below the conservative per-result allowance;
- installed authenticated Engine discovery/transport.

No claim in this record promotes ORC-ENG-004 to cross-platform production
availability.
