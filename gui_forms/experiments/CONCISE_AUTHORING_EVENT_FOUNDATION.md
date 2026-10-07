# Concise authoring: owner-held event foundation

Date: 2026-10-07. Scope: GUI.Forms only.

Baseline: file_manager `960d1c6450a5bdc32e67a17c14dbce99c3f056e5`.
Development branch: `codex/gui-forms-authoring-foundation`, based on the tested
Clang transition follow-up. The design/availability plan is
`../planning/CONCISE_AUTHORING_AVAILABILITY.md`.

## Implemented behavior

`on(event, owner, &Owner::method)` keeps a subscription token in its Component
owner, using the Component lifecycle explicitly rather than discovering
coincidentally named derived methods. Named member state lives in the event
slot. Disposal, destruction, callback exceptions, nested dispatch, disconnect
before a turn and defer-until-next-emission behavior are covered. Null members
are rejected and dead owners cannot register work. `own_subscription` transfers
an existing token; legacy `subscribe` still requires its caller to keep the
returned token alive.

Member binding borrows the owner. Retaining an active event slot keeps binding
state alive through revocation, not the owner's object after destruction. The
natural Component destructor now enters `disposing` before releasing callbacks,
so a callback-state destructor cannot register new work on that dying owner.

## Measurements

**MEASURED:** local M4 arm64, Apple Clang 21.0.0
(`clang-2100.1.1.101`), C++20, `-O3 -DNDEBUG`. The benchmark is
`tools/owned_event_lab.cpp`. Each event has one handler adding to a 64-bit
counter; setup is excluded. Each measured trial emits 1,000,000 events and
checks exactly 1,000,000 callbacks. Seven trials follow 10,000,000 warm-up
emissions per path. This is dispatch cost, not retained painting or host
presentation.

| Path | Median ns/emission | Min–max ns/emission |
|---|---:|---:|
| Baseline caller-owned Delegate | 8.25225 | 8.21946–8.28929 |
| Current caller-owned Delegate | 8.34488 | 8.32333–8.36525 |
| Current owner-held member | 7.38029 | 7.36279–7.40813 |

The existing path's measured median increased about 0.093 ns (1.1%) in this
run; the new member path was below both. These are short in-process samples,
not a statistical claim of a platform-wide improvement or regression. Raw
counts and durations are in `concise-authoring-events/before.csv` and
`concise-authoring-events/after.csv`.

**REJECTED comparison:** the initial 10,000-emission warm-up left a pronounced
descending timing trend across trials (baseline 19.68 to 8.71 ns). It cannot
support a speedup claim. Both original CSVs remain under
`concise-authoring-events/short-warmup-*.csv`; the corrected run above used the
longer warm-up after concurrent builds finished.

**MEASURED:** `sizeof(Component)` changes from 40 to 48 bytes on this ABI. The
additional pointer owns a lazy token vector. A fresh `on` registration takes
five allocations in the fixture: member slot, event slot storage, token-store
object, revocation storage and token storage. Registration failure is injected
at each allocation in turn. Every failed attempt leaves zero connected
callbacks and balanced statistics; a subsequent registration works.

**MEASURED:** 10,000 steady-state member emissions perform zero `operator new`
allocations. The fixture replaces ordinary, array and aligned allocation only
within its executable and scopes counting/fault injection outside setup,
assertions, logging and teardown.

**OBSERVED:** this implementation adds no scheduler request, timer activation,
posting or idle poll. Idle CPU, native paint costs, grid layout costs and skin
costs were not measured for this event-only change. No PlaySuite measurement or
adoption is claimed.

Reproduction: build/run `gui_forms_owned_event_lab` in Release. For the baseline,
compile the same lab source with `GUI_FORMS_EVENT_LAB_BASELINE` defined against
the baseline headers and `src/core/component/component/component.cpp`. That
flag omits only the new-helper case; the Delegate workload is identical. The
test executable `gui_forms_owned_event_tests` prints allocation/failure counts.

## Verification

- macOS arm64 Release, native Skia/HarfBuzz with prepared text and text masks:
  99/99 toolkit CTest tests passed, including the existing event tests and the
  new owner-held event tests. The compiled reference example also built.
- Separate renderer-free Debug build with AddressSanitizer and
  UndefinedBehaviorSanitizer: both focused event executables passed.
- Separate renderer-free Release SDK installation: the standalone reference
  program configured through `find_package(GUIForms)`, compiled, linked and
  passed its click/dispose test using only the installed public package.
- Windows and Linux qualification are recorded with the final PR checks;
  no result is implied by the macOS pass. CI also runs the standalone installed
  reference program on all three platforms.

The prepared-text development build intentionally refuses SDK installation.
This guard was encountered and respected; package validation uses a separate
configuration with that profile disabled.

The first installed-consumer compile found an older `/usr/local/include`
GUI.Forms header before the chosen SDK's system include directory. Header
tracing confirmed the mismatch. The standalone example uses
`NO_SYSTEM_FROM_IMPORTED` so its selected SDK is an ordinary explicit include
directory, preceding the compiler's older installation. No global installation
was changed or removed. The same example then compiled and ran successfully.

## House-style source review

Reviewed implementation scope: complete Component header/implementation,
Event header, extracted SubscriptionToken header, `owned_event_tests.cpp`,
`owned_event_lab.cpp` and `examples/reference/owned_events.cpp`. Also reviewed
the focused CMake additions and CI spelling-check command.

The review used `planning/PROGRAMMING_HOUSE_STYLE.md`: explicit types and
initialization; named callback state; Component authority; owner borrowing and
in-flight slot retention; move-empty tokens; deterministic disposal and
registration order; allocation-failure publication; and no per-emission
allocation. Raw addresses in the test are confined to replacement allocation
boundaries. The small benchmark's integer/duration conversions are explicit.
The spelling scanner reported zero findings for all seven C++ files. No
remaining violations were found in this reviewed scope; no whole-repository
or vendored-source compliance is asserted.

Compatibility: Component and private event storage layouts change. Rebuild the
toolkit and every C++ consumer coherently. No C ABI or frozen FM0 availability
claim changes. Grid, Options/attach, themes and authoring tools are still the
explicit remaining deliveries in the availability plan.
