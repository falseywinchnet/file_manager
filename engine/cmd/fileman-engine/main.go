// fileman-engine is the standalone development entry point. It intentionally
// refuses to start without a sandbox root.
package main

import (
	"context"
	"encoding/json"
	"errors"
	"filemanager/engine/internal/observation"
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
	{
		var err error = nil
		err = run(os.Args[1:], os.Stdin, os.Stdout)
		if err != nil {
			fmt.Fprintln(os.Stderr, "fileman-engine:", err)
			os.Exit(1)
		}
	}
}

func run(args []string, input io.Reader, output io.Writer) error {
	if len(args) != 0 {
		switch args[0] {
		case "create-windows-manifest", "serve-windows":
			var failure error = windowsCommand(args[0], args[1:], output)
			return failure
		case "serve-launchd":
			var failure error = runLaunchd(args[1:])
			return failure
		case "create-macos-manifest":
			var failure error = createManifest(args[1:], output)
			return failure
		case "call-local":
			var failure error = callLocal(args[1:], output)
			return failure
		case "write-launchd-plist":
			var failure error = writeLaunchdPlist(args[1:], output)
			return failure
		}
	}
	var flags *flag.FlagSet = flag.NewFlagSet("fileman-engine", flag.ContinueOnError)
	flags.SetOutput(io.Discard)
	var sandboxRoot *string = flags.String("sandbox-root", "", "absolute disposable root visible to the development service")
	var storeRoot *string = flags.String("store-root", "", "existing engine-owned directory for the M2 one-root durable service")
	var rootID *string = flags.String("root-id", "", "approved startup root id (requires --root-path)")
	var rootPath *string = flags.String("root-path", "", "approved startup root inside the disposable sandbox (requires --root-id)")
	var backgroundAdapter *string = flags.String("background-observation", "", "experimental native adapter: fsevents (macOS) or rdcw (Windows)")
	{
		var err error = nil
		err = flags.Parse(args)
		if err != nil {
			return err
		}
	}
	var guard *sandbox.Guard = nil
	var err error = nil
	guard, err = sandbox.New(*sandboxRoot)
	if err != nil {
		var failure error = fmt.Errorf("refuses to start: %w", err)
		return failure
	}
	var failure error = serveConfigured(input, output, guard, serveOptions{
		storeRoot: *storeRoot, rootID: api.RootID(*rootID), rootPath: *rootPath,
		backgroundAdapter: *backgroundAdapter,
	})
	return failure
}

