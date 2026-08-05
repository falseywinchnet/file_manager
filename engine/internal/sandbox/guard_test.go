package sandbox

import (
	"errors"
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func TestGuardContainsOnlySandboxPaths(t *testing.T) {
	root := t.TempDir()
	guard, err := New(root)
	if err != nil {
		t.Fatal(err)
	}

	inside, err := guard.Resolve(filepath.Join("child", "file.txt"))
	if err != nil {
		t.Fatalf("inside path rejected: %v", err)
	}
	if want := filepath.Join(guard.Root(), "child", "file.txt"); inside != want {
		t.Fatalf("inside path = %q, want %q", inside, want)
	}

	outside := filepath.Join(filepath.Dir(root), "outside.txt")
	if _, err := guard.Resolve(outside); !errors.Is(err, ErrOutsideRoot) {
		t.Fatalf("outside path error = %v, want ErrOutsideRoot", err)
	}

	prefixConfusion := root + "-other/file.txt"
	if _, err := guard.Resolve(prefixConfusion); !errors.Is(err, ErrOutsideRoot) {
		t.Fatalf("prefix-confusion path error = %v, want ErrOutsideRoot", err)
	}
}

func TestGuardRequiresRoot(t *testing.T) {
	if _, err := New(""); err == nil {
		t.Fatal("New accepted an empty root")
	}
}

func TestGuardRejectsUnsafeRoots(t *testing.T) {
	if _, err := New("relative"); err == nil {
		t.Fatal("New accepted a relative root")
	}
	if _, err := New(string(filepath.Separator)); err == nil {
		t.Fatal("New accepted the filesystem root")
	}
	current, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}
	if _, err := New(current); err == nil {
		t.Fatal("New accepted the current directory")
	}
	home, err := os.UserHomeDir()
	if err != nil {
		t.Fatal(err)
	}
	if _, err := New(home); err == nil {
		t.Fatal("New accepted the user home directory")
	}
}

func TestGuardRequiresExistingDirectory(t *testing.T) {
	missing := filepath.Join(t.TempDir(), "missing")
	if _, err := New(missing); err == nil {
		t.Fatal("New accepted a nonexistent root")
	}
	file := filepath.Join(t.TempDir(), "file")
	if err := os.WriteFile(file, []byte("fixture"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := New(file); err == nil {
		t.Fatal("New accepted a file as its root")
	}
}

func TestResolveDirectoryRejectsSymlinkEscape(t *testing.T) {
	if runtime.GOOS == "windows" {
		t.Skip("Windows junction/reparse containment is an explicit platform gate")
	}
	container := t.TempDir()
	root := filepath.Join(container, "root")
	outside := filepath.Join(container, "outside")
	if err := os.Mkdir(root, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(outside, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Symlink(outside, filepath.Join(root, "escape")); err != nil {
		t.Skipf("symlinks unavailable: %v", err)
	}
	guard, err := New(root)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := guard.ResolveDirectory("escape"); !errors.Is(err, ErrOutsideRoot) {
		t.Fatalf("ResolveDirectory symlink escape error = %v, want ErrOutsideRoot", err)
	}
}
