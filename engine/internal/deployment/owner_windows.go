//go:build windows

package deployment

import "os"

func fileOwner(os.FileInfo) (int, bool) { return 0, false }
