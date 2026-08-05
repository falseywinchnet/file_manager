# Kolmogrov: the perceptual hash program

Status: **independent research scaffold; no production claim**.

Kolmogrov is the standalone successor program for ConeDAG, practical
algorithmic-information retrieval, and a fixed-width “hash function to rule them
all.” Its ambition is to give arbitrary finite objects several complementary
retrieval locations—content, position, combination, hierarchy, prevalence,
containment, scale, and context—then prove and measure how their joint geometry
preserves useful perceptual distance.

The program does not begin by squeezing data into a fashionable vector. It
begins by constructing the most complete observable breakdown we can define,
with every feature and relation inspectable. Compression and speed are later
theorem-bearing transformations of that oracle.

## Project phases

1. **Perfect breakdown:** exhaustive sparse features, exact transformations,
   channel-level distances, ambiguity classes, and counterexample generation.
2. **Formal geometry:** kernels, metrics, stability/separation bounds,
   prevalence laws, containment estimators, collision and quantization bounds.
3. **Fixed-width superposition:** independently addressable channel hashes at
   declared serialized bit budgets and multiple seeds/resolutions.
4. **Retrieval:** candidate indexes, exact verification, top-k quality,
   ambiguity preservation, and PRV/other fusion experiments.
5. **Optimization:** streaming construction, cache-aware layouts, SIMD/GPU only
   where measured, deterministic parallelism, and production-quality libraries.
6. **Closure:** paper-quality theorem chain, formal artifacts, reproducible
   corpus, negative-results atlas, and a versioned library release.

## Layout

- `HUMAN_NOTES.md` — the architect's thesis and source commentary.
- `CLAIM_REGISTER.md` — every consequential claim and its status.
- `PROOF_LEDGER.md` — theorem dependency and formalization state.
- `NEGATIVE_RESULTS.md` — failed constructions retained as knowledge.
- `PROVENANCE.md` — source locators and import boundary.
- `docs/` — research charter, mathematics, algorithms, experiments, handoff.
- `src/kolmogrov/` — audit-first reference contracts and future oracle.
- `tests/` — reference and invariant tests.
- `formal/` — proof-assistant boundary.
- `experiments/` — replayable experiment packages and manifests.
- `conformance/` — candidate cross-project reference vectors and lab profiles.
- `results/` — summaries, least-support obstructions, and rejected candidates.

## Bootstrap

The initial diagnostic contract checks use only the Python standard library:

```sh
python3 -m unittest discover -s tests -v
```

Their passage is not evidence for a mathematical claim. Claims advance through
proof objects, least-support obstructions, or a benchmark whose target is
intrinsically empirical.

No worker should begin with ANN selection, low-level optimization, or learned
weights. Begin with [RESEARCH_CHARTER.md](docs/RESEARCH_CHARTER.md) and the
perfect-breakdown oracle.

The first Phase A definition and finite checker are recorded in
[PHASE_A_ORACLE_V0.md](docs/PHASE_A_ORACLE_V0.md) and
[`experiments/e0_oracle_bootstrap`](experiments/e0_oracle_bootstrap).

The current original-construction round is summarized in
[GAP_SIMPLEX_PROGRAM.md](docs/GAP_SIMPLEX_PROGRAM.md), with paper proof objects
under [`formal/`](formal) and exhaustive E1–E6 checks under [`experiments/`](experiments).

The support/Jacobian/scale boundary is developed in
[SUPPORT_SCALE_JACOBIAN_PROGRAM.md](docs/SUPPORT_SCALE_JACOBIAN_PROGRAM.md),
including the projective contraction tower, minimum-support distinctions,
atomic SNR capacity law, and the quantized-scale obstruction.
The concrete support candidate is
[ALGORITHMIC_SUPPORT_BASIS.md](docs/ALGORITHMIC_SUPPORT_BASIS.md): a nested
semantic-cylinder route over canonical content, position, combination, scale,
and interaction evidence.
Unresolved theorem inputs are kept explicit in
[SUPPORT_ASSUMPTION_LEDGER.md](docs/SUPPORT_ASSUMPTION_LEDGER.md).

