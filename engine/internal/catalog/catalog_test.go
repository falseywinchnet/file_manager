package catalog

import (
	"encoding/hex"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/identity"
)

func fixtureIdentity(object uint64) identity.Observation {
	return identity.Observation{Platform: identity.PlatformFixture, Volume: 1, Object: object}
}

func fixtureObject(object uint64, kind api.ObjectKind) Object {
	return Object{Identity: fixtureIdentity(object), Kind: kind}
}

func fixtureBinding(parent uint64, object uint64, path string) ObservedBinding {
	return ObservedBinding{
		Object: fixtureObject(object, api.ObjectRegular), Parent: fixtureIdentity(parent),
		Name: filepath.Base(path), RelativePath: path,
	}
}

func BenchmarkReferenceShardBuild100k(b *testing.B) {
	root := api.RootSpec{ID: "root", Path: filepath.Join(b.TempDir(), "root")}
	observations := make([]ObservedBinding, 100_000)
	for index := range observations {
		name := fmt.Sprintf("file-%06d", index)
		observations[index] = fixtureBinding(0, uint64(index+1), name)
	}
	b.ResetTimer()
	for b.Loop() {
		iteration := append([]ObservedBinding(nil), observations...)
		if _, err := NewShard(root, fixtureObject(0, api.ObjectDirectory), iteration); err != nil {
			b.Fatal(err)
		}
	}
}

func TestPublishedGenerationsAreImmutable(t *testing.T) {
	root := api.RootSpec{ID: "root", Path: filepath.Join(t.TempDir(), "root")}
	store := NewStore()
	if _, _, err := store.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	firstShard, err := NewShard(root, fixtureObject(0, api.ObjectDirectory), []ObservedBinding{fixtureBinding(0, 1, "a")})
	if err != nil {
		t.Fatal(err)
	}
	first, err := store.Publish(root.ID, firstShard)
	if err != nil {
		t.Fatal(err)
	}
	secondShard, err := NewShard(root, fixtureObject(0, api.ObjectDirectory), []ObservedBinding{fixtureBinding(0, 2, "b")})
	if err != nil {
		t.Fatal(err)
	}
	second, err := store.Publish(root.ID, secondShard)
	if err != nil {
		t.Fatal(err)
	}
	if first.Generation == second.Generation {
		t.Fatal("publish did not advance generation")
	}
	if got, _ := first.Roots[root.ID].Shard.Record(0); got.Name != "a" {
		t.Fatalf("old reader generation changed to %q", got.Name)
	}
	if got, _ := second.Roots[root.ID].Shard.Record(0); got.Name != "b" {
		t.Fatalf("new reader generation = %q", got.Name)
	}
}

func TestObjectAndBindingAlgebraPreservesHardLinks(t *testing.T) {
	root := api.RootSpec{ID: "root", Path: filepath.Join(t.TempDir(), "root")}
	shared := fixtureObject(7, api.ObjectRegular)
	shard, err := NewShard(root, fixtureObject(0, api.ObjectDirectory), []ObservedBinding{
		{Object: shared, Parent: fixtureIdentity(0), Name: "first", RelativePath: "first"},
		{Object: shared, Parent: fixtureIdentity(0), Name: "second", RelativePath: "second"},
	})
	if err != nil {
		t.Fatal(err)
	}
	if shard.ObjectCount() != 2 || shard.Len() != 2 {
		t.Fatalf("objects=%d bindings=%d, want root+shared object and two bindings", shard.ObjectCount(), shard.Len())
	}
	if got := len(shard.IDRange(api.ObjectID(shared.Identity.ObjectID()))); got != 2 {
		t.Fatalf("identity addresses = %d, want 2", got)
	}
	bindings := shard.Bindings()
	if bindings[0].Object != bindings[1].Object || bindings[0].Parent != 0 || bindings[1].Parent != 0 {
		t.Fatalf("hard-link bindings = %#v", bindings)
	}
}

