# File Manager resume after the deadline pause

**GIVEN:** the owner resumes File Manager, asks for PR/build repair, and reserves
the active Notepad work to its sibling. Games is paused and its visuals belong
to another model. Do not resume Games or apply the historical shutdown order.

## Repository and CI evidence

**OBSERVED:** after fetching origin, `origin/main...6ec8712` reports zero commits
exclusive to main and 86 exclusive to the development branch. Main is already
an ancestor; rebasing would not incorporate additional work. PR #4 is open,
draft, and conflict-free. No merge into main has been performed.

**OBSERVED:** the two red checks on that head are cancelled, not failed tests:
push run 36963651158 was cancelled while manual run 36963650387 used the same
`native-refs/heads/codex/native-dogfood` concurrency group. PR run 36963654641
passed spelling checks but skipped native jobs because of its draft predicate.
The correction separates concurrency by event and runs native PR checks even
for drafts. New checks must finish before claiming the PR validated.

**MEASURED locally:** the workflow correction passed `git diff --check`;
12 house-style scanner tests passed, and the existing frontend plus C++
Orchestrator client scope (51 files) reported zero spelling findings. This is
not full semantic style acceptance or native platform validation.

## Product restart order

1. Complete native validation of the repaired PR workflow. Preserve failed and
   cancelled attempts as evidence; do not reinterpret a skipped matrix as a pass.
2. Resume the unfinished frontend source review from
   `frontend/results/2026-09-30-house-style/README.md`, especially effectful test
   assertions and remaining filesystem/conversion sequences. Review against
   the complete `planning/PROGRAMMING_HOUSE_STYLE.md`, not just its scanner.
3. Rebuild a matching current GUI.Forms/frontend pair and repeat the recorded
   File Manager navigation/repaint workload before changing performance code.
   Compare native presentation with the owner's prototype: breadcrumb material,
   adaptive chrome, object selection/full names, preview and keyboard behavior.
4. Advance indexed ordinary filename search through Engine and Orchestrator.
   Current ordinary substring queries use the live traversal lane; exact
   metadata queries use the catalogue. Preserve authoritative exact no-match
   behavior. Establish baseline workloads and negotiate indexed substring
   semantics before implementing or claiming acceleration.
5. Export the resulting coherent three-platform dogfood archives with matching
   receipts. Installer/signing and daily mutation readiness remain distinct
   from archive compilation and startup checks.

**OBSERVED:** the active Notepad sibling reports no shared-checkout edits and
continues in its own repository with SDK 723cd7f pinned. The prepared-window
candidate is not a finished public DocumentView. Neither that work nor the
optional Mac profiler establishes a fix for reported blank-window CPU use.

This record locates unfinished work; it does not accept a new architecture,
certify legacy style compliance, or promote a release.
