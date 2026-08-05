// fileman-engine is the standalone development entry point. It intentionally
// refuses to start without a sandbox root.
package main

import (
	"context"
	"flag"
	"fmt"
	"io"
	"os"

	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
	"filemanager/engine/internal/transport"
)

func main() {
	sandboxRoot := flag.String("sandbox-root", "", "absolute disposable root visible to the development service")
	storeRoot := flag.String("store-root", "", "existing engine-owned directory for the M2 one-root durable service")
	flag.Parse()

	guard, err := sandbox.New(*sandboxRoot)
	if err != nil {
		fmt.Fprintln(os.Stderr, "fileman-engine refuses to start:", err)
		os.Exit(2)
	}

	if err := serveWithStore(os.Stdin, os.Stdout, guard, *storeRoot); err != nil {
		fmt.Fprintln(os.Stderr, "fileman-engine:", err)
		os.Exit(1)
	}
}

func serve(input io.Reader, output io.Writer, guard *sandbox.Guard) error {
	return serveWithStore(input, output, guard, "")
}

func serveWithStore(input io.Reader, output io.Writer, guard *sandbox.Guard, storeRoot string) error {
	var engine *service.Service
	var err error
	if storeRoot == "" {
		engine, err = service.New(guard)
	} else {
		engine, err = service.NewPersistent(guard, storeRoot)
	}
	if err != nil {
		return err
	}
	defer engine.Close()
	return transport.Serve(context.Background(), input, output, engine)
}
