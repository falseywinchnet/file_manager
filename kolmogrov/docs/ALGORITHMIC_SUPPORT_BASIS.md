# Algorithmic support basis: semantic cylinders and progressive detail

Status: **CANDIDATE construction derived from the support program; no routing
schedule, channel norm, width, or quantizer selected**.

The active concrete instantiation is now the deformation-jet superposition hash
in `DEFORMATION_JET_SUPERPOSITION_HASH.md`. The semantic-cylinder basis remains
a general coordinate-routing candidate; it does not supersede the exact
occurrence-derived deletion contraction.

## 1. Support atom

The exhaustive oracle supplies canonical atoms whose fields remain separately
visible:

```text
u = (channel, family, pattern, occurrence order, gap position,
     degree, object scale, direction, source anchor).
```

Exact source anchors are not compressed into perceptual identity. The support
basis groups observable atoms for candidate geometry.

There is no support-neutral universal observation basis. T-UNIVERSAL-SUPPORT-1
proves that each task or modality has its own coarsest observation partition;
support for simultaneous tasks is their joint refinement. Cross-modality reuse
should therefore target the algebra, contraction law, certificate schema, and
exact-verification seam—not silently charge every task for the joint partition.

## 2. Infinite semantic route

Assign every atom an infinite base-`b` route

```text
rho(u) in {0,...,b-1}^N.
```

The first `m` digits name one of `K_m=b^m` cylinder cells. Level `m+1` splits
each cell into `b` children; contraction sums or averages children into their
parent.

The route must be structurally derived, not an arbitrary engineering hash.
Candidate digit families are:

- channel and interaction family;
- content refinement: symbol class, literal symbol, fragment prefix, motif;
- combination refinement: degree, contiguous/global order, gap/order pattern;
- position refinement: gap-simplex region and progressively finer gap digits;
- object scale: length and degree/length regime;
- directional role: symmetric, query-to-record, record-to-query;
- sparse coupling certificate linking otherwise marginal addresses.

An interleaving schedule declares which axis receives each refinement digit.
T-TENSOR-SUPPORT-1 shows that the underlying scale is a multi-index lattice;
the digit string is only one serialization path through it. That schedule is
part of representation identity. A learned or corpus-tuned schedule is a
separate candidate with a frozen artifact; it cannot be silently presented as
a universal mathematical construction.

T-PATH-ORDER-1 shows that commutativity does not select the schedule: at the
same intermediate dimension, one path can annihilate a least-support position
direction that the other partly retains, with the preference reversed for a
content direction. Schedule selection must therefore cite the required affine
direction family and each progressive support budget.

T-NESTED-SCHEDULE-1 gives the exact static-prefix boundary. When detail blocks
are independently addressable and each task has a unique least sufficient block
set, one prefix order realizes every task without overhead if and only if those
sets are totally ordered by inclusion. Two tasks needing distinct singleton
blocks are the least obstruction. Incomparable task supports require direct
addressing, multiple schedules, or declared overhead.

## 3. Coordinate state

At each level, aggregate atom mass separately by primary channel and explicit
interaction channel. Positive count/mass coordinates admit exact `l_1` parent
sums. Signed or centered coordinates use the normalized Banach/Hilbert
contraction of T-SCALE-1.

The construction must preserve an audit map from each occupied cell back to its
source atoms. Two atoms sharing a cell are a declared scale collision, not an
identity merger.

## 4. Progressive Hilbert support

For the Hilbert contraction, every fine state decomposes uniquely as

```text
z_(m+1) = R_m z_m + d_m,
z_m = A_m z_(m+1),
d_m in ker(A_m).
```

The fine support is therefore coarse support plus new orthogonal detail—not a
replacement hash unrelated to the previous scale.

For symbolic deletion, T-DELETION-ORBIT-1 now supplies a stronger intrinsic
scale object than generic block contraction. The normalized occurrence
derivatives are the actual averaged descendant states, and a truncated jet's
entire flow defect is its next derivative component. Generic Hilbert detail
remains an accounting control for probes and quantization around that object.

Choose an orthonormal basis for each detail space. Through level `M`, the number
of scalar coefficients is exactly

```text
K_0 + sum_(m=0)^(M-1) (K_(m+1)-K_m) = K_M.
```

Thus a progressive pre-quantized representation can expose every coarse prefix
and every refinement using the same scalar dimension as the finest full state.
Headers, scale identities, confidence margins, and nonuniform quantization add
serialized overhead and must be counted separately.

## 5. Why this differs from truncating random bits