func writeLaunchdPlist(args []string, output io.Writer) error {
	var flags *flag.FlagSet = flag.NewFlagSet("write-launchd-plist", flag.ContinueOnError)
	var label *string = flags.String("label", "com.filemanager.engine.m4-dogfood", "launchd label")
	var binary *string = flags.String("binary", "", "installed Engine executable")
	var manifest *string = flags.String("manifest", "", "host-bound deployment manifest")
	var path *string = flags.String("output", "", "LaunchAgent plist output")
	var stdoutPath *string = flags.String("stdout", "", "service stdout log")
	var stderrPath *string = flags.String("stderr", "", "service stderr log")
	{
		var err error = nil
		err = flags.Parse(args)
		if err != nil {
			return err
		}
	}
	{
		var name string = ""
		var value string = ""
		for name, value = range map[string]string{"binary": *binary, "manifest": *manifest, "output": *path, "stdout": *stdoutPath, "stderr": *stderrPath} {
			if value == "" || !filepath.IsAbs(value) {
				var failure error = fmt.Errorf("%s must be an absolute path", name)
				return failure
			}
		}
	}
	var xml string = fmt.Sprintf(`<?xml version="1.0" encoding="UTF-8"?>
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
	{
		var err error = nil
		err = os.WriteFile(*path, []byte(xml), 0o644)
		if err != nil {
			return err
		}
	}
	{
		var err error = nil
		err = os.Chmod(*path, 0o644)
		if err != nil {
			return err
		}
	}
	var err error = nil
	_, err = fmt.Fprintln(output, *path)
	return err
}

func runLaunchd(args []string) error {
	var flags *flag.FlagSet = flag.NewFlagSet("serve-launchd", flag.ContinueOnError)
	var manifestPath *string = flags.String("manifest", "", "host-bound deployment manifest")
	{
		var err error = nil
		err = flags.Parse(args)
		if err != nil {
			return err
		}
	}
	var manifest deployment.Manifest = deployment.Manifest{}
	var err error = nil
	manifest, err = deployment.LoadSecure(*manifestPath)
	if err != nil {
		return err
	}
	var guard *sandbox.Guard = nil
	guard, err = manifest.Guard()
	if err != nil {
		return err
	}
	var engine *service.Service = nil
	engine, err = service.NewLaunchdPersistent(guard, manifest.StoreRoot)
	if err != nil {
		return err
	}
	defer engine.Close()
	{
		var err error = nil
		_, err = engine.ApplyRoots(context.Background(), manifest.RootSpecs())
		if err != nil {
			var failure error = fmt.Errorf("apply manifest root policy: %w", err)
			return failure
		}
	}
	var reconcileContext context.Context = nil
	var cancel context.CancelFunc = nil
	reconcileContext, cancel = context.WithTimeout(context.Background(), 5*time.Minute)
	defer cancel()
	{
		var err error = nil
		err = ensureManifestAdmission(reconcileContext, engine, manifest)
		if err != nil {
			return err
		}
	}
	var failure error = transport.ServeLocalWithOptions(
		context.Background(), manifest.RuntimeDir, engine,
		transport.LocalOptions{AfterSuccessfulDispatch: manifestAdmissionCommitter(manifest)},
	)
	return failure
}

func ensureManifestAdmission(ctx context.Context, engine *service.Service, manifest deployment.Manifest) error {
	var status api.Status = api.Status{}
	var err error = nil
	status, err = engine.Status(ctx)
	if err != nil {
		var failure error = fmt.Errorf("inspect recovered generation: %w", err)
		return failure
	}
	var admissionDigest string = manifest.AdmissionDigest()
	var admissionCurrent bool = false
	if status.Generation != 0 {
		admissionCurrent, err = deployment.AdmissionMatches(manifest.StoreRoot, admissionDigest, status.Generation)
		if err != nil {
			return err
		}
	}
	if !admissionCurrent {
		var generation api.Generation = 0
		{
			var root api.RootSpec = api.RootSpec{}
			for _, root = range manifest.RootSpecs() {
				var report api.ReconcileReport = api.ReconcileReport{}
				var err error = nil
				report, err = engine.Reconcile(ctx, root.ID)
				if err != nil {
					var failure error = fmt.Errorf("reconcile changed admission policy for root %q: %w", root.ID, err)
					return failure
				}
				generation = report.Generation
			}
		}
		{
			var err error = nil
			err = deployment.CommitAdmission(manifest.StoreRoot, admissionDigest, generation)
			if err != nil {
				var failure error = fmt.Errorf("commit admission generation: %w", err)
				return failure
			}
		}
	}
	return nil
}

// manifestAdmissionWriter retains immutable policy metadata for the registered
// post-dispatch hook. The server joins all request handlers before releasing it.
type manifestAdmissionWriter struct {
	digest    string
	storeRoot string
}

func manifestAdmissionCommitter(manifest deployment.Manifest) func(transport.Request, transport.Response) error {
	var writer manifestAdmissionWriter = manifestAdmissionWriter{
		digest: manifest.AdmissionDigest(), storeRoot: manifest.StoreRoot,
	}
	var callback func(transport.Request, transport.Response) error = writer.Commit
	return callback
}

func (writer manifestAdmissionWriter) Commit(request transport.Request, response transport.Response) error {
	switch request.Method {
	case "scan.reconcile", "engine.scan_reconcile", "projection.rebuild", "engine.projection_rebuild":
		var report api.ReconcileReport = api.ReconcileReport{}
		var ok bool = false
		report, ok = response.Result.(api.ReconcileReport)
		if !ok || report.Generation == 0 {
			var failure error = errors.New("successful reconciliation returned no generation")
			return failure
		}
		{
			var err error = nil
			err = deployment.CommitAdmission(writer.storeRoot, writer.digest, report.Generation)
			if err != nil {
				var failure error = fmt.Errorf("commit admission generation: %w", err)
				return failure
			}
		}
	}
	return nil
}

func createManifest(args []string, output io.Writer) error {
	var flags *flag.FlagSet = flag.NewFlagSet("create-macos-manifest", flag.ContinueOnError)
	var deploymentID *string = flags.String("deployment-id", "m4-dogfood", "deployment identity")
	var rootID *string = flags.String("root-id", "m4-source", "approved root id")
	var rootPath *string = flags.String("root-path", "", "approved root path")
	var storeRoot *string = flags.String("store-root", "", "private durable store directory")
	var runtimeDir *string = flags.String("runtime-dir", "", "private endpoint directory")
	var manifestPath *string = flags.String("output", "", "manifest output path")
	var exclusions *string = flags.String("exclude", ".git,engine/build,engine/tmp,engine/results/raw,gui_forms/build,gui_forms/.build", "comma-separated relative exclusions")
	{
		var err error = nil
		err = flags.Parse(args)
		if err != nil {
			return err
		}
	}
	if *rootPath == "" || *storeRoot == "" || *runtimeDir == "" || *manifestPath == "" {
		var failure error = errors.New("root-path, store-root, runtime-dir, and output are required")
		return failure
	}
	var host deployment.Host = deployment.Host{}
	var err error = nil
	host, err = deployment.CurrentHost()
	if err != nil {
		return err
	}
	var objectID string = ""
	objectID, err = deployment.RootObjectID(*rootPath)
	if err != nil {
		return err
	}
	var manifest deployment.Manifest = deployment.Manifest{
		Schema: deployment.ManifestSchema, SchemaMajor: deployment.ManifestMajor,
		DeploymentID: *deploymentID, HostUUID: host.UUID, UID: host.UID,
		StoreRoot: *storeRoot, RuntimeDir: *runtimeDir,
		Roots: []deployment.Root{{ID: api.RootID(*rootID), Path: *rootPath, ObjectID: objectID, Exclusions: splitNonempty(*exclusions)}},
	}
	{
		var err error = nil
		err = deployment.Validate(&manifest, host)
		if err != nil {
			return err
		}
	}
	var payload []byte = nil
	payload, err = json.MarshalIndent(manifest, "", "  ")
	if err != nil {
		return err
	}
	payload = append(payload, '\n')
	{
		var err error = nil
		err = writeManifestAtomically(*manifestPath, payload)
		if err != nil {
			return err
		}
	}
	_, err = fmt.Fprintf(output, "%s\n", *manifestPath)
	return err
}

func writeManifestAtomically(path string, payload []byte) error {
	var directory string = filepath.Dir(path)
	var temporary *os.File = nil
	var err error = nil
	temporary, err = os.CreateTemp(directory, ".deployment-*.new")
	if err != nil {
		return err
	}
	var temporaryPath string = temporary.Name()
	defer os.Remove(temporaryPath)
	{
		var err error = nil
		err = temporary.Chmod(0o600)
		if err != nil {
			temporary.Close()
			return err
		}
	}
	{
		var err error = nil
		_, err = temporary.Write(payload)
		if err != nil {
			temporary.Close()
			return err
		}
	}
	{
		var err error = nil
		err = temporary.Sync()
		if err != nil {
			temporary.Close()
			return err
		}
	}
	{
		var err error = nil
		err = temporary.Close()
		if err != nil {
			return err
		}
	}
	{
		var err error = nil
		err = os.Rename(temporaryPath, path)
		if err != nil {
			return err
		}
	}
	var directoryHandle *os.File = nil
	directoryHandle, err = os.Open(directory)
	if err != nil {
		return err
	}
	var syncErr error = directoryHandle.Sync()
	var closeErr error = directoryHandle.Close()
	if syncErr != nil {
		return syncErr
	}
	return closeErr
}

func callLocal(args []string, output io.Writer) error {
	var flags *flag.FlagSet = flag.NewFlagSet("call-local", flag.ContinueOnError)
	var runtimeDir *string = flags.String("runtime-dir", "", "private endpoint directory")
	var authorityValue *string = flags.String("authority", "query", "query or admin")
	var requestJSON *string = flags.String("request", "", "single JSON request")
	var timeout *time.Duration = flags.Duration("timeout", 35*time.Second, "request timeout")
	{
		var err error = nil
		err = flags.Parse(args)
		if err != nil {
			return err
		}
	}
	var request transport.Request = transport.Request{}
	var input *strings.Reader = strings.NewReader(*requestJSON)
	var decoder *json.Decoder = json.NewDecoder(input)
	{
		var err error = nil
		err = decoder.Decode(&request)
		if err != nil {
			var failure error = fmt.Errorf("decode request: %w", err)
			return failure
		}
	}
	var trailing any = nil
	{
		var err error = nil
		err = decoder.Decode(&trailing)
		if err != io.EOF {
			if err == nil {
				err = errors.New("multiple JSON values")
			}
			var failure error = fmt.Errorf("request contains trailing data: %w", err)
			return failure
		}
	}
	var ctx context.Context = nil
	var cancel context.CancelFunc = nil
	ctx, cancel = context.WithTimeout(context.Background(), *timeout)
	defer cancel()
	var response transport.Response = transport.Response{}
	var err error = nil
	response, err = transport.CallLocal(ctx, *runtimeDir, transport.Authority(*authorityValue), request)
	if err != nil {
		return err
	}
	var encoder *json.Encoder = json.NewEncoder(output)
	var failure error = encoder.Encode(response)
	return failure
}

func splitNonempty(value string) []string {
	var result []string = nil
	{
		var item string = ""
		for _, item = range strings.Split(value, ",") {
			item = strings.TrimSpace(item)
			if item != "" {
				result = append(result, item)
			}
		}
	}
	return result
}

func serve(input io.Reader, output io.Writer, guard *sandbox.Guard) error {
	var failure error = serveWithStore(input, output, guard, "")
	return failure
}

func serveWithStore(input io.Reader, output io.Writer, guard *sandbox.Guard, storeRoot string) error {
	var failure error = serveConfigured(input, output, guard, serveOptions{storeRoot: storeRoot})
	return failure
}

type serveOptions struct {
	storeRoot         string
	rootID            api.RootID
	rootPath          string
	backgroundAdapter string
}

func serveConfigured(input io.Reader, output io.Writer, guard *sandbox.Guard, options serveOptions) error {
	var engine *service.Service = nil
	var err error = nil
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
		var failure error = errors.New("--root-id and --root-path must be supplied together")
		return failure
	}
	if options.rootID != "" {
		{
			var err error = nil
			_, err = engine.ApplyRoots(context.Background(), []api.RootSpec{{ID: options.rootID, Path: options.rootPath}})
			if err != nil {
				var failure error = fmt.Errorf("apply startup root: %w", err)
				return failure
			}
		}
	}
	switch options.backgroundAdapter {
	case "":
	case "fsevents":
		if options.rootID == "" {
			var failure error = errors.New("--background-observation=fsevents requires an explicit startup root")
			return failure
		}
		var adapter observation.Adapter = nil
		var err error = nil
		var configuration api.EffectiveConfiguration = engine.Configuration()
		adapter, err = fsevents.New(configuration.RootPolicy, fsevents.DefaultConfig())
		if err != nil {
			var failure error = fmt.Errorf("configure FSEvents observation: %w", err)
			return failure
		}
		{
			var err error = nil
			err = engine.StartBackgroundObservation(context.Background(), adapter, service.DefaultBackgroundPolicy())
			if err != nil {
				var failure error = fmt.Errorf("start FSEvents observation: %w", err)
				return failure
			}
		}
	case "rdcw":
		if options.rootID == "" {
			var failure error = errors.New("--background-observation=rdcw requires an explicit startup root")
			return failure
		}
		var adapter observation.Adapter = nil
		var err error = nil
		var configuration api.EffectiveConfiguration = engine.Configuration()
		adapter, err = rdcw.New(configuration.RootPolicy, rdcw.DefaultConfig())
		if err != nil {
			var failure error = fmt.Errorf("configure ReadDirectoryChangesW observation: %w", err)
			return failure
		}
		{
			var err error = nil
			err = engine.StartBackgroundObservation(context.Background(), adapter, service.DefaultBackgroundPolicy())
			if err != nil {
				var failure error = fmt.Errorf("start ReadDirectoryChangesW observation: %w", err)
				return failure
			}
		}
	default:
		var failure error = fmt.Errorf("unknown background observation adapter %q", options.backgroundAdapter)
		return failure
	}
	var failure error = transport.Serve(context.Background(), input, output, engine)
	return failure
}
