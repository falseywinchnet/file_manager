# Native color checkpoint verification

The exact source `aa4567b632e319facd84a66e514f66840416c9c8` passed research push
run `37172405278` and PR run `37172407771` on Windows 2022, macOS 26 and Ubuntu
24.04. The downloaded exact-head push logs are retained beside this record:
three research tests on Windows (including WIC), two on each Unix platform.
Both full development matrices also passed: push `37172405291`, PR
`37172407781`. Their success does not activate the research codec in the app.

PR37 then rebase-merged to `4c80dd7fab5b145b369a4eb4f5e2da9bba3588bb` after its
macOS push retry passed. Root verified its entire merged tree equals tested
`29d39ecbaf81c285c6686acd2287cce7b29ef71d`. The rejected first macOS push logs
remain in the preceding WIC evidence directory: an AppKit host-close timing
check and focused-CPU interval exceeded their declared bounds. The second
attempt at the same source passed. This is variable-runner evidence, not a
diagnosed or fixed toolkit timing defect; no tolerance was weakened.

Changing PR38's base from the old branch to main exposed duplicate predecessor
history and prevented a clean merge. Root rebased only its two color commits
onto the merged predecessor. New head
`0029499ab344112a125c6f675e1ec77d4eccd828` has an identical complete tree to
tested `aa4567b6`; `git diff --quiet` verified that fact before the guarded
force-with-lease update. The lifecycle draft was preserved/restored through an
exact, temporary stash; historical research stashes remain untouched.

Both post-rebase research matrices passed again: push `37173418262`, PR
`37173420268`. The post-rebase full development matrices `37173418280` and
`37173420294` were still running when this record was authored. Do not treat
those as completed or PR38 as merged without checking current GitHub state.

The final Unix build correction adds only explicit Threads discovery before
importing the pinned static color library. Root reviewed that CMake delta;
the preceding independent house-style review covers the unchanged C++ core,
fixtures, and original build/workflow inputs. No product-format, latency,
physical-Mac dogfood, installer or general profile-accuracy claim follows.
