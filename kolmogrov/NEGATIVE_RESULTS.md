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
- Exact protected replacement: for a frozen pair-separation graph `G`, the
  minimum collision-free binary width is `ceil(log2 chi(G))`; a Hamming margin
  strengthens the labeling problem. Width therefore versions the task
  obstruction contract, not content length alone.

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

## K-N014 — one universal observation partition is not support-neutral across tasks

Status: **REJECTED** as a free cross-modality basis.

- Claim attacked: if two tasks each need `k` observation states, one shared
  representation also needs at most `k`.
- Least obstruction: three fine states with crossing binary partitions
  `{a,b}|{c}` and `{a,c}|{b}`. Each task needs two states; their joint
  observation needs three.
- Lower bound: with at most two fine states, a two-valued observation is already
  literal and cannot incur a strict joint-support increase.
- Restricted survivor: tasks may share algebra and contract schemas; their
  simultaneous exact support is the joint observation partition.

## K-N015 — geometric detail energy is not relevant information

Status: **REJECTED** without a joint probabilistic model.

- Claim attacked: the energy or complete marginal law of a detail coefficient
  determines how much it says about the target.
- Least obstruction: uniform binary target `Y`; `Z=Y` and independent uniform
  `Z` have identical binary marginal and unit energy but carry one and zero bits
  respectively.
- Reverse obstruction: `Z=aY` preserves one bit for every nonzero `a` while
  energy `a^2` is arbitrary.
- Lower bound: nonzero mutual information needs at least two target and two
  observation states.
- Restricted survivor: energy and conditional information may be linked only
  after declaring conditional laws, noise, prior, and task readout.

## K-N016 — one progressive prefix cannot serve incomparable task supports without overhead

Status: **REJECTED** unless least sufficient task block sets are nested.

- Claim attacked: one block order can expose every task's minimum support as a
  prefix.
- Least obstruction: blocks `u,v` and tasks needing `{u}` and `{v}`. Either
  order serves one task at cost one and makes the other consume two blocks or
  fail.
- Lower bound: one block cannot create incomparable task requirements.
- Restricted survivor: a zero-overhead schedule exists exactly when all least
  sufficient task block sets are totally ordered by inclusion.

## K-N017 — interaction support is not automatically sparse

Status: **REJECTED** for universal linear fidelity.

- Claim attacked: a fixed number of interaction scalars can preserve every
  difference hidden from two axis marginals as the axes grow.
- Exact obstruction: the zero-marginal interaction space on `p x q` cells has
  dimension `(p-1)(q-1)`; rank-nullity forces every universally faithful linear
  certificate to have at least that dimension.
- Least nontrivial sparsity failure: `2 x 3`, whose two-dimensional interaction
  space cannot be preserved by one scalar.
- Restricted survivor: sparse certificates require a declared low-dimensional
  relevant span, finite protected obstruction set, or accepted ambiguity.

## K-N018 — the broad constituent mechanisms are not supportable novelty claims

Status: **REJECTED** within the scoped primary-source audit.

- Claims attacked: novelty of lumpable/predictive minimum state, graph-colored
  support, relevant-information compression, tensor interactions, four-cell
  marginal cycles, multiresolution detail, ordered prefixes, subsequence
  features, edit sketches, or multiple/product hash addresses.
- Evidence: the primary-source overlap map in
  `docs/PRIOR_ART_BOUNDARY_001.md`.
- What remains open: four narrower integrated constructions are **UNVERIFIED**,
  not novel.
- Reconsideration condition: a deeper scoped audit plus a claim chart showing
  the exact construction and theorem absent from each closest source.

## K-N019 — unlabelled deletion-orbit marginals lose common history

Status: **REJECTED** as a sufficient deformation-neighborhood hash.

- Claim attacked: if every exact positional query atom occurs somewhere in the
  source's radius-`t` deletion orbit, one source descendant contains the query.
- Absolute least obstruction: source `bab`, query `aa`, one deletion. Deleting
  the first `b` supports `a` at target position zero; deleting the last `b`
  supports `a` at target position one; no deletion produces `aa`.
- Lower bound: a target of length one has only one indexed-symbol atom, so
  marginal support and common-history support coincide. A strict source then
  needs length at least three; one symbol cannot fail.
