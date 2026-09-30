//go:build windows

package main

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"filemanager/engine/internal/deployment"
	"filemanager/engine/internal/transport"
	"filemanager/engine/internal/windowssecure"
	"os"
	"path/filepath"
	"testing"
	"time"
)

type manifestRestartCase struct{ indexed bool }

func TestWindowsManifestLiveAndIndexedRestart(t *testing.T) {
	var live manifestRestartCase = manifestRestartCase{indexed: false}
	var indexed manifestRestartCase = manifestRestartCase{indexed: true}
	t.Run("live", live.Run)
	t.Run("indexed", indexed.Run)
}

func (test manifestRestartCase) Run(t *testing.T) {
	var parent string = t.TempDir()
	var source string = filepath.Join(parent, "source")
	{
		var err error = nil
		err = os.MkdirAll(filepath.Join(source, "private"), 0700)
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		err = os.WriteFile(filepath.Join(source, "private", "secret.txt"), []byte("fixture"), 0600)
		if err != nil {
			t.Fatal(err)
		}
	}
	var runtimeDir string = filepath.Join(parent, "runtime")
	var manifestPath string = filepath.Join(parent, "policy", "manifest.json")
	var args []string = []string{"--deployment-id", "test-win", "--root-id", "docs", "--root-path", source, "--runtime-dir", runtimeDir, "--output", manifestPath}
	if test.indexed {
		args = append(args, "--index-enabled", "--store-root", filepath.Join(parent, "store"))
	}
	{
		var err error = nil
		err = createWindowsManifest(args, &bytes.Buffer{})
		if err != nil {
			t.Fatal(err)
		}
	}
	var manifest deployment.WindowsManifest = deployment.WindowsManifest{}
	var err error = nil
	manifest, err = deployment.LoadWindows(manifestPath)
	if err != nil {
		t.Fatal(err)
	}
	if manifest.IndexEnabled != test.indexed {
		t.Fatal("index consent changed")
	}
	serveAndCheckWindows(t, manifestPath, runtimeDir, test.indexed, false)
	manifest.Roots[0].Exclusions = []string{"private"}
	var payload []byte = nil
	payload, _ = json.Marshal(manifest)
	{
		var err error = nil
		err = windowssecure.Write(manifestPath, payload)
		if err != nil {
			t.Fatal(err)
		}
	}
	serveAndCheckWindows(t, manifestPath, runtimeDir, test.indexed, true)
	manifest.UserSID = "S-1-5-18"
	payload, _ = json.Marshal(manifest)
	{
		var err error = nil
		err = windowssecure.Write(manifestPath, payload)
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		_, err = deployment.LoadWindows(manifestPath)
		if err == nil {
			t.Fatal("wrong user accepted")
		}
	}
	manifest.UserSID, err = windowssecure.CurrentSID()
	if err != nil {
		t.Fatal(err)
	}
	manifest.HostID = "another-host"
	{
		var err error = nil
		err = manifest.Validate()
		if err == nil {
			t.Fatal("wrong host accepted")
		}
	}
	manifest.HostID, err = deployment.WindowsHostID()
	if err != nil {
		t.Fatal(err)
	}
	{
		var err error = nil
		err = os.Rename(source, source+"-old")
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		err = os.Mkdir(source, 0700)
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		err = manifest.Validate()
		if err == nil {
			t.Fatal("replaced root identity accepted")
		}
	}
}

func serveAndCheckWindows(t *testing.T, manifestPath, runtimeDir string, indexed, excluded bool) {
	t.Helper()
	var ctx context.Context = nil
	var cancel context.CancelFunc = nil
	ctx, cancel = context.WithTimeout(context.Background(), 10*time.Second)
	var server manifestTestServer = manifestTestServer{context: ctx, cancel: cancel, manifestPath: manifestPath, done: make(chan error, 1)}
	go server.Run()
	defer server.Stop(t)
	var startupErr error = nil
	for {
		var err error = nil
		_, err = transport.CallLocal(ctx, runtimeDir, transport.AuthorityQuery, transport.Request{ID: "ready", Method: "engine.status"})
		if err == nil {
			break
		}
		select {
		case startupErr = <-server.done:
			server.done <- startupErr
			t.Fatalf("startup: %v", startupErr)
		case <-ctx.Done():
			t.Fatal(ctx.Err())
		case <-time.After(10 * time.Millisecond):
		}
	}
	var requests []transport.Request = []transport.Request{
		{ID: "query", Method: "engine.query_live", Params: json.RawMessage(`{"query_id":"fixture","scope":{"root_id":"docs","descendants":true},"text":"secret"}`)},
	}
	if indexed {
		requests = append(requests, transport.Request{ID: "query", Method: "engine.query", Params: json.RawMessage(`{"scope":{"root":"docs","descendants":true},"filters":{"name":"secret.txt"},"limit":10}`)})
	}
	{
		var request transport.Request = transport.Request{}
		for _, request = range requests {

			var response transport.Response = transport.Response{}
			var err error = nil
			response, err = transport.CallLocal(ctx, runtimeDir, transport.AuthorityQuery, request)
			if err != nil || response.Error != nil {
				t.Fatalf("%s response=%+v err=%v", request.Method, response, err)
			}
			var encoded []byte = nil
			encoded, _ = json.Marshal(response.Result)
			var page manifestQueryPage = manifestQueryPage{}
			{
				var err error = nil
				err = json.Unmarshal(encoded, &page)
				if err != nil {
					t.Fatal(err)
				}
			}
			var want int = 1
			if excluded {
				want = 0
			}
			if len(page.Results) != want {
				t.Fatalf("%s results=%s", request.Method, encoded)
			}
		}
	}
}

func TestWindowsManifestRejectsSourceStateBeforeWriting(t *testing.T) {
	var source string = t.TempDir()
	var err error = createWindowsManifest([]string{"--deployment-id", "bad", "--root-id", "docs", "--root-path", source, "--runtime-dir", filepath.Join(source, "runtime"), "--output", filepath.Join(source, "policy", "manifest.json")}, &bytes.Buffer{})
	if err == nil {
		t.Fatal("source state accepted")
	}
	var entries []os.DirEntry = nil
	entries, err = os.ReadDir(source)
	if err != nil || len(entries) != 0 {
		t.Fatalf("source changed: %v %v", entries, err)
	}
}

// The test owns the server context and joins it before TempDir cleanup.
type manifestTestServer struct {
	context      context.Context
	cancel       context.CancelFunc
	manifestPath string
	done         chan error
}
type manifestQueryPage struct {
	Results []json.RawMessage `json:"results"`
}

func (server *manifestTestServer) Run() {
	var args []string = []string{"--manifest", server.manifestPath}
	var err error = serveWindowsManifestContext(server.context, args)
	server.done <- err
}
func (server *manifestTestServer) Stop(t *testing.T) {
	t.Helper()
	server.cancel()
	var err error = nil
	select {
	case err = <-server.done:
		if err != nil && !errors.Is(err, context.Canceled) {
			t.Error(err)
		}
	case <-time.After(5 * time.Second):
		t.Error("shutdown timed out")
	}
}
