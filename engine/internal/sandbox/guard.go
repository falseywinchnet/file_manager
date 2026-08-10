// Package sandbox confines development and test access to an explicit root.
package sandbox

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"filemanager/engine/api"
	"filemanager/engine/internal/identity"
)

var ErrOutsideRoot = errors.New("path is outside sandbox root")

type Guard struct {
	root       string
	deployment string
	sandboxed  bool
	approved   map[api.RootID]ApprovedRoot
}

// ApprovedRoot is an installed-service admission. ObjectID is checked by the
// deployment loader before construction; Guard then enforces the immutable
// id/path and relative exclusion policy for every operation.
type ApprovedRoot struct {
	ID         api.RootID
	Path       string
	ObjectID   string
	Exclusions []string
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
	return &Guard{root: abs, deployment: "development_sandbox", sandboxed: true}, nil
}

// NewApproved constructs a manifest-bound installed-service guard. This is
// deliberately not a bypass flag: only the exact canonical id/path pairs in
// approved can be configured, and exclusions can only reduce their scope.
func NewApproved(deployment string, approved []ApprovedRoot) (*Guard, error) {
	if deployment == "" || deployment == "development_sandbox" {
		return nil, errors.New("installed deployment id is required")
	}
	if len(approved) == 0 {
		return nil, errors.New("at least one approved root is required")
	}
	guard := &Guard{deployment: deployment, approved: make(map[api.RootID]ApprovedRoot)}
	for _, candidate := range approved {
		if candidate.ID == "" {
			return nil, errors.New("approved root id is required")
		}
		if _, exists := guard.approved[candidate.ID]; exists {
			return nil, fmt.Errorf("duplicate approved root id %q", candidate.ID)
		}
		path, err := canonicalDirectory(candidate.Path)
		if err != nil {
			return nil, fmt.Errorf("resolve approved root %q: %w", candidate.ID, err)
		}
		volumeRoot := filepath.Clean(filepath.VolumeName(path) + string(filepath.Separator))
		if path == volumeRoot {
			return nil, fmt.Errorf("approved root %q may not be a filesystem root", candidate.ID)
		}
		if candidate.ObjectID == "" {
			return nil, fmt.Errorf("approved root %q object identity is required", candidate.ID)
		}
		objectID, err := observeRootObjectID(path)
		if err != nil {
			return nil, fmt.Errorf("observe approved root %q: %w", candidate.ID, err)
		}
		if objectID != candidate.ObjectID {
			return nil, fmt.Errorf("approved root %q identity changed", candidate.ID)
		}
		exclusions, err := canonicalExclusions(candidate.Exclusions)
		if err != nil {
			return nil, fmt.Errorf("approved root %q exclusions: %w", candidate.ID, err)
		}
		candidate.Path = path
		candidate.Exclusions = exclusions
		guard.approved[candidate.ID] = candidate
	}
	return guard, nil
}

func canonicalExclusions(values []string) ([]string, error) {
	result := make([]string, 0, len(values))
	seen := make(map[string]bool)
	for _, value := range values {
		clean := filepath.Clean(filepath.FromSlash(value))
		if clean == "." || filepath.IsAbs(clean) || clean == ".." || strings.HasPrefix(clean, ".."+string(filepath.Separator)) {
			return nil, fmt.Errorf("%q is not a contained relative path", value)
		}
		if !seen[clean] {
			seen[clean] = true
			result = append(result, clean)
		}
	}
	sort.Strings(result)
	return result, nil
}

func observeRootObjectID(path string) (string, error) {
	root, err := os.OpenRoot(path)
	if err != nil {
		return "", err
	}
	defer root.Close()
	info, err := root.Lstat(".")
	if err != nil {
		return "", err
	}
	observed, err := identity.Observe(root, ".", info)
	if err != nil {
		return "", err
	}
	return observed.ObjectID(), nil
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

func (g *Guard) Sandboxed() bool { return g.sandboxed }

func (g *Guard) Deployment() string { return g.deployment }

// ResolveRoot proves that a requested policy entry is exactly one of the
// immutable manifest admissions. Development mode retains contained-subroot
// behavior for disposable fixtures.
func (g *Guard) ResolveRoot(root api.RootSpec) (string, error) {
	if g.sandboxed {
		return g.ResolveDirectory(root.Path)
	}
	approved, exists := g.approved[root.ID]
	if !exists {
		return "", ErrOutsideRoot
	}
	resolved, err := canonicalDirectory(root.Path)
	if err != nil {
		return "", err
	}
	if resolved != approved.Path {
		return "", ErrOutsideRoot
	}
	objectID, err := observeRootObjectID(resolved)
	if err != nil || objectID != approved.ObjectID {
		return "", ErrOutsideRoot
	}
	return resolved, nil
}

func (g *Guard) ExpectedObjectID(rootID api.RootID) string {
	if approved, exists := g.approved[rootID]; exists {
		return approved.ObjectID
	}
	return ""
}

// Allows is the final per-entry admission predicate. Root ownership prevents
// overlapping projections; this predicate additionally prunes manifest
// exclusions before metadata is observed.
func (g *Guard) Allows(rootID api.RootID, absolute string) bool {
	if g.sandboxed {
		return g.contains(absolute)
	}
	approved, exists := g.approved[rootID]
	if !exists || !contains(approved.Path, absolute) {
		return false
	}
	relative, err := filepath.Rel(approved.Path, absolute)
	if err != nil {
		return false
	}
	for _, exclusion := range approved.Exclusions {
		if relative == exclusion || strings.HasPrefix(relative, exclusion+string(filepath.Separator)) {
			return false
		}
	}
	return true
}

// ResolveDirectory resolves an existing directory through symlinks and then
// proves that the resolved directory remains under the development sandbox.
// Scanners subsequently use os.Root so later traversal cannot escape through a
// symlink swap.
func (g *Guard) ResolveDirectory(candidate string) (string, error) {
	if candidate == "" {
		return "", errors.New("directory path is required")
	}
	resolved := candidate
	if !filepath.IsAbs(resolved) && g.sandboxed {
		resolved = filepath.Join(g.root, resolved)
	}
	var err error
	resolved, err = canonicalDirectory(resolved)
	if err != nil {
		return "", err
	}
	if g.sandboxed && !g.contains(resolved) {
		return "", ErrOutsideRoot
	}
	if !g.sandboxed {
		for _, approved := range g.approved {
			if resolved == approved.Path {
				return resolved, nil
			}
		}
		return "", ErrOutsideRoot
	}
	return resolved, nil
}

// Resolve returns an absolute contained path. This lexical guard is followed by
// platform identity/symlink checks before production scanning is admitted.
func (g *Guard) Resolve(candidate string) (string, error) {
	if !g.sandboxed {
		return "", ErrOutsideRoot
	}
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
	return contains(g.root, absolute)
}

func contains(root, absolute string) bool {
	relative, err := filepath.Rel(root, absolute)
	if err != nil {
		return false
	}
	return relative != ".." && !strings.HasPrefix(relative, ".."+string(filepath.Separator))
}