- Content-multiplicity repair for this witness: the source contains only one
  `a`. This does not repair the general history loss.
- Least ordered obstruction with sufficient symbol multiplicities: source
  `aabb`, query `ba`, two deletions. `bb` supplies query position zero and `aa`
  supplies query position one, but no source subsequence orders `b` before `a`.
- Restricted survivor: the derivative orbit is exact for unlabelled sums and
  averages. Exact containment requires one common history supporting all query
  atoms; T-HISTORY-COUPLING-1 retains that axis before compact projection.

## K-N020 — pairwise common-history coherence does not close two deletions

Status: **REJECTED** from deletion radius two onward.

- Claim attacked: if every pair of exact positional query atoms shares some
  deletion history, one history supports the entire query.
- Least obstruction: source `ababa`, query `bbb`, two deletions. Descendants
  `bba`, `bab`, and `abb` support the three query-position pairs separately;
  the source has only two `b` symbols, so no history produces `bbb`.
- Lower bound: pairwise/global disagreement needs at least three query atoms.
  T-PAIRWISE-HISTORY-1 proves pairwise support is globally sufficient for one
  deletion. Hence at least three target positions plus two deletions require
  source length five, attained by the witness.
- Restricted survivor: radius-one positional history supports are intervals and
  pairwise coherence is exact. Radius two requires triple binding, degree-three
  occurrence evidence, or another explicitly sufficient interaction.

## K-N021 — deletion interaction below degree `t+1` is universally insufficient

Status: **REJECTED** for exact radius-`t` positional containment.

- Claim attacked: a fixed interaction degree no greater than deletion radius
  `t` can certify that one history supports the whole query.
- Sharp obstruction family: source `(ab)^t a`, query `b^(t+1)`. The source has
  only `t` copies of `b`, so the query is not a descendant, while every `t`
  query positions share a deletion history.
- Least support: distinguishing order `t` from `t+1` requires at least `t+1`
  target atoms and therefore source length at least `2t+1`, attained by the
  witness.
- Restricted survivor: T-DELETION-CLOSURE-1 proves degree `t+1` sufficient for
  the exact deletion predicate; bounded schedules may sample that degree while
  exposing a false-candidate residual.

## K-N022 — full outcome deduplication is not a safe product-phase quotient

Status: **REJECTED** from deletion radius two onward for cancellative
per-position multiplicative phases.

- Claim attacked: identify every pair of deletion histories producing the same
  descendant by equal product phase while separating every different
  descendant.
- Least radius-two obstruction: `aaba`. Equal codes for the three histories
  producing `aa` force the phase factors at positions `0`, `1`, and `3` equal;
  the histories producing distinct `ba` and `ab` then have equal product.
- General obstruction: `a^t b a` for every `t>=2`.
- Restricted survivor: deletion-count vectors over maximal constant runs are a
  safe factorable quotient. Full observable deduplication must use an
  idempotent membership operation or retain additional nonfactorable state.

## K-N023 — fixed certificate support cannot observe unbounded content length

Status: **REJECTED** when every one-symbol change must remain observable.

- Claim attacked: a fixed `Q` degree-`m` certificate schedule can remain
  sensitive to every position as target length grows without bound.
- Incidence obstruction: `Q` certificates contain only `Qm` position
  incidences. If target length `L>Qm`, some position is absent and can change
  without changing any certificate.
- Exact-mask consequence: with alphabet size `A` and `c` witnesses per
  position, payload obeys `B>=ceil(cL/m)A^m` bits.
- Restricted survivor: declare a supported length band, add independently
  addressable granular schedules, or accept and measure uncovered/colliding
  detail. Pattern hashing does not remove the position-incidence bound.

## K-N024 — independent projection membership does not preserve witness provenance

Status: **REJECTED** as exact multi-address collision rescue for a pattern set.

- Claim attacked: if several projection values jointly identify every individual
  pattern, intersecting their independent membership masks exactly tests whether
  one source pattern matches the query.
- Least whole-pattern obstruction: modulo three, use `u+v` and `u+2v`. Their
  joint map is injective, but source patterns `{(0,0),(1,0)}` accept query
  `(2,1)` using a different source witness in each view.
- Lower bound: one view cannot mix provenance across views; one source pattern
  supplies every view itself. The failure first needs two views, two source
  patterns, and an absent query.