The current high-level synthesis is
[CYBERNETIC_SUPPORT_THEORY.md](docs/CYBERNETIC_SUPPORT_THEORY.md): exhaustive
occurrence transport, a multi-axis support lattice, exact support by flow
closure, and explicit breakdown residuals. The evidence-weighted judgment of
what is strongest and what remains speculative is
[THEORY_ASSESSMENT_001.md](docs/THEORY_ASSESSMENT_001.md).

The evidence method now prioritizes direct lower bounds and least-support
failures; see [MINIMAL_OBSTRUCTION_PROTOCOL.md](docs/MINIMAL_OBSTRUCTION_PROTOCOL.md).
The first five-step application is
[FALSIFIABLE_WORK_ROUND_002.md](docs/FALSIFIABLE_WORK_ROUND_002.md), covering
joint edit-flow closure, minimum interaction cycles, protected approximate
closure, progressive path order, and quantization certificates.

The next five least-support results close the previously named low-confidence
claims in [LOW_CONFIDENCE_ROUND_003.md](docs/LOW_CONFIDENCE_ROUND_003.md).
The scoped novelty boundary is in
[PRIOR_ART_BOUNDARY_001.md](docs/PRIOR_ART_BOUNDARY_001.md). Earlier candidate
contract implications are retained in
[ORC_KOL_RESEARCH_IMPLICATIONS_001.md](docs/ORC_KOL_RESEARCH_IMPLICATIONS_001.md);
the active reply and canonical-authority boundary are recorded in
[ORCHESTRATOR_INTERFACE_NEGOTIATION.md](docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md).

The active invention direction is now the
[deformation-jet superposition hash](docs/DEFORMATION_JET_SUPERPOSITION_HASH.md).
[INVENTION_ROUND_004.md](docs/INVENTION_ROUND_004.md) derives its exact
deletion-orbit, history-coupling, and bounded-radius history-moment objects. The
goal is explicitly a qualitatively better perceptual hash for search and
exact-anchored memory retrieval; the new construction remains a candidate until
it wins at equal serialized support.

Its active build algorithm and highlighted iteration structure are in
[DEFORMATION_ALGORITHM_REFINEMENT_001.md](docs/DEFORMATION_ALGORITHM_REFINEMENT_001.md).
E7 measures common-history coherence and the collapse from descendant and
occurrence enumeration to one streaming degree/radius ladder.

The next retrieval result is
[RETRIEVAL_REFINEMENT_002.md](docs/RETRIEVAL_REFINEMENT_002.md). It proves the
radius-`t` semantic degree floor `t+1`, separates safe run-count quotienting
from impossible full product-phase outcome quotienting, introduces indexable
affine observable certificates, and records E8 candidate-load and fixed-radius
specialization measurements.

The rich-alphabet refinement is
[RICH_CONTENT_PROJECTION_003.md](docs/RICH_CONTENT_PROJECTION_003.md). It
replaces exact `A^m` masks with coupled nested occupancy cells, proves a
private-witness support/breakdown law, rejects independent-view membership as
exact rescue, and records the E9 obstruction-selected projection curve.

The first filename instantiation is
[FILENAME_PROJECTION_REFINEMENT_004.md](docs/FILENAME_PROJECTION_REFINEMENT_004.md),
grounded by [Filename Observation Profile 001](docs/FILENAME_OBSERVATION_PROFILE_001.md).
It freezes exact source-anchored atom lanes, derives the source-symbol
certificate Jacobian, rejects low-bit atom projection, separates same-length
rewrites from cross-length edit spans, and begins the logical posting ledger.

The first cross-project conformance seam is the **CANDIDATE**
[ORC-KOL laboratory profile 001](docs/ORC_KOL_LAB_PROFILE_001.md), with a
[machine-readable fixture](conformance/orc_kol_001/lab_profile_001.json). It is
sufficient for an Orchestrator fake provider and an engine experimental adapter;
it does not advertise production similarity availability.

Start a dedicated worker with [HANDOFF_PROMPT.md](docs/HANDOFF_PROMPT.md).
