//go:build windows

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

	"golang.org/x/sys/windows/registry"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/windowssecure"
)

const WindowsManifestSchema string = "fileman.engine.windows-deployment.v1"

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
	var key registry.Key = 0
	var err error = nil
	key, err = registry.OpenKey(registry.LOCAL_MACHINE, `SOFTWARE\Microsoft\Cryptography`, registry.QUERY_VALUE|registry.WOW64_64KEY)
	if err != nil {
		return "", err
	}
	defer key.Close()
	var value string = ""
	value, _, err = key.GetStringValue("MachineGuid")
	if err != nil {
		return "", err
	}
	value = strings.TrimSpace(value)
	if value == "" {
		var failure error = errors.New("Windows host identity is empty")
		return "", failure
	}
	var result string = strings.ToLower(value)
	return result, nil
}

func (m *WindowsManifest) Validate() error {
	if m.Schema != WindowsManifestSchema || m.DeploymentID == "" || m.DeploymentID == "development_sandbox" {
		var failure error = errors.New("invalid Windows deployment schema or identity")
		return failure
	}
	var sid string = ""
	var err error = nil
	sid, err = windowssecure.CurrentSID()
	if err != nil {
		return err
	}
	var host string = ""
	host, err = WindowsHostID()
	if err != nil {
		return err
	}
	if m.UserSID != sid || m.HostID != host {
		var failure error = errors.New("Windows manifest belongs to another host or user")
		return failure
	}
	if !filepath.IsAbs(m.RuntimeDir) {
		var failure error = errors.New("runtime directory must be absolute")
		return failure
	}
	var runtime *os.File = nil
	runtime, err = windowssecure.Open(m.RuntimeDir, true)
	if err != nil {
		var failure error = fmt.Errorf("runtime directory: %w", err)
		return failure
	}
	runtime.Close()
	if m.IndexEnabled {
		if len(m.Roots) != 1 {
			var failure error = errors.New("persistent Windows profile currently requires exactly one approved root")
			return failure
		}
		var store *os.File = nil
		var err error = nil
		store, err = windowssecure.Open(m.StoreRoot, true)
		if err != nil {
			var failure error = fmt.Errorf("store directory: %w", err)
			return failure
		}
		store.Close()
		if strings.EqualFold(filepath.Clean(m.StoreRoot), filepath.Clean(m.RuntimeDir)) {
			var failure error = errors.New("store and runtime directories must differ")
			return failure
		}
	} else if m.StoreRoot != "" {
		var failure error = errors.New("store_root requires explicit index_enabled consent")
		return failure
	}
	{
		var index int = 0
		for index = range m.Roots {
			var root *Root = &m.Roots[index]
			if !filepath.IsAbs(root.Path) {
				var failure error = errors.New("approved root must be absolute")
				return failure
			}
			{
				var err error = nil
				err = windowssecure.RejectReparsePath(root.Path)
				if err != nil {
					return err
				}
			}
			var canonical string = ""
			var err error = nil
			canonical, err = exactDirectory(root.Path)
			if err != nil {
				return err
			}
			if contains(canonical, m.RuntimeDir) || (m.StoreRoot != "" && contains(canonical, m.StoreRoot)) {
				var failure error = errors.New("approved source root contains Engine state")
				return failure
			}
			var object string = ""
			object, err = RootObjectID(canonical)
			if err != nil {
				return err
			}
			if root.ObjectID != object {
				var failure error = fmt.Errorf("approved root %q identity changed", root.ID)
				return failure
			}
			root.Path = canonical
		}
	}
	_, err = m.Guard()
	return err
}

func (m WindowsManifest) Guard() (*sandbox.Guard, error) {
	var count int = len(m.Roots)
	var approved []sandbox.ApprovedRoot = make([]sandbox.ApprovedRoot, count)
	var index int = 0
	for index = 0; index < count; index++ {
		var root Root = m.Roots[index]
		approved[index] = sandbox.ApprovedRoot{ID: root.ID, Path: root.Path, ObjectID: root.ObjectID, Exclusions: root.Exclusions}
	}
	var guard *sandbox.Guard = nil
	var failure error = nil
	guard, failure = sandbox.NewApproved(m.DeploymentID, approved)
	return guard, failure
}

func (m WindowsManifest) RootSpecs() []api.RootSpec {
	var count int = len(m.Roots)
	var result []api.RootSpec = make([]api.RootSpec, count)
	var index int = 0
	for index = 0; index < count; index++ {
		var root Root = m.Roots[index]
		result[index] = api.RootSpec{ID: root.ID, Path: root.Path}
	}
	return result
}

func LoadWindows(path string) (WindowsManifest, error) {
	var manifest WindowsManifest = WindowsManifest{}
	var payload []byte = nil
	var err error = nil
	payload, err = windowssecure.Read(path, maxManifest)
	if err != nil {
		return manifest, err
	}
	var input *bytes.Reader = bytes.NewReader(payload)
	var decoder *json.Decoder = json.NewDecoder(input)
	decoder.DisallowUnknownFields()
	{
		var err error = nil
		err = decoder.Decode(&manifest)
		if err != nil {
			return manifest, err
		}
	}
	var trailing any = nil
	{
		var err error = nil
		err = decoder.Decode(&trailing)
		if err != io.EOF {
			var failure error = errors.New("Windows manifest contains trailing data")
			return manifest, failure
		}
	}
	var failure error = manifest.Validate()
	return manifest, failure
}
