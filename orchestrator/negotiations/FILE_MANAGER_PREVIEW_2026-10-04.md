# File Manager bounded preview intake

Status: **outline / candidate comparison; no runtime capability available**.

The active owner goal requests useful real-world previews and indexed
thumbnails after macOS dogfood exposed failures. The source audit and scoped
comparison are in
[`PREVIEW_AND_THUMBNAIL_NEXT_SLICE.md`](../../planning/PREVIEW_AND_THUMBNAIL_NEXT_SLICE.md).
That paper's limits are experimental candidates, not canonical API values.

## Intake 001: selected preview first

**Orchestrator requirement:** negotiate a closed first-party rendering profile
for selected JPEG and first-page PDF previews before choosing a codec or freezing
adapters. Compare a supervised portable helper, contained native adapters, and
any independently admissible trusted built-in alternative. Native/third-party
preview execution cannot enter a trusted process. The existing UTF-8/PNG path
and catalogue-independent navigation remain available.

The profile must define:

- request identity, version, authenticated caller and scoped source-read authority;
- freshly validated source identity/revision and the filesystem assumptions used
  to detect replacement or modification while rendering;
- bounded output geometry, orientation, color and page interpretation;
- named ownership for input, decoder scratch, transfer, retained display and any
  copies; global admission and peak replacement accounting;
- cancellation, deadline, process termination/reaping, resource rejection and
  structured terminal reasons;
- result acknowledgement, expiry, revocation and late-result disposal so one
  connected but nonresponsive client cannot hold the only result slot;
- availability when the provider or Orchestrator is absent and restart behavior;
- minimum and hostile fixtures before an adapter freeze.

**Observed frontend reply:** the current application already owns a selected
preview generation, hidden/deferred/pending/ready state and stale-publication
checks. PR35 and PR36 exercise those transitions and retained geometry. They do
not provide a worker read capability, a codec process, or a general raster-result
protocol. A selected-preview experiment can use current-file identity without
waiting for Engine/frontend identity reconciliation or a persistent cache.
The project-local reply is appended to
[`frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`](../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md).

**Reconciliation status:** intake accepted for paper comparison and contract
work. No new contract ID, wire method, profile version or runtime capability is
frozen. ORC-PLG-002/003 remain stubbed. The next round must decide whether the
chosen first-party helper opens the gated plugin-supervisor subsystem or uses
a narrower first-party integration profile; neither outcome follows from naming
alone. Record applicable existing owner direction and any genuinely new
subsystem opening, rather than requiring a blanket repeated permission message.
ADR-020's admitted format profile still requires independent bounds and evidence
before it can be extended. No dependency or implementation choice is made here.

## Separate thumbnail round

Only index-supplied thumbnails in admitted indexed trees are eligible. Before
that adapter opens, obtain Engine's reply on exact record eligibility and
revision association; reconcile Windows identity representations, root removal,
revocation and source replacement. Decide disposable blob ownership and atomic
publication without selecting a media hive by implication. The first candidate
profile excludes offline imagery. Existing coarse-catalogue permission is not
permission to retain thumbnails.

These are thumbnail-specific questions. They do not block the selected-preview
comparison above. No Engine private implementation is changed by this intake.

## Review and next evidence

The originating paper received independent review against the full programming
house style, ownership rules and accepted interview constraints, followed by a
correction pass. That is planning review, not a decoder safety, performance or
implementation-compliance claim. Next evidence must include primary-source
codec/platform comparison, scoped capability/lifetime fixtures and three-platform
resource-enforcement feasibility before provider selection.

### Provider evidence update

The first comparison is now recorded in
[`PREVIEW_PROVIDER_COMPARISON_2026-10-04.md`](../../planning/PREVIEW_PROVIDER_COMPARISON_2026-10-04.md):
historical native JPEG/EXIF correctness, a new generated-fixture Windows WIC
control, bounded-output fresh-call measurements, and primary-source PDF/build
and OS resource-limit constraints. The portable candidate has lower medians on
the declared Windows fixtures; color-policy equivalence, real photos, process
limits and application pixels remain unproved. The intake stays an outline;
no helper placement, decoder dependency or additional format is admitted.

## Lifecycle fragment 002

**Orchestrator requirement:** test the selected-preview lifecycle independently
of provider/transport choice: bounded pending storage, stop/reap ordering,
monotonic deadlines, wrong-ticket/source rejection, result acknowledgement and
expiry, window loss and restart. Numeric budgets remain revisable candidates.

**Frontend reply:** its current selection generation/demand state is compatible
with replacing one pending request per window and refusing late completion.
It must retain a completed grant when merely collapsing/reopening the inspector,
retire that grant on selection/window loss, and validate any future incoming
pixels before GUI.Forms admission. Current code has no broker adapter and its
existing worker queue is not replaced by this reply. See the appended local
round in `../../frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`.

**Reconciliation:** the candidate lifecycle fragment and disposable laboratory
are recorded in `../spec/SELECTED_PREVIEW_LIFECYCLE_DRAFT.md`. A reap event cannot
be inferred from a successful stop call or decoded pixels. A client missing its
acknowledgement deadline loses the offered result; grant accounting is distinct
from proof of physical memory release. The full profile remains incomplete;
no runtime operation, source capability, codec selection or plugin execution
is opened by this fragment.

## Native process feasibility follow-up

**CANDIDATE experiment:** `../experiments/preview_process/` exercises a closed,
generated-input native child with normal/failure exit, repeated cancellation,
observed exit before reuse, and controlled memory-admission comparisons.
Windows Job committed-memory limits and POSIX address-space limits are measured
separately. An unavailable Mac limit is recorded as a skip, not resource-cap
conformance. This disposable C++ OS probe does not select a production language,
activate plugin execution, connect the lifecycle model, or freeze a provider.
The existing Rust Orchestrator boundary remains authoritative. Startup footprint,
descendants, source authority, bounded transport and full sandbox policy remain
unresolved before an actual preview adapter can be admitted.

## Result-byte/source fragment 003

**Orchestrator requirement:** a decoder cannot publish an arbitrary native
record, claim file authority through its own source identifiers, or induce
unbounded raster allocation. Establish exact byte extent, ticket matching,
version/profile refusal, terminal shape, and publication only after EOF plus
host-owned lifecycle/source checks.

**Frontend reply:** any new raster result must be owned and independently
validated before GUI.Forms image admission. Current `NativeReadFile` descriptor
and visible-path observations are useful existing evidence but not atomic
snapshot or ancestor-route guarantees. The frontend does not authorize a new
decoder merely by accepting its echoed ticket. Its generation/demand state
still rejects stale publication and retains completed content across hiding.

**Reconciliation:** the candidate frame/source fragment is recorded in
`../spec/SELECTED_PREVIEW_FRAME_DRAFT.md`, with a disposable Rust receiver and
independent C++ byte producer under `../experiments/preview_frame/`. No real file,
codec, provider discovery or production IPC participates. Native read authority,
source revision assumptions, private transport authentication, global copies and
Mac resource policy remain open. ORC-PLG remains stubbed; no adapter is frozen.
