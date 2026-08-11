# GUI.Forms exact-behavior rewrite completion report

Status: **COMPLETE on 2026-08-10 under O-032-A**.

The first-party C++ rewrite outside experiments is complete. GUI.Forms remains
a C++20 project compiled with current toolchains, but its authored source uses
the selected explicit house subset. This was not an STL purge, ownership
redesign, allocator project, firmware port, dependency reorganization, or
private-standard-library project.

## Phase status

| Phase | Result | Closure evidence |
|---|---|---|
| 0. Snapshot and semantic checker | Complete | Standalone LibTooling inventory/closure checker, exact compilation databases, rewrite-plan validation, and black-box negative/admitted fixtures. |
| 1. Binary-search proof | Complete | Named contiguous index operations replaced every production lower/upper/exact search; standard controls remain in equivalence tests. |
| 2. Delegate/Event proof | Complete | Named non-owning Delegate binding, Delegate-first Event overloads, preserved legacy owning callback overload, behavior tests, and allocation measurements. |
| 3. Explicit-language waves | Complete | Production first, then first-party tests, demos, tools, platform compatibility code, and consumers required by O-001. |
| 4. Sorting tournament | Complete | Three selected house candidates measured against standard controls; one bounded call site migrated and losing/uncertain cases retained the standard implementation. |
| 5. `std::function` audit | Complete | 208 explicit spellings classified by role; allocation-heavy synthetic behavior quantified; no speculative general replacement admitted. |
| 6. Specialized storage | Complete with no implementation | No present large, non-growing, edited-in-place allocation earned a BFFT-style heap array. `std::vector` remains the ordinary growable contiguous container. |
| 7. Supporting repair and closure | Complete | Native, no-HarfBuzz, MinGW, selected Wine tests, managed facade consumers, ABI tests, and all semantic inventories passed. |

## O-032-A acceptance

### 1. House grammar

The final semantic inventories report zero violations:

| Configuration | Translation units | Findings | Violations |
|---|---:|---:|---:|
| Native macOS, HarfBuzz enabled, complete first-party scope | 247 | 607 | 0 |
| Native macOS, HarfBuzz disabled | 240 | 580 | 0 |
| Windows x64 MinGW | 242 | 592 | 0 |
| Checker implementation itself | separate self-inventory | 0 | 0 |

Native and MinGW were rerun in fatal `closure` mode; both exited successfully.
The no-HarfBuzz branch was collected in non-fatal inventory mode and contains
the same zero-violation result.

Findings are admitted or explicitly reviewed constructs, not suppressed
matches. The only approved `std::any` findings are the symbol-bound O-011 Tag
surface. Seven reviewed designated initializers are plain configuration
records. `CXX20_FEATURE_AUDIT.md` records the complete retained C++20 surface.

The checker forbids source-spelled `auto`/`decltype(auto)`, pointer-member
arrow, trailing return, lambdas, structured bindings, coroutines, ranges,
requires/concepts, ordinary `decltype(expression)`, unapproved `std::any`, and
defaulted comparison machinery. Its fixture suite proves that the negative
sample is rejected and the admitted sample is accepted.

### 2. Approved native API deltas

The germane native C++ deltas are:

- `Event` has Delegate-first subscription overloads while preserving the
  legacy `std::function` owning-callback overload.
- Named Delegate/bound-member support exposes non-owning binding and makes the
  lifetime decision separate from callback invocation.
- Image and raster result records use named `success`/`failure` factories
  (`PngValidationResult`, `ImageLoadResult`, and private `DecodeResult`) instead
  of stateful designated initialization.
- Purpose-named private binary-search and sorting toolbox files were added.

No C ABI ownership or entry-point contract changed. C11 and C++ ABI tests pass
on native macOS and under Wine from the MinGW build. Generated managed facade,
smoke, and behavior-consumer projects compile with zero warnings or errors.

### 3. Files and dependent boundaries

Existing source paths and component boundaries remain. New files are limited to
the approved private algorithms, named callback/test support, semantic checker,
tests, and measurement tools. GUI.Forms did not gain a BFFT link dependency,
private-runtime hierarchy, allocator propagation layer, ownership framework,
concurrency facade, firmware target, or fake portability layer. Skia,
HarfBuzz/text, host, C ABI, and managed-facade boundaries remain in their prior
roles.

### 4. Behavior and consumer gates

