# ORC-HIV / ORC-SET: hives and settings contracts

Status: **`ORC-SET-001` semantic v1 frozen by ADR-018; provider and semantic
hives remain stubbed**.

Orchestrator owns storage and publication. Plugins receive capability APIs, never
database files, page formats, or writable engine handles.

Candidate future authority classes retained for the architect's fact design:

- explicit user-authored semantic memory;
- AI/plugin-proposed semantic facts awaiting the applicable authority rule;
- disposable plugin-derived provider records;
- operational registries and settings.

These classes use distinct namespaces, provenance, retention, migration, sync,
quota, conflict, and erase rules. A derived claim cannot overwrite a user fact
or exact filesystem field.

No semantic-fact operation is currently admitted. A status-only CLI call reports
the system as stubbed; code must not infer create/propose/query operations from
this planning vocabulary.

Future provider deposit is expected to follow declare schema → begin generation → emit bounded
records → validate → publish → retire. Query never observes half a generation.
Plugins never receive arbitrary settings paths or another namespace.

## ORC-SET-001 version 1.0

Authority: Orchestrator owns schemas, committed values, revisions, validation,
recovery provenance, and audit identity. File Manager and CLI are equivalent
local-user clients. The service has no UI schema and does not return arbitrary
controls.

Operations:

| Method | Result |
|---|---|
| `orchestrator.settings.schema` | immutable bounded schema snapshot |
| `orchestrator.settings.snapshot` | immutable revisioned value snapshot |
| `orchestrator.settings.apply` | one optimistic all-or-nothing mutation transaction |

The schema snapshot includes `schema_revision` and at most 256 fields. Every
field has a stable ID, namespace, owner, candidate presentation tab, scalar
type, default, bounds/allowed values, localization keys, sensitivity, restart
effect, availability, and validation text. Version 1 admits Boolean, bounded
unsigned integer, and bounded UTF-8 string/choice values. It admits no secret
value; a future secret field contains only an opaque platform handle.

The value snapshot contains the exact schema revision, monotonic value revision,
recovery provenance (`primary`, `recovered_previous`, or `new_defaults`), and
one typed value per schema field. Defaults used for a missing new-format store
are explicit `new_defaults`; malformed existing stores never become defaults.

An apply request contains `expected_revision` and 1–64 unique mutations. Each
mutation names a field and exactly one of `set` or `reset_to_default`. Unknown,
unavailable, sensitive, duplicate, wrong-type, out-of-bounds, or invalid-choice
mutations make the entire transaction `invalid`. A mismatched revision makes it
`stale`. Deadlines are checked before validation and before durable publication;
version 1 has no mid-fsync cancellation promise.

A successful result returns the complete committed snapshot, changed field IDs,
whether any field requires application/service restart, and a stable monotonic
`settings-audit-<sequence>` identity. It never returns a secret or storage path.
Reset is the same transaction with `reset_to_default`; migration effect is the
schema revision and recovery provenance in every snapshot.

Physical storage follows ADR-018. The version-1 limits are 128 KiB encoded
snapshot, 256 fields, 64 mutations/transaction, 4 KiB string, and one writer
inside the user daemon. Provider hives, semantic memory, and operational
registries do not share this file by implication. The Unix/macOS projection
opens existing snapshots with `O_NOFOLLOW`, requires exact same-user/private
file and directory modes, publishes with create-new temporary files plus atomic
rename and directory fsync, and has a symlink-refusal regression test.

See `../../planning/HIVE_AND_SETTINGS_MODEL.md` for the lifecycle program.
