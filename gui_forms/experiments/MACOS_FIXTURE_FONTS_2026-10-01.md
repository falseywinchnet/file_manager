# macOS native fixture font provisioning correction

Date: 2026-10-01.

**OBSERVED:** SwiftEdit's consumer review found its standalone native fixtures
reported `incomplete bundled font pack`. Provider source review confirmed the
same setup defect: the five native host test targets were bare executables,
while `macos_host.mm` registers fonts through `NSBundle`'s `fonts` resources.
The host does not use a `GUI_FORMS_FONT_DIR` environment override.

Earlier bare-fixture CPU receipts, including the provider paint-cost attempts
and focused attribution attempt at `be0ed41`, do not establish loaded-font or
packaged-application CPU performance. Preserve them as evidence of their actual
setup. This finding does not establish a font defect in packaged SwiftEdit.

**OBSERVED correction:** all five provider native host fixtures now use macOS
application bundles with the same explicit font and notice list as the gallery.
That list is available when tests are enabled even if the gallery is disabled.
Both diagnostic runners resolve the executable inside the bundle. Both CPU
experiments inspect each model's renderer readiness before every interval and
reject missing-font operation explicitly. No timing threshold is relaxed.

**MEASURED prior run:** native CI 36948765463 at `be0ed41` completed Windows and
Linux successfully. macOS passed 78/79 ordinary tests; exposure aborted with
`std::bad_function_call` after 1.95 seconds. The focused CPU attempt rejected
interval zero because normal ten-second caret counts diverged. Neither result
establishes a CPU improvement. Raw Mac job 110656819770 is preserved locally at
`.build/provider-be0ed41-evidence/macos-job.log` and by the CI run.

**Validation:** Windows CMake configure with gallery/tests/Skia/macOS host off
passes after the resource-list restructuring. Native bundle generation, resource
loading, and Objective-C++ execution require the next macOS CI run; they are not
claimed locally. Reviewed the authored CMake resource scope, two runner path
changes, and two admission checks against `planning/PROGRAMMING_HOUSE_STYLE.md`:
explicit initialized snapshots, named existing operations, no retained new
callbacks or borrows, readiness checks outside measured intervals, and rejection
before measurement. No blanket certification of unchanged fixture code.
