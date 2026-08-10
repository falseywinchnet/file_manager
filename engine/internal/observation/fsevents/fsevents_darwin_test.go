//go:build darwin && cgo

package fsevents

import (
	"context"
	"os"
	"path/filepath"
	"runtime/cgo"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation"
)

func TestAdapterFromHandleContainsOnlyLateTeardownLookup(t *testing.T) {
	adapter := &Adapter{}
	handle := cgo.NewHandle(adapter)
	if observed, ok := adapterFromHandle(uintptr(handle)); !ok || observed != adapter {
		t.Fatal("live cgo handle did not resolve to its adapter")
	}
	handle.Delete()
	if observed, ok := adapterFromHandle(uintptr(handle)); ok || observed != nil {
		t.Fatal("deleted cgo handle was accepted")
	}
}

func TestFSEventsObservesDisposableRootWithChainedCursor(t *testing.T) {
	root := t.TempDir()
	config := DefaultConfig()
	config.Latency = 50 * time.Millisecond
	adapterValue, err := New([]api.RootSpec{{ID: "docs", Path: root}}, config)
	if err != nil {
		t.Fatal(err)
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	subscription, err := adapterValue.Subscribe(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if subscription.Initial.Source != "macos.fsevents.host" || subscription.Initial.Epoch == "" {
		t.Fatalf("initial cursor=%+v", subscription.Initial)
	}
	if err := os.WriteFile(filepath.Join(root, "native.txt"), []byte("native"), 0o600); err != nil {
		t.Fatal(err)
	}

	deadline := time.After(10 * time.Second)
	after := subscription.Initial
	for {
		select {
		case <-deadline:
			t.Fatal("FSEvents did not report the disposable-root mutation")
		case batch, open := <-subscription.Batches:
			if !open {
				t.Fatal("FSEvents stream closed before reporting the mutation")
			}
			if batch.After != after || batch.Through.Source != after.Source || batch.Through.Epoch != after.Epoch || batch.Through.Position < after.Position {
				t.Fatalf("unchained batch=%+v after=%+v", batch, after)
			}
			after = batch.Through
			for _, event := range batch.Events {
				if event.Root == "docs" {
					return
				}
			}
		}
	}
}

func TestFSEventsReportsHardLinkCreateAndNonFinalUnlink(t *testing.T) {
	root := t.TempDir()
	target := filepath.Join(root, "target.txt")
	link := filepath.Join(root, "link.txt")
	if err := os.WriteFile(target, []byte("target"), 0o600); err != nil {
		t.Fatal(err)
	}
	config := DefaultConfig()
	config.Latency = 20 * time.Millisecond
	adapterValue, err := New([]api.RootSpec{{ID: "docs", Path: root}}, config)
	if err != nil {
		t.Fatal(err)
	}
	reporter := adapterValue.(observation.CoverageReporter)
	if reporter.ObservationCoverage().CompleteForExactCurrent {
		t.Fatal("FSEvents adapter claimed unproved final-hard-link removal coverage")
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	subscription, err := adapterValue.Subscribe(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if err := os.Link(target, link); err != nil {
		t.Fatal(err)
	}
	afterCreate := waitForNativePath(t, subscription.Batches, "link.txt", subscription.Initial)
	if err := os.Remove(link); err != nil {
		t.Fatal(err)
	}
	afterUnlink := waitForNativePath(t, subscription.Batches, "link.txt", afterCreate)
	t.Logf("hard-link cursors initial=%d create=%d non_final_unlink=%d", subscription.Initial.Position, afterCreate.Position, afterUnlink.Position)
}

func TestMeasureFSEventsFinalHardLinkRemoval(t *testing.T) {
	if os.Getenv("FILEMAN_ENGINE_MEASURE_HARDLINK") != "1" {
		t.Skip("set FILEMAN_ENGINE_MEASURE_HARDLINK=1 for the native coverage measurement")
	}
	root := t.TempDir()
	target := filepath.Join(root, "target.txt")
	link := filepath.Join(root, "link.txt")
	if err := os.WriteFile(target, []byte("target"), 0o600); err != nil {
		t.Fatal(err)
	}
	config := DefaultConfig()
	config.Latency = 20 * time.Millisecond
	adapterValue, err := New([]api.RootSpec{{ID: "docs", Path: root}}, config)
	if err != nil {
		t.Fatal(err)
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	subscription, err := adapterValue.Subscribe(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if err := os.Link(target, link); err != nil {
		t.Fatal(err)
	}
	afterCreate := waitForNativePath(t, subscription.Batches, "link.txt", subscription.Initial)
	if err := os.Remove(target); err != nil {
		t.Fatal(err)
	}
	afterNonFinal := waitForNativePath(t, subscription.Batches, "target.txt", afterCreate)
	if err := os.Remove(link); err != nil {
		t.Fatal(err)
	}
	afterFinal, observed := waitForNativePathWithin(subscription.Batches, "link.txt", 2*time.Second)
	t.Logf("hard-link final-removal observed=%t initial=%d create=%d non_final=%d final=%d", observed, subscription.Initial.Position, afterCreate.Position, afterNonFinal.Position, afterFinal.Position)
}

func waitForNativePath(t *testing.T, batches <-chan observation.Batch, wanted string, after observation.Cursor) observation.Cursor {
	t.Helper()
	deadline := time.After(10 * time.Second)
	for {
		select {
		case <-deadline:
			t.Fatalf("FSEvents did not report %q after %+v", wanted, after)
		case batch, open := <-batches:
			if !open {
				t.Fatalf("FSEvents closed before reporting %q", wanted)
			}
			for _, event := range batch.Events {
				if event.Path == wanted {
					return batch.Through
				}
			}
		}
	}
}

func waitForNativePathWithin(batches <-chan observation.Batch, wanted string, duration time.Duration) (observation.Cursor, bool) {
	deadline := time.NewTimer(duration)
	defer deadline.Stop()
	for {
		select {
		case <-deadline.C:
			return observation.Cursor{}, false
		case batch, open := <-batches:
			if !open {
				return observation.Cursor{}, false
			}
			for _, event := range batch.Events {
				if event.Path == wanted {
					return batch.Through, true
				}
			}
		}
	}
}
