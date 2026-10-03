# Catalogue and observation status capture

**OBSERVED repaired behavior:** `Status` captures the immutable catalogue,
copied durable-reader metadata and background observation values under the
compatible `readerMu -> background.mu -> controller mutex` order. It releases
those locks before sorting roots, constructing warnings, projecting currentness
or hashing configuration. The configuration uses the captured roots and
ingestion mode. It takes no administrative lock, performs no scan and adds no
retry or polling loop. Public fields and currentness meanings remain unchanged.

The original native Linux failure and a deterministic negative control are in
`../rejected/2026-10-03-status-sampling/`. The negative control uses a private Go
overlay to restore the late background read during projection; it does not edit
production source and is not a checkout of the old revision. Both volatile and
persistent capture tests fail with the original contradictory state. The patch
and full failure output are retained. Passing a separate native run did not
erase the initial failure.

**MEASURED correctness:** Go 1.27.1, Windows/amd64 on Shadow; `GOMAXPROCS=2`
and at most two compiler jobs, with no concurrent C++ compilation or timing.
From `engine/`, all of these passed:

```text
go test -p 2 ./internal/service -run 'TestStatus(CaptureSurvives|ReturnsDuring)|TestBackgroundObservationBaselineGapAndStopState' -count=1
go test -p 2 ./...
go test -race -p 2 ./...
go vet -p 2 ./...
```

The focused service result was 0.377 s. Full correctness and race logs are
retained here; vet exited zero without diagnostics. `gofmt -l` on the four
changed files returned no output. These are correctness results, not status
latency or query-throughput benchmarks. Native Linux/macOS acceptance is pending.

The four new tests force both memory-backed and persistent baseline scans to
wait at a fixture gate. They capture status before reconciliation, finish real
reconciliation and coalescer publication, then project the old capture. It must
retain the old catalogue and old reconciling state. Fresh public status must
show the indexed catalogue and watermark 10. Stopping the adapter cannot change
the old capture's configuration mode/digest. Separate tests prove public status
returns while each kind of scanner is still blocked. Named cleanup releases
the scanner gate before service shutdown, including assertion failures.

Root and the visible sibling “Audit File Manager Details against interviews”
reviewed all of `status.go` and `status_consistency_test.go`, the background
capture/projection additions and the two configuration construction functions
against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`. The review covered
explicit Go types, named executable behavior, immutable ownership, reader and
test-goroutine lifetime, lock ordering, initialization, conversions, failure and
cleanup paths, and loop storage. No concrete violation remains in that authored
scope. Unchanged legacy helpers are not certified. The normalized source hashes
identify exact reviewed files without implying whole-file review for the two
partially changed files.

This fixes mixed catalogue/observation sampling. `ApplyRoots` still publishes
root policy before separately invalidating background currentness, and lifecycle
counters are sampled after the capture locks are released. Those are explicit
remaining boundaries; this change does not claim all administrative transitions
or all status fields form one globally atomic transaction.
