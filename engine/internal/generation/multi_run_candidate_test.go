package generation

import (
	"context"
	"errors"
	"os"
	"path/filepath"
	"reflect"
	"sort"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/workload"
)

func TestMultiRunOverlayAndConsolidationMatchReference(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	baseCorpus := workload.CorrectnessV1()
	baseShard, err := baseCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	basePath := filepath.Join(t.TempDir(), "base.seg")
	if _, err := Write(basePath, 1, baseShard); err != nil {
		t.Fatal(err)
	}
	base, err := Open(basePath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer base.Close()

	secondCorpus := multiRunSecondCorpus(baseCorpus)
	secondShard, err := secondCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	firstPath := filepath.Join(t.TempDir(), "run-2.delta")
	firstMetadata, err := WriteDeltaCandidate(context.Background(), firstPath, 2, base, secondShard)
	if err != nil {
		t.Fatal(err)
	}
	first, err := OpenDeltaCandidate(firstPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer first.Close()
	secondView, err := NewOverlayCandidate(context.Background(), base, first, 3)
	if err != nil {
		t.Fatal(err)
	}

	thirdCorpus := multiRunThirdCorpus(baseCorpus)
	thirdShard, err := thirdCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	secondPath := filepath.Join(t.TempDir(), "run-3.delta")
	secondMetadata, err := WriteDeltaFromOverlayCandidate(context.Background(), secondPath, 3, secondView, thirdShard)
	if err != nil {
		t.Fatal(err)
	}
	second, err := OpenDeltaCandidate(secondPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer second.Close()
	if secondMetadata.BaseGeneration != 2 || secondMetadata.BaseCatalogDigest != secondShard.Digest() ||
		secondMetadata.CatalogDigest != thirdShard.Digest() {
		t.Fatalf("second run does not bind its exact predecessor: %+v", secondMetadata)
	}

	runs := []*DeltaReader{first, second}
	overlay, err := NewMultiRunOverlayCandidate(context.Background(), base, runs, 2, 10)
	if err != nil {
		t.Fatal(err)
	}
	if overlay.Generation() != 3 || overlay.Digest() != thirdShard.Digest() || overlay.RunCount() != 2 ||
		overlay.ChangeCount() != firstMetadata.Changes()+secondMetadata.Changes() || overlay.ChangedPathCount() != 4 ||
		overlay.Len() != uint64(thirdShard.Len()) {
		t.Fatalf("multi-run identity/counts are wrong: generation=%d runs=%d changes=%d paths=%d len=%d",
			overlay.Generation(), overlay.RunCount(), overlay.ChangeCount(), overlay.ChangedPathCount(), overlay.Len())
	}
	assertOverlayRowsEqualShard(t, base, overlay, thirdShard)
	assertOrderedRowsEqualShard(t, overlay, thirdShard)

	consolidatedPath := filepath.Join(t.TempDir(), "consolidated.delta")
	consolidatedMetadata, err := WriteConsolidatedDeltaCandidate(context.Background(), consolidatedPath, overlay)
	if err != nil {
		t.Fatal(err)
	}
	if consolidatedMetadata.BaseGeneration != 1 || consolidatedMetadata.Generation != 3 ||
		consolidatedMetadata.BaseCatalogDigest != base.Digest() || consolidatedMetadata.CatalogDigest != thirdShard.Digest() ||
		consolidatedMetadata.Changes() != 4 || consolidatedMetadata.Changes() >= overlay.ChangeCount() {
		t.Fatalf("consolidated metadata=%+v overlay_changes=%d", consolidatedMetadata, overlay.ChangeCount())
	}
	consolidated, err := OpenDeltaCandidate(consolidatedPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer consolidated.Close()
	consolidatedView, err := NewOverlayCandidate(context.Background(), base, consolidated, 4)
	if err != nil {
		t.Fatal(err)
	}
	assertOverlayRowsEqualShard(t, base, consolidatedView, thirdShard)
	assertOrderedRowsEqualShard(t, consolidatedView, thirdShard)

	if _, err := NewMultiRunOverlayCandidate(context.Background(), base, runs, 1, 10); err == nil {
		t.Fatal("multi-run overlay ignored its run-count budget")
	}
	if _, err := NewMultiRunOverlayCandidate(context.Background(), base, runs, 2, 9); err == nil {
		t.Fatal("multi-run overlay ignored its cumulative change budget")
	}
	if _, err := NewMultiRunOverlayCandidate(context.Background(), base, []*DeltaReader{second, first}, 2, 10); err == nil {
		t.Fatal("multi-run overlay admitted an out-of-order digest chain")
	}

	wrongCorpus := cloneCorpus(baseCorpus)
	for index := range wrongCorpus.Entries {
		if wrongCorpus.Entries[index].RelativePath == filepath.Join("alpha", "file10") {
			wrongCorpus.Entries[index].Size++
		}
	}
	wrongShard, err := wrongCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	wrongPath := filepath.Join(t.TempDir(), "wrong-base.seg")
	if _, err := Write(wrongPath, 1, wrongShard); err != nil {
		t.Fatal(err)
	}
	wrongBase, err := Open(wrongPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer wrongBase.Close()
	if _, err := NewMultiRunOverlayCandidate(context.Background(), wrongBase, runs, 2, 10); err == nil {
		t.Fatal("multi-run overlay admitted a matching generation with the wrong catalogue digest")
	}

	cancelled, cancel := context.WithCancel(context.Background())
	cancel()
	cancelledPath := filepath.Join(t.TempDir(), "cancelled-consolidation.delta")
	if _, err := WriteConsolidatedDeltaCandidate(cancelled, cancelledPath, overlay); !errors.Is(err, context.Canceled) {
		t.Fatalf("cancelled consolidation error=%v", err)
	}
	if _, err := os.Stat(cancelledPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("cancelled consolidation left output: %v", err)
	}
}

func assertOrderedRowsEqualShard(t *testing.T, overlay *OverlayCandidate, want *catalog.Shard) {
	t.Helper()
	gotRows := make([]catalog.Row, 0, overlay.Len())
	if err := overlay.IterateRows(context.Background(), func(row catalog.Row) error {
		gotRows = append(gotRows, row)
		return nil
	}); err != nil {
		t.Fatal(err)
	}
	wantRows := make([]catalog.Row, 0, want.Len())
	for index := uint32(0); uint64(index) < uint64(want.Len()); index++ {
		row, exists := want.Row(index)
		if !exists {
			t.Fatalf("reference row %d disappeared", index)
		}
		wantRows = append(wantRows, row)
	}
	if !reflect.DeepEqual(gotRows, wantRows) {
		t.Fatalf("ordered overlay rows differ:\n got  %+v\n want %+v", gotRows, wantRows)
	}
}

func multiRunSecondCorpus(base workload.Corpus) workload.Corpus {
	result := cloneCorpus(base)
	entries := result.Entries[:0]
	for _, entry := range result.Entries {
		switch entry.RelativePath {
		case filepath.Join("unicode-é", "case"):
			continue
		case filepath.Join("alpha", "README"):
			entry.Mode = 0o644
		}
		entries = append(entries, entry)
	}
	result.Entries = append(entries, workload.Entry{
		Object: 30, Parent: 1, RelativePath: filepath.Join("alpha", "added"), Kind: api.ObjectRegular, Size: 30,
	})
	sort.Slice(result.Entries, func(i, j int) bool { return result.Entries[i].RelativePath < result.Entries[j].RelativePath })
	return result
}

func multiRunThirdCorpus(base workload.Corpus) workload.Corpus {
	result := cloneCorpus(base)
	for index := range result.Entries {
		switch result.Entries[index].RelativePath {
		case filepath.Join("alpha", "file10"):
			result.Entries[index].Size++
		case filepath.Join("alpha", "file2"):
			result.Entries[index].Parent = 2
			result.Entries[index].RelativePath = filepath.Join("unicode-é", "moved-file2")
		}
	}
	result.Entries = append(result.Entries, workload.Entry{
		Object: 11, Parent: 0, RelativePath: "shared.bin", Kind: api.ObjectRegular,
		Size: 4096, Mode: 0o640, ModifiedUnixNano: 200,
	})
	sort.Slice(result.Entries, func(i, j int) bool { return result.Entries[i].RelativePath < result.Entries[j].RelativePath })
	return result
}

func cloneCorpus(source workload.Corpus) workload.Corpus {
	return workload.Corpus{Name: source.Name, Entries: append([]workload.Entry(nil), source.Entries...)}
}
