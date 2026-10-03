# Search page preparation and UI publication — 2026-10-02

Status: **MEASURED Windows consumer component improvement; native integration
of the new worker preparation is pending.** No indexed-search acceleration or
whole-application smoothness claim follows from this experiment.

## Behavior and ownership

**OBSERVED baseline:** Application::apply_engine_search and apply_engine_criteria
performed route checks, native identity/metadata observations, display formatting
and accumulated collection replacement synchronously on the UI thread. Ordinary
SearchWork and CriteriaWork now prepare owned DirectoryEntry observations on the
existing worker after their service reply. UI completion consumes those values
and preserves generation rejection, deduplication, selection and page provenance.
No GUI objects or borrowed filesystem handles cross into preparation.

prepare_search_page preserves parent-link/out-of-root/unavailable/absent refusal,
uses native facts rather than provider display metadata, and checks cancellation
before/between observations and before publication. Cancellation discards the
whole prepared page. Pages above the existing frontend maximum of 500 results
fail before entry allocation/observation. An in-progress native call remains
synchronous. Raw provider result/name vectors are retired before completion.
Criteria replies from the wrong source skip observation and retain their normal
UI refusal. The wire contract and installed picker/GUI.Forms API are unchanged.

Prepared identity and metadata describe the path observation during preparation.
They do not prove equality to a cached provider match or continuing freshness.
Operations retain their own identity/revision revalidation. Search evidence now
says "Filesystem observation" and attributes the match to the provider. The
typed match-identity/currentness projection gap remains open; exact-criteria
freshness is not repaired by moving observation between threads.

## Workload and comparison

**MEASURED environment:** Windows 11 Home 10.0.22621, amd64, NTFS C:,
AMD EPYC 9354 reported, 16,757,176 KiB visible RAM; MSYS2 GNU C++ 16.2.0,
CMake Release `-O3 -DNDEBUG`, two build jobs. Adjacent toolchain consumed
read-only. Engine sibling confirmed CPU clear before compilation and measurement;
no local compiler/profile job overlapped these samples. External host load and
storage hardware were not independently controlled.

`file_manager_application_latency_benchmark --search-projection` creates exactly
1,000 temporary regular files named entry-0.txt through entry-999.txt, each with
the ten binary bytes `benchmark\n`. It never scans personal content. Each run
uses a new, verified temporary child directory; all writes/cleanup remain there.
Page contents are explicitly simulated provider data over real native files.
No live service readiness is claimed. Each Application worker remains idle in
this component harness; preparation is timed as a separate synchronous phase
representing the operation now called by production SearchWork/CriteriaWork.

Each process measures 30 replacements each at 25, 100 and 500 results, then ten
repetitions of ten appended 100-result pages (1,000 accumulated rows). Fixture,
Application/window setup, page construction, output logging and result checks
are outside timed intervals. Each publication checks count and identity-available
first/last generated entries. Retained layout is measured separately; native
input, paint, presentation, transport, provider work and queue wait are excluded.
The benchmark does not bind bootstrap/navigation or start a native event loop.

Six process pairs alternate baseline-first and worker-first. Both variants use
the same generation formula, contents and root-name shape on the same filesystem,
but fresh temporary directories/file identities per process. Raw CSV retains all
190 samples per run. Summary excludes repetition zero of each case: 174 pooled
warm samples per first-page case; 54 for the final appended page. P95/P99 use
nearest rank; these pooled observations are not a stable end-to-end tail estimate.

| Page | Baseline UI apply p50 / p95 / p99 / max, ms | Prepared UI apply p50 / p95 / p99 / max, ms |
|---|---|---|
| 25 first results | 1.957 / 3.564 / 3.688 / 3.958 | 0.131 / 0.203 / 0.292 / 0.384 |
| 100 first results | 7.484 / 12.794 / 13.805 / 13.951 | 0.413 / 0.712 / 0.775 / 1.157 |
| 500 first results | 41.259 / 52.063 / 55.353 / 55.935 | 2.593 / 4.326 / 4.957 / 5.053 |
| Final 100 of 1,000 accumulated | 11.171 / 16.593 / 17.666 / 17.666 | 3.725 / 7.097 / 8.101 / 8.101 |

Preparation p50 was 1.896 / 7.551 / 42.058 / 7.303 ms respectively. It still
does real work; it was moved off the UI, not eliminated. Total operation cost
can increase, particularly due to owned staging and scheduling. Do not add
independent medians and describe the sum as a measured total percentile.
Retained-layout medians stayed approximately 0.10–0.15 ms. The accumulated
rebuild still grows with loaded results; the observed 8.101 ms UI maximum at
1,000 rows is a remaining optimization target, not proof of unlimited scale.

