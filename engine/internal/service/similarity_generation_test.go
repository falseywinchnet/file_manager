package service

import (
	"context"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"sort"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/similarity"
)

// TestPersistentSimilaritySandboxDogfood exercises the experimental channel
// against a disposable real filesystem and the durable exact service. The
// source, exact store, and disposable projection roots are disjoint children
// of t.TempDir; no ambient path is admitted or scanned.
func TestPersistentSimilaritySandboxDogfood(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "exact-store")
	projectionPath := filepath.Join(sandboxPath, "similarity-store")
	for _, path := range []string{source, storePath, projectionPath, filepath.Join(source, "nested")} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	writeDogfoodFile(t, filepath.Join(source, "report-final.txt"), "first identity")
	writeDogfoodFile(t, filepath.Join(source, "obsolete-note.txt"), "delete me")
	writeDogfoodFile(t, filepath.Join(source, "replace-me.txt"), "first incarnation")
	writeDogfoodFile(t, filepath.Join(source, "nested", "résumé-2026.txt"), "unicode")
	for index := 0; index < 32; index++ {
		writeDogfoodFile(t, filepath.Join(source, fmt.Sprintf("record-%04d.txt", index)), "fanout")
	}

	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	if _, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "dogfood", Path: source}}); err != nil {
		t.Fatal(err)
	}
	firstReport, err := engine.Reconcile(ctx, "dogfood")
	if err != nil {
		t.Fatal(err)
	}
	if firstReport.Generation != 1 {
		t.Fatalf("first generation=%d, want 1", firstReport.Generation)
	}

	configuration, err := similarity.DefaultHistoryTupleConfiguration()
	if err != nil {
		t.Fatal(err)
	}
	firstLease, err := engine.PinSimilarityGeneration(ctx, "dogfood")
	if err != nil {
		t.Fatal(err)
	}
	if firstLease.Generation() != firstReport.Generation || firstLease.RecordCount() != uint32(firstReport.Records) {
		t.Fatalf("first similarity lease generation=%d records=%d report=%+v", firstLease.Generation(), firstLease.RecordCount(), firstReport)
	}
	firstProjection := filepath.Join(projectionPath, "generation-1.kht")
	if _, err := similarity.BuildHistoryTuplePostingFileFromResolver(ctx, firstProjection, configuration, firstLease); err != nil {
		t.Fatal(err)
	}
	firstIndex, _, err := similarity.OpenHistoryTuplePostingFile(firstProjection, firstLease)
	if err != nil {
		t.Fatal(err)
	}
	transposed, err := firstIndex.SearchVerified(ctx, "report-fnial.txt", similarity.Budget{})
	if err != nil {
		t.Fatal(err)
	}
	firstCandidate := requireDogfoodCandidate(t, transposed, "report-final.txt", similarity.EditAdjacentTransposition)
	inspected, err := engine.Inspect(ctx, api.ObjectRef{
		Root: firstCandidate.Candidate.Anchor.Root,
		ID:   firstCandidate.Candidate.Anchor.Object,
		Path: firstCandidate.Candidate.Anchor.Path,
	})
	if err != nil || inspected.Object.ID != firstCandidate.Candidate.Anchor.Object || inspected.Generation != firstReport.Generation {
		t.Fatalf("exact inspection did not authorize candidate: result=%+v err=%v", inspected, err)
	}
	unicodeBatch, err := firstIndex.SearchVerified(ctx, "résumé-2206.txt", similarity.Budget{})
	if err != nil {
		t.Fatal(err)
	}
	requireDogfoodCandidate(t, unicodeBatch, "résumé-2026.txt", similarity.EditAdjacentTransposition)

	beforeReads := dogfoodTreeState(t, storePath, projectionPath)
	for index := 0; index < 16; index++ {
		if _, err := engine.Query(ctx, api.Query{
			Scope: api.Scope{Root: "dogfood", Descendants: true}, Filters: map[string]string{"name": "report-final.txt"}, Limit: 8,
		}); err != nil {
			t.Fatal(err)
		}
		if _, err := firstIndex.SearchVerified(ctx, "report-fnial.txt", similarity.Budget{}); err != nil {
			t.Fatal(err)
		}
	}
	afterReads := dogfoodTreeState(t, storePath, projectionPath)
	if !reflect.DeepEqual(afterReads, beforeReads) {
		t.Fatalf("read-only exact/fuzzy queries changed engine files:\nbefore=%v\nafter=%v", beforeReads, afterReads)
	}

	if err := os.Rename(filepath.Join(source, "report-final.txt"), filepath.Join(source, "report-draft.txt")); err != nil {
		t.Fatal(err)
	}
	if err := os.Remove(filepath.Join(source, "obsolete-note.txt")); err != nil {
		t.Fatal(err)
	}
	if err := os.Remove(filepath.Join(source, "replace-me.txt")); err != nil {
		t.Fatal(err)
	}
	writeDogfoodFile(t, filepath.Join(source, "replace-me.txt"), "second incarnation")
	writeDogfoodFile(t, filepath.Join(source, "inserted-note.txt"), "new")
	secondReport, err := engine.Reconcile(ctx, "dogfood")
	if err != nil {
		t.Fatal(err)
	}
	if secondReport.Generation != 2 || secondReport.Records != firstReport.Records {
		t.Fatalf("second report=%+v first=%+v", secondReport, firstReport)
	}
	status, err := engine.Status(ctx)
	if err != nil || !status.Ready || status.Generation != secondReport.Generation {
		t.Fatalf("post-mutation exact status=%+v err=%v", status, err)
	}
	staleBatch, err := firstIndex.SearchVerified(ctx, "report-fnial.txt", similarity.Budget{})
	if err != nil {
		t.Fatal(err)
	}
	staleCandidate := requireDogfoodCandidate(t, staleBatch, "report-final.txt", similarity.EditAdjacentTransposition)
	if staleCandidate.Candidate.Anchor.Generation == status.Generation {
		t.Fatal("old disposable projection silently crossed an exact generation boundary")
	}

	secondLease, err := engine.PinSimilarityGeneration(ctx, "dogfood")
	if err != nil {
		t.Fatal(err)
	}
	if _, _, err := similarity.OpenHistoryTuplePostingFile(firstProjection, secondLease); channelStatus(err) != similarity.StatusConfigurationMismatch {
		t.Fatalf("stale projection opened against same-cardinality generation: %v", err)
	}
	secondProjection := filepath.Join(projectionPath, "generation-2.kht")
	if _, err := similarity.BuildHistoryTuplePostingFileFromResolver(ctx, secondProjection, configuration, secondLease); err != nil {
		t.Fatal(err)
	}
	secondIndex, _, err := similarity.OpenHistoryTuplePostingFile(secondProjection, secondLease)
	if err != nil {
		t.Fatal(err)
	}
	draftBatch, err := secondIndex.SearchVerified(ctx, "report-darft.txt", similarity.Budget{})
	if err != nil {
		t.Fatal(err)
	}
	requireDogfoodCandidate(t, draftBatch, "report-draft.txt", similarity.EditAdjacentTransposition)

	cancelled, cancel := context.WithCancel(ctx)
	cancel()
	cancelledPath := filepath.Join(projectionPath, "cancelled.kht")
	_, err = similarity.BuildHistoryTuplePostingFileFromResolver(cancelled, cancelledPath, configuration, secondLease)
	if channelStatus(err) != similarity.StatusCancelled {
		t.Fatalf("cancelled projection build error=%v", err)
	}
	if _, err := os.Stat(cancelledPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("cancelled build left an artifact: %v", err)
	}

	bounded := configuration
	bounded.Bounds.MaximumPartition = 1
	overCapacityPath := filepath.Join(projectionPath, "over-capacity.kht")
	_, err = similarity.BuildHistoryTuplePostingFileFromResolver(ctx, overCapacityPath, bounded, secondLease)
	if channelStatus(err) != similarity.StatusOverCapacity {
		t.Fatalf("over-capacity projection build error=%v", err)
	}
	if _, err := os.Stat(overCapacityPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("over-capacity build left an artifact: %v", err)
	}
	exactAfterRejectedProjection, err := engine.Query(ctx, api.Query{
		Scope: api.Scope{Root: "dogfood", Descendants: true}, Filters: map[string]string{"name": "report-draft.txt"}, Limit: 8,
	})
	if err != nil || len(exactAfterRejectedProjection.Results) != 1 {
		t.Fatalf("rejected projection impaired exact query: response=%+v err=%v", exactAfterRejectedProjection, err)
	}

	if err := secondIndex.Close(); err != nil {
		t.Fatal(err)
	}
	if err := secondLease.Close(); err != nil {
		t.Fatal(err)
	}
	if err := firstIndex.Close(); err != nil {
		t.Fatal(err)
	}
	if err := firstLease.Close(); err != nil {
		t.Fatal(err)
	}
	if err := engine.Close(); err != nil {
		t.Fatal(err)
	}

	restarted, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	defer restarted.Close()
	restartedLease, err := restarted.PinSimilarityGeneration(ctx, "dogfood")
	if err != nil {
		t.Fatal(err)
	}
	defer restartedLease.Close()
	restartedIndex, _, err := similarity.OpenHistoryTuplePostingFile(secondProjection, restartedLease)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := restartedIndex.SearchVerified(ctx, "report-darft.txt", similarity.Budget{}); err != nil {
		t.Fatal(err)
	}
	if err := restartedIndex.Close(); err != nil {
		t.Fatal(err)
	}

	projection, err := os.OpenFile(secondProjection, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var damaged [1]byte
	if _, err := projection.ReadAt(damaged[:], 16); err != nil {
		t.Fatal(err)
	}
	damaged[0] ^= 0xff
	if _, err := projection.WriteAt(damaged[:], 16); err != nil {
		t.Fatal(err)
	}
	if err := projection.Close(); err != nil {
		t.Fatal(err)
	}
	if _, _, err := similarity.OpenHistoryTuplePostingFile(secondProjection, restartedLease); channelStatus(err) != similarity.StatusCorruptProjection {
		t.Fatalf("damaged disposable projection error=%v", err)
	}
	exactAfterCorruption, err := restarted.Query(ctx, api.Query{
		Scope: api.Scope{Root: "dogfood", Descendants: true}, Filters: map[string]string{"name": "report-draft.txt"}, Limit: 8,
	})
	if err != nil || len(exactAfterCorruption.Results) != 1 || exactAfterCorruption.Generation != secondReport.Generation {
		t.Fatalf("corrupt projection impaired restarted exact service: response=%+v err=%v", exactAfterCorruption, err)
	}
	assertNoEngineWritesInSource(t, source)
}

