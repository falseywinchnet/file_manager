package scan

import (
	"context"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"

	"filemanager/engine/api"
)

func ownerFor(roots ...api.RootSpec) OwnsFunc {
	return func(rootID api.RootID, path string) bool {
		var best api.RootSpec
		for _, root := range roots {
			relative, err := filepath.Rel(root.Path, path)
			if err == nil && relative != ".." && !strings.HasPrefix(relative, ".."+string(filepath.Separator)) && (best.ID == "" || len(root.Path) > len(best.Path)) {
				best = root
			}
		}
		return best.ID == rootID
	}
}

func TestScanCollectsMetadataWithoutFollowingSymlink(t *testing.T) {
	if runtime.GOOS == "windows" {
		t.Skip("Windows reparse-point no-follow identity is an explicit platform gate")
	}
	container := t.TempDir()
	rootPath := filepath.Join(container, "root")
	outsidePath := filepath.Join(container, "outside")
	if err := os.Mkdir(rootPath, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(outsidePath, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(rootPath, "inside"), []byte("inside"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(outsidePath, "secret"), []byte("secret"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.Symlink(outsidePath, filepath.Join(rootPath, "escape")); err != nil {
		t.Skipf("symlinks unavailable: %v", err)
	}
	root := api.RootSpec{ID: "root", Path: rootPath}
	shard, err := (Scanner{}).Scan(context.Background(), root, ownerFor(root))
	if err != nil {
		t.Fatal(err)
	}
	if shard.Len() != 2 {
		t.Fatalf("record count = %d, want inside file and symlink only", shard.Len())
	}
	if _, ok := shard.Path(filepath.Join(rootPath, "escape", "secret")); ok {
		t.Fatal("scanner followed an escaping directory symlink")
	}
	link, ok := shard.Path(filepath.Join(rootPath, "escape"))
	if !ok || link.Kind != api.ObjectSymlink {
		t.Fatalf("symlink record = %#v, %v", link, ok)
	}
}

func TestScanPrunesMoreSpecificChildRoot(t *testing.T) {
	parentPath := filepath.Join(t.TempDir(), "parent")
	childPath := filepath.Join(parentPath, "child")
	if err := os.MkdirAll(childPath, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(parentPath, "parent-file"), nil, 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(childPath, "child-file"), nil, 0o600); err != nil {
		t.Fatal(err)
	}
	parent := api.RootSpec{ID: "parent", Path: parentPath}
	child := api.RootSpec{ID: "child", Path: childPath}
	shard, err := (Scanner{}).Scan(context.Background(), parent, ownerFor(parent, child))
	if err != nil {
		t.Fatal(err)
	}
	if shard.Len() != 1 {
		t.Fatalf("parent record count = %d, want 1", shard.Len())
	}
	if _, ok := shard.Path(filepath.Join(childPath, "child-file")); ok {
		t.Fatal("parent shard retained child-owned record")
	}
}
