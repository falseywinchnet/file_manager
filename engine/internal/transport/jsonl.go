// Package transport projects the engine object model onto newline-delimited
// JSON. It contains no catalogue or ranking logic.
package transport

import (
	"bufio"
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"

	"filemanager/engine/api"
	"filemanager/engine/internal/service"
)

type Request struct {
	ID     string          `json:"id"`
	Method string          `json:"method"`
	Params json.RawMessage `json:"params,omitempty"`
}

type Response struct {
	ID     string `json:"id"`
	Result any    `json:"result,omitempty"`
	Error  *Fault `json:"error,omitempty"`
}

type Fault struct {
	Code    api.ErrorCode `json:"code"`
	Message string        `json:"message"`
}

type legacyVersion struct {
	Protocol string   `json:"protocol"`
	Features []string `json:"features"`
}

type rootParams struct {
	Roots                       []api.RootSpec `json:"roots"`
	ExpectedConfigurationDigest string         `json:"expected_configuration_digest,omitempty"`
}

type reconcileParams struct {
	Root api.RootID `json:"root"`
}

func Serve(ctx context.Context, input io.Reader, output io.Writer, engine *service.Service) error {
	if engine == nil {
		return errors.New("engine service is required")
	}
	decoder := json.NewDecoder(bufio.NewReader(input))
	encoder := json.NewEncoder(output)
	for {
		var incoming Request
		if err := decoder.Decode(&incoming); err != nil {
			if err == io.EOF {
				return nil
			}
			return fmt.Errorf("decode request: %w", err)
		}
		outgoing, shutdown := dispatch(ctx, engine, incoming)
		if err := encoder.Encode(outgoing); err != nil {
			return fmt.Errorf("encode response: %w", err)
		}
		if shutdown {
			return nil
		}
	}
}

func dispatch(ctx context.Context, engine *service.Service, incoming Request) (Response, bool) {
	response := Response{ID: incoming.ID}
	if incoming.ID == "" || incoming.Method == "" {
		response.Error = &Fault{Code: api.ErrorInvalidRequest, Message: "request id and method are required"}
		return response, false
	}
	var result any
	var err error
	shutdown := false
	switch incoming.Method {
	case "version":
		features := []string{"exact-reference", "integrity", "root-policy", "scan-reconcile"}
		if engine.Persistent() {
			features = append(features, "immutable-generation-v1")
		}
		result = legacyVersion{Protocol: api.ProtocolVersion, Features: features}
	case "engine.version":
		result = engine.Version()
	case "status", "engine.status":
		result, err = engine.Status(ctx)
	case "configuration.get", "engine.configuration_get":
		result = engine.Configuration()
	case "sandbox.root":
		result = struct {
			Root string `json:"root"`
		}{Root: engine.SandboxRoot()}
	case "root.plan", "engine.root_plan":
		var params rootParams
		if err = decodeParams(incoming.Params, &params); err == nil {
			result, err = engine.PlanRoots(ctx, params.Roots)
		}
	case "root.apply":
		var params rootParams
		if err = decodeParams(incoming.Params, &params); err == nil {
			result, err = engine.ApplyRoots(ctx, params.Roots)
		}
	case "engine.root_apply":
		var params rootParams
		if err = decodeParams(incoming.Params, &params); err == nil {
			result, err = engine.ApplyRootsExpected(ctx, params.Roots, params.ExpectedConfigurationDigest)
		}
	case "scan.reconcile", "engine.scan_reconcile":
		var params reconcileParams
		if err = decodeParams(incoming.Params, &params); err == nil {
			result, err = engine.Reconcile(ctx, params.Root)
		}
	case "projection.rebuild", "engine.projection_rebuild":
		var params reconcileParams
		if err = decodeParams(incoming.Params, &params); err == nil {
			result, err = engine.Rebuild(ctx, params.Root)
		}
	case "query", "engine.query":
		var query api.Query
		if err = decodeParams(incoming.Params, &query); err == nil {
			result, err = engine.Query(ctx, query)
		}
	case "inspect", "engine.inspect":
		var ref api.ObjectRef
		if err = decodeParams(incoming.Params, &ref); err == nil {
			result, err = engine.Inspect(ctx, ref)
		}
	case "integrity.check", "engine.integrity_check":
		result, err = engine.Integrity(ctx)
	case "shutdown":
		_, err = engine.Shutdown(ctx)
		result = struct {
			Accepted bool `json:"accepted"`
		}{Accepted: err == nil}
		shutdown = err == nil
	case "engine.shutdown":
		result, err = engine.Shutdown(ctx)
		shutdown = err == nil
	default:
		err = api.NewFault(api.ErrorMethodUnavailable, "method is not implemented by this engine version")
	}
	if err != nil {
		response.Error = publicFault(err)
		return response, false
	}
	response.Result = result
	return response, shutdown
}

func decodeParams(raw json.RawMessage, target any) error {
	if len(raw) == 0 {
		return api.NewFault(api.ErrorInvalidRequest, "method params are required")
	}
	decoder := json.NewDecoder(bytes.NewReader(raw))
	if err := decoder.Decode(target); err != nil {
		return api.WrapFault(api.ErrorInvalidRequest, "method params are invalid", err)
	}
	var trailing any
	if err := decoder.Decode(&trailing); err != io.EOF {
		if err == nil {
			err = errors.New("multiple JSON values")
		}
		return api.WrapFault(api.ErrorInvalidRequest, "method params contain trailing data", err)
	}
	return nil
}

func publicFault(err error) *Fault {
	var typed *api.Fault
	if errors.As(err, &typed) {
		return &Fault{Code: typed.Code, Message: typed.Message}
	}
	if errors.Is(err, context.Canceled) || errors.Is(err, context.DeadlineExceeded) {
		return &Fault{Code: api.ErrorResourceBudget, Message: "request context ended before completion"}
	}
	return &Fault{Code: api.ErrorInternal, Message: "engine request failed"}
}
