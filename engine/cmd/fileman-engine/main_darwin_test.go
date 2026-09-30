//go:build darwin

package main

import (
	"context"
	"os"
	"path/filepath"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/deployment"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
	"filemanager/engine/internal/transport"
)

func TestChangedInstalledExclusionsReconcileBeforeServingRecoveredGeneration(t *testing.T) {
	container := t.TempDir()
	source := filepath.Join(container, "source")
	store := filepath.Join(container, "store")
	runtimeDir := filepath.Join(container, "runtime")
	if err := os.MkdirAll(filepath.Join(source, "private"), 0o700); err != nil {
		t.Fatal(err)
	}
	for _, path := range []string{store, runtimeDir} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.WriteFile(filepath.Join(source, "private", "secret.txt"), []byte("fixture"), 0o600); err != nil {
		t.Fatal(err)
	}
	canonicalSource, err := filepath.EvalSymlinks(source)
	if err != nil {
		t.Fatal(err)
	}
	objectID, err := deployment.RootObjectID(canonicalSource)
	if err != nil {
		t.Fatal(err)
	}
	manifest := deployment.Manifest{
		Schema: deployment.ManifestSchema, SchemaMajor: deployment.ManifestMajor,
		DeploymentID: "test-installed", HostUUID: "TEST", UID: os.Getuid(),
		StoreRoot: store, RuntimeDir: runtimeDir,
		Roots: []deployment.Root{{ID: "docs", Path: canonicalSource, ObjectID: objectID}},
	}
	guard, err := sandbox.NewApproved("test-installed", []sandbox.ApprovedRoot{{ID: "docs", Path: canonicalSource, ObjectID: objectID}})
	if err != nil {
		t.Fatal(err)
	}
	first, err := service.NewPersistent(guard, store)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := first.ApplyRoots(context.Background(), manifest.RootSpecs()); err != nil {
		t.Fatal(err)
	}
	if err := ensureManifestAdmission(context.Background(), first, manifest); err != nil {
		t.Fatal(err)
	}
	if err := first.Close(); err != nil {
		t.Fatal(err)
	}

	manifest.Roots[0].Exclusions = []string{"private"}
	guard, err = sandbox.NewApproved("test-installed", []sandbox.ApprovedRoot{{ID: "docs", Path: canonicalSource, ObjectID: objectID, Exclusions: []string{"private"}}})
	if err != nil {
		t.Fatal(err)
	}
	second, err := service.NewPersistent(guard, store)
	if err != nil {
		t.Fatal(err)
	}
	defer second.Close()
	if _, err := second.ApplyRoots(context.Background(), manifest.RootSpecs()); err != nil {
		t.Fatal(err)
	}
	before, err := second.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if err := ensureManifestAdmission(context.Background(), second, manifest); err != nil {
		t.Fatal(err)
	}
	after, err := second.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if after.Generation <= before.Generation {
		t.Fatalf("changed exclusion did not replace recovered generation: before=%d after=%d", before.Generation, after.Generation)
	}
	response, err := second.Query(context.Background(), api.Query{
		Scope: api.Scope{Root: "docs", Descendants: true}, Filters: map[string]string{"name": "secret.txt"}, Limit: 10,
	})
	if err != nil {
		t.Fatal(err)
	}
	if len(response.Results) != 0 {
		t.Fatalf("newly excluded record remained queryable: %+v", response.Results)
	}
}

func TestManifestAdmissionCommitterAdvancesSuccessfulReconciliation(t *testing.T) {
	storeRoot := t.TempDir()
	manifest := deployment.Manifest{
		Schema: deployment.ManifestSchema, SchemaMajor: deployment.ManifestMajor,
		DeploymentID: "test-installed", HostUUID: "host", UID: os.Getuid(), StoreRoot: storeRoot,
	}
	commit := manifestAdmissionCommitter(manifest)
	if err := commit(
		transport.Request{Method: "engine.scan_reconcile"},
		transport.Response{Result: api.ReconcileReport{Generation: 7}},
	); err != nil {
		t.Fatal(err)
	}
	matches, err := deployment.AdmissionMatches(storeRoot, manifest.AdmissionDigest(), 7)
	if err != nil || !matches {
		t.Fatalf("admission match=%t err=%v", matches, err)
	}
}
