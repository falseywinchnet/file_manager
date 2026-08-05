package generation

import (
	"context"
	"errors"
	"os"
	"path/filepath"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/workload"
)

func TestDeltaCandidateStreamsCheckedExactChanges(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	beforeCorpus := workload.CorrectnessV1()
	before, err := beforeCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	basePath := filepath.Join(t.TempDir(), "base.seg")
	baseMetadata, err := Write(basePath, 7, before)
	if err != nil {
		t.Fatal(err)
	}
	base, err := Open(basePath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer base.Close()

	afterCorpus := workload.CorrectnessV1()
	afterCorpus.Entries = append([]workload.Entry(nil), afterCorpus.Entries...)
	removed := filepath.Join("unicode-é", "case")
	entries := afterCorpus.Entries[:0]
	for _, entry := range afterCorpus.Entries {
		if entry.RelativePath == removed {
			continue
		}
		if entry.RelativePath == filepath.Join("alpha", "README") {
			entry.Mode = 0o644
		}
		entries = append(entries, entry)
	}
	afterCorpus.Entries = append(entries, workload.Entry{
		Object: 30, Parent: 1, RelativePath: filepath.Join("alpha", "added"), Kind: api.ObjectRegular, Size: 9,
	})
	after, err := afterCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}

	var want []Change
	wantSummary, err := Diff(context.Background(), base, after, func(change Change) error {
		want = append(want, change)
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	deltaPath := filepath.Join(t.TempDir(), "changes.delta")
	metadata, err := WriteDeltaCandidate(context.Background(), deltaPath, 8, base, after)
	if err != nil {
		t.Fatal(err)
	}
	if metadata.BaseGeneration != 7 || metadata.Generation != 8 ||
		metadata.Added != wantSummary.Added || metadata.Updated != wantSummary.Updated || metadata.Deleted != wantSummary.Deleted ||
		metadata.CatalogDigest != after.Digest() || metadata.Size >= baseMetadata.Size {
		t.Fatalf("delta metadata=%+v base_size=%d summary=%+v", metadata, baseMetadata.Size, wantSummary)
	}

	reader, err := OpenDeltaCandidate(deltaPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer reader.Close()
	if reader.Root() != root || reader.Metadata() != metadata {
		t.Fatalf("reopened delta metadata/root differ: root=%+v metadata=%+v", reader.Root(), reader.Metadata())
	}
	if err := reader.Check(); err != nil {
		t.Fatal(err)
	}
	var got []Change
	err = reader.Iterate(context.Background(), func(kind ChangeKind, row catalog.Row) error {
		change := Change{Kind: kind, After: row}
		if kind == ChangeDelete {
			change.Before, change.After = row, catalog.Row{}
		}
		got = append(got, change)
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	if len(got) != len(want) {
		t.Fatalf("delta records=%d, want %d", len(got), len(want))
	}
	for index := range want {
		expected := want[index]
		if expected.Kind == ChangeUpdate {
			expected.Before = catalog.Row{}
		}
		if got[index] != expected {
			t.Fatalf("delta record %d mismatch:\n got  %+v\n want %+v", index, got[index], expected)
		}
	}
}

func TestDeltaCandidateRejectsNoopCancellationCorruptionAndTruncation(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	before, err := workload.CorrectnessV1().ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	basePath := filepath.Join(t.TempDir(), "base.seg")
	if _, err := Write(basePath, 1, before); err != nil {
		t.Fatal(err)
	}
	base, err := Open(basePath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer base.Close()

	noopPath := filepath.Join(t.TempDir(), "noop.delta")
	if _, err := WriteDeltaCandidate(context.Background(), noopPath, 2, base, before); !errors.Is(err, ErrNoDeltaChanges) {
		t.Fatalf("no-op delta error=%v", err)
	}
	if _, err := os.Stat(noopPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("no-op delta left a file: %v", err)
	}
	sentinelPath := filepath.Join(t.TempDir(), "sentinel.delta")
	if err := os.WriteFile(sentinelPath, []byte("do-not-touch"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := WriteDeltaCandidate(context.Background(), sentinelPath, 2, base, before); !errors.Is(err, ErrNoDeltaChanges) {
		t.Fatalf("no-op delta over sentinel error=%v", err)
	}
	if got, err := os.ReadFile(sentinelPath); err != nil || string(got) != "do-not-touch" {
		t.Fatalf("no-op delta touched an existing path: content=%q err=%v", got, err)
	}

	cancelled, cancel := context.WithCancel(context.Background())
	cancel()
	cancelledPath := filepath.Join(t.TempDir(), "cancelled.delta")
	if _, err := WriteDeltaCandidate(cancelled, cancelledPath, 2, base, before); !errors.Is(err, context.Canceled) {
		t.Fatalf("cancelled delta error=%v", err)
	}
	if _, err := os.Stat(cancelledPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("cancelled delta left a file: %v", err)
	}

	afterCorpus := workload.CorrectnessV1()
	afterCorpus.Entries = append([]workload.Entry(nil), afterCorpus.Entries...)
	afterCorpus.Entries[0].Size++
	after, err := afterCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	deltaPath := filepath.Join(t.TempDir(), "corrupt.delta")
	metadata, err := WriteDeltaCandidate(context.Background(), deltaPath, 2, base, after)
	if err != nil {
		t.Fatal(err)
	}
	reader, err := OpenDeltaCandidate(deltaPath, root)
	if err != nil {
		t.Fatal(err)
	}
	if err := reader.Close(); err != nil {
		t.Fatal(err)
	}
	file, err := os.OpenFile(deltaPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var value [1]byte
	offset := int64(deltaCandidateHeaderSize + len(root.ID) + len(root.Path) + deltaRecordHeaderSize)
	if _, err := file.ReadAt(value[:], offset); err != nil {
		t.Fatal(err)
	}
	value[0] ^= 0xff
	if _, err := file.WriteAt(value[:], offset); err != nil {
		t.Fatal(err)
	}
	if err := file.Close(); err != nil {
		t.Fatal(err)
	}
	reader, err = OpenDeltaCandidate(deltaPath, root)
	if err != nil {
		t.Fatal(err)
	}
	if err := reader.Check(); err == nil {
		reader.Close()
		t.Fatal("corrupt delta payload passed its checksum")
	}
	if err := reader.Close(); err != nil {
		t.Fatal(err)
	}
	if err := os.Truncate(deltaPath, int64(metadata.Size)-1); err != nil {
		t.Fatal(err)
	}
	if reader, err := OpenDeltaCandidate(deltaPath, root); err == nil {
		reader.Close()
		t.Fatal("truncated delta candidate was opened")
	}
}
