package ownership

import (
	"errors"
	"path/filepath"
	"strconv"
	"testing"
)

func TestMostSpecificRootOwnsPath(t *testing.T) {
	base := t.TempDir()
	parent := filepath.Join(base, "home")
	child := filepath.Join(parent, "project")
	router := &Router{}
	if err := router.Replace([]Root{{ID: "parent", Path: parent}, {ID: "child", Path: child}}); err != nil {
		t.Fatal(err)
	}

	tests := []struct {
		path string
		want string
	}{
		{filepath.Join(parent, "notes.txt"), "parent"},
		{filepath.Join(child, "src", "main.go"), "child"},
		{child, "child"},
	}
	for _, test := range tests {
		owner, err := router.Owner(test.path)
		if err != nil {
			t.Fatalf("Owner(%q): %v", test.path, err)
		}
		if owner.ID != test.want {
			t.Fatalf("Owner(%q) = %q, want %q", test.path, owner.ID, test.want)
		}
	}
}

func TestUnownedPath(t *testing.T) {
	base := t.TempDir()
	router := &Router{}
	if err := router.Replace([]Root{{ID: "root", Path: filepath.Join(base, "root")}}); err != nil {
		t.Fatal(err)
	}
	if _, err := router.Owner(filepath.Join(base, "elsewhere")); !errors.Is(err, ErrNoOwner) {
		t.Fatalf("Owner error = %v, want ErrNoOwner", err)
	}
}

func BenchmarkOwnerAmongNestedRoots(b *testing.B) {
	base := b.TempDir()
	roots := make([]Root, 512)
	current := base
	for index := range roots {
		current = filepath.Join(current, "r")
		roots[index] = Root{ID: strconv.Itoa(index + 1), Path: current}
	}
	router := &Router{}
	if err := router.Replace(roots); err != nil {
		b.Fatal(err)
	}
	target := filepath.Join(current, "file")
	b.ResetTimer()
	for index := 0; index < b.N; index++ {
		if _, err := router.OwnerCleanAbsolute(target); err != nil {
			b.Fatal(err)
		}
	}
}
