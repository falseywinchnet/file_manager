# Separate bounded scan-reader candidate — 2026-10-03

**REJECTED for admission: the no-regression gate was not established.**
Six short alternating pairs do not establish statistical significance or prove
that the candidate causes a slowdown. No threshold was relaxed, further
optimization attempted, public capability activated or production path changed.
The candidate is archived and its four active source edits have been reversed.

## Candidate and baseline

**GIVEN:** one private Engine candidate, generated-fixture checks, narrow pure
decode extraction, and a receipt. Root subsequently authorized preservation
under `results/rejected/` and removal of the owned implementation. No Git
operations, workers, GUI edits or cache-policy changes occurred.

**HYPOTHESIS:** keep context, logical byte/string limits, sticky failure and
reusable string scratch on a separate accessor; leave legacy Reader methods
and cache access free of optional budget dispatch. Binding/path field extraction
is shared through pure helpers; object decoding uses existing `decodeObject`.
No second format decoder or whole-catalogue mirror was introduced.

The current Engine was copied before edits. Root identified its baseline as
`d4dcb245`; no Git command independently checked that revision. Both earlier
rejected bounded-reader receipts and preserved follow-up were read. The
comparison used this assignment's captured baseline, not the older experiment's
baseline. The archived aggregate harness was identical in both copies.

**OBSERVED:** shared binding/path helpers inline at costs 35/28, including in
legacy methods; legacy `readString` retains inline cost 78. `cache.go` remained
byte-identical. The intended structure was achieved; that alone does not prove
performance neutrality.

The accessor precharges logical requested bytes, including cache hits and reads
that subsequently fail. Refused reservations do not charge. Component/string
extents are validated by subtraction before offset addition or string allocation.
Context checks surround block-aligned cache requests of at most 32 KiB. Returned
strings own their bytes; errors publish empty row/record results and remain sticky.
The caller must retain the checked immutable reader and exclude concurrent
Check/Close. No service lease or query state was added to Reader.

**Cost and limit:** construction and every Row/Record call perform segment-handle
`file.Stat`. A matched substring row invokes both methods. This is not a
cache-only scan or current-result-path verification. Stat, cache-lock waits and
native ReadAt are synchronous; surrounding context checks provide no hard
wall-clock deadline. Physical prefetch and complete query/output storage are
not bounded by the logical counter.

## Validation and paired result

Measured source, Go 1.27.1, `GOMAXPROCS=2`, `GOTOOLCHAIN=local`, one compiler job:

| Command/control | Result |
|---|---|
| Focused `TestBoundedScan*` in generation/benchmarks | Passed |
| `go test -p 1 ./...` | Passed |
| `go test -race -p 1 ./...` | Passed |
| `go vet -p 1 ./...` | Passed; no diagnostics |
| Independent ten-query 10k substring oracle | Complete record digests/counts matched; empty pages progressed |
| Twelve legacy aggregate runs | Passed; exact-path/name/pipeline digests unchanged |

Generated-fixture checks cover complete legacy row/record equality, EOF, exact
budgets, insufficient row budgets, cache-hit charging, pre-allocation refusal,
sticky errors, independent request state, closed warm-cache reader rejection,
70,000-byte owned strings and real cancellation at a chunk checkpoint. No new
storage attack reproduction or fault-injection framework was added. The initial
failed run is preserved: one uint64/int comparison needed explicit conversion;
one closed-reader assertion incorrectly required os.ErrClosed instead of
accepting Windows' invalid-handle error with empty output.

Environment: Windows 11 Home 10.0.22621, amd64, C: NTFS, reported AMD EPYC 9354,
16,757,176 KiB visible RAM. Root was compiler-clear during measurement; external
host load/power/storage state was not controlled. All binaries were built before
timing. Existing ScaleV1(10000,10,10) performs 20,000 path, 2,000 name-first-100
and 2,000 pipeline-first-100 queries per run, using its recovered/warmed setup.
Baseline runs first on even pairs; candidate first on odd pairs.

