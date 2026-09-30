//go:build windows

package transport

import (
	"context"
	"encoding/json"
	"errors"
	"net"
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
	var parent string = t.TempDir()
	var source string = filepath.Join(parent, "source")
	var runtimeDir string = filepath.Join(parent, "runtime")
	{
		var err error = nil
		err = os.Mkdir(source, 0700)
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		err = os.WriteFile(filepath.Join(source, "needle.txt"), []byte("fixture"), 0600)
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		err = windowssecure.CreateDirectory(runtimeDir)
		if err != nil {
			t.Fatal(err)
		}
	}
	var object string = ""
	var err error = nil
	object, err = deployment.RootObjectID(source)
	if err != nil {
		t.Fatal(err)
	}
	var guard *sandbox.Guard = nil
	guard, err = sandbox.NewApproved("windows-test", []sandbox.ApprovedRoot{{ID: "docs", Path: source, ObjectID: object}})
	if err != nil {
		t.Fatal(err)
	}
	var engine *service.Service = nil
	engine, err = service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	{
		var err error = nil
		_, err = engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: "docs", Path: source}})
		if err != nil {
			t.Fatal(err)
		}
	}
	var ctx context.Context = nil
	var cancel context.CancelFunc = nil
	ctx, cancel = context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()
	var server pipeTestServer = pipeTestServer{context: ctx, cancel: cancel, runtimeDir: runtimeDir, engine: engine, done: make(chan error, 1)}
	go server.Run()
	defer server.Stop(t)
	var startupErr error = nil
	var discovery WindowsDiscovery = WindowsDiscovery{}
	for {
		discovery, err = loadWindowsDiscovery(runtimeDir)
		if err == nil {
			break
		}
		select {
		case startupErr = <-server.done:
			server.done <- startupErr
			t.Fatalf("server startup: %v", startupErr)
		case <-ctx.Done():
			t.Fatal(ctx.Err())
		case <-time.After(10 * time.Millisecond):
		}
	}
	// Discovery may not redirect credentials to a different server process.
	var changed WindowsDiscovery = discovery
	changed.ServerPID++
	var payload []byte = nil
	payload, _ = json.Marshal(changed)
	{
		var err error = nil
		err = windowssecure.Write(Files(runtimeDir).Discovery, payload)
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		_, err = CallLocal(ctx, runtimeDir, AuthorityQuery, Request{ID: "wrong-pid", Method: "engine.version"})
		if err == nil {
			t.Fatal("wrong server PID accepted")
		}
	}
	payload, _ = json.Marshal(discovery)
	{
		var err error = nil
		err = windowssecure.Write(Files(runtimeDir).Discovery, payload)
		if err != nil {
			t.Fatal(err)
		}
	}
	// Query credentials cannot authenticate to the administrative endpoint.
	var token []byte = nil
	token, err = windowssecure.Read(discovery.QueryTokenFile, 64)
	if err != nil {
		t.Fatal(err)
	}
	var connection net.Conn = nil
	connection, err = winio.DialPipeAccessImpLevel(ctx, discovery.AdminPipe, windows.GENERIC_READ|windows.GENERIC_WRITE, winio.PipeImpLevelIdentification)
	if err != nil {
		t.Fatal(err)
	}
	connection.SetDeadline(time.Now().Add(2 * time.Second))
	{
		var err error = nil
		err = writeFrame(connection, hello{Protocol: LocalProtocol, Authority: AuthorityAdmin, Token: string(token)})
		if err != nil {
			t.Fatal(err)
		}
	}
	var reply helloResponse = helloResponse{}
	err = readFrame(connection, &reply)
	connection.Close()
	if err != nil || reply.Accepted {
		t.Fatalf("cross-authority hello=%+v err=%v", reply, err)
	}
	var response Response = Response{}
	response, err = CallLocal(ctx, runtimeDir, AuthorityQuery, Request{ID: "denied", Method: "engine.shutdown"})
	if err != nil || response.Error == nil {
		t.Fatalf("query shutdown=%+v err=%v", response, err)
	}
	var params []byte = nil
	params, _ = json.Marshal(api.LiveQuery{QueryID: "live", Scope: api.LiveQueryScope{RootID: "docs", Descendants: true}, Text: "needle"})
	response, err = CallLocal(ctx, runtimeDir, AuthorityQuery, Request{ID: "live", Method: "engine.query_live", Params: params})
	if err != nil || response.Error != nil {
		t.Fatalf("live=%+v err=%v", response, err)
	}
	var encoded []byte = nil
	encoded, _ = json.Marshal(response.Result)
	if !strings.Contains(string(encoded), "needle.txt") || !strings.Contains(string(encoded), "live_filesystem") {
		t.Fatalf("live result=%s", encoded)
	}
	var status api.Status = api.Status{}
	status, err = engine.Status(ctx)
	if err != nil || status.RootStates[0].Indexed {
		t.Fatalf("live search created catalogue: %+v err=%v", status, err)
	}
	// A connected client that sends no hello must not keep shutdown blocked.
	var idle net.Conn = nil
	idle, err = winio.DialPipeAccessImpLevel(ctx, discovery.QueryPipe, windows.GENERIC_READ|windows.GENERIC_WRITE, winio.PipeImpLevelIdentification)
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
	var ctx context.Context = nil
	var cancel context.CancelFunc = nil
	ctx, cancel = context.WithCancel(context.Background())
	cancel()
	var err error = nil
	_, err = winio.DialPipeAccessImpLevel(ctx, `\\.\pipe\filemanager-engine-nonexistent-cancellation-fixture`, windows.GENERIC_READ|windows.GENERIC_WRITE, winio.PipeImpLevelIdentification)
	if err == nil {
		t.Fatal("cancelled dial succeeded")
	}
}

