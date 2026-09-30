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
	var container string = t.TempDir()
	var source string = filepath.Join(container, "source")
	var store string = filepath.Join(container, "store")
	var runtimeDir string = filepath.Join(container, "runtime")
	{
		var err error = nil
		err = os.MkdirAll(filepath.Join(source, "private"), 0o700)
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var path string = ""
		for _, path = range []string{store, runtimeDir} {
			{
				var err error = nil
				err = os.Mkdir(path, 0o700)
				if err != nil {
					t.Fatal(err)
				}
			}
		}
	}
	{
		var err error = nil
		err = os.WriteFile(filepath.Join(source, "private", "secret.txt"), []byte("fixture"), 0o600)
		if err != nil {
			t.Fatal(err)
		}
	}
	var canonicalSource string = ""
	var err error = nil
	canonicalSource, err = filepath.EvalSymlinks(source)
	if err != nil {
		t.Fatal(err)
	}
	var objectID string = ""
	objectID, err = deployment.RootObjectID(canonicalSource)
	if err != nil {
		t.Fatal(err)
	}
	var manifest deployment.Manifest = deployment.Manifest{
		Schema: deployment.ManifestSchema, SchemaMajor: deployment.ManifestMajor,
		DeploymentID: "test-installed", HostUUID: "TEST", UID: os.Getuid(),
		StoreRoot: store, RuntimeDir: runtimeDir,
		Roots: []deployment.Root{{ID: "docs", Path: canonicalSource, ObjectID: objectID}},
	}
	var guard *sandbox.Guard = nil
	guard, err = sandbox.NewApproved("test-installed", []sandbox.ApprovedRoot{{ID: "docs", Path: canonicalSource, ObjectID: objectID}})
	if err != nil {
		t.Fatal(err)
	}
	var first *service.Service = nil
	first, err = service.NewPersistent(guard, store)
	if err != nil {
		t.Fatal(err)
	}
	{
		var err error = nil
		_, err = first.ApplyRoots(context.Background(), manifest.RootSpecs())
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		err = ensureManifestAdmission(context.Background(), first, manifest)
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		err = first.Close()
		if err != nil {
			t.Fatal(err)
		}
	}

	manifest.Roots[0].Exclusions = []string{"private"}
	guard, err = sandbox.NewApproved("test-installed", []sandbox.ApprovedRoot{{ID: "docs", Path: canonicalSource, ObjectID: objectID, Exclusions: []string{"private"}}})
	if err != nil {
		t.Fatal(err)
	}
	var second *service.Service = nil
	second, err = service.NewPersistent(guard, store)
	if err != nil {
		t.Fatal(err)
	}
	defer second.Close()
	{
		var err error = nil
		_, err = second.ApplyRoots(context.Background(), manifest.RootSpecs())
		if err != nil {
			t.Fatal(err)
		}
	}
	var before api.Status = api.Status{}
	before, err = second.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	{
		var err error = nil
		err = ensureManifestAdmission(context.Background(), second, manifest)
		if err != nil {
			t.Fatal(err)
		}
	}
	var after api.Status = api.Status{}
	after, err = second.Status(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if after.Generation <= before.Generation {
		t.Fatalf("changed exclusion did not replace recovered generation: before=%d after=%d", before.Generation, after.Generation)
	}
	var response api.QueryResponse = api.QueryResponse{}
	response, err = second.Query(context.Background(), api.Query{
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
	var storeRoot string = t.TempDir()
	var manifest deployment.Manifest = deployment.Manifest{
		Schema: deployment.ManifestSchema, SchemaMajor: deployment.ManifestMajor,
		DeploymentID: "test-installed", HostUUID: "host", UID: os.Getuid(), StoreRoot: storeRoot,
	}
	var commit func(transport.Request, transport.Response) error = manifestAdmissionCommitter(manifest)
	{
		var err error = nil
		if err = commit(
			transport.Request{Method: "engine.scan_reconcile"},
			transport.Response{Result: api.ReconcileReport{Generation: 7}},
		); err != nil {
			t.Fatal(err)
		}
	}
	var matches bool = false
	var err error = nil
	matches, err = deployment.AdmissionMatches(storeRoot, manifest.AdmissionDigest(), 7)
	if err != nil || !matches {
		t.Fatalf("admission match=%t err=%v", matches, err)
	}
}
