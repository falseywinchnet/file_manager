# Advancement experiments

An intrinsically empirical benchmark occupies its own directory with:

- `MANIFEST.json` — claim, revision, corpus, split, seed, and command;
- source and configuration;
- correctness oracle;
- `RESULT.md` — evidence label, measurements, anomalies, inference boundary;
- counterexamples minimized into permanent fixtures;
- negative-result entry when rejected.

Paper-theorem searches do not receive benchmark packaging merely because code
ran. Follow `docs/MINIMAL_OBSTRUCTION_PROTOCOL.md`: retain the least witness and
its lower-bound proof, not pass counts, environment text, or checksum churn.

For an empirical claim, do not overwrite a prior run. A changed mechanism,
corpus, split, seed policy, or metric creates a new run identity.

## Current Phase A runs

- `e0_oracle_bootstrap` — canonical literal reconstruction and first minimized
  content collision.
- `e1_gap_deletion_laws` — exact one-deletion gap-simplex laws.
- `e2_residue_addresses` — modular address collision census and coupling loss.
- `e3_edit_factorization` — deletion sets, substitution, and transposition.
- `e4_orthant_containment` — cross-length directional matching and low-degree
  false-parent counterexamples.
- `e5_support_boundary_atlas` — exact support, contraction, influence, and
  quantized-scale boundary arithmetic.
- `e6_cybernetic_support` — exact minimum-support partition census under
  position-resolved versus averaged binary edit controls.
- `e7_deformation_coherence` — common-history coherence, selected-character
  interference, and combinatorial-to-streaming algorithm collapse.
- `e8_quotient_retrieval_specialization` — exact interaction order, safe and
  unsafe history quotients, affine certificate retrieval, and fixed-support
  specialization.
- `e9_rich_projection` — coupled rich-alphabet projection, nested refinement,
  private-witness breakdown, and cross-view provenance obstructions.
- `e10_filename_jacobian` — source-anchored Unicode filename views, exact
  mutation flow, low-bit projection failure, and logical posting charge.
- `e11_filename_retrieval_cost` — protected-kernel row compilation, disjoint
  generated retrieval, exact `n-1/n/n+1` plans, concrete posting payloads, and
  matrix-free radius-one verification.
- `e12_guard_coverage_specialization` — three-way guard/coverage composition,
  certificate-count specialization, genuine policy negatives, provenance
  residual, and immutable-segment liveness.
- `e13_history_bound_occupancy` — exact candidate attribution, rolling
  whole-descendant fingerprints, separable saturation, coupled history tuples,
  nested capacity, and mutation contribution energy.

`make research` remains a diagnostic replay of the historical Phase A sequence.
Its successful execution does not promote any claim.
