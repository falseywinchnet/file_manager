# Selected previews and indexed thumbnails: next slice

2026-10-04 UTC. Status: **CANDIDATE for contract negotiation and experiments**.
This record does not admit a decoder, activate ORC-PLG, change ADR-020, or freeze
an adapter. It turns the missing product behavior into a bounded comparison.

## Product outcome and existing authority

The proposed first visible outcome is an orientation-correct JPEG preview and
a first-page PDF preview in the existing inspector. A later step with its own
accepted contract/profile supplies revision-bound thumbnails for indexed objects in the visible
folder range. Folder navigation remains usable with no index or Orchestrator.

| Status | Constraint and source |
|---|---|
| GIVEN | The owner reports failed macOS previews across multiple types and absent thumbnails; useful previews and refinement are active goal work. The exact personal-file corpus is unknown. |
| DECIDED | [ADR-020](../decisions/ADR-020-FILE-MANAGER-TRUSTED-LOCAL-ACTIONS-PREVIEWS-AND-PICKER-PACKAGE.md) currently admits UTF-8 and PNG built-ins. Its reversal path requires independent bounds and evidence for additional formats. |
| GIVEN | [Design DNA 006, DNA-O13](../frontend/planning/visual/DESIGN_DNA_006.md) permits object thumbnails only when supplied by the index. Unindexed locations keep type/material icons. Selected-file preview is a separate interaction. |
| GIVEN | [Product exclusions](PRODUCT_NEGATIVE.md) prohibit in-process third-party/native preview execution. [Handler requirements](ARCHITECTURE_INPUTS.md#handlers-and-previews) require hard process/container isolation when reusing native providers. |
| OBSERVED | [ORC-PLG](../orchestrator/spec/contracts/PLUGIN_AND_CAPABILITIES.md) has no worker, grant or preview operation in the bootstrap. A built-in rendering profile cannot pretend those operations already exist. |
| OBSERVED | [Source-record preservation](../orchestrator/spec/FRONTEND_SEARCH_SOURCE_RECORDS.md) retains Engine evidence but expressly does not establish thumbnail eligibility or equivalence to current native identity. Windows 64-bit and 128-bit file IDs are not interchangeable. |
| CANDIDATE | A media hive is an option in [search system options](search/SEARCH_SYSTEM_OPTIONS.md), not the selected thumbnail store. This slice does not select it. |

The owner's direct-worktree/no-sandbox development direction is not evidence
that decoder failure may take down the application. Conversely, the native
provider requirement does not make every first-party utility a plugin.

## Compare these implementations before selecting dependencies

| Candidate | Useful property to test | Failure or cost that can reject it |
|---|---|---|
| Closed first-party rendering helper, supervised by Orchestrator, with portable codecs | One format/output profile and comparable behavior on all three platforms; process death can be contained | Supervisor start gate, scoped I/O, portable resource enforcement, dependency maintenance and package size may outweigh the benefit |
| Platform preview adapters behind the same bounded process contract | May reuse mature format coverage and avoid shipping some codecs | Output/orientation/color differences, extension loading, unbounded native provider behavior, unavailable host services or unenforceable descendant limits |
| Additional trusted built-in first-party decoder with independently demonstrated bounds | Small startup and IPC cost | In-process failure cannot be contained; admission cannot be inferred from current PNG support; unsuitable for third-party/native preview execution |

**CANDIDATE recommendation:** evaluate the closed helper first for JPEG and PDF,
without dynamic plugin discovery, installation, arbitrary commands, controls or
network access. This is a proposed narrow Orchestrator capability profile, not
permission to implement the entire plugin runtime. Keep the existing UTF-8/PNG
path available during the experiment. Do not migrate it merely for symmetry.
Codec licensing, pinned versions, malformed-input handling, orientation and
color behavior must be compared from primary sources and fixtures before a
dependency decision. No codec has been selected in this record.

## Proposed data and ownership contract

The request owns a nonce, output specification, format profile/version,
deadline, cancellation identity and immutable source identity/revision evidence.
It carries a supervisor-issued read capability rather than treating a path as
authority. The exact descriptor/handle/stream projection is a negotiation item;
raw Go, Rust or C++ object layouts never cross the process boundary.

The response echoes the nonce and source evidence and owns a bounded raster or
an explicit outcome. Raster metadata names width, height, stride, pixel format,
orientation applied, color policy, provider build and page number where relevant.
The receiver validates every count/product and the exact payload extent before
admitting pixels to the public GUI.Forms raster interface. Successful IPC is
not evidence of a displayed preview.

The worker owns decoding scratch. Orchestrator owns process lifetime, capability
revocation and result validation. The frontend owns presentation demand,
selection generations and retirement. GUI.Forms owns retained image display.
No retained file/view borrow outlives its owner; every deferred object has an
explicit owner. Decoder code never receives a frontend control or callback.

For indexed thumbnails, Engine owns indexed-record eligibility and the
association with a committed observation. Orchestrator owns the rendering
policy. **CANDIDATE:** a bounded, disposable derivative cache holds the blobs;
whether Engine or Orchestrator owns that store remains to be reconciled. Do not
add a frontend-private disk cache while that decision is unresolved.

Thumbnail keys must distinguish admitted root/volume, object incarnation,
source revision, producer profile/version and output specification. A path-only
key is insufficient. An unrelated catalogue publication must not invalidate
all valid thumbnails. Source replacement, revocation and root removal must
invalidate the relevant association. A stat tuple alone is not a cryptographic
proof that bytes stayed unchanged during decode; the contract must name its
filesystem revision assumptions and reject observed changes.

The first thumbnail profile should expose only current online sources.
Offline cached imagery needs a separate privacy/staleness decision; coarse
offline catalogue permission does not admit it. Index absence or an unindexed
location schedules no thumbnail generation. Selected-file preview can still
operate independently under its admitted first-party content-read authority.

## Initial experimental bounds

All numbers below are **CANDIDATE test limits**, not measured requirements or
promises. Binary units are explicit. Reject oversize work before allocation
where the format permits, and enforce a hard process limit where it does not.

| Resource | Selected JPEG/PDF preview | Indexed thumbnail |
|---|---|---|
| Encoded source | 32 MiB JPEG; 128 MiB PDF | Same format limits |
| Raster source geometry | At most 64 million pixels and 32,768 per axis, including each PDF embedded raster | Same; never decode merely because output is small |
| Output geometry | Fit within 1,024 by 1,024 physical pixels | Fit within 256 by 256 physical pixels |
| Output bytes | At most 4 MiB tightly packed premultiplied BGRA8 | At most 256 KiB in the same format |
| Worker memory | 256 MiB hard limit, including decoder scratch | Same; budget is process-wide, not per allocation |
| Worker deadline | 3 seconds elapsed; terminate and reap at expiry | Same; zero foreground navigation waits |
| Concurrency | One active decode across this initial profile | Shares that single slot; selected preview has priority |
| Pending requests | One replaceable selection per window, at most 8 selections globally; reject excess admission without retaining a capability | At most 64 queued identities globally; visible range first |
| Resident raster cache | No reusable selected-preview cache beyond current display | At most 16 MiB evictable pixels |
| Pinned display bytes | 32 MiB aggregate across clients for this profile, at most 8 full-size displays; release grants on client loss | Shares display budget; refuse excess image admission and retain ordinary icons |
| Transfer and replacement bytes | At most one 4 MiB validated result and one 4 MiB receiver staging/replacement allocation globally; acknowledgements retire them | Shares limit; do not dequeue another render until result retirement |
| Disposable disk cache | None for selected previews | Initial candidate cap 256 MiB; owner and eviction semantics unresolved |

A 3-second deadline is a failure ceiling, not an acceptable interaction target.
Measure actual CPU, RSS, source bytes and queue delay. A large PDF may exceed the
memory/time bound even when its byte size is admitted; report that outcome.
Source size/geometry eligibility does not promise success within the memory
limit: a 64-million-pixel BGRA buffer alone consumes about 244.14 MiB. Aggregate
decoder input, scratch and output may force a resource rejection. Separate
resident, pinned and transient budgets include actual allocations/copies, not
only nominal image dimensions; the proposed display grant requires an explicit
release/acknowledgement protocol and must not depend on trusting a stale client.
GUI.Forms internal copies must be counted before this profile can be accepted.
The acknowledgement protocol needs a bounded expiry for a connected client
that stops acknowledging, explicit cancellation/revocation, and safe rejection
of late acknowledgements. Such a client cannot hold the sole result slot forever.
Process limit enforcement differs by OS and must be demonstrated rather than
described as equivalent. Control-message bounds and bulk-data transport need
their own canonical limits before adapters freeze.

JPEG output fits the oriented source within the requested extent without
enlargement. PDF proposes the valid CropBox intersected with MediaBox, falling
back to MediaBox when CropBox is absent; invalid/empty boxes are rejected. Apply
the page's declared rotation, then aspect-fit the resulting page into the
requested physical-pixel extent. Vector pages have no intrinsic pixel size,
so the JPEG no-enlargement rule does not apply. The negotiated profile must
specify rounding and the resulting raster scale.

JPEG fixtures cover EXIF orientations 1 through 8, progressive/baseline coding,
portrait/landscape, grayscale and rejected/converted color profiles. Proposed
output policy is sRGB; conversion support and unsupported profile behavior must
be explicit. PDF proposes page 1 only, no scripts, attachments, remote resources,
password prompt or whole-document rasterization. Password-protected files get
a readable unavailable result. Parser traversal and embedded-image amplification
remain bounded by the worker resource envelope and need adversarial fixtures.

## Scheduling and visible states

Only a current visible selected preview requests work. Replacement cancels the
old request; a stale completion cannot change content or caption. If thumbnail
work occupies the slot, cancel it for a selected preview; measure termination
and restart cost before accepting that preemption policy. No unbounded priority
queue or background sweep is implied.

Thumbnail arrival replaces only the matching cell's image. It must not reorder
rows, change selection, grow the cell or reset scroll. An unavailable thumbnail
uses the ordinary icon. Distinguish ready, pending, unsupported, unavailable,
stale and revoked/not-eligible internally; show concise useful explanations
without turning each transient state into a modal or an error badge. Preserve
structured terminal reasons for cancellation, deadline, memory/output exhaustion,
malformed input, password protection and provider unavailability even when they
share a concise unavailable presentation.

## Acceptance sequence

1. For selected-preview experiments, reconcile the render request/result
   profile, current-file revision validation, source-read capability, process
   containment and resource enforcement in Orchestrator's canonical registry
   and affected component notes. Record applicable owner authorization and any
   genuinely new subsystem opening required by the chosen implementation. A
   first-party helper is not automatically plugin execution, but renaming a
   supervisor does not bypass its gate. Contract reconciliation is ordinary
   authorized work; it is not a blanket new permission event. Do not mark stubs
   live or freeze adapters before semantic fixtures are complete.
2. Compare one selected JPEG and one first-page PDF implementation against the
   existing unsupported state and the current PNG path. Keep malformed, huge,
   truncated, encrypted, orientation and color failures as evidence.
3. Test worker death/timeout, cancellation, handle revocation, malformed response,
   output overflow, same-path replacement and file changes during reading.
   No stale output, leaked descendants or GUI-thread decoding may pass.
4. Verify actual pixels and readable outcomes on Windows, macOS and Linux.
   Measure selection-to-first-paint and worst UI stall, cold/warm p50/p95/p99,
   CPU/RSS, bytes and repeated rapid navigation on mixed generated corpora.
5. Before thumbnail implementation, additionally reconcile Engine record
   eligibility, cross-component identity association, derivative publication
   and store ownership. These do not block the selected-preview experiment,
   which has no persistent cache. Add thumbnails only after exact eligibility
   fixtures pass. Include
   unindexed and Engine-absent zero-generation cases, source replacement,
   unrelated catalogue publications, root revocation, visible-range churn,
   memory pressure and cold/warm cache eviction.
6. Publish a separately identified portable dogfood checkpoint. Do not claim
   general format support, finished installers or physical Mac acceptance from
   a synthetic native fixture.

All first-party implementation, tests and authored tools use the complete
[programming house style](PROGRAMMING_HOUSE_STYLE.md), including semantic source
review of named execution, explicit state/types, lifetime, failure, conversions
and repeated work. Vendored dependencies are not rewritten or certified by that
review. This planning record contains no implementation or measurement claim.

## Independent paper review

The authorized visible audit sibling reviewed this proposal against the
interviews, ADR-020, Orchestrator gates and the complete house style. Corrections
separate thumbnail prerequisites from selected previews, distinguish PDF page
geometry from raster enlargement, bound global selection admission, account
separately for pinned/cache/transient bytes, and avoid invented blanket approval
requirements. Resource limits and helper placement remain candidates; no
implementation or source-compliance certification follows from paper review.
The follow-up review accepted the revised candidate without a blocking
contradiction; its remaining acknowledgement-expiry requirement is included
above and remains part of contract completion.
