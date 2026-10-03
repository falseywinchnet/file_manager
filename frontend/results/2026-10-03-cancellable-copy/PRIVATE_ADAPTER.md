# Private cancellable regular-file stage adapter

**GIVEN:** This assignment implements only `frontend/src/native_copy.hpp`,
`frontend/src/native_copy.cpp`, `frontend/tests/native_copy_tests.cpp`, and this
receipt. Root owns service integration, CMake, UI, publication, and cleanup.
This is not an accepted general metadata policy or a public ABI change.

## Interface and ownership

`copy_regular_file_to_stage(source, stage, expected, cancelled, workspace)` is
synchronous and `noexcept`. It borrows the two paths, expected `ObjectIdentity`,
existing `CancellationCheck`, and a caller-owned `NativeCopyWorkspace`. The
workspace allocates one 256 KiB buffer in its constructor before file I/O and
is reused across the batch; constructor allocation failure can throw. Neither
the callback nor the workspace may be concurrently reused or destroyed during
an invocation. No callback state or borrow survives the call.

`NativeCopyResult` initializes its terminal, error, two close errors, copied
byte count, creation flag, and stage identity. Terminals are `complete`,
`cancelled`, `source_changed`, `failed`, and `callback_failed`. Native failures
retain platform error categories; logical validation/cancellation uses generic
errors. A source revision mismatch can have no native error because the native
observation itself succeeded. Callback exceptions are contained and yield
`callback_failed`; they are not propagated through C or to the caller.

Exclusive create refusal always returns `stage_created=false` and an unavailable
stage identity. A successful create sets the flag immediately and observes the
new file through the same handle. If this first identity observation fails,
creation is reported but the caller must preserve the uncertain path. Otherwise
the returned identity is refreshed from the handle before closing; a failed
refresh retains the previously established object identity. Its revision is not
a promise about timestamps after Windows closes the writer. Cleanup/publication
must independently revalidate object identity and route. The helper never
publishes, renames, removes, or rolls back a path.

Both primary handles are explicitly closed on every terminal path, with
separate `source_close_error` and `stage_close_error`. Close failures prevent
`complete`, and remain visible alongside a pre-existing cancellation/failure.
The additional source-path verification handle reports any close failure as the
primary failure. Automatic handle cleanup is the exception fallback. POSIX
`close` is not retried after EINTR because that can close a reused descriptor;
close interruption is reported, with no claim that every platform guarantees
the same descriptor disposition.

## Implemented behavior and limits

**OBSERVED:** The source opens with no-follow semantics. Windows additionally
requires a disk handle; POSIX uses nonblocking open so a nonregular source such
as a FIFO cannot block before type validation. The same handle must report a
regular file and `same_revision(expected)` before destination creation. The
destination uses Windows `CREATE_NEW` or POSIX `O_CREAT|O_EXCL|O_NOFOLLOW`.

Windows/Linux request at most the remaining expected extent and 256 KiB per
read, handle short reads/writes, reject zero progress, and poll cancellation
before reads and each pending write. POSIX interrupted reads/writes retry only
after returning to the cancellation boundary. Open retries EINTR. Buffer
storage is never resized or allocated within the copy loop. Successful byte
counts are accumulated only after writes report success.

Completion requires the exact expected copied count and stage size, a new
source-handle revision check, and a fresh no-follow source-path open whose
identity/revision matches expected. Windows applies the observed read-only
attribute through the stage handle. POSIX applies only ordinary mode bits
(`0777`); setuid/setgid/sticky bits are not selected for copying. No explicit
alternate-stream, arbitrary xattr, ACL, ownership, old timestamp, sparse, or
clone policy is added by these loops. Permissions are finalized on successful
copies; incomplete stages are cleanup material, not publishable files.

Parent routes remain pathname-resolved, not pinned directory authorities.
Source writers can race observations, including edits with indistinguishable
revision fingerprints. Changes can occur after the final check. A matching
revision is not a filesystem snapshot or a content hash. Cancellation has no
hard I/O deadline: individual native calls can block. Disk durability is not
claimed; no fsync/FlushFileBuffers policy was introduced.

### macOS route and adoption gate

