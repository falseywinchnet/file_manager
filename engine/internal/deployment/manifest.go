// Package deployment validates the host-bound authority manifest used by an
// installed Engine service. It is the production replacement for the
// disposable development sandbox, not a bypass around root admission.
package deployment

import (
	"bytes"
	"crypto/sha256"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"filemanager/engine/api"
	"filemanager/engine/internal/identity"
	"filemanager/engine/internal/sandbox"
)

const (
	ManifestSchema = "fileman.engine.deployment"
	ManifestMajor  = 1
	maxManifest    = 1 << 20
	admissionFile  = "ADMISSION"
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

type admissionRecord struct {
	Schema     string         `json:"schema"`
	Digest     string         `json:"digest"`
	Generation api.Generation `json:"generation"`
}

const admissionSchema = "fileman.engine.admission.v1"

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

// AdmissionDigest identifies the security-relevant root authority independently
// of JSON field or list order. Store and runtime placement are validated
// separately and do not affect which source records a generation may contain.
func (m Manifest) AdmissionDigest() string {
	type digestRoot struct {
		ID         api.RootID `json:"id"`
		Path       string     `json:"path"`
		ObjectID   string     `json:"object_id"`
		Exclusions []string   `json:"exclusions,omitempty"`
	}
	roots := make([]digestRoot, 0, len(m.Roots))
	for _, root := range m.Roots {
		exclusions := append([]string(nil), root.Exclusions...)
		for index := range exclusions {
			exclusions[index] = filepath.Clean(filepath.FromSlash(exclusions[index]))
		}
		sort.Strings(exclusions)
		roots = append(roots, digestRoot{ID: root.ID, Path: root.Path, ObjectID: root.ObjectID, Exclusions: exclusions})
	}
	sort.Slice(roots, func(i, j int) bool {
		if roots[i].ID != roots[j].ID {
			return roots[i].ID < roots[j].ID
		}
		return roots[i].Path < roots[j].Path
	})
	material := struct {
		Schema       string       `json:"schema"`
		SchemaMajor  int          `json:"schema_major"`
		DeploymentID string       `json:"deployment_id"`
		HostUUID     string       `json:"host_uuid"`
		UID          int          `json:"uid"`
		Roots        []digestRoot `json:"approved_roots"`
	}{m.Schema, m.SchemaMajor, m.DeploymentID, strings.ToUpper(m.HostUUID), m.UID, roots}
	encoded, _ := json.Marshal(material)
	digest := sha256.Sum256(encoded)
	return fmt.Sprintf("%x", digest[:])
}

// AdmissionMatches proves that the recovered generation was produced under
// the current root/exclusion authority. A missing marker is a safe cache miss;
// malformed or loosely protected state fails closed.
func AdmissionMatches(storeRoot, digest string, generation api.Generation) (bool, error) {
	if len(digest) != sha256.Size*2 || generation == 0 {
		return false, errors.New("admission digest and generation are required")
	}
	path := filepath.Join(storeRoot, admissionFile)
	file, err := openManifestNoFollow(path)
	if errors.Is(err, os.ErrNotExist) {
		return false, nil
	}
	if err != nil {
		return false, fmt.Errorf("open admission record: %w", err)
	}
	defer file.Close()
	info, err := file.Stat()
	if err != nil {
		return false, err
	}
	if !info.Mode().IsRegular() || info.Mode().Perm() != 0o600 {
		return false, errors.New("admission record must be a regular 0600 file")
	}
	if owner, ok := fileOwner(info); !ok || owner != os.Getuid() {
		return false, errors.New("admission record must be owned by the service uid")
	}
	var record admissionRecord
	decoder := json.NewDecoder(io.LimitReader(file, 4_097))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&record); err != nil {
		return false, fmt.Errorf("decode admission record: %w", err)
	}
	var trailing any
	if err := decoder.Decode(&trailing); err != io.EOF {
		return false, errors.New("admission record contains trailing data")
	}
	if record.Schema != admissionSchema || len(record.Digest) != sha256.Size*2 || record.Generation == 0 {
		return false, errors.New("admission record fields are invalid")
	}
	return record.Digest == digest && record.Generation == generation, nil
}

// CommitAdmission atomically records the policy/generation pair only after a
// checked reconciliation has published successfully.
func CommitAdmission(storeRoot, digest string, generation api.Generation) error {
	if len(digest) != sha256.Size*2 || generation == 0 {
		return errors.New("admission digest and generation are required")
	}
	payload, err := json.Marshal(admissionRecord{Schema: admissionSchema, Digest: digest, Generation: generation})
	if err != nil {
		return err
	}
	payload = append(payload, '\n')
	temporary, err := os.CreateTemp(storeRoot, ".admission-*.new")
	if err != nil {
		return err
	}
	temporaryPath := temporary.Name()
	defer os.Remove(temporaryPath)
	if err := temporary.Chmod(0o600); err != nil {
		temporary.Close()
		return err
	}
	if _, err := temporary.Write(payload); err != nil {
		temporary.Close()
		return err
	}
	if err := temporary.Sync(); err != nil {
		temporary.Close()
		return err
	}
	if err := temporary.Close(); err != nil {
		return err
	}
	if err := os.Rename(temporaryPath, filepath.Join(storeRoot, admissionFile)); err != nil {
		return err
	}
	directory, err := os.Open(storeRoot)
	if err != nil {
		return err
	}
	syncErr := directory.Sync()
	closeErr := directory.Close()
	if syncErr != nil {
		return syncErr
	}
	return closeErr
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