## Reproduction anchors and validation

Baseline product source: `6009dd76e70e16edb9c1a6b393d3aa5db4db3b01`, now
rebase-merged as `8f84952ad8ac86fb1ad43124a9f3ac38c931a501` with identical tree
`468cb119b5b4f609e100ff70bea28cc13392b4b8`. Both native matrices for that preceding
queued-search checkpoint passed: 36994098674 and 36994102785. They do not verify
the later preparation change.

Preserved baseline executable SHA-256:
`7b483e51c52d412beed14572eb6d4aa83456a668ad4d4a6ece5d1fcffa7d8b3c`.
Measured candidate executable:
`d60cc26de886e8d3d88e2a72072918a5ad18741affc33acbb1d00fcf4a465887`.
Measured preparation source:
`0624aba41085726246f7593338c449c9e454584619d448d0f04569f519262d66`;
application.cpp:
`6ca65290049271563f8d41b112830ddbc42855adae7fe3769c48164fc23c4dcb`;
application_jobs.hpp:
`53a1cbf2153abd4ce87f31c4c93502f8999308023a21a838b53e9c2a8e298f29`.

The baseline executable was copied before rebuilding the changed Application.
`baseline-harness.patch` reconstructs its workload against 6009dd7 by reversing
the preparation instrumentation; it also includes the later named count return
outside timed intervals. The current harness adds the separate preparation
interval. After measurement, its count accessor received that style correction
and its usage text gained the new mode; no timed production code changed.
The final benchmark rebuild and complete 190-sample smoke run also pass.
`measure_search_publication.py` reproduces the alternating runs when the two
executables are supplied under `.build/details-consumer/` with matching runtime
DLLs. The runner's later explicit loop declarations and relocated ROOT affect
orchestration only. Raw CSV and summary.json are retained here.

**MEASURED:** matching Windows frontend build and all 12 tests pass (2.85 s).
New cases cover native facts versus provider labels, absent/outside/unavailable
paths, parent symlink refusal where host fixture creation is supported, immediate
and between-record cancellation, oversized pages and owned values after source
deletion. Existing search/criteria selection and duplicate handling tests pass.
This Windows run lacked symlink creation privilege (error 1314), so link-specific
assertions were skipped; they require the native Mac/Linux run or an enabled
Windows fixture host. WindowsLastTest.log preserves those skips and test results.
An initial assertion assumed the fixture's text stream wrote five bytes; Windows
CRLF produced six. The test was corrected to compare the independent file-size
observation. No product size calculation was changed for that test failure.

**House-style review:** all new search_preparation source/header; changed
Application publication bodies and SearchWork/CriteriaWork/ready values; adapted
interaction probes and new cases; new benchmark mode/temporary-root validation;
the named measurement runner and CMake entry. Explicit types/initialization,
named cancellation state, synchronous borrow lifetime, owned transfer, atomic
supersession, per-page bounds, failure states and per-row work were reviewed.
Seven C++ files pass the spelling scanner. Unchanged legacy benchmark methods,
provider identity semantics, whole Application and GUI.Forms are not certified.

**Open acceptance:** native Windows/macOS/Linux integration at this new source;
actual service-to-worker-to-UI cancellation/race coverage; physical interaction;
RSS/peak staging memory, physical I/O and CPU attribution; deep/long/large trees,
hard links/replacement/currentness, longer accumulated lists and native paint.
This experiment identifies and moves a measured UI blocking cost; it does not
complete the search or responsiveness objective.

## Subsequent native verification and delivery

2026-10-03 UTC: source `81473681782555fd1cd3b949273b4bcf74a882f6` passed both
complete native matrices, runs `36997543145` and `36997653188`. PR 9 rebase-merged
as `8f20ccaec656c18909bdae27a3316d4b6f4aff05`; both source trees are
`ef425f7897d389ef4e68c522c3d0183227ce368b`. Independent download checks verified
clean source receipts, SHA-256 sidecars and all 40 Linux/42 Mac/44 Windows
receipt-listed files. Published archive digests read back from GitHub match.
Release: `https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.8147368`.

The actual Mac preview harness passed text/PNG/unsupported/Details checks;
its saved text-preview screenshot was visually reviewed. Caption/readable-text
checks counted 235/1078 pixels and the four-column Details header check passed.
These checks close the earlier native build/integration gate for this source,
not physical search interaction, end-to-end latency or all cross-platform cases.
The later coverage projection is excluded from this release.
