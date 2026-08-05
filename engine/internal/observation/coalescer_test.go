package observation

import (
	"errors"
	"testing"
	"time"

	"filemanager/engine/api"
)

func TestCoalescesDuplicatesAndPreservesRenameSides(t *testing.T) {
	now := time.Unix(100, 0)
	c := initializedCoalescer(t, Limits{MaxBatchEvents: 10, MaxOperations: 10, MaxChanges: 10, MaxBytes: 4096, MaxAge: time.Second}, false)
	outcome, err := c.Ingest(now, Batch{
		After: cursor(0), Through: cursor(1), Events: []Event{
			{Root: "docs", Kind: KindWrite, Path: "a.txt"},
			{Root: "docs", Kind: KindMetadata, Path: "a.txt"},
			{Root: "docs", Kind: KindRename, Path: "b.txt", PreviousPath: "a.txt"},
			{Root: "docs", Kind: KindRemove, Path: "b.txt"},
			{Root: "docs", Kind: KindCreate, Path: "b.txt"},
		},
	})
	if err != nil {
		t.Fatal(err)
	}
	if outcome.RetainedChanges != 2 || outcome.AcceptedEvents != 5 || outcome.FlushRecommended {
		t.Fatalf("outcome=%+v", outcome)
	}
	work, ok := c.Take(now, true)
	if !ok || len(work.Changes) != 2 || work.RequiresFullScan {
		t.Fatalf("work=%+v ok=%v", work, ok)
	}
	if work.Changes[0].Path != "a.txt" || work.Changes[0].Kinds&(KindWrite|KindMetadata|KindRename|KindRemove) != KindWrite|KindMetadata|KindRename|KindRemove {
		t.Fatalf("old rename side=%+v", work.Changes[0])
	}
	if work.Changes[1].Path != "b.txt" || work.Changes[1].PreviousPath != "a.txt" || work.Changes[1].Kinds&(KindRename|KindRemove|KindCreate) != KindRename|KindRemove|KindCreate {
		t.Fatalf("new rename side=%+v", work.Changes[1])
	}
}

func TestCursorChainDetectsGapWithoutAssumingContiguousPositions(t *testing.T) {
	now := time.Unix(200, 0)
	c := initializedCoalescer(t, DefaultLimits(), false)
	if _, err := c.Ingest(now, Batch{After: cursor(0), Through: cursor(100)}); err != nil {
		t.Fatal(err)
	}
	outcome, err := c.Ingest(now, Batch{
		After: cursor(100), Through: cursor(9000),
		Events: []Event{{Root: "docs", Kind: KindWrite, Path: "sparse.txt"}},
	})
	if err != nil || outcome.GapDetected {
		t.Fatalf("sparse ordered cursor outcome=%+v err=%v", outcome, err)
	}
	outcome, err = c.Ingest(now, Batch{
		After: cursor(9001), Through: cursor(9002),
		Events: []Event{{Root: "docs", Kind: KindWrite, Path: "gap.txt"}},
	})
	if err != nil || !outcome.GapDetected || !outcome.FlushRecommended {
		t.Fatalf("gap outcome=%+v err=%v", outcome, err)
	}
	work, ok := c.Take(now, false)
	if !ok || !work.RequiresFullScan || work.Through.Position != 9002 {
		t.Fatalf("gap work=%+v ok=%v", work, ok)
	}
}

func TestReplayIsIgnoredAndEpochChangeRequiresReconciliation(t *testing.T) {
	now := time.Unix(300, 0)
	c := initializedCoalescer(t, DefaultLimits(), false)
	batch := Batch{After: cursor(0), Through: cursor(5), Events: []Event{{Root: "docs", Kind: KindCreate, Path: "x"}}}
	if _, err := c.Ingest(now, batch); err != nil {
		t.Fatal(err)
	}
	replayed, err := c.Ingest(now, batch)
	if err != nil || !replayed.Duplicate || replayed.AcceptedEvents != 0 {
		t.Fatalf("replay=%+v err=%v", replayed, err)
	}
	changed := Batch{
		After:   Cursor{Source: "test", Epoch: "new", Position: 0},
		Through: Cursor{Source: "test", Epoch: "new", Position: 2},
		Events:  []Event{{Root: "docs", Kind: KindWrite, Path: "x"}},
	}
	outcome, err := c.Ingest(now, changed)
	if err != nil || !outcome.GapDetected {
		t.Fatalf("epoch outcome=%+v err=%v", outcome, err)
	}
}

