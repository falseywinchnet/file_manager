# E13 result: coherence belongs in the key, and capacity belongs to the contract

Status: **MEASURED on three generated splits of 288 records and 216 queries;
not a production or corpus-general guarantee**.

## Certificate breakdown

The evaluation candidate set of the E12 frozen control partitioned with no
nesting failures:

| Class | Aggregate | Per-query p95 | Meaning |
|---|---:|---:|---|
| declared relation and parent exact predicate | 336 | 6 | intended hash relation also accepted downstream |
| declared relation beyond parent exact predicate | 12 | 1 | relation breadth, not a hash false positive |
| complete-certificate incompatible history | 0 | 0 | absent on this workload |
| bounded-schedule omission | 696 | 7 | dominant certificate relaxation |
| projection collision | 323 | 8 | retained projection error |

This corrects the E12 diagnosis. Its tail was not evidence of complete
certificate history incompatibility on these cases. The measured causes were
the eight-certificate omission and projection collision. Ranking changes none
of these sets.

## Least dense-support obstruction

At the E12 control's 384 total dense cells, separable whole-history occupancy
was saturated. Evaluation p95 candidates ranged from 174 at one 128-cell
address per semantic view to 193 at four 32-cell addresses. Splitting fixed
support made the premise smaller while each name still contributed about 18
histories. The relevant pair opportunity is quadratic in history count.

Increasing separable support helped, but independent address acceptance still
allowed a different history pair to witness each coordinate. Two 2,048-cell
addresses reduced evaluation excess to one p95 and one maximum. Two 8,192-cell
addresses happened to remove all measured excess, at 93.29 memberships/record.

## Coupled history tuples

The coupled construction publishes one tuple per deletion history. It therefore
uses about `n`, rather than `q*n`, posting memberships and cannot combine
coordinate witnesses from different histories.

Selected points from the all-split sweep are:

| Tuple | Key bits/view | Worst split maximum excess | Evaluation p95 excess |
|---|---:|---:|---:|
| 1 x 256 | 8 | 172 | 111 |
| 2 x 64 | 12 | 7 | 3 |
| 2 x 128 | 14 | 2 | 1 |
| 3 x 32 | 15 | 1 | 0 |
| 2 x 256 | 16 | 1 | 0 |
| 2 x 512 | 18 | 0 | 0 |
| 3 x 64 | 18 | 0 | 0 |

Two 8-bit coordinates had exactly one all-split obstruction:

```text
tuning insertion query: "tyopaz cache 16.cfg"
nonrelation record:      "sable signal 76.ctg"
```

Two 9-bit coordinates were the least tested equal-coordinate configuration
with zero excess on all 648 queries. This is a measured crossing only. It does
not prove that 18 bits supports arbitrary names or larger candidate pools.

The deterministic obstruction audit makes that caveat concrete. Over the
16-letter alphabet `abcdefghijklmnop`, the combined literal/fold two-coordinate
map had no direct descendant-key collision through length five. At length six:

```text
ldfioi -> literal (258,216), fold (100,319)
kbmedf -> literal (258,216), fold (100,319)
```

Both descendants are structurally all-letter. Lifting them to `ldfioia` and
`kbmedfa` yields two length-seven sources with no common exact deletion
descendant, yet the full three-view two-by-512 tuple hash accepts them. Thus
18 bits/view is already deterministically fallible below the generated
workload's name length; zero generated excess is distribution-specific.

At that crossing, memberships/record were 44.83 development, 47.67 tuning,
and 46.67 evaluation. E12's certificate control used 83.21 on evaluation.
Hybrid posting payload was 95.37 bytes/record versus 49.42 for the certificate
control because the more selective tuple keys shared shorter posting lists;
address dictionary and key bytes remain outside that payload measure.

## Contribution energy

Across the 108 same-length nonexact evaluation queries, no tested separable
configuration made the mutation wholly invisible. At two 8,192-cell addresses,
changed occupied cells were 128 p50 and 144 p95 across the semantic views.
Case changes affected only the literal channel and therefore changed two of six
addresses; this is expected profile behavior, not loss.

The measurement does not establish a deterministic minimum influence. Any
finite projection admits collision obstructions. The useful boundary is the
declared minimum changed-cell threshold together with a maximum content length
and candidate-pool size.

## Algorithmic consequence

For `q` coordinates and source length `n`, the active builder performs `q*n`
atom-code reads and `q*(n-1)` rolling fingerprint updates per semantic view.
It never constructs descendant strings and never iterates position subsets.
Same-length lookup enumerates at most `n` coupled tuple postings per semantic
view; a shorter full-sequence query uses one tuple probe. Multiple semantic
views are intersected as independent declared channels.

The candidate is therefore a useful hash instrument if its contract freezes:
the observation profile, transformation relation, maximum input length,
coordinate family, tuple width, cell scale, and measured pool envelope. Parent
engine ngrams, transposition tests, exact record identity, and ranking remain
external.
