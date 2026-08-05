# ORC-HIV / ORC-SET: hives and settings contracts

Status: **paper outline**.

The Oracle owns storage and publication. Plugins receive capability APIs, never
database files, page formats, or writable engine handles.

Required authority classes:

- explicit user-authored semantic memory;
- AI/plugin-proposed semantic facts awaiting the applicable authority rule;
- disposable plugin-derived provider records;
- operational registries and settings.

These classes use distinct namespaces, provenance, retention, migration, sync,
quota, conflict, and erase rules. A derived claim cannot overwrite a user fact
or exact filesystem field.

Provider deposit follows declare schema → begin generation → emit bounded
records → validate → publish → retire. Query never observes half a generation.
Settings use declarative schemas and supervisor-owned values; plugins never
receive arbitrary settings paths or another namespace.

See `../../planning/HIVE_AND_SETTINGS_MODEL.md` for the lifecycle program.
