// fileman-engine is the standalone development entry point. It intentionally
// refuses to start without a sandbox root.
package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation/fsevents"
	"filemanager/engine/internal/observation/rdcw"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
	"filemanager/engine/internal/transport"
)

func main() {
	sandboxRoot := flag.String("sandbox-root", "", "absolute disposable root visible to the development service")
	storeRoot := flag.String("store-root", "", "existing engine-owned directory for the M2 one-root durable service")
	rootID := flag.String("root-id", "", "approved startup root id (requires --root-path)")
	rootPath := flag.String("root-path", "", "approved startup root inside the disposable sandbox (requires --root-id)")
	backgroundAdapter := flag.String("background-observation", "", "experimental native adapter: fsevents (macOS) or rdcw (Windows)")
	flag.Parse()

	guard, err := sandbox.New(*sandboxRoot)
	if err != nil {
		fmt.Fprintln(os.Stderr, "fileman-engine refuses to start:", err)
		os.Exit(2)
	}

	options := serveOptions{
		storeRoot: *storeRoot, rootID: api.RootID(*rootID), rootPath: *rootPath,
		backgroundAdapter: *backgroundAdapter,
	}
	if err := serveConfigured(os.Stdin, os.Stdout, guard, options); err != nil {
		fmt.Fprintln(os.Stderr, "fileman-engine:", err)
		os.Exit(1)
	}
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
