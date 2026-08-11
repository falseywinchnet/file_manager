// fileman-engine is the standalone development entry point. It intentionally
// refuses to start without a sandbox root.
package main

import (
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"html"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/deployment"
	"filemanager/engine/internal/observation/fsevents"
	"filemanager/engine/internal/observation/rdcw"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
	"filemanager/engine/internal/transport"
)

func main() {
	if err := run(os.Args[1:], os.Stdin, os.Stdout); err != nil {
		fmt.Fprintln(os.Stderr, "fileman-engine:", err)
		os.Exit(1)
	}
}

func run(args []string, input io.Reader, output io.Writer) error {
	if len(args) != 0 {
		switch args[0] {
		case "serve-launchd":
			return runLaunchd(args[1:])
		case "create-macos-manifest":
			return createManifest(args[1:], output)
		case "call-local":
			return callLocal(args[1:], output)
		case "write-launchd-plist":
			return writeLaunchdPlist(args[1:], output)
		}
	}
	flags := flag.NewFlagSet("fileman-engine", flag.ContinueOnError)
	flags.SetOutput(io.Discard)
	sandboxRoot := flags.String("sandbox-root", "", "absolute disposable root visible to the development service")
	storeRoot := flags.String("store-root", "", "existing engine-owned directory for the M2 one-root durable service")
	rootID := flags.String("root-id", "", "approved startup root id (requires --root-path)")
	rootPath := flags.String("root-path", "", "approved startup root inside the disposable sandbox (requires --root-id)")
	backgroundAdapter := flags.String("background-observation", "", "experimental native adapter: fsevents (macOS) or rdcw (Windows)")
	if err := flags.Parse(args); err != nil {
		return err
	}
	guard, err := sandbox.New(*sandboxRoot)
	if err != nil {
		return fmt.Errorf("refuses to start: %w", err)
	}
	return serveConfigured(input, output, guard, serveOptions{
		storeRoot: *storeRoot, rootID: api.RootID(*rootID), rootPath: *rootPath,
		backgroundAdapter: *backgroundAdapter,
	})
}

func writeLaunchdPlist(args []string, output io.Writer) error {
	flags := flag.NewFlagSet("write-launchd-plist", flag.ContinueOnError)
	label := flags.String("label", "com.filemanager.engine.m4-dogfood", "launchd label")
	binary := flags.String("binary", "", "installed Engine executable")
	manifest := flags.String("manifest", "", "host-bound deployment manifest")
	path := flags.String("output", "", "LaunchAgent plist output")
	stdoutPath := flags.String("stdout", "", "service stdout log")
	stderrPath := flags.String("stderr", "", "service stderr log")
	if err := flags.Parse(args); err != nil {
		return err
	}
	for name, value := range map[string]string{"binary": *binary, "manifest": *manifest, "output": *path, "stdout": *stdoutPath, "stderr": *stderrPath} {
		if value == "" || !filepath.IsAbs(value) {
			return fmt.Errorf("%s must be an absolute path", name)
		}
	}
	xml := fmt.Sprintf(`<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key><string>%s</string>
  <key>ProgramArguments</key>
  <array><string>%s</string><string>serve-launchd</string><string>--manifest</string><string>%s</string></array>
  <key>RunAtLoad</key><true/>
  <key>KeepAlive</key><true/>
  <key>ProcessType</key><string>Background</string>
  <key>ThrottleInterval</key><integer>1</integer>
  <key>StandardOutPath</key><string>%s</string>
  <key>StandardErrorPath</key><string>%s</string>
  <key>Umask</key><integer>63</integer>
</dict>
</plist>
`, html.EscapeString(*label), html.EscapeString(*binary), html.EscapeString(*manifest), html.EscapeString(*stdoutPath), html.EscapeString(*stderrPath))
	if err := os.WriteFile(*path, []byte(xml), 0o644); err != nil {
		return err
	}
	if err := os.Chmod(*path, 0o644); err != nil {
		return err
	}
	_, err := fmt.Fprintln(output, *path)
	return err
}

func runLaunchd(args []string) error {
	flags := flag.NewFlagSet("serve-launchd", flag.ContinueOnError)
	manifestPath := flags.String("manifest", "", "host-bound deployment manifest")
	if err := flags.Parse(args); err != nil {
		return err
	}
	manifest, err := deployment.LoadSecure(*manifestPath)
	if err != nil {
		return err
	}
	guard, err := manifest.Guard()
	if err != nil {
		return err
	}
	engine, err := service.NewLaunchdPersistent(guard, manifest.StoreRoot)
	if err != nil {
		return err
	}
	defer engine.Close()
	if _, err := engine.ApplyRoots(context.Background(), manifest.RootSpecs()); err != nil {
		return fmt.Errorf("apply manifest root policy: %w", err)
	}
	reconcileContext, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
	defer cancel()
	if err := ensureManifestAdmission(reconcileContext, engine, manifest); err != nil {
		return err
	}
	return transport.ServeLocalWithOptions(
		context.Background(), manifest.RuntimeDir, engine,
		transport.LocalOptions{AfterSuccessfulDispatch: manifestAdmissionCommitter(manifest)},
	)
}

