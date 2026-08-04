# M1a retained lifetime/event kernel experiment

Date: 2026-08-04

## Claim and rejection criterion

**HYPOTHESIS:** the existing GUI.Forms proving slice can gain explicit component
ownership, synchronous/idempotent disposal, tokenized subscription revocation,
UI-thread rejection, complete focus/capture/press cleanup, and deterministic
renderer-free traces without changing the renderer, AppKit host, DML schema, or
gallery-facing `Control::Ptr` API.

Reject this slice if any required lifecycle oracle fails; if a wrong-thread
operation mutates retained state; if two canonical trace runs differ; if idle
work remains after quiescence; if the core-only lane configures Objective-C++ or
links AppKit/Skia; or if an existing gallery/Skia/archive-policy gate regresses.

## Epistemic boundary

- **GIVEN:** GUI.Forms is retained, stateful, C++20, CPU-only, UI-thread-affine,
  and has no immediate-mode control API or perpetual frame loop.
- **GIVEN:** visual parents strongly own children; children hold weak visual
  parent links; detached controls require another strong owner; event ownership
  edges are tokenized and weak where a strong edge would create a cycle.
- **GIVEN:** component ownership is separate from visual parenting.
- **OBSERVED before this experiment:** the proving slice used `shared_ptr` /
  `weak_ptr`, virtual routed callbacks, stable IDs scoped to one `Window`, and
  unconditional Objective-C++/Skia/AppKit CMake configuration. Four existing
  CTest gates passed before edits.
- **CANDIDATE, unchanged:** the final C ABI and generational handle layout,
  public event ordering/exception policy, accepted host protocol, final
  headless-host contract, layout observability, renderer, and DML binary format.

The implementation rule tested here is: disposing a visual parent recursively
disposes its visually owned children. A separate `ComponentContainer` may own a
control without visually parenting it; disposing that container disposes all
components it owns. This is an executable M1a rule, not a stable ABI promise.

## Revision and dirty worktree

Repository HEAD at measurement: `56decf7851e7b090aaec8a5085c62545e858e6f5`.

The worktree was dirty before this experiment. Initial `git status --short`
showed a modified root `README.md` and untracked `AGENTS.md`, `gui_forms/`,
`planning/`, `plugin_runtime/`, `research/`, and `third_party/` trees. This
experiment changed only the M1a-authorized paths under `gui_forms/`; it did not
stage, commit, reset, clean, or rewrite unrelated work. Because `gui_forms/` is
untracked at repository level, HEAD alone does not identify its pre-experiment
contents.

## Environment

- Machine: `quentins-iMac.local`, arm64 (`T8122`)
- OS: macOS 14.8.7, build 23J520; Darwin 23.6.0
- Compiler: Apple Clang 16.0.0 (`clang-1600.0.26.6`)
- CMake: 4.2.3
- Build date: 2026-08-04, America/Chicago

## Correctness oracles

The executable tests cover:

1. component ownership independent from visual parenting;
2. visual strong-child/weak-parent lifetime and detached lifetime;
3. idempotent disposal, stable-ID removal, and rejected later mutation;
4. recursive visual-child disposal and container-owned component disposal;
5. owner-bound event revocation and the within-emission snapshot rule;
6. disposal/reparenting inside pointer callbacks;
7. disable, ancestor hide, removal, reparent, and disposal cleanup for focus,
   capture, pressed state, activation, key input, and text input;
8. wrong-thread mutation, dispatch, layout, and paint rejection with unchanged
   state;
9. duplicate stable-ID and parent-cycle rejection;
10. deterministic resize/scale/pointer/key/text/focus/reparent/dispose trace;
11. zero layout, paint, present, callback, frame, or wake work after quiescence;
12. all pre-existing gallery, Skia smoke, and archive-policy gates.

## Commands and raw results

### Pre-edit baseline

```sh
cmake --build /Users/quentinkuttenkuler/file_manager/gui_forms/build --parallel
ctest --test-dir /Users/quentinkuttenkuler/file_manager/gui_forms/build --output-on-failure
```

**MEASURED:** build passed; 4/4 tests passed, 0 failed, CTest reported 0.19
seconds total.

### Renderer-free core-only lane

```sh
cmake -S /Users/quentinkuttenkuler/file_manager/gui_forms \
  -B /tmp/gui-forms-core-m1a \
  -DGUI_FORMS_BUILD_GALLERY=OFF \
  -DGUI_FORMS_BUILD_TESTS=ON \
  -DGUI_FORMS_ENABLE_SKIA=OFF \
  -DGUI_FORMS_ENABLE_MACOS_HOST=OFF
cmake --build /tmp/gui-forms-core-m1a --parallel
ctest --test-dir /tmp/gui-forms-core-m1a --output-on-failure
```

**MEASURED:** configuration and build passed; 3/3 tests passed, 0 failed,
CTest reported 1.57 seconds total on the final rerun.

`otool -L /tmp/gui-forms-core-m1a/gui_forms_retained_lifetime_tests` listed
only `/usr/lib/libc++.1.dylib` and `/usr/lib/libSystem.B.dylib`. The core-only
cache contained no configured `CMAKE_OBJCXX_*` entry, and the build tree had no
Objective-C/Objective-C++ compiler directory. A symbol/source audit found no
Skia, AppKit, Objective-C, Metal, OpenGL, Vulkan, WebGPU, Ganesh, or Graphite
reference in public headers, core sources, or core tests.

