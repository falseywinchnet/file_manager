# M5 Kolmogrov coupled-history tuple measurement 001

Status: **MEASURED disabled component evidence; experimental transfer admitted,
production fuzzy search and live-service integration remain blocked**.

Date: 2026-08-05.

Source manifest (Go/C files, `go.mod`, and `Makefile`):
`350c5001aa853c698ab54363165a04dbca3cba0a773c637767f5b57b427b26c1`.

Raw transcript: `results/raw/M5_KOLMOGROV_HISTORY_TUPLE_001.txt`, SHA-256
`0e80792505d881cbc8e01a99d19faaca98cc29d1dc0cd68322bf8f1ea7f15acc`.

## Claim boundary

This round transfers Kolmogrov's sealed literal-only radius-one coupled-history
guidance into native Go and measures candidate production plus exact
adjudication. It proves that the recurrence, three length plans, explicit
capacity/budget states, exact ordinal anchoring, immutable liveness, bounded
one-edit/transposition verifier, checked component file, and bounded direct
builder operate together.

It does not select the polynomial family or 40-bit width for production, freeze
tuple bytes as an ABI, connect the channel to the engine manifest/service,
measure judged relevance, admit folded/structural filters, implement radius two,
or establish update/compaction durability.

## Configuration and workload

- macOS 14.8.7, Apple M3 arm64, 8 logical CPUs, 8 GiB RAM;
- Go 1.26.5; host APFS SSD; warm filesystem/process state; cache not cleared;
- one million generated non-sensitive names, distributed over sixteen exact
  lengths so every partition remains below 65,536 live records;
- literal Unicode-scalar stream, two generation-random coordinates, 20 bits
  each, maximum 64 atoms, candidate cap 4,096, probe cap 129;
- 5,000 exact/substitution/adjacent-transposition/deletion/insertion queries;
- 100 identical queries also scanned all one million names with the same exact
  matrix-free verifier.

The names contain nearby hexadecimal counters and intentionally create many
legitimate one-edit neighbors. This is a resource/correctness stressor, not a
consented or judged relevance corpus.

## Correctness and negative evidence

- **OBSERVED:** the Go encoder matches the pinned Python-derived history/full
  keys in `testdata/similarity/history_tuple_reference_001.json`.
- **OBSERVED:** exhaustive strings over `abc`, lengths two through five, return
  exactly the flat verifier's equality/substitution/transposition/insertion/
  deletion set after candidate adjudication.
- **OBSERVED:** all 100 million-record flat-control comparisons have the same
  verified result count as the hash path.
- **OBSERVED:** all 5,000 mutated queries retain their exact source.
- **OBSERVED negative fixture:** the public 9-bit `ldfioia`/`kbmedfa` collision
  is returned by the hash and rejected by exact verification. Collision is
  candidate evidence and never identity.
- invalid UTF-8, NUL, lengths outside 2--64, mixed generations, descriptor
  mismatch, partition overflow, probe/posting/candidate exhaustion,
  cancellation, corrupt headers/descriptors/directories/postings, and stale
  liveness cardinality fail explicitly.

## One-million result

| Measurement | Result |
|---|---:|
| whole-posting memory control build | 6.817 s |
| memory-control total allocation / retained heap | 354,423,208 / 354,411,696 B |
| direct bounded build | 6.599 s |
| direct-build total allocation / retained heap after return | 46,327,912 / 2,400 B |
| maximum declared direct sort scratch | 27,000,000 B |
| immutable file / posting bytes | 199,739,911 / 199,476,576 B |
| posting / total component+liveness bytes per record | 199.48 / 199.86 B |
| full checksum/order open | 136.246 ms |
| off-heap retained heap delta before queries | 427,336 B |

| Warm path | Samples | p50 | p95 | p99 | max |
|---|---:|---:|---:|---:|---:|
| in-memory hash + exact verify | 1,000 | 21.833 us | 162.291 us | 202.208 us | 1.911 ms |
| off-heap hash + exact verify | 5,000 | 98.917 us | 276.084 us | 445.917 us | 6.113 ms |
| flat exact one-edit scan | 100 | 77.669 ms | 85.028 ms | 86.743 ms | 93.188 ms |

Off-heap p95 is about 308 times lower than the flat control p95 on the same
verifier semantics. This ratio is descriptive for this generated workload, not
a general quality or hardware claim.

| Work distribution | p50 | p95 | p99 | max |
|---|---:|---:|---:|---:|
| posting probes | 33 | 35 | 35 | 35 |
| posting visits | 1 | 253 | 255 | 256 |
| hash candidates before exact verify | 1 | 235 | 237 | 238 |

## Resource and write disposition

**MEASURED improvement:** the direct builder avoids the 354 MiB retained
whole-posting sort. It groups only exact ordinals by source length, sorts one
final packed partition under a 32 MiB hard limit, and writes every posting once
to its final component. The header, descriptor, and 262,148-byte range
directory fill previously unwritten reserved space; no temporary posting run or
rewrite is used. Configuration changes erase/rebuild this derived component.

**PASS component proxy:** verified fuzzy p95 0.276 ms is below the 35 ms
one-million fuzzy target.

**PASS component proxy:** off-heap retained state under 0.5 MiB leaves the
48 MiB service memory target plausible; complete service private memory and
idle behavior were not measured.

**UNRESOLVED storage gate:** 199.86 bytes/record is economical for this channel,
but it must be measured on the identical exact-catalogue corpus before adding it
to the 149.27-byte M2 result. Arithmetic across different corpora is not a
storage pass.

**FAIL if full similarity validation gates service readiness:** checked open is
136.246 ms, above 100 ms. The current safe disposition is
`available_experimental` only after validation, while exact service readiness
does not wait for it. Lazy authenticated blocks or asynchronous validation need
their own correctness decision.

**UNMEASURED:** idle CPU/writes, cold/HDD queries, update bytes, dead-posting
amplification, compaction, cancellation during publication, manifest atomicity,
disk full, repeated rebuild SSD cost, and ten-million scale.

## Verification matrix

- **PASS:** `go test ./...` on macOS;
- **PASS:** `go test -race ./...` on macOS;
- **PASS:** `go vet ./...`;
- **PASS:** Linux/amd64 and Windows/amd64 `go build ./...`;
- **PASS compatibility oracle:** all 13 test-bearing Windows/amd64 package
  binaries under Wine 11.10 on host APFS;
- **PASS compatibility oracle:** all 13 test-bearing Linux/arm64 package
  binaries copied to Lima guest `/var/tmp` and run from their mounted source
  directories on guest ext4.

Wine and Lima establish deterministic API/component behavior only. They do not
establish native NTFS/ext4 filesystem identity, installer behavior, durable
directory publication, or physical power-loss behavior.

## Gate disposition and next work

The sealed guidance passes the engine's experimental-transfer gate and remains
a **CANDIDATE** production mechanism. The next gate requires:

1. a consented/frozen real filename split and exact length/pool histograms;
2. character-gram, exact-deletion-key, and flat controls on identical judgments
   and serialized support;
3. production parameter lifecycle and adversarial polynomial-family audit;
4. live generation publication, cancellation, liveness rollover, dead-key and
   compaction write accounting under the M2 durability constitution;
5. service planner fallback for unsupported/over-capacity/budget states;
6. ORC-KOL descriptor/address/evidence reconciliation and independent wire
   conformance;
7. APFS, native NTFS, and native ext4 resource runs. Wine and Lima remain
   compatibility oracles only.
