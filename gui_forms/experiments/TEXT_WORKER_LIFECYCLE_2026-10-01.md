# Private native shaping worker proof — 2026-10-01

## Status and exact scope

**OBSERVED:** after integrating per-call reuse as `aa1f8d4`, the coordinating
chat authorized this private lifecycle proof. The new files are
`src/render/text/worker_probe/worker_probe.hpp`, its `.cpp`, and
`benchmarks/text_worker_probe.cpp`, with the OFF-by-default CMake target
`GUI_FORMS_BUILD_TEXT_WORKER_PROBE`. No production host, Painter, public API,
installed SDK, frozen checkpoint or application stage changed.

This diagnostic is a measured-capacity native experiment, not a hard-quota
service or a selected scheduling architecture. The ordinary Shadow application
profile still has HarfBuzz OFF; this record is not a current File Manager or
SwiftEdit executable responsiveness result.

## Ownership and state

One named `std::thread` entry creates, registers, uses and destroys its own
HarfBuzz engine and mutable FT/HB objects. Only immutable encoded-font owners,
result-local face IDs and copied text/glyph records cross the boundary. Font
owners originate as `make_shared<const ...>` without mutable aliases. The
result retains exact encoded bytes, face index and registered configuration.
No painter or paint-side native face is implemented by this proof.

One mutex protects queued/running/ready slot transitions, completion status,
desired intent and publication authority. Control methods, `displayed()` and
close belong to one foreground executor; the harness does not authorize
concurrent control callers. The displayed borrow ends on successful publication
or destruction; close currently preserves the displayed result.

The replacement slot stays occupied from queued input through native completion
and retained ready output. Cancellation clears desired publication authority
without freeing the slot or interrupting the native call. Desired intent stores
only one replaceable identity record. Input allocation/copy occurs only after
slot admission, with a 16,384-byte limit. Completed stale/failed work requires
explicit discard before another job can enter; successful publication moves
the immutable result into the separately retained displayed owner.

Publication compares document/revision, page serial, source interval, viewport,
layout serial, provider instance/generation, font-set identity/generation,
context generation, effective font, scale, wrap width and tab policy. The probe
admits only scale 1, no wrap and four-column tab metadata; it does not implement
wrapping or tab geometry. Twenty-two changed-key fixtures exercise comparison.
These keys carry context metadata; this harness does not establish a D1 source
mapping or source-grapheme proof.

Source review found a cancellation-revival edge before the timed run:
`cancel()` followed by a new desire with the exact old identity could authorize
the old completion. Each valid desire now advances a private checked uint64
authority epoch, captured by the admitted job and result. Publication requires
both full identity and epoch. Invalid desires and busy submission retries do not
advance it. At maximum epoch, further desire is refused without wrapping;
exhaustion is source-reviewed, not directly executed by a test.

Close first stops admission and clears desired authority, then waits for
noninterruptible work, joins the thread and releases unpublished input/result.
Native engine destruction completes on the worker before close reports joined.
This synchronous close can block the foreground; it is not an instantaneous
cancel/close promise or a production UI shutdown policy.

## Executed fixtures and measurement

**MEASURED:** the Release diagnostic built and exited 0 with empty stderr. It
uses the same toolchain, machine, exact four fonts and pinned text libraries as
`TEXT_LAYOUT_NATIVE_PROBE_2026-10-01.md`, with text diagnostics and geometry trace
OFF. Each execution had an external 60-second process deadline; internal waits
also check ten-second limits. There was no timeout or failed fixture result.

Fixtures exercised:

- Initial successful display; cancelled in-flight work retains its slot.
- Cancel, desire the exact same key, then refuse the old completion by epoch.
- Coalesce desired metadata, reject stale publication, preserve old display,
  retain the ready slot until discard, then admit and publish the newest intent.
- Missing primary-font role produces a failed completion while preserving old
  display. The failed slot is released before a later real shaping job succeeds.
- Close before start, idle close, ready-result close, repeated close, and close
  while native shaping is running; admission is refused after close.
- Invalid font-generation intent and oversized input leave the previous valid
  authority/empty slot available for a valid retry.
- Displayed geometry and its immutable encoded-font lease remain available after
  the worker engine is destroyed. This does not prove an independent painter.

