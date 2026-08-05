//go:build !darwin || !cgo

package fsevents

import (
	"filemanager/engine/api"
	"filemanager/engine/internal/observation"
)

func New([]api.RootSpec, Config) (observation.Adapter, error) {
	return nil, ErrUnavailable
}
