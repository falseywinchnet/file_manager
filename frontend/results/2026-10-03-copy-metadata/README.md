# Copy metadata baseline receipt

**GIVEN:** This is a baseline experiment only. No product metadata policy,
adapter, public contract, or architecture decision is selected here.

**MEASURED:** The standalone probe completed on Windows 11 Home 10.0.22621,
x64, NTFS with 4,096-byte allocation units. Compiler: adjacent read-only MinGW
GCC 16.2.0; libstdc++ header date 20260807. The receipt directory uses the
requested UTC date; the local recording time was October 2, 2026, 22:11 PDT.
See `environment.txt` for the source SHA-256 and environment capture.

| Generated fixture | Observed result |
| --- | --- |
| Ordinary file | Contents equal; permission masks both decimal 438 (0666) |
| Read-only file | Contents equal; permission masks both decimal 292 (0444) |
| Five-year-old last-write time | Contents equal; source time not preserved |
| Named stream `fm-copy-baseline` | Source payload read back; destination stream absent (Windows error 2) |
| Sparse file | Logical length 8,388,608 bytes on both sides; source allocation 65,536 bytes; copy allocation 8,388,608 bytes; all contents equal |

All five copy operations succeeded; all reported source/destination permission
masks matched. Source permission requests are interpreted by Windows, so this
does not establish POSIX executable-bit preservation. Timestamps happened to
match for newly created fixtures within the same reported clock interval; only
the deliberately old timestamp fixture distinguishes preservation from a fresh
destination timestamp. This run shows that the old timestamp was not retained.
It does not establish timestamp precision beyond the recorded library values.

The probe reported exact-scope cleanup success and exit zero. Optional Windows
named-stream and sparse-fixture capabilities were available. macOS quarantine,
macOS ordinary xattrs, and Linux user xattrs were neither compiled nor run on
this host. Their guarded source is included for later platform receipts; no
preservation result is claimed for them.

## Validation and retained failures

The successful build used C++20, `-O2 -Wall -Wextra -Wpedantic -Werror` and one
compiler process. `compile.txt` records exit zero; `windows.tsv` contains the
full observations and cleanup result; `run.txt` records probe exit zero.
Reproduction and proposed CI commands are in the experiment README.

The first compiler invocation, before prepending the adjacent MinGW bin
directory to PATH, exited one without diagnostics. No measurement resulted.
After setting PATH, the next compile failed because MinGW had already defined
`NOMINMAX`; the exact diagnostics are retained in
`compile-failed-nominmax.txt`. Guarding that definition resolved the warning.
The final compile and run succeeded. No repository build or existing test suite
was invoked; this is independently compiled experimental code.

## Semantic house-style review

**OBSERVED:** Reviewed the complete authored
`frontend/experiments/copy_metadata/copy_metadata_probe.cpp` against the complete
`planning/PROGRAMMING_HOUSE_STYLE.md`, including all Windows, Apple, and Linux
preprocessor branches. Reviewed the experiment README, local `.gitignore`, and
this receipt for scope and factual claims. Logs are captured tool output.

The source uses explicit types, named executable behavior, an enum for metadata
states, initialized fields, and a noncopyable fixture owner with lifetime
cleanup. There are no callbacks or deferred borrows. Stream owners close by
lifetime; native Windows handles close before failure propagation. Buffer
borrows remain within synchronous I/O calls. Two 64 KiB comparison buffers are
allocated before the byte loop and reused without per-byte allocation. Native
byte counts are bounded; negative POSIX allocation counts and multiplication
overflow are rejected. Read-only access is restored only after cleanup scope
verification. Metadata-creation unavailability, read failure, copy failure, and
cleanup failure have separate recorded outcomes. Cleanup deliberately preserves
uncertain scope instead of escalating to recursive deletion.

No known house-style violations remain in that reviewed authored scope. This
is a source review, not a claim of macOS/Linux compilation or runtime proof.
Existing product source, other experiments, generated code, and dependencies
were not reviewed or modified for this assignment.

