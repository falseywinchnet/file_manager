# Phase A symbolic oracle v0

Status: **CANDIDATE definition with an OBSERVED reference implementation**.

## Domain and scope

The v0 object domain is a finite sequence of Unicode strings. A string is an
atomic symbol at this layer; Unicode normalization, grapheme segmentation,
bytes, tokens, paths, and modality-specific observations remain separate future
views. Exact object IDs remain external anchors.

The default oracle emits an uncompressed, canonical sparse breakdown. It is
deliberately exponential because it enumerates every ordered subsequence. It is
only a correctness oracle for short objects. Configuration limits may truncate
contiguous or subsequence degree, but the resulting metadata says `truncated`
and that configuration is not the perfect breakdown.

## v0 channels

- `literal`: exact length and every `(index, symbol)` pair. This is the explicit
  reconstruction witness for the initial form of T-ORACLE-1.
- `content`: symbol multiplicity and every contiguous fragment, with every
  occurrence anchored to an index or half-open span.
- `position`: exact absolute and rational `(index, length)` content-position
  crosses. Dyadic soft position remains unimplemented.
- `combination`: every contiguous path and every global ordered subsequence of
  degree two through object length. Each occurrence retains its index tuple.
- `shape`: exact length and unique-symbol count controls.

Hierarchy, prevalence, context, repetition/periodicity, directional
containment scores, normalization, and channel geometry are unresolved. The v0
breakdown does not claim perceptual sufficiency.

## Canonicalization

Feature identities use canonical compact JSON. Features are aggregated by
channel, identity, resolution, and direction; their values are occurrence
counts or declared scalar magnitudes, and the ordered anchor list retains every
source occurrence. The final tuple has a deterministic total ordering.

Transformation lineage is canonical JSON embedded in breakdown metadata. The
implemented edit constructors are substitution, insertion, deletion, and
adjacent transposition. Each step names its exact source object and sorted
parameters.

## Initial theorem target

**T-ORACLE-1-v0 (HYPOTHESIS pending paper proof):** for every finite symbolic
sequence accepted by this definition, `reconstruct_literal(exhaustive_breakdown(x))
= x`.

The executable checker covers all binary sequences through length five in the
bootstrap experiment. That finite check is not a universal proof. The literal
channel makes the paper argument direct; the next formal round must state the
canonical representation assumptions and prove uniqueness of length/index
evidence.

## First retained counterexample

The exhaustive collision search rejects symbol multiplicity as a complete
sequence representation: over alphabet `(a, b)`, the first length-then-lexical
collision is `(a, b)` versus `(b, a)`. This establishes only the declared
finite enumeration result and the elementary general failure of order erasure;
it does not establish that the current combination channel is sufficient for
perceptual retrieval.
