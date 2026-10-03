package service

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"sort"

	"filemanager/engine/api"
)

// Version reports build, protocol, contract, feature, instance, and capability
// identity without exposing storage representation.
func (s *Service) Version() api.VersionInfo {
	lifecycle, _ := s.lifecycleStatus()
	features := []string{"exact-reference", "integrity", "root-policy", "scan-reconcile", "live-query", "service-lifecycle", "effective-configuration"}
	if s.Persistent() {
		features = append(features, "immutable-generation-v1")
	}
	if s.backgroundIngestionMode() != "manual_reconcile" {
		features = append(features, "background-observation-experimental")
	}
	return api.VersionInfo{
		Component: api.EngineComponentName, BuildVersion: api.EngineBuildVersion,
		Protocol: api.ProtocolVersion, InstanceID: lifecycle.InstanceID,
		ContractFamilies: []string{"ORC-LIF-001", "ORC-ENG-001", "ORC-ENG-002", "ORC-ENG-003", "ORC-ENG-004"},
		Features:         features, Capabilities: s.capabilities(),
	}
}

// Configuration returns the enacted configuration and a stable digest. Root
// currentness is health state and is intentionally excluded from that digest.
func (s *Service) Configuration() api.EffectiveConfiguration {
	snapshot := s.store.Snapshot()
	roots := make([]api.RootSpec, 0, len(snapshot.Roots))
	for _, projection := range snapshot.Roots {
		roots = append(roots, projection.Spec)
	}
	sort.Slice(roots, func(i, j int) bool { return roots[i].ID < roots[j].ID })
	persistentPolicy := false
	if s.durable != nil {
		s.readerMu.RLock()
		persistentPolicy = len(roots) == 1 && s.reader != nil && s.reader.Root() == roots[0]
		s.readerMu.RUnlock()
	}
	return s.effectiveConfiguration(roots, persistentPolicy)
}

func (s *Service) effectiveConfiguration(roots []api.RootSpec, persistentPolicy bool) api.EffectiveConfiguration {
	var ingestionMode string = s.backgroundIngestionMode()
	var configuration api.EffectiveConfiguration = s.effectiveConfigurationWithIngestion(roots, persistentPolicy, ingestionMode)
	return configuration
}

// effectiveConfigurationWithIngestion retains the supplied root slice in the
// result. The caller supplies owned roots and a mode from the same capture.
func (s *Service) effectiveConfigurationWithIngestion(roots []api.RootSpec, persistentPolicy bool, ingestionMode string) api.EffectiveConfiguration {
	var configuration api.EffectiveConfiguration = api.EffectiveConfiguration{
		Schema: api.EngineConfigurationSchema, SchemaMajor: 0, SchemaMinor: 1,
		Deployment: s.guard.Deployment(), Sandboxed: s.guard.Sandboxed(), Persistent: s.Persistent(),
		StoreOutsideRoots: s.durable == nil || !anyRootContains(roots, s.durableDirectory),
		IngestionMode:     ingestionMode, RootPolicy: roots, RootPolicyPersistent: persistentPolicy,
	}
	var digestMaterial api.EffectiveConfiguration = configuration
	digestMaterial.Digest = ""
	digestMaterial.RootPolicyPersistent = false
	var encoded []byte = nil
	encoded, _ = json.Marshal(digestMaterial)
	var digest [sha256.Size]byte = sha256.Sum256(encoded)
	configuration.Digest = hex.EncodeToString(digest[:])
	return configuration
}

func anyRootContains(roots []api.RootSpec, path string) bool {
	for _, root := range roots {
		if pathContains(root.Path, path) {
			return true
		}
	}
	return false
}
