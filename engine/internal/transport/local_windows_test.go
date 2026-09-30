//go:build windows

package transport

import (
	"context"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	winio "github.com/Microsoft/go-winio"
	"golang.org/x/sys/windows"

	"filemanager/engine/api"
	"filemanager/engine/internal/deployment"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
	"filemanager/engine/internal/windowssecure"
)

func TestWindowsNamedPipeLiveAuthorityAndLifecycle(t *testing.T) {
	parent := t.TempDir()
	source := filepath.Join(parent, "source")
	runtimeDir := filepath.Join(parent, "runtime")
	if err := os.Mkdir(source, 0700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "needle.txt"), []byte("fixture"), 0600); err != nil {
		t.Fatal(err)
	}
	if err := windowssecure.CreateDirectory(runtimeDir); err != nil {
		t.Fatal(err)
	}
	object, err := deployment.RootObjectID(source)
	if err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.NewApproved("windows-test", []sandbox.ApprovedRoot{{ID: "docs", Path: source, ObjectID: object}})
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()
	done := make(chan error, 1)
	go func() { done <- ServeLocal(ctx, runtimeDir, engine) }()
	defer func() {
		cancel()
		select {
		case <-done:
		case <-time.After(5 * time.Second):
			t.Error("server failed to stop")
		}
	}()
	var discovery WindowsDiscovery
	for {
		discovery, err = loadWindowsDiscovery(runtimeDir)
		if err == nil {
			break
		}
		select {
		case err := <-done:
			done <- err
			t.Fatalf("server startup: %v", err)
		case <-ctx.Done():
			t.Fatal(ctx.Err())
		case <-time.After(10 * time.Millisecond):
		}
	}
	// Discovery may not redirect credentials to a different server process.
	changed := discovery
	changed.ServerPID++
	payload, _ := json.Marshal(changed)
	if err := windowssecure.Write(Files(runtimeDir).Discovery, payload); err != nil {
		t.Fatal(err)
	}
	if _, err := CallLocal(ctx, runtimeDir, AuthorityQuery, Request{ID: "wrong-pid", Method: "engine.version"}); err == nil {
		t.Fatal("wrong server PID accepted")
	}
	payload, _ = json.Marshal(discovery)
	if err := windowssecure.Write(Files(runtimeDir).Discovery, payload); err != nil {
		t.Fatal(err)
	}
	// Query credentials cannot authenticate to the administrative endpoint.
	token, err := windowssecure.Read(discovery.QueryTokenFile, 64)
	if err != nil {
		t.Fatal(err)
	}
	connection, err := winio.DialPipeAccessImpLevel(ctx, discovery.AdminPipe, windows.GENERIC_READ|windows.GENERIC_WRITE, winio.PipeImpLevelIdentification)
	if err != nil {
		t.Fatal(err)
	}
	connection.SetDeadline(time.Now().Add(2 * time.Second))
	if err := writeFrame(connection, hello{Protocol: LocalProtocol, Authority: AuthorityAdmin, Token: string(token)}); err != nil {
		t.Fatal(err)
	}
	var reply helloResponse
	err = readFrame(connection, &reply)
	connection.Close()
	if err != nil || reply.Accepted {
		t.Fatalf("cross-authority hello=%+v err=%v", reply, err)
	}
	response, err := CallLocal(ctx, runtimeDir, AuthorityQuery, Request{ID: "denied", Method: "engine.shutdown"})
	if err != nil || response.Error == nil {
		t.Fatalf("query shutdown=%+v err=%v", response, err)
	}
	params, _ := json.Marshal(api.LiveQuery{QueryID: "live", Scope: api.LiveQueryScope{RootID: "docs", Descendants: true}, Text: "needle"})
	response, err = CallLocal(ctx, runtimeDir, AuthorityQuery, Request{ID: "live", Method: "engine.query_live", Params: params})
	if err != nil || response.Error != nil {
		t.Fatalf("live=%+v err=%v", response, err)
	}
	encoded, _ := json.Marshal(response.Result)
	if !strings.Contains(string(encoded), "needle.txt") || !strings.Contains(string(encoded), "live_filesystem") {
		t.Fatalf("live result=%s", encoded)
	}
	status, err := engine.Status(ctx)
	if err != nil || status.RootStates[0].Indexed {
		t.Fatalf("live search created catalogue: %+v err=%v", status, err)
	}
	// A connected client that sends no hello must not keep shutdown blocked.
	idle, err := winio.DialPipeAccessImpLevel(ctx, discovery.QueryPipe, windows.GENERIC_READ|windows.GENERIC_WRITE, winio.PipeImpLevelIdentification)
	if err != nil {
		t.Fatal(err)
	}
	defer idle.Close()
	response, err = CallLocal(ctx, runtimeDir, AuthorityAdmin, Request{ID: "stop", Method: "engine.shutdown"})
	if err != nil || response.Error != nil {
		t.Fatalf("shutdown=%+v err=%v", response, err)
	}
}

func TestWindowsNamedPipeCancelledCall(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	_, err := winio.DialPipeAccessImpLevel(ctx, `\\.\pipe\filemanager-engine-nonexistent-cancellation-fixture`, windows.GENERIC_READ|windows.GENERIC_WRITE, winio.PipeImpLevelIdentification)
	if err == nil {
		t.Fatal("cancelled dial succeeded")
	}
}