func ensureManifestAdmission(ctx context.Context, engine *service.Service, manifest deployment.Manifest) error {
	status, err := engine.Status(ctx)
	if err != nil {
		return fmt.Errorf("inspect recovered generation: %w", err)
	}
	admissionDigest := manifest.AdmissionDigest()
	admissionCurrent := false
	if status.Generation != 0 {
		admissionCurrent, err = deployment.AdmissionMatches(manifest.StoreRoot, admissionDigest, status.Generation)
		if err != nil {
			return err
		}
	}
	if !admissionCurrent {
		var generation api.Generation
		for _, root := range manifest.RootSpecs() {
			report, err := engine.Reconcile(ctx, root.ID)
			if err != nil {
				return fmt.Errorf("reconcile changed admission policy for root %q: %w", root.ID, err)
			}
			generation = report.Generation
		}
		if err := deployment.CommitAdmission(manifest.StoreRoot, admissionDigest, generation); err != nil {
			return fmt.Errorf("commit admission generation: %w", err)
		}
	}
	return nil
}

func manifestAdmissionCommitter(manifest deployment.Manifest) func(transport.Request, transport.Response) error {
	digest := manifest.AdmissionDigest()
	return func(request transport.Request, response transport.Response) error {
		switch request.Method {
		case "scan.reconcile", "engine.scan_reconcile", "projection.rebuild", "engine.projection_rebuild":
			report, ok := response.Result.(api.ReconcileReport)
			if !ok || report.Generation == 0 {
				return errors.New("successful reconciliation returned no generation")
			}
			if err := deployment.CommitAdmission(manifest.StoreRoot, digest, report.Generation); err != nil {
				return fmt.Errorf("commit admission generation: %w", err)
			}
		}
		return nil
	}
}

func createManifest(args []string, output io.Writer) error {
	flags := flag.NewFlagSet("create-macos-manifest", flag.ContinueOnError)
	deploymentID := flags.String("deployment-id", "m4-dogfood", "deployment identity")
	rootID := flags.String("root-id", "m4-source", "approved root id")
	rootPath := flags.String("root-path", "", "approved root path")
	storeRoot := flags.String("store-root", "", "private durable store directory")
	runtimeDir := flags.String("runtime-dir", "", "private endpoint directory")
	manifestPath := flags.String("output", "", "manifest output path")
	exclusions := flags.String("exclude", ".git,engine/build,engine/tmp,engine/results/raw,gui_forms/build,gui_forms/.build", "comma-separated relative exclusions")
	if err := flags.Parse(args); err != nil {
		return err
	}
	if *rootPath == "" || *storeRoot == "" || *runtimeDir == "" || *manifestPath == "" {
		return errors.New("root-path, store-root, runtime-dir, and output are required")
	}
	host, err := deployment.CurrentHost()
	if err != nil {
		return err
	}
	objectID, err := deployment.RootObjectID(*rootPath)
	if err != nil {
		return err
	}
	manifest := deployment.Manifest{
		Schema: deployment.ManifestSchema, SchemaMajor: deployment.ManifestMajor,
		DeploymentID: *deploymentID, HostUUID: host.UUID, UID: host.UID,
		StoreRoot: *storeRoot, RuntimeDir: *runtimeDir,
		Roots: []deployment.Root{{ID: api.RootID(*rootID), Path: *rootPath, ObjectID: objectID, Exclusions: splitNonempty(*exclusions)}},
	}
	if err := deployment.Validate(&manifest, host); err != nil {
		return err
	}
	payload, err := json.MarshalIndent(manifest, "", "  ")
	if err != nil {
		return err
	}
	payload = append(payload, '\n')
	if err := writeManifestAtomically(*manifestPath, payload); err != nil {
		return err
	}
	_, err = fmt.Fprintf(output, "%s\n", *manifestPath)
	return err
}

