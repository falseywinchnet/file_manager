//go:build darwin

package deployment

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"
)

func fixtureManifest(t *testing.T) (Manifest, string) {
	t.Helper()
	container := t.TempDir()
	root := filepath.Join(container, "source")
	store := filepath.Join(container, "store")
	runtimeDir := filepath.Join(container, "runtime")
	for _, path := range []string{root, store, runtimeDir} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	host, err := CurrentHost()
	if err != nil {
		t.Fatal(err)
	}
	objectID, err := RootObjectID(root)
	if err != nil {
		t.Fatal(err)
	}
	return Manifest{
		Schema: ManifestSchema, SchemaMajor: ManifestMajor, DeploymentID: "test-installed",
		HostUUID: host.UUID, UID: host.UID, StoreRoot: store, RuntimeDir: runtimeDir,
		Roots: []Root{{ID: "source", Path: root, ObjectID: objectID, Exclusions: []string{"private"}}},
	}, filepath.Join(container, "manifest.json")
}

func TestValidateBindsHostRootAndPrivateState(t *testing.T) {
	manifest, _ := fixtureManifest(t)
	host, err := CurrentHost()
	if err != nil {
		t.Fatal(err)
	}
	if err := Validate(&manifest, host); err != nil {
		t.Fatal(err)
	}
	wrong := manifest
	wrong.HostUUID = "OTHER"
	if err := Validate(&wrong, host); err == nil {
		t.Fatal("different host was admitted")
	}
	wrong = manifest
	wrong.Roots[0].ObjectID = "darwin:0000000000000000:0000000000000000"
	if err := Validate(&wrong, host); err == nil {
		t.Fatal("changed root identity was admitted")
	}
}

func TestLoadSecureRejectsLooseModeAndSymlink(t *testing.T) {
	manifest, path := fixtureManifest(t)
	payload, err := json.Marshal(manifest)
	if err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(path, payload, 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadSecure(path); err != nil {
		t.Fatal(err)
	}
	if err := os.Chmod(path, 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadSecure(path); err == nil {
		t.Fatal("loosely readable manifest was admitted")
	}
	if err := os.Chmod(path, 0o600); err != nil {
		t.Fatal(err)
	}
	link := filepath.Join(filepath.Dir(path), "manifest-link.json")
	if err := os.Symlink(path, link); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadSecure(link); err == nil {
		t.Fatal("manifest symlink was admitted")
	}
}
