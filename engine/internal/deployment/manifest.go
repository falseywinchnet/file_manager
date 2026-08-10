// Package deployment validates the host-bound authority manifest used by an
// installed Engine service. It is the production replacement for the
// disposable development sandbox, not a bypass around root admission.
package deployment

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"filemanager/engine/api"
	"filemanager/engine/internal/identity"
	"filemanager/engine/internal/sandbox"
)

const (
	ManifestSchema = "fileman.engine.deployment"
	ManifestMajor  = 1
	maxManifest    = 1 << 20
)

type Host struct {
	UUID string
	UID  int
}

type Root struct {
	ID         api.RootID `json:"id"`
	Path       string     `json:"path"`
	ObjectID   string     `json:"object_id"`
	Exclusions []string   `json:"exclusions,omitempty"`
}

type Manifest struct {
	Schema       string `json:"schema"`
	SchemaMajor  int    `json:"schema_major"`
	DeploymentID string `json:"deployment_id"`
	HostUUID     string `json:"host_uuid"`
	UID          int    `json:"uid"`
	StoreRoot    string `json:"store_root"`
	RuntimeDir   string `json:"runtime_directory"`
	Roots        []Root `json:"approved_roots"`
}

func CurrentHost() (Host, error) {
	uuid, err := hostUUID()
	if err != nil {
		return Host{}, err
	}
	return Host{UUID: strings.ToUpper(strings.TrimSpace(uuid)), UID: os.Getuid()}, nil
}

// RootObjectID observes the exact root directory identity through os.Root.
func RootObjectID(path string) (string, error) {
	root, err := os.OpenRoot(path)
	if err != nil {
		return "", fmt.Errorf("open approved root: %w", err)
	}
	defer root.Close()
	info, err := root.Lstat(".")
	if err != nil {
		return "", fmt.Errorf("stat approved root: %w", err)
	}
	observed, err := identity.Observe(root, ".", info)
	if err != nil {
		return "", fmt.Errorf("observe approved root identity: %w", err)
	}
	return observed.ObjectID(), nil
}

// LoadSecure reads a regular, same-uid, 0600 manifest without following a
// manifest-path symlink, rejects trailing JSON, and validates it for this host.
func LoadSecure(path string) (Manifest, error) {
	file, err := openManifestNoFollow(path)
	if err != nil {
		return Manifest{}, fmt.Errorf("open deployment manifest: %w", err)
	}
	defer file.Close()
	info, err := file.Stat()
	if err != nil {
		return Manifest{}, fmt.Errorf("inspect deployment manifest: %w", err)
	}
	if !info.Mode().IsRegular() {
		return Manifest{}, errors.New("deployment manifest must be a regular file, not a symlink")
	}
	if info.Mode().Perm() != 0o600 {
		return Manifest{}, fmt.Errorf("deployment manifest mode is %04o, want 0600", info.Mode().Perm())
	}
	if owner, ok := fileOwner(info); !ok || owner != os.Getuid() {
		return Manifest{}, errors.New("deployment manifest must be owned by the service uid")
	}
	var manifest Manifest
	decoder := json.NewDecoder(io.LimitReader(file, maxManifest+1))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&manifest); err != nil {
		return Manifest{}, fmt.Errorf("decode deployment manifest: %w", err)
	}
	var trailing any
	if err := decoder.Decode(&trailing); err != io.EOF {
		return Manifest{}, errors.New("deployment manifest contains trailing data")
	}
	host, err := CurrentHost()
	if err != nil {
		return Manifest{}, err
	}
	if err := Validate(&manifest, host); err != nil {
		return Manifest{}, err
	}
	return manifest, nil
}

func Validate(manifest *Manifest, host Host) error {
	if manifest == nil {
		return errors.New("deployment manifest is required")
	}
	if manifest.Schema != ManifestSchema || manifest.SchemaMajor != ManifestMajor {
		return errors.New("unsupported deployment manifest schema")
	}
	if manifest.DeploymentID == "" || manifest.DeploymentID == "development_sandbox" {
		return errors.New("installed deployment id is required")
	}
	if !strings.EqualFold(manifest.HostUUID, host.UUID) || manifest.UID != host.UID {
		return errors.New("deployment manifest is bound to a different host or uid")
	}
	store, err := exactDirectory(manifest.StoreRoot)
	if err != nil {
		return fmt.Errorf("store root: %w", err)
	}
	runtimeDir, err := exactDirectory(manifest.RuntimeDir)
	if err != nil {
		return fmt.Errorf("runtime directory: %w", err)
	}
	if err := requirePrivateDirectory(store); err != nil {
		return fmt.Errorf("store root: %w", err)
	}
	if err := requirePrivateDirectory(runtimeDir); err != nil {
		return fmt.Errorf("runtime directory: %w", err)
	}
	manifest.StoreRoot, manifest.RuntimeDir = store, runtimeDir
	approved := make([]sandbox.ApprovedRoot, 0, len(manifest.Roots))
	for index := range manifest.Roots {
		root := &manifest.Roots[index]
		path, err := exactDirectory(root.Path)
		if err != nil {
			return fmt.Errorf("approved root %q: %w", root.ID, err)
		}
		if contains(path, store) || contains(path, runtimeDir) {
			return fmt.Errorf("approved root %q contains engine-owned state", root.ID)
		}
		objectID, err := RootObjectID(path)
		if err != nil {
			return err
		}
		if !bytes.Equal([]byte(objectID), []byte(root.ObjectID)) {
			return fmt.Errorf("approved root %q identity changed", root.ID)
		}
		root.Path = path
		approved = append(approved, sandbox.ApprovedRoot{ID: root.ID, Path: path, ObjectID: root.ObjectID, Exclusions: root.Exclusions})
	}
	_, err = sandbox.NewApproved(manifest.DeploymentID, approved)
	return err
}

func (m Manifest) Guard() (*sandbox.Guard, error) {
	approved := make([]sandbox.ApprovedRoot, 0, len(m.Roots))
	for _, root := range m.Roots {
		approved = append(approved, sandbox.ApprovedRoot{ID: root.ID, Path: root.Path, ObjectID: root.ObjectID, Exclusions: root.Exclusions})
	}
	return sandbox.NewApproved(m.DeploymentID, approved)
}

func (m Manifest) RootSpecs() []api.RootSpec {
	result := make([]api.RootSpec, 0, len(m.Roots))
	for _, root := range m.Roots {
		result = append(result, api.RootSpec{ID: root.ID, Path: root.Path})
	}
	return result
}

func exactDirectory(path string) (string, error) {
	if !filepath.IsAbs(path) {
		return "", errors.New("path must be absolute")
	}
	resolved, err := filepath.EvalSymlinks(path)
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

func requirePrivateDirectory(path string) error {
	info, err := os.Stat(path)
	if err != nil {
		return err
	}
	if info.Mode().Perm()&0o077 != 0 {
		return fmt.Errorf("mode %04o grants group or other access", info.Mode().Perm())
	}
	if owner, ok := fileOwner(info); !ok || owner != os.Getuid() {
		return errors.New("directory is not owned by the service uid")
	}
	return nil
}

func contains(root, path string) bool {
	relative, err := filepath.Rel(root, path)
	return err == nil && relative != ".." && !strings.HasPrefix(relative, ".."+string(filepath.Separator))
}