### Full existing macOS/gallery lane

```sh
cmake -S /Users/quentinkuttenkuler/file_manager/gui_forms \
  -B /Users/quentinkuttenkuler/file_manager/gui_forms/build \
  -DCMAKE_BUILD_TYPE=Release
cmake --build /Users/quentinkuttenkuler/file_manager/gui_forms/build --parallel
ctest --test-dir /Users/quentinkuttenkuler/file_manager/gui_forms/build \
  --output-on-failure
```

**MEASURED:** gallery bundle and all targets built; 6/6 tests passed, 0 failed,
CTest reported 1.90 seconds total on the final rerun. The two added gates are
the lifetime/event test and canonical headless trace test; all four pre-existing
gates remained green.

### AddressSanitizer and UndefinedBehaviorSanitizer

```sh
cmake -S /Users/quentinkuttenkuler/file_manager/gui_forms \
  -B /tmp/gui-forms-core-m1a-sanitize \
  -DGUI_FORMS_BUILD_GALLERY=OFF \
  -DGUI_FORMS_BUILD_TESTS=ON \
  -DGUI_FORMS_ENABLE_SKIA=OFF \
  -DGUI_FORMS_ENABLE_MACOS_HOST=OFF \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build /tmp/gui-forms-core-m1a-sanitize --parallel
ctest --test-dir /tmp/gui-forms-core-m1a-sanitize --output-on-failure
```

**MEASURED:** 3/3 tests passed, 0 failed, with no ASan/UBSan report; CTest
reported 1.83 seconds total on the final rerun.

### ThreadSanitizer

```sh
cmake -S /Users/quentinkuttenkuler/file_manager/gui_forms \
  -B /tmp/gui-forms-core-m1a-tsan \
  -DGUI_FORMS_BUILD_GALLERY=OFF \
  -DGUI_FORMS_BUILD_TESTS=ON \
  -DGUI_FORMS_ENABLE_SKIA=OFF \
  -DGUI_FORMS_ENABLE_MACOS_HOST=OFF \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=thread -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=thread'
cmake --build /tmp/gui-forms-core-m1a-tsan --parallel
ctest --test-dir /tmp/gui-forms-core-m1a-tsan --output-on-failure
```

**MEASURED:** TSan was supported on this host; 3/3 tests passed, 0 failed, with
no TSan report. CTest reported 2.42 seconds total on the final rerun.

### Strict warning lane

```sh
cmake -S /Users/quentinkuttenkuler/file_manager/gui_forms \
  -B /tmp/gui-forms-core-m1a-werror \
  -DGUI_FORMS_BUILD_GALLERY=OFF \
  -DGUI_FORMS_BUILD_TESTS=ON \
  -DGUI_FORMS_ENABLE_SKIA=OFF \
  -DGUI_FORMS_ENABLE_MACOS_HOST=OFF \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-Wall -Wextra -Wpedantic -Werror'
cmake --build /tmp/gui-forms-core-m1a-werror --parallel
ctest --test-dir /tmp/gui-forms-core-m1a-werror --output-on-failure
```

**MEASURED:** 3/3 tests passed, 0 failed; CTest reported 1.59 seconds total on
the final rerun.

## Deterministic trace result

Two separate process runs of `gui_forms_headless_trace_tests` were compared with
`cmp` and hashed. Both outputs were 42 lines / 1,959 bytes and had SHA-256:

```text
7fd93035ae50971f7f8fa04f8a0dc3e6164fc8a9bd23978d36ca78ad55d0f99a
```

Representative tail:

```text
t=60 dispose id=trace.target state=disposed
t=60 pointer id=trace.root phase=preview action=up
t=60 pointer id=trace.right phase=preview action=up
t=60 pointer id=trace.right phase=target action=up
t=60 pointer id=trace.root phase=bubble action=up
paint draws=3
metrics focus=4 activation=1 disposal=1 focus_revoke=2 capture_revoke=1
t=1060 idle frame=0 wake=0 layout=0 paint=0 present=0 callback=0
gui_forms_headless_trace_tests: byte-identical replay passed
```

The hash command emitted a host-locale fallback warning because `C.UTF-8` was
not installed. It did not alter either captured trace; `cmp` succeeded and the
hashes matched.

## Failures and negative results

- No correctness, sanitizer, warning, renderer-free, or full-gallery gate
  failed in the recorded runs.
- The GUI was not launched. The assignment explicitly limited launch to visual
  failure diagnosis; the existing native gallery bundle was rebuilt and its
  automated interaction/raster gates passed.
- No performance budget was supplied and no latency, memory, launch, or binary-
  size claim is made. CTest wall times above are raw harness observations only.

## Scope of inference

**MEASURED within this slice:** the named M1a lifecycle rules execute
deterministically on the recorded macOS/Apple-Clang environment; rejected
wrong-thread operations leave the tested state unchanged; the tested callback
mutations are memory-safe under ASan/UBSan/TSan; the renderer-free lane neither
configures the macOS host nor links the renderer; and the existing gallery
continues to build and pass its tests.

This does **not** establish the final reference-count implementation, stable C
ABI, stale generational handles, dispatcher/timer semantics, public event order
or exception translation, accepted headless host protocol, cross-platform host
conformance, complete text/IME behavior, display chunks, renderer selection,
accessibility publication, performance budgets, or GUI.Forms 1.0 completeness.
Those remain later milestones and/or **CANDIDATE** decision gates.
