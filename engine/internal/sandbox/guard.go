// Package sandbox confines development and test access to an explicit root.
package sandbox

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

var ErrOutsideRoot = errors.New("path is outside sandbox root")

type Guard struct {
	root string
}

func New(root string) (*Guard, error) {
	if root == "" {
		return nil, errors.New("sandbox root is required")
	}
	if !filepath.IsAbs(root) {
		return nil, errors.New("sandbox root must be absolute")
	}
	abs, err := canonicalDirectory(root)
	if err != nil {
		return nil, fmt.Errorf("resolve sandbox root: %w", err)
	}
	volumeRoot := filepath.Clean(filepath.VolumeName(abs) + string(filepath.Separator))
	if abs == volumeRoot {
		return nil, errors.New("sandbox root may not be a filesystem root")
	}
	if current, err := canonicalDirectory("."); err == nil && abs == current {
		return nil, errors.New("sandbox root may not be the current directory")
	}
	if home, err := os.UserHomeDir(); err == nil {
		if canonicalHome, err := canonicalDirectory(home); err == nil && abs == canonicalHome {
			return nil, errors.New("sandbox root may not be the user home directory")
		}
	}
	return &Guard{root: abs}, nil
}

func canonicalDirectory(path string) (string, error) {
	abs, err := filepath.Abs(path)
	if err != nil {
		return "", err
	}
	resolved, err := filepath.EvalSymlinks(abs)
	if err != nil {
		return "", err
	}
	info, err := os.Stat(resolved)
	if err != nil {
		return "", err
	}
	if !info.IsDir() {
		return "", errors.New("path is not a directory")
	}
	return filepath.Clean(resolved), nil
}

func (g *Guard) Root() string { return g.root }

// ResolveDirectory resolves an existing directory through symlinks and then
// proves that the resolved directory remains under the development sandbox.
// Scanners subsequently use os.Root so later traversal cannot escape through a
// symlink swap.
func (g *Guard) ResolveDirectory(candidate string) (string, error) {
	if candidate == "" {
		return "", errors.New("directory path is required")
	}
	resolved := candidate
	if !filepath.IsAbs(resolved) {
		resolved = filepath.Join(g.root, resolved)
	}
	var err error
	resolved, err = canonicalDirectory(resolved)
	if err != nil {
		return "", err
	}
	if !g.contains(resolved) {
		return "", ErrOutsideRoot
	}
	return resolved, nil
}

// Resolve returns an absolute contained path. This lexical guard is followed by
// platform identity/symlink checks before production scanning is admitted.
func (g *Guard) Resolve(candidate string) (string, error) {
	var absolute string
	if filepath.IsAbs(candidate) {
		absolute = filepath.Clean(candidate)
	} else {
		absolute = filepath.Join(g.root, candidate)
	}
	if !g.contains(absolute) {
		return "", ErrOutsideRoot
	}
	return absolute, nil
}

func (g *Guard) contains(absolute string) bool {
	relative, err := filepath.Rel(g.root, absolute)
	if err != nil {
		return false
	}
	return relative != ".." && !strings.HasPrefix(relative, ".."+string(filepath.Separator))
}
