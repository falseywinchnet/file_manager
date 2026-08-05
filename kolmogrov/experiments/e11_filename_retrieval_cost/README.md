# E11: protected rows, exact-length retrieval, and posting charge

Status: **synthetic development/evaluation experiment; not a production corpus
or native engine benchmark**.

E11 freezes binary rows on one generated filename/mutation family, evaluates a
disjoint family, and charges the complete radius-one query plan:

```text
stored length = query length + 1   directional mask/key lookup
stored length = query length       occupied-cell certificate overlap
stored length = query length - 1   query-mask/stored-key reverse lookup
```

Each plan uses eight affine degree-two certificates per literal, anchored-fold,
and structural view. Candidate records are classified by the matrix-free exact
one-edit verifier; exact engine identity remains external.

The development and evaluation generators use disjoint stems, separators,
numbers, extensions, substitutions, and transposition edges. Both are synthetic
and deterministic. No home-directory filenames, network corpus, or private
records are read.

The posting controls are exact sorted record-ID lists encoded as:

- unsigned delta varints;
- a dense `N`-bit bitmap per nonempty list;
- a one-byte-tag hybrid choosing the smaller payload per list.

Address dictionaries, block framing, checksums, allocator overhead, cache
misses, filesystem writes, and compaction are deliberately excluded and
reported as open engine costs.

Replay:

```sh
PYTHONPATH=src python3 experiments/e11_filename_retrieval_cost/run.py
```

The printed JSON is the complete replay output. Checks are diagnostic; the
retained evidence is the declared workload, direct support obstruction, and
measured distributions summarized in `RESULT.md`.
