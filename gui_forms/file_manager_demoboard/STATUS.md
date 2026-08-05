# Demoboard status ledger

Last updated: 2026-08-05

Status: **specification complete; implementation not started**.

This is the first file a worker updates after completing the mandatory reading
order. It reports evidence; it does not turn an unavailable parent capability
into a local implementation task.

## Milestones

| Slice | State | Current evidence | Blocker/next action |
|---|---|---|---|
| DEMO-D0 package/consumer proof | not started | specification only | create opt-in public-package consumer target under explicit implementation authority |
| DEMO-D1 shell/material | not started | copied atlas and screenshots | requires public shell/layout/material subset |
| DEMO-D2 Folder composition | not started | fixture and control contracts | requires admitted command/tree/list/menu primitives |
| DEMO-D3 panes/edit/responsive | not started | interaction and acceptance contracts | requires split/scroll/editor/priority-collapse primitives |
| DEMO-D4 path matrix | not started | exact fixture and state machine | requires breadcrumb/editor/popup/suggestion primitives |
| DEMO-D5 Search correspondence | not started | seven-result fixture and geometry | requires variable-height virtualization/inspection expansion |
| DEMO-D6 Criteria virtual folder | not started | three-module fixture | requires editable choice/property/rack composition |
| DEMO-D7 transfer/operations | not started | operation scenarios | outbound drag and operation surfaces remain parent work |
| DEMO-D8 accessibility/accommodation | not started | semantic requirements | native publishers and text ranges remain parent work |
| DEMO-D9 review laboratories | not started | copied atlas content | follows default daily-surface material proof |
| DEMO-D10 closure | not started | acceptance matrix | follows D0–D9 evidence |

Allowed states: `not started`, `in progress`, `blocked by <FM-ID>`, `measured
partial`, `complete`. Use `complete` only when the slice's exit and applicable
acceptance gates pass.

## Current capability posture

The parent GUI.Forms project currently proves a retained kernel, damage,
display chunks, a native AppKit/Win32 proving host subset, basic/container/range
controls, PNG resources, Unicode text-store/grapheme foundations, and
experimental ABIs. This summary is informational and can become stale; the
worker must inspect the parent capability report and relevant milestone evidence
before changing a row.

| Consumer area | Initial posture | Required recording |
|---|---|---|
| stable shell/layout | partial | exact public controls and missing FM-W/FM-LY IDs |
| House drawing/material | partial | admitted FM-R operations and explicit fallback/unavailable states |
| commands/ribbon/menu | open/partial | FM-C IDs and shared-command evidence |
| breadcrumb/path editor | open | FM-N IDs; do not build a private editor/popup stack |
| virtual tree/object views | open | FM-O IDs and realization bounds |
| selection PropertyList/preview host | open | FM-O11/O12 and one-scroll-plane evidence |
| correspondence rows | open | FM-C09/C10, FM-S01, FM-LY08 |
| criteria rack | open | FM-S02/S03 and public editor/choice composition |
| outbound drag/external peer | open | FM-D IDs; inbound proof is not outbound proof |
| native accessibility | open | FM-A IDs and physical publisher results |
| HarfBuzz/FreeType/bundled body fonts | selected direction, implementation gated | FM-T IDs, pack hashes, coverage and raster profile |
| motion/sounds | open | FM-V05–V07, fake-clock and muted equivalence |

## Evidence locations to create

When implementation begins, create evidence beneath this project so the
standalone consumer can be audited without searching temporary build output:

```text
results/
  <date>-d0-consumer/
  <date>-d1-shell/
  <date>-d2-folder/
  <date>-d3-responsive/
  <date>-d4-path-matrix/
  <date>-d5-search/
  <date>-d6-criteria/
  <date>-d7-transfer/
  <date>-d8-accessibility/
  <date>-d9-review-boards/
  <date>-d10-closure/
```

Each evidence directory contains, as applicable:

- `README.md` with revision, environment, claim, and inference boundary;
- command/test output;
- headless layout/input/command/semantic traces;
- capability report;
- native screenshots and capture manifest;
- damage/layout/text/resource measurements;
- host/symbol/include boundary audits;
- accessibility notes;
- failures and negative results.

Do not check in generated build trees, transient caches, or unbounded logs.

## Worker update template

```text
Date:
Slice:
State:
Control/state IDs covered:
Public GUI.Forms surface consumed:
Fixture generation:
Tests/traces:
Native captures:
Measurements:
Accessibility/accommodation evidence:
Unavailable/incompatible FM IDs:
Visual deltas:
Negative results:
What this does not prove:
Next action:
```

## Present evidence

- frozen atlas source and four historical screenshots under `reference/`;
- SHA-256 manifest and correction ledger in `reference/README.md`;
- detailed project, visual, control, interaction, fixture, implementation, and
  acceptance specifications;
- no native demoboard source, executable, test, or measured capture yet.
