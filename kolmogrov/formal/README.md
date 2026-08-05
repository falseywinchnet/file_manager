# Formalization boundary

Freeze mathematical definitions in `docs/FORMAL_PROGRAM.md` and paper proofs in
the proof ledger before selecting a proof assistant encoding. The initial likely
targets are finite sequences, sparse feature maps, channel kernels, deletion
survival, multi-address collision models, and quantization bounds.

Every formal file must name the theorem ID it closes. Generated proof artifacts
and toolchain locks belong here; exploratory algebra belongs under a named
experiment until its definitions stabilize.

## Current paper objects

- `T-ORACLE-1.md` — literal reconstruction.
- `T-GAP-1.md` — gap-simplex occurrence bijection.
- `T-DELETE-1.md` — exact one-deletion law.
- `T-MULTIDEL-1.md` — exact deletion-set law.
- `T-EDIT-1.md` — substitution and adjacent-transposition factorization.
- `T-RESIDUE-1.md` — modular satellites and the marginal-coupling failure.
- `T-ORTHANT-1.md` — directional deletion-lift containment.
- `T-SCALE-1.md` — Banach contraction and Hilbert detail energy.
- `T-SUPPORT-1.md` — exact, fuzzy, and affine support bounds.
- `T-INFLUENCE-1.md` — atomic contribution-energy/SNR capacity.
- `T-QUANT-SCALE-1.md` — sign-only scale obstruction and margin replacement.
- `T-JACOBIAN-SCALE-1.md` — affine response and residual flow across scale.
- `T-ENTROPY-SCALE-1.md` — relevant-information loss across scale and addresses.
- `T-PROGRESSIVE-SUPPORT-1.md` — exact coarse-plus-detail dimension and error.
- `T-TENSOR-SUPPORT-1.md` — multi-axis detail and interaction decomposition.
- `T-CYBERNETIC-SUPPORT-1.md` — minimum observation-preserving closed support.
- `T-ACTION-SUPPORT-1.md` — support forced by resolved versus averaged edits.
- `T-JOINT-FLOW-1.md` — least failure of low-degree joint edit-flow closure.
- `T-INTERACTION-MIN-1.md` — least marginal-invisible interaction cycle.
- `T-APPROX-CLOSURE-1.md` — weighted residual and rare-state obstruction.
- `T-PATH-ORDER-1.md` — least progressive path-order obstruction.
- `T-QUANT-CERT-1.md` — sharp interval certificate after quantization.
- `T-UNIVERSAL-SUPPORT-1.md` — minimum shared multi-task observation support.
- `T-OBSTRUCTION-CODE-1.md` — protected graph coloring and exact bit width.
- `T-ENERGY-INFORMATION-1.md` — least energy/relevance separation.
- `T-NESTED-SCHEDULE-1.md` — exact criterion for a zero-overhead prefix order.
- `T-INTERACTION-SPARSITY-1.md` — full-dimensional universal interaction bound.
- `T-DELETION-ORBIT-1.md` — exact differential deletion orbit and jet shift.
- `T-HISTORY-COUPLING-1.md` — deformation-history lift and least marginal failure.
- `T-HISTORY-MOMENTS-1.md` — bounded-radius multiplicative history codes.
- `T-HISTORY-COHERENCE-1.md` — exact common-history character correlation.
- `T-SAMPLED-HISTORY-1.md` — selected-phase signal/interference certificate.
- `T-STREAMING-JET-1.md` — linear degree/radius streaming recurrence.
- `T-PAIRWISE-HISTORY-1.md` — pair closure at radius one and least radius-two failure.
- `T-DELETION-CLOSURE-1.md` — exact radius-`t` interaction order and support bound.
- `T-HISTORY-QUOTIENT-1.md` — safe run quotient and full-outcome phase obstruction.
- `T-PROJECTION-REFINEMENT-1.md` — nested rich-pattern projection, private
  witnesses, and provenance-mixing obstructions.
- `T-SYMBOL-JACOBIAN-1.md` — exact source-symbol flow into certificate cells.
- `T-TYPED-EDIT-SEAM-1.md` — same-length secants and cross-length edit spans.
- `T-POSTING-SUPPORT-1.md` — logical posting membership and information charge.
- `T-SYMMETRIC-CERTIFICATE-1.md` — same-length common-descendant front index.

All are **HYPOTHESIS** pending independent review. “Complete paper proof” in
`PROOF_LEDGER.md` means the definitions, quantifiers, proof, exclusions, and
finite support are reviewable; it does not mean formal verification or owner
acceptance.
