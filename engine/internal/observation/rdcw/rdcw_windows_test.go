//go:build windows

package rdcw

import (
	"context"
	"os"
	"path/filepath"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation"
)

func TestReadDirectoryChangesWObservesDisposableRootAndStops(t *testing.T) {
	root, err := filepath.EvalSymlinks(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	adapterValue, err := New([]api.RootSpec{{ID: "docs", Path: root}}, DefaultConfig())
	if err != nil {
		t.Fatal(err)
	}
	if adapterValue.(observation.CoverageReporter).ObservationCoverage().CompleteForExactCurrent {
		t.Fatal("ReadDirectoryChangesW adapter claimed unproved exact-current coverage")
	}
	ctx, cancel := context.WithCancel(context.Background())
	subscription, err := adapterValue.Subscribe(ctx)
	if err != nil {
		cancel()
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(root, "native.txt"), []byte("native"), 0o600); err != nil {
		cancel()
		t.Fatal(err)
	}
	deadline := time.After(10 * time.Second)
	observed := false
	for !observed {
		select {
		case <-deadline:
			cancel()
			t.Fatal("ReadDirectoryChangesW did not report the disposable-root mutation")
		case batch, open := <-subscription.Batches:
			if !open {
				cancel()
				t.Fatal("ReadDirectoryChangesW stream closed before reporting the mutation")
			}
			for _, event := range batch.Events {
				if event.Root == "docs" && event.Path == "native.txt" {
					observed = true
				}
			}
		}
	}
	cancel()
	stopDeadline := time.After(5 * time.Second)
	for {
		select {
		case _, open := <-subscription.Batches:
			if !open {
				return
			}
		case <-stopDeadline:
			t.Fatal("ReadDirectoryChangesW stream did not stop after cancellation")
		}
	}
}