## Native workflow failure and clock-reporting repair

**OBSERVED:** Native workflow run `37099310256` at `0964c58` produced a macOS
compile failure: both direct `std::to_string(file_time_duration.count())` calls
were ambiguous in libc++. The full supplied diagnostic artifact is preserved
unchanged as `macos-0964c58-compile-failed.txt`. No macOS runtime metadata
observation resulted from that run. The earlier local-only limitations above
describe the original receipt; the Linux evidence below arrived subsequently.

**MEASURED:** The supplied Linux artifact from that same revision is preserved
as `linux-0964c58.tsv`. It records GCC 13.3.0, libstdc++ date 20240904,
x86_64 Linux kernel 6.17.0-1022-azure, matching contents and permission masks
for all five copies, an unpreserved old timestamp, an absent destination user
xattr (errno 61), and sparse allocation increasing from 8,192 to 8,388,608
bytes. It records successful exact-scope cleanup and completion. Filesystem
identity is not present in that TSV; no filesystem type is inferred. These are
baseline observations, not accepted metadata policy.

The repair adds `format_file_clock_ticks`, a named checked formatting boundary.
Its explicitly typed original count is compared to signed 64-bit bounds in the
original `file_time_type::duration::rep`. Static assertions require a signed
integral representation with at least 63 value bits, making both bound
conversions exact. Only after both comparisons pass is the count converted to
`std::int64_t` and formatted. Out-of-range values fail explicitly through the
existing exception/cleanup path. Tick precision and epoch are unchanged.

**MEASURED:** The repaired source compiled locally with the same one-process
GCC command and warning-as-error flags documented above. Compile and probe
exited zero (`clock-repair-compile.txt`, `clock-repair-checks.txt`). The new raw
run is `clock-repair-windows.tsv`; measured preservation outcomes match the
original Windows receipt. Its exact reported temporary path was independently
checked absent after successful owned cleanup. The repair source SHA-256 is
recorded in `clock-repair-checks.txt`; the earlier `environment.txt` hash applies
only to the original source. Native macOS compilation and execution of the
repair remain pending; the wider representation's out-of-range branch was not
runtime-exercised on this Windows host.

**OBSERVED:** Semantic review of the complete changed scope against
`planning/PROGRAMMING_HOUSE_STYLE.md` covered the new formatter, both changed
reporting call sites, the experiment README addition, and this receipt addition.
Types, initialization, signed-width bounds, compare-before-convert order,
allocation of the returned string, value ownership, and existing failure cleanup
were reviewed. No callback, retained borrow, mutable shared state, or repeated
element-loop work was introduced. No known house-style violations remain in
this changed scope. The repository spelling check was run as
`C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/python.exe tools/check_house_style.py frontend/experiments/copy_metadata/copy_metadata_probe.cpp`;
it reported one file, zero findings, and exit zero. Its output and exit status
are in `clock-repair-spelling.txt`, including the preceding failed attempt via
the unavailable default `python` launcher (exit 9009). An additional
`rg` spelling scan found no prohibited spellings. Neither spelling scan
substitutes for the semantic review. No workflow, production source, or Git
state was changed by this repair assignment.

## Coordinator integration

The coordinator reviewed the complete probe, owned-fixture cleanup, native
handle closure, metadata failure states, and the platform-specific baseline
limits. The initial source SHA-256 matched `environment.txt`; the corrected
source hash is recorded in `clock-repair-checks.txt`. The independent spelling
check reports zero candidates. This does not establish platform execution.

`.github/workflows/copy-metadata.yml` runs the standalone specimen on the same
runner families as the application: Windows 2022 with MinGW64, macOS 26 arm64,
and Ubuntu 24.04. It records the checked-out commit, compiler, standard-library
identity, observations and failed compilation output. Each job uses one compiler
process and only probe-owned temporary fixtures. Shell pipeline failures remain
fatal; optional metadata unavailability remains an explicit observation. No
application source, build target, public contract or shipped package is changed.
The workflow is source-reviewed; native results are pending.
