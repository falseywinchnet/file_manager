# Filename observation profile 001: source-anchored scalar views

Status: **CANDIDATE research feature policy; not an accepted File Manager or
ORC-KOL production profile**.

## Purpose

This profile closes one experimental input boundary without claiming that one
normalization is universally perceptual. It accepts a filename component that
the caller has already decoded into Unicode scalar values and emits three
independently addressable, one-atom-per-source-scalar streams.

Exact filesystem identity, native path bytes, platform decoding, separator
validity, case-sensitive identity, and record authority remain outside
Kolmogrov.

```text
profile_id:
org.filemanager.kolmogrov.filename-observation@0.1.0-research.1
```

The descriptor includes the Unicode Character Database version. Different
versions are incompatible projection identities and require rebuild.

## Input contract

- nonempty decoded string;
- every element is a Unicode scalar, excluding surrogate code points;
- NUL is unsupported;
- the string is treated as one filename component, not parsed as a path;
- no whole-string normalization is applied before the views split.

The profile deliberately does not reinterpret `/`, `\`, `:`, or another
platform-specific character as a separator. The caller owns path structure.

## Independent streams

### `filename.literal-scalar`

One atom `scalar:hhhhhh` for each exact scalar value. This view is lossless for
the accepted decoded string and declares no case, compatibility, accent, or
script invariance.

### `filename.anchored-nfkc-casefold`

For each source scalar independently:

```text
NFKC(scalar) -> full casefold -> NFKC
```

The resulting UTF-8 bytes form one tagged atom. The scalar-to-atom count remains
one-to-one even when the capsule contains several normalized scalars. This is
not whole-string NFKC case folding: cross-scalar composition is deliberately
excluded so every output atom retains the exact source span `[i,i+1)`.

Intended invariance is limited to substitutions whose two per-scalar capsules
are equal. Canonically composed versus combining sequences may remain different;
that residual is exposed rather than hidden.

### `filename.structural-class`

One coarse role per source scalar:

```text
letter, mark, number, dot, separator, punctuation,
symbol, control, or other
```

Dot is kept separate because extension-boundary evidence is a protected File
Manager candidate. Underscore, hyphen, and whitespace share `separator`.
Letter case and literal identity are intentionally absent. This view proposes
shape candidates only and cannot veto the literal/folded channels.

## Source anchoring and Jacobian

Every stream has exactly the input length and every atom carries source span
`[i,i+1)`. A one-scalar substitution therefore changes at most one observed
atom per stream before certificate expansion. This constraint was chosen to
make the source-symbol Jacobian exact and inspectable.

Whole-string normalization, grapheme segmentation, tokenization, and phonetic
rewriting can move or merge source spans. They remain separate future views
whose nonlocal Jacobians must be declared rather than smuggled into this one.

## Transformation dispositions

| Change | Literal | Anchored fold | Structural |
|---|---|---|---|
| exact scalar replacement | sensitive | sensitive unless capsules equal | sensitive unless roles equal |
| case-only replacement | sensitive | intended invariant where capsules agree | invariant within `letter` |
| compatibility replacement | sensitive | intended invariant where capsules agree | role-dependent |
| digit replacement | sensitive | sensitive | invariant within `number`; protected by other views |
| extension replacement | sensitive | sensitive modulo fold | usually invariant within `letter`; dot boundary retained |
| composed/combining rewrite | sensitive | not promised invariant | length/mark structure sensitive |
| insertion/deletion | length and position change | length and position change | length and position change |

No row declares that two names are semantically equivalent. It declares only
which observation changed. Retrieval labels remain a separate workload object.

## Atom coding boundary

The profile freezes exact pre-projection codes without hashing:

```text
literal scalar:    21-bit Unicode scalar value
structural role:    4-bit index over the nine frozen roles
fold capsule:     270 bits = 6-bit byte length + 33 zero-padded UTF-8 bytes
```

An exhaustive Unicode 16.0.0 scalar audit found maximum anchored-fold payload
length 33 bytes, attained by `U+FDFA`. A different Unicode version must repeat
that finite bound audit and receives a different descriptor. The reference
rejects a capsule exceeding the frozen bound rather than truncating it.

The exact code can be large because it is not the stored hash. Progressive bit
routes, affine rows, seeds, and collision rescue are separate projection
configuration and remain audited through T-PROJECTION-REFINEMENT-1 and
T-SYMBOL-JACOBIAN-1. No opaque digest collision precedes the support ledger.

## Exclusions and next gate

The profile does not select path-segment streams, tokens, length bands,
certificate radii, projection rows, prevalence, or channel fusion. Its next gate
is a generated filename transformation atlas with disjoint development/test
families, followed by an exact per-view mutation-flow audit. Only then may one
view enter an experimental ORC-KOL configuration proposal.