The final Release native build completed and all **67 of 67** configured tests
passed. This includes retained lifetime, event, dispatcher, scheduling,
rendering, headless trace, focus, layout, accessibility/semantic, host-boundary,
macOS multi-window/close, C ABI, and drawing ABI controls.

The no-HarfBuzz Release configuration built and passed **64 of 64** configured
tests. The Windows x64 MinGW/Skia tree built all targets. The following selected
executables then passed under Wine: binary search, sort algorithms,
Delegate/Event, C11 ABI, C++ ABI, Windows GDI compatibility, Windows live
surface, and byte-identical headless trace.

The generated managed facade and both managed smoke consumers built with zero
warnings/errors. The native headless facade smoke passed callback, dispatch,
fault, disposal, and retained-tree checks. Ten managed behavior modes passed:
form/owner semantics, secondary forms, cursors, dock/padding, geometry/tab
order, layout transactions, scrolling, split containers, property grids, and
lifecycle order.

### 5. Delegate/Event and retained `std::function`

First-party direct member Event subscriptions are Delegate-first. Event slot,
snapshot, registration order, mutation-during-emission, revocation, token,
exception, and statistics behavior remain intact and are tested.

On the M4 Release callback laboratory, Delegate is 16 bytes and binds with zero
allocation. A small libc++ `std::function<void(int)>` is 32 bytes and also binds
without allocation; a 200-byte named owning target allocates once. Event
snapshot copies account for further measured allocations. Delegate was chosen
for explicit non-ownership and zero binding allocation, not from an unsupported
nanosecond claim. `STD_FUNCTION_AUDIT.md` classifies every retained explicit
spelling; distinct dispatch, scheduling, host, registry, command, render, and
owning lifetime semantics were not collapsed into Event.

### 6. Sorting and binary search

`SORT_AND_SEARCH_AUDIT.md` closes every production sort/stable-sort call site.
Stable insertion was selected only for `CorrespondenceView`'s exact maximum of
three role indices. Standard sort/stable-sort remains selected for all other
sites because it won, the distribution was not measured, or a house candidate
had a material input-dependent regression. House introsort and the adaptive
stable candidate won no admitted corpus; those negative results remain in
`SORT_LAB_M4_RESULTS.csv`.

All production lower-bound, upper-bound, and exact binary-search operations use
the named house index core. Tests cover empty/singleton/boundary/duplicate,
heterogeneous comparator, constexpr, deterministic generated, and standard-
equivalence cases.

### 7. Tag compatibility exception

`Tag` is inert app-owned metadata and, where deliberately used, a lifetime
anchor. `Control`, `ImageList`, `ErrorProvider`, and `HelpProvider` expose their
existing native `tag()`/`set_tag(std::any)` compatibility surface; storage is
cleared during disposal. It does not participate in rendering, layout,
dispatch, ownership discovery, or generic service lookup.

First-party dogfooding stores explicit shared context/lifetime objects in a root
control Tag and retrieves the exact type with `std::any_cast`. That use explains
why the exception is allowed: it supplies WinForms-compatible arbitrary
metadata without making `std::any` ordinary architectural vocabulary. No new
Tag-bearing type or non-Tag `std::any` use is admitted.

### 8. Preservation and deferred evidence

Shared/weak retained ownership, exceptions, RTTI, threads, atomics, clocks,
scheduling, strings, streams, filesystems, locales, and hosted dynamic loading
retain their existing semantics. `OWNERSHIP_AUDIT.md` was regenerated by the
checked-in Python tracer, including support source. It is evidence for a future
lifecycle round and was not used to redesign ownership now.

No heap-array use was admitted because no current site met the required
large/non-growing/edited-in-place facts. No standard heap algorithm was
introduced. The allocation and sorting laboratories preserve negative results
as required.

## Known non-blocking debt and separate future rounds

- The macOS host emits seven macOS 15 CoreVideo display-link deprecation
  warnings. They are observable host/API migration debt, not rewrite-created
  language violations. They should be resolved in a dedicated host change with
  display timing behavior retested.
- GUI.Forms has ASan/UBSan-oriented fuzz and mutation controls, but no CBMC
  proof. `CBMC_AUDIT.md` states the honest gap and proposes binary search as the
  first bounded feasibility harness with explicit safety properties and
  unwinding assertions. This is not silently counted as rewrite coverage.
- Ownership redesign, hive/stable-pool experiments, allocator work,
  constrained-runtime/firmware work, and further algorithm migrations remain
  unopened projects. Their deferral is part of completion, not an incomplete
  rewrite phase.
