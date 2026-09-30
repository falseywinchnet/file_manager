//go:build windows

package main

import (
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io"
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
		return createWindowsManifest(args, output)
	case "serve-windows":
		return serveWindowsManifest(args)
	default:
		return errors.New("unknown Windows command")
	}
}

func createWindowsManifest(args []string, output io.Writer) error {
	flags := flag.NewFlagSet("create-windows-manifest", flag.ContinueOnError)
	destination := flags.String("output", "", "manifest file within an Engine-owned private directory")
	rootPath := flags.String("root-path", "", "explicit user-approved source root")
	rootID := flags.String("root-id", "", "explicit root identity")
	id := flags.String("deployment-id", "", "deployment identity")
	runtimeDir := flags.String("runtime-dir", "", "private runtime leaf to create or verify")
	storeRoot := flags.String("store-root", "", "private store leaf; only with --index-enabled")
	indexEnabled := flags.Bool("index-enabled", false, "explicit persistent-index consent (default live-only)")
	exclusions := flags.String("exclude", "", "comma-separated relative exclusions")
	if err := flags.Parse(args); err != nil {
		return err
	}
	if *destination == "" || *rootPath == "" || *rootID == "" || *id == "" || *runtimeDir == "" {
		return errors.New("output, root-path, root-id, deployment-id, runtime-dir are required")
	}
	if *indexEnabled != (*storeRoot != "") {
		return errors.New("index-enabled and store-root must be supplied together")
	}
	if !filepath.IsAbs(*destination) {
		return errors.New("manifest output must be absolute")
	}
	if !filepath.IsAbs(*rootPath) {
		return errors.New("source root must be absolute")
	}
	if err := windowssecure.RejectReparsePath(*rootPath); err != nil {
		return err
	}
	for _, state := range []string{*destination, *runtimeDir, *storeRoot} {
		if state == "" {
			continue
		}
		relative, err := filepath.Rel(*rootPath, state)
		if err == nil && relative != ".." && !strings.HasPrefix(relative, ".."+string(filepath.Separator)) {
			return errors.New("Engine state may not be created within an approved source root")
		}
	}
	// Only explicit Engine state leaves are created; source paths are read-only.
	for _, directory := range []string{filepath.Dir(*destination), *runtimeDir} {
		if err := windowssecure.CreateDirectory(directory); err != nil {
			return err
		}
	}
	if *indexEnabled {
		if err := windowssecure.CreateDirectory(*storeRoot); err != nil {
			return err
		}
	}
	host, err := deployment.WindowsHostID()
	if err != nil {
		return err
	}
	sid, err := windowssecure.CurrentSID()
	if err != nil {
		return err
	}
	if err := windowssecure.RejectReparsePath(*rootPath); err != nil {
		return err
	}
	object, err := deployment.RootObjectID(*rootPath)
	if err != nil {
		return err
	}
	manifest := deployment.WindowsManifest{Schema: deployment.WindowsManifestSchema, DeploymentID: *id, HostID: host,
		UserSID: sid, RuntimeDir: *runtimeDir, StoreRoot: *storeRoot, IndexEnabled: *indexEnabled,
		Roots: []deployment.Root{{ID: api.RootID(*rootID), Path: *rootPath, ObjectID: object, Exclusions: splitNonempty(*exclusions)}}}
	if err := manifest.Validate(); err != nil {
		return err
	}
	payload, err := json.MarshalIndent(manifest, "", "  ")
	if err != nil {
		return err
	}
	if err := windowssecure.Write(*destination, append(payload, '\n')); err != nil {
		return err
	}
	_, err = fmt.Fprintln(output, *destination)
	return err
}

func serveWindowsManifest(args []string) error {
	flags := flag.NewFlagSet("serve-windows", flag.ContinueOnError)
	path := flags.String("manifest", "", "explicit host/SID/root-bound Windows manifest")
	if err := flags.Parse(args); err != nil {
		return err
	}
	manifest, err := deployment.LoadWindows(*path)
	if err != nil {
		return err
	}
	guard, err := manifest.Guard()
	if err != nil {
		return err
	}
	// Pin runtime/store leaf identities during startup and serving.
	runtimeDir, err := windowssecure.Open(manifest.RuntimeDir, true)
	if err != nil {
		return err
	}
	defer runtimeDir.Close()
	startupLock, err := windowssecure.Lock(filepath.Join(manifest.RuntimeDir, "startup.lock"))
	if err != nil {
		return err
	}
	defer startupLock.Close()
	var engine *service.Service
	if manifest.IndexEnabled {
		store, err := windowssecure.Open(manifest.StoreRoot, true)
		if err != nil {
			return err
		}
		defer store.Close()
		writerLock, err := windowssecure.Lock(filepath.Join(manifest.StoreRoot, "writer.lock"))
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
	if _, err := engine.ApplyRoots(context.Background(), manifest.RootSpecs()); err != nil {
		return err
	}
	if manifest.IndexEnabled {
		// Reconcile before endpoint publication: recovered records never bypass
		// changed manifest exclusions. No Windows admission-marker shortcut.
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
		defer cancel()
		for _, root := range manifest.RootSpecs() {
			if _, err := engine.Reconcile(ctx, root.ID); err != nil {
				return err
			}
		}
	}
	return transport.ServeLocal(context.Background(), manifest.RuntimeDir, engine)
}
