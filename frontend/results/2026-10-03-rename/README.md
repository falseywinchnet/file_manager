# Rename interaction alignment — 2026-10-03 UTC

**GIVEN:** `planning/ARCHITECTURE_INPUTS.md` requires the full extension visible
during inline rename, with only the basename initially selected.

**OBSERVED repair:** F2 and the shared Rename command retain the complete name,
selecting the prefix before its last nonempty extension. Dotfiles with only a
leading dot and extensionless files select their full name; `.config.json`
selects `.config`; `archive.tar.gz` selects `archive.tar`. Directory names remain
fully selected even when they contain dots. An empty trailing suffix is not
treated as an extension. The ASCII dot boundary is a valid UTF-8 selection byte
offset; no code-point/byte conversion or copied substring is required.

Submitting an unchanged name now closes the editor, restores object focus and
reports `Name unchanged`, without scheduling a filesystem rename or replacing
undo authority. A real name change keeps the existing revalidation/publication
path. Case-only rename, batch rename, cross-directory rename and invalid-name
policy are unchanged.

## Evidence

**MEASURED:** Shadow Windows GNU/MinGW Release with two compile jobs: all 13
CTest suites pass in 3.21 seconds. `WindowsLastTest.log` retains the run. The
assembled application test navigates an owned fixture, uses semantic selection
and F2/Enter, and verifies seven cases: ordinary extension, multiple suffixes,
extensionless file, dotfile, dotfile with extension, Unicode basename and dotted
directory. Unchanged input preserves filesystem revision, editor closure and
focus. A dispatched text event replaces `root` with `renamed`, commits the actual
rename as `renamed.txt`, and verifies retained object identity/selection after
folder refresh. The existing Ctrl+A test still selects the entire `root.txt`;
it was never an initial-selection assertion. The audit citation is corrected.

The first new fixture lookup mistakenly requested `fm.objects`; the authored
control is `fm.objects.current-folder`. It failed before behavior assertions.
Correcting the lookup passed the focused suite, then the final full suite after
the real keyboard-rename case was added. `InitialFixtureFailure.log` is retained.
No product workaround or disabled assertion was used.

Native Windows/macOS/Linux CI remains pending. These are retained-application
input tests, not physical keyboard dogfood or accessibility acceptance. Trailing-
dot names are source-reviewed behavior and are not in the portable fixture set
because ordinary Windows filename rules differ.

## House-style review

Reviewed `rename_basename_extent`, the changed `begin_rename` and `commit_rename`
lines, the named test probe, initial F2 assertion and complete new rename case
against `planning/PROGRAMMING_HOUSE_STYLE.md`. Explicit initialized types, byte
units, UTF-8 boundary reasoning, synchronous DirectoryEntry observation, no
retained borrows, operation order, focus restoration and no-op failure semantics
were checked. Test predicates borrow owners that survive the synchronous wait;
Application is stopped before fixture cleanup. Named case records keep filename,
expected selection and directory mode visible. No per-character allocation or
new callback is added. Two C++ files have zero spelling candidates; unchanged
Application, tests and GUI.Forms are not certified by this scoped review.
