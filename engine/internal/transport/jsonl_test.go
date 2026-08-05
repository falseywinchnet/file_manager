package transport

import (
	"bytes"
	"context"
	"encoding/json"
	"os"
	"path/filepath"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
)

type decodedResponse struct {
	ID     string          `json:"id"`
	Result json.RawMessage `json:"result"`
	Error  *Fault          `json:"error"`
}

func TestExactVerticalSliceOverJSONL(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "needle.txt"), []byte("fixture"), 0o600); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	requests := []Request{
		{ID: "apply", Method: "root.apply", Params: mustJSON(t, rootParams{Roots: []api.RootSpec{{ID: "docs", Path: source}}})},
		{ID: "scan", Method: "scan.reconcile", Params: mustJSON(t, reconcileParams{Root: "docs"})},
		{ID: "rebuild", Method: "projection.rebuild", Params: mustJSON(t, reconcileParams{Root: "docs"})},
		{ID: "query", Method: "query", Params: mustJSON(t, api.Query{Scope: api.Scope{Root: "docs", Descendants: true}, Filters: map[string]string{"name": "needle.txt"}})},
		{ID: "integrity", Method: "integrity.check"},
		{ID: "shutdown", Method: "shutdown"},
	}
	var input bytes.Buffer
	encoder := json.NewEncoder(&input)
	for _, request := range requests {
		if err := encoder.Encode(request); err != nil {
			t.Fatal(err)
		}
	}
	var output bytes.Buffer
	if err := Serve(context.Background(), &input, &output, engine); err != nil {
		t.Fatal(err)
	}
	decoder := json.NewDecoder(&output)
	for index, request := range requests {
		var response decodedResponse
		if err := decoder.Decode(&response); err != nil {
			t.Fatalf("decode response %d: %v", index, err)
		}
		if response.ID != request.ID || response.Error != nil {
			t.Fatalf("response %d = %#v", index, response)
		}
		if response.ID == "query" {
			var query api.QueryResponse
			if err := json.Unmarshal(response.Result, &query); err != nil {
				t.Fatal(err)
			}
			if len(query.Results) != 1 || query.Results[0].Metadata.Name != "needle.txt" {
				t.Fatalf("query response = %#v", query)
			}
		}
		if response.ID == "integrity" {
			var report api.IntegrityReport
			if err := json.Unmarshal(response.Result, &report); err != nil {
				t.Fatal(err)
			}
			if !report.Healthy {
				t.Fatalf("integrity response = %#v", report)
			}
		}
	}
}

func TestInvalidParamsAreNamedProtocolErrors(t *testing.T) {
	guard, err := sandbox.New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	input := bytes.NewBufferString("{\"id\":\"bad\",\"method\":\"query\",\"params\":[]}\n{\"id\":\"stop\",\"method\":\"shutdown\"}\n")
	var output bytes.Buffer
	if err := Serve(context.Background(), input, &output, engine); err != nil {
		t.Fatalf("framed param error terminated transport: %v", err)
	}
	var response decodedResponse
	if err := json.NewDecoder(&output).Decode(&response); err != nil {
		t.Fatal(err)
	}
	if response.Error == nil || response.Error.Code != api.ErrorInvalidRequest {
		t.Fatalf("invalid params response = %#v", response)
	}
}

func mustJSON(t *testing.T, value any) json.RawMessage {
	t.Helper()
	encoded, err := json.Marshal(value)
	if err != nil {
		t.Fatal(err)
	}
	return encoded
}