Dropping a semantic detail block has an exact squared-error charge and a named
axis/scale. Dropping the tail of an unrelated binary hash has neither property.
The cylinder route makes resolution loss inspectable:

- content cells merge declared content distinctions;
- position cells merge declared gap regions;
- combination cells merge declared incidence structures;
- interaction cells lose declared coupling.

The first interaction candidates should be four-cell checkerboard contrasts.
T-INTERACTION-MIN-1 proves that four cells are the least support invisible to
both product marginals, and exhibits such a cell inside the E2 obstruction.
Those rectangles span the entire marginal-invisible subspace. Given a frozen
obstruction set, select the minimum rectangle coordinates hitting every required
collision difference. Retaining the full interaction tensor is not the default
repair, and success on the frozen hitting set is not evidence of held-out
generalization.

T-INTERACTION-SPARSITY-1 prevents a stronger claim. For a `p` by `q` product,
the marginal-invisible interaction space has dimension `(p-1)(q-1)`, and no
smaller linear certificate is universally faithful. Sparse interaction support
is defensible only relative to a declared relevant subspace, finite protected
obstruction set, or accepted ambiguity. The `2` by `3` table is the least case
where one interaction scalar necessarily misses a direction.

The route is good only if those merges match the transformation and retrieval
loss. The generic tree supplies bookkeeping, not relevance.

## 6. Minimal support along one direction

For detail space `W_m=ker(A_m)` of dimension `q_m=K_(m+1)-K_m`, let edit
direction `v` contribute

```text
E_detail(m,v)=||D_m J_(m+1)v||^2.
```

Average new-coordinate energy is `E_detail/q_m`; at least one coordinate has no
more than that energy. If every new coordinate is claimed necessary and must
exceed threshold `theta`, necessarily `q_m theta<=E_detail`.

This does not license deleting the entire detail block when average energy is
small: energy may be concentrated in a few indispensable coordinates. Record
active support, effective energy support, maximum contribution, and the full
distribution before pruning.

For a declared affine direction family `V_req`, the minimum supported level is
the first level satisfying all of:

1. `d_m(tau)` meets the required thresholded direction count;
2. every required atomic edit meets the SNR gate;
3. discarded detail tail is below the distortion budget;
4. relevant conditional information loss is below its budget;
5. quantization margins certify every bit claimed stable.

The gates need not be monotone together. The admissible scale set may be empty
or disconnected.

After the pre-quantized flow passes those gates, T-QUANT-CERT-1 permits an
inherited coarse sign only when its complete feasible contraction interval lies
strictly on one side of zero. Every other bit is `uncertified`; interval or
margin certificate storage counts against serialized support.

## 7. Breakdown controls

Every candidate support round reports:

- cell occupancy and collision classes by axis and level;
- coarse/detail energy by channel, edit family, and singular direction;
- projective residual when independently generated levels fail to commute;
- thresholded Jacobian support `d_m(tau)`;
- atomic response/SNR distributions, especially the minimum;
- effective occupied-codeword entropy and conditional relevant information;
- quantizer margin distribution and uncertified coarse bits;
- exact anchors recovered by candidate verification.

No scalar “support score” replaces this vector until a task loss proves the
reduction valid.

For finite protected collision requirements, T-OBSTRUCTION-CODE-1 supplies an
exact serialized-width floor: build the forbidden-collision graph `G`; then
`B_min=ceil(log2 chi(G))` bits are necessary and sufficient at zero Hamming
margin. The triangle is the least one-bit obstruction. Thus width is relative
to a protected task graph and margin, not content length alone.

T-ENERGY-INFORMATION-1 also keeps two breakdown ledgers separate. A binary
observation can have the same marginal energy and either one bit or zero bits of
target information; conversely, rescaling can make energy arbitrarily small
without changing information. Any bridge from SNR/contribution energy to
information requires an explicit conditional observation/noise model.

## 8. Immediate construction questions

1. Which semantic digit schedule gives the smallest detail tail for deletion,
   substitution, transposition, containment, and hard negatives separately?
2. Do gap-simplex barycentric digits contract more faithfully than raw integer
   residues across unequal object lengths?
3. What minimal interaction detail prevents the mod-2/mod-3 marginal-coupling
   failure without recreating the full witness?
4. Can a frozen universal route retain adequate Jacobian support, or must
   modality-specific routes share only the contraction law?
5. Which detail coefficients require amplitude margins so coarse quantized bits
   remain certified?

These are algorithmic-support questions. Index choice, SIMD, and storage layout
remain later implementation questions.
