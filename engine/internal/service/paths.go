package service

import (
	"path/filepath"
	"strings"
)

func pathContains(root, path string) bool {
	if root == path {
		return true
	}
	relative, err := filepath.Rel(root, path)
	return err == nil && relative != ".." && !strings.HasPrefix(relative, ".."+string(filepath.Separator))
}
