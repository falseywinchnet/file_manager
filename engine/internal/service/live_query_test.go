package service

import (
	"context"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/deployment"
	"filemanager/engine/internal/sandbox"
)

func TestPersistentLiveQueryPerformsNoEngineStoreWrites(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	store := filepath.Join(sandboxPath, "store")
	for _, path := range []string{source, store} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.WriteFile(filepath.Join(source, "needle.txt"), []byte("fixture"), 0o600); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := NewPersistent(guard, store)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	before := treeFingerprint(t, store)
	page, err := engine.QueryLive(context.Background(), api.LiveQuery{
		QueryID: "persistent-no-write", Scope: api.LiveQueryScope{RootID: "docs", Descendants: true}, Text: "needle",
	})
	if err != nil || !page.Complete || len(page.Results) != 1 {
		t.Fatalf("persistent live page=%+v err=%v", page, err)
	}
	after := treeFingerprint(t, store)
	if before != after {
		t.Fatalf("live query changed persistent store: before=%q after=%q", before, after)
	}
}

func TestLiveQueryNeedsNoCatalogueAndPaginatesWithoutLaneSwitch(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	if err := os.MkdirAll(filepath.Join(source, "nested"), 0o700); err != nil {
		t.Fatal(err)
	}
	for _, name := range []string{"needle-one.txt", filepath.Join("nested", "needle-two.txt")} {
		if err := os.WriteFile(filepath.Join(source, name), []byte("fixture"), 0o600); err != nil {
			t.Fatal(err)
		}
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := New(guard)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}

	before, err := engine.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if before.Generation != 1 || before.RootStates[0].Indexed {
		t.Fatalf("test requires an unreconciled root: %+v", before)
	}
	query := api.LiveQuery{
		QueryID: "live-no-catalogue", Scope: api.LiveQueryScope{RootID: "docs", Descendants: true}, Text: "needle",
		Budget: api.LiveQueryBudget{MaxResults: 1, MaxVisitedEntries: 100, MaxStatCalls: 100, MaxWallTimeMS: 1_000, MaxOpenDirectories: 8, MaxResponseBytes: 64 * 1024},
	}
	first, err := engine.QueryLive(context.Background(), query)
	if err != nil {
		t.Fatal(err)
	}
	if first.Source != api.LiveFilesystemSource || first.Complete || first.NextCursor == "" || len(first.Results) != 1 {
		t.Fatalf("first live page = %+v", first)
	}
	paths := map[string]struct{}{first.Results[0].Object.Path: {}}
	page := first
	for pages := 1; !page.Complete && pages < 4; pages++ {
		query.Cursor = page.NextCursor
		page, err = engine.QueryLive(context.Background(), query)
		if err != nil {
			t.Fatal(err)
		}
		if page.ScanID != first.ScanID || (!page.Complete && page.NextCursor == "") {
			t.Fatalf("continued live page = %+v", page)
		}
		for _, result := range page.Results {
			if _, duplicate := paths[result.Object.Path]; duplicate {
				t.Fatalf("pagination repeated result %q", result.Object.Path)
			}
			paths[result.Object.Path] = struct{}{}
		}
	}
	if !page.Complete || len(paths) != 2 {
		t.Fatalf("live pagination did not terminate with both matches: page=%+v paths=%v", page, paths)
	}
	after, err := engine.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if after.Generation != before.Generation || after.RootStates[0].Indexed {
		t.Fatalf("live query changed catalogue state: before=%+v after=%+v", before.RootStates, after.RootStates)
	}
}

func TestLiveQueryDoesNotTraverseDirectorySymlink(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	outside := filepath.Join(sandboxPath, "outside")
	if err := os.MkdirAll(source, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(outside, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(outside, "needle-secret.txt"), []byte("secret"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.Symlink(outside, filepath.Join(source, "needle-link")); err != nil {
		t.Skipf("symlink unavailable: %v", err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := New(guard)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	page, err := engine.QueryLive(context.Background(), api.LiveQuery{
		QueryID: "symlink", Scope: api.LiveQueryScope{RootID: "docs", Descendants: true}, Text: "needle",
	})
	if err != nil {
		t.Fatal(err)
	}
	canonicalSource, err := filepath.EvalSymlinks(source)
	if err != nil {
		t.Fatal(err)
	}
	// Windows intentionally refuses reparse-point identity until its no-follow
	// adapter is admitted. Still require explicit unavailable state, no target
	// result, and exactly one visited entry: the link must never be traversed.
	if runtime.GOOS == "windows" {
		if !page.Complete || len(page.Results) != 0 || len(page.UnavailablePaths) != 1 ||
			page.UnavailablePaths[0] != "needle-link" || page.Work.VisitedEntries != 1 || len(page.Warnings) < 2 {
			t.Fatalf("Windows symlink refusal did not preserve traversal boundary: %+v", page)
		}
		return
	}
	if !page.Complete || len(page.Results) != 1 || page.Results[0].Metadata.Kind != api.ObjectSymlink || page.Results[0].Object.Path != filepath.Join(canonicalSource, "needle-link") {
		t.Fatalf("symlink live results = %+v", page)
	}
}

func TestLiveQueryRejectsAnExcludedStartingScopeBeforeOpeningIt(t *testing.T) {
	container := t.TempDir()
	source := filepath.Join(container, "source")
	excluded := filepath.Join(source, "private")
	if err := os.MkdirAll(excluded, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(excluded, "needle-secret.txt"), []byte("secret"), 0o600); err != nil {
		t.Fatal(err)
	}
	objectID, err := deployment.RootObjectID(source)
	if err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.NewApproved("test-installed", []sandbox.ApprovedRoot{{ID: "docs", Path: source, ObjectID: objectID, Exclusions: []string{"private"}}})
	if err != nil {
		t.Fatal(err)
	}
	engine, err := New(guard)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	_, err = engine.QueryLive(context.Background(), api.LiveQuery{
		QueryID: "excluded-scope", Scope: api.LiveQueryScope{RootID: "docs", RelativePath: "private", Descendants: true}, Text: "needle",
	})
	var fault *api.Fault
	if !errors.As(err, &fault) || fault.Code != api.ErrorUnapprovedRoot {
		t.Fatalf("excluded scope error = %v", err)
	}
}

func treeFingerprint(t *testing.T, root string) string {
	t.Helper()
	var fingerprint string
	err := filepath.Walk(root, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		relative, err := filepath.Rel(root, path)
		if err != nil {
			return err
		}
		fingerprint += fmt.Sprintf("%s:%d:%d:%d\n", relative, info.Mode(), info.Size(), info.ModTime().UnixNano())
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	return fingerprint
}
