package generation

import (
	"context"
	"errors"
	"fmt"
	"math"
	"os"
	"path/filepath"
	"sort"
	"sync"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/workload"
)

func TestTieredIndexCandidateMatchesExactReference(t *testing.T) {
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

	secondShard, err := multiRunSecondCorpus(baseCorpus).ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	firstDelta, firstIndex := writeIndexedTieredTestRun(t, root, base, nil, secondShard, 2)
	defer firstIndex.Close()
	defer firstDelta.Close()
	secondView, err := NewOverlayCandidate(context.Background(), base, firstDelta, int(firstDelta.Metadata().Changes()))
	if err != nil {
		t.Fatal(err)
	}
	thirdShard, err := multiRunThirdCorpus(baseCorpus).ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	secondDelta, secondIndex := writeIndexedTieredTestRun(t, root, base, secondView, thirdShard, 3)
	defer secondIndex.Close()
	defer secondDelta.Close()

	tiered, err := NewTieredIndexCandidate(base, []*DeltaExactIndex{firstIndex, secondIndex}, 2, 32)
	if err != nil {
		t.Fatal(err)
	}
	if tiered.Generation() != 3 || tiered.Digest() != thirdShard.Digest() || tiered.Len() != uint64(thirdShard.Len()) || tiered.RunCount() != 2 {
		t.Fatalf("tiered identity generation=%d len=%d runs=%d", tiered.Generation(), tiered.Len(), tiered.RunCount())
	}

	tieredSecond, err := NewTieredIndexCandidate(base, []*DeltaExactIndex{firstIndex}, 1, 16)
	if err != nil {
		t.Fatal(err)
	}
	cohortPath := filepath.Join(t.TempDir(), "second-cohort.delta")
	cohortMetadata, err := WriteTieredCohort(context.Background(), cohortPath, tieredSecond, tiered)
	if err != nil {
		t.Fatal(err)
	}
	if cohortMetadata.BaseGeneration != 2 || cohortMetadata.BaseCatalogDigest != secondShard.Digest() || cohortMetadata.CatalogDigest != thirdShard.Digest() {
		t.Fatalf("streamed cohort metadata=%+v", cohortMetadata)
	}
	cohort, err := OpenDeltaCandidate(cohortPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer cohort.Close()
	cohortIndexPath := filepath.Join(t.TempDir(), "second-cohort.dxi")
	if _, err := WriteDeltaExactIndex(context.Background(), cohortIndexPath, cohort, int(cohortMetadata.Changes())); err != nil {
		t.Fatal(err)
	}
	cohortIndex, err := OpenDeltaExactIndex(cohortIndexPath, cohort)
	if err != nil {
		t.Fatal(err)
	}
	defer cohortIndex.Close()
	cohortTiered, err := NewTieredIndexCandidate(base, []*DeltaExactIndex{firstIndex, cohortIndex}, 2, 32)
	if err != nil || cohortTiered.Digest() != tiered.Digest() || cohortTiered.Len() != tiered.Len() {
		t.Fatalf("streamed cohort tiered=%+v err=%v", cohortTiered, err)
	}

	consolidatedPath := filepath.Join(t.TempDir(), "base-cohort.delta")
	consolidatedMetadata, err := WriteTieredCohortFromBase(context.Background(), consolidatedPath, base, tiered)
	if err != nil || consolidatedMetadata.BaseGeneration != 1 || consolidatedMetadata.CatalogDigest != tiered.Digest() {
		t.Fatalf("base cohort metadata=%+v err=%v", consolidatedMetadata, err)
	}
	cancelled, cancel := context.WithCancel(context.Background())
	cancel()
	cancelledPath := filepath.Join(t.TempDir(), "cancelled-cohort.delta")
	if _, err := WriteTieredCohort(cancelled, cancelledPath, tieredSecond, tiered); !errors.Is(err, context.Canceled) {
		t.Fatalf("cancelled cohort error=%v", err)
	}
	if _, err := os.Stat(cancelledPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("cancelled cohort left output: %v", err)
	}
	for ordinal := uint32(0); uint64(ordinal) < uint64(thirdShard.Len()); ordinal++ {
		want, exists := thirdShard.Row(ordinal)
		if !exists {
			t.Fatalf("reference row %d disappeared", ordinal)
		}
		virtual, exists, err := tiered.PathIndex(filepath.Join(root.Path, want.RelativePath))
		if err != nil || !exists {
			t.Fatalf("path %q virtual=%d exists=%v err=%v", want.RelativePath, virtual, exists, err)
		}
		got, exists, err := tiered.Row(virtual)
		if err != nil || !exists || got != want {
			t.Fatalf("row %q got=%+v exists=%v err=%v want=%+v", want.RelativePath, got, exists, err, want)
		}
	}

	allNames := make(map[string]struct{})
	allIDs := make(map[api.ObjectID]struct{})
	for ordinal := uint32(0); uint64(ordinal) < uint64(thirdShard.Len()); ordinal++ {
		row, _ := thirdShard.Row(ordinal)
		allNames[row.Name] = struct{}{}
		allIDs[api.ObjectID(row.Identity.ObjectID())] = struct{}{}
	}
	for name := range allNames {
		got, exceeded, err := tiered.CandidateName(name, 100)
		if err != nil || exceeded {
			t.Fatalf("name %q exceeded=%v err=%v", name, exceeded, err)
		}
		gotPaths := tieredPaths(t, tiered, got)
		wantPaths := shardPaths(thirdShard, thirdShard.NameRange(name))
		if !equalStringSlices(gotPaths, wantPaths) {
			t.Fatalf("name %q paths=%v want=%v", name, gotPaths, wantPaths)
		}
		page, total, exceeded, err := tiered.CandidateNamePage(name, 1, 1, 100)
		if err != nil || exceeded || total != len(wantPaths) {
			t.Fatalf("name page %q total=%d exceeded=%v err=%v", name, total, exceeded, err)
		}
		if len(wantPaths) > 1 {
			if gotPage := tieredPaths(t, tiered, page); len(gotPage) != 1 || gotPage[0] != wantPaths[1] {
				t.Fatalf("name page %q=%v want=%q", name, gotPage, wantPaths[1])
			}
		}
	}
	for id := range allIDs {
		got, exceeded, err := tiered.CandidateID(id, 100)
		if err != nil || exceeded {
			t.Fatalf("identity %q exceeded=%v err=%v", id, exceeded, err)
		}
		gotPaths := tieredPaths(t, tiered, got)
		wantPaths := shardPaths(thirdShard, thirdShard.IDRange(id))
		if !equalStringSlices(gotPaths, wantPaths) {
			t.Fatalf("identity %q paths=%v want=%v", id, gotPaths, wantPaths)
		}
	}
	proofRow, exists := thirdShard.Row(0)
	if !exists {
		t.Fatal("reference proof row disappeared")
	}
	proofPage, proofTotal, exceeded, err := tiered.CandidateNamePage(proofRow.Name, 0, 100, 100)
	if err != nil || exceeded || len(proofPage) == 0 {
		t.Fatalf("proof page total=%d exceeded=%v err=%v", proofTotal, exceeded, err)
	}
	wantFirst := proofPage[0]
	proofPage[0] = math.MaxUint32
	proofPage, _, exceeded, err = tiered.CandidateNamePage(proofRow.Name, 0, 100, 100)
	if err != nil || exceeded || len(proofPage) == 0 || proofPage[0] != wantFirst {
		t.Fatalf("caller mutation changed cached proof page: page=%v exceeded=%v err=%v", proofPage, exceeded, err)
	}

	const workers = 8
	errCh := make(chan error, workers)
	var wait sync.WaitGroup
	for worker := 0; worker < workers; worker++ {
		wait.Add(1)
		go func() {
			defer wait.Done()
			for iteration := 0; iteration < 100; iteration++ {
				ordinal, exists, err := tiered.PathIndex(filepath.Join(root.Path, proofRow.RelativePath))
				if err != nil || !exists {
					errCh <- fmt.Errorf("concurrent path exists=%v err=%v", exists, err)
					return
				}
				row, exists, err := tiered.Row(ordinal)
				if err != nil || !exists || row != proofRow {
					errCh <- fmt.Errorf("concurrent row exists=%v err=%v", exists, err)
					return
				}
				if _, _, exceeded, err := tiered.CandidateNamePage(proofRow.Name, 0, 2, 100); err != nil || exceeded {
					errCh <- fmt.Errorf("concurrent name exceeded=%v err=%v", exceeded, err)
					return
				}
			}
		}()
	}
	wait.Wait()
	close(errCh)
	for err := range errCh {
		t.Error(err)
	}

	deleted := filepath.Join(root.Path, "alpha", "file2")
	if _, exists, err := tiered.PathIndex(deleted); err != nil || exists {
		t.Fatalf("deleted path exists=%v err=%v", exists, err)
	}
	if _, err := NewTieredIndexCandidate(base, []*DeltaExactIndex{firstIndex, secondIndex}, 1, 32); err == nil {
		t.Fatal("tiered index ignored run budget")
	}
	if _, err := NewTieredIndexCandidate(base, []*DeltaExactIndex{firstIndex, secondIndex}, 2, 1); err == nil {
		t.Fatal("tiered index ignored change budget")
	}
}

func writeIndexedTieredTestRun(
	t *testing.T,
	root api.RootSpec,
	base *Reader,
	overlay *OverlayCandidate,
	target *catalog.Shard,
	generation api.Generation,
) (*DeltaReader, *DeltaExactIndex) {
	t.Helper()
	deltaPath := filepath.Join(t.TempDir(), "run.delta")
	var metadata DeltaMetadata
	var err error
	if overlay == nil {
		metadata, err = WriteDeltaCandidate(context.Background(), deltaPath, generation, base, target)
	} else {
		metadata, err = WriteDeltaFromOverlayCandidate(context.Background(), deltaPath, generation, overlay, target)
	}
	if err != nil {
		t.Fatal(err)
	}
	delta, err := OpenDeltaCandidate(deltaPath, root)
	if err != nil {
		t.Fatal(err)
	}
	indexPath := filepath.Join(t.TempDir(), "run.dxi")
	if _, err := WriteDeltaExactIndex(context.Background(), indexPath, delta, int(metadata.Changes())); err != nil {
		delta.Close()
		t.Fatal(err)
	}
	index, err := OpenDeltaExactIndex(indexPath, delta)
	if err != nil {
		delta.Close()
		t.Fatal(err)
	}
	return delta, index
}

func tieredPaths(t *testing.T, tiered *TieredIndexCandidate, ordinals []uint32) []string {
	t.Helper()
	paths := make([]string, 0, len(ordinals))
	for _, ordinal := range ordinals {
		row, exists, err := tiered.Row(ordinal)
		if err != nil || !exists {
			t.Fatalf("tiered ordinal %d exists=%v err=%v", ordinal, exists, err)
		}
		paths = append(paths, row.RelativePath)
	}
	sort.Strings(paths)
	return paths
}

func shardPaths(shard *catalog.Shard, ordinals []uint32) []string {
	paths := make([]string, 0, len(ordinals))
	for _, ordinal := range ordinals {
		row, _ := shard.Row(ordinal)
		paths = append(paths, row.RelativePath)
	}
	sort.Strings(paths)
	return paths
}

func equalStringSlices(left, right []string) bool {
	if len(left) != len(right) {
		return false
	}
	for index := range left {
		if left[index] != right[index] {
			return false
		}
	}
	return true
}
