# Frontend owner direction — 2026-08-07

Status: **GIVEN grand-architect direction**. This record supersedes conflicting
older mockup and demoboard language. It does not manufacture dependency
readiness or open separately gated future applications.

## Implementation direction

- Begin the File Manager frontend workstream as soon as its two recorded
  technical predecessors are truthfully available: Orchestrator Core 1.0 and a
  named GUI.Forms FM0 consumption snapshot.
- Build an operational program, not a disconnected presentation specimen.
  Bring it up with the live Orchestrator, navigate a protected filesystem root,
  connect the Engine through the registered Orchestrator route, and exercise
  real search and navigation before widening dogfood.
- Destructive early dogfood remains confined to an explicit chroot or
  capability-rooted disposable sandbox. The exact macOS containment mechanism
  must be selected without weakening the existing root guard.

## Visual and interaction corrections

- The frontend concept atlas is closer to the intended product than the current
  native GUI.Forms demoboard in the areas where they differ. The demoboard is
  evidence for mechanics, not an authority that can reverse the vision.
- The compact Office-like ribbon/control shelf direction is retained and is
  more correct in the concept atlas than in the current demoboard. Its command
  anatomy, materials, density, state coverage, responsive collapse, icon work,
  and platform behavior still require substantial polish.
- Path editing is a first-order inline operation in the ordinary breadcrumb
  trail. It is not entered through the pulled-down full-path matrix used by the
  current demoboard.
- Breadcrumbs form one vertically dense, continuous chevron cascade. They are
  not a row of unrelated rectangular buttons and do not reserve a second band
  of content beneath the trail.
- Editing preserves the breadcrumb instrument's identity, geometry, focus
  history, canonical-resolution preview, autocomplete, commit, cancel, and
  accessibility relations. Suggestions may use an anchored popup; the editor
  itself remains inline.
- The previous recent-full-trail matrix is no longer authoritative as the path
  editor. Whether recent full trails survive behind a separate history
  affordance remains an explicit product question.

## Settings and suite access

- File Manager requires an extensive, multi-tab Settings surface, including
  configuration and truthful status/control for backend services.
- Orchestrator remains the authority for settings schemas, values,
  transactions, service state, handler/command registries, provider
  availability, and grants. File Manager owns the tab structure, house
  rendering, interaction, validation presentation, and degraded states.
- Settings are not arbitrary server-driven UI. A backend may provide bounded
  typed schema/state; the frontend maps it into product-owned control
  compositions.
- Access to other installed Malkuth programs, including future Games, Paint,
  and Text Editor entries when they become available, belongs in OS-native
  application menu-bar entries. It must not become an in-program styled suite
  launcher.
- Menu entries are availability-driven. This direction does not create fake
  applications, open their implementation gates, or require placeholders for
  software that is not installed and admitted.

## Preserved boundaries

This direction does not add tabs, dual panes, web search, a bundled web engine,
provider-owned controls, a second frontend index, or direct frontend access to
plugin workers. It does not permit File Manager to call private GUI.Forms,
Engine, or Orchestrator implementation types.
