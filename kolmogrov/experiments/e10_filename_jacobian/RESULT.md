# E10 result: filename atoms close; low-bit projection fails; schedule rescue remains

Status: **MEASURED on seven named synthetic substitutions under Unicode 16.0.0;
not a relevance, held-out quality, or native-latency result**.

## Observation profile

Filename Profile 001 emitted three one-atom-per-scalar views for each
seven-scalar name. The named cases behaved as declared before projection:

- `A -> a` and full-width `Ａ -> A` changed literal atoms but were invariant in
  anchored fold and structural-letter views;
- letter, digit, extension-letter, and accent changes remained visible in
  literal atoms;
- letter, digit, dot, extension, and accent changes remained visible in the
  anchored fold except the two intended case/compatibility invariances;
- only dot-to-underscore changed the structural view; same-role literal changes
  were deliberately invariant there.

The profile therefore separates observation invariance from compression loss.
A zero structural response to `1 -> 2` is not a projection failure; a zero
literal/fold response is.

## Exact source-symbol flow

The complete radius-two/degree-three schedule contained ten certificates and
ten monotone offset states per certificate. Depending on source position, a
single changed scalar reached six, nine, or ten certificates and 36–46 offset
states. Observation-changing views produced 48–86 exact removed/added pattern
events.

These totals verify the distinction in T-SYMBOL-JACOBIAN-1:

```text
incident certificate
  != changed exact pattern set
  != changed projected bit
```

## Low-bit affine failure

The first diagnostic projected exact atom integers by `[1,3,5] mod 16`. Fold
codes store their byte length in the low six bits, so nearly every one-byte
capsule shared the same low residue. It produced zero total folded-view bit
change for letter, digit, dot, and extension substitutions despite 66–86 exact
pattern events. Literal `A -> a` and `Ａ -> A` also vanished modulo 16.

This is a retained construction failure: exact atom codes do not justify using
their low bits as a perceptual prefix.

## Balanced-bit control

A four-row binary control assigned every input bit a nonzero output column
before constructing the same 16-cell occupancy mask. Total bit response for
every observation-changing case became positive:

| Case | Literal energy | Anchored-fold energy | Structural energy |
|---|---:|---:|---:|
| case only | 36 | 0 intended | 0 intended |
| compatibility width | 37 | 0 intended | 0 intended |
| letter identity | 58 | 49 | 0 intended |
| digit identity | 46 | 31 | 0 intended |
| dot boundary | 49 | 48 | 28 |
| extension letter | 27 | 38 | 0 intended |
| accent removal | 45 | 36 | 0 intended |

The schedule rescued remaining per-certificate collisions: case-only literal
and extension-literal each had one exact-changing certificate with zero bit
response; digit-fold had two. Other certificates responded, so total schedule
energy remained positive. This is evidence for complementary certificate
directions, not permission to ignore the zero cells.

The balanced rows are a deterministic diagnostic, not obstruction-selected or
held out. They prove only that touching the payload bits repairs the low-lane
failure on these cases.

## Posting support

Each layout used 160 dense mask bits/object and ten query lookups per view. The
balanced layout produced these posting memberships per object:

| View | Mean entries | Range | Mean exact pattern incidences |
|---|---:|---:|---:|
| literal | 79.22 | 70–84 | 94.67 |
| anchored fold | 78.89 | 68–85 | 94.67 |
| structural | 42.22 | 42–44 | 49.00 |

The failed low-bit fold used only 10.67 entries/object because it collapsed
almost everything. Fewer postings can therefore mean catastrophic support loss,
not efficiency. Entry count, private influence, and candidate quality must be
reported together.

These are logical posting memberships, not encoded bytes or cache work.

## Unicode bound

An exhaustive scan of Unicode 16.0.0 scalar values found maximum anchored-fold
UTF-8 payload length 33 bytes, uniquely attained by `U+FDFA`. Filename Profile
001 consequently has an exact 270-bit fold atom code with no preliminary digest
collision. A Unicode-version change requires a new bound and descriptor.

## Remaining gate

The profile and same-length substitution Jacobian are now concrete. K1 remains
blocked by:

- a disjoint generated filename workload and protected mutation labels;
- obstruction compilation over the exact 21/270/4-bit atom lanes;
- length-conditioned insertion/deletion semantics;
- posting encoding bytes and intersection work;
- selected length bands, schedule sizes, and SNR gates.