| Control | Baseline median aggregate ns/query | Candidate median aggregate ns/query | Median paired change |
|---|---:|---:|---:|
| Exact path | 2,031.30 | 2,090.35 | +3.1589% |
| Name first 100 | 45,717.40 | 46,016.10 | +1.1777% |
| Name pipeline first 100 | 58,154.65 | 59,636.00 | +1.7585% |

Percent changes are calculated per pair before taking the median. These are
aggregate means, not reliable per-query percentiles; individual pairs include
gains and losses. Allocated bytes/query were effectively unchanged. The data
fails to establish the required no-regression result; it does not establish
statistical significance, causal attribution or a bounded-scan speed claim.
Raw elapsed times, allocation counters, digests and all negative observations
remain in the archive.

## Canonical-profile feasibility

The root-owned catalogue substring development profile and negotiation round
010 were reviewed without edits. No contradiction was identified in the logical
read semantics. Material producer gaps remain: progressing page stops versus
terminal request failures; oversized single-row handling; complete response
encoding/envelope accounting; the 8 MiB total query-storage ceiling; voluntary
timeslice versus request deadline; authenticated/stale/restart cursors; transport
cancellation/error mapping; broader predicate and nested-root conformance;
10k/100k/1m page distributions; native integration and frontend provenance.
Per-row Stat cost must be included in future scan measurements. This private
control is not proof of those gates or of full service conformance.

The earlier cache-replacement validity repair was not imported; its unchanged
failure-path dependency remains separate. Legacy path-offset addition was also
left unchanged; only the bounded accessor validates its own addition first.
No admission claim is made for either legacy repair.

## Evidence, source review and final state

Permanent evidence: `results/rejected/bounded-scan-reader-2026-10-03/`.

- `manifest.json` lists archive/final-source hashes and validation limits;
  `measured-manifest.json` records measured source/binary/log hashes and raw data.
- `measured.patch` is the tested/timed four-file delta; `candidate.patch` adds
  seven final named-return house-style corrections. Both reconstruct with no
  fuzz and match their captured sources after stated CRLF/LF normalization.
- `sources.zip` holds the baseline and measured candidate with identical timing
  harnesses already applied, plus the four final source files.
- `logs.zip` preserves initial failures, focused/full checks, compiler reports,
  all paired runs and analysis. Preparation/analysis/archival scripts and the
  checked restoration input are retained alongside it.

**Validation distinction:** full tests/race/vet and timing precede the seven
final return-expression style corrections. The final patch has source review
and reconstruction verification only. No compiler ran after root closed this
chat's compiler slot. Root independently verified all nine artifact hashes and
the unchanged restored segment.

**Exact house-style review scope:** the final bounded accessor, both new Go test
files, authored decoder-extraction statements in segment.go, and saved
prepare.ps1, audit.py and archive.ps1. Review considered explicit initialized
types, named callback state, ownership/borrows, conversions, ordering, failure
outputs and repeated storage/work. The measured Go version's seven direct-call
return expressions were corrected in the final archive; no further Go violation
was identified in that authored scope. Untouched legacy methods, existing
harness/fixture code and vendor code were not certified compliant.

**Remaining tooling style issues:** audit.py retains chained path/read/decode,
hash/hexdigest, JSON serialization/write and other expressions that obscure
intermediate types/allocations. The PowerShell scripts also retain composed
path/read/hash expressions. These exact scripts and their hashes are preserved
as one-off historical experiment tooling, not accepted maintained tools. No
blanket house-style compliance is claimed for them.

After archive/hash/reconstruction checks, only
`engine/internal/generation/segment.go` was restored and only these new files
were removed: `engine/internal/generation/bounded_scan_candidate.go`,
`engine/internal/generation/bounded_scan_candidate_test.go`, and
`engine/benchmarks/bounded_scan_candidate_test.go`. Pre-removal hashes excluded
concurrent edits. No candidate implementation remains active. CPU/compiler work
is released; authoring stops at this checkpoint.
