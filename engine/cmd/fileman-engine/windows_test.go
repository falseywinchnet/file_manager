//go:build windows

package main

import (
	"bytes"
	"context"
	"encoding/json"
	"filemanager/engine/internal/deployment"
	"filemanager/engine/internal/transport"
	"filemanager/engine/internal/windowssecure"
	"os"
	"path/filepath"
	"testing"
	"time"
)

func TestWindowsManifestLiveAndIndexedRestart(t *testing.T) {
	for _, indexed := range []bool{false, true} {
		label := "live"
		if indexed {
			label = "indexed"
		}
		t.Run(label, func(t *testing.T) {
			parent := t.TempDir()
			source := filepath.Join(parent, "source")
			if err := os.MkdirAll(filepath.Join(source, "private"), 0700); err != nil {
				t.Fatal(err)
			}
			if err := os.WriteFile(filepath.Join(source, "private", "secret.txt"), []byte("fixture"), 0600); err != nil {
				t.Fatal(err)
			}
			runtimeDir := filepath.Join(parent, "runtime")
			manifestPath := filepath.Join(parent, "policy", "manifest.json")
			args := []string{"--deployment-id", "test-win", "--root-id", "docs", "--root-path", source, "--runtime-dir", runtimeDir, "--output", manifestPath}
			if indexed {
				args = append(args, "--index-enabled", "--store-root", filepath.Join(parent, "store"))
			}
			if err := createWindowsManifest(args, &bytes.Buffer{}); err != nil {
				t.Fatal(err)
			}
			manifest, err := deployment.LoadWindows(manifestPath)
			if err != nil {
				t.Fatal(err)
			}
			if manifest.IndexEnabled != indexed {
				t.Fatal("index consent changed")
			}
			serveAndCheckWindows(t, manifestPath, runtimeDir, indexed, false)
			manifest.Roots[0].Exclusions = []string{"private"}
			payload, _ := json.Marshal(manifest)
			if err := windowssecure.Write(manifestPath, payload); err != nil {
				t.Fatal(err)
			}
			serveAndCheckWindows(t, manifestPath, runtimeDir, indexed, true)
			manifest.UserSID = "S-1-5-18"
			payload, _ = json.Marshal(manifest)
			if err := windowssecure.Write(manifestPath, payload); err != nil {
				t.Fatal(err)
			}
			if _, err := deployment.LoadWindows(manifestPath); err == nil {
				t.Fatal("wrong user accepted")
			}
			manifest.UserSID, err = windowssecure.CurrentSID()
			if err != nil {
				t.Fatal(err)
			}
			manifest.HostID = "another-host"
			if err := manifest.Validate(); err == nil {
				t.Fatal("wrong host accepted")
			}
			manifest.HostID, err = deployment.WindowsHostID()
			if err != nil {
				t.Fatal(err)
			}
			if err := os.Rename(source, source+"-old"); err != nil {
				t.Fatal(err)
			}
			if err := os.Mkdir(source, 0700); err != nil {
				t.Fatal(err)
			}
			if err := manifest.Validate(); err == nil {
				t.Fatal("replaced root identity accepted")
			}
		})
	}
}

func serveAndCheckWindows(t *testing.T, manifestPath, runtimeDir string, indexed, excluded bool) {
	t.Helper()
	done := make(chan error, 1)
	go func() { done <- serveWindowsManifest([]string{"--manifest", manifestPath}) }()
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	for {
		_, err := transport.CallLocal(ctx, runtimeDir, transport.AuthorityQuery, transport.Request{ID: "ready", Method: "engine.status"})
		if err == nil {
			break
		}
		select {
		case e := <-done:
			t.Fatalf("startup: %v", e)
		case <-ctx.Done():
			t.Fatal(ctx.Err())
		case <-time.After(10 * time.Millisecond):
		}
	}
	defer func() {
		stopCtx, stop := context.WithTimeout(context.Background(), 3*time.Second)
		defer stop()
		_, err := transport.CallLocal(stopCtx, runtimeDir, transport.AuthorityAdmin, transport.Request{ID: "stop", Method: "engine.shutdown"})
		if err != nil {
			t.Error(err)
		}
		select {
		case err := <-done:
			if err != nil {
				t.Error(err)
			}
		case <-time.After(5 * time.Second):
			t.Error("shutdown timed out")
		}
	}()
	methods := map[string]json.RawMessage{"engine.query_live": json.RawMessage(`{"query_id":"fixture","scope":{"root_id":"docs","descendants":true},"text":"secret"}`)}
	if indexed {
		methods["engine.query"] = json.RawMessage(`{"scope":{"root":"docs","descendants":true},"filters":{"name":"secret.txt"},"limit":10}`)
	}
	for method, params := range methods {
		response, err := transport.CallLocal(ctx, runtimeDir, transport.AuthorityQuery, transport.Request{ID: "query", Method: method, Params: params})
		if err != nil || response.Error != nil {
			t.Fatalf("%s response=%+v err=%v", method, response, err)
		}
		encoded, _ := json.Marshal(response.Result)
		var page struct {
			Results []json.RawMessage `json:"results"`
		}
		if err := json.Unmarshal(encoded, &page); err != nil {
			t.Fatal(err)
		}
		want := 1
		if excluded {
			want = 0
		}
		if len(page.Results) != want {
			t.Fatalf("%s results=%s", method, encoded)
		}
	}
}

func TestWindowsManifestRejectsSourceStateBeforeWriting(t *testing.T) {
	source := t.TempDir()
	err := createWindowsManifest([]string{"--deployment-id", "bad", "--root-id", "docs", "--root-path", source, "--runtime-dir", filepath.Join(source, "runtime"), "--output", filepath.Join(source, "policy", "manifest.json")}, &bytes.Buffer{})
	if err == nil {
		t.Fatal("source state accepted")
	}
	entries, err := os.ReadDir(source)
	if err != nil || len(entries) != 0 {
		t.Fatalf("source changed: %v %v", entries, err)
	}
}
