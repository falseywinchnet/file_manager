# Native preview process laboratory

2026-10-04 UTC. **CANDIDATE feasibility experiment**, independently buildable.
This extends the process evidence requested by
[`PREVIEW_PROVIDER_COMPARISON_2026-10-04.md`](../../../planning/PREVIEW_PROVIDER_COMPARISON_2026-10-04.md).
It does not implement an Orchestrator provider or activate ORC-PLG. Native C++20
is used to inspect OS calls directly; production Orchestrator supervision remains
Rust. No codec, user file, plugin, GUI or install target is present.

## Fixed workload

The parent launches only its own executable with a closed fixture argument.
CTest invokes it by its built absolute path. Ten sequential children exercise
normal exit, nonzero exit and four repetitions of deadline termination followed
by a successful fresh child. Every result records whether process exit was
actually observed. A five-second delay fixture is finite even if its parent
disappears. No descendants are created; descendant containment is unproved.

The separate memory test compares a 384 MiB allocation request without a limit
against the same request with a candidate 256 MiB limit. A passing memory test
requires the control to succeed and the limited request to fail. Neither request
touches pages: this is allocation admission, not resident-memory measurement.
The values are fixture constants, not a selected product budget.

- Windows creates an unnamed Job Object, configures kill-on-close and one active
  process, creates the child suspended with no inherited handles, assigns it to
  the job, then resumes its primary thread. The memory fixture additionally uses
  `JOB_OBJECT_LIMIT_PROCESS_MEMORY`, which limits committed virtual memory.
  On setup failure it attempts direct termination of the suspended child and
  waits for exit. On timeout it terminates and waits up to five seconds. Failure
  to observe exit is a failed result, never permission to claim cleanup.
- POSIX forks and immediately executes the fixture. The child executes only
  `execv`/`_exit` between fork and exec. The parent polls its exact child PID,
  kills it on deadline and calls `waitpid` until reaped (retrying interruptions).
  The memory fixture applies `RLIMIT_AS` **after loader startup** and before the
  allocation. This limits virtual address space, not committed or resident bytes.
  It does not establish a limit over executable loading. The test process is
  single-threaded; inherited POSIX file descriptors/environment are not a
  production capability policy. The blocking final reap has no separate hard
  deadline; CTest's 30-second timeout detects a stalled experiment.

Windows' 250 ms cancellation wait begins after resume; POSIX's deadline begins
before fork. Printed duration includes launch and cleanup. Neither establishes
a uniform hard wall-clock ceiling or an acceptable preview UX latency.
The controller stores one outcome and one child at a time, with no growing
queue or asynchronous callback state. Path/argument storage is prepared before
launch and remains borrowed through synchronous completion.

macOS can reject or fail to enforce the proposed small address-space limit.
Either observation produces explicit `LIMIT_UNAVAILABLE` and CTest skip code
77. A green workflow with that skip **does not establish a Mac memory cap**.
Windows/Linux require the limit to refuse the allocation. Setup, exec, wait,
unexpected signal or allocation errors fail the experiment.

The first native Mac run rejected 256 MiB with `EINVAL`. A separate Mac-only
headroom probe now measures `MACH_TASK_BASIC_INFO` virtual/resident bytes, then
tries an address-space ceiling of current virtual size plus 256 MiB. It retains
the original fixed-ceiling test and its negative result. Even if this second
probe refuses a new 384 MiB mapping, existing reserved ranges may become resident
without growing virtual size. It cannot establish a 256 MiB physical-memory cap
or a Windows-equivalent commit limit. This hypothesis follows the published
[XNU limit check](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/kern/kern_resource.c)
and [task-info layout](https://github.com/apple-oss-distributions/xnu/blob/main/osfmk/mach/task_info.h);
the runner result, not those source files, determines feasibility here.

## Build and authority

On Shadow, use the shared read-only toolchain and one compiler job:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake -S orchestrator/experiments/preview_process -B orchestrator/.build/preview-process -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build orchestrator/.build/preview-process --parallel 1
ctest --test-dir orchestrator/.build/preview-process --verbose --output-on-failure
```

The dedicated workflow records native Windows/MSVC, macOS/Clang and Linux/GCC
observations. The complete
[`PROGRAMMING_HOUSE_STYLE.md`](../../../planning/PROGRAMMING_HOUSE_STYLE.md)
applies to all authored source and tooling; functional success is not style
acceptance. This is ordinary authored C++, not the stricter generated profile.

Primary semantics: [Windows creation flags](https://learn.microsoft.com/en-us/windows/win32/procthread/process-creation-flags),
[Job memory accounting](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_extended_limit_information),
[Linux resource limits](https://man7.org/linux/man-pages/man2/setrlimit.2.html).
These do not establish filesystem/network confinement. Actual startup mapping
footprint, OS sandbox policy, descendant handling, crash recovery, bounded IPC,
source authority, result validation and production failure escalation remain
open. The selected-preview lifecycle model is not connected to this laboratory.
