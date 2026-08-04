# Requirements and open-decision ledger

Date: 2026-08-03.

## GIVEN

| ID | Requirement or exclusion | Consequence |
|---|---|---|
| PR-G01 | Rust is preferred for hostile plugin wrappers. | Rust supervisor/worker candidates are admitted; Rust alone is not a sandbox. |
| PR-G02 | Third-party/native preview execution is never in the GUI process. | Every preview crosses a process or stronger isolation boundary. |
| PR-G03 | Preview crash must not crash File Manager. | The GUI owns no plugin stack frames, pointers, callbacks, or destructors. |
| PR-G04 | Plugins may provide previews, thumbnails, virtual systems, and search results. | These four extension classes form the current maximum public surface. |
| PR-G05 | Plugins may not replace, insert, or restyle core controls. | Results are data, validated pixels/surfaces, or fixed-schema fields rendered by the host. |
| PR-G06 | Non-PNG media decoders are plugins; internal GUI/theme PNG remains core. | Decoder attack surface is isolated except for the separately audited PNG path. |
| PR-G07 | File Manager is local-first and has no core web search/store/discovery. | Network and catalogue mechanisms cannot be ambient supervisor services. |
| PR-G08 | The internal handler registry does not change OS defaults. | Handler proposals are File Manager-local data. Unknown types may invoke an OS chooser. |
| PR-G09 | Thumbnails exist only for admitted indexed trees. | Thumbnail jobs require a host-issued indexed-tree attestation/grant. |
| PR-G10 | Exact filesystem records remain authoritative. | Plugin identities and ranks never replace host file identity or exact metadata. |
| PR-G11 | No production telemetry or automatic crash upload. | Diagnostics are local; manual export is explicit and redacted. |
| PR-G12 | Plugins expose information only; file mutation authority is not admitted. | Protocol has no generic filesystem write or shell-command escape hatch. |

## OBSERVED source constraints

- Root `planning/PRODUCT_NEGATIVE.md` admits exactly four plugin classes and
  rejects uncontrolled shell-style expansion.
- Root `planning/ARCHITECTURE_INPUTS.md` permits a future in-application
  catalogue only after a separate trust, signing, network, and capability
  design.
- Root `planning/EVIDENCE_REGISTER.md` identifies the plugin threat model as an
  evidence gap; no existing code settles it.

## Terms

- **Supervisor:** trusted Rust process/library that authenticates callers,
  validates packages, creates workers, enforces grants, and validates results.
- **Worker:** killable process containing a guest runtime or native adapter.
- **Guest:** plugin-owned code and assets. It is always untrusted, even when
  signed; a signature establishes publisher/package identity, not safety.
- **Capability:** an operation plus scope, limits, provenance, expiry, and grant
  source. It is not a boolean permission name.
- **Job:** one bounded invocation with immutable input identity, deadline,
  budgets, cancellation token, and output contract.
- **Derived data:** thumbnails, extracted fields, previews, indexes, and caches
  attributable to plugin ID plus plugin/extractor version.

## Questions that block accepted architecture

| Root IDs | Unresolved choice | Required evidence/owner answer |
|---|---|---|
| C007, O011, V007 | Binary portability and compatibility horizon | Decide source SDK vs wire compatibility promises after two-version conformance. |
| D006, O003 | Per-call, per-plugin, or pooled worker topology | Measure startup, steady memory, crash containment, and state leakage. |
| D012 | Crash UX and disable threshold | Architect chooses visible fallback and quarantine policy. |
| G008–G009 | Plugin columns/sort keys | Not admitted by current four-class list; require explicit expansion. |
| J014–J015 | Mixed search providers and segregation | Define provenance UI and local/remote distinction before admitting federation. |
| M003–M010 | Handler/icon precedence and cache invalidation | File Manager registry semantics must be closed first. |
| N003, N008–N015 | Preview payloads, caching, sampling, active content | Run decoder surface experiments and obtain product answers. |
| O004–O008 | Network and grant UX | Default remains no network; any grant requires explicit owner decision. |
| O005, O014 | Catalogue/update ownership | Compare manual packages, core-managed, and OS/package-manager paths. |
| O006, O016 | Development mode, signatures, reproducibility | Define publisher and local-development trust policies. |
| O012 | Bundled models/runtimes | Bound executable content, licensing, storage, and resource risks. |
| O013 | Resource budgets | Derive from measured preview/search workloads and reference machines. |
| O015 | Uninstall cleanup | Decide retention/export rules for derived user value. |
| S006, S010, S014 | Adversaries, audit depth, signing | Grand architect approval plus red-team results. |
| U007 | Plugin settings | Candidate is supervisor-owned namespaced schema; not yet decided. |
| W010 | First-party powers | Candidate answer is no implicit privilege; any exception needs its own signed policy. |

## Explicit non-goals for the first implementation program

- no plugin store, network client, updater, or publisher account;
- no plugin-defined GUI, CSS, HTML, native window, or control callback;
- no generic command/automation plugin and no file mutation;
- no stable 1.0 ABI promise;
- no semantic model runtime or remote filesystem implementation;
- no claim that Rust memory safety contains a malicious native library.

