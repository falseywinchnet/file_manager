# GUI.Forms sibling-thread handoff

Status: **paste-ready first implementation assignment**. This is a bounded
M1a slice from `MASTER_IMPLEMENTATION_PLAN.md`, not authorization to implement
the entire framework or accept architecture decisions.

## Instructions to the grand architect

Open a new sibling Codex task rooted at:

```text
/Users/quentinkuttenkuler/file_manager
```

Paste the complete prompt below without shortening it. The sibling should work
in the same repository/worktree. Do not ask it to “finish GUI.Forms”; this first
assignment establishes the lifetime/event/conformance substrate on which later
parallel work depends.

## Paste-ready sibling prompt

```text
You are implementing the next bounded GUI.Forms proving slice in:

  /Users/quentinkuttenkuler/file_manager

Objective
---------

Implement M1a, “reusable lifetime/event kernel and deterministic headless
traces,” without expanding the control catalogue or selecting unresolved
architecture. At the end, the existing GUI.Forms Gallery must still build and
behave as before, while the renderer-free core has executable contracts for
component ownership, visual parenting, deterministic disposal, tokenized event
subscriptions, UI-thread affinity, focus/capture cleanup, and idle behavior.

This is an implementation task. Diagnose, edit, test, and leave the bounded
slice working. Do not merely write another plan.

Required reading before any edit
--------------------------------

Read these files completely and follow the nearest AGENTS.md:

1. /Users/quentinkuttenkuler/file_manager/AGENTS.md
2. /Users/quentinkuttenkuler/file_manager/planning/README.md
3. /Users/quentinkuttenkuler/file_manager/planning/PREPLAN_CHARTER.md
4. /Users/quentinkuttenkuler/file_manager/planning/EVIDENCE_REGISTER.md
5. /Users/quentinkuttenkuler/file_manager/planning/DECISION_PROTOCOL.md
6. /Users/quentinkuttenkuler/file_manager/planning/SURFACE_PIPELINE.md
7. /Users/quentinkuttenkuler/file_manager/planning/gui_forms/GUI_FORMS_INTERVIEW_LEDGER.md
8. /Users/quentinkuttenkuler/file_manager/gui_forms/AGENTS.md
9. /Users/quentinkuttenkuler/file_manager/gui_forms/README.md
10. /Users/quentinkuttenkuler/file_manager/gui_forms/PROVING_SLICE.md
11. /Users/quentinkuttenkuler/file_manager/gui_forms/planning/MASTER_IMPLEMENTATION_PLAN.md
12. /Users/quentinkuttenkuler/file_manager/gui_forms/planning/CONTROL_COMPLETENESS_MATRIX.md
13. all current public headers, core sources, tests, demo control sources, and
    CMake under /Users/quentinkuttenkuler/file_manager/gui_forms, excluding the
    fetched third_party/skia source tree.

Before editing, run and record:

  git status --short
  cmake --build /Users/quentinkuttenkuler/file_manager/gui_forms/build --parallel
  ctest --test-dir /Users/quentinkuttenkuler/file_manager/gui_forms/build --output-on-failure

The worktree already contains user work and untracked project files. Preserve
all unrelated changes. Do not stage, commit, reset, clean, delete, or rewrite
another person's files.

Epistemic constraints
---------------------

Use the repository labels exactly:

- GIVEN: GUI.Forms is retained, stateful, C++20, CPU-only, no immediate-mode
  control API, no .NET/Java/browser/Godot dependency, no Skia/platform type in
  a public seam, no upward File Manager dependency, and no perpetual frame loop.
- GIVEN: parent-to-child ownership is strong, child-to-parent is weak, detached
  controls live only while externally retained, and event subscriptions are
  tokenized and weak by default where a strong edge would create a cycle.
- GIVEN: eventual public binary identity is generational opaque C handles, but
  this slice must NOT freeze or pretend to implement the stable C ABI.
- OBSERVED: the spike currently uses shared_ptr/weak_ptr and virtual callbacks;
  preserve source compatibility for the existing gallery during this slice.
- CANDIDATE: exact public event ordering, layout observability, host protocol,
  renderer choice, ABI layout, and DML binary format remain unresolved. Do not
  mark them DECIDED or encode a new compatibility promise.

Scope: implement these deliverables
----------------------------------

1. Separate component ownership from visual parenting.

   Add a small renderer-neutral Component/ComponentContainer substrate.
   A container strongly owns components but does not visually parent them.
   Controls can be component-owned and independently attached/detached in the
   visual tree. Container disposal deterministically disposes owned components.
   Disposal is idempotent and observable through a renderer-neutral state.

   Preserve current Control::Ptr source compatibility. Do not replace the
   public spike with raw pointers, a garbage collector, or the final C ABI in
   this task.

2. Add deterministic control disposal semantics.

   A disposed control must immediately:

   - stop being eligible for hit testing, focus, capture, and activation;
   - detach from its visual parent/window identity registry;
   - revoke its event subscriptions and queued/timer tokens owned by this
     substrate;
   - release or dispose owned children according to one documented, tested
     component/visual ownership rule;
   - reject later state mutation in development builds without corrupting the
     tree;
   - make a second dispose call a no-op;
   - remain memory-safe when disposal happens inside an event callback.

   Do not invent plugin process behavior or C# finalizer behavior here.

3. Add a reusable tokenized event primitive.

   Implement renderer-neutral Event/SubscriptionToken behavior usable by later
   controls. It must support deterministic subscribe, disconnect, owner-bound
   revocation, safe subscription/unsubscription during emission, and no strong
   ownership cycle by default. Define and test the within-emission snapshot
   rule. Keep existing virtual on_* overrides working; migrate only enough code
   to prove the primitive without rewriting every event route.

   Do not settle the final C ABI callback record or claim strict WinForms event
   ordering. Do not silently swallow callback exceptions. If exception policy
   beyond tree-consistency requires a decision, preserve invariants, document
   the observed behavior, and stop short of declaring a public policy.

4. Enforce the UI-thread domain in the renderer-neutral core.

   Capture/bind the owning UI thread for a Window/application context. Reject
   cross-thread visual-tree mutation, focus/capture operations, event dispatch,
   layout, and paint in a deterministic testable way. The wrong-thread attempt
   must leave state unchanged. Do not add a general multithreaded control model.
   A full dispatcher/Invoke API is a later slice; add only the minimum test
   support needed to prove affinity and safe rejection.

5. Make focus/capture/press cleanup complete.

   Verify and, where required, fix disposal, disabling, hiding, removing, and
   reparenting of the focused/captured/pressed control. No subsequent release,
   text, or key event may target a dead/ineligible object. Record deterministic
   focus-transition and activation counts.

6. Add deterministic headless lifecycle/event traces.

   Under tests/support or an equally private test namespace, create a fake
   monotonic clock and trace recorder. Drive the real Window/control APIs with
   fixed resize, scale, pointer, key, text, focus, reparent, dispose, and idle
   sequences. Emit canonical line or JSON traces with no pointer addresses,
   wall-clock timestamps, random IDs, or platform-specific data. Running the
   same trace twice must produce byte-identical output.

   This is test support, not yet the accepted public host protocol. Do not call
   it the final headless host unless an ADR later accepts that contract.

7. Keep a renderer-free core-only build path.

   Refactor CMake only as much as necessary so this command configures, builds,
   and tests the core/lifecycle/trace lane without Objective-C++, AppKit, Skia,
   third-party fetches, or the gallery:

     cmake -S /Users/quentinkuttenkuler/file_manager/gui_forms \
       -B /tmp/gui-forms-core-m1a \
       -DGUI_FORMS_BUILD_GALLERY=OFF \
       -DGUI_FORMS_BUILD_TESTS=ON \
       -DGUI_FORMS_ENABLE_SKIA=OFF \
       -DGUI_FORMS_ENABLE_MACOS_HOST=OFF
     cmake --build /tmp/gui-forms-core-m1a --parallel
     ctest --test-dir /tmp/gui-forms-core-m1a --output-on-failure

   The project declaration should require only CXX. Enable OBJCXX and discover
   Apple frameworks only inside the admitted macOS-host branch. Keep the current
   full macOS gallery configuration working. Do not redesign the Skia scripts,
   update the Skia pin, or change PNG/GPU policy.

8. Add structured diagnostics needed by this slice.

   Extend MetricsSnapshot only with well-defined counts useful here, such as
   disposals, rejected wrong-thread operations, subscriptions connected/
   disconnected, callbacks emitted, and focus/capture revocations. Existing
   metrics and JSON output remain backward compatible within this spike.
   Do not call a counter “display chunks rebuilt” evidence of retained chunks;
   actual display chunks remain later work.

9. Write one experiment record.

   Add:

     /Users/quentinkuttenkuler/file_manager/gui_forms/experiments/M1A_RETAINED_KERNEL.md

   It must state claim, GIVEN/OBSERVED/CANDIDATE boundaries, exact revision and
   dirty-worktree note, environment, commands, correctness oracles, raw test
   results, failures/negative results, and scope of inference. Do not invent a
   speed claim or budget.

Files you may create or modify
------------------------------

You may modify only these project areas:

- gui_forms/CMakeLists.txt
- gui_forms/include/gui_forms/component.hpp                 (new)
- gui_forms/include/gui_forms/event.hpp                     (new)
- gui_forms/include/gui_forms/control.hpp
- gui_forms/include/gui_forms/window.hpp
- gui_forms/include/gui_forms/metrics.hpp
- gui_forms/include/gui_forms/gui_forms.hpp
- gui_forms/src/core/component.cpp                          (new)
- gui_forms/src/core/event.cpp                              (new if needed)
- gui_forms/src/core/control.cpp
- gui_forms/src/core/window.cpp
- gui_forms/src/core/metrics.cpp
- gui_forms/tests/core_tests.cpp                            (only compatible extensions)
- gui_forms/tests/retained_lifetime_tests.cpp               (new)
- gui_forms/tests/headless_trace_tests.cpp                  (new)
- gui_forms/tests/support/**                                (new)
- gui_forms/experiments/M1A_RETAINED_KERNEL.md              (new)

You may make the smallest necessary compatibility edit to
gui_forms/src/controls/gallery_controls.* or gui_forms/demo/* only if the new
kernel otherwise breaks the existing gallery. Explain every such edit. Do not
redesign the gallery.

Files and areas you must not touch
----------------------------------

- anything outside /Users/quentinkuttenkuler/file_manager/gui_forms
- gui_forms/src/render/skia/**
- gui_forms/src/host/macos/**
- gui_forms/third_party/**
- gui_forms/planning/**
- File Manager sources or planning
- generated/fetched Skia sources and current build artifacts

Use apply_patch for source/document edits. Preserve user changes. Do not use a
destructive git command.

Required tests and acceptance criteria
--------------------------------------

Add tests proving at minimum:

1. component ownership is independent from visual parenting;
2. parent-to-child visual ownership and weak parent links do not form a cycle;
3. detached externally retained controls survive, and unretained ones die;
4. disposal is idempotent and removes stable-ID lookup;
5. disposing a parent gives the documented result for owned and merely visual
   children;
6. disposing/reparenting inside a callback does not use freed state;
7. an owner-bound event subscription disconnects on owner disposal;
8. subscribe/unsubscribe during emission follows the documented snapshot rule;
9. disabling, hiding, removing, reparenting, or disposing a focused/captured/
   pressed control revokes ineligible state and prevents later activation;
10. a wrong-thread mutation/dispatch/layout/paint is rejected and leaves the
    tree and metrics internally consistent;
11. two runs of the canonical lifecycle/event trace are byte-identical;
12. the fixed idle sequence performs no layout, paint, present, callback, or
    wake work after quiescence;
13. duplicate stable IDs and parent cycles still fail safely;
14. every pre-existing core, gallery, Skia smoke, and archive-policy test still
    passes in the full configuration.

Run and report both lanes:

Core-only lane:

  cmake -S /Users/quentinkuttenkuler/file_manager/gui_forms \
    -B /tmp/gui-forms-core-m1a \
    -DGUI_FORMS_BUILD_GALLERY=OFF \
    -DGUI_FORMS_BUILD_TESTS=ON \
    -DGUI_FORMS_ENABLE_SKIA=OFF \
    -DGUI_FORMS_ENABLE_MACOS_HOST=OFF
  cmake --build /tmp/gui-forms-core-m1a --parallel
  ctest --test-dir /tmp/gui-forms-core-m1a --output-on-failure

Full existing macOS lane:

  cmake -S /Users/quentinkuttenkuler/file_manager/gui_forms \
    -B /Users/quentinkuttenkuler/file_manager/gui_forms/build \
    -DCMAKE_BUILD_TYPE=Release
  cmake --build /Users/quentinkuttenkuler/file_manager/gui_forms/build --parallel
  ctest --test-dir /Users/quentinkuttenkuler/file_manager/gui_forms/build --output-on-failure

Also compile/run the new core tests with AddressSanitizer and UndefinedBehavior
Sanitizer when supported by the local compiler. If ThreadSanitizer cannot run
reliably on the host, record that as a negative/environment result rather than
claiming it passed.

Do not launch the GUI unless a test failure requires inspection. If you launch
it, inspect it and close it; leave no process running.

Stop and report instead of guessing if
--------------------------------------

- the task requires removing shared_ptr from the public spike or freezing the
  final C ABI/handle bit layout;
- a correct disposal rule conflicts with a previously recorded GIVEN rule;
- event exception behavior or callback order requires a public compatibility
  decision rather than an internal invariant-preserving implementation;
- you need to edit renderer, AppKit host, Skia pin/patch, DML schema, C# bridge,
  theme/language pack, or files outside the allowed list;
- the clean baseline tests fail before your edits for a reason unrelated to
  your task;
- user/unrelated work overlaps an intended edit and cannot be preserved;
- a test would require the unavailable Portmouth Rapids asset;
- a destructive migration or cleanup would be necessary.

Final report
------------

Lead with the outcome. Give:

- exact changed paths;
- exact build/test/sanitizer commands and pass/fail counts;
- the canonical trace artifact or representative excerpt and determinism proof;
- any measurements, labelled MEASURED with environment and scope;
- negative results and limitations;
- every remaining CANDIDATE/decision gate encountered;
- confirmation that no process remains and no out-of-scope file was changed.

Do not commit, push, open a PR, or claim GUI.Forms is complete.
```

## Expected next task after M1a

Do not pre-authorize this in the M1a thread. After review, the likely next
bounded assignment is M1b: typed invalidation declarations, cached affected-
subtree layout, damage compaction, and truthful visit/chunk metrics. That task
depends on the M1a lifecycle and trace contract remaining stable.

