package service

import (
	"context"
	"errors"
	"os"
	"path/filepath"
	"runtime"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
)

func testService(t *testing.T) (*Service, string) {
	t.Helper()
	root := t.TempDir()
	guard, err := sandbox.New(root)
	if err != nil {
		t.Fatal(err)
	}
	service, err := New(guard)
	if err != nil {
		t.Fatal(err)
	}
	return service, root
}

func TestSandboxIdentityOracleRenameMoveReplaceHardLinkAndSymlink(t *testing.T) {
	if runtime.GOOS == "windows" {
		t.Skip("Windows reparse-point no-follow identity adapter is an explicit remaining M1 gate")
	}
	engine, sandboxRoot := testService(t)
	rootPath := filepath.Join(sandboxRoot, "oracle")
	directoryA := filepath.Join(rootPath, "a")
	directoryB := filepath.Join(rootPath, "b")
	if err := os.MkdirAll(directoryA, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(directoryB, 0o700); err != nil {
		t.Fatal(err)
	}
	originalPath := filepath.Join(directoryA, "item")
	if err := os.WriteFile(originalPath, []byte("original"), 0o600); err != nil {
		t.Fatal(err)
	}
	symlinkPath := filepath.Join(rootPath, "item-link")
	if err := os.Symlink(originalPath, symlinkPath); err != nil {
		t.Skipf("symlinks unavailable: %v", err)
	}
	ctx := context.Background()
	plan, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "oracle", Path: rootPath}})
	if err != nil {
		t.Fatal(err)
	}
	canonicalRoot := plan.Roots[0].Path
	canonicalOriginal := filepath.Join(canonicalRoot, "a", "item")
	canonicalSymlink := filepath.Join(canonicalRoot, "item-link")
	if _, err := engine.Reconcile(ctx, "oracle"); err != nil {
		t.Fatal(err)
	}
	originalID := exactPathID(t, engine, canonicalOriginal)
	symlinkID := exactPathID(t, engine, canonicalSymlink)
	if symlinkID == originalID {
		t.Fatal("symbolic link was conflated with its target object")
	}

	renamedPath := filepath.Join(directoryA, "renamed")
	if err := os.Rename(originalPath, renamedPath); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.Reconcile(ctx, "oracle"); err != nil {
		t.Fatal(err)
	}
	if got := exactPathID(t, engine, filepath.Join(canonicalRoot, "a", "renamed")); got != originalID {
		t.Fatalf("rename changed object identity: %q != %q", got, originalID)
	}
	assertPathAbsent(t, engine, canonicalOriginal)

	movedPath := filepath.Join(directoryB, "moved")
	if err := os.Rename(renamedPath, movedPath); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.Reconcile(ctx, "oracle"); err != nil {
		t.Fatal(err)
	}
	canonicalMoved := filepath.Join(canonicalRoot, "b", "moved")
	if got := exactPathID(t, engine, canonicalMoved); got != originalID {
		t.Fatalf("same-volume move changed object identity: %q != %q", got, originalID)
	}

	oldPath := filepath.Join(directoryB, "old-object")
	if err := os.Rename(movedPath, oldPath); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(movedPath, []byte("replacement"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.Reconcile(ctx, "oracle"); err != nil {
		t.Fatal(err)
	}
	replacementID := exactPathID(t, engine, canonicalMoved)
	if replacementID == originalID {
		t.Fatal("replace-at-path retained the old object identity")
	}
	if got := exactPathID(t, engine, filepath.Join(canonicalRoot, "b", "old-object")); got != originalID {
		t.Fatalf("moved old incarnation identity = %q, want %q", got, originalID)
	}

	hardLinkPath := filepath.Join(directoryB, "hard-link")
	if err := os.Link(movedPath, hardLinkPath); err != nil {
		t.Skipf("hard links unavailable: %v", err)
	}
	if _, err := engine.Reconcile(ctx, "oracle"); err != nil {
		t.Fatal(err)
	}
	if got := exactPathID(t, engine, filepath.Join(canonicalRoot, "b", "hard-link")); got != replacementID {
		t.Fatalf("hard-link identity = %q, want %q", got, replacementID)
	}
}

func exactPathID(t *testing.T, engine *Service, path string) api.ObjectID {
	t.Helper()
	response, err := engine.Query(context.Background(), api.Query{
		Scope: api.Scope{Root: "oracle", Descendants: true}, Filters: map[string]string{"path": path},
	})
	if err != nil {
		t.Fatal(err)
	}
	if len(response.Results) != 1 {
		t.Fatalf("exact path %q returned %d results", path, len(response.Results))
	}
	return response.Results[0].Object.ID
}

func assertPathAbsent(t *testing.T, engine *Service, path string) {
	t.Helper()
	response, err := engine.Query(context.Background(), api.Query{
		Scope: api.Scope{Root: "oracle", Descendants: true}, Filters: map[string]string{"path": path},
	})
	if err != nil {
		t.Fatal(err)
	}
	if len(response.Results) != 0 {
		t.Fatalf("stale path %q remained in exact catalogue", path)
	}
}

func TestRootReconcileQueryAndInspect(t *testing.T) {
	service, sandboxRoot := testService(t)
	source := filepath.Join(sandboxRoot, "source")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	source, err := filepath.EvalSymlinks(source)
	if err != nil {
		t.Fatal(err)
	}
	file := filepath.Join(source, "notes.txt")
	if err := os.WriteFile(file, []byte("notes"), 0o600); err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	if _, err := service.ApplyRoots(ctx, []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	report, err := service.Reconcile(ctx, "docs")
	if err != nil {
		t.Fatal(err)
	}
	if report.Records != 1 {
		t.Fatalf("records = %d, want 1", report.Records)
	}
	response, err := service.Query(ctx, api.Query{
		Scope:   api.Scope{Root: "docs", Descendants: true},
		Filters: map[string]string{"name": "notes.txt"},
	})
	if err != nil {
		t.Fatal(err)
	}
	if len(response.Results) != 1 || response.Results[0].Object.Path != file {
		t.Fatalf("query results = %#v", response.Results)
	}
	inspected, err := service.Inspect(ctx, response.Results[0].Object)
	if err != nil {
		t.Fatal(err)
	}
	if inspected.Object.ID != response.Results[0].Object.ID || inspected.Metadata.Size != 5 {
		t.Fatalf("inspect = %#v", inspected)
	}
	integrity, err := service.Integrity(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if !integrity.Healthy {
		t.Fatalf("integrity = %#v", integrity)
	}
}

func TestNestedRootPolicyNeverReturnsParentDuplicate(t *testing.T) {
	service, sandboxRoot := testService(t)
	parent := filepath.Join(sandboxRoot, "parent")
	child := filepath.Join(parent, "child")
	if err := os.MkdirAll(child, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(child, "owned"), nil, 0o600); err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	if _, err := service.ApplyRoots(ctx, []api.RootSpec{{ID: "parent", Path: parent}}); err != nil {
		t.Fatal(err)
	}
	if _, err := service.Reconcile(ctx, "parent"); err != nil {
		t.Fatal(err)
	}
	if _, err := service.ApplyRoots(ctx, []api.RootSpec{{ID: "parent", Path: parent}, {ID: "child", Path: child}}); err != nil {
		t.Fatal(err)
	}
	response, err := service.Query(ctx, api.Query{Scope: api.Scope{Root: "parent", Descendants: true}, Filters: map[string]string{"name": "owned"}})
	if err != nil {
		t.Fatal(err)
	}
	if len(response.Results) != 0 {
		t.Fatalf("parent returned child-owned record: %#v", response.Results)
	}
	if _, err := service.Reconcile(ctx, "child"); err != nil {
		t.Fatal(err)
	}
	childResponse, err := service.Query(ctx, api.Query{Scope: api.Scope{Root: "child", Descendants: true}, Filters: map[string]string{"name": "owned"}})
	if err != nil {
		t.Fatal(err)
	}
	if len(childResponse.Results) != 1 {
		t.Fatalf("child results = %#v", childResponse.Results)
	}
}

func TestRootPlanRejectsOutsideSandbox(t *testing.T) {
	service, _ := testService(t)
	_, err := service.PlanRoots(context.Background(), []api.RootSpec{{ID: "outside", Path: filepath.Dir(service.SandboxRoot())}})
	var fault *api.Fault
	if !errors.As(err, &fault) || fault.Code != api.ErrorOutsideRoot {
		t.Fatalf("outside-root plan error = %v", err)
	}
}
