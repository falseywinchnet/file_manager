package identity

import (
	"os"
	"path/filepath"
	"testing"
)

func TestRenamePreservesObjectIdentity(t *testing.T) {
	directory := t.TempDir()
	oldPath := filepath.Join(directory, "old")
	newPath := filepath.Join(directory, "new")
	if err := os.WriteFile(oldPath, []byte("fixture"), 0o600); err != nil {
		t.Fatal(err)
	}
	root, err := os.OpenRoot(directory)
	if err != nil {
		t.Fatal(err)
	}
	defer root.Close()
	oldInfo, err := root.Lstat("old")
	if err != nil {
		t.Fatal(err)
	}
	before, err := Observe(root, "old", oldInfo)
	if err != nil {
		t.Fatal(err)
	}
	if err := os.Rename(oldPath, newPath); err != nil {
		t.Fatal(err)
	}
	newInfo, err := root.Lstat("new")
	if err != nil {
		t.Fatal(err)
	}
	after, err := Observe(root, "new", newInfo)
	if err != nil {
		t.Fatal(err)
	}
	if before.ObjectID() != after.ObjectID() {
		t.Fatalf("object identity changed across rename: %q != %q", before.ObjectID(), after.ObjectID())
	}
	if before.ObjectID() == "" || before.Fields()["object"] == "" {
		t.Fatal("identity observation is incomplete")
	}
}

func TestObjectIDRoundTrip(t *testing.T) {
	want := Observation{
		Platform: PlatformFixture, Volume: 0x12, Object: 0x34,
		Incarnation: Incarnation{A: 0x56, B: 0x78, Available: true},
	}
	got, err := ParseObjectID(want.ObjectID())
	if err != nil {
		t.Fatal(err)
	}
	if Compare(got, want) != 0 {
		t.Fatalf("round trip = %#v, want %#v", got, want)
	}
}

func TestIncarnationDistinguishesReusedNativeIdentifier(t *testing.T) {
	first := Observation{Platform: PlatformFixture, Volume: 1, Object: 2, Incarnation: Incarnation{A: 3, Available: true}}
	second := first
	second.Incarnation.A = 4
	if first.ObjectID() == second.ObjectID() || first.Key() == second.Key() || Compare(first, second) == 0 {
		t.Fatal("distinct incarnations were conflated")
	}
}

func TestHardLinksShareObjectIdentity(t *testing.T) {
	directory := t.TempDir()
	firstPath := filepath.Join(directory, "first")
	secondPath := filepath.Join(directory, "second")
	if err := os.WriteFile(firstPath, []byte("fixture"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.Link(firstPath, secondPath); err != nil {
		t.Skipf("hard links unavailable: %v", err)
	}
	root, err := os.OpenRoot(directory)
	if err != nil {
		t.Fatal(err)
	}
	defer root.Close()
	firstInfo, err := root.Lstat("first")
	if err != nil {
		t.Fatal(err)
	}
	secondInfo, err := root.Lstat("second")
	if err != nil {
		t.Fatal(err)
	}
	first, err := Observe(root, "first", firstInfo)
	if err != nil {
		t.Fatal(err)
	}
	second, err := Observe(root, "second", secondInfo)
	if err != nil {
		t.Fatal(err)
	}
	if first.ObjectID() != second.ObjectID() {
		t.Fatalf("hard-link identities differ: %q != %q", first.ObjectID(), second.ObjectID())
	}
}
