# Editor, breadcrumb, and menu house-style review

Status: **OBSERVED** source review; runtime validation recorded below.

## Authority and scope

**GIVEN:** the owner requested a semantic rewrite against
`C:/Users/Shadow/Downloads/programming-house-style (2).md`, preserving the
existing behavior and public contracts. The shared checkout baseline is
`9ec42b110766bde45f785b7a5601be346dde83ab`. This review covers exactly the ten
translation/header/test units below. It does not certify the rest of GUI.Forms
or select a new architecture. Other sibling-owned changes are outside this
source review, even when included in the shared build.

## Spelling inventory

**OBSERVED:** `tools/check_house_style.py` applied to the baseline source bytes
reported these counts. The three categories are explicit-type (`auto`), arrow
member/trailing-return spelling, and lambda review candidates. Manual inspection
confirmed the reported callbacks in this scope were actual lambdas.

| File relative to `gui_forms/` | Type | Arrow | Lambda |
| --- | ---: | ---: | ---: |
| `include/gui_forms/controls/panel/text_box/text_box.hpp` | 0 | 0 | 0 |
| `src/controls/panel/text_box/text_box.cpp` | 6 | 2 | 1 |
| `tests/multiline_text_box_tests.cpp` | 9 | 119 | 0 |
| `include/gui_forms/controls/panel/breadcrumb_trail/breadcrumb_trail.hpp` | 0 | 0 | 0 |
| `src/controls/panel/breadcrumb_trail/breadcrumb_trail.cpp` | 8 | 2 | 7 |
| `include/gui_forms/controls/menu_strip/menu_strip.hpp` | 0 | 0 | 0 |
| `src/controls/menu/menu_strip/menu_strip.cpp` | 1 | 1 | 2 |
| `tests/basic_controls_tests.cpp` | 6 | 19 | 0 |
| `tests/collection_controls_tests.cpp` | 2 | 6 | 7 |
| `tests/menu_controls_tests.cpp` | 0 | 4 | 1 |
| Total | 32 | 153 | 18 |

The baseline total is 203. **OBSERVED:** the final scan of these ten files reports
zero spelling findings/review candidates. `git diff --check` passes for the
assigned units. A clean spelling scan alone is not semantic evidence.

## Semantic changes and reviewed boundaries

- **OBSERVED:** TextBox uses explicit result types and named return values.
  Clipboard/edit operations are evaluated before returning their status.
  Host replacement offsets widen validated nonnegative values before addition.
  Password-mask capacity is checked before multiplying the encoded width.
- **OBSERVED:** multiline layout reserves one call-owned scratch row from the
  maximum logical-line byte bound and reuses its boundary, position, and run
  vectors through the grapheme walk. Retained output rows reuse available
  capacity; growing the number of output rows can still allocate. Cache keys
  commit only after rebuilding, and invalidation precedes mutation. Allocation
  or provider failure can leave partial internal rows, but the invalid cache
  forces rebuilding on the next call; this is not an atomic old-layout promise.
- **OBSERVED:** tab advancement is separate from the text-prefix shaping loop.
  Text runs retain their original shaping boundaries. Wrapping, exact stored
  UTF-8, line terminators, navigation, hit testing, and device-scale invalidation
  retain the existing contracts. The 1 MiB document and 4096-byte logical-line
  admission bounds are unchanged.
- **OBSERVED:** breadcrumb layout measures natural segment widths once, reserves
  proposal storage before the suffix walk, swaps reused proposal buffers, and
  traverses the ordered visible indices to identify hidden segments. Named
  traversal replaces anonymous predicates and append closures. Output string
  copies remain owning copies and may allocate.
- **OBSERVED:** menu item measurement stores named normal/selected metrics;
  navigation and semantic actions perform the open operation before returning
  its result. Stable identity validation has explicit insertion results.
- **OBSERVED:** named callbacks expose retained or borrowed state. Breadcrumb
  editor callbacks weakly observe their owning trail and lock it for invocation.
  Menu popup handlers borrow their containing owner. Existing subscription and
  disposal protocols remain in place. The test focus listener borrows its Window
  and weakly observes the destination; its subscription ends first.
- **OBSERVED:** the multiline fixture owns its metrics provider longer than the
  borrowing Window. Test input/semantic/clipboard/history operations are sequenced
  before assertions. Split compound assertions preserve short-circuit failure
  order so dependent observations cannot read invalid state. Existing named
  recorder callbacks replace the remaining anonymous counters.
- **OBSERVED:** fields have explicit initialization, numerical conversions are
  visible at the touched layout boundaries, and selection helpers calculate
  named values. No public method signatures or object field ordering changed.

## Validation and limits

The Windows profile is `gui_forms/.build/shadow-windows`, Release, C++20,
MinGW GCC 16.2.0, selected through `tools/Enter-WindowsToolchain.ps1`. It borrows
the adjacent Plan Paint toolchain read-only. HarfBuzz and Skia are disabled in
this profile. Source and outputs stay in File Manager; frozen SDK checkpoints
and installed/published products are not modified.

The first compilation exposed five malformed header return statements from an
initialization edit. A subsequent compilation exposed two shared-pointer test
guards needing explicit Boolean conversion after assertion splitting. These
negative results were corrected before final validation.

**MEASURED:** the full build and final incremental build completed successfully:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake --build gui_forms/.build/shadow-windows --parallel 2
ctest --test-dir gui_forms/.build/shadow-windows --output-on-failure --parallel 2
```

CTest passed **65/65**, zero failures, in 17.53 seconds wall time. This includes
the sibling-owned application contract/native and label/renderer changes present
in the shared checkout. The new focused fixture passes for a tab wider than
the viewport, adjacent complete text runs, and retained-row shrink/regrowth.
Existing fixtures pass for exact line endings, Unicode graphemes, wrapping,
navigation, clipboard, history boundaries, DPI changes, and bounded workloads.
These durations describe this test run, not a before/after speed comparison.

**HYPOTHESIS / unmeasured:** reservation/reuse reduces allocator work during
repeated layout. No allocator-count benchmark or controlled speed comparison was
run. Prefix measurement retains its bounded quadratic cost. The test metrics
provider is deterministic and does not establish native complex-text fidelity.
This Windows run does not validate macOS, Skia, or a HarfBuzz-enabled runtime.

## Independent parent review and integration follow-up

**OBSERVED:** the independent review of the parent's application/text changes
found paragraph-sized label scratch reservation and per-grapheme font-run
reservation that could greatly exceed actual output. Both were corrected. Label
wrapping now reuses one candidate line and publishes copies of live text;
HarfBuzz font-run storage grows for distinct runs. The fatal-close path swaps
its callable into an active local owner without allocating and conditionally
restores it if native delivery throws while the window remains open.

**MEASURED:** focused regressions cover approximately 128 KiB of whitespace
around one character, repeated long ASCII words, combining characters, joined
emoji, normalized spacing, and explicit empty paragraphs. The latest basic,
application-contract and application-native suites rebuilt and passed 3/3 in
0.45 seconds. This run includes those follow-up changes; the earlier 65/65 run
predates them. The parent's separate pinned HarfBuzz build passed its suite
after the reservation correction. Resource-exhaustion cleanup remains reasoned
from ownership and was not validated by allocation-failure injection.
