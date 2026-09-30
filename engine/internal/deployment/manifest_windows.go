//go:build windows

package deployment

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"path/filepath"
	"strings"

	"golang.org/x/sys/windows/registry"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/windowssecure"
)

const WindowsManifestSchema = "fileman.engine.windows-deployment.v1"

type WindowsManifest struct {
	Schema       string `json:"schema"`
	DeploymentID string `json:"deployment_id"`
	HostID       string `json:"host_id"`
	UserSID      string `json:"user_sid"`
	RuntimeDir   string `json:"runtime_directory"`
	StoreRoot    string `json:"store_root,omitempty"`
	IndexEnabled bool   `json:"index_enabled"`
	Roots        []Root `json:"approved_roots"`
}

func WindowsHostID() (string, error) {
	key, err := registry.OpenKey(registry.LOCAL_MACHINE, `SOFTWARE\Microsoft\Cryptography`, registry.QUERY_VALUE|registry.WOW64_64KEY)
	if err != nil {
		return "", err
	}
	defer key.Close()
	value, _, err := key.GetStringValue("MachineGuid")
	if err != nil {
		return "", err
	}
	if strings.TrimSpace(value) == "" {
		return "", errors.New("Windows host identity is empty")
	}
	return strings.ToLower(strings.TrimSpace(value)), nil
}

func (m *WindowsManifest) Validate() error {
	if m.Schema != WindowsManifestSchema || m.DeploymentID == "" || m.DeploymentID == "development_sandbox" {
		return errors.New("invalid Windows deployment schema or identity")
	}
	sid, err := windowssecure.CurrentSID()
	if err != nil {
		return err
	}
	host, err := WindowsHostID()
	if err != nil {
		return err
	}
	if m.UserSID != sid || m.HostID != host {
		return errors.New("Windows manifest belongs to another host or user")
	}
	if !filepath.IsAbs(m.RuntimeDir) {
		return errors.New("runtime directory must be absolute")
	}
	runtime, err := windowssecure.Open(m.RuntimeDir, true)
	if err != nil {
		return fmt.Errorf("runtime directory: %w", err)
	}
	runtime.Close()
	if m.IndexEnabled {
		if len(m.Roots) != 1 {
			return errors.New("persistent Windows profile currently requires exactly one approved root")
		}
		store, err := windowssecure.Open(m.StoreRoot, true)
		if err != nil {
			return fmt.Errorf("store directory: %w", err)
		}
		store.Close()
		if strings.EqualFold(filepath.Clean(m.StoreRoot), filepath.Clean(m.RuntimeDir)) {
			return errors.New("store and runtime directories must differ")
		}
	} else if m.StoreRoot != "" {
		return errors.New("store_root requires explicit index_enabled consent")
	}
	for index := range m.Roots {
		root := &m.Roots[index]
		if !filepath.IsAbs(root.Path) {
			return errors.New("approved root must be absolute")
		}
		if err := windowssecure.RejectReparsePath(root.Path); err != nil {
			return err
		}
		canonical, err := exactDirectory(root.Path)
		if err != nil {
			return err
		}
		if contains(canonical, m.RuntimeDir) || (m.StoreRoot != "" && contains(canonical, m.StoreRoot)) {
			return errors.New("approved source root contains Engine state")
		}
		object, err := RootObjectID(canonical)
		if err != nil {
			return err
		}
		if root.ObjectID != object {
			return fmt.Errorf("approved root %q identity changed", root.ID)
		}
		root.Path = canonical
	}
	_, err = m.Guard()
	return err
}

func (m WindowsManifest) Guard() (*sandbox.Guard, error) {
	approved := make([]sandbox.ApprovedRoot, 0, len(m.Roots))
	for _, root := range m.Roots {
		approved = append(approved, sandbox.ApprovedRoot{ID: root.ID, Path: root.Path, ObjectID: root.ObjectID, Exclusions: root.Exclusions})
	}
	return sandbox.NewApproved(m.DeploymentID, approved)
}

func (m WindowsManifest) RootSpecs() []api.RootSpec {
	result := make([]api.RootSpec, 0, len(m.Roots))
	for _, root := range m.Roots {
		result = append(result, api.RootSpec{ID: root.ID, Path: root.Path})
	}
	return result
}

func LoadWindows(path string) (WindowsManifest, error) {
	var manifest WindowsManifest
	payload, err := windowssecure.Read(path, maxManifest)
	if err != nil {
		return manifest, err
	}
	decoder := json.NewDecoder(bytes.NewReader(payload))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&manifest); err != nil {
		return manifest, err
	}
	var trailing any
	if err := decoder.Decode(&trailing); err != io.EOF {
		return manifest, errors.New("Windows manifest contains trailing data")
	}
	return manifest, manifest.Validate()
}
