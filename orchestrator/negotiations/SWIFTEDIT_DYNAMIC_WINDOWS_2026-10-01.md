# SwiftEdit dynamic document windows — intake 001

Status: **provider audit confirms the gap within its reviewed scope; separate
lifecycle negotiation pending**.

## Provenance and scope

**OBSERVED** — On 2026-10-01 the coordinating chat
`01a0f009-7508-78a2-9fd4-cbf544e9193d` directed Orchestrator to record and
triage SwiftEdit New Window separately from document-view D1. The requested
behavior is dynamically created independent document windows with owned
dialogs. This intake does not authorize host or runtime implementation.

**OBSERVED** — The coordinator relayed the consumer's report that the installed
GUI.Forms `Application` accepts only a fixed initial window vector. This is
evidence of the consumer report, not a source-verified finding that no other
public route exists. The GUI.Forms provider has been asked to confirm an
existing route after D1 closure or propose a separate lifecycle contract.

Provider coordination is with chat
`01a0f0bf-3d19-7fe1-8995-119065b2448b`; consumer coordination is with SwiftEdit
chat `01a0f0bd-1423-7710-923f-c76ade7a8490` in the separate
`C:\Users\Shadow\notepad` repository. The planning-only Text Editor project
in this repository is not this consumer; its implementation gates are unchanged.

## Questions for provider confirmation

The following are unresolved negotiation questions, not admitted API semantics:

- Does an existing installed public API create a document window after the
  application has entered its event loop? Name the API, installed SDK version,
  source locator and independent consumer evidence if it does.
- What owns each window and document session, and what are the create, close,
  destruction and application-exit transitions? How does closing the last
  document window interact with application lifetime?
- How are dialogs owned by their originating document window, including focus,
  modality and an attempted owner close while a dialog is active?
- How are pending work, callbacks and late results cancelled or drained before
  a window and its document state are destroyed? What identifies the target
  window without permitting reuse to revive stale work?
- What limits and typed failure outcomes apply to creation and lifecycle
  operations, and what state remains authoritative after failure?

**CANDIDATE** — If no existing route satisfies the need, a distinct public
window-lifecycle proposal may enter project-local negotiation. The provider
must supply ownership, operation ordering, failures, cancellation, limits and
fixture proposals before Orchestrator reconciles semantics. This intake does
not select a handle design, ownership model, host adapter or ABI shape.

## Queue disposition and evidence boundary

This item remains in the negotiation queue. No contract ID, runtime capability,
installed availability or implementation authorization is added by this record.
Provider confirmation is the next evidence needed; a proposed API alone will
not establish installed availability.

[Document-view D1](SWIFTEDIT_DOCUMENT_VIEW_2026-10-01.md) retains its existing
scope and agreement. D2/D3/D4 and printing P1 remain separate. This intake does
not block or widen the current D1 work.

Records-only review covers provenance, consumer/provider ownership, unresolved
lifecycle questions and the distinction between requested and available
behavior. No implementation, test or authored tooling is changed; source
review against `planning/PROGRAMMING_HOUSE_STYLE.md` remains required if a
later authorized implementation proceeds.

## Provider reply 001 — source audit received

**OBSERVED** — The provider supplied
[the dynamic-window capability audit](../../gui_forms/docs/DYNAMIC_WINDOW_CAPABILITY_AUDIT_2026-10-01.md)
on 2026-10-01. This reply advances the initial report above: the provider's
source/header review confirms no current public Application route for dynamic
independent in-process document windows within the audited scope.

The audit records byte-identical source and frozen `house-style-final` installed
public headers, including their SHA-256. It names portable validation requiring
exactly one independent primary among at most 64 initial windows, fixed native
entry preparation, and rejection of nested `Application::run`. The Windows
host creates the initial window states before its message loop and quits that
loop when the primary closes. Existing owned dialogs and a separate executable
do not establish the requested capability. No macOS/Linux native lifecycle run
was performed. Orchestrator has reviewed the audit document; it has not repeated
the provider's source/hash inspection or inferred native measurements from it.

**CANDIDATE** — The provider proposes W1 as a separate application-session or
window-group negotiation. Its weak session handle, generation identities,
creation outcomes, close-intent policy, modal scope and teardown ordering remain
proposals. W1 is a discussion label here, not a newly admitted registry contract.
Consumer lifecycle confirmation and a reconciled proposal are still required;
parent coordination of host/runtime/build ownership precedes implementation.

The next evidence needed is the consumer/provider lifecycle agreement, replacing
the initial request merely to confirm whether a route exists. No runtime
availability, frozen SDK change or host implementation is authorized by this
reply. D1-D4 and print P1 remain independent.

## Consumer reply 001 — requested W1 lifecycle

**OBSERVED** — SwiftEdit's consumer chat confirmed New Window means an
independent blank document with its own main window and picker/find/font owned
dialogs. No shared document/history or tabs are requested. Failed creation must
leave existing windows intact. Each main has independent Save As and unsaved
continuation state.

