package generation

import (
	"context"
	"path/filepath"
	"reflect"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/exact"
	"filemanager/engine/internal/workload"
)

var _ exact.Index = (*Reader)(nil)

func TestExactPipelineMatchesReferenceOverDurableReader(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	corpus, err := workload.ScaleV1(1_000, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	shard, err := corpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	referenceStore := catalog.NewStore()
	if _, _, err := referenceStore.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	reference, err := referenceStore.Publish(root.ID, shard)
	if err != nil {
		t.Fatal(err)
	}
	policyStore := catalog.NewStore()
	if _, _, err := policyStore.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	directory := t.TempDir()
	durableStore, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	reader, err := durableStore.Publish(reference.Generation, shard)
	if err != nil {
		t.Fatal(err)
	}
	defer reader.Close()

	queries := []api.Query{
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"name": "repeated"}, Limit: 17},
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"path": filepath.Join("dir-0000050", "file-000000501")}},
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"name": "repeated", "size_min": "300"}, Order: []api.SortKey{{Field: "size", Direction: api.SortDescending}}, Limit: 9},
		{Scope: api.Scope{Root: root.ID, Descendants: true}, Filters: map[string]string{"kind": "file", "size_min": "990"}, Limit: 7},
	}
	for _, query := range queries {
		want, wantCursor, err := exact.Query(context.Background(), reference, query)
		if err != nil {
			t.Fatal(err)
		}
		got, gotCursor, err := exact.QueryIndex(context.Background(), policyStore.Snapshot(), reference.Generation, root.ID, reader, query)
		if err != nil {
			t.Fatal(err)
		}
		if !reflect.DeepEqual(got, want) || gotCursor != wantCursor {
			t.Fatalf("durable exact mismatch for %+v:\n got  %+v cursor=%q\n want %+v cursor=%q", query, got, gotCursor, want, wantCursor)
		}
	}
}

func TestInspectPipelinePreservesHardLinkAmbiguity(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	shard, err := workload.CorrectnessV1().ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	policyStore := catalog.NewStore()
	if _, _, err := policyStore.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	durableStore, err := OpenStore(t.TempDir(), nil)
	if err != nil {
		t.Fatal(err)
	}
	reader, err := durableStore.Publish(1, shard)
	if err != nil {
		t.Fatal(err)
	}
	defer reader.Close()
	shared, _ := shard.Path(filepath.Join(root.Path, "alpha", "shared.bin"))
	if _, err := exact.InspectIndex(policyStore.Snapshot(), root.ID, reader, api.ObjectRef{Root: root.ID, ID: shared.ObjectID()}); err == nil {
		t.Fatal("hard-link identity without a path was not ambiguous")
	}
	want, err := exact.InspectIndex(policyStore.Snapshot(), root.ID, reader, api.ObjectRef{
		Root: root.ID, ID: shared.ObjectID(), Path: filepath.Join(root.Path, "alpha", "shared.bin"),
	})
	if err != nil || want.ObjectID() != shared.ObjectID() {
		t.Fatalf("path-qualified hard-link inspect=%+v err=%v", want, err)
	}
}
