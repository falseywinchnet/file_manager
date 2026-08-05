# Experiment and adversarial-evaluation program

Status: **mandatory empirical companion to the theorem program**.

## Corpora

### Symbolic foundations

- exhaustive short binary/small-alphabet strings;
- generated Unicode sequences with NFC/NFD, scripts, combining marks, emoji;
- filenames and paths across code, documents, media, dates, identifiers;
- source code with rename/refactor/format/reorder transformations;
- OCR-style corruption and keyboard-layout errors;
- prose, lists, structured records, and mixed punctuation.

### Structural objects

- rooted trees and paths;
- labeled DAGs and typed graphs;
- nested document structures;
- repeated motifs, periodic objects, and symmetry controls.

### Modalities after symbolic closure

- images with crop, scale, rotation, color, compression, occlusion, and semantic
  hard negatives;
- audio with shift, crop, resample, noise, pitch/time changes;
- other objects only after an exact observation and transformation model exists.

No private corpus enters by accident. Every corpus has consent/license, source
version, schema, split, generator, and seed. These identify an empirical input;
they do not substitute for an explanatory obstruction.

## Transformation atlas

Generate controlled families with exact lineage:

- one/multiple substitution, insertion, deletion, transposition;
- block move, reversal, reorder, duplication, repetition;
- prefix/suffix removal, crop, subsequence, interpolation;
- case/normalization/script/OCR/phonetic variants;
- path moves and ancestor changes;
- scale and resolution changes;
- combinations at increasing severity;
- perceptually important small changes as hard negatives;
- superficially large but meaning-preserving changes as positive cases where the
  domain supplies justified labels.

Record both source identity and every equally valid ambiguity parent.

## Experiment ladder

### E0 — oracle correctness

Exhaustively enumerate small objects. Verify canonical features, reconstruction
claim if made, transformation lineage, and exact channel distances. Minimize all
counterexamples.

### E1 — channel necessity and sufficiency

Content-only, position-only, combination-only, every pair, full family, and
leave-one-out. Measure which transformations and hard negatives each resolves.

### E2 — prevalence

Static, growing, shifted, and adversarial corpora. Measure quality, hash/rank
drift, rare-feature rescue, common-feature suppression, privacy scope, and flood
resistance.

### E3 — fixed-width equal-bit comparison

Compare fused and multi-address layouts at identical serialized bits. Run at
least ten seeds and report seed intervals, worst seed, collision/fanout, and
per-class Recall@k.

### E4 — geometry

Estimate distortion against declared transformation distances; search for
violations of proposed stability/separation; compare symmetric and directional
relations; retain zero-distance equivalence classes.

### E5 — quantization

Full precision through binary signatures. Measure pair-order reversals, top-k
loss, saturation, cancellation, and theorem-bound tightness.

### E6 — index transfer

Flat scan versus candidate indexes from 10k through at least 1m anchors. Separate
hash error from index miss. Record update/delete behavior and bytes/object.

### E7 — rank fusion

Independent channels as truncated rankings. Compare deterministic tiers,
weighted sums, reciprocal-rank fusion, calibrated fusion, and bounded PRV-derived
completion. Include malicious/missing/noisy channels and inferred-evidence caps.

### E8 — optimized equivalence

Differential-test scalar reference, streaming, parallel, vectorized, and any GPU
path over conformance and random corpora. Benchmark construction and retrieval
distributions under controlled hardware.

### E9 — cross-domain law

Test whether one configuration, one family with modality adapters, or separate
families best preserve the shared theorem structure. Reject false universality
and retain the strongest valid common law.

## Metrics

- Recall@1/5/10/20, MRR, nDCG, rank correlation;
- ambiguity-class recall and strict-source recall separately;
- false candidates by transformation and hard-negative class;
- collision, multi-address rescue, bucket/fanout distributions;
- distance distortion, Lipschitz-ratio distributions, theorem violations;
- prevalence drift, rank churn, adversarial influence;
- build/query p50/p95/p99/max, throughput, CPU, memory, bytes/object;
- seed, corpus, split, and implementation variance;
- explanation completeness and exact-anchor verification.

## Promotion gates

A compact candidate cannot replace the oracle unless:

- every claimed invariant has a proof or explicit measured status;
- quality improves or matches the best equal-bit control on held-out data;
- no exact identity behavior depends on the hash;
- hard-negative regressions are bounded and named;
- seed instability is acceptable under a declared bound;
- the representation and configuration are fully reproducible;
- optimized and reference paths pass differential conformance.