- E9 consequence: four 4-cell views jointly identified all 64 individual
  patterns but achieved only 0.0676 candidate precision because their set
  memberships were unlinked.
- Restricted survivor: use one coupled whole-pattern cell, pay the product-space
  cost for a joint cell, retain an explicitly colliding provenance tag, or leave
  the residual to exact verification.

## K-N025 — an exact fine code does not select a useful coarse prefix

Status: **REJECTED** as a progressive-projection rule.

- Claim attacked: any invertible finest-scale pattern code has useful nested
  coarse prefixes.
- Least obstruction: two protected patterns may share the first digit and differ
  only in a later digit of an injective code. Two patterns and one proper coarse
  prefix are minimal.
- E9 consequence: an untuned full-rank six-bit mixer was exact at 64 cells but
  only 0.2955 precise at 16 cells, versus 0.8182 for a noninjective balanced
  affine view at the same support.
- Restricted survivor: compile prefix order against protected edges, private
  witnesses, and candidate tails, then freeze it before held-out evaluation.

## K-N026 — low bits of an exact composite atom are not a perceptual prefix

Status: **REJECTED** for Filename Profile 001's direct residue projection.

- Claim attacked: because an atom code is exact, reducing its integer value
  modulo a small power of two gives a useful first projection scale.
- Least mechanism: two distinct exact integers congruent modulo the selected
  width collide; one protected pair suffices.
- E10 witness: anchored-fold codes store one-byte length in their low bits, so
  `[1,3,5] mod 16` erased 66–86 exact pattern events for letter, digit, dot, and
  extension substitutions. Literal `A` and `a` also agree modulo 16.
- Restricted survivor: compile rows across all metadata and payload lanes so
  every protected atom difference has an early nonzero image; then audit
  multi-bit cancellation separately.

## K-N027 — certificate incidence does not guarantee source-symbol response

Status: **REJECTED** even before compact projection.

- Claim attacked: if a source position participates in a certificate offset
  state, changing that source atom changes the exact idempotent certificate.
- Least binary witness: radius two, degree three, `aaaaab -> aaaabb` at source
  position four, certificate subset `(0,1,3)`. Both exact pattern sets are
  `{aaa,aab}`.
- Minimality: radius-one three-state pair certificates cannot replace all old
  and new pairs from unaffected states; at radius two/source length five the
  full length-three subsequence set changes under every bit flip by the extremal
  symbol-count argument in T-SYMBOL-JACOBIAN-1. Length six is attained.
- Restricted survivor: incidence bounds possible work; exact response is the
  removed/added set followed by projected occupancy cells crossing zero.

## K-N028 — one length-only transport cannot represent insertion position

Status: **REJECTED** for exact positional cross-length flow.

- Claim attacked: stored hashes at adjacent lengths have a canonical transport
  determined only by old and new length.
- Least obstruction: for one old atom, left insertion maps it to new position
  one while right insertion maps it to new position zero.
- Lower bound: an insertion and one retained atom are the smallest objects that
  can disagree about transported position.
- Restricted survivor: use lineage-conditioned transport, hide/average
  position with explicit residual, or perform directional cross-length
  certificate retrieval without subtracting stored hashes.

## K-N029 — fewer posting memberships do not imply a cheaper useful index

Status: **REJECTED** without support, candidate, and verification gates.

- Claim attacked: a projection with fewer occupied posting cells is
  automatically more economical.
- Least mechanism: collapse two distinguishable patterns into one cell. Posting
  membership falls while private influence disappears and the posting list
  admits both candidate classes.
- E10 witness: failed folded low-bit projection used only 10.67 entries/object,
  versus 78.89 for the balanced control, because it erased most one-byte
  content changes.
- Restricted survivor: compare encoded bytes and physical reads only among
  configurations that pass private/SNR, candidate-tail, and exact-verification
  gates.

## K-N030 — degree-`t+1` certificate overlap is not exact symmetric history closure

Status: **REJECTED** as a converse for same-length common descendants.

- Claim attacked: if every complete degree-`t+1` certificate pattern set of two
  sources intersects, the sources have one common radius-`t` descendant.
