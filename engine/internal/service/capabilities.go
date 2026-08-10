package service

import "filemanager/engine/api"

func (s *Service) capabilities() []api.CapabilityStatus {
	persistentState := api.CapabilityUnavailable
	persistentReason := "persistent store is not enabled for this process instance"
	if s.Persistent() {
		persistentState, persistentReason = api.CapabilityAvailable, ""
	}
	rootRevision := "development-sandbox-v0"
	transportState, transportReason := api.CapabilityDeferred, "installed authenticated endpoint is not active in development sandbox mode"
	launchdState, launchdReason := api.CapabilityUnavailable, "launchd projection is admitted only by the M4 dogfood deployment"
	if !s.guard.Sandboxed() {
		rootRevision = "host-bound-manifest-v1"
		transportState, transportReason = api.CapabilityAvailable, ""
		if s.guard.Deployment() == "m4-dogfood" {
			launchdState, launchdReason = api.CapabilityAvailable, ""
		}
	}
	return []api.CapabilityStatus{
		{ID: "engine.lifecycle", State: api.CapabilityAvailable, Revision: "0.1"},
		{ID: "engine.configuration.inspect", State: api.CapabilityAvailable, Revision: "0.1"},
		{ID: "engine.exact.catalogue", State: api.CapabilityAvailable, Revision: "reference-v1"},
		{ID: "engine.exact.query", State: api.CapabilityAvailable, Revision: "engine.v0"},
		{ID: "engine.live.query", State: api.CapabilityAvailable, Revision: "name-path-v0.1"},
		{ID: "engine.ordered_metadata", State: api.CapabilityAvailable, Revision: "reference-v1"},
		{ID: "engine.root_policy", State: api.CapabilityAvailable, Revision: rootRevision},
		{ID: "engine.scan.reconcile", State: api.CapabilityAvailable, Revision: "metadata-full-scan-v0"},
		{ID: "engine.durable.immutable_generation", State: persistentState, Revision: "v1", Reason: persistentReason},
		{ID: "engine.delta_publication", State: api.CapabilityExperimental, Revision: "component-only", Reason: "not admitted to the live manifest or service"},
		{ID: "engine.lexical.index", State: api.CapabilityUnavailable, Reason: "M3 lexical dictionary and postings are not implemented"},
		{ID: "engine.content_feature.intake", State: api.CapabilityDeferred, Reason: "anchored fragment/provider input contract is not reconciled"},
		{ID: "engine.fragment.descriptor", State: api.CapabilityDeferred, Reason: "content-fragment boundaries, anchors, privacy policy, and descriptor lifecycle are not reconciled"},
		{ID: "engine.similarity.fixed_width", State: api.CapabilityExperimental, Revision: "history-tuple-0.1", Reason: "candidate channel is not admitted to the public planner"},
		{ID: "engine.background.observation", State: api.CapabilityExperimental, Revision: "portable-coalescer-0.2+macos-fsevents-0.2+windows-rdcw-0.1", Reason: "bounded macOS and Windows native adapters exist, but coverage is incomplete, the watermark is volatile, Linux is absent, and Windows has compatibility-only validation"},
		{ID: "engine.status.subscribe", State: api.CapabilityNegotiating, Reason: "bounded replay and overflow fixtures remain red"},
		{ID: "engine.transport.framed_local", State: transportState, Revision: "ENG1-v1", Reason: transportReason},
		{ID: "engine.supervisor.launchd", State: launchdState, Revision: "m4-dogfood-v1", Reason: launchdReason},
		{ID: "contract.ORC-LIF-001", State: api.CapabilityAvailable, Revision: "semantic-v0.1"},
		{ID: "contract.ORC-ENG-001", State: api.CapabilityAvailable, Revision: "semantic-v0.1"},
		{ID: "contract.ORC-ENG-002", State: api.CapabilityAvailable, Revision: "semantic-v0.1"},
		{ID: "contract.ORC-ENG-003", State: api.CapabilityAvailable, Revision: "snapshot-v0.1"},
		{ID: "contract.ORC-ENG-004", State: api.CapabilityAvailable, Revision: "semantic-v0.1"},
	}
}
