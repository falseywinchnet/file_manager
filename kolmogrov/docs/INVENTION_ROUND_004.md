# Invention round 004: from terminal hashes to deformation jets

Status: **HYPOTHESIS-bearing original construction round; no architecture or
production selection**.

Date: 2026-08-05.

## Correction of direction

The goal is a better perceptual hash. Prior obstruction work remains useful,
but it cannot become the generative agenda. This round asked which concealed
state a conventional terminal hash discards and whether exposing that state
creates a qualitatively different fixed-width candidate.

No external source or literature catalogue was consulted in this round.

## Instinct followed

Gap-simplex occurrences already make a surviving deletion local. Package their
gap coordinates as monomials and ask how the entire deletion neighborhood acts
on the resulting polynomial.

That yielded two primary exact objects and a concrete compression chain:

1. the unlabelled deletion orbit is a differential jet;
2. the missing coupling is the common deletion history shared by all evidence
   in one descendant;
3. bounded-radius histories admit structural moment characters;
4. full character coherence counts common histories exactly;
5. sampled coherence has an explicit signal/interference certificate.

## Advancement 1: differential deletion orbit

For each pattern and degree,

```text
sum_(|R|=t) G_(x\R) = D^t G_x / t!.
```

All deletion radii are the coefficients of `G_x(z+s*1)`. After normalization,
the jet components shift exactly under uniformly averaged deletion. The first
omitted component is the complete closure defect of a truncated jet.

This replaces the generic imposed contraction with one derived from the
candidate evidence itself.

Proof object: `formal/T-DELETION-ORBIT-1.md`.

## Advancement 2: deformation-history binding

The unlabelled derivative orbit can combine features produced by incompatible
descendants. `bab` versus query `aa` is the least failure. Bind every feature
from deletion set `R` to a common multiplicative phase `chi(R)` before
superposition.

The phase-projected orbit has a direct product formula over unselected source
positions, so it need not enumerate every descendant. This supplies the first
concrete mechanism for compressing a deformation neighborhood while retaining
some cross-feature history coherence.

Proof object: `formal/T-HISTORY-COUPLING-1.md`.

## Advancement 3: bounded-radius history moments

A radius-`t` deletion history is determined by its first `t` positional power
sums over a prime field whose range covers the source. The corresponding
characters factor multiplicatively over deleted positions and therefore fit
the history product formula exactly.

This replaces an `n`-bit deletion mask with `t` structural history coordinates
before compact projection. Exact bounded-range identity costs approximately
`t log n` bits; fixed smaller moduli become complementary fuzzy satellites.

Proof object: `formal/T-HISTORY-MOMENTS-1.md`.

## Advancement 4: exact history coherence

The complete moment-character field does more than distinguish histories.
Character orthogonality turns a zero-sum correlation of feature responses into
the exact number of deletion histories common to all those features. A true
descendant has positive full coherence; `bab`/`aa` has zero despite positive
unlabelled marginals.

The fixed-width problem is now a sharply defined approximation problem: retain
enough characters and feature-combination order that the common-history
diagonal survives incompatible-history interference.

Proof object: `formal/T-HISTORY-COHERENCE-1.md`.

## Advancement 5: sampled-history SNR

For any selected character family, pair coherence splits exactly into the
number of common histories plus incompatible-history sidelobes. If protected
atoms have at most `L` histories, maximum sidelobe `mu_A`, and stored-score
uncertainty `epsilon`, the fixed support certifies zero versus positive
intersection under

```text
1-mu_A(2L^2-1) > 2 epsilon.
```

This is the requested construction-specific breakdown law: an individual
deformation becomes uncertifiable because history multiplicity, selected phase
coherence, or quantization consumed its contribution—not because of a generic
content-length slogan.

Proof object: `formal/T-SAMPLED-HISTORY-1.md`.

## Hash candidate

The deformation-jet superposition hash retains fixed probes across:

- content pattern;
- gap position;
- occurrence degree;
- deletion radius;
- deletion-history phase.

Base, radial-flow, and history-coupling addresses remain independently
retrievable. Query evidence is compared with the appropriate record orbit
channels; exact records verify candidates.

Construction: `docs/DEFORMATION_JET_SUPERPOSITION_HASH.md`.

## Confidence

**Strong:** the exact derivative-orbit, translation, survival contraction, jet
shift, product lift, common-history criterion, moment-code injection, full
coherence identity, sampled pair decomposition, and two least obstructions.

**Moderate:** this is the right new pre-hash object. It unifies content,
position, combination, deformation scale, and history without first fusing
them into one vector.

**Low:** a small fixed phase/probe family will preserve enough history coherence
to beat equal-bit controls. That is now the central falsifiable risk, not an
unstructured uncertainty about “one universal hash.”

## Shortest path to uncertainty

Do not broaden modalities yet. Attack the smallest phase family on the two
history obstructions, then construct a least-support family whose history
multiplicity makes the sampled certificate fail. If the required sidelobe must
shrink with source length even for bounded deletion radius and protected
combination order, this version of the fixed hash fails cleanly. If a constant
family preserves the required coherence under a declared object class, the
program has its first credible new hashing mechanism.

## Subsequent measured refinement

E7 rejected small raw degree-one character sets as universally sufficient at
radius two, while showing that the address itself need not be combinatorial to
build. T-STREAMING-JET-1 collapses descendant and occurrence enumeration into
one linear retain/delete/select recurrence. T-PAIRWISE-HISTORY-1 proves pair
coherence closes radius one and locates the first required rise in interaction
order at radius two. See `DEFORMATION_ALGORITHM_REFINEMENT_001.md`.

E8 then closed the general deletion interaction order at exactly `t+1` and
rejected full identical-descendant quotienting inside cancellative product
phases from radius two. This demotes “find a sufficiently large character
subset” from the central path. The surviving construction uses safe run-count
phase blocks for graded numeric energy and an idempotent degree-`t+1` affine
certificate layer for indexable history deduplication. See
`RETRIEVAL_REFINEMENT_002.md`.
