# E12: guard/coverage composition and specialization

Status: **three-way generated research experiment; no production selection**.

E12 freezes row guards on development plus tuning mutation graphs, selects an
equal-dense-support layout and certificate count on tuning, and evaluates once
on a third disjoint generator. It addresses:

- protected guard versus ambient coverage roles;
- separate-address provenance mixing;
- certificate-count/support specialization;
- genuine one-edit digit and extension policy negatives;
- immutable-segment liveness and dead posting mass.

The compared layouts are:

```text
coverage-3                    one 8-cell balanced address
coverage-4                    one 16-cell balanced address
union-guard-4                 one 16-cell development+tuning guard address
split-guard3-coverage3        two separate 8-cell addresses (16 dense cells)
coupled-guard3-coverage3      one coupled 64-cell address
```

All three splits contain 288 generated records and 216 queries. The exact query
set includes equality, substitution, adjacent transposition, deletion,
insertion, and case changes. Number and extension families include genuine
one-edit siblings. No private or network corpus is read.

Replay:

```sh
PYTHONPATH=src python3 experiments/e12_guard_coverage_specialization/run.py
```

The experiment uses direct candidate-set evaluation to isolate representation
semantics, plus exact posting payload construction for each stored layout. It
does not claim native query latency or cache behavior.
