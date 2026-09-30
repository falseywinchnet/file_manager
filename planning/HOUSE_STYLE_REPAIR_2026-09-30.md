# House-style correction

Status: **review checkpoint for shutdown preservation; remaining acceptance limits below**.

**GIVEN:** On 2026-09-30 the owner rejected the delegated implementations for
failing the supplied coding standard and directed that the work be redone.
The exact supplied document is [PROGRAMMING_HOUSE_STYLE.md](PROGRAMMING_HOUSE_STYLE.md).
Its SHA-256 is `2B16CEAD4EA11983009B6B7433CE12AC48701EEEE63A00DF4620EA7CD928652C`.

**OBSERVED baseline:** File Manager `9ec42b1`, SwiftEdit `cbfbbef`. SwiftEdit had
151 production source lines containing `auto`, as well as anonymous callbacks,
pointer arrows, structured bindings, and a defaulted comparison. Previous
functional verification did not constitute house-style acceptance.

## Responsibility and scope

Existing visible sibling chats perform the corrections; no workers or new
worktrees are used. The parent reviews the resulting source independently.

| Owner | Correction scope |
|---|---|
| SwiftEdit | Authored production source, tests, tooling and repository instructions in the separate `notepad` repository |
| Frontend | Application, non-picker modules, headers, tests and local launch tools |
| Picker | Controller, view, public headers, tests and installed consumer example |
| GUI.Forms controls | TextBox, breadcrumbs, menu strip and associated control tests |
| Engine | Go units and tests introduced or changed by the Shadow bring-up and search integration |
| Orchestrator | Rust integration, C++ conformance client, tests and search launcher |
| Build/export | Build/package/diagnostic tooling, API reference generator, install verification and associated tests |
| Parent | Policy, spelling checker, application bridge, imported rendering/text changes, independent review and final integration |

Imported toolkit work is compared with the pre-bring-up revision `ddade5b^`.
Unchanged inherited code and vendored dependencies must not be reported as
reviewed or compliant merely because an adjacent change passes.

## Acceptance evidence

Each scope needs a concrete file inventory, source review against the entire
standard, regression tests for changed lifetime/failure behavior, and matching
rebuilt dependencies. Record remaining violations explicitly. C++ spelling rules
do not define Rust/Go syntax; the document's language-neutral rules still apply.

`python tools/check_house_style.py <explicit C++ source paths>` scans comments
and literals separately and reports common forbidden forms. It is a lexical
review aid, not a complete C++ parser or a semantic compliance certificate.
`python -m unittest discover -s tools -p test_house_style.py` checks its fixtures.
Ownership, reentrancy, cleanup, loop storage, conversions and transparent
sequencing require direct review in addition to that scan.

Published `342c42a` artifacts and the installed `dbe3766` SDK pair are frozen
historical outputs. Picker deferred-completion state changes its private class
layout; integration therefore requires a fresh matching SDK and clean consumer
builds. Earlier test results cannot certify those new binaries.

Rust's stable scoped-thread API currently leaves a proposed one-call closure
bridge around named owned context. This is **OPEN**, not an approved exception
or a claim of compliance. Unsafe trampolines or global context registries are
not an automatic remedy.

## Independent integration evidence

**MEASURED on Shadow / MinGW GCC 16.2 / Release:** the corrected toolkit's
65-test run passed. After independent review corrected label scratch sizing and
fatal-close callable ownership, the three affected basic/application suites
were rebuilt and passed. A separate pinned HarfBuzz/FreeType build passed its
text-engine suite after the font-run reservation correction. That separate
profile does not enable Skia or certify macOS rendering.

**OBSERVED:** source review corrected an allocation window after SwiftEdit
temporary-file acquisition, picker cancellation/completion during nested reload,
and excessive label/font-run reservation. These findings demonstrate why the
lexical checker is insufficient by itself. Failure-path and lifetime regressions
exercise actual callback revocation, exceptions and owner destruction.

**MEASURED:** the new, unpublished matching review pair is under
`.build/sdk-checkpoints/house-style-review/windows-x64/`. It was produced from
the modified working tree, not a clean commit. GUI.Forms installed to
`gui-forms-sdk`; a fresh Release picker build against that exact prefix passed
both picker suites (0.60 s), installed to `picker-sdk`, and passed a fresh
independent consumer (0.10 s). A relocated GUI.Forms native consumer passed
(0.51 s), and the missing-component rejection check passed.

