//go:build windows

package main

import (
	"context"
	"encoding/json"
	"errors"
	"filemanager/engine/internal/sandbox"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/deployment"
	"filemanager/engine/internal/service"
	"filemanager/engine/internal/transport"
	"filemanager/engine/internal/windowssecure"
)

func windowsCommand(command string, args []string, output io.Writer) error {
	switch command {
	case "create-windows-manifest":
		var failure error = createWindowsManifest(args, output)
		return failure
	case "serve-windows":
		var failure error = serveWindowsManifest(args)
		return failure
	default:
		var failure error = errors.New("unknown Windows command")
		return failure
	}
}

func createWindowsManifest(args []string, output io.Writer) error {
	var flags *flag.FlagSet = flag.NewFlagSet("create-windows-manifest", flag.ContinueOnError)
	var destination *string = flags.String("output", "", "manifest file within an Engine-owned private directory")
	var rootPath *string = flags.String("root-path", "", "explicit user-approved source root")
	var rootID *string = flags.String("root-id", "", "explicit root identity")
	var id *string = flags.String("deployment-id", "", "deployment identity")
	var runtimeDir *string = flags.String("runtime-dir", "", "private runtime leaf to create or verify")
	var storeRoot *string = flags.String("store-root", "", "private store leaf; only with --index-enabled")
	var indexEnabled *bool = flags.Bool("index-enabled", false, "explicit persistent-index consent (default live-only)")
	var exclusions *string = flags.String("exclude", "", "comma-separated relative exclusions")
	{
		var err error = nil
		err = flags.Parse(args)
		if err != nil {
			return err
		}
	}
	if *destination == "" || *rootPath == "" || *rootID == "" || *id == "" || *runtimeDir == "" {
		var failure error = errors.New("output, root-path, root-id, deployment-id, runtime-dir are required")
		return failure
	}
	if *indexEnabled != (*storeRoot != "") {
		var failure error = errors.New("index-enabled and store-root must be supplied together")
		return failure
	}
	if !filepath.IsAbs(*destination) {
		var failure error = errors.New("manifest output must be absolute")
		return failure
	}
	if !filepath.IsAbs(*rootPath) {
		var failure error = errors.New("source root must be absolute")
		return failure
	}
	{
		var err error = nil
		err = windowssecure.RejectReparsePath(*rootPath)
		if err != nil {
			return err
		}
	}
	{
		var state string = ""
		for _, state = range []string{*destination, *runtimeDir, *storeRoot} {
			if state == "" {
				continue
			}
			var relative string = ""
			var err error = nil
			relative, err = filepath.Rel(*rootPath, state)
			if err == nil && relative != ".." && !strings.HasPrefix(relative, ".."+string(filepath.Separator)) {
				var failure error = errors.New("Engine state may not be created within an approved source root")
				return failure
			}
		}
	}
	// Only explicit Engine state leaves are created; source paths are read-only.
	{
		var directory string = ""
		for _, directory = range []string{filepath.Dir(*destination), *runtimeDir} {
			{
				var err error = nil
				err = windowssecure.CreateDirectory(directory)
				if err != nil {
					return err
				}
			}
		}
	}
	if *indexEnabled {
		{
			var err error = nil
			err = windowssecure.CreateDirectory(*storeRoot)
			if err != nil {
				return err
			}
		}
	}
	var host string = ""
	var err error = nil
	host, err = deployment.WindowsHostID()
	if err != nil {
		return err
	}
	var sid string = ""
	sid, err = windowssecure.CurrentSID()
	if err != nil {
		return err
	}
	{
		var err error = nil
		err = windowssecure.RejectReparsePath(*rootPath)
		if err != nil {
			return err
		}
	}
	var object string = ""
	object, err = deployment.RootObjectID(*rootPath)
	if err != nil {
		return err
	}
	var manifest deployment.WindowsManifest = deployment.WindowsManifest{Schema: deployment.WindowsManifestSchema, DeploymentID: *id, HostID: host,
		UserSID: sid, RuntimeDir: *runtimeDir, StoreRoot: *storeRoot, IndexEnabled: *indexEnabled,
		Roots: []deployment.Root{{ID: api.RootID(*rootID), Path: *rootPath, ObjectID: object, Exclusions: splitNonempty(*exclusions)}}}
	{
		var err error = nil
		err = manifest.Validate()
		if err != nil {
			return err
		}
	}
	var payload []byte = nil
	payload, err = json.MarshalIndent(manifest, "", "  ")
	if err != nil {
		return err
	}
	{
		var err error = nil
		payload = append(payload, '\n')
		err = windowssecure.Write(*destination, payload)
		if err != nil {
			return err
		}
	}
	_, err = fmt.Fprintln(output, *destination)
	return err
}

func serveWindowsManifest(args []string) error {
	var ctx context.Context = context.Background()
	var err error = serveWindowsManifestContext(ctx, args)
	return err
}

func serveWindowsManifestContext(parent context.Context, args []string) error {
	var flags *flag.FlagSet = flag.NewFlagSet("serve-windows", flag.ContinueOnError)
	var path *string = flags.String("manifest", "", "explicit host/SID/root-bound Windows manifest")
	{
		var err error = nil
		err = flags.Parse(args)
		if err != nil {
			return err
		}
	}
	var manifest deployment.WindowsManifest = deployment.WindowsManifest{}
	var err error = nil
	manifest, err = deployment.LoadWindows(*path)
	if err != nil {
		return err
	}
	var guard *sandbox.Guard = nil
	guard, err = manifest.Guard()
	if err != nil {
		return err
	}
	// Pin runtime/store leaf identities during startup and serving.
	var runtimeDir *os.File = nil
	runtimeDir, err = windowssecure.Open(manifest.RuntimeDir, true)
	if err != nil {
		return err
	}
	defer runtimeDir.Close()
	var startupLock *os.File = nil
	startupLock, err = windowssecure.Lock(filepath.Join(manifest.RuntimeDir, "startup.lock"))
	if err != nil {
		return err
	}
	defer startupLock.Close()
	var engine *service.Service = nil
	if manifest.IndexEnabled {
		var store *os.File = nil
		var err error = nil
		store, err = windowssecure.Open(manifest.StoreRoot, true)
		if err != nil {
			return err
		}
		defer store.Close()
		var writerLock *os.File = nil
		writerLock, err = windowssecure.Lock(filepath.Join(manifest.StoreRoot, "writer.lock"))
		if err != nil {
			return err
		}
		defer writerLock.Close()
		engine, err = service.NewPersistent(guard, manifest.StoreRoot)
		if err != nil {
			return err
		}
	} else {
		engine, err = service.New(guard)
		if err != nil {
			return err
		}
	}
	defer engine.Close()
	{
		var err error = nil
		_, err = engine.ApplyRoots(parent, manifest.RootSpecs())
		if err != nil {
			return err
		}
	}
	if manifest.IndexEnabled {
		// Reconcile before endpoint publication: recovered records never bypass
		// changed manifest exclusions. No Windows admission-marker shortcut.
		var ctx context.Context = nil
		var cancel context.CancelFunc = nil
		ctx, cancel = context.WithTimeout(parent, 5*time.Minute)
		defer cancel()
		{
			var root api.RootSpec = api.RootSpec{}
			for _, root = range manifest.RootSpecs() {
				{
					var err error = nil
					_, err = engine.Reconcile(ctx, root.ID)
					if err != nil {
						return err
					}
				}
			}
		}
	}
	var failure error = transport.ServeLocal(parent, manifest.RuntimeDir, engine)
	return failure
}
