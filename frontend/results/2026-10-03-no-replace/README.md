# No-replace filesystem publication — 2026-10-03 UTC

Status: **implemented; local Windows and native Mac/Linux verified; Windows CI
fixture correction awaiting its fresh run**.

**GIVEN:** ADR-017 admits rename, move, staged copy, quarantine and restoration
only with no destination replacement. **OBSERVED:** the previous implementation
checked vacancy and later called ordinary `std::filesystem::rename`. That is not
a portable no-replace commit operation. No data-loss incident was reproduced.

The new private `native_publication` adapter uses these documented operations:

- Windows [MoveFileExW](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw)
  with zero flags: no replacement, cross-volume copy or deferred move.
- macOS [renamex_np](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/man/man2/rename.2)
  with `RENAME_EXCL`; unsupported filesystem behavior remains an error.
- Linux [renameat2](https://man7.org/linux/man-pages/man2/renameat2.2.html)
  with `RENAME_NOREPLACE`; no ordinary-rename or link/unlink fallback.

All five publication/restore sites use this adapter. A native occupied-target
error produces conflict; unsupported no-replace or cross-volume operation is
unavailable; other native failures remain failed with their error message.
Linux `EINVAL` is retained as a failure rather than guessed to mean a particular
unsupported-filesystem cause. macOS `ENOTSUP` and `EOPNOTSUPP` are both handled.

Destination preflight now distinguishes vacancy, occupied leaf and observation
failure. Name-search loops stop on observation failure instead of treating it as
vacancy or exhausting all candidates. New Folder still uses atomic directory
creation; an already-created directory is classified as conflict. Failed copy
publication cleans its stage when possible, reporting the intended destination
after cleanup or the retained stage when cleanup fails. Existing undo authority
is not replaced by a refused publication. No public layout, wire, capability or
Orchestrator contract changes are introduced.

## Verification

**MEASURED:** GNU/MinGW 16.2.0 Release, Shadow Windows 11 Home 10.0.22621, two
compile jobs, matching local installed GUI.Forms SDK. All 13 CTest suites pass
in 3.26 seconds; `WindowsLastTest.log` retains the complete run. Existing operation
tests continue to cover normal rename/copy/move/quarantine/undo, cancelled stage
cleanup, conflict/retry, fault outcomes, in-memory undo and protected admission.

The new direct native adapter cases verify occupied regular-file identity/content
preservation; refusal to replace an empty directory; successful file/directory
publication to vacant names; failed-source nonpublication; and invalid-parent
observation failure. Dangling destination and source symlink cases are included,
but Windows creation privilege error 1314 skips them here. Mac/Linux execution
must validate those cases before three-platform acceptance. The test uses owned
disposable objects, without a concurrent writer. Source review confirms that
each admitted operation uses the same native primitive; this is not a full
concurrent-filesystem or physical-error campaign.

An unavailable kernel/filesystem is fail-closed in source; no unsupported-volume
fixture was exercised on Shadow. Permission failure, unsupported-volume behavior,
remote filesystems, case-only rename and all failure-state publication races are
not claimed proven by this run. This change prevents destination replacement;
it does not make earlier source identity or parent-route observations atomic,
admit broader mutation roots, enable merge/replace/cross-volume moves or provide
durable crash recovery. Those remain separate requirements.

## House-style review

### Native CI fixture correction

**MEASURED:** native push run `37097850866` passed Linux and macOS. Windows
compiled successfully but failed the new source-link publication assertion.
`WindowsInitialCIFailure.log` preserves its CTest excerpt. Local Windows had
skipped this case because symlink creation privilege is unavailable here.

**OBSERVED:** `parse_windows_symlink_target` deliberately returns an absolute
target using the Win32 extended namespace. The new test incorrectly compared
that representation with the ordinary path passed to fixture creation. It now
captures the stored target before the move and compares it afterward. Separate
assertions retain the available symlink identity, exact stored target, vacancy
of the old link name and absence of the dangling target. No production behavior
or symlink skip rule was changed. The native Windows rerun must establish that
the representation mismatch accounts for the reported failure.

The focused local operation suite passes in 0.19 seconds; the unavailable-link
skip remains explicit in `WindowsFixtureCorrection.log`. Reviewed the added
observations and assertions against the house style: explicit initialized
owned paths/statuses, synchronous observations, immediate failure exits and no
retained borrows or new callbacks. One changed C++ file has zero spelling
candidates; this does not substitute for native Windows link execution.

### Initial implementation scope

Reviewed against `planning/PROGRAMMING_HOUSE_STYLE.md`: the entire new private
header/implementation; changed service preflight, publication, error projection,
stage-reporting and naming-loop exits; explicit `CopyOutcome` string initialization;
removed private declaration; CMake source addition; new fixture-reading and native
publication checks. Explicit initialized types, synchronous path borrows, immediate
native error capture, named behavior, known flags, owned error/path results,
conversion boundaries and success/failure order were reviewed. No callbacks,
threads, deferred borrows, per-byte buffers or retry loops were added.

Five changed C++ files produce zero spelling candidates. This does not certify
unchanged recursive copy, legacy fixture lifetime/cleanup, the whole service or
GUI.Forms. Existing repeated naming allocations and nested path expressions are
not performance claims and remain outside this repair's reviewed source scope.