- Least witness: radius one, `aaba` and `babb`. All three degree-two certificate
  intersections contain `ab`, but their one-deletion descendant sets
  `{aba,aab,aaa}` and `{abb,bbb,bab}` are disjoint.
- Minimality: at source length three, degree two is the full two-position target
  and its one certificate is exact. Length four first admits incompatible
  proper-subset witnesses.
- Restricted survivor: certificate overlap has exact common-descendant recall
  and supports a bounded posting-union front index; exact verification rejects
  incompatible histories.

## K-N031 — development kernel avoidance does not imply unseen mutation support

Status: **REJECTED** as a generalization claim.

- Attempt: compile the shortest binary prefix whose kernel avoids every
  literal, folded, and structural pattern difference induced by one generated
  filename mutation graph, then lift its rows into support-balanced ambient
  cosets.
- Strongest result: E11 proved that two rows are the exact minimum for all three
  development graphs and erased none of their 51 literal, 45 folded, or nine
  structural differences.
- Failure: the disjoint graph still had 13 of 48 literal and nine of 42 folded
  differences erased at two rows. At the development-selected four rows it had
  four and eight erased respectively, and worse candidate tails than the
  equal-depth balanced control. Tested unseen differences reached zero only at
  six rows.
- Least mechanism: one unseen nonzero difference outside the declared graph may
  lie in the selected kernel. The protection theorem constrains no such
  direction, so one unseen edge is the smallest possible failure.
- Restricted survivor: protected rows are a guard, not a sufficient selection
  objective. Reopening requires independent-graph minimax or an explicit
  ambient-coverage reserve without retuning E11.

## K-N032 — semantic minimum row support is not useful retrieval support

Status: **REJECTED** as an unqualified minimum-hash claim.

- Attempt: treat the first prefix protecting every named development
  difference as the minimum adequate filename projection.
- Failure: E11's exact semantic minimum was two rows in every view, yet p95
  candidate fraction was 55.56% and p95 false candidates were 159 of 288
  records. The predeclared candidate-tail gate first passed at four rows.
- Least mechanism: `L(delta)!=0` protects a named pattern edge before
  idempotent occupancy. It does not stop an unprotected pair from colliding or
  incompatible histories from satisfying different certificates. One such
  collapsed pair is sufficient.
- Restricted survivor: report semantic protection depth, candidate-tail depth,
  and unseen-direction completion depth separately. None alone is “the minimum
  hash size.”

## K-N033 — separate guard and coverage overlap is not joint witness evidence

Status: **REJECTED** as a provenance claim.

- Claim attacked: if guard masks overlap and coverage masks overlap, one common
  exact pattern supplied both matches.
- Least obstruction: two left patterns and two right patterns, with the guard
  equality witnessed by the first pair and coverage equality by the second,
  while every coupled `(guard,coverage)` code differs.
- E12 result: the separate split admitted candidates rejected by the coupled
  pair-cell control on 46 of 216 evaluation queries, with p95 four and maximum
  nine split-only candidates.
- Restricted survivor: separate overlap remains recall-safe and additive in
  dense support, but must expose provenance mixing and exactly verify.

## K-N034 — equal dense occupancy support is not equal posting support

Status: **REJECTED** as a resource equivalence.

- Claim attacked: two three-bit addresses costing `8+8=16` mask cells have the
  same practical index support as one four-bit/16-cell address.
- E12 result: the split guard3/coverage3 layout used 106.44 hybrid payload
  bytes/record on tuning versus 61.40 for coverage4, because independently
  addressable masks and reverse keys duplicate memberships and dictionaries.
- Least mechanism: one record occupying one cell in each of two address
  families creates two posting memberships, while one coupled address creates
  one. A singleton pattern is sufficient.
- Restricted survivor: equal-support comparisons must name dense mask bits,
  posting memberships, address dictionaries, and query predicates separately.

## K-N035 — policy-sensitive filename evidence is not a universal veto

Status: **REJECTED** without a transformation-conditioned relevance policy.

- Claim attacked: every digit, extension, or boundary-affecting one-edit pair
  should be rejected after verification.
- E12 result: typed evidence marked every genuine verified digit/extension hard
  decoy, but it also marked every intended separator-deletion source.
- Least mechanism: one declared-positive boundary deletion and one declared-
  negative boundary deletion can share the same evidence class. No classifier
  over that class can separate their labels.