// The fixture borrows engine until Stop has joined the server and its sessions.
type pipeTestServer struct {
	context    context.Context
	cancel     context.CancelFunc
	runtimeDir string
	engine     *service.Service
	done       chan error
}

func (server *pipeTestServer) Run() {
	var err error = ServeLocal(server.context, server.runtimeDir, server.engine)
	server.done <- err
}
func (server *pipeTestServer) Stop(t *testing.T) {
	t.Helper()
	server.cancel()
	var err error = nil
	select {
	case err = <-server.done:
		if err != nil && !errors.Is(err, context.Canceled) {
			t.Error(err)
		}
	case <-time.After(5 * time.Second):
		t.Error("server failed to stop")
	}
}

func TestConnectionCancellationRevokesBeforeStart(t *testing.T) {
	var left net.Conn = nil
	var right net.Conn = nil
	left, right = net.Pipe()
	defer left.Close()
	defer right.Close()
	var ctx context.Context = nil
	var cancel context.CancelFunc = nil
	ctx, cancel = context.WithCancel(context.Background())
	defer cancel()
	var callback connectionCancellation = connectionCancellation{connection: left, finished: make(chan struct{}), stop: nil}
	callback.stop = context.AfterFunc(ctx, callback.CloseConnection)
	callback.Revoke()
	cancel()
	if callback.stop != nil {
		t.Fatal("callback registration was retained")
	}
	var deadline time.Time = time.Now().Add(time.Second)
	var err error = left.SetDeadline(deadline)
	if err != nil {
		t.Fatalf("revoked callback closed connection: %v", err)
	}
}

// Only Close is exercised: the embedded interface is deliberately unavailable.
// The test controls release so it can observe whether Revoke joins Close.
type blockedCloseConnection struct {
	net.Conn
	started chan struct{}
	release chan struct{}
}

func (connection *blockedCloseConnection) Close() error {
	close(connection.started)
	<-connection.release
	return nil
}

type cancellationRevoker struct {
	callback *connectionCancellation
	finished chan struct{}
}

func (revoker *cancellationRevoker) Run() {
	revoker.callback.Revoke()
	close(revoker.finished)
}
func TestConnectionCancellationJoinsRunningClose(t *testing.T) {
	var connection blockedCloseConnection = blockedCloseConnection{Conn: nil, started: make(chan struct{}), release: make(chan struct{})}
	var ctx context.Context = nil
	var cancel context.CancelFunc = nil
	ctx, cancel = context.WithCancel(context.Background())
	defer cancel()
	var callback connectionCancellation = connectionCancellation{connection: &connection, finished: make(chan struct{}), stop: nil}
	callback.stop = context.AfterFunc(ctx, callback.CloseConnection)
	cancel()
	select {
	case <-connection.started:
	case <-time.After(5 * time.Second):
		t.Fatal("callback did not start")
	}
	var revoker cancellationRevoker = cancellationRevoker{callback: &callback, finished: make(chan struct{})}
	go revoker.Run()
	select {
	case <-revoker.finished:
		t.Error("Revoke returned before Close completed")
	case <-time.After(20 * time.Millisecond):
	}
	close(connection.release)
	select {
	case <-revoker.finished:
	case <-time.After(5 * time.Second):
		t.Fatal("Revoke did not finish after Close")
	}
	if callback.stop != nil {
		t.Fatal("callback registration was retained")
	}
}
