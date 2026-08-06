# Text Editor implementation sequence

Status: **post-gate candidate plan; no implementation authorized**.

## T0 — byte/text document reference

Implement bounded load/decode/newline/BOM observation, immutable revision IDs,
edits, selection, dirty state, encoding/newline conversion and serialization
without GUI or filesystem mutation. Preserve a simple exhaustive string/byte
reference and adversarial Unicode/malformed fixtures.

## T1 — safe file lifecycle

Against disposable files, implement open, external-change observation, save,
save-as, atomic replacement, permissions/metadata policy, conflicts, failures,
crash points and reopen verification. No privileged paths.

## T2 — GUI.Forms editor laboratory

One document window using only the named GUI.Forms package: multiline editing,
scroll/wrap, clipboard, undo, IME/bidi/shaping, menu/status, unsaved-close,
accessibility and deterministic input/layout/damage traces.

## T3 — Document Picker and Orchestrator

Use real application profile/settings/help/handler contracts and the shared
picker for visible/hidden open, open-many and save-as. Prove no-Engine behavior,
native fallback, Orchestrator restart and incompatible capability state.

## T4 — find/replace and whitecards

Implement literal search first, then only the architect-approved wildcard
grammar. Exhaustively test empty patterns, overlapping matches, Unicode/case/
word boundaries, selection scope, wrap, replacement expansion, huge lines,
cancellation and undo grouping.

## T5 — color hints

Implement the accepted first-party/provider boundary, exact revision-anchored
spans, incremental/cancellable work, stale drop, quotas and theme categories.
Typing/save remains independent of provider failure.

## T5C — Characters dialog

Implement the owned virtualized character popup using the admitted pinned
Unicode/font path. Prove search, code point details, encoding-safe insertion,
copy, undo, owner-close/focus behavior and keyboard/screen-reader navigation.

## T6 — polish and three-platform dogfood

Complete popup Help/greaseboard, fonts, settings, keyboard/accessibility, high
contrast, external changes, large-file degradation, handlers/CLI and native
macOS/Windows/Linux workflows. Compatibility layers do not count as native
support.

## T7 — Malkuth release candidate

Add installer/handler integration, local/online docs, website captures,
encoding/save/security corpus, licenses/SBOM, migrations/settings compatibility
and accepted known issues to a Malkuth release manifest.

## Cross-cutting measurements

- cold/warm launch, first editable text, RSS and idle wakeups;
- input/IME/caret/scroll p50/p95/p99/worst and allocations;
- load/find/replace/save for representative size/line/encoding corpora;
- undo memory and worst edit/reflow cost;
- color-hint latency, cancellation and stale-work cost;
- safe-write bytes/fsync/metadata/identity behavior;
- deterministic byte round trips and malformed-input rejection;
- native accessibility text-range and keyboard workflows.
