package catalog

import (
	"filemanager/engine/api"
	"filemanager/engine/internal/identity"
)

const LightModeImmediateChildren uint32 = 65_536

// ObservationWatermark identifies the native observation boundary incorporated
// by a generation. M1 full scans leave Adapter and Opaque empty; event adapters
// must not fabricate a replay cursor where the platform supplies none.
type ObservationWatermark struct {
	Adapter  string
	Opaque   string
	Complete bool
	Gap      bool
}

// Volume is the lightweight routing/availability object above approved-root
// shards. Native is adapter evidence (currently st_dev or volume serial), not a
// promise of eternal cross-mount identity.
type Volume struct {
	Key       identity.VolumeKey
	MountPath string
	Available bool
	Watermark ObservationWatermark
}

type RootManifest struct {
	Root               api.RootSpec
	Volume             Volume
	Generation         api.Generation
	Stale              bool
	ObjectCount        uint64
	BindingCount       uint64
	LightModeThreshold uint32
	LightDirectories   uint64
}

func (p Projection) Manifest() RootManifest {
	manifest := RootManifest{
		Root: p.Spec, Generation: p.Generation, Stale: p.Stale,
		LightModeThreshold: LightModeImmediateChildren,
	}
	if p.Shard != nil {
		rootObject := p.Shard.objects[0]
		manifest.Volume = Volume{
			Key: rootObject.Identity.VolumeKey(), MountPath: p.Spec.Path, Available: true,
		}
		manifest.ObjectCount = uint64(p.Shard.ObjectCount())
		manifest.BindingCount = uint64(p.Shard.Len())
		manifest.LightDirectories = p.Shard.LightDirectoryCount()
	}
	return manifest
}
