# E0 symbolic-oracle bootstrap result

Status: **MEASURED** within the declared finite corpus only.

Date: 2026-08-05.

## Replay

```sh
cd /Users/quentinkuttenkuler/file_manager/kolmogrov
python3 -m unittest discover -s tests -v
python3 experiments/e0_oracle_bootstrap/run.py
```

Environment recorded by the run: Darwin arm64, Python 3.14.0. The stored raw
output is `RAW_OUTPUT.json`, SHA-256
`d4432cf8fc8630587b713388d3d92b801983996e3f91ada7939319e76408cbda`.
Its canonical-breakdown stream digest is
`964074df2605f1ff980b719655378f4a49b541d78afcbe3a7093c043a0a531a3`.

## Observations

- All 63 sequences over `(a, b)` through length five reconstructed exactly from
  the literal evidence.
- Repeated encoding produced equal `Breakdown` values for every object.
- Every one of the 2,325 aggregated features retained at least one source
  anchor.
- The four-operation synthetic lineage retained substitution, insertion,
  adjacent transposition, and deletion in order.
- The minimal content-multiset collision was `(a, b)` versus `(b, a)`.
- The complete unit suite passed 10 tests.

## Inference boundary

This run does not prove T-ORACLE-1 for arbitrary length or alphabet. It does not
measure retrieval quality, channel necessity, latency, memory, or fixed-width
behavior. Resource distributions are intentionally **UNMEASURED** in this
correctness bootstrap; the exponential reference oracle is not a performance
candidate.

No new negative arose beyond K-N001. The next falsifiable objective is to define
channel-level exact distances and exhaustively enumerate their zero-distance
equivalence classes under elementary transformations.