func TestRootManifestNamesVolumeAndLightModePolicy(t *testing.T) {
	root := api.RootSpec{ID: "root", Path: filepath.Join(t.TempDir(), "root")}
	rootObject := fixtureObject(0, api.ObjectDirectory)
	shard, err := NewShard(root, rootObject, []ObservedBinding{fixtureBinding(0, 1, "file")})
	if err != nil {
		t.Fatal(err)
	}
	manifest := (Projection{Spec: root, Shard: shard, Generation: 7}).Manifest()
	if manifest.Volume.Key != rootObject.Identity.VolumeKey() || manifest.BindingCount != 1 || manifest.ObjectCount != 2 {
		t.Fatalf("manifest = %#v", manifest)
	}
	if manifest.LightModeThreshold != 65_536 {
		t.Fatalf("light-mode threshold = %d", manifest.LightModeThreshold)
	}
}

func TestLightDirectoryThresholdCountsParentsNotWholeShard(t *testing.T) {
	bindings := []Binding{
		{Parent: 0}, {Parent: 0}, {Parent: 0},
		{Parent: 1}, {Parent: 1},
	}
	if got := lightDirectoryCount(bindings, 3, 3); got != 1 {
		t.Fatalf("light directories = %d, want only parent 0", got)
	}
}

func TestBindingRequiresObservedParent(t *testing.T) {
	root := api.RootSpec{ID: "root", Path: filepath.Join(t.TempDir(), "root")}
	_, err := NewShard(root, fixtureObject(0, api.ObjectDirectory), []ObservedBinding{fixtureBinding(99, 1, filepath.Join("missing", "file"))})
	if err == nil {
		t.Fatal("accepted a binding whose parent object was never observed")
	}
}

func TestCanonicalObjectBindingSerialization(t *testing.T) {
	root := api.RootSpec{ID: "fixture-root", Path: filepath.Join(t.TempDir(), "fixture")}
	directory := fixtureObject(1, api.ObjectDirectory)
	directory.Size = 64
	file := fixtureObject(2, api.ObjectRegular)
	file.Size = 5
	file.Mode = 0o600
	file.ModifiedUnixNano = 1234
	shard, err := NewShard(root, fixtureObject(0, api.ObjectDirectory), []ObservedBinding{
		{Object: directory, Parent: fixtureIdentity(0), Name: "dir", RelativePath: "dir"},
		{Object: file, Parent: fixtureIdentity(1), Name: "file.txt", RelativePath: filepath.Join("dir", "file.txt")},
	})
	if err != nil {
		t.Fatal(err)
	}
	encoded, err := shard.CanonicalBytes()
	if err != nil {
		t.Fatal(err)
	}
	wantHex, err := os.ReadFile(filepath.Join("..", "..", "testdata", "catalog", "v1", "object-binding.hex"))
	if err != nil {
		t.Fatal(err)
	}
	want, err := hex.DecodeString(strings.TrimSpace(string(wantHex)))
	if err != nil {
		t.Fatal(err)
	}
	if string(encoded) != string(want) {
		t.Fatalf("canonical bytes changed:\n got %x\nwant %x", encoded, want)
	}
}

func TestRootPolicyChangeMakesReusedProjectionStale(t *testing.T) {
	base := t.TempDir()
	parent := api.RootSpec{ID: "parent", Path: filepath.Join(base, "parent")}
	child := api.RootSpec{ID: "child", Path: filepath.Join(parent.Path, "child")}
	store := NewStore()
	if _, _, err := store.ApplyRoots([]api.RootSpec{parent}); err != nil {
		t.Fatal(err)
	}
	directory := fixtureObject(1, api.ObjectDirectory)
	shard, err := NewShard(parent, fixtureObject(0, api.ObjectDirectory), []ObservedBinding{
		{Object: directory, Parent: fixtureIdentity(0), Name: "child", RelativePath: "child"},
		fixtureBinding(1, 2, filepath.Join("child", "file")),
	})
	if err != nil {
		t.Fatal(err)
	}
	if _, err := store.Publish(parent.ID, shard); err != nil {
		t.Fatal(err)
	}
	snapshot, changed, err := store.ApplyRoots([]api.RootSpec{parent, child})
	if err != nil {
		t.Fatal(err)
	}
	if !changed || !snapshot.Roots[parent.ID].Stale {
		t.Fatal("nested-root admission did not stale the parent projection")
	}
	owner, ok := snapshot.Owner(filepath.Join(child.Path, "file"))
	if !ok || owner.ID != child.ID {
		t.Fatalf("owner = %#v, %v; want child", owner, ok)
	}
}
