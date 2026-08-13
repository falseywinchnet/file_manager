package exact

import (
	"context"
	"errors"
	"path/filepath"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
	"filemanager/engine/internal/workload"
)

func queryFixture(t *testing.T) (*catalog.Store, api.RootSpec) {
	t.Helper()
	root := api.RootSpec{ID: "root", Path: filepath.Join(t.TempDir(), "root")}
	rootObject := fixtureObject(0, api.ObjectDirectory, 0)
	directoryA := fixtureObject(1, api.ObjectDirectory, 0)
	directoryB := fixtureObject(2, api.ObjectDirectory, 0)
	observations := []catalog.ObservedBinding{
		{Object: directoryA, Parent: rootObject.Identity, Name: "a", RelativePath: "a"},
		{Object: fixtureObject(3, api.ObjectRegular, 1), Parent: directoryA.Identity, Name: "same", RelativePath: filepath.Join("a", "same")},
		{Object: directoryB, Parent: rootObject.Identity, Name: "b", RelativePath: "b"},
		{Object: fixtureObject(4, api.ObjectRegular, 2), Parent: directoryB.Identity, Name: "same", RelativePath: filepath.Join("b", "same")},
		{Object: fixtureObject(5, api.ObjectRegular, 3), Parent: directoryB.Identity, Name: "other", RelativePath: filepath.Join("b", "other")},
	}
	shard, err := catalog.NewShard(root, rootObject, observations)
	if err != nil {
		t.Fatal(err)
	}
	store := catalog.NewStore()
	if _, _, err := store.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	if _, err := store.Publish(root.ID, shard); err != nil {
		t.Fatal(err)
	}
	return store, root
}

func fixtureObject(object uint64, kind api.ObjectKind, size int64) catalog.Object {
	return catalog.Object{
		Identity: identity.Observation{Platform: identity.PlatformFixture, Volume: 1, Object: object},
		Kind:     kind, Size: size,
	}
}

func benchmarkQuerySnapshot(b *testing.B, records int, repeatedEvery int) (*catalog.Snapshot, api.Query) {
	b.Helper()
	root := api.RootSpec{ID: "root", Path: filepath.Join(b.TempDir(), "root")}
	directorySize := 1000
	if repeatedEvery != 0 {
		directorySize = repeatedEvery
	}
	corpus, err := workload.ScaleV1(records, directorySize, repeatedEvery)
	if err != nil {
		b.Fatal(err)
	}
	shard, err := corpus.ReferenceShard(root)
	if err != nil {
		b.Fatal(err)
	}
	store := catalog.NewStore()
	if _, _, err := store.ApplyRoots([]api.RootSpec{root}); err != nil {
		b.Fatal(err)
	}
	if _, err := store.Publish(root.ID, shard); err != nil {
		b.Fatal(err)
	}
	return store.Snapshot(), api.Query{
		Scope: api.Scope{Root: root.ID, Descendants: true}, Limit: 100,
		Filters: map[string]string{"name": "repeated"},
	}
}

func BenchmarkExactNameUnique100k(b *testing.B) {
	snapshot, request := benchmarkQuerySnapshot(b, 100_000, 0)
	request.Filters["name"] = "file-000050000"
	b.ResetTimer()
	for b.Loop() {
		matches, _, err := Query(context.Background(), snapshot, request)
		if err != nil || len(matches) != 1 {
			b.Fatalf("matches=%d err=%v", len(matches), err)
		}
	}
}

func BenchmarkExactNameRepeated10kOf100k(b *testing.B) {
	snapshot, request := benchmarkQuerySnapshot(b, 100_000, 10)
	b.ResetTimer()
	for b.Loop() {
		matches, _, err := Query(context.Background(), snapshot, request)
		if err != nil || len(matches) != 100 {
			b.Fatalf("matches=%d err=%v", len(matches), err)
		}
	}
}

func TestExactQueryPaginationIsGenerationBound(t *testing.T) {
	store, root := queryFixture(t)
	request := api.Query{Scope: api.Scope{Root: root.ID, Descendants: true}, Limit: 1, Filters: map[string]string{"name": "same"}}
	first, cursor, err := Query(context.Background(), store.Snapshot(), request)
	if err != nil {
		t.Fatal(err)
	}
	if len(first) != 1 || cursor == "" || first[0].Record.Path != filepath.Join(root.Path, "a", "same") {
		t.Fatalf("first page = %#v, cursor %q", first, cursor)
	}
	request.Cursor = cursor
	second, next, err := Query(context.Background(), store.Snapshot(), request)
	if err != nil {
		t.Fatal(err)
	}
	if len(second) != 1 || next != "" || second[0].Record.Path != filepath.Join(root.Path, "b", "same") || second[0].Rank != 2 {
		t.Fatalf("second page = %#v, cursor %q", second, next)
	}
	if _, err := store.MarkStale(root.ID, "fixture generation advance"); err != nil {
		t.Fatal(err)
	}
	if _, _, err := Query(context.Background(), store.Snapshot(), request); err == nil {
		t.Fatal("old cursor was accepted against a newer generation")
	} else {
		var fault *api.Fault
		if !errors.As(err, &fault) || fault.Code != api.ErrorGenerationExpired {
			t.Fatalf("cursor error = %v", err)
		}
	}
}

