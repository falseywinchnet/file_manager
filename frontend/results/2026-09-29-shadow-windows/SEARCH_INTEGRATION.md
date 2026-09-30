# Windows frontend search integration

Status: **MEASURED development integration**, September 29, 2026 on Shadow
Windows x64. This is not installed-service, background-currentness, relevance,
or comparative search-engine performance acceptance.

The optional frontend probe uses the real `Application`, its ordinary initial
directory navigation, the search text field and `apply_filter`, its asynchronous
typed C++ Orchestrator client, and its result application/identity checks. It
does not inject fixture results into the UI model. GUI rendering and user input
are not exercised by this headless probe.

## First live-filesystem result

The Engine and Orchestrator sibling provided a private, owned temporary test
deployment under `orchestrator/.build/`. The source directory contained
`needle-report.txt`; root ID was `docs`. Separate Go Engine and Rust Orchestrator
processes communicated using their Windows local transports. No personal root
was scanned, no service was installed, and no system startup setting changed.

With `FILEMAN_ORCHESTRATOR_RUNTIME_DIR` set to that test deployment's
`orc-runtime`, the command shape was:

```text
file_manager_application_latency_benchmark.exe <fixture>/source --live-search docs needle needle-report.txt
```

The expected filename must occur in the actual applied search result collection
with an available exact filesystem identity. Initial directory listing alone
cannot satisfy the test. Query failure, cancellation or a missing result fails
the probe; the deadline is 30 seconds.

```text
live_search.initial,total_ms=28.3779,drain_ms=0.4737,max_drain_ms=0.472,drains=2,final_layout_ms=0.6722,flushes=1,measure_passes=2,arrange_passes=2
live_search,expected_file_present=true,elapsed_ms=30.5448
```

This is one small-fixture timing, not a latency distribution or scale claim.
The separately checked service response reported `source=live_filesystem`;
this first run does not prove indexed retrieval.

Executable SHA-256:
`1ebae140e7a123fcea9dd3e58bad6d2a53e57b0306a296224912a20414ac3fa1`.
Matching GUI.Forms DLL SHA-256:
`e89cf95f7c5599c0f3718589f456847f5bdfdaae13ba2586a7e6d42a5ff11b9b`.
These are the preceding coherent SDK artifacts, not the new multiline SDK.
## Current SDK repeat

After a clean frontend/picker rebuild against the multiline SDK, the same real
frontend query passed again: initial navigation 21.7404 ms, search 15.3626 ms.
The raw output is `search-current-sdk.txt`. These remain single small-fixture
observations, not a claim that the newer toolkit caused the timing difference.

Executable SHA-256:
`27c21fa19beb410fb20510cb4dc04fb43d0e11b810f66e08738e9e8b39f5d550`.
Matching GUI.Forms DLL SHA-256:
`aafc07514503f1e86ca2ef27687bddf8f121996831f21975a81d1b053ef7fd72`.

## Indexed deployment and a material semantic limitation

The provider then created a separate admitted manifest with indexing enabled,
separate private catalogue storage and a separate Orchestrator runtime, using
the same generated fixture directory. The current-SDK frontend probe passed
with query `needle-report.txt`: initial navigation 20.1805 ms, query 15.2553 ms.
Raw output is `search-indexed-current-sdk.txt`; executable and SDK hashes are
unchanged from the current-SDK live run above.

**OBSERVED / retained negative result:** query `needle` did not return the file
through the indexed route. Initial navigation was 18.4297 ms; the probe failed
its expected-result assertion. The existing catalogue text projection matches
an exact name, while the live route supports name/path substring matching.
Repeating with the full filename passed. Enabling indexing therefore does not
yet provide consistent general filename search semantics. This is unfinished
product work, not a reason to silently widen an authoritative catalogue
no-match into a live fallback or to claim content/lexical search support.

## Corrected ordinary-text route

The subsequent Orchestrator correction explicitly plans ordinary frontend text
as bounded name/path substring matching. A catalogue lacking that predicate is
classified unsupported before any catalogue query executes; the admitted live
route then handles it. Explicit exact-name metadata criteria and their
authoritative no-match results retain catalogue semantics. This is documented
as an ORC-FE compatibility correction, not indexed substring support.

Against a newly built Orchestrator with the same indexed Engine fixture, the
actual frontend query `needle` now passed: initial navigation 19.4576 ms,
search 30.5365 ms. Raw output is `search-indexed-partial-route.txt`. The frontend
probe/DLL hashes remain those of the current-SDK repeat above. Independent
typed-client checks reported `live_filesystem` with one result for ordinary
`needle`, `catalogue` with one result for explicit exact name
`needle-report.txt`, and `catalogue` with zero results for explicit exact name
`needle`. No failed exact query is reinterpreted as a fallback trigger.
