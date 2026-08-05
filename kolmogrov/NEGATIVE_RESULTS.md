# Negative-results ledger

Status: **active retained-failure ledger**.

Retain for every rejected construction:

- strongest version tested;
- exact claim and baseline;
- corpus, seed, environment, and command;
- what improved;
- what regressed;
- smallest counterexample or failure class;
- whether the objective, proof, implementation, or tuning failed;
- genuinely new mechanism required before reconsideration.

Known inherited warnings to re-audit rather than forget:

- a fused fixed-width vector can collapse independent evidence back into one
  brittle location;
- raw cumulative features allow length to dominate direction;
- moment position features were smoother but retrieved worse in the observed
  corpus;
- optional anagram features improved transpositions but reduced discrimination;
- exact scan may cease to meet latency at scale;
- deterministic engineering hashes do not automatically inherit ideal-random
  concentration theorems;
- deletion can create irreducible source ambiguity;
- semantic equivalence is not supplied by lexical geometry alone.

## K-N001 — symbol multiplicity is not sequence-complete

Status: **REJECTED** as a complete representation; retained as a content
channel.

- Claim attacked: a symbol-count multiset uniquely determines a finite symbolic
  sequence.
- Strongest version tested here: exact, uncompressed symbol counts over every
  binary sequence through length five.
- Control: exact literal length plus indexed symbols.
- Smallest counterexample under length-then-declared-alphabet enumeration:
  `(a, b)` and `(b, a)` share `{a: 1, b: 1}`.
- What it preserves: exact content identity and multiplicity without order.
- What it loses: position and combination/order.
- Reconsideration condition: none for completeness by itself; it remains useful
  only as independent evidence combined with other channels.

## K-N002 — degree-2 order multiplicity is not sequence-complete

Status: **REJECTED** as a complete representation; retained as a combination
channel.

- Claim attacked: the exact multiset of all ordered degree-2 symbol patterns
  uniquely determines a fixed-length sequence.
- Strongest version tested: exact uncompressed occurrence multiplicity over all
  binary sequences through length twelve.
- Smallest counterexample: `abba` and `baab`.
- Finite census: 83,322 collision pairs; largest class 58.
- What it improves over content: it retains pair order and cuts collision count
  sharply.
- What it loses: occurrence position and higher-order coupling.
- Reconsideration condition: only as one independently measured channel, never
  as an exact or complete representation.

## K-N003 — a fixed modular gap satellite is not universally injective

Status: **REJECTED** as universal exact representation; retained as a
collision/stability candidate.

- Claim attacked: one fixed modulus on every degree-2 gap coordinate uniquely
  identifies arbitrary finite symbolic sequences.
- Exact counterexamples found: mod 2 first collides at length seven; mod 3 at
  length ten; targeted mod-5 search first collides at length sixteen.
- Mod-5 counterexample: `aaaaabaaaabaaaaa` versus
  `baaaaaaaaaaaaaab`.
- What it improves: small residue coordinates preserve a one-step cyclic motion
  law under deletion and drastically reduce collision classes on the finite
  corpus.
- What it loses: gap quotienting aliases positions separated by the modulus.
- Reconsideration condition: a bounded-length theorem, multiple complementary
  satellites, or an explicit coupling mechanism—not a universal claim.

## K-N004 — coprime marginal addresses lose occurrence coupling

Status: **REJECTED** as a CRT-equivalent replacement for a joint address.

- Claim attacked: independent mod-2 and mod-3 aggregated occurrence addresses
  retain the same reconstruction information as coupled mod-6 occurrences.
- Smallest counterexample in the E2 exhaustive binary search:
  `aaabbabbaaa` versus `bbaaaaaaabb` at length eleven.
- Both separate marginals collide; the joint mod-6 address and mod-5 satellite
  distinguish the pair.
- Failure mechanism: aggregation discards which mod-2 and mod-3 residues came
  from the same occurrence, so per-coordinate CRT hypotheses are absent.
