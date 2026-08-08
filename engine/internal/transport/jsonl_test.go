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

func TestLiveVerticalSliceOverJSONLWithoutReconcile(t *testing.T) {
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
		{ID: "live", Method: "engine.query_live", Params: mustJSON(t, api.LiveQuery{QueryID: "wire-live", Scope: api.LiveQueryScope{RootID: "docs", Descendants: true}, Text: "needle"})},
		{ID: "shutdown", Method: "shutdown"},
	}
	var input bytes.Buffer
	for _, request := range requests {
		if err := json.NewEncoder(&input).Encode(request); err != nil {
			t.Fatal(err)
		}
	}
	var output bytes.Buffer
	if err := Serve(context.Background(), &input, &output, engine); err != nil {
		t.Fatal(err)
	}
	decoder := json.NewDecoder(&output)
	for _, request := range requests {
		var response decodedResponse
		if err := decoder.Decode(&response); err != nil {
			t.Fatal(err)
		}
		if response.Error != nil {
			t.Fatalf("%s response error = %+v", request.ID, response.Error)
		}
		if request.ID == "live" {
			var page api.LiveQueryResponse
			if err := json.Unmarshal(response.Result, &page); err != nil {
				t.Fatal(err)
			}
			if page.Source != api.LiveFilesystemSource || !page.Complete || len(page.Results) != 1 || page.Results[0].Metadata.Name != "needle.txt" {
				t.Fatalf("live response = %+v", page)
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

func TestOversizedJSONLRequestIsRejectedAtFrameCeiling(t *testing.T) {
	guard, err := sandbox.New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	input := bytes.NewBuffer(bytes.Repeat([]byte{'x'}, MaxJSONLFrameBytes+1))
	var output bytes.Buffer
	if err := Serve(context.Background(), input, &output, engine); err == nil {
		t.Fatal("oversized JSONL request unexpectedly succeeded")
	}
	if output.Len() != 0 {
		t.Fatalf("oversized frame produced output bytes: %d", output.Len())
	}
}

func TestCanonicalLifecycleAndConfigurationProjection(t *testing.T) {
	guard, err := sandbox.New(t.TempDir())
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	requests := []Request{
		{ID: "version", Method: "engine.version"},
		{ID: "status", Method: "engine.status"},
		{ID: "configuration", Method: "engine.configuration_get"},
		{ID: "shutdown", Method: "engine.shutdown"},
	}
	var input bytes.Buffer
	for _, request := range requests {
		if err := json.NewEncoder(&input).Encode(request); err != nil {
			t.Fatal(err)
		}
	}
	var output bytes.Buffer
	if err := Serve(context.Background(), &input, &output, engine); err != nil {
		t.Fatal(err)
	}
	decoder := json.NewDecoder(&output)
	responses := make(map[string]json.RawMessage, len(requests))
	for range requests {
		var response decodedResponse
		if err := decoder.Decode(&response); err != nil {
			t.Fatal(err)
		}
		if response.Error != nil {
			t.Fatalf("canonical response %q error=%+v", response.ID, response.Error)
		}
		responses[response.ID] = response.Result
	}
	var version api.VersionInfo
	if err := json.Unmarshal(responses["version"], &version); err != nil {
		t.Fatal(err)
	}
	var status api.Status
	if err := json.Unmarshal(responses["status"], &status); err != nil {
		t.Fatal(err)
	}
	var configuration api.EffectiveConfiguration
	if err := json.Unmarshal(responses["configuration"], &configuration); err != nil {
		t.Fatal(err)
	}
	var stopped api.LifecycleStatus
	if err := json.Unmarshal(responses["shutdown"], &stopped); err != nil {
		t.Fatal(err)
	}
	if version.InstanceID == "" || version.InstanceID != status.Lifecycle.InstanceID || stopped.InstanceID != version.InstanceID {
		t.Fatalf("instance identity mismatch: version=%+v status=%+v stopped=%+v", version, status.Lifecycle, stopped)
	}
	if status.Lifecycle.State != api.LifecycleReady || stopped.State != api.LifecycleStopped {
		t.Fatalf("lifecycle status=%+v stopped=%+v", status.Lifecycle, stopped)
	}
	if configuration.Schema != api.EngineConfigurationSchema || configuration.Digest == "" || configuration.IngestionMode != "manual_reconcile" {
		t.Fatalf("effective configuration=%+v", configuration)
	}
}

func TestCanonicalRootApplyRequiresCurrentConfigurationDigest(t *testing.T) {
	sandboxPath := t.TempDir()
	firstPath := filepath.Join(sandboxPath, "first")
	secondPath := filepath.Join(sandboxPath, "second")
	for _, path := range []string{firstPath, secondPath} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	firstRoots := []api.RootSpec{{ID: "first", Path: firstPath}}
	secondRoots := []api.RootSpec{{ID: "second", Path: secondPath}}

	planResponse, stop := dispatch(context.Background(), engine, Request{
		ID: "plan", Method: "engine.root_plan", Params: mustJSON(t, rootParams{Roots: firstRoots}),
	})
	if stop || planResponse.Error != nil {
		t.Fatalf("plan response=%+v stop=%v", planResponse, stop)
	}
	plan, ok := planResponse.Result.(api.RootPlan)
	if !ok || plan.CurrentConfigurationDigest == "" {
		t.Fatalf("canonical plan=%+v", planResponse.Result)
	}

	missing, stop := dispatch(context.Background(), engine, Request{
		ID: "missing", Method: "engine.root_apply", Params: mustJSON(t, rootParams{Roots: firstRoots}),
	})
	if stop || missing.Error == nil || missing.Error.Code != api.ErrorInvalidRequest {
		t.Fatalf("missing digest response=%+v stop=%v", missing, stop)
	}
	applied, stop := dispatch(context.Background(), engine, Request{
		ID: "apply", Method: "engine.root_apply",
		Params: mustJSON(t, rootParams{Roots: firstRoots, ExpectedConfigurationDigest: plan.CurrentConfigurationDigest}),
	})
	if stop || applied.Error != nil {
		t.Fatalf("apply response=%+v stop=%v", applied, stop)
	}
	stale, stop := dispatch(context.Background(), engine, Request{
		ID: "stale", Method: "engine.root_apply",
		Params: mustJSON(t, rootParams{Roots: secondRoots, ExpectedConfigurationDigest: plan.CurrentConfigurationDigest}),
	})
	if stop || stale.Error == nil || stale.Error.Code != api.ErrorStaleConfiguration {
		t.Fatalf("stale response=%+v stop=%v", stale, stop)
	}
	configuration := engine.Configuration()
	if len(configuration.RootPolicy) != 1 || configuration.RootPolicy[0].ID != "first" {
		t.Fatalf("stale canonical apply changed state: %+v", configuration)
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