func TestOverflowDropsHintDetailAndStaysBounded(t *testing.T) {
	now := time.Unix(400, 0)
	limits := Limits{MaxBatchEvents: 20, MaxOperations: 20, MaxChanges: 2, MaxBytes: 1024, MaxAge: time.Minute}
	c := initializedCoalescer(t, limits, false)
	outcome, err := c.Ingest(now, Batch{
		After: cursor(0), Through: cursor(1), Events: []Event{
			{Root: "docs", Kind: KindWrite, Path: "one"},
			{Root: "docs", Kind: KindWrite, Path: "two"},
			{Root: "docs", Kind: KindWrite, Path: "three"},
			{Root: "docs", Kind: KindWrite, Path: "four"},
		},
	})
	if err != nil || !outcome.Overflowed || !outcome.GapDetected || !outcome.FlushRecommended {
		t.Fatalf("outcome=%+v err=%v", outcome, err)
	}
	snapshot := c.Snapshot()
	if snapshot.PendingChanges != 0 || snapshot.PendingBytes != 0 || snapshot.PendingObservations != 4 || !snapshot.ReconcileRequired {
		t.Fatalf("snapshot=%+v", snapshot)
	}
}

func TestAgeAndOperationTriggersNeedNoQuietTimer(t *testing.T) {
	now := time.Unix(500, 0)
	limits := Limits{MaxBatchEvents: 10, MaxOperations: 2, MaxChanges: 10, MaxBytes: 4096, MaxAge: time.Second}
	c := initializedCoalescer(t, limits, false)
	if _, ok := c.Deadline(); ok {
		t.Fatal("quiet coalescer requested a deadline")
	}
	first := Batch{After: cursor(0), Through: cursor(1), Events: []Event{{Root: "docs", Kind: KindWrite, Path: "a"}}}
	if outcome, err := c.Ingest(now, first); err != nil || outcome.FlushRecommended {
		t.Fatalf("first=%+v err=%v", outcome, err)
	}
	deadline, ok := c.Deadline()
	if !ok || !deadline.Equal(now.Add(time.Second)) || c.Due(now.Add(time.Second-time.Nanosecond)) || !c.Due(now.Add(time.Second)) {
		t.Fatalf("deadline=%v ok=%v", deadline, ok)
	}
	second := Batch{After: cursor(1), Through: cursor(2), Events: []Event{{Root: "docs", Kind: KindWrite, Path: "a"}}}
	if outcome, err := c.Ingest(now, second); err != nil || !outcome.FlushRecommended {
		t.Fatalf("second=%+v err=%v", outcome, err)
	}
}

func TestEmptyBatchAdvancesVolatileReconciledCursorWithoutWork(t *testing.T) {
	now := time.Unix(550, 0)
	c := initializedCoalescer(t, DefaultLimits(), true)
	work, ok := c.Take(now, false)
	if !ok {
		t.Fatal("missing baseline work")
	}
	if err := c.Finish(work.ID, true, ""); err != nil {
		t.Fatal(err)
	}
	outcome, err := c.Ingest(now, Batch{After: cursor(0), Through: cursor(500)})
	if err != nil || outcome.FlushRecommended {
		t.Fatalf("empty outcome=%+v err=%v", outcome, err)
	}
	snapshot := c.Snapshot()
	if snapshot.Reconciled.Position != 500 || snapshot.Observed.Position != 500 || snapshot.PendingObservations != 0 || c.Due(now) {
		t.Fatalf("empty snapshot=%+v", snapshot)
	}
}

func TestZeroPositionDiscontinuityCanInvalidateRootWithoutCursorAdvance(t *testing.T) {
	now := time.Unix(575, 0)
	c := initializedCoalescer(t, DefaultLimits(), false)
	outcome, err := c.Ingest(now, Batch{
		After: cursor(0), Through: cursor(0), Discontinuity: true,
		Events: []Event{{Root: "docs", Kind: KindRootInvalidated}},
	})
	if err != nil || !outcome.GapDetected || !outcome.FlushRecommended {
		t.Fatalf("root invalidation=%+v err=%v", outcome, err)
	}
}

