//go:build !windows

package main

import (
	"errors"
	"io"
)

func windowsCommand(string, []string, io.Writer) error {
	return errors.New("Windows deployment is unavailable on this platform")
}
