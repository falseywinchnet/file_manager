# Results ledger

Store compact, reviewable experiment manifests and summaries here. Each result
names the raw-output location and checksum. Failed candidates remain under
`rejected/` with the mechanism, strongest tested form, gain, regression, and the
new evidence required for reconsideration.

Current M2 component summaries include `M2_DELTA_CANDIDATE_001.md` for the
standalone run encoding and `M2_OVERLAY_CANDIDATE_001.md` for one checked base
plus one checked run. `M2_MULTI_RUN_CONSOLIDATION_001.md` measures a checked run
chain and one net-run rewrite. `M2_REPEATED_CONSOLIDATION_002.md` rejects
indefinite eight-run consolidation and same-trigger full-base replacement after
their measured cumulative write amplification exceeds the gate. None is a
live-format admission record.

`M2_TIERED_COHORT_COMPACTION_003.md` retains cohort-only tiered compaction after
22 epochs measure about 2.00x with eight visible runs. It separately rejects
the current heap overlay at 10,000-change scale and requires disk-indexed query
and streaming-compactor controls before live admission.

`M2_TIERED_INDEXED_COMPACTION_004.md` implements those two controls. Indexed
write amplification, retained memory, exact state, and the warm relative-query
gate pass; cold/background-overlap evidence and the long-horizon policy remain
open.

`M2_TIERED_MANIFEST_RECOVERY_005.md` records the isolated `TIERED.*` atomic
publication, fallback, repair, subprocess-exit, short-write, schema, and
pin-aware reclamation campaign. It is not live-service manifest admission.

`M5_KOLMOGROV_HISTORY_TUPLE_001.md` records the first disabled native transfer
of Kolmogrov's sealed coupled-history filename guidance. It is component
evidence, not production fuzzy-search admission.
`M5_KOLMOGROV_SERVICE_DOGFOOD_002.md` records the private exact-generation
lease, mutation/restart/corruption sandbox, and a preliminary 10,000-file
same-corpus service measurement. It does not admit a public fuzzy method or
similarity manifest.

`M0_SERVICE_LIFECYCLE_DOGFOOD_001.md` records lifecycle/configuration/status
projection cost, stale-safe apply conformance, unchanged-reconcile no-write
suppression, and clean drain behavior in a disposable persistent sandbox. It
is not native supervisor or power-loss evidence.

`M4_BACKGROUND_CURRENTNESS_001.md` records the portable bounded observation
state machine, native macOS FSEvents sandbox dogfood, scan-overlap/gap
conformance, retained storm batching, and zero-write quiet interval. Its
watermark is deliberately volatile; it is not manifest or cross-platform
admission.