func TestSuccessfulWorkDoesNotEraseGapArrivingDuringScan(t *testing.T) {
	now := time.Unix(600, 0)
	c := initializedCoalescer(t, DefaultLimits(), true)
	work, ok := c.Take(now, false)
	if !ok || !work.RequiresFullScan {
		t.Fatalf("initial work=%+v ok=%v", work, ok)
	}
	if _, err := c.Ingest(now, Batch{
		After: cursor(1), Through: cursor(2),
		Events: []Event{{Root: "docs", Kind: KindWrite, Path: "during"}},
	}); err != nil {
		t.Fatal(err)
	}
	if err := c.Finish(work.ID, true, ""); err != nil {
		t.Fatal(err)
	}
	snapshot := c.Snapshot()
	if !snapshot.ReconcileRequired || !snapshot.HasReconciled || snapshot.Reconciled.Position != 0 {
		t.Fatalf("snapshot=%+v", snapshot)
	}
}

func TestEmptyBatchDuringScanDoesNotLeaveUnserviceableCursorLag(t *testing.T) {
	now := time.Unix(650, 0)
	c := initializedCoalescer(t, DefaultLimits(), false)
	if _, err := c.Ingest(now, Batch{
		After: cursor(0), Through: cursor(1),
		Events: []Event{{Root: "docs", Kind: KindWrite, Path: "changed"}},
	}); err != nil {
		t.Fatal(err)
	}
	work, ok := c.Take(now, true)
	if !ok {
		t.Fatal("missing change work")
	}
	if _, err := c.Ingest(now, Batch{After: cursor(1), Through: cursor(99)}); err != nil {
		t.Fatal(err)
	}
	if err := c.Finish(work.ID, true, ""); err != nil {
		t.Fatal(err)
	}
	snapshot := c.Snapshot()
	if snapshot.Reconciled.Position != 99 || snapshot.Observed.Position != 99 || snapshot.PendingObservations != 0 || c.Due(now) {
		t.Fatalf("snapshot=%+v", snapshot)
	}
}

func TestFailureRequiresRetryAndWorkIDsAreChecked(t *testing.T) {
	now := time.Unix(700, 0)
	c := initializedCoalescer(t, DefaultLimits(), true)
	work, ok := c.Take(now, false)
	if !ok {
		t.Fatal("missing initial work")
	}
	if err := c.Finish(work.ID+1, true, ""); !errors.Is(err, ErrWorkMismatch) {
		t.Fatalf("mismatch error=%v", err)
	}
	if err := c.Finish(work.ID, false, "scan failed"); err != nil {
		t.Fatal(err)
	}
	if snapshot := c.Snapshot(); !snapshot.ReconcileRequired || snapshot.GapReason != "scan failed" || snapshot.InFlight {
		t.Fatalf("snapshot=%+v", snapshot)
	}
}

func TestInvalidAndOversizedBatchesForceReconciliation(t *testing.T) {
	now := time.Unix(800, 0)
	limits := Limits{MaxBatchEvents: 1, MaxOperations: 10, MaxChanges: 10, MaxBytes: 4096, MaxAge: time.Second}
	for name, batch := range map[string]Batch{
		"absolute": {After: cursor(0), Through: cursor(1), Events: []Event{{Root: "docs", Kind: KindWrite, Path: "/escape"}}},
		"oversized": {After: cursor(0), Through: cursor(1), Events: []Event{
			{Root: "docs", Kind: KindWrite, Path: "a"}, {Root: "docs", Kind: KindWrite, Path: "b"},
		}},
	} {
		t.Run(name, func(t *testing.T) {
			c := initializedCoalescer(t, limits, false)
			outcome, err := c.Ingest(now, batch)
			if err == nil || !outcome.GapDetected || !outcome.FlushRecommended {
				t.Fatalf("outcome=%+v err=%v", outcome, err)
			}
		})
	}
}

func initializedCoalescer(t *testing.T, limits Limits, reconcile bool) *Coalescer {
	t.Helper()
	c, err := New(limits)
	if err != nil {
		t.Fatal(err)
	}
	if err := c.Initialize(cursor(0), reconcile, "restart has no committed observation watermark"); err != nil {
		t.Fatal(err)
	}
	return c
}

func cursor(position uint64) Cursor {
	return Cursor{Source: "test", Epoch: "one", Position: position}
}

var _ api.RootID = "docs"
