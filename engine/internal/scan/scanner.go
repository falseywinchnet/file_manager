// Package scan performs metadata-only observation through an os.Root. It does
// not open file contents, follow directory symlinks, or write into source roots.
package scan

import (
	"context"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
)

type OwnsFunc func(api.RootID, string) bool

type Scanner struct{}

func (Scanner) Scan(ctx context.Context, root api.RootSpec, owns OwnsFunc) (*catalog.Shard, error) {
	rootHandle, err := os.OpenRoot(root.Path)
	if err != nil {
		return nil, fmt.Errorf("open approved root: %w", err)
	}
	defer rootHandle.Close()

	rootInfo, err := rootHandle.Lstat(".")
	if err != nil {
		return nil, fmt.Errorf("observe approved root: %w", err)
	}
	rootIdentity, err := identity.Observe(rootHandle, ".", rootInfo)
	if err != nil {
		return nil, fmt.Errorf("observe approved root identity: %w", err)
	}
	rootObject := observedObject(rootIdentity, rootInfo)
	observations := make([]catalog.ObservedBinding, 0, 1024)
	parents := map[string]identity.Observation{".": rootIdentity}
	seen := uint64(0)
	err = fs.WalkDir(rootHandle.FS(), ".", func(relative string, entry fs.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		seen++
		if seen&1023 == 0 {
			if err := ctx.Err(); err != nil {
				return err
			}
		}
		if relative == "." {
			return nil
		}
		absolute := filepath.Join(root.Path, filepath.FromSlash(relative))
		if !owns(root.ID, absolute) {
			if entry.IsDir() {
				return fs.SkipDir
			}
			return nil
		}
		info, err := rootHandle.Lstat(relative)
		if err != nil {
			return err
		}
		observedIdentity, err := identity.Observe(rootHandle, relative, info)
		if err != nil {
			return fmt.Errorf("observe identity for %q: %w", relative, err)
		}
		parentPath := filepath.Dir(filepath.FromSlash(relative))
		parentIdentity, exists := parents[parentPath]
		if !exists {
			return fmt.Errorf("parent object for %q was not observed", relative)
		}
		observations = append(observations, catalog.ObservedBinding{
			Object:       observedObject(observedIdentity, info),
			Parent:       parentIdentity,
			Name:         entry.Name(),
			RelativePath: filepath.FromSlash(relative),
		})
		if info.IsDir() {
			parents[filepath.FromSlash(relative)] = observedIdentity
		}
		return nil
	})
	if err != nil {
		return nil, fmt.Errorf("walk approved root: %w", err)
	}
	if err := ctx.Err(); err != nil {
		return nil, err
	}
	return catalog.NewShard(root, rootObject, observations)
}

func observedObject(observed identity.Observation, info os.FileInfo) catalog.Object {
	return catalog.Object{
		Identity: observed, Kind: objectKind(info.Mode()), Size: info.Size(),
		Mode: uint32(info.Mode()), ModifiedUnixNano: info.ModTime().UnixNano(),
	}
}

func objectKind(mode os.FileMode) api.ObjectKind {
	switch {
	case mode&os.ModeSymlink != 0:
		return api.ObjectSymlink
	case mode.IsDir():
		return api.ObjectDirectory
	case mode.IsRegular():
		return api.ObjectRegular
	default:
		return api.ObjectOther
	}
}
