# History-bound hash refinement 007

Status: **research synthesis; coupled history tuple is a CANDIDATE, not an
accepted engine contract**.

## The narrowed objective

Kolmogrov is not a search engine and does not decide relevance. Its job is to
emit deterministic candidate evidence for a declared transformation relation
inside a declared capacity envelope. The parent engine may decompose ngrams,
run multiple transposition/edit tests, rank, and consult exact stored records.

For Filename Profile 001 at radius one, the core hash question is:

> Can two names sharing an exact one-deletion descendant be made to share an
> index key, with bounded excess, bounded support, and linear construction?

The coupled history tuple now has a clean affirmative answer under explicit
length/pool/hash-family bounds.

## The object

For each semantic atom stream and each possible deletion position, compute the
fingerprint of the complete resulting descendant. Project that fingerprint
through `q` nested coordinates and retain the coordinates as one tuple:

```text
history h -> complete descendant -> (cell_1, ..., cell_q)
```

The tuple must remain coupled. Turning it into `q` independent occupancy masks
permits a different history to satisfy each coordinate and reopens the
coherence failure.

## Active algorithm, shorthand

For one source stream of length `n`:

1. Encode its `n` atoms once for each coordinate.
2. Compute the fingerprint with position zero deleted.
3. Emit its coupled tuple key.
4. Move the deleted position right using one rolling correction.
5. Repeat steps 3--4 through position `n-1`.

The only content-length iteration is highlighted here:

```text
INITIAL SCAN:       n atom-code reads per coordinate
HISTORY ITERATION:  n tuple emissions
ROLLING UPDATES:    n-1 per coordinate
SUBSET ITERATIONS:  none
DESCENDANT COPIES:  none
```

Construction is `O(qn)` arithmetic and `O(n)` emitted sparse tuple keys per
semantic view. Coordinate arithmetic is independent and vectorizable. Powers
advance by multiplication; there is no inner loop over deletion suffixes.

## Lookup shape

An inverted index stores two logical posting families:

- history tuple keys for a source of length `n`;
- one full-sequence tuple key, addressed to the `n+1` directional plan.

For a query of length `n`:

```text
same-length records:   at most n history-key posting probes per semantic view
length n+1 records:    one full-key probe per semantic view
length n-1 records:    at most n history-key probes into full-key postings
```

Unions occur within one semantic view; declared views are intersected. Posting
work can short-circuit on an empty/small view, but no universal precombined
table removes the occupied-key iteration without exponential predicate support
(T-OCCUPANCY-QUERY-1).

## Necessity

Necessary for the hash relation:

- a source-anchored observation profile;
- one complete-history key per supported deletion outcome;
- coupling of all collision-rescue coordinates for that history;
- enough address cardinality for length and same-partition record count;
- an explicit overflow result when capacity is exceeded;
- deterministic configuration identity and migration rules.

Not necessary inside Kolmogrov:

- relevance labels or ranking weights;
- ngram decomposition;
- transposition/edit adjudication beyond the declared hash relation;
- exact filesystem identity;
- AI interpretation or semantic memory policy.

Those are valid consumers or companions, but importing them into the hash gate
would blur capability with policy.

## Capability

The coupled tuple provides:

- theorem-level recall for exact common one-deletion descendants and both
  directional length seams;
- whole-history coherence inside each tuple;
- linear rolling construction;
- sparse inverted postings rather than materialized `B^q` bitmaps;
- exact coarse/fine contraction across nested cell scales;
- a conditional support law involving content length and candidate-pool width;
- an observable changed-key energy for mutation/SNR gates.

It does not provide:

- deterministic collision freedom at finite width;
- arbitrary-length support;
- cryptographic integrity;
- ranked similarity;
- a proof that semantic-view normalization commutes with every edit;
- exact identity.

## Why the prior object broke down

E13 separates the failures.

The E12 bounded certificate control admitted 696 evaluation candidates from
omitted certificate directions and 323 from projection collisions. Complete
degree-two certificates had no incompatible-history excess on that workload.
The earlier suspicion was therefore too broad.