- Least-support kernel inside the counterexample: the `bb` joint counts contain
  a four-cell checkerboard, which is the minimum nonzero signed product measure
  with both marginals zero by T-INTERACTION-MIN-1.
- Least repair for this pair: one checkerboard interaction contrast; no claim is
  made that it repairs other marginal collisions.
- Reconsideration condition: preserve a bounded interaction certificate or
  prove that the retrieval loss does not require the discarded coupling.

## K-N005 — low-degree compatible witnesses need not share one parent embedding

Status: **REJECTED** as an exact containment test below full-query degree;
retained as a candidate-generation score.

- Claim attacked: if every degree-`k` query occurrence has a distinct
  coordinatewise gap-dominated record occurrence, then the whole query is an
  ordered subsequence of the record.
- Smallest degree-1 counterexample in E4: query `ba`, record `aabb`.
- Smallest degree-2 counterexample in E4: query `baa`, record `aabbab`.
- Failure mechanism: separate witness matches can require mutually incompatible
  deletion sets and order embeddings.
- What it improves: on all binary query/record pairs through record length six,
  orthant position compatibility reduced pattern-only false positives from
  2,520 to 154 at degree one and from 550 to four at degree two, without losing
  an exact deletion descendant.
- Reconsideration condition: use the score as an explicitly low-degree
  candidate channel, add a higher-degree consistency certificate, or require
  full-query degree for exact subsequence control.

## K-N006 — fine sign bits do not determine a coarse contraction sign

Status: **REJECTED** for unrestricted pre-quantized amplitudes.

- Claim attacked: a deterministic contraction of fine binary sign bits can
  always reproduce the sign of the corresponding coarse coefficient sum.
- Least two-coordinate mechanism: for any `eta>0`, `(1+eta,-1)` and
  `(1,-1-eta)` expose identical fine signs `(+,-)` but have positive and
  negative coarse sums. One positive contraction coordinate preserves sign and
  cannot obstruct the claim.
- Failure object: quantization discarded the amplitude comparison needed by the
  coarse scale.
- What remains valid: pre-quantized Banach contraction and margin-qualified
  fuzzy sign certification.
- Reconsideration condition: bound/quantize amplitudes and retain a sufficient
  margin certificate, or compute coarse bits from pre-quantized state rather
  than fine signs alone.

## K-N007 — hash width alone does not define perceptual content capacity

Status: **REJECTED** as an under-specified universal claim.

- Claim attacked: a fixed bit or coordinate width by itself determines the
  largest object length a perceptual candidate hash can support.
- Failure mechanism: exact identity, fuzzy decoding, local affine separation,
  retrieval recall, and atomic SNR impose different lower bounds and equivalence
  classes.
- Exact width yields only a codeword cardinality bound. A fuzzy bound additionally
  needs a decoder and distortion radius. An influence bound additionally needs
  a required edit set, norm, total response-energy bound, noise, and SNR gate.
- Reconsideration condition: name the support meaning and all loss/noise
  assumptions, then apply the matching theorem rather than a universal `n(B)`.

## K-N008 — marginal address entropies are not additive support

Status: **REJECTED** as an information-contribution rule.

- Claim attacked: total information support of independent retrieval addresses
  is the sum of their marginal entropies or marginal relevant informations.
- Minimal counterexample: let `A_2=A_1`. The marginal sum doubles while the joint
  address contains exactly the information of `A_1`.
- Failure mechanism: redundancy and coupling are ignored.
- Correct control: joint relevant information and an explicitly ordered chain
  of conditional mutual informations; shared credit remains order-dependent.
- Reconsideration condition: prove independence under the declared distribution
  and target variable, or use a symmetric allocation rule while retaining its
  assumptions and interaction terms.

## K-N009 — one scalar refinement depth is not the fundamental support scale