func writeManifestAtomically(path string, payload []byte) error {
	directory := filepath.Dir(path)
	temporary, err := os.CreateTemp(directory, ".deployment-*.new")
	if err != nil {
		return err
	}
	temporaryPath := temporary.Name()
	defer os.Remove(temporaryPath)
	if err := temporary.Chmod(0o600); err != nil {
		temporary.Close()
		return err
	}
	if _, err := temporary.Write(payload); err != nil {
		temporary.Close()
		return err
	}
	if err := temporary.Sync(); err != nil {
		temporary.Close()
		return err
	}
	if err := temporary.Close(); err != nil {
		return err
	}
	if err := os.Rename(temporaryPath, path); err != nil {
		return err
	}
	directoryHandle, err := os.Open(directory)
	if err != nil {
		return err
	}
	syncErr := directoryHandle.Sync()
	closeErr := directoryHandle.Close()
	if syncErr != nil {
		return syncErr
	}
	return closeErr
}

func callLocal(args []string, output io.Writer) error {
	flags := flag.NewFlagSet("call-local", flag.ContinueOnError)
	runtimeDir := flags.String("runtime-dir", "", "private endpoint directory")
	authorityValue := flags.String("authority", "query", "query or admin")
	requestJSON := flags.String("request", "", "single JSON request")
	timeout := flags.Duration("timeout", 35*time.Second, "request timeout")
	if err := flags.Parse(args); err != nil {
		return err
	}
	var request transport.Request
	decoder := json.NewDecoder(strings.NewReader(*requestJSON))
	if err := decoder.Decode(&request); err != nil {
		return fmt.Errorf("decode request: %w", err)
	}
	var trailing any
	if err := decoder.Decode(&trailing); err != io.EOF {
		if err == nil {
			err = errors.New("multiple JSON values")
		}
		return fmt.Errorf("request contains trailing data: %w", err)
	}
	ctx, cancel := context.WithTimeout(context.Background(), *timeout)
	defer cancel()
	response, err := transport.CallLocal(ctx, *runtimeDir, transport.Authority(*authorityValue), request)
	if err != nil {
		return err
	}
	return json.NewEncoder(output).Encode(response)
}

func splitNonempty(value string) []string {
	var result []string
	for _, item := range strings.Split(value, ",") {
		if item = strings.TrimSpace(item); item != "" {
			result = append(result, item)
		}
	}
	return result
}

func serve(input io.Reader, output io.Writer, guard *sandbox.Guard) error {
	return serveWithStore(input, output, guard, "")
}

func serveWithStore(input io.Reader, output io.Writer, guard *sandbox.Guard, storeRoot string) error {
	return serveConfigured(input, output, guard, serveOptions{storeRoot: storeRoot})
}

type serveOptions struct {
	storeRoot         string
	rootID            api.RootID
	rootPath          string
	backgroundAdapter string
}

func serveConfigured(input io.Reader, output io.Writer, guard *sandbox.Guard, options serveOptions) error {
	var engine *service.Service
	var err error
	if options.storeRoot == "" {
		engine, err = service.New(guard)
	} else {
		engine, err = service.NewPersistent(guard, options.storeRoot)
	}
	if err != nil {
		return err
	}
	defer engine.Close()
	if (options.rootID == "") != (options.rootPath == "") {
		return errors.New("--root-id and --root-path must be supplied together")
	}
	if options.rootID != "" {
		if _, err := engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: options.rootID, Path: options.rootPath}}); err != nil {
			return fmt.Errorf("apply startup root: %w", err)
		}
	}
	switch options.backgroundAdapter {
	case "":
	case "fsevents":
		if options.rootID == "" {
			return errors.New("--background-observation=fsevents requires an explicit startup root")
		}
		adapter, err := fsevents.New(engine.Configuration().RootPolicy, fsevents.DefaultConfig())
		if err != nil {
			return fmt.Errorf("configure FSEvents observation: %w", err)
		}
		if err := engine.StartBackgroundObservation(context.Background(), adapter, service.DefaultBackgroundPolicy()); err != nil {
			return fmt.Errorf("start FSEvents observation: %w", err)
		}
	case "rdcw":
		if options.rootID == "" {
			return errors.New("--background-observation=rdcw requires an explicit startup root")
		}
		adapter, err := rdcw.New(engine.Configuration().RootPolicy, rdcw.DefaultConfig())
		if err != nil {
			return fmt.Errorf("configure ReadDirectoryChangesW observation: %w", err)
		}
		if err := engine.StartBackgroundObservation(context.Background(), adapter, service.DefaultBackgroundPolicy()); err != nil {
			return fmt.Errorf("start ReadDirectoryChangesW observation: %w", err)
		}
	default:
		return fmt.Errorf("unknown background observation adapter %q", options.backgroundAdapter)
	}
	return transport.Serve(context.Background(), input, output, engine)
}
