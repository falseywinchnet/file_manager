# Native process observations

2026-10-04 UTC. **MEASURED local Windows result:** GCC 16.2.0, Release,
Windows Shadow desktop, one compiler job, commands from the experiment README.
Both CTest cases pass. The final local run's ten lifecycle children all exited
and were observed before reuse. Four 250 ms cancellation waits plus launch and
cleanup took 270, 260, 257 and 257 ms. Normal/failure/recovery cases took 19–22 ms.
These are single-run observations, not a latency distribution or product budget.
The unlimited 384 MiB commit request succeeded; the same request under the
256 MiB job limit was refused. The raw local LastTest.log is retained beside
this record. There is no native Mac/Linux claim before CI results arrive.

Root source review covers `process.hpp`, `main.cpp`, `windows_process.cpp`,
`posix_process.cpp`, `CMakeLists.txt`, the dedicated workflow and README against
the full `planning/PROGRAMMING_HOUSE_STYLE.md`. Explicit state/types, fixed work,
named execution, pre-launch path storage, synchronous borrows, Windows handle
ownership, termination/wait ordering, numeric bounds and failure status were
inspected. POSIX borrowed argv casts adapt execv's historical mutable signature;
the buffers are not mutated. No retained closures or first-party heap pixels.
Independent source review is preserved in `SOURCE_REVIEW.md`; it accepted the
seven-file scope including the Mac extension, with one header wording correction
subsequently applied by root. This is not a claim about legacy runtime
or any production supervisor. Functional tests and the spelling scanner do not
replace that review.

No memory equivalence between Windows commit and POSIX address space is claimed.
The Mac skip is an explicit unresolved capability. Only the child itself is
tested, not descendants, hostile native code or filesystem/network confinement.
Failure paths for OS launch/assign/kill/wait errors are inspected but not all
fault-injected. The POSIX final reap is blocking and Windows creation is not
bounded by the later wait; the README preserves these experimental limitations.

## First native matrix: 03561cec01eaf848cace619dd984de219ed32c14

[Run 37174876232](https://github.com/falseywinchnet/file_manager/actions/runs/37174876232)
completed on all three native hosts. Raw logs are retained as
`Native-{Windows,Mac,Linux}-03561ce.log`.

- Windows/MSVC: lifecycle and memory admission tests passed. Job
  `111355323283`; all four terminated children observed, recovery passed.
- Ubuntu 24.04/GCC: both passed. Job `111355323130`; all four terminated
  children observed, recovery passed; 384 MiB mapping refused under RLIMIT_AS.
- macOS 26/AppleClang 21.0.0.21000101: lifecycle passed; memory test **skipped**.
  Job `111355323236`; setrlimit returned errno 22 (`EINVAL`) before allocation.
  Unlimited control succeeded. The four cancellations/reaps and recoveries
  passed; no Mac resource cap is established.

CTest's LastTest.log says `Test Passed` even for its configured skip. The Mac
console explicitly reports `preview_process_memory ... ***Skipped` and lists it
under tests that did not run. The child's `LIMIT_UNAVAILABLE` output and exit 77
in the retained log are authoritative to interpret that status. The workflow's
green conclusion must not be promoted to a passing Mac memory capability.

## Headroom follow-up: 4c1f6a4d321554184b283a6df2491188ab4effeb

[Run 37175076041](https://github.com/falseywinchnet/file_manager/actions/runs/37175076041)
passed its native workflow. Windows and Linux passed both tests. Mac passed
lifecycle and the additional headroom test; its original fixed-limit test
remained **skipped**. Raw native logs with suffix `4c1f6a4` preserve all outputs.

Mac job `111355907120` measured 445,746,298,880 virtual bytes and 1,277,952 resident
bytes before setting a ceiling of 446,014,734,336 bytes. That baseline plus
256 MiB ceiling was accepted, and the subsequent 384 MiB mapping was refused.
The unrestricted control mapping succeeded. This is evidence for additional
mapping admission on this macOS 26 runner, not a resident/commit cap, codec
footprint, stable process startup budget or secure sandbox. Existing reservations
can still gain residency. Source and semantic policy must account for that gap
before production integration; the absolute 256 MiB Mac proposal stays rejected.

The final source review covers this headroom extension. Root's subsequent header
comment and evidence/acceptance-record updates change no executable behavior.
Full application workflow checks remain a separate PR gate; this laboratory
does not create a new dogfood release or render JPEGs in File Manager.
