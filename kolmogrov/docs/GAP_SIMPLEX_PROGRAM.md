# Gap-simplex multi-address research round

Status: **CANDIDATE construction family; formal and finite support recorded,
novelty unverified**.

## Motivation from the architect's guidance

The construction does not begin with a learned vector, ANN index, edit-distance
surrogate, or one fused hash. It asks what survives a deformation and keeps the
answer factored:

- content is the selected symbol pattern;
- combination is the ordered selection itself;
- position is the unselected mass before, between, and after selections;
- scale is object length and selection degree;
- containment is the directional inclusion of occurrence multiplicity;
- exact identity remains an external anchor.

For a degree-`k` occurrence, the `k+1` gap coordinates inhabit a discrete
simplex. This makes deletion unusually transparent: destroyed witnesses are
counted separately from surviving witnesses, and each survivor moves one local
lattice step rather than undergoing a global positional rewrite.

## Formal support obtained

- `T-GAP-1`: exact bijection between occurrence indices and gap-simplex
  coordinates.
- `T-DELETE-1`: exact survival, destruction, pattern-containment, and `k/n`
  source-damage laws for one deletion.
- `T-RESIDUE-1`: modular gap addresses retain one-step cyclic motion; bounded
  injectivity holds for a sufficiently large joint modulus.
- `T-ORTHANT-1`: cross-length deletion compatibility is coordinatewise gap
  dominance; full-query degree is exact, while low degrees require explicit
  global-consistency caution.
- `T-ORACLE-1`: the explicit literal witness reconstructs the exact sequence.

All remain **HYPOTHESIS** pending independent proof review and formal encoding.
Executable finite checks support implementation agreement but do not replace
the paper arguments.

## Candidate address forms tried

### Exact gap witness

Retain `(pattern,g_0,...,g_k)` at every occurrence. This is canonical and
explanatory but grows combinatorially. It is the oracle control, not the release
representation.

### Single modular satellite

Retain each gap coordinate modulo `m`. A survivor changes one cyclic step in one
coordinate. Small moduli deliberately identify distant positions and create
collisions; larger moduli cost more but preserve a wider exact positional
window.

### Independent coprime satellites

Keep mod-2, mod-3, and possibly mod-5 occurrence multisets independently. This
offers complementary collision patterns: E2 shows the first mod-2/mod-3 joint
marginal collision is rescued by mod-5, while the first mod-5 collision is
rescued by mod-2 and mod-3.

However, separate marginals are not equivalent to a joint composite modulus.
They forget cross-modulus occurrence coupling. This is a retained failure, not
an inconvenience to hide.

### Coupling certificate

A joint composite-modulus address retains per-occurrence residue coupling. At
the same nominal residue alphabet as mod-2 plus mod-3, joint mod-6 distinguishes
the length-11 pair their separate marginals collapse. The candidate research
direction is therefore a sparse coupling satellite alongside independent
primary addresses—not complete fusion of all channels.

## What was deliberately not imported

This round did not choose embeddings, locality-sensitive hashing, MinHash,
SimHash, product quantization, edit-distance sketches, or an ANN structure.
Known combinatorial identities and modular arithmetic are used transparently,
but this exact gap-simplex/residue organization was independently derived from
the supplied thesis. No claim of literature novelty is made until a scoped
prior-art audit is authorized, pinned, and recorded in `PROVENANCE.md`.

## Next attacks

1. Generalize the exact one-deletion gap law to `t` deletions and mixed edits.
2. Determine whether sparse coupling certificates can guarantee rescue under a
   declared dependence model at equal serialized budgets.
3. Search for smallest collisions by alphabet size, degree, modulus, and scale;
   classify whether failure comes from pattern aggregation, positional residue,
   or lost cross-address coupling.
4. Define a cross-length gap transport score that distinguishes deletion
   survivors from coincidental residue matches. E4 establishes the exact
   orthant control; graded transport and compact approximation remain open.
5. Add hierarchy and prevalence only after their evidence objects remain as
   inspectable as the current witnesses.
