# Kolmogrov fixed-width channel seam

Status: **CANDIDATE interface; no hash or index selected**.

The engine reserves `internal/similarity` for the fixed-width perceptual-hash
family being researched in `../kolmogrov/`. The package is deliberately only a
boundary. It contains no placeholder hash, approximate-nearest-neighbor index,
distance function, or fusion weight.

## GIVEN constraints

- Every sketch is disposable and versioned by family, revision, byte width,
  and parameter digest.
- Every sketch and returned candidate anchors to an exact catalogue object,
  observed path, root, and committed generation.
- Candidate count and elapsed work are caller-bounded.
- Channel scores and named contributions remain separate evidence. They do not
  become identity and do not outrank exact facts merely by being available.
- A configuration change permits independent erase/rebuild of this channel.

## Transfer gate

Kolmogrov may enter an experiment build only after it supplies a pinned
configuration, deterministic cross-platform vectors, collision/ambiguity
behavior, encoding cost, candidate-retrieval mechanism, resource bounds, and an
exact-catalogue verification path. It must then beat fixed-width and lexical
controls on the same judged File Manager workload. Until that evidence exists,
the service advertises no similarity feature and exact/lexical delivery does
not wait for it.
