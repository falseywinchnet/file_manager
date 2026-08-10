//go:build windows

package deployment

import "os"

func openManifestNoFollow(path string) (*os.File, error) { return os.Open(path) }
