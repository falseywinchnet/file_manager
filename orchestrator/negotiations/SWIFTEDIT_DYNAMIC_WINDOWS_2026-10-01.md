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
