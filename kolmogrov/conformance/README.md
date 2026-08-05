# Kolmogrov conformance artifacts

Status: **candidate cross-project fixtures; Orchestrator reconciliation controls
canonical contract status**.

This directory contains machine-readable reference vectors for proposed
Kolmogrov transfer profiles. A fixture may support a fake or experimental
adapter without promoting its family to production. The corresponding
project-local negotiation reply and profile document state the authority,
semantic scope, and unresolved fields.

- `orc_kol_001/lab_profile_001.json` — binary deletion-certificate laboratory
  profile from `docs/ORC_KOL_LAB_PROFILE_001.md`.
- `orc_kol_001/history_hash_usage_guidance_001.json` — sealed candidate
  defaults and mandatory constraints for disabled filename-history dogfooding;
  it is guidance from `docs/HISTORY_HASH_ENGINE_CONTRACT_001.md`, not an
  accepted transfer profile or architecture decision.
