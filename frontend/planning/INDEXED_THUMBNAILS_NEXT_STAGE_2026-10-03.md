# Indexed thumbnails: source trace and next complete stage

Status: **CANDIDATE; no new contract, decoder, grant or provider admitted**.
The visible Details sibling performed a read-only source trace. Root checked
the owner constraints, current C++ projection, Engine identity and registry.
This record does not close the owner's missing-thumbnail acceptance failure.

## Required behavior and observed gaps

**GIVEN:** `visual/DESIGN_DNA_006.md` DNA-O13 and the recovered ceremonial verdicts
require index-supplied thumbnails; unindexed locations retain material icons.
ADR-001 makes projections disposable and imposes placement, revocation and
light-mode rules. Indexing cannot become a prerequisite for navigation.

**OBSERVED:** Engine `api/types.go` already represents root/object/platform key,
incarnation, stored metadata and catalogue generation. `internal/service/query.go`
has exact-record inspection. Inspection of a catalogue record is not proof that
current bytes match it. The current C++ `SearchResultInfo` retains name/path/kind/
unsigned size/unavailable, omitting per-record identity, incarnation and stored
mtime. `frontend/src/search_preparation.cpp` observes current filesystem facts;
it does not establish equivalence to the indexed record.

Windows Engine identity currently uses `ByHandleFileInformation`'s 64-bit index
and volume serial with creation-time incarnation. Frontend identity includes
128-bit file identity. Paths, truncation and serialized-string equality cannot
bridge those representations. Preserve provider observations separately until
an explicit equivalence/opaque binding contract is selected.

**OBSERVED:** ObjectView already supports per-item image keys, and ImageList and
Window admit keyed images and PNG/BGRA data. The application assigns house art.
Visible-row calculations are private; efficient thumbnail scheduling must not
duplicate those calculations in frontend. `ORC-PLG-003` and its supervision
dependencies remain stubs. ADR-020's bounded first-party PNG preview is not
already a background thumbnail extraction/cache decision.

## Candidate vertical implementation

1. Retain typed source object/revision evidence through existing C++ search
   projection without granting mutation or thumbnail authority. Reconcile the
   additive projection against `ORC-ENG-001` and existing wire fixtures.
2. Negotiate a bounded indexed-object lookup for visible directory entries,
   including policy revision, indexed/unindexed/excluded/light-mode/stale states,
   and an exact producer-verifiable source binding. Search hits alone are not a
   directory thumbnail eligibility API.
3. Decide a first-party producer/execution boundary and disposable projection
   store. PNG is a candidate initial codec because existing preview support is
   available; it is not selected here. Define input/output/memory/time bounds,
   cache placement, producer/profile version, root revocation and rebuild.
   Reconcile content-read/projection policy explicitly with indexing consent;
   do not invent a new user prompt or assume a new grant in this record.
4. Add a negotiated bounded visible-item observation to GUI.Forms, then request
   only eligible visible thumbnails. Apply results only to matching navigation,
   indexed binding and current source revision. Retire image resources on
   eviction/revocation. Unavailable/rejected results keep ordinary icons.
5. Prove the complete path with real indexed PNG pixels; no source reads or
   thumbnails for unindexed/revoked locations; replacement/rename and stale
   refusal; light-mode suppression; bounded scrolling/cancellation and cache
   eviction; native application images and packaged dependencies on all targets.

The Orchestrator negotiation must select whether this is a first-party profile
or an admitted `ORC-PLG-003` extension before production adapters freeze. A
frontend-owned competing supervisor or live per-folder thumbnail scan would
miss the requested end state. This candidate needs a concrete contract/decision
record and measured implementation; the GUI painting hook alone is insufficient.

Follow-on assignments must name `planning/PROGRAMMING_HOUSE_STYLE.md` and review
the complete authored C++/Go/Rust scope: explicit types, named execution and
retained state, identity/revision distinctions, ownership and borrow lifetimes,
dimension/byte conversions, cancellation/failure publication, stable repeated
work and reusable storage. The trace does not certify inspected legacy files.
