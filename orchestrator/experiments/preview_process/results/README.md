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
Independent source review is pending; this is not a claim about legacy runtime
or any production supervisor. Functional tests and the spelling scanner do not
replace that review.

No memory equivalence between Windows commit and POSIX address space is claimed.
The Mac skip is an explicit unresolved capability. Only the child itself is
tested, not descendants, hostile native code or filesystem/network confinement.
Failure paths for OS launch/assign/kill/wait errors are inspected but not all
fault-injected. The POSIX final reap is blocking and Windows creation is not
bounded by the later wait; the README preserves these experimental limitations.
