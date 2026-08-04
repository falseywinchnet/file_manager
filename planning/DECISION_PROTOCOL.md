# Decision and research protocol

## 1. Claim grammar

Every consequential statement carries one status:

| Status | Meaning | May drive architecture? |
|---|---|---|
| GIVEN | Directly required/excluded by grand architect | Yes |
| OBSERVED | Direct code/source fact with locator | As a constraint, not a forecast |
| MEASURED | Reproducible result under named conditions | Within measured domain |
| HYPOTHESIS | Falsifiable proposal | No; creates an experiment |
| CANDIDATE | Option admitted for comparison | No |
| REJECTED | Failed a declared gate | No, absent a new mechanism |
| DECIDED | Explicit accepted record | Yes |

Words such as “native,” “fast,” “semantic,” “lightweight,” and “safe” are not
statuses. They need definitions.

## 2. Architecture decision record

Every high-reversal-cost choice will use:

```text
# ADR-NNN: Question
Status: proposed | accepted | superseded | rejected
Date:
Owner approval:

## GIVEN constraints
## Workloads and failure modes
## Candidates
## Evidence and measurements
## Decision
## Why the other candidates lost
## Consequences
## Reversal and migration path
## Unresolved edges
```

No ADR is accepted merely because code already exists.

## 3. Experiment record

Each spike or benchmark records:

- claim under test and rejection criterion;
- baseline and guarded alternatives;
- exact code revision and dependency lock;
- OS, filesystem, machine, power state, storage, and cold/warm state;
- synthetic fixtures plus representative real fixtures;
- correctness oracle before performance;
- latency distribution, throughput, CPU, memory, bytes read/written, and index
  write amplification where relevant;
- raw outputs, summary, anomalies, and failures;
- scope of inference—what the result does not establish.

Median alone is inadequate for interactive work. Record at least p50, p95,
p99, and worst relevant stall, plus visible frame misses for UI tests.

## 4. Guard workloads

Candidate designs must face several incompatible pressures:

- empty and tiny directories;
- 10k, 100k, and million-entry synthetic directories where supported;
- deep trees, wide trees, long names, Unicode, combining marks, and mixed case;
- small files, huge files, sparse files, packages/bundles, links, hard links;
- local SSD, removable media, slow disks, temporarily unavailable volumes;
- event storms, bulk extract/build trees, renames, and moves;
- cold launch, warm launch, service absent, stale index, damaged index;
- assistive technology, keyboard-only operation, IME, scaling, multiple displays;
- plugin crash, timeout, memory pressure, malicious file, malformed metadata;
- exact search, typo search, ambiguous search, semantic search, and “no result.”

An optimization that wins one guard and loses another becomes conditional or is
rejected; it does not silently become the default.

## 5. Retrieval evaluation

Keep separate scores for:

- exact identity and exact metadata predicates;
- lexical relevance;
- structural/containment similarity;
- typed semantic relations;
- temporal/user-context inference;
- confidence and rejection.

Measure Recall@k, MRR/nDCG where judged grades exist, false positives for
contradiction/antonym/association, ambiguity retention, calibration, index cost,
and end-to-end latency. Every semantic result must expose the exact records and
evidence channels responsible for its rank.

No embedding is allowed to overwrite a fact. No extracted description is allowed
to impersonate file contents. Model version, extractor version, confidence, and
source anchor travel with derived metadata.

## 6. Negative-results ledger

Rejected work remains in a future `research/rejected/` or experiment record with:

- what was attempted;
- the strongest version tested;
- what it improved;
- what it harmed;
- why the objective or mechanism was wrong;
- what genuinely new evidence would justify another attempt.

This is institutional memory, not clutter.