**OBSERVED:** The Apple branch uses already-opened descriptors with
`fcopyfile(COPYFILE_DATA)`. A named synchronous callback borrows an explicit
context. It returns `COPYFILE_QUIT` on requested cancellation, caught callback
exceptions, or `COPYFILE_ERR` (never retrying an error indefinitely). It reads
`COPYFILE_STATE_COPIED` into `off_t`, checks conversion and monotonicity, and
checks the expected extent. The final count is read again before freeing state.
If that query fails, the last confirmed count remains a lower bound and an
error is returned; an existing callback-failed/cancelled terminal is retained.
There is no clone flag or block-size state constant. macOS cancellation
granularity is native write callbacks, not a proven 256 KiB bound. A changing
source can exceed the expected extent in a native write before the callback
detects it; that stage is never returned complete.

Apple's [copyfile manual](https://raw.githubusercontent.com/apple-oss-distributions/copyfile/main/copyfile.3)
documents descriptor copying, write callbacks, quit/error behavior, and the
`off_t` count. The [header](https://raw.githubusercontent.com/apple-oss-distributions/copyfile/main/copyfile.h)
provides the callback/state interface. The [implementation](https://raw.githubusercontent.com/apple-oss-distributions/copyfile/main/copyfile.c)
shows the callback-address convention and quarantine processing inside the
descriptor route. The adapter leaves that route intact and does not request
broader metadata flags. State receives no owned filenames, so descriptor
ownership stays with the adapter rather than path-based state cleanup.

**GIVEN adoption gate:** native macOS compilation/execution and metadata
equivalence must pass before root adopts this branch. The new macOS-only test
compares the helper to `std::filesystem::copy_file` on generated ordinary xattr
and quarantine fixtures, after source readback. Missing fixture capability is
printed as `UNMEASURED`, not preservation success. Root reported the independent
baseline matrix at `5d01f4d` preserves quarantine but drops ordinary xattr and
old mtime; that observation is not proof that this new helper is equivalent.
The existing baseline's macOS source was fully allocated, so no sparse
preservation conclusion follows from it.

## Local validation

**MEASURED:** Windows 11 Home build 22621, x64, NTFS; adjacent read-only MinGW
GCC 16.2.0. Standalone compilation used one compiler process, C++20, and
`-O2 -Wall -Wextra -Wpedantic -Werror`. No CMake invocation or dependency rebuild
was performed. The existing metadata-development model archive supplies native
observation functions. This does not certify that archive's unrelated code.

From the repository root, with the adjacent MinGW bin directory prepended to
PATH:

```powershell
& 'C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/g++.exe' -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -I frontend/include -I frontend/src frontend/src/native_copy.cpp frontend/tests/native_copy_tests.cpp frontend/.build/metadata-development/libfile_manager_frontend_model.a -lole32 -luuid -lshell32 -ladvapi32 -o frontend/.build/native-copy-private/native_copy_tests.exe
& frontend/.build/native-copy-private/native_copy_tests.exe
& 'C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/python.exe' tools/check_house_style.py frontend/src/native_copy.hpp frontend/src/native_copy.cpp frontend/tests/native_copy_tests.cpp
```

Final compile and tests exited zero. All requested generated-fixture checks
passed: empty/small/multichunk bytes, source and created-stage identities,
existing destination preservation with no claimed ownership, cancellation
before creation, cancellation after copied data, post-write callback exception
containment, source revision mismatch, ordinary/read-only permissions, and
workspace reuse. Windows post-write tests stop within the first 256 KiB chunk.
Fixtures are exclusively created in a random temporary directory. Their owner
checks the exact canonical parent/root and root identity before nonrecursive
cleanup of registered direct children, restores write permission, and verifies
root removal. Tests do not edit user files or run concurrent hostile writers.

macOS and Linux adapter code/tests have not been compiled or run here. Native
metadata comparison, close-error behavior, native short-write/EINTR failures,
and the Mac callback error path still need platform evidence; no general
fault-injection harness was introduced. The tests exercise a deliberately
throwing named callback only against their own generated data.

## Exact semantic review scope

**OBSERVED:** Reviewed all authored lines of `native_copy.hpp`,
`native_copy.cpp` (including all platform branches), `native_copy_tests.cpp`,
and this receipt against the complete `planning/PROGRAMMING_HOUSE_STYLE.md`.
The review covers explicit types/initialization, enum terminal states,
calculate-before-return order, checked native count conversions, partial-write
arithmetic, ownership on exclusive-create failure, handle lifetime/close
errors, buffer extent and reuse, callback context lifetime and exception
containment, and fixture cleanup boundaries. The workspace and file owners are
noncopyable; raw memory/handle access is confined to I/O and observation
boundaries. There are no anonymous callbacks or retained deferred borrows.
No known house-style violations remain in this authored scope. The spelling
checker reported three files and zero findings; it does not replace semantic
review. Existing model/library, integration, UI, vendored, and generated source
are outside this review.

## Retained failed attempts

The first link used the older `frontend-style-final` model archive, which lacks
`observation_from_handle`, and omitted required Windows platform libraries. It
failed without running tests. The next link used the existing
`metadata-development` archive plus Windows libraries and succeeded.

The first test run failed its post-write cancellation assertion because its
observer used the CRT-backed `std::filesystem::file_size` path projection while
the writer remained open. Changing only the test observer to the existing fresh
native-handle identity observation made cancellation and exception tests pass.
The copy loop was unchanged by this correction. The failed run still reported
owned-scope cleanup. Raw failures and final output follow; ignored local logs
also remain under `frontend/.build/native-copy-private/`.

### Initial link failure

```text
C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/../lib/gcc/x86_64-w64-mingw32/16.2.0/../../../../x86_64-w64-mingw32/bin/ld.exe: C:\Users\Shadow\AppData\Local\Temp\cc8VHcgz.o:native_copy.cpp:(.text+0xd1): undefined reference to `file_manager::observation_from_handle(void*)'
C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/../lib/gcc/x86_64-w64-mingw32/16.2.0/../../../../x86_64-w64-mingw32/bin/ld.exe: C:\Users\Shadow\AppData\Local\Temp\cc8VHcgz.o:native_copy.cpp:(.text+0x3bc): undefined reference to `file_manager::observation_from_handle(void*)'
C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/../lib/gcc/x86_64-w64-mingw32/16.2.0/../../../../x86_64-w64-mingw32/bin/ld.exe: C:\Users\Shadow\AppData\Local\Temp\cc8VHcgz.o:native_copy.cpp:(.text+0x4dc): undefined reference to `file_manager::observation_from_handle(void*)'
C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/../lib/gcc/x86_64-w64-mingw32/16.2.0/../../../../x86_64-w64-mingw32/bin/ld.exe: C:\Users\Shadow\AppData\Local\Temp\cc8VHcgz.o:native_copy.cpp:(.text+0x778): undefined reference to `file_manager::observation_from_handle(void*)'
C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/../lib/gcc/x86_64-w64-mingw32/16.2.0/../../../../x86_64-w64-mingw32/bin/ld.exe: C:\Users\Shadow\AppData\Local\Temp\cc8VHcgz.o:native_copy.cpp:(.text+0x9f1): undefined reference to `file_manager::observation_from_handle(void*)'
C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/../lib/gcc/x86_64-w64-mingw32/16.2.0/../../../../x86_64-w64-mingw32/bin/ld.exe: frontend/.build/frontend-style-final/libfile_manager_frontend_model.a(platform_paths.cpp.obj):platform_paths.cpp:(.text+0xd37): undefined reference to `__imp_CoTaskMemFree'
C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/../lib/gcc/x86_64-w64-mingw32/16.2.0/../../../../x86_64-w64-mingw32/bin/ld.exe: frontend/.build/frontend-style-final/libfile_manager_frontend_model.a(platform_paths.cpp.obj):platform_paths.cpp:(.rdata$.refptr.FOLDERID_Profile[.refptr.FOLDERID_Profile]+0x0): undefined reference to `FOLDERID_Profile'
collect2.exe: error: ld returned 1 exit status
compile_exit=1
```

### First test run

```text
owned_temp=C:/Users/Shadow/AppData/Local/Temp/file-manager-native-copy-1300271239
FAIL post-write cancellation/exception not contained
PASS empty/small/multichunk; contents and identities
PASS existing destination preserved; no ownership
owned_cleanup=verified and removed
test_exit=1
```

### Final local output

```text
compile_exit=0
owned_temp=C:/Users/Shadow/AppData/Local/Temp/file-manager-native-copy-2680960062
PASS empty/small/multichunk; contents and identities
PASS existing destination preserved; no ownership
PASS before-create cancellation; post-write cancellation and callback exception
PASS source revision mismatch before creation
PASS ordinary and read-only permissions
owned_cleanup=verified and removed
PASS native copy fixtures; workspace reused
test_exit=0
3 files; 0 spelling findings/review candidates. Semantic review still required.
spelling_exit=0
```
