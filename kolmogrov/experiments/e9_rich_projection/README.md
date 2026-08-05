# E9: rich-alphabet projection and private-witness breakdown

Status: **finite CANDIDATE comparison; not a File Manager corpus result**.

E9 replaces the binary exact mask with nested whole-pattern projections on an
exhaustive four-symbol sequence domain. It asks three narrow questions:

1. At equal dense support, do slot-balanced affine views reduce candidate load
   relative to one path-biased radix prefix?
2. How much observed pattern mass has no private bit witness?
3. What are the least collisions and cross-view provenance-mixing witnesses?

The complete degree-three certificate schedule is the exact radius-two control.
Every projected layout is candidate-only and is required to retain that control.
The run does not select Unicode normalization, tokenization, production weights,
or an engine index representation.

Run from the Kolmogrov root:

```sh
python3 experiments/e9_rich_projection/run.py
```
