# Independent read-only source review

2026-10-04 UTC, visible audit thread `01a0fb48-1437-7302-8b6f-f3bcf02e7319`.
Report preserved verbatim below. Root corrected the noted header wording afterward;
no executable behavior changed for that correction.

**No blocking findings in the current seven-file native process laboratory, including the new Mac headroom probe.** This is source-review acceptance for the disposable feasibility experiment; it does not establish production supervision or containment.

The reviewed paths support the documented scope:

- **Windows:** limits are configured before the suspended child runs. Job, process and thread handles have unique lifetime-bound owners. Assignment/resume failures take the direct termination-and-wait path. Failure to observe exit produces a failed result. The allocation probe measures commit admission without touching pages.
- **POSIX:** argument storage remains valid across fork/exec, and the child performs only `execv`/`_exit` before executable startup. Deadline cancellation targets the exact child and retries interrupted reaping. The blocking final reap and lack of a separate hard cleanup deadline are explicitly documented.
- **Mac headroom extension:** the Mach information buffer is initialized, supplied with `MACH_TASK_BASIC_INFO_COUNT`, and checked for both successful status and the expected returned count. The cast is confined to the Mach API boundary. The baseline conversion has a width assertion, and addition checks `baseline > maximum - headroom` before calculating the ceiling. The README correctly explains that limiting additional virtual mappings cannot cap residency arising within existing reservations or establish a Windows-equivalent commitment limit.
- **Evidence handling:** the unrestricted allocation control must succeed first. Only Mac converts unavailable or ineffective limits into skip status 77; Windows/Linux require refusal. The original fixed-limit test remains present alongside the headroom hypothesis. A skipped Mac test supplies no memory-cap evidence.

**One nonblocking documentation correction remains:** [process.hpp](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_process/process.hpp:26) says `run` owns its child “until exit is observed.” Windows can return `failed=true, reaped=false` after its final wait. The README describes this accurately; the header should likewise qualify the successful cleanup contract and explicitly permit an unobserved-exit failure result.

I reviewed the complete [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md), including explicit types, named behavior, ownership and borrows, initialization, conversions, operation order, failure handling, and repeated-loop storage. Beyond the header wording above, I found no remaining house-style violations in this scope:

- [CMakeLists.txt](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_process/CMakeLists.txt)
- [process.hpp](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_process/process.hpp)
- [main.cpp](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_process/main.cpp)
- [windows_process.cpp](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_process/windows_process.cpp)
- [posix_process.cpp](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_process/posix_process.cpp)
- [README.md](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_process/README.md)
- [preview-process.yml](C:/Users/Shadow/file_manager/.github/workflows/preview-process.yml)

This review covers the current working-tree bytes, including the extension added after `03561ce`. I ran no builds or tests and made no edits or Git mutations. The three-platform lifecycle passes, Windows/Linux allocation refusal, and original Mac `EINVAL`/skip are root-reported measurements; the new headroom probe’s runtime outcome remains outside this source-review conclusion.
