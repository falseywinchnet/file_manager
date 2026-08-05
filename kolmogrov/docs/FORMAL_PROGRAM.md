# Formal mathematical program

Status: **theorem agenda; definitions to be frozen by the first advancement**.

## 1. Objects and exact anchors

For an initial symbolic domain, let an object be a finite sequence

\[
x=(x_0,\ldots,x_{n-1})\in\Sigma^*.
\]

Let `E(x)` be its exact external record and identity. `E` is not compressed into
the perceptual hash. The hash family supplies candidate neighborhoods over exact
anchors.

Later domains instantiate a typed object algebra: trees, graphs, images, audio,
and compound file objects. Each defines its own exact observation object before
sharing channel laws.

## 2. Perfect-breakdown oracle

Define an exhaustive sparse observable object

\[
\mathcal B(x)=
\big(C(x),P(x),X(x),H(x),R(x),S(x),M(x)\big),
\]

where candidate families are:

- `C`: content, multiplicity, symbols, n-grams, motifs;
- `P`: ordered local/global paths and subsequences;
- `X`: absolute/relative/multiresolution position and content-position crosses;
- `H`: hierarchy, ancestry, graph, or typed structural relations;
- `R`: repetition, periodicity, symmetry, and relational combinations;
- `S`: scale, shape, length, density, and bounded statistics;
- `M`: modality-specific observables.

The oracle may be large. Its purpose is to expose exactly what each later stage
discards. T-ORACLE-1 should establish a declared completeness property, such as
reconstruction within a finite object class or injectivity after inclusion of an
explicit literal channel.

## 3. Channel geometry

For each channel `j`, define feature space `V_j`, normalization `N_j`, and
distance/kernel/divergence `d_j` or `k_j`. Do not assume every useful relation is
symmetric:

- cosine/kernel geometry for symmetric neighborhoods;
- weighted set/multiset distances;
- directional containment for partial queries;
- edit/transformation cost controls;
- typed graph alignment;
- scale-normalized and magnitude-aware variants.

Prove which channels are positive semidefinite kernels, metrics, pseudometrics,
or merely ordered scores. State zero-distance equivalence classes explicitly.

## 4. Prevalence operator

Let a corpus generation be `D` and feature prevalence be a typed measure
`p_D(f,j,scope,resolution)`. Define a bounded weight operator `W_D` with smoothing
and clipping. Desired theorems include:

- bounded weight change under one-record corpus updates;
- convergence or controlled drift under stationary sampling;
- resistance to adversarial feature flooding;
- invariance of exact/literal channels;
- rank stability under bounded corpus perturbation;
- relationship between prevalence-weighted geometry and description cost.

Corpus generation and weights are part of hash identity. A hash from one
prevalence generation is not silently compared as though it came from another.

## 5. Multi-address superposition

Do not immediately concatenate every channel into one normalized point. Define
an address family

\[
\mathcal A_{W,S}(x)=
\left(h_{1,W_1,s_1}(x),\ldots,h_{q,W_q,s_q}(x)\right),
\qquad \sum_j W_j=W,
\]

where channels, resolutions, and seeds remain independently retrievable. Fusion
acts on evidence returned by these addresses.

T-SUPER-1 should compare a multi-address family against an equal-total-bit fused
projection. Desired results:

- probability that all useful channels collide;
- rescue probability conditional on dependence among channels;
- candidate recall versus total bit budget;
- minimum number of addresses for a declared transformation class;
- robustness when one channel is erased, corrupted, or adversarially flooded.

## 6. Stability and separation

For each transformation `T` in a declared family, seek upper stability

\[
d(\mathcal A(x),\mathcal A(Tx))\le L_T\,\rho_T(x,Tx)+b_T
\]

and restricted lower separation

\[
d(\mathcal A(x),\mathcal A(y))\ge
\ell_{\mathcal C}\,\rho_{\mathcal C}(x,y)-b_{\mathcal C}
\]

for a class `C` where such a bound is true. Search for sharp constants and
smallest counterexamples. Distinguish:

- substitution, insertion, deletion, transposition;
- block move, reversal, reorder, duplication;
- crop, containment, prefix/suffix, subsequence;
- normalization, OCR, Unicode/script variants;
- semantic/contextual transformations only after their evidence exists.

The right theorem may be piecewise, directional, or scale-dependent rather than
one global bi-Lipschitz claim.

## 7. Hash projection and collision law

For sparse channel vector `v`, define deterministic signed projection,
quantization, and serialization separately. Ideal-random theorems and fixed-hash
engineering behavior receive different claim IDs.

Study:

- unbiased inner products and concentration under declared hash families;
- limited-independence requirements;
- adversarial versus random feature keys;
- seed ensembles and deterministic seed selection;
- collision covariance across content/position/combination channels;
- saturation, cancellation, and heavy-feature effects;
- quantization error at equal serialized bit budgets;
- hash-generation compatibility and migration.

## 8. Containment and ambiguity

Define directional containment over exhaustive features and sampled estimators.
Prove estimator bias/concentration under the actual sampling protocol. Preserve
ambiguity classes when several exact parents minimize the declared deformation.

A retrieval theorem should guarantee a candidate set or equivalence class, not
invent a unique source where the observation cannot identify one.

## 9. Practical algorithmic-information track

Define corpus- and transformation-relative description quantities rather than
leaving “complexity” metaphorical:

\[
K_{\text{literal}}(x),\quad
K_{\text{prevalence}}(x\mid D),\quad
K_{\text{transform}}(x\mid y,\mathcal T),\quad
K_{\text{context}}(x\mid q,D).
\]

Determine when multi-address distance estimates or bounds these quantities and
whether nearest anchors yield a practical minimum-description retrieval law.
This is the exact battlefield named by the architect: defeat the engineering
conclusion that one fixed location is required to make algorithmic-information
ideas useful for comprehensive retrieval.

## 10. Formalization sequence

1. Finite symbolic oracle definitions and injectivity/completeness.
2. Existing ConeDAG channel theorems independently reconstructed.
3. Multi-address product construction and equal-bit comparison theorem.
4. Transformation stability and restricted separation.
5. Prevalence perturbation bounds.
6. Projection/collision/quantization chain.
7. Containment and ambiguity.
8. End-to-end candidate-recall theorem under explicit assumptions.

The support-scale subprogram adds an orthogonal dependency chain before compact
projection is selected:

1. distinguish ambient, active, affine, code, influence, and scale support;
2. define the projective Banach/Hilbert tower and exact detail energy;
3. bound exact/fuzzy cardinality and locally preserved affine rank;
4. bind atomic influence to independently declared contribution-energy and SNR
   budgets;
5. state what quantization destroys and what margin support restores;
6. instantiate the abstract `Phi_m` from canonical gap-simplex evidence and
   measure projective residuals before choosing widths.

Each theorem begins on paper, is attacked by least-support obstruction search,
and moves to `formal/` when definitions stabilize. Search output is discarded
once a direct witness and minimality argument replace it.