At installation, `libgui_forms_application.dll` SHA-256 was
`C9E79914273562042AE38BFFDFDD296B6CA7C2BFA0F28BF4D4B2EFDA9DBB9336`;
`libfile_manager_document_picker_view.a` SHA-256 was
`FF19B71E39D3AC56C757C54AA3C8F20CFB6BE0008546D216E17565449EE91855`.
Consumers must use the complete matching pair. SwiftEdit's fresh build against
both prefixes passed all six suites (3.03 s) after the final Windows error and
file-operation sequencing correction. Its review stage is
`C:/Users/Shadow/notepad/dist/SwiftEdit-house-style-integrated-review`; GUI SHA-256
is `5FE58158865627E446A9E3B945F46B50B81CA97B776E7ED53B451020B5586CEE`,
CLI SHA-256 is
`BACB7104021C6B58CB423893B88170AEC6339420D89997B5C932D40D32269932`.
Its DLL matches the value above. The remaining frontend source correction is
still in progress. These are local review artifacts, not a published release.

**MEASURED:** parent independently reran eight native-tool fixtures, five
compiler-reference fixtures, two install-verifier fixtures and twelve scanner
fixtures; all passed. Scanner fixtures include digit-separated numeric literals
so apostrophes cannot hide subsequent executable source as character literals.

The native-build workflow now gates builds on the scanner's fixtures and the
corrected frontend/independent C++ client source directories. This is a spelling
regression gate with explicit scope, not certification of ownership, behavior,
generated output, other languages or the inherited toolkit. Semantic review
remains required. The new workflow has not yet run remotely.

## Shutdown preservation

**GIVEN:** the owner requested pushing all relevant repositories except Plan
Paint, then warned of machine shutdown in five minutes. The coordinator froze
further implementation and is preserving the complete corrected working trees
in File Manager and the separate SwiftEdit repository. This checkpoint is not
an assertion that every platform or all inherited code has passed acceptance.

**OBSERVED:** independent review verified named-job lifetimes and stop/join
ordering. The frontend now snapshots `show_hidden` into worker jobs, publishes
undo availability from worker results into UI-owned state, and rejects further
callbacks after a callback stops a drained UI batch. The shutdown regression
passed; the interaction source also completed a separate execution-order pass
with all 171 effectful call arguments and source ordering retained.

**MEASURED:** the final spelling gate passes 50 File Manager frontend/client
files; SwiftEdit's 17 C++ files also pass. This remains lexical evidence only.
The refreshed complete provider pair is under
`.build/sdk-checkpoints/house-style-final/windows-x64/`; both picker suites pass
after the final model/test edits (0.53 s). Older review prefixes remain intact.
Per-component records state which final build/tests completed before the freeze.

**OPEN:** three Rust standard-thread forwarding closures remain unapproved
exceptions to the literal anonymous-execution prohibition. Cross-platform CI
for this new checkpoint, final native visual acceptance, and any validation
explicitly listed as pending in the component receipts remain outstanding.
Existing published archives are historical artifacts, not rebuilt by this push.

### Final pre-shutdown handoff

**MEASURED:** the final fresh frontend build and its latency-benchmark target
compiled; all 12 frontend suites passed in 2.47 seconds. The interaction suite
includes shutdown revocation. A source-to-source raster comparison preserved
exact bytes for 136 icon cases. See the final section of
`frontend/results/2026-09-30-house-style/README.md` and `final-ctest.txt`.

**MEASURED:** SwiftEdit's final localized production and test sequencing edits
compiled and passed all six suites in 2.64 seconds against the preserved
`house-style-review` provider pair. The refreshed `house-style-final` pair was
not used for another clean SwiftEdit integration before shutdown. The latest
integrated review-stage GUI SHA-256 is
`11C972A9569D340DB6CAFE89910B526F08D0D4109CB622A0C5434208886BC303`;
CLI SHA-256 is
`E14DB1A875C031DA0FAFA149B375DED7D90FDC647725A242B52E5656ED062A7A`.
Those supersede the earlier review-stage executable hashes above.

**OPEN:** frontend's exhaustive remaining compound-expression review did not
finish before the shutdown freeze. Known follow-up includes effectful
`undo_last()` assertions in `file_operations_tests.cpp` and some leaf-test
filesystem/conversion chains. This checkpoint must not be described as complete
house-style acceptance. The broader visual, platform and performance limits
remain as recorded; passing tests do not close them.

**OBSERVED:** implementation checkpoint `37552ac` was pushed to File Manager's
`codex/native-dogfood` branch. SwiftEdit checkpoint `e9d2715` was pushed to
`master` in the new private `falseywinchnet/swiftedit` repository. Plan Paint
was not changed or pushed by this correction. Native CI runs remotely after the
File Manager push; its result was pending at this handoff.
