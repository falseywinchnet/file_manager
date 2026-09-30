# File Manager adaptive layout — 2026-09-29

Status: **MEASURED adaptive geometry/state regression checks passed; interactive visual acceptance remains open**.

## Authority and source comparison

**GIVEN:** Design DNA 006 DNA-13, DNA-C12 and DNA-A07 and
`visual/DESIGN_DNA_VERDICTS_007.md` DDV-007-02/03/14 require relationship-preserving
reflow, then priority collapse, readable text/targets, a recoverable 150×150
logical-unit minimum, and the existing permanent shelf membership. Menus remain
the complete non-contextual command vocabulary. Persistent tree and Selection
panes remain File Manager-specific under ADR-013; this does not import Paint's
application topology or ribbon.

**OBSERVED:** `C:/Users/Shadow/plan-paint/src/forms/ribbon.cpp`, `Ribbon::arrange`,
adapts group positions, chooses image tiers by available width and measures
captions. Its font shrinking to 8–10 units is not adopted here because the File
Manager interviews explicitly prioritize reflow/collapse before unusable targets.
Its retained command instances and bounded popup placement are useful patterns.
`EditorDialog::arrange` centers an owned panel and retains its controls; it is not
evidence that every dialog automatically fits every viewport.

**OBSERVED baseline gaps:** seven menu titles can exceed a narrow width; the
location row continues fitting a 90-unit navigation cluster, path and search
beside each other down to the minimum window; the shell has no declared vertical
collapse priorities; selected-object preview reserves 174 units even in short
windows. A screenshot at ordinary size does not verify these transitions.

## Bounded implementation choices

These breakpoints are **CANDIDATE implementation policy**, exercised by tests;
they do not amend the accepted product topology or promote an old research
candidate to an architecture decision.

- The authored shell collapses decorative title, permanent shelf, then status
  when its measured minimum tracks do not fit. Menu, location, and a minimum
  content region remain. Shelf commands remain available through the complete
  menu even when the whole shelf collapses. Focus on a collapsing shelf returns
  through the public ResponsiveTrackPanel fallback to the menu.
- The ordinary seven-category menu consolidates into one `Menu` entry when
  measured caption widths and padding no longer fit. Every category retains the
  same underlying command objects and state. Wider layouts restore geography.
- At least 720 logical units retain navigation buttons, path and search in one
  row. Between 420 and 720, with at least 360 units of height, path and query
  stack as separate retained fields. Navigation remains in Go and its shortcuts.
- The smallest or short/narrow layouts show one full-width field at a time.
  Go → Enter location / Search this subtree and Ctrl/Command+L/F reveal the
  relevant existing field. Draft text is not copied between path and search;
  no query is submitted merely by changing the composition. Unavailable search
  remains disabled under existing command authority.
- Below 560 units of window height, the selected-object preview becomes its
  compact identity header. The existing property control and selection remain;
  restoring height restores preview. An empty selection always stays compact.
- The folder tree minimum grows to 200 units so its caption/root selector are
  not forced into a 150-unit pane. Existing public split automatic/user collapse
  semantics remain authoritative.

All runtime composition changes use public GUI.Forms APIs over the generated
Web.Forms tree. No browser runtime, toolkit-private type, search protocol,
document-picker implementation, or new service is introduced.

## Verification and retained failures

The interaction suite now exercises 150×150, 360×260, 540×620, 800×320, 1340×850
and 1920×1080 at 1×, 1.5× and 2× device scale. It checks usable in-window content
and fields, distinct path/query state, exact selection retention, menu vocabulary,
compact preview, and restoration. These are headless geometry/state tests, not
desktop visual acceptance.

**REJECTED first implementation:** lowering minimum height alone did not reset a
retained requested height raised by a preceding stacked layout. A second DPI/
resize cycle retained a 79-unit location row, forcing the 150×150 content region
to 48.6667 units. The fix explicitly restores requested height as well as minimum
height for the location row and preview. The repeated-transition test is kept.

**MEASURED:** after the coherent Windows service-client source became available,
the focused application interaction suite passed in 2.16 seconds. A subsequent
fresh compile of all eleven frontend test targets passed **11/11 in 3.26 seconds**,
including the latest document-picker controller/view changes from their owner.
The application interaction suite took 2.28 seconds in that combined run. Exact
log: `../results/2026-09-29-shadow-windows/adaptive-ctest.txt`.

**MEASURED native performance control:** the test-owned actual File Manager
window ran against the exact installed/staged SDK digest
`e89cf95f7c5599c0f3718589f456847f5bdfdaae13ba2586a7e6d42a5ff11b9b`.
Native readiness was 147.805 ms; the first presented sample was observed at
330.475 ms. Steady full-window samples 2–4 took 38.797–39.777 ms at 2× DPI, with
140 controls and four measure passes. Final private bytes were 38,973,440; peak
working set was 53,563,392 bytes. It self-closed after five frames, with zero
input events or callback faults. This single run is not a universal frame-time
claim or visual fidelity acceptance. Exact log:
`../results/2026-09-29-shadow-windows/adaptive-native-full.txt`.

**OBSERVED build limit:** the side-by-side latency executable was opened by the
user during this work, so the final application link there returned Permission
denied. That instance was left alone. Fresh test targets and the separate native
benchmark linked and ran successfully; no old test executable is counted as
verification. The build/export owner is staging the application in the separate
`.build/native-windows-x64/frontend` tree to avoid replacing a running image.

The initial service-client header absence and the earlier 48.6667-unit content
failure remain disclosed. No private toolkit access, desktop input automation,
search execution from draft edits, or user-file mutation was needed.
