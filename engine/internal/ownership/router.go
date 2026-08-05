// Package ownership routes each path to its most-specific approved root.
package ownership

import (
	"errors"
	"path/filepath"
	"sync"
)

var ErrNoOwner = errors.New("no approved root owns path")

type Root struct {
	ID   string
	Path string
}

type Router struct {
	mu    sync.RWMutex
	roots map[string]Root
}

func (r *Router) Replace(roots []Root) error {
	normalized := make(map[string]Root, len(roots))
	seenID := make(map[string]struct{}, len(roots))
	seenPath := make(map[string]struct{}, len(roots))
	for _, root := range roots {
		if root.ID == "" || root.Path == "" {
			return errors.New("root id and path are required")
		}
		if _, exists := seenID[root.ID]; exists {
			return errors.New("duplicate root id")
		}
		absolute, err := filepath.Abs(root.Path)
		if err != nil {
			return err
		}
		absolute = filepath.Clean(absolute)
		if _, exists := seenPath[absolute]; exists {
			return errors.New("duplicate root path")
		}
		seenID[root.ID] = struct{}{}
		seenPath[absolute] = struct{}{}
		normalized[absolute] = Root{ID: root.ID, Path: absolute}
	}
	r.mu.Lock()
	r.roots = normalized
	r.mu.Unlock()
	return nil
}

func (r *Router) Owner(path string) (Root, error) {
	absolute, err := filepath.Abs(path)
	if err != nil {
		return Root{}, err
	}
	absolute = filepath.Clean(absolute)
	return r.ownerCleanAbsolute(absolute)
}

// OwnerCleanAbsolute avoids repeated absolute-path normalization in scanners
// that already construct canonical absolute paths.
func (r *Router) OwnerCleanAbsolute(path string) (Root, error) {
	if !filepath.IsAbs(path) {
		return Root{}, errors.New("path must be a clean absolute path")
	}
	return r.ownerCleanAbsolute(path)
}

func (r *Router) ownerCleanAbsolute(path string) (Root, error) {
	r.mu.RLock()
	defer r.mu.RUnlock()
	current := path
	for {
		if root, exists := r.roots[current]; exists {
			return root, nil
		}
		parent := filepath.Dir(current)
		if parent == current {
			break
		}
		current = parent
	}
	return Root{}, ErrNoOwner
}
