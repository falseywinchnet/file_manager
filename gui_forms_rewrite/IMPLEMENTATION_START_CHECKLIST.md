# GUI.Forms rewrite implementation-start checklist

Status: **ready for an explicitly directed sibling; no source implementation
authorized by this file**.

## 1. Confirm authority and overlap

- Read the parent and GUI.Forms `AGENTS.md` files and every file in this
  directory.
- Treat `ARCHITECT_SELECTIONS.md` as the closure authority over historical
  `OPEN` labels.
- Record branch, commit, full `git status --short`, toolchains, dependency
  configuration, and all overlapping user edits.
- Do not clean, reset, stash, or overwrite the dirty worktree.
- Stop if the proposed first batch overlaps user work that cannot be preserved.

## 2. Freeze the exact evidence snapshot

- Refresh the production file list and banned-construct inventory from
  `include/` and `src/`.
- Regenerate ownership evidence without acting on it:

  ```sh
  /usr/bin/python3 -B gui_forms_rewrite/tools/trace_ownership.py \
    --output gui_forms_rewrite/OWNERSHIP_AUDIT.md
  ```

- Generate the actual `compile_commands.json` for the configurations used by
  the semantic checker.
- Record public/native API declarations, C ABI tables, managed-facade
  assumptions, dependency/link boundaries, and existing file paths.
- Capture applicable ADR-014 lifecycle, event, focus, dispatch, rendering,
  accessibility, headless, and host traces before changing code.

## 3. Establish enforcement in observation mode

- Implement the standalone LibTooling checker in `ENFORCEMENT_SPEC.md`.
- Add cheap textual controls and inventory relevant warning profiles.
- Run without failing legacy source.
- Classify every finding by exact replacement role, not merely token spelling.
- Verify the WinForms-compatible Tag exception is symbol-bound and no broader.

## 4. Propose the first implementation batch

The selected first proof is the contiguous range/index binary-search toolbox.
The proposal must name:

- the new purpose-named private toolbox file(s);
- current lower-bound, upper-bound, and exact-search production call sites;
- explicit value/index/comparator types;
- empty, singleton, edge, duplicate, heterogeneous-comparator, and overflow
  behavior;
- standard-equivalence/property/fuzz controls;
- any minor native C++ API delta;
- exact production files touched first and supporting tests repaired afterward;
- rollback boundary and user-work overlap.

Do not include Event, general lambda removal, ownership, sorting, allocator,
or storage work in this first batch.

## 5. Obtain the start direction

Implementation begins only after the architect explicitly directs the sibling
to start and accepts the exact first-batch proposal. The planning package being
complete is not by itself that direction.

## 6. Required phase order after start

1. binary-search proof;
2. one behavior-rich Delegate/Event proof with legacy callback compatibility;
3. small production explicit-language waves;
4. selected sorting laboratory and evidence-bound call-site migration;
5. focused quantified `std::function` cleanup;
6. only earned large/non-growing heap-array cases;
7. supporting-source repair and O-032-A closure.

Every phase stays compilable, reversible, and behavior-checked. Current
shared/weak ownership remains untouched; `OWNERSHIP_AUDIT.md` belongs to a
future lifecycle round.

