# History hash engine usage contract 001

Status: **SEALED CANDIDATE guidance for disabled engine dogfooding; not an ADR,
production profile, or cross-project ABI**.

Authority boundary: Orchestrator remains canonical for reconciled API/ABI
semantics. This document is Kolmogrov's complete reply to the framework's
request for efficient experimental-use defaults. Sibling projects may rely on
the stated candidate semantics while keeping the feature disabled and
disposable. They may not infer production acceptance.

Normative terms `MUST`, `MUST NOT`, `SHOULD`, and `MAY` apply to an adapter that
claims conformance to this experimental guidance.

## Purpose and non-purpose

The adapter produces candidates for a declared radius-one filename relation.
It is a hash instrument. It does not rank relevance, establish filesystem
identity, replace exact records, or implement the parent engine's ngram,
transposition, edit, or ranking policy.

An adapter MUST label every positive as `candidate_only`. It MAY pass raw hash
evidence to the parent. It MUST NOT label a candidate an exact relation match
without downstream exact adjudication, and MUST never use it as exact object
identity.

## Default dogfood profile

```text
family_id:              org.filemanager.kolmogrov.filename-history-tuple
family_version:         0.1.0-experimental.1
profile_id:             filename-literal-r1-q2-d20-n64-p65536
observation_profile:    org.filemanager.kolmogrov.filename-observation@0.1.0-research.1
required_view:          filename.literal-scalar
transformation_radius:  1 deletion
relation:               same-length common one-deletion descendant,
                        plus both exact directional one-deletion seams
modulus:                2^61-1
coordinate_count:       2
cell_bits/coordinate:   20
coupled_key_bits:       40
source_atom_length:     2..64
records/plan_partition: <= 65,536
candidate_return_cap:   4,096
exact_anchor:            required and external
availability:            available_experimental only
```

This width is a conservative dogfood option, not a selected minimum. The ideal
uniform/independent model gives, per maximum partition,

```text
M n^2 / 2^w <= 65536 * 64^2 / 2^40 = 2^-12.
```

Three queried length plans give a pessimistic union bound of `3*2^-12`. The
current polynomial family has no proven uniform/adversarial guarantee, so these
numbers are capacity diagnostics only. The explicit E13 18-bit collision is a
mandatory negative fixture even when this wider option is used.

## Coordinate configuration

An index generation MUST serialize the full coordinate parameters in its
configuration descriptor:

```text
modulus;
ordered base_j and multiplier_j pairs;
coordinate count and cell bits;
observation profile and Unicode version;
required semantic views;
minimum/maximum atom length;
relation radius and length plans;
pool and query budgets;
packing/byte rule if bytes cross a contract seam.
```

For dogfooding, each base MUST lie in `[2,P-1]`, each multiplier in `[1,P-1]`,
and coordinate pairs MUST be distinct. Parameters SHOULD be sampled once from
the operating system random source for an index generation and then stored as
explicit descriptor values. They are collision randomization, not secrets or a
cryptographic guarantee. Query and stored segment parameters MUST agree.

The fixed E13 bases/mixers MAY be used for reproducible reference work but MUST
NOT be the dogfood default; their retained short collision is public.

Any parameter, profile, Unicode-version, length, or packing change creates a
new incompatible configuration. The adapter MUST return
`configuration_mismatch` or `rebuild_required`; it MUST NOT reinterpret old
keys in place.

## Required observation behavior

The literal-scalar stream is the only required filtering view. It defines the
current recall theorem's source relation.

Folded and structural streams MAY be emitted as independently identified
companion evidence. They MUST NOT be mandatory candidate filters under this
profile: normalization can change atom count and deletion alignment, and a
general commutation theorem is absent. A future profile may require them only
after its admitted Unicode domain and recall proof are frozen.

Inputs outside 2--64 literal atoms MUST return `unsupported_input` and invoke
the parent's declared fallback. Truncation, suffix-only hashing, or silent
scale reduction is forbidden because each changes the relation.

## Build directive

For each literal atom stream `a_0,...,a_(n-1)`, the adapter MUST compute one
coupled coordinate tuple for each complete one-deletion descendant. Coordinates
from one history MUST remain one key.

```text
INITIAL ITERATION:   q*n atom-code reads
HISTORY ITERATION:   n coupled tuple emissions
ROLLING ITERATION:   q*(n-1) recurrence updates
SUBSET ITERATION:    none
DESCENDANT COPIES:   none
```

The recurrence is:

```text
D_0       = sum_(i=1)^(n-1) a_i b^(i-1) mod P
D_(h+1)   = D_h + (a_h-a_(h+1)) b^h mod P.
```

The adapter MUST also compute the full-sequence tuple. Duplicate history tuples
MUST be removed before posting publication. Repeated symbols can otherwise
create redundant memberships without adding capability.

