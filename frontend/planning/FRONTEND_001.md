# Frontend 001 specification

Status: **DECIDED scope; waiting for GUI.Forms go-ahead and explicit architect
start direction under ADR-004**.

Frontend 001 is the first executable File Manager application slice. It is a
GUI.Forms consumer and a deterministic composition laboratory, not the first
fully integrated product build.

## Opening predicate

Implementation begins only after both are recorded:

1. GUI.Forms gives a named go-ahead for the consumption surface below.
2. The grand architect explicitly directs File Manager frontend work to begin.

Engine, Orchestrator, plugin, semantic-fact, and Kolmogrov readiness are not
part of this predicate. Their 001-facing states are supplied by deterministic
frontend fixtures until their individual contracts are ready.

## GUI.Forms consumption surface

The GUI.Forms go-ahead must identify a buildable, dependency-facing snapshot
that supports the 001 shell without private-header access. At minimum it must
cover:

- application, window, retained-tree, UI-thread, ownership, and shutdown
  lifecycle;
- deterministic layout, resize, DPI/scale, damage, input, focus, and keyboard
  traces;
- the controls or compositional primitives needed for command shelf, path and
  search instruments, collapsible panes, object field, status, and overlays;
- text display and the bounded editing behavior actually admitted to 001;
- PNG/resource, semantic naming, theme/state, and headless-test seams;
- a clean CMake consumption path and named C ABI/C++ wrapper version.

The complete requested substrate and its FM0/FM1/FM2/FMX staging are recorded
in
[`../../gui_forms/planning/FILE_MANAGER_CONSUMER_CAPABILITY_PROFILE.md`](../../gui_forms/planning/FILE_MANAGER_CONSUMER_CAPABILITY_PROFILE.md).
Frontend 001 requires the admitted FM0 subset, not every later row.

If GUI.Forms excludes a feature, 001 narrows or records a follow-up. The
frontend does not implement a shadow widget framework.

## Delivered application slice

001 delivers an independently buildable C++ application under `frontend/` with:

- one location-oriented window; no tabs and no dual content panes;
- Watercolor identity and Office Pearl command zones above a subtle
  middle-value Workshop Graphite navigation chassis;
- a compact labelled task shelf;
- back, forward, up, breadcrumb/path, terminal `./`, and separate scoped-search
  instruments;
- collapsible tree and selection/properties panes around the sole central file
  surface;
- deterministic sample file objects, selection, focus, status, availability,
  and empty/degraded states;
- the Sapphire Dusk atmosphere as the default;
- semantic icon slots following Fluent Color plus original House Material
  direction, using only locally generated or provenance-cleared 001 assets;
- deterministic keyboard, pointer, resize, pane-collapse, activation, and
  shutdown scenarios;
- explicit frontend ports for filesystem, Engine, and Orchestrator data.

The path matrix, path editor, search-result expansion, direct property editing,
drag/drop, motion, sound, and other debt-ledger objects enter 001 only to the
extent supported by the GUI.Forms go-ahead and an existing GIVEN rule. Missing
behavior is labelled, not improvised.

## Design authority

[`visual/DESIGN_DNA_006.md`](visual/DESIGN_DNA_006.md) is the normative design
constitution for this slice. Its sixteen principles and records marked GIVEN
or DECIDED are requirements. CANDIDATE and HYPOTHESIS entries remain deliberate
experiments with named failure/reversal points.

The primary supporting records are:

- [`visual/FRONTEND_DESIGN_BRIEF_001.md`](visual/FRONTEND_DESIGN_BRIEF_001.md);
- [`visual/STYLE_FAMILY_ATLAS_004.md`](visual/STYLE_FAMILY_ATLAS_004.md);
- [`visual/VAPORWAVE_PALETTE_ATLAS_003.md`](visual/VAPORWAVE_PALETTE_ATLAS_003.md);
- [`visual/ICON_AUDIT_MIT_002.md`](visual/ICON_AUDIT_MIT_002.md).

Those documents direct appearance and behavior; they do not override
GUI.Forms, Engine, or Orchestrator contract ownership.

## Fixture boundary

Frontend-owned ports present fake capabilities as `simulated`. Fixtures cover:

1. ordinary folder with mixed objects;
2. empty folder;
3. selection, keyboard focus, secondary-but-participating window,
   drag-proximity, and genuinely deactivated-surface distinctions;
4. Orchestrator ready, degraded, unavailable, and incompatible;
5. Engine ready, stale, rebuilding, unavailable, and direct-fallback-eligible;
6. plugins and semantic facts explicitly `stubbed`;
7. long names, deep paths, dense content, narrow windows, and 125–200% scale.

Fixture schemas are application test data. They do not freeze a provider ABI.

## Exit gate

Frontend 001 is complete when:

- it builds from a clean tree against only the named GUI.Forms snapshot and
  declared ordinary build dependencies;
- all service scenarios run without Engine or Orchestrator processes;
- headless traces for construction, focus, navigation intent, pane collapse,
  resize, activation, and shutdown are deterministic;
- no frontend code includes GUI.Forms private headers or provider-private
  Engine/Orchestrator types;
- unavailable auxiliary services never block window input or local navigation
  state;
- every shipped external or derived visual asset has a pinned provenance and
  license record;
- key/secondary-participating/deactivated, selection/focus, high-contrast,
  reduced-motion, and scale behavior are either demonstrated or explicitly
  recorded as blocked by the named GUI.Forms snapshot;
- the DNA debt ledger is updated with measured implementation findings rather
  than silently resolved by code.

## Explicit exclusions

001 does not require real filesystem mutation, real indexed search, a live
Orchestrator daemon, settings writes, handler invocation, plugin workers,
semantic facts, installer/signing, or daily-use replacement. It must not claim
those capabilities from fixtures.
