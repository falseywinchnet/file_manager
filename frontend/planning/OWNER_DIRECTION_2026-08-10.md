# File Manager implementation direction — 2026-08-10

Status: **GIVEN; Frontend implementation and M4 dogfood are open**.

## Direction

The grand architect directed work to begin on File Manager, to implement and
dogfood it on the M4 Mac mini, to inspect the Web.Forms and GUI.Forms GUI paths,
and to pursue a fully running 1.0. The architect explicitly permits scoped
extensions to other repository components when they are required to enable the
File Manager frontend and its intended initial features. The architect further
stated that the prior blockers are cleared.

## Machine and visual-operation boundary

The Neo checkout remains authoritative. Source is mirrored to the named M4 by
`m4build`. Browser preview and native GUI operation occur inside the M4 Aqua
session through Screen Sharing; they are not launched on the Neo. On this date
the existing `frontend-concept-atlas.html` was opened and maximized in a browser
inside the M4 Screen Sharing desktop before product implementation continued.

## Interpretation

- **DECIDED by ADR-016:** Web.Forms and GUI.Forms are complementary, not rival
  runtimes. Browser-valid HTML/CSS is design/build input; generated public C++
  runs in the retained native GUI.Forms host.
- **OBSERVED:** Orchestrator Core 1.0 already has accepted installed launchd
  evidence and a real independent C++ bootstrap edge.
- **MEASURED:** GUI.Forms' named FM0 package now has a clean external native
  install/consume probe on the M4.
- Later provider capabilities remain truthful availability states. This start
  direction does not turn deferred settings, handlers, plugins, or cross-
  platform evidence into implemented features.

This record supplies architect authority. Provider readiness and measurements
remain owned by their named manifests and evidence records.