- Restricted survivor: typed evidence may drive an owner-labelled conditional
  rank policy; it is not itself the policy.

## K-N036 — a tuning candidate-tail threshold need not transfer

Status: **REJECTED** as a frozen-distribution guarantee.

- Claim attacked: the first certificate schedule passing p95 eight candidates
  on a disjoint tuning generator will retain that tail on another generator.
- E12 result: eight certificates first passed the tuning gate, then produced
  p95 sixteen candidates on evaluation while retaining perfect recall.
- Least mechanism: one evaluation query with a larger compatible collision
  class is sufficient to violate a tail threshold; recall theorems do not bound
  that class.
- Restricted survivor: candidate-tail gates are empirical distribution claims
  and require representative consented data or a direct worst-case bound.

## K-N037 — universal exact posting precombination is exponential in cell count

Status: **REJECTED** as a general removal of occupied-cell iteration.

- Claim attacked: precompute a bounded family of posting unions that answers
  every nonempty `B`-cell query mask with one lookup.
- Obstruction: distinct query masks define distinct predicates; a singleton
  stored mask in their symmetric difference separates them. There are `2^B-1`
  nonempty masks.
- Restricted survivor: store `B` bit-sliced postings contiguously and perform
  explicit wordwise ORs for selected cells, or prove a smaller closed query-mask
  language.

## K-N038 — equal dense-cell support does not support a fixed history count

Status: **REJECTED** for small separable history addresses.

- Claim attacked: redistributing 384 dense cells among more independent history
  occupancy addresses should improve radius-one selectivity.
- E13 result: evaluation p95 candidates worsened from 174 at `1 x 128` cells
  per semantic view to 193 at `4 x 32`; recall stayed exact.
- Least mechanism: two length-`n` history sets expose `n^2` collision pairs.
  When `n^2/B>=1`, each coordinate's union-bound premise is saturated; making
  `B` smaller cannot rescue it.
- Restricted survivor: increase address cardinality past the history-pair load
  or couple coordinates into one history key.

## K-N039 — separate coordinate agreement is not whole-history coherence

Status: **REJECTED** as a coherence claim.

- Claim attacked: requiring overlap in every independent fingerprint coordinate
  proves that one common deletion history generated all overlaps.
- Least mechanism: coordinate one may collide on history pair `(h1,g1)` while
  coordinate two collides on `(h2,g2)`. Both occupancy intersections are
  nonempty although no tuple key is shared.
- E13 result: coupling coordinates cut memberships roughly in half and reached
  zero measured excess at two 9-bit coordinates across all three generated
  splits.
- Restricted survivor: independent masks remain a recall-safe fuzzy control;
  they are not provenance-bound evidence.

## K-N040 — E12's tail did not establish complete-certificate history mixing

Status: **REJECTED** as the causal explanation for that workload.

- Claim attacked: the E12 candidate plateau was materially caused by
  incompatible histories in the complete degree-two certificate relation.
- E13 result: `C minus T` contained zero evaluation pairs. Bounded-schedule omission
  contributed 696 candidates and projection collision contributed 323.
- Restricted survivor: complete certificates can mix histories in general, as
  the formal obstruction shows, but that mechanism must be measured before it
  is blamed for a particular tail.

## K-N041 — an 18-bit generated crossing is not collision freedom

Status: **REJECTED** as a deterministic or length-only guarantee.

- Claim attacked: because two 9-bit coupled coordinates produced zero excess
  on all E13 generated queries, they support arbitrary names of similar or
  shorter length.
- Obstruction: descendants `ldfioi` and `kbmedf` share literal tuple
  `(258,216)`, fold tuple `(100,319)`, and the all-letter structural tuple.
  Sources `ldfioia` and `kbmedfa` have no common exact deletion descendant but
  the full three-view hash accepts them.
- Minimality boundary: exhaustive direct combined literal/fold key enumeration
  over `abcdefghijklmnop` found no collision through descendant length five;
  the retained direct collision has length six. This does not prove minimality
  among all cross-view borrowed-history candidate failures.
- Restricted survivor: 18 bits is an E13 workload point. Production support
  requires a declared pool/failure envelope, wider or keyed coordinates, and
  exact external authority.