The slow input is the complete 16,384-byte mixed Arabic/Hebrew/Latin fixture.
The foreground repeatedly snapshots state and services a counter, with a
requested one-millisecond sleep between ticks. This is a foreground scheduling
probe, **not a native-window event/paint/input responsiveness test**. The sleep
request does not promise a one-millisecond timer resolution or deadline.

| Run | Foreground ticks | Ticks observing running | Maximum gap ms | Pending elapsed ms | Close/drain/join ms |
| --- | ---: | ---: | ---: | ---: | ---: |
| Initial | 7 | 6 | 17.6227 | 111.028 | 81.9916 |
| Confirmed slot | 7 | 6 | 18.9779 | 107.543 | 90.2690 |

Five jobs completed in the main lifecycle scenario. The two CSV files are
`TEXT_WORKER_LIFECYCLE_2026-10-01.csv` and
`TEXT_WORKER_LIFECYCLE_confirmed_2026-10-01.csv`. SwiftEdit's desktop-release
message arrived as the first probe returned, prompting one follow-up run after
confirmation. SwiftEdit subsequently confirmed its smoke had ended before the
first probe as well. Both records are retained; other development/system load
was not isolated. No more measurements were required for this slice.

The foreground did progress during actual native shaping, but its measured
maximum gap was approximately 19 ms. The roughly 90 ms drain demonstrates the
material shutdown cost rather than hiding it behind cancellation language.

Post-close retained capacity in both runs: text 16,384 bytes, run vector 4,096
elements, glyph vectors 10,752 elements in total, and shared encoded fonts
1,314,192 bytes. These exclude allocator overhead, owner objects, temporary
native allocations, live native font caches and peak simultaneous retained
output. This is not an RSS/peak-memory measurement or a hard bound. Independent
paint-side cache memory remains unknown.

## Validation and source review

Build command: configure `GUI_FORMS_ENABLE_HARFBUZZ_TEXT=ON` and
`GUI_FORMS_BUILD_TEXT_WORKER_PROBE=ON`, build `gui_forms_text_worker_probe`, then
run the executable with the `gui_forms/assets/fonts` directory argument. The
existing `.build/house-style-text` Release projection was used; the target is
standalone and is not added to the default build when its option is OFF.

Three-file spelling scan reported zero findings. Strict syntax checking of both
new `.cpp` files passed with `-Wall -Wextra -Wconversion -Wsign-conversion
-Werror`, treating existing public headers as system includes. `git diff
--check` passed. This proves neither thread-sanitizer nor leak-checker coverage;
neither tool was run in this slice.

Semantic review against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`
covers all three new C++ files and the narrow CMake option/target. Review covered
explicit types, full identity fields, named thread entry, initialized retained
state, checked epoch growth, input admission before allocation, native-owner
executor/lifetime, move ownership, foreground borrow lifetime, lock boundaries,
exception containment during engine setup and each job, cleanup before slot
release, and repeated-loop storage. Parent independently reviewed the initial
source, identified the epoch edge, and reviewed the correction and lifecycle
extensions. No remaining house-style violation was identified in this scope.
No whole-repository, legacy renderer or dependency compliance claim is made.
Parent's final style review requested typed `Job&`/`Prepared&` local borrows for
repeated owner access in submit/run. These were applied after measurement;
borrows are not used after owner moves/resets and the run borrows leave the try
scope before failure cleanup. The target rebuilt, strict syntax checking and
the three-file scan passed again. No timing resample was taken for this
equivalent-operation readability change. The raw runs use worker SHA-256
`87dcc411f9e8361146c4e0df31eea071bbd9d39f8c0309da04d27bb021c05061`.

Source SHA-256:

- Header: `a0e4fe7b973ef61f62250feee416225918ab7d8c7891a60fb684c56bbef9dcb0`
- Worker: `78feff8d80cd025722ef1efd5c7f457e82561d72d212999376e980a0aa9faaa3`
- Probe: `97bb88ee5359a0c99d669437eb5e7a78f2d485b61f08548c0b7fd5a9704312b0`

Remaining integration work includes a reconciled prepared-layout contract,
paint-side face/glyph/config compatibility refusal, real host publication and
shutdown behavior, native input/paint responsiveness, full resource accounting,
and matched SDK/consumer adoption. No such capability is advertised here.
