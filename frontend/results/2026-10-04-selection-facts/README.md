# Selected-object facts

Status: **MEASURED three-platform checks; merged and published.**
See `../2026-10-04-copy-path/DELIVERY.md` for the combined release receipt.

The live inspector previously displayed generic multiple-value placeholders for
every multi-selection. It now presents file/folder/other/unavailable counts,
observed regular-file logical bytes, and common type/date where the observations
agree. Missing sizes, known zero, overflow and uncounted folder contents remain
distinct. Search selections do not falsely claim the browsing folder as their
common location. Aggregate names remain noneditable.

The summary borrows existing directory entries for one synchronous pass and
retains only scalar/optional values. It performs no filesystem reads, recursive
folder sizing or per-entry allocation. Byte totals count selected bindings, not
unique physical disk extents. Status now uses the same typed logical-size facts
as Details, rather than the identity revision fingerprint.

## Verification

- Windows GCC 16.2 Release, existing `shadow-windows-latency` build against the
  installed `shadow-sdk`, borrowed read-only Plan Paint tools, one compiler job.
- Full frontend build and all 15 CTest suites passed (8.38 seconds total).
  `Windows-LastTest.log` preserves the output; this is correctness evidence,
  not a responsiveness benchmark.
- Six changed C++/Objective-C++ files: zero house-style spelling findings.
  Separate semantic review is recorded in `SOURCE_REVIEW.md`.
- Model checks distinguish metadata from identity size, missing/zero values,
  nanosecond time differences, overflow, and folder/link exclusion.
- Assembled application checks exercise two files, mixed file/folder selection,
  folders only, disabled aggregate rename, and return to single/empty selection.
- macOS native harness now checks three generated files and saves
  `native-selection-facts.png`, then restores the preview before continuing the
  existing local-action checks. Its rendering still requires native execution
  and inspection; local Windows tests do not certify that screenshot.

## Retained negative result

`Windows-rejected-detached-label.log` records the first incorrect test's failure:
it looked for an authored facts label that is detached when the live PropertyList
is installed. The initial conjecture of visible stale labels was wrong. The test
now checks the actual PropertyList; redundant writes to detached labels were
removed. Earlier test compilation mistakes (a helper outside its scope and
calling a string method on an optional) were corrected before the passing run.

This change adds no mutation capability, preview format, index, thumbnail cache,
public contract or recursive size engine. Native/large-selection latency and
full accessibility remain separate evidence gaps.
