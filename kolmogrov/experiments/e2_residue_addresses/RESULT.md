# E2 modular residue-address result

Status: **MEASURED** on the declared finite domain; every construction remains
a **CANDIDATE**.

Date: 2026-08-05.

## Replay and artifacts

```sh
cd /Users/quentinkuttenkuler/file_manager/kolmogrov
python3 experiments/e2_residue_addresses/run.py
```

Raw SHA-256:
`f89a793d5461ea93b801e67018b7752c3105e95b0fa35d29940280c461559bfe`.

The experiment differentially checked its direct degree-2 residue spelling
against `projected_gap_signature` for every binary object through length 12.

## Collision census through length 12

| Address | Collision pairs | Largest class | First collision length |
|---|---:|---:|---:|
| content multiplicity | 1,826,175 | 924 | 2 |
| degree-2 order | 83,322 | 58 | 4 |
| gap residues mod 2 | 3,966 | 7 | 7 |
| gap residues mod 3 | 324 | 2 | 10 |
| independent mod 2 + mod 3 | 24 | 2 | 11 |
| gap residues mod 5 | 0 | 1 | none in bound |
| joint gap residues mod 6 | 0 | 1 | none in bound |
| independent mod 2 + mod 3 + mod 5 | 0 | 1 | none in bound |

Zero in this table means no collision in the finite search, not universal
injectivity.

## Complementarity and coupling

The first independent mod-2/mod-3 marginal collision is:

```text
aaabbabbaaa
bbaaaaaaabb
```

Both mod-5 and joint mod-6 distinguish it. The first mod-5 collision, found by
the targeted exhaustive search through length 16, is:

```text
aaaaabaaaabaaaaa
baaaaaaaaaaaaaab
```

Both mod-2 and mod-3 distinguish it.

The mod-2/mod-3 failure is structurally important: independent aggregated
marginals discard which residues co-occurred on one witness. Joint mod-6 keeps
that coupling. Coprimality alone therefore does not authorize a CRT
reconstruction argument.

## Boundary

The addresses are exact uncompressed occurrence maps, not equal-bit hashes.
No retrieval labels, latency, memory, seed variance, cross-length ranking, or
adversarial corpus were measured. The finite results motivate complementary
satellites and sparse coupling certificates; they select neither.
