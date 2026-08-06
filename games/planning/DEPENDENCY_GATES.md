# Games dependency gates

Status: **mandatory; planning open, implementation closed**.

## GA0 — architect interview closure

- packaging topology, public names, first implementation module and first
  platform selected;
- per-game rules profiles and explicit deviations recorded;
- animation, input-during-animation, sound, hint and reduced-motion laws;
- save/resume, undo, statistics and difficulty policy;
- art/resource provenance and accessibility requirements;
- each game plan has a smallest dogfood workflow and correctness oracle;
- no unresolved choice changes shared saved-state or window topology.

## GA1 — GUI.Forms consumption gate

A named package passes card/board/grid controls, custom retained drawing,
damage, pointer/keyboard/gamepad-if-admitted input, text entry, timers/test
clocks, animation, sound hooks, owned modal/modeless dialogs, help, resources,
scaling, accessibility and native host fixtures. No perpetual idle redraw.

## GA2 — shared kernel reference gate

Before visual breadth, the deterministic state/command/replay/save envelope and
test clock pass differential fixtures. Game-specific models remain independent;
the kernel must not become an inheritance hierarchy that encodes every rule.

## GA3 — Orchestrator application-services gate

The used portions of `ORC-APP-001`, `ORC-SET-001`, and `ORC-HLP-001` are
reconciled and fixture-backed. Games do not require Engine, Kolmogrov, semantic
hives, plugin execution, federation, LAN availability, or the Document Picker
for their first slice.

## GA4 — module gate

The selected module's individual plan records exact rules, oracle corpus,
presentation states, dialogs, accessibility, reduced motion and performance
workload. Passing one module does not claim the others are implemented.

Crossword additionally requires the Lexicon corpus-enumeration snapshot and a
pinned Shakespeare corpus manifest.

## GA5 — owner start gate

The grand architect explicitly opens Games implementation and names the first
module/platform/sandbox. Technical readiness never starts code automatically.

## GA6 — release inclusion gate

Each shipping module independently supplies native three-platform evidence as
claimed, help/rules, deterministic corpus, resource licenses, installer
manifest, settings/save compatibility, accessibility/reduced-motion record,
performance evidence and accepted known issues. Unfinished modules remain
absent rather than simulated.
