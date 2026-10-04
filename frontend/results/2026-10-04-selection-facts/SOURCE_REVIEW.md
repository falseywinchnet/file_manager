# Source review

Standard: `planning/PROGRAMMING_HOUSE_STYLE.md`, complete document.

Independent read-only review: visible sibling **Audit File Manager Details
against interviews**, thread `01a0fb48-1437-7302-8b6f-f3bcf02e7319`.
The reviewer performed no edits, builds or Git changes.

Reviewed scope:

- Entire `frontend/src/selection_summary.hpp`.
- `application.cpp`: kind-text overload, selected-fact publication, browsing
  status summary, inspector assembly and removed detached-label updates.
- `application.hpp`: summary include and status overload declaration.
- `object_order_tests.cpp`: selection-observation check and invocation.
- `application_interaction_tests.cpp`: aggregate inspector check and invocation.
- `macos_preview_tests.mm`: selection stage, incoming transition and preview
  restoration before local actions.
- `.github/workflows/native-builds.yml`: selection screenshot artifact path.

The review found no correctness blockers. It checked explicit types, named
behavior, initialization, input borrows, overflow arithmetic, partial/unknown
states, and repeated-loop work. One style finding required `[[nodiscard]]` on
the three value-producing summary helpers. The primary thread added all three
attributes and verified the resulting declarations before the full passing
Windows build/test run. No other new violations were identified in this scope.

The follow-up corrected the initial description of authored labels as visible:
the facts container clears its children and installs the PropertyList; those
labels are detached. The updated tests inspect the live controls. The Mac
sequence restores preview expansion after its snapshot as required by later
assertions. This review does not certify unrelated legacy source, native pixels,
interactive latency or a production release.
