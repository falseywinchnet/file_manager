//go:build darwin && cgo

package transport

import (
	"bytes"
	"context"
	"encoding/binary"
	"net"
	"os"
	"path/filepath"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
)

func TestBoundedFrameRejectsMagicAndOversize(t *testing.T) {
	var encoded bytes.Buffer
	if err := writeFrame(&encoded, Request{ID: "one", Method: "engine.status"}); err != nil {
		t.Fatal(err)
	}
	var decoded Request
	if err := readFrame(&encoded, &decoded); err != nil {
		t.Fatal(err)
	}
	if decoded.ID != "one" {
		t.Fatalf("decoded request = %+v", decoded)
	}
	bad := make([]byte, frameHeader)
	copy(bad, "NOPE")
	binary.BigEndian.PutUint32(bad[4:], 2)
	if err := readFrame(bytes.NewReader(append(bad, []byte("{}")...)), &decoded); err == nil {
		t.Fatal("bad frame magic was accepted")
	}
	copy(bad, frameMagic)
	binary.BigEndian.PutUint32(bad[4:], MaxJSONLFrameBytes+1)
	if err := readFrame(bytes.NewReader(bad), &decoded); err == nil {
		t.Fatal("oversized frame was accepted")
	}
}

type oneByteWriter struct{ bytes.Buffer }

func (w *oneByteWriter) Write(payload []byte) (int, error) {
	if len(payload) > 1 {
		payload = payload[:1]
	}
	return w.Buffer.Write(payload)
}

func TestWriteFrameCompletesShortWrites(t *testing.T) {
	var encoded oneByteWriter
	if err := writeFrame(&encoded, Request{ID: "short", Method: "engine.status"}); err != nil {
		t.Fatal(err)
	}
	var decoded Request
	if err := readFrame(&encoded.Buffer, &decoded); err != nil {
		t.Fatal(err)
	}
	if decoded.ID != "short" {
		t.Fatalf("decoded request = %+v", decoded)
	}
}

func TestLocalEndpointsSeparateQueryAndAdministration(t *testing.T) {
	root, err := os.MkdirTemp("/tmp", "eng-local-")
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = os.RemoveAll(root) })
	runtimeDir := filepath.Join(root, "runtime")
	if err := os.Mkdir(runtimeDir, 0o700); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(root)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	serverDone := make(chan error, 1)
	go func() { serverDone <- ServeLocal(context.Background(), runtimeDir, engine) }()
	files := Files(runtimeDir)
	deadline := time.Now().Add(3 * time.Second)
	for {
		if _, err := os.Stat(files.Discovery); err == nil {
			break
		}
		if time.Now().After(deadline) {
			t.Fatal("local endpoint did not publish discovery")
		}
		time.Sleep(10 * time.Millisecond)
	}
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()
	idle, err := net.Dial("unix", files.QuerySock)
	if err != nil {
		t.Fatal(err)
	}
	defer idle.Close()
	status, err := CallLocal(ctx, runtimeDir, AuthorityQuery, Request{ID: "status", Method: "engine.status"})
	if err != nil || status.Error != nil {
		t.Fatalf("query status response=%+v err=%v", status, err)
	}
	denied, err := CallLocal(ctx, runtimeDir, AuthorityQuery, Request{ID: "integrity", Method: "engine.integrity_check"})
	if err != nil {
		t.Fatal(err)
	}
	if denied.Error == nil || denied.Error.Code != api.ErrorMethodUnavailable {
		t.Fatalf("query endpoint admitted admin method: %+v", denied)
	}
	stopped, err := CallLocal(ctx, runtimeDir, AuthorityAdmin, Request{ID: "stop", Method: "engine.shutdown"})
	if err != nil || stopped.Error != nil {
		t.Fatalf("admin shutdown response=%+v err=%v", stopped, err)
	}
	select {
	case err := <-serverDone:
		if err != nil {
			t.Fatal(err)
		}
	case <-time.After(3 * time.Second):
		t.Fatal("local server did not stop")
	}
}

func TestDuplicateServerCannotRotateLiveCredentials(t *testing.T) {
	root, err := os.MkdirTemp("/tmp", "eng-duplicate-")
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = os.RemoveAll(root) })
	runtimeDir := filepath.Join(root, "runtime")
	if err := os.Mkdir(runtimeDir, 0o700); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(root)
	if err != nil {
		t.Fatal(err)
	}
	first, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	done := make(chan error, 1)
	go func() { done <- ServeLocal(context.Background(), runtimeDir, first) }()
	files := Files(runtimeDir)
	waitForFile(t, files.Discovery)
	before, err := os.ReadFile(files.QueryToken)
	if err != nil {
		t.Fatal(err)
	}
	second, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	defer second.Close()
	if err := ServeLocal(context.Background(), runtimeDir, second); err == nil {
		t.Fatal("duplicate local server unexpectedly started")
	}
	after, err := os.ReadFile(files.QueryToken)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(before, after) {
		t.Fatal("failed duplicate server rotated the live credential")
	}
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()
	if response, err := CallLocal(ctx, runtimeDir, AuthorityAdmin, Request{ID: "stop", Method: "engine.shutdown"}); err != nil || response.Error != nil {
		t.Fatalf("shutdown response=%+v err=%v", response, err)
	}
	select {
	case err := <-done:
		if err != nil {
			t.Fatal(err)
		}
	case <-ctx.Done():
		t.Fatal("first server did not stop")
	}
}

func waitForFile(t *testing.T, path string) {
	t.Helper()
	deadline := time.Now().Add(3 * time.Second)
	for {
		if _, err := os.Stat(path); err == nil {
			return
		}
		if time.Now().After(deadline) {
			t.Fatalf("%s was not published", path)
		}
		time.Sleep(10 * time.Millisecond)
	}
}
