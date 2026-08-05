# Research charter: beyond one-location hashing

Status: **grand research mission**.

## Governing objective

Construct, analyze, and optimize a fixed-width perceptual-hash family whose
multi-address geometry provides the most reliable practical similarity measure
we can deliver across declared dimensions and transformations.

The project seeks work of theorem-and-system significance: new definitions when
old ones collapse distinctions, exact counterexamples when a desired theorem is
too broad, tight restricted theorems rather than loose slogans, and an optimized
library that remains traceable to the mathematical reference.

## Central conjecture

### Practical Superposition Conjecture

For a declared object class, transformation family, and retrieval loss, a
bounded collection of complementary fixed-width projections—kept independently
addressable and fused only after measurement—can preserve task-relevant
neighborhoods far more faithfully than an equal-bit single-location hash, while
exact external anchors prevent collision from becoming false identity.

The program must determine:

- conditions under which the conjecture is true;
- the minimum channel/bit structure required;
- tight distortion and rescue bounds;
- counterexample families where it fails;
- whether a universal construction emerges across modalities or a principled
  family with shared laws is the strongest attainable object.

## What “all dimensions” demands

Do not use “all” poetically. Build a dimension inventory and either encode,
exclude, or defer every axis:

- symbol/content identity and multiplicity;
- local adjacency and longer ordered paths;
- absolute, relative, multiresolution, and hierarchical position;
- cross-features between content, position, and context;
- length, scale, density, and shape;
- containment, crop, prefix/suffix, and subsequence direction;
- repetition, periodicity, symmetry, and motif structure;
- path/tree/graph ancestry and typed relations;
- prevalence by corpus, root, channel, resolution, and time;
- Unicode/script, typography/OCR, tokenization, and raw-byte evidence;
- modality-specific spatial, spectral, temporal, or structural axes;
- explicit context and conditional similarity.

The perfect-breakdown phase is allowed to grow with the object. Fixed-width
compression is judged against it later.

## Research questions

1. What is the canonical exhaustive feature object for each modality?
2. Which channel distances are metrics, kernels, divergences, or directional
   containments?
3. Which transformations should be smooth, invariant, equivariant, or sharply
   separated?
4. Which changes are perceptually nonuniform despite equal edit cost?
5. How can several addresses rescue collisions without assuming false
   independence?
6. How should prevalence alter geometry without making hashes unstable across
   corpus generations?
7. Can fixed-width quantization retain channel guarantees at useful bit budgets?
8. What index structures retrieve multi-address neighbors without destroying
   determinism, updates, or exact verification?
9. Can one family span text, filenames, code, paths, images, audio, and structured
   objects, or must shared laws instantiate modality-specific channels?
10. What theorem chain is strong enough that the optimized implementation can be
    trusted rather than merely benchmarked?

## Advancement modes

### Exploration

Generate constructions, analogies, counterexamples, and experiments freely.
Record everything under a named round. Do not promote claims during exploration.

### Refinement

Reduce the round into definitions, claims, proofs, negative results, fixtures,
and next gates. Update the ledgers. Do not explore unrelated branches while
performing integration.

### Formal closure

Freeze definitions, audit quantifiers, search finite counterexamples, write the
paper proof, reconstruct dependencies, and formalize the critical chain.

### Optimization

Only after the reference hash and theorem identity freeze: profile, change one
mechanism, differential-test, benchmark, and retain failures.

## Success levels

- **Level I:** best-in-class measured retrieval at equal bit budgets.
- **Level II:** new restricted stability, separation, prevalence, or collision
  theorems explaining the win.
- **Level III:** a unified multi-address theory and independently reproducible
  reference corpus.
- **Level IV:** formalized critical results and a production-quality fixed-width
  library matching the theory.
- **Level V:** a construction or theorem that materially changes how perceptual
  similarity hashing is understood across fields.

The program aims at Level V. Lower levels remain publishable and useful rather
than being discarded if the largest conjecture fractures.
