//go:build darwin && cgo

package fsevents

import (
	"context"
	"os"
	"path/filepath"
	"testing"
	"time"

	"filemanager/engine/api"
)

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