Status: **REJECTED** as a universal geometry; retained as a serialization
schedule.

- Claim attacked: content, position, combination, object scale, and interaction
  support must all refine at one common depth `m`.
- Failure mechanism: distinct axis contractions commute, so the underlying
  state is indexed by a multi-index. A scalar depth chooses one path through
  that lattice and silently couples unrelated allocation decisions.
- Exact control: T-TENSOR-SUPPORT-1 decomposes axis details and their
  interactions without selecting a path.
- What remains useful: a scalar semantic-cylinder route is a reproducible
  storage/transmission order once its axis schedule is declared.
- Reconsideration condition: prove that one declared path is optimal or within
  a bound for the chosen task distribution and support budget.

## K-N010 — exact position-resolved edit closure is not compressible perceptual support

Status: **REJECTED** for the binary systems in T-ACTION-SUPPORT-1 and E6.

- Claim attacked: a small state observed only through symbol mass can still
  predict every separately addressed position edit exactly.
- Strongest finite systems checked: all fixed binary lengths through eight for
  flips and all binary strings through maximum length eight for deletions.
- Failure mechanism: the response to the edit at each position reveals the bit
  at that position, so the closed support equivalence is literal equality.
- Smallest strict cases: length two needs four flip-flow states rather than
  three weight states; strings through length two need seven deletion-flow
  states rather than six length/weight states.
- What remains useful: uniformly averaged edit kernels are exactly closed on
  weight support, with `n+1` fixed-length states or triangular variable-length
  support.
- Reconsideration condition: weaken exactness, hide/average the position label,
  restrict the object class, or leave exact position control to authoritative
  records. Each option changes the declared contract and must be explicit.

## K-N011 — occurrence refinement does not monotonically preserve edit-flow closure

Status: **REJECTED** under uniformly averaged binary deletion.

- Claim attacked: adding degree-two ordered occurrence counts to a closed
  degree-one content observation cannot increase the state needed for closed
  averaged edit dynamics.
- Least obstruction: `0110` and `1001`, the first and only degree-two
  occurrence collision through length four. Their uniform-deletion target
  multisets differ.
- Lower bound: degree-two occurrence evidence is injective through length three,
  so no shorter binary obstruction exists.
- Least repair through length four: split this one cell, making the partition
  literal.
- Restricted survivor: length/weight remains closed under averaged deletion,
  flip, and adjacent transposition; the failure appears when the observation is
  refined without also refining its dynamic state.

## K-N012 — small average closure residual does not protect rare-state recall

Status: **REJECTED** without a protected-state, tail, or task-margin guard.

- Claim attacked: arbitrarily small distribution-weighted transition residual
  guarantees adequate recall for every epistemically relevant state.
- Least obstruction: three states `{a,b,c}`, merging `a,b` while only `b`
  transitions to the distinct cell of `c`. Assigning mass `epsilon` to `b`
  gives optimal average residual `epsilon` and conditional recall zero on `b`.
- Lower bound: two merged states plus a distinct observable target require at
  least three fine states.
- Restricted survivor: average residual bounds expected Lipschitz readout error
  under the same distribution.
- Reconsideration condition: add a supremum/tail bound, protected hard-negative
  set, minimum group mass, or a decision-margin theorem.

## K-N013 — commuting axis contractions do not make progressive order irrelevant

Status: **REJECTED** at intermediate support; true at the common finest state.

- Claim attacked: because content and position contractions commute, either
  refinement order preserves the same information at every equal-size prefix.
- Least obstruction: a two-atom mass-conserving position secant is annihilated
  by position-coarse/content-fine support but partly retained by the opposite
  path. A content secant reverses the preference.
- Lower bound: a nonzero mass-conserving direction requires at least two active
  atoms.
- Restricted survivor: both paths agree after both axes are refined.
- Reconsideration condition: prove equality for the declared direction family,
  or choose order from measured direction energy at every budget.
