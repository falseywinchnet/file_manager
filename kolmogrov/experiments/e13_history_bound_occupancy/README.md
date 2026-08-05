# E13: whole-history occupancy and coupled tuple capacity

Status: **MEASURED generated comparison**.

E13 asks why the E12 certificate tail persists without treating candidate order
as relevance. It separates the exact declared common-descendant relation from
complete certificates, bounded certificates, and projected certificates. It
then compares two rolling whole-history hashes:

- separable occupancy, which permits a different history witness per address;
- coupled tuples, which retain one complete multi-coordinate key per history.

All configurations preserve the declared radius-one relation by construction.
The capacity sweep measures only excess candidates, posting memberships, and
same-length mutation influence. Parent ngram, transposition, edit, and ranking
policy is outside the experiment.

Run from the repository root:

```text
PYTHONPATH=kolmogrov/src python3 kolmogrov/experiments/e13_history_bound_occupancy/run.py
```

The generator and the selected E12 certificate control are imported from E12.
No private filenames or network corpus are used.
