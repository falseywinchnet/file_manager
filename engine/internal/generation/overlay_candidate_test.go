package generation

import (
	"context"
	"errors"
	"path/filepath"
	"reflect"
	"sort"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/exact"
	"filemanager/engine/internal/workload"
)

func TestOverlayCandidateMatchesReferenceAfterMixedMutations(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	beforeCorpus := workload.CorrectnessV1()
	before, err := beforeCorpus.ReferenceShard(root)
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

	afterCorpus := mixedMutationCorpus(t, beforeCorpus)
	after, err := afterCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	deltaPath := filepath.Join(t.TempDir(), "mixed.delta")
	metadata, err := WriteDeltaCandidate(context.Background(), deltaPath, 2, base, after)
	if err != nil {
		t.Fatal(err)
	}
	if metadata.Added != 3 || metadata.Updated != 2 || metadata.Deleted != 2 {
		t.Fatalf("mixed delta metadata=%+v", metadata)
	}
	delta, err := OpenDeltaCandidate(deltaPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer delta.Close()
	overlay, err := NewOverlayCandidate(context.Background(), base, delta, 7)
	if err != nil {
		t.Fatal(err)
	}
	if overlay.Generation() != 2 || overlay.ChangeCount() != 7 || overlay.RetainedRowCount() != 5 || overlay.Digest() != after.Digest() {
		t.Fatalf("overlay identity/counts are wrong: generation=%d changes=%d rows=%d digest=%x",
			overlay.Generation(), overlay.ChangeCount(), overlay.RetainedRowCount(), overlay.Digest())
	}

	assertOverlayRowsEqualShard(t, base, overlay, after)

	store := catalog.NewStore()
	if _, _, err := store.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	if _, err := store.Publish(root.ID, after); err != nil {
		t.Fatal(err)
	}
	snapshot := store.Snapshot()
	queries := []api.Query{
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"path": filepath.Join("alpha", "README")}, Limit: 10},
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"path": filepath.Join("alpha", "file2")}, Limit: 10},
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"path": filepath.Join("unicode-é", "moved-file2")}, Limit: 10},
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"path": filepath.Join("alpha", "file10")}, Limit: 10},
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"name": "shared.bin"}, Limit: 1},
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"name": "added"}, Limit: 10},
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"name": "README", "kind": "file", "size_min": "10"}, Order: []api.SortKey{{Field: "modified", Direction: api.SortDescending}}, Limit: 10},
	}
	for index, query := range queries {
		want, wantCursor, err := exact.Query(context.Background(), snapshot, query)
		if err != nil {
			t.Fatalf("reference query %d: %v", index, err)
		}
		got, gotCursor, err := exact.QueryIndex(context.Background(), snapshot, overlay.Generation(), root.ID, overlay, query)
		if err != nil {
			t.Fatalf("overlay query %d: %v", index, err)
		}
		if !reflect.DeepEqual(got, want) || gotCursor != wantCursor {
			t.Fatalf("query %d differs:\n got  %+v cursor=%q\n want %+v cursor=%q", index, got, gotCursor, want, wantCursor)
		}
		if gotCursor != "" {
			query.Cursor = gotCursor
			want, wantCursor, err = exact.Query(context.Background(), snapshot, query)
			if err != nil {
				t.Fatalf("reference continuation %d: %v", index, err)
			}
			got, gotCursor, err = exact.QueryIndex(context.Background(), snapshot, overlay.Generation(), root.ID, overlay, query)
			if err != nil {
				t.Fatalf("overlay continuation %d: %v", index, err)
			}
			if !reflect.DeepEqual(got, want) || gotCursor != wantCursor {
				t.Fatalf("continuation %d differs:\n got  %+v cursor=%q\n want %+v cursor=%q", index, got, gotCursor, want, wantCursor)
			}
		}
	}

	shared, exists := after.Path(filepath.Join(root.Path, "shared.bin"))
	if !exists {
		t.Fatal("mixed hard-link addition is missing")
	}
	wantInspect, wantErr := exact.Inspect(snapshot, api.ObjectRef{Root: root.ID, ID: shared.ObjectID()})
	gotInspect, gotErr := exact.InspectIndex(snapshot, root.ID, overlay, api.ObjectRef{Root: root.ID, ID: shared.ObjectID()})
	if wantInspect != gotInspect || faultCode(wantErr) != faultCode(gotErr) {
		t.Fatalf("ambiguous hard-link inspect differs: got=%+v/%v want=%+v/%v", gotInspect, gotErr, wantInspect, wantErr)
	}
}

