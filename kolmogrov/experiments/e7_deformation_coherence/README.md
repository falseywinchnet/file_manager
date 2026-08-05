# E7 deformation coherence and algorithm collapse

Status: **empirical refinement of a CANDIDATE hash; no production selection**.

Run from the Kolmogrov root:

```sh
python3 experiments/e7_deformation_coherence/run.py
```

The experiment measures:

- exact common-history loss versus unlabelled and pairwise marginals;
- selected moment-character coherence error, sidelobes, and separation margin;
- exact equality of descendant enumeration, occurrence-product collapse, and
  streaming dynamic programming;
- named loop-body counts and construction time for all three forms.

No raw-output or checksum artifact is retained. `RESULT.md` records the measured
summary, least explanatory failures, environment, and inference boundary.
