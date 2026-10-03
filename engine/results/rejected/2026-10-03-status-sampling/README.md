# Mixed catalogue and observation status

**OBSERVED native failure:** PR31 push source e81575fcbe445bb83be49b1d5da36fe89ed2ee3d, Linux job111195177449 in run37120371055 failed TestBackgroundObservationBaselineGapAndStopState at background_test.go84 before building the GUI change.

[Run and original log](https://github.com/falseywinchnet/file_manager/actions/runs/37120371055/job/111195177449).
`LinuxFailureExcerpt.log` preserves the failing test and full contradictory status record. Catalogue generation1 contains unindexed stale root generation0, while background currentness is current_volatile with observed/reconciled watermark10. Other native runs passing does not erase this failure.

**OBSERVED source ordering:** Status captures the catalogue before decorateStatus separately samples the background controller. A completed reconciliation between those reads can combine old catalogue state with new observation progress. Persistent status also releases the reader lock before decoration. The existing wait predicate is retained; increasing sleeps or weakening its readiness assertion is not the repair.

**CANDIDATE repair:** capture immutable catalogue/durable metadata and background state under compatible short-lived locks, then project outside locks with configuration roots and ingestion mode from that capture. No scan/admin wait, retry loop or public schema change. Native acceptance and deterministic regression evidence are pending. Root-policy publication followed by later background invalidation is a separate existing transition; this record does not assert globally atomic administrative status.