func TestExactQueryRejectsUndefinedLexicalSemantics(t *testing.T) {
	store, root := queryFixture(t)
	_, _, err := Query(context.Background(), store.Snapshot(), api.Query{Text: "same", Scope: api.Scope{Root: root.ID}})
	var fault *api.Fault
	if !errors.As(err, &fault) || fault.Code != api.ErrorMethodUnavailable {
		t.Fatalf("text query error = %v", err)
	}
}

func TestMetadataOnlyQueryIsBoundedPagedAndGenerationBound(t *testing.T) {
	store, root := queryFixture(t)
	request := api.Query{
		Scope:   api.Scope{Root: root.ID, Descendants: true},
		Filters: map[string]string{"kind": "file", "size_min": "2"},
		Limit:   1,
	}
	first, cursor, err := Query(context.Background(), store.Snapshot(), request)
	if err != nil {
		t.Fatal(err)
	}
	if len(first) != 1 || cursor == "" || first[0].Record.Path != filepath.Join(root.Path, "b", "other") ||
		len(first[0].Evidence) != 1 || first[0].Evidence[0].Kind != api.EvidenceMetadata {
		t.Fatalf("metadata first page = %#v, cursor %q", first, cursor)
	}
	request.Cursor = cursor
	second, next, err := Query(context.Background(), store.Snapshot(), request)
	if err != nil {
		t.Fatal(err)
	}
	if len(second) != 1 || next != "" || second[0].Record.Path != filepath.Join(root.Path, "b", "same") || second[0].Rank != 2 {
		t.Fatalf("metadata second page = %#v, cursor %q", second, next)
	}
	if _, err := store.MarkStale(root.ID, "metadata fixture generation advance"); err != nil {
		t.Fatal(err)
	}
	if _, _, err := Query(context.Background(), store.Snapshot(), request); err == nil {
		t.Fatal("old metadata cursor was accepted against a newer generation")
	} else {
		var fault *api.Fault
		if !errors.As(err, &fault) || fault.Code != api.ErrorGenerationExpired {
			t.Fatalf("metadata cursor error = %v", err)
		}
	}
}

func TestMetadataOnlyQueryHonorsScopeAndCandidateBudget(t *testing.T) {
	store, root := queryFixture(t)
	request := api.Query{
		Scope:   api.Scope{Root: root.ID, Path: "a", Descendants: false},
		Filters: map[string]string{"kind": "file"},
	}
	matches, _, err := Query(context.Background(), store.Snapshot(), request)
	if err != nil {
		t.Fatal(err)
	}
	if len(matches) != 1 || matches[0].Record.Path != filepath.Join(root.Path, "a", "same") {
		t.Fatalf("scoped metadata matches = %#v", matches)
	}

	snapshot := store.Snapshot()
	projection := snapshot.Roots[root.ID]
	limited := candidateBudgetIndex{Index: referenceIndex{projection.Shard}}
	_, _, err = QueryIndex(context.Background(), snapshot, snapshot.Generation, root.ID, limited, api.Query{
		Scope:   api.Scope{Root: root.ID, Descendants: true},
		Filters: map[string]string{"kind": "file"},
	})
	var fault *api.Fault
	if !errors.As(err, &fault) || fault.Code != api.ErrorResourceBudget {
		t.Fatalf("metadata candidate budget error = %v", err)
	}
}

func TestMetadataOnlyQueryPreservesCancellation(t *testing.T) {
	store, root := queryFixture(t)
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	_, _, err := Query(ctx, store.Snapshot(), api.Query{
		Scope:   api.Scope{Root: root.ID, Descendants: true},
		Filters: map[string]string{"kind": "file"},
	})
	if !errors.Is(err, context.Canceled) {
		t.Fatalf("metadata cancellation error = %v", err)
	}
}

type candidateBudgetIndex struct{ Index }

func (candidateBudgetIndex) CandidateAll(context.Context, int) ([]uint32, bool, error) {
	return nil, true, nil
}

func TestInspectRequiresAddressForHardLinks(t *testing.T) {
	root := api.RootSpec{ID: "root", Path: filepath.Join(t.TempDir(), "root")}
	rootObject := fixtureObject(0, api.ObjectDirectory, 0)
	shared := fixtureObject(1, api.ObjectRegular, 7)
	shard, err := catalog.NewShard(root, rootObject, []catalog.ObservedBinding{
		{Object: shared, Parent: rootObject.Identity, Name: "first", RelativePath: "first"},
		{Object: shared, Parent: rootObject.Identity, Name: "second", RelativePath: "second"},
	})
	if err != nil {
		t.Fatal(err)
	}
	store := catalog.NewStore()
	if _, _, err := store.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	if _, err := store.Publish(root.ID, shard); err != nil {
		t.Fatal(err)
	}
	objectID := api.ObjectID(shared.Identity.ObjectID())
	_, err = Inspect(store.Snapshot(), api.ObjectRef{Root: root.ID, ID: objectID})
	var fault *api.Fault
	if !errors.As(err, &fault) || fault.Code != api.ErrorAmbiguousObject {
		t.Fatalf("identity-only inspect error = %v", err)
	}
	if _, err := Inspect(store.Snapshot(), api.ObjectRef{Root: root.ID, ID: objectID, Path: filepath.Join(root.Path, "first")}); err != nil {
		t.Fatalf("address-qualified inspect: %v", err)
	}
}
