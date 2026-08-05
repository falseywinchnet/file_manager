# E5 support-boundary atlas result

Status: **MEASURED exact arithmetic within the declared parameter table; no
support scale or representation selected**.

Date: 2026-08-05.

## Replay and artifacts

```sh
cd /Users/quentinkuttenkuler/file_manager/kolmogrov
python3 experiments/e5_support_boundary_atlas/run.py
```

Raw SHA-256:
`7ef2fd497653e0a0473359512afd20e4c4ee41ca08298dc51cc4297566207907`.

## Exact checks

- Exhaustive integer blocks of factors two and three checked 150 instances of
  the `l_1` contraction, `l_infinity` contraction, and exact Hilbert
  coarse/detail energy identity.
- Exact cardinality tables distinguish fixed-length support from support for
  every length through `n`; for example, 128 binary bits name all exact
  length-128 binary strings but only all lengths through 127.
- Binary Hamming covering bounds were computed exactly for lengths 32, 64, and
  128 at radii 0, 1, 2, 4, and 8.
- Normalized one-bit influence was retained as an exact rational. At 64 bits its
  squared energy is `1/16`; at 2,048 bits it is `1/512`.
- The sign-only scale counterexample was reproduced: `(100,-99)` and `(1,-100)`
  have the same fine signs but opposite coarse sums.

## Interpretation boundary

The atlas checks theorem arithmetic and exposes scale. It does not select a
noise energy, SNR gate, alphabet, distortion radius, coordinate precision,
scale base, or content limit. The covering bound applies only to worst-case
decoding within the declared Hamming radius; it is not evidence for perceptual
retrieval quality.
