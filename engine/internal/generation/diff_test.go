package generation

import (
	"context"
	"errors"
	"path/filepath"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/workload"
)

func TestDiffStreamsExactPathProjectionChanges(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	beforeCorpus := workload.CorrectnessV1()
	beforeShard, err := beforeCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "before.seg")
	if _, err := Write(path, 1, beforeShard); err != nil {
		t.Fatal(err)
	}
	reader, err := Open(path, root)
	if err != nil {
		t.Fatal(err)
	}
	defer reader.Close()

	afterCorpus := workload.CorrectnessV1()
	afterCorpus.Entries = append([]workload.Entry(nil), afterCorpus.Entries...)
	removed := filepath.Join("unicode-é", "case")
	filtered := afterCorpus.Entries[:0]
	for _, entry := range afterCorpus.Entries {
		if entry.RelativePath == removed {
			continue
		}
		if entry.RelativePath == filepath.Join("alpha", "README") {
			entry.Mode = 0o644
		}
		filtered = append(filtered, entry)
	}
	afterCorpus.Entries = append(filtered, workload.Entry{
		Object: 30, Parent: 1, RelativePath: filepath.Join("alpha", "added"), Kind: api.ObjectRegular, Size: 9,
	})
	afterShard, err := afterCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	var changes []Change
	summary, err := Diff(context.Background(), reader, afterShard, func(change Change) error {
		changes = append(changes, change)
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	if summary.Added != 1 || summary.Updated != 1 || summary.Deleted != 1 ||
		summary.Unchanged != uint64(beforeShard.Len()-2) || len(changes) != 3 {
		t.Fatalf("summary=%+v changes=%+v", summary, changes)
	}
	if changes[0].Kind != ChangeUpdate || changes[0].After.RelativePath != filepath.Join("alpha", "README") ||
		changes[1].Kind != ChangeAdd || changes[1].After.RelativePath != filepath.Join("alpha", "added") ||
		changes[2].Kind != ChangeDelete || changes[2].Before.RelativePath != removed {
		t.Fatalf("unexpected canonical change order: %+v", changes)
	}
}

func TestDiffHonorsCancellationAndSinkErrors(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	shard, err := workload.CorrectnessV1().ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "before.seg")
	if _, err := Write(path, 1, shard); err != nil {
		t.Fatal(err)
	}
	reader, err := Open(path, root)
	if err != nil {
		t.Fatal(err)
	}
	defer reader.Close()
	cancelled, cancel := context.WithCancel(context.Background())
	cancel()
	if _, err := Diff(cancelled, reader, shard, nil); !errors.Is(err, context.Canceled) {
		t.Fatalf("cancelled diff error=%v", err)
	}

	changedCorpus := workload.CorrectnessV1()
	changedCorpus.Entries = append([]workload.Entry(nil), changedCorpus.Entries...)
	changedCorpus.Entries[0].Size++
	changedShard, err := changedCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	sinkError := errors.New("sink stopped")
	if _, err := Diff(context.Background(), reader, changedShard, func(Change) error { return sinkError }); !errors.Is(err, sinkError) {
		t.Fatalf("sink error=%v, want injected sink failure", err)
	}
}

func TestDiffDetectsParentIdentityReplacement(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	beforeCorpus := workload.CorrectnessV1()
	beforeShard, err := beforeCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "before.seg")
	if _, err := Write(path, 1, beforeShard); err != nil {
		t.Fatal(err)
	}
	reader, err := Open(path, root)
	if err != nil {
		t.Fatal(err)
	}
	defer reader.Close()

	afterCorpus := workload.CorrectnessV1()
	afterCorpus.Entries = append([]workload.Entry(nil), afterCorpus.Entries...)
	for index := range afterCorpus.Entries {
		if afterCorpus.Entries[index].Object == 1 {
			afterCorpus.Entries[index].Object = 101
		}
		if afterCorpus.Entries[index].Parent == 1 {
			afterCorpus.Entries[index].Parent = 101
		}
	}
	afterShard, err := afterCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	summary, err := Diff(context.Background(), reader, afterShard, nil)
	if err != nil {
		t.Fatal(err)
	}
	if summary.Updated != 5 || summary.Changes() != 5 {
		t.Fatalf("parent replacement summary=%+v, want directory and four child bindings updated", summary)
	}
}