Naive whole-history occupancy then failed for a different reason: each name had
about 18 history elements but each small address offered only 16--128 cells.
Unrelated names had about `18^2` opportunities to intersect. Dividing 384 fixed
cells among more separate addresses made every coordinate more saturated, and
separate acceptance could borrow a different history in each coordinate.

Coupled tuples remove that borrowing. Under uniform independent coordinates,
their false-pair bound is

```text
n_x n_y / 2^w,
```

not `(n_x n_y / B)^q`. For `M` records and desired expected excess `tau`, the
working support law is

```text
w >= log2(M n_x n_y / tau).
```

This is the first support statement here that names both content size and the
population against which the hash operates.

## Granular scale

Coordinate `c_B(y)=floor(yB/P)` is an exact quotient of any finer coordinate
whose cell count is an integer multiple of `B`. A fine tuple can be contracted
componentwise to a coarse tuple without content access. Candidate sets grow
monotonically toward coarse scale; declared recall does not change.

This is a projective refinement tower, not a Banach fixed-point theorem. It is
enough for a practical coarse-to-fine index:

```text
coarse tuple -> broad posting partition -> read more stored coordinate bits
             -> refined tuple -> narrower posting partition
```

The content is fingerprinted once. Refinement consumes retained key detail,
not another pass over the name.

## SNR and the one-letter boundary

Define contribution energy as the symmetric difference in occupied history
keys before and after a mutation. Zero means the hash cannot observe that
change at the chosen scale. No finite hash has a positive universal minimum:
collisions and occupancy cancellation are least-support obstructions.

Under a uniform tuple address of cardinality `R`, the expected fraction of
histories retaining private keys is `(1-1/R)^(n-1)`. This gives a defensible
overflow/SNR gate. Once private contribution falls below a declared floor—or a
measured named mutation class reaches the invisibility threshold—the content
has exceeded that configuration's perceptual support.

E13 observed no invisible same-length nonexact mutation. That is evidence for
the generated domain only.

## What looks viable now

**Higher confidence:**

- the rolling recurrence and exact relation recall;
- coupled tuple necessity for coordinate coherence;
- sparse `O(n)` posting support;
- exact nested scale contraction;
- length-plus-pool capacity as the correct K1 boundary;
- exclusion of ranking/relevance from Kolmogrov's contract.

**Medium confidence:**

- two coordinates are preferable to three at equal tuple bits because they do
  less arithmetic and E13 found equal selectivity at 18 bits;
- 18 bits/view is a useful small-corpus research point, not a safe width;
- a coarse-to-fine tuple index can control broad partitions without rereading
  content.

**Low confidence/open:**

- behavior at million-record same-length partitions and longer Unicode names;
- independence and adversarial collision quality of the current polynomial
  bases/mixers;
- exact key/dictionary cost in the intended Go posting representation;
- semantic-view edit commutation and whether every view belongs in the core
  hash rather than an optional companion channel;
- the production SNR floor and overflow API.

The width caveat has an explicit witness. Descendants `ldfioi` and `kbmedf`
collide in the two-by-512 literal and fold tuple maps, and their all-letter
structural hashes also agree. Sources `ldfioia` and `kbmedfa` consequently form
a false full-profile candidate without a common exact deletion descendant.
This is why external exact authority and a wider/keyed production family remain
necessary even though E13's generated split showed zero excess.

## Revised K1

K1 should no longer ask whether Kolmogrov has a relevance model. It should ask
whether a proposed hash configuration freezes and demonstrates:

1. observation profile and declared transformation relation;
2. exact recall argument for that relation;
3. maximum input/history length and same-partition pool envelope;
4. coupled coordinate family, tuple width, and scale tower;
5. mutation contribution and invisibility gates;
6. posting, lookup, build, deletion, and migration resource bounds;
7. deterministic cross-platform configuration identity;
8. explicit candidate-only and overflow semantics.

On that definition, K1 is not blocked by absent ranking. It remains blocked for
production by external-scale capacity, hash-family audit, and concrete index
resource evidence. The object is ready to serve as an experimental hash
instrument now, under its measured envelope.