Recommended private hot-path layout:

- precompute powers through atom 63 per coordinate/configuration;
- update both coordinates in one atom/history pass;
- pack two 20-bit cells into one `uint64`, coordinate zero in low bits;
- retain the upper 24 bits zero for the private experimental layout;
- sort/deduplicate at most 64 keys in bounded scratch storage;
- batch posting publication by generation, length, key kind, and tuple key;
- perform no hot-path allocation proportional to the corpus or posting count.

The packing recommendation is not a cross-project byte ABI. If tuple bytes
cross the Orchestrator seam, its reconciled descriptor owns byte order.

## Posting namespaces

The engine SHOULD expose two immutable logical posting families:

```text
history:
  (configuration, generation, source_length, semantic_view, tuple_key)

full:
  (configuration, generation, target_plan_length, semantic_view, tuple_key)
```

Every posting resolves to exact engine ordinals. Segment liveness MUST filter
deleted/superseded ordinals. Dead posting mass MAY remain until compaction, but
must be measured separately from live candidate work.

One record publishes at most `n` distinct history keys plus one full key in the
required literal view. With the default length bound this is at most 65 logical
memberships before duplicate collapse. Optional companion views are charged
separately and cannot be described as free support.

## Query plans

For literal query length `n`, query only the three typed plans:

```text
stored length n:
  build <=n query history tuples;
  union matching history postings.

stored length n+1:
  build one query full tuple;
  read matching history posting.

stored length n-1:
  build <=n query history tuples;
  union matching stored-full postings.
```

The three plan results are unioned and deduplicated by exact engine ordinal.
The live-ordinal filter is applied before return. Parent evidence or exact
adjudication follows candidate production.

The default literal-only probe ceiling is `2n+1`, at most 129 posting probes.
The adapter SHOULD execute the one-probe directional plan first, then order
remaining posting reads by available cardinality metadata. It MAY short-circuit
once a stricter caller budget is exhausted, but partial candidates MUST return
`budget_exceeded`, never successful-complete status.

No universal precombined table is allowed: arbitrary occupancy unions require
exponential predicate support. Contiguous keys, batched posting reads,
cardinality ordering, bounded scratch, and early empty/intersection exits are
the admitted optimizations.

## Capacity and overflow

The engine MUST maintain the live record count for every queried
`(configuration,generation,source_length,key-kind)` partition. Above 65,536
records, the default capacity statement no longer applies. The engine MUST do
one of:

1. return `over_capacity` and use the parent fallback;
2. rebuild that partition with a wider declared tuple profile;
3. route through a parent-owned prepartition and explicitly relinquish global
   Kolmogrov recall outside the supplied pool.

It MUST NOT silently continue advertising the default capacity envelope.

Candidate fanout above 4,096, posting-work exhaustion, cancellation, or response
budget exhaustion returns the corresponding terminal status and activates the
parent fallback. An unavailable or failed hash is not an empty successful
candidate set.

## Coarse/fine scale

Coordinate cells use

```text
c_B(y)=floor(yB/P).
```

For `B_f=rB_c`, `c_Bc(y)=floor(c_Bf(y)/r)`. A stored fine key can therefore
derive a coarse key componentwise without content access. The engine MAY use
this tower for prefix/range routing only if its private key order makes the
coarse child range economical. It MUST NOT promise that a coarse key can
reconstruct fine detail.

The default stores and queries the fine 20-bit coordinates directly. Coarse
routing is an optional measured optimization, not required behavior.

## Terminal statuses

The adapter MUST distinguish:

```text
available_experimental
unavailable
unsupported_input
over_capacity
configuration_mismatch
rebuild_required
budget_exceeded
cancelled
corrupt_projection
internal_failure
```

Continuations bind to engine generation, complete configuration descriptor,
normalized query plan, and caller limits. A generation/configuration change
invalidates the continuation.

## Evidence returned to siblings

The experimental result SHOULD expose:

```text
configuration identity;
queried length plan;
candidate exact ordinal/anchor;
matched literal tuple key or keys;
posting/probe count;
live/dead filtering count;
capacity and budget state;
candidate_only=true.
```

It MUST NOT expose a fabricated relevance score. Parent systems may attach
their own ngram, edit, transposition, or ranking evidence under separate IDs.

## Promotion exclusions

This guidance does not select:

- a production hash width or coordinate family;
- folded/structural filtering;
- path hashing or content hashing;
- radius greater than one;
- relevance or ranking policy;
- posting codec, segment format, or stable tuple ABI;
- a deterministic collision-free input length;
- migration other than erase/rebuild.

The open concerns in `HISTORY_HASH_OPEN_CONCERNS_001.md` are normative context
for every sibling implementation plan. Omitting them from a local plan does not
close them.