Closing one main prompts only that document. Cancelling close leaves its state
and windows live; committed close revokes its queued callbacks and closes its
owned dialogs without terminating other documents. The final main close exits
after owned tasks drain. Application shutdown should preflight save/cancel for
dirty mains, avoid discarding any cancelled document, and preserve already-saved
documents if a later prompt cancels shutdown.

These are consumer-requested semantics awaiting provider reconciliation, not a
runtime contract. The precise interaction between shutdown preflight, successful
saves, queued creation, modal dialogs and committed teardown remains to be
specified before implementation permission or availability can be recorded.

## Current-source refresh and SDK consumer report — round 002

**OBSERVED** — The coordinator relayed SwiftEdit's report that installed
Application SDK `6def54a` still exposes a fixed startup vector of at most 64
windows and show/hide/close/fullscreen handles. Its installed path/hash remains
requested from the consumer; that installed-artifact claim is not independently
verified by this source inspection.

Orchestrator inspected current
`gui_forms/include/gui_forms/application/application.hpp`: it declares
`maximum_windows = 64`, only one-window/vector `run` overloads, and the four
named weak window-handle operations. It declares no runtime window-registration
operation. `src/core/application/application.cpp` rejects a count above the
maximum and requires exactly one independent primary. The runtime `run` accepts
the vector, refuses nested running state, validates, then constructs a fixed
bridge/native entry collection before invoking the platform host. This verifies
the current public Application gap within the inspected scope, without
repeating the old installed-header equality claim for a new SDK.

The existing API explicitly consumes models even on failure. A future dynamic
creation edge must decide its own transfer law; it must not accidentally inherit
that behavior while promising atomic rejection. Current primary-close behavior,
accepted lifecycle ADRs and existing Application overload semantics are not
silently rewritten by this intake.

## Dynamic create/lifetime requirements under reconciliation

The requested behavior is a new independent document root admitted on the UI
executor after the application is running. A preallocated fixed window pool is
not the requested behavior. Each created root owns its document model and
dialogs, with generation-bound native handles, ready/closing/closed ordering
and no stale wake/dispatch resurrection.

The concrete proposal must distinguish synchronous refusal from accepted pending
creation and native readiness, stating exactly when model ownership transfers.
Invalid/wrong-thread/limit/closing/unsupported and native/callback failures need
typed outcomes; partial native creation must clean up without damaging other
roots or leaving an orphaned model. Counts include creating/live/closing and
unreleased work; runtime creation is not an unbounded window allocation API.

Closing must distinguish user/save cancellation from committed teardown and
native cleanup failure. Save error or cancelled close must not silently discard
the document. Define owned-dialog completion, focus/capture/cursor revocation,
worker/wake draining and handle invalidation before model/native owner release.
The existing consumer shutdown preflight and last-root exit requirements remain
in force. Creation during quit preflight and cancellation of pending creation
need explicit outcomes; no late create may resurrect a quitting application.

Report Windows/macOS/Linux support separately, with unsupported absence rather
than a claimed portable native implementation. Reconcile exact platform adapter
feasibility and finite limits before freezing any new API. This remains W1
research/planning with no source assignment. A2/document view and print P1 retain
their separate scopes; this refresh neither blocks nor widens their work.

## Consumer reply 002 and installed-header verification

SwiftEdit confirms the existing independent-document lifetime requirements with
no new owner scope. Its implementation preference is synchronous create with
model transfer only on success. During quit preflight/commit it proposes explicit
`quit_pending` refusal without taking ownership or queuing a surprise window
after shutdown cancellation. These are consumer proposals, not additional owner
requirements. If the provider requires accepted/pending creation, cancellation
before publication, stable completion identity and all-or-nothing document-root/
owned-group cleanup must be reconciled first.

Save failure or user cancellation retains the document/window. Earlier successful
saves stay saved if a later shutdown prompt cancels. Windows, macOS Apple silicon
and Linux are requested targets; the reply supplies no new native lifecycle
evidence or host ownership expansion.

**OBSERVED, independently checked:** the installed header at
`C:/Users/Shadow/notepad/.build/provider-sdks/6def54a/windows-x64/installed/gui-forms-sdk/include/gui_forms/application/application.hpp`
has SHA-256
`0a1f581f4e53e0b16a23534b6797d51223d911e2128acd5087d0abb97f993f50`,
matching the current public header inspected in this round. Thus the fixed-startup
API and missing public dynamic-registration operation are confirmed for this
specific installed header as well as current source. This does not infer native
behavior from a header hash.

The consumer additionally reports provider revision
`6def54ad2bf2089b57c09337c0b3f82cc1178187`, pinned in its
`ci/native-sdk-lock.json`, and Windows ZIP SHA-256
`1d8ae608f8365dcf18f883673a30eac14df9aad234921153c613418f779f3d67`.
Orchestrator verified the installed header only, not that ZIP or the complete
installed package. W1 remains research/planning with no source assignment.