func TestOverlayCandidateEnforcesGenerationAndChangeBudget(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	beforeCorpus := workload.CorrectnessV1()
	before, err := beforeCorpus.ReferenceShard(root)
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
	after, err := mixedMutationCorpus(t, beforeCorpus).ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	deltaPath := filepath.Join(t.TempDir(), "mixed.delta")
	if _, err := WriteDeltaCandidate(context.Background(), deltaPath, 2, base, after); err != nil {
		t.Fatal(err)
	}
	delta, err := OpenDeltaCandidate(deltaPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer delta.Close()
	if _, err := NewOverlayCandidate(context.Background(), base, delta, 6); err == nil {
		t.Fatal("overlay admitted a delta beyond its caller budget")
	}
	cancelled, cancel := context.WithCancel(context.Background())
	cancel()
	if _, err := NewOverlayCandidate(cancelled, base, delta, 7); !errors.Is(err, context.Canceled) {
		t.Fatalf("cancelled overlay error=%v", err)
	}
	otherBasePath := filepath.Join(t.TempDir(), "base-2.seg")
	if _, err := Write(otherBasePath, 2, before); err != nil {
		t.Fatal(err)
	}
	otherBase, err := Open(otherBasePath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer otherBase.Close()
	if _, err := NewOverlayCandidate(context.Background(), otherBase, delta, 7); err == nil {
		t.Fatal("overlay admitted a delta against the wrong base generation")
	}
}

func mixedMutationCorpus(t *testing.T, before workload.Corpus) workload.Corpus {
	t.Helper()
	after := workload.Corpus{Name: before.Name + "-mixed", Entries: append([]workload.Entry(nil), before.Entries...)}
	entries := after.Entries[:0]
	for _, entry := range after.Entries {
		switch entry.RelativePath {
		case filepath.Join("unicode-é", "case"):
			continue
		case filepath.Join("alpha", "README"):
			entry.Mode = 0o644
		case filepath.Join("alpha", "file2"):
			entry.Parent = 2
			entry.RelativePath = filepath.Join("unicode-é", "moved-file2")
		case filepath.Join("alpha", "file10"):
			entry.Object = 31
			entry.Size = 31
		}
		entries = append(entries, entry)
	}
	after.Entries = append(entries,
		workload.Entry{Object: 30, Parent: 1, RelativePath: filepath.Join("alpha", "added"), Kind: api.ObjectRegular, Size: 30},
		workload.Entry{Object: 11, Parent: 0, RelativePath: "shared.bin", Kind: api.ObjectRegular, Size: 4096, Mode: 0o640, ModifiedUnixNano: 200},
	)
	sort.Slice(after.Entries, func(i, j int) bool { return after.Entries[i].RelativePath < after.Entries[j].RelativePath })
	return after
}

func assertOverlayRowsEqualShard(t *testing.T, base *Reader, overlay *OverlayCandidate, after *catalog.Shard) {
	t.Helper()
	for index := uint32(0); uint64(index) < uint64(after.Len()); index++ {
		wantRow, _ := after.Row(index)
		wantRecord, _ := after.Record(index)
		ordinal, exists, err := overlay.PathIndex(wantRecord.Path)
		if err != nil || !exists {
			t.Fatalf("overlay path %q: ordinal=%d exists=%v err=%v", wantRecord.Path, ordinal, exists, err)
		}
		gotRow, exists, err := overlay.Row(ordinal)
		if err != nil || !exists || gotRow != wantRow {
			t.Fatalf("overlay row %q differs: got=%+v exists=%v err=%v want=%+v", wantRecord.Path, gotRow, exists, err, wantRow)
		}
		gotRecord, exists, err := overlay.Record(ordinal)
		if err != nil || !exists || gotRecord != wantRecord {
			t.Fatalf("overlay record %q differs: got=%+v exists=%v err=%v want=%+v", wantRecord.Path, gotRecord, exists, err, wantRecord)
		}
	}
	for index := uint32(0); uint64(index) < base.Len(); index++ {
		baseRecord, _, err := base.Record(index)
		if err != nil {
			t.Fatal(err)
		}
		afterRecord, stillExists := after.Path(baseRecord.Path)
		_, overlayExists, err := overlay.PathIndex(baseRecord.Path)
		if err != nil || stillExists != overlayExists {
			t.Fatalf("overlay ghost/missing path %q: after=%v overlay=%v err=%v", baseRecord.Path, stillExists, overlayExists, err)
		}
		if !stillExists || afterRecord != baseRecord {
			if stale, staleExists, err := overlay.Record(index); err != nil || staleExists {
				t.Fatalf("overlay exposed shadowed base ordinal %d: record=%+v exists=%v err=%v", index, stale, staleExists, err)
			}
		}
	}
}

func faultCode(err error) api.ErrorCode {
	var fault *api.Fault
	if errors.As(err, &fault) {
		return fault.Code
	}
	return ""
}