func writeDogfoodFile(t *testing.T, path, content string) {
	t.Helper()
	if err := os.WriteFile(path, []byte(content), 0o600); err != nil {
		t.Fatal(err)
	}
}

func requireDogfoodCandidate(t *testing.T, batch similarity.VerifiedBatch, name string, kind similarity.EditKind) similarity.VerifiedCandidate {
	t.Helper()
	for _, candidate := range batch.Candidates {
		if filepath.Base(candidate.Candidate.Anchor.Path) == name && candidate.Evidence.Verification.Kind == kind {
			return candidate
		}
	}
	t.Fatalf("verified batch has no %s candidate %q: %+v", kind, name, batch)
	return similarity.VerifiedCandidate{}
}

func channelStatus(err error) similarity.TerminalStatus {
	var channelError *similarity.ChannelError
	if errors.As(err, &channelError) {
		return channelError.Status
	}
	return ""
}

type dogfoodFileState struct {
	Path string
	Size int64
	Mode os.FileMode
	Time int64
}

func dogfoodTreeState(t *testing.T, roots ...string) []dogfoodFileState {
	t.Helper()
	var state []dogfoodFileState
	for _, root := range roots {
		err := filepath.WalkDir(root, func(path string, entry os.DirEntry, err error) error {
			if err != nil {
				return err
			}
			info, err := entry.Info()
			if err != nil {
				return err
			}
			relative, err := filepath.Rel(root, path)
			if err != nil {
				return err
			}
			state = append(state, dogfoodFileState{Path: filepath.Join(filepath.Base(root), relative), Size: info.Size(), Mode: info.Mode(), Time: info.ModTime().UnixNano()})
			return nil
		})
		if err != nil {
			t.Fatal(err)
		}
	}
	sort.Slice(state, func(i, j int) bool { return state[i].Path < state[j].Path })
	return state
}

func assertNoEngineWritesInSource(t *testing.T, source string) {
	t.Helper()
	err := filepath.WalkDir(source, func(path string, entry os.DirEntry, err error) error {
		if err != nil {
			return err
		}
		if entry.IsDir() {
			return nil
		}
		switch filepath.Ext(entry.Name()) {
		case ".seg", ".kht":
			t.Fatalf("engine artifact was written inside indexed source: %s", path)
		}
		if entry.Name() == "MANIFEST.0" || entry.Name() == "MANIFEST.1" {
			t.Fatalf("engine manifest was written inside indexed source: %s", path)
		}
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
}
