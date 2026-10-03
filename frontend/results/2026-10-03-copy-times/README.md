# Regular-file Copy modification dates

**OBSERVED:** the production native stage writer at `31d1319` copied regular
file bytes and ordinary permissions but left the stage's new modification date.
This affects the published file as well as regular-file children of copied trees.

## Repair

The writer captures the source's native modification time alongside its existing
permission observation, using the source handle. After copying, source revision
and path-binding revalidation, and destination extent validation, it applies
that timestamp through the still-owned stage handle. Windows uses the existing
`FILE_BASIC_INFO` setter; macOS/Linux use `futimens` with `UTIME_OMIT` for access
time, followed by the existing permission application. Metadata failure is an
explicit native failure and follows the service's existing owned-stage cleanup
law. Full byte progress still does not establish successful publication.

Windows nonpositive native timestamps are refused before stage creation, rather
than treating them as ordinary absolute values in a setter with control values.
This boundary is source-reviewed; the generated fixtures exercise a positive
absolute timestamp, not a filesystem-supplied sentinel. Destination filesystem
precision controls timestamp representation; cross-filesystem rounding is not
measured here. Access, creation and status-change timestamps retain native
new-object behavior. No date is written back to the source.

The platform contracts are documented in Microsoft's
[FILE_BASIC_INFO reference](https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-file_basic_info)
and its underlying
[time-field sentinel contract](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_file_basic_information),
and the Linux man-pages project's
[utimensat/futimens reference](https://man7.org/linux/man-pages/man2/utimensat.2.html).
The native timestamp representations stay private; no chrono epoch conversion
or new public API is introduced by the production change.

## Correctness evidence

**REJECTED:** the two focused Windows suites both failed with the added
regressions before the repair. `WindowsRejectedLastTest.log` records the native
post-close and service post-publication failures. Fixtures set the old instant
2000-01-01 UTC plus 123,456,700 ns using native APIs. Comparison uses observed
signed seconds and a nonnegative nanosecond fraction, avoiding the Windows
standard-library file-clock reporting precision seen in the earlier probe.

**MEASURED Windows:** integrated Release build, shared MinGW GCC 16.2.0 toolchain,
at most two compiler jobs. File-operation and native-copy suites pass in 0.37 s
and 0.26 s; CTest total 0.64 s, retained in `WindowsLastTest.log`. Checks cover
empty, short and multichunk files, ordinary/read-only permissions, post-close
date preservation, source revision stability, distinct destination identities,
final no-replace publication and nested regular-file dates. Existing progress,
cancellation, callback-failure and cleanup tests remain passing. Windows link
fixtures report missing privilege and skip; they do not establish link behavior.

These are correctness checks, not throughput measurements. Native macOS/Linux
execution of this production repair and package delivery are pending.

## Earlier metadata control, newly inspected

`baseline-{windows,macos,linux}.tsv` retain the raw downloaded probe observations
for source `5d01f4d3e1faa8734a30e01282c4054a05115e9b` from PR15. The dedicated
[metadata workflow](https://github.com/falseywinchnet/file_manager/actions/runs/37099943253)
passed all three platforms. That is not a claim that PR15's separate application
builds passed. Each source receipt in the downloaded artifacts identifies that
commit; all three probes report exact-scope cleanup and completion.

This control uses `std::filesystem::copy_file`, not the current production
native adapter. Its old-time fixture loses the modification date on all three
platforms. Its ordinary extended attribute is absent on Mac and Linux; the
Mac quarantine attribute survives the specific fixture. The Windows named
stream is absent. Sparse allocation expands on Windows and Linux. The Mac
source fixture itself is not sparse, so that row establishes no sparse-file
preservation result. All copied data in those fixtures compare equal. These
observations inform further work; they do not choose a complete metadata policy.

## House style and remaining scope

Root reviewed every authored hunk in the five implementation/test files listed
by `reviewed-source-sha256.json` against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`. Review covers explicit initialized types,
native timestamp records without epoch conversion, named fixture behavior,
fixed two-element timestamp arrays, handle lifetime through application/close,
failure before publication, and once-per-file work outside the byte loop.
The test fixture performs no throwing work between native open and close; it
operates only on paths supplied by existing owned fixture controllers. There
are no new callbacks, retained borrows or per-byte allocations. The five-file
spelling scan reports zero candidates. Independent sibling review also identified
the Windows zero-time sentinel; root's overlapping guard explicitly rejects
that input before staging. No remaining house-style violation was
identified in the authored scope; unchanged legacy code is not certified.

Directory/link timestamps, extended attributes and named streams, ACL/ownership,
sparse/clone fidelity, readonly-stage cleanup on later publication failure,
ordinary endpoint validation/admission and general recovery remain unfinished.
This regular-file repair does not enable ordinary Copy or declare its complete
metadata policy accepted. It does not weaken existing source, cancellation,
collision or publication checks.
