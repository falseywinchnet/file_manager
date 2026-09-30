package main

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"filemanager/engine/internal/sandbox"
)

func TestProtocolSmoke(t *testing.T) {
	guard, err := sandbox.New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	input := strings.NewReader("{\"id\":\"1\",\"method\":\"version\"}\n{\"id\":\"2\",\"method\":\"shutdown\"}\n")
	var output bytes.Buffer
	if err := serve(input, &output, guard); err != nil {
		t.Fatal(err)
	}

	want := "{\"id\":\"1\",\"result\":{\"protocol\":\"engine.v0\",\"features\":[\"exact-reference\",\"integrity\",\"root-policy\",\"scan-reconcile\",\"live-query\"]}}\n" +
		"{\"id\":\"2\",\"result\":{\"accepted\":true}}\n"
	if output.String() != want {
		t.Fatalf("protocol output:\n%s\nwant:\n%s", output.String(), want)
	}
}

func TestProtocolGolden(t *testing.T) {
	guard, err := sandbox.New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	requests, err := os.ReadFile(filepath.Join("..", "..", "testdata", "protocol", "v0", "smoke.requests.jsonl"))
	if err != nil {
		t.Fatal(err)
	}
	want, err := os.ReadFile(filepath.Join("..", "..", "testdata", "protocol", "v0", "smoke.responses.jsonl"))
	if err != nil {
		t.Fatal(err)
	}
	var output bytes.Buffer
	if err := serve(bytes.NewReader(requests), &output, guard); err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(output.Bytes(), want) {
		t.Fatalf("golden response:\n%s\nwant:\n%s", output.Bytes(), want)
	}
}

func TestCallLocalRejectsTrailingJSONBeforeEndpointAccess(t *testing.T) {
	err := callLocal([]string{"--runtime-dir", "/unreachable", "--request", `{"id":"one","method":"engine.status"} {}`}, &bytes.Buffer{})
	if err == nil || !strings.Contains(err.Error(), "trailing data") {
		t.Fatalf("trailing request error = %v", err)
	}
}
