# E8 — quotient, retrieval, and specialization

Status: **MEASURED finite symbolic experiment with exact controls; no release
hash, native kernel, or general-file workload claim**.

Run from the Kolmogrov root:

```text
python3 experiments/e8_quotient_retrieval_specialization/run.py
```

The experiment keeps no raw or checksum artifact. It reports four intrinsic
measurements to standard output:

1. exact interaction-order candidate counts for binary sources of length ten;
2. position, run-count, outcome, and multiplicative quotient sizes;
3. inverted-index retrieval using exact Boolean pattern masks on frozen affine
   target-position certificates at declared support widths;
4. equivalence and reference timing for the radius-two rectangular streaming
   specialization, plus run-block quotient work counts.

Truth is exact radius-`t` deletion containment. Every candidate is resolved
against that truth; reported recall, precision, false candidates, and bucket
loads exclude hashing and quantization. Pattern-mask width is the ideal binary
payload `Q*2^m` bits and excludes index headers and posting representation.

The complete degree-`t+1` schedule is the theorem control. The affine schedule
is a bounded candidate selected by modular directions and coverage balance; it
does not inherit exactness from the complete schedule.
