# M12-P11 retained layout transactions

Status: **MEASURED PARTIAL M12-P11** on 2026-08-06.

## Question

Can GUI.Forms replace its generated managed-only `SuspendLayout` counter with
one renderer-neutral transaction that preserves committed geometry, defers
nested work, tolerates faults, and exposes the admitted WinForms call family
without freezing a platform layout engine?

## Evidence and decision boundary

- **GIVEN:** a no-op compatibility shim is insufficient; ordinary geometry
  reads outside explicit update scopes are minimal read barriers, while reads
  inside a scope observe the last committed geometry.
- **OBSERVED:** the prior generated facade dropped `PerformLayout` while
  suspended or re-entered, emitted an empty `LayoutEventArgs`, and never told
  the native retained scheduler that a control was suspended.
- **OBSERVED:** pinned LibreWinForms increments a per-control suspension count,
  defers layout while nonzero, and performs cached work at the final requested
  resume. This is behavioral evidence, not an imported implementation.
- **DECIDED FOR THIS 0.x SLICE:** native `Control` owns suspension depth and
  deferred/requested/committed revisions. A suspended node blocks its own
  retained subtree while unrelated runnable siblings continue. `Window`
  remains the bounded layout executor.

## Implemented center

- nested `Control::suspend_layout`, `resume_layout(bool)`, and
  `perform_layout` with an inspectable renderer-neutral snapshot;
- suspended-subtree exclusion from measure/arrange without clearing its dirty
  state or changing committed bounds;
- runnable-dirty detection so intentionally suspended work neither spins nor
  records a false layout-pass-limit hit;
- final resume, explicit perform, and later geometry read share the ordinary
  bounded retained flush;
- layout callback exceptions restore dirty state and release the native
  re-entry guard for an explicit retry;
- experimental ABI 0.21 adds suspend/resume/perform/snapshot operations;
- generated facade implements both `ResumeLayout` overloads, both
  `PerformLayout` overloads, exact affected component/control/property
  `LayoutEventArgs`, an eight-pass deferred re-entry bound, and fault recovery;
- the member ledger promotes only this measured center.

## Verification

- `gui_forms_core_tests`: committed geometry, nested suspension, runnable
  siblings, `ResumeLayout(false)`, read-barrier commit, explicit perform,
  harmless unmatched resume, and native callback-fault retry;
- `gui_forms_c_api_c11_tests`: ABI 0.21 negotiation, nested depth, detached
  deferred retention, attached commit, explicit retry, and invalid arguments;
- `GuiForms.FacadeBehaviorSmoke layout-transactions`: coalescing, exact args,
  bounded re-entry, `ResumeLayout(false)`, unmatched resume, and recoverable
  managed callback fault on host .NET and Wine;
- generated surface: 1,104/1,104 catalogue rows resolved;
- renderer-free and ASan/UBSan focused suites pass; strict Win64 and the
  Skia-enabled Win64 ABI build compile cleanly; the focused generated behavior
  gate passes under Wine with the same exact trace as host .NET.

## Honest remainder

- exhaustive independent WinForms event ordering and affected-property caching;
- large designer-generated trees and hard convergence/pass-limit telemetry;
- reparenting, removal, and disposal from within layout callbacks;
- baseline, RTL, scale/DPI rounding, and difficult nested Dock/Anchor families;
- DML construction/reorder policy and public diagnostics/availability
  projection.

This record does not claim complete WinForms layout parity or a final ABI.
