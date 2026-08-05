# T-ORACLE-1: literal reconstruction for the v0 symbolic oracle

Status: **HYPOTHESIS with complete paper proof; independent review and formal
encoding absent**.

## Statement

Let `x = (x_0, ..., x_{n-1})` be any finite tuple of Unicode strings accepted by
`SymbolicCase`, with an exact nonempty external object anchor. Let `B(x)` be the
output of `exhaustive_breakdown` under any valid `OracleConfiguration`, including
a truncated diagnostic configuration. Then

```text
reconstruct_literal(B(x)) = x.
```

This theorem is about the explicit literal witness only. It makes no claim that
the similarity channels reconstruct `x`, preserve meaning, or fit a fixed
width.

## Definitions used

The encoder emits exactly one literal `length` feature whose key is `n`. For
each integer `i` in `[0,n)`, it emits exactly one literal `indexed_symbol`
feature whose key is `(i,x_i)`. Oracle truncation limits apply only to
contiguous and ordered-subsequence degrees; they do not alter literal features.

`reconstruct_literal` requires one integer length, equality between that length
and `Breakdown.literal_size`, and exactly the index set `{0,...,n-1}`. It returns
the associated symbols in increasing index order.

## Proof

Fix an accepted `x` of length `n`. The encoder's loop ranges once over each
index `i` in `[0,n)`, so it emits the pair `(i,x_i)` once and emits no
indexed-symbol pair outside that range. The accumulator key contains the entire
feature identity, including `i` and `x_i`; because each `i` is visited once,
aggregation cannot merge two different source indices.

The reconstructor reads the unique declared length `n`. Its indexed-symbol map
therefore has domain exactly `{0,...,n-1}` and maps every `i` to `x_i`. The
returned tuple is `(x_0,...,x_{n-1})`, which is `x`. The empty case has length
zero, an empty indexed-symbol map with the required empty domain, and returns
the empty tuple. Therefore the statement holds for every accepted finite
sequence. QED.

## Excluded failures

- A caller-manufactured malformed `Breakdown` is outside the encoder image and
  is rejected when it lacks, duplicates, or contradicts literal evidence.
- Canonical JSON serialization must preserve JSON string and integer values.
- This proof does not make the external object anchor derivable from the
  sequence; identity remains external by GIVEN K-G01.

## Executable support

`experiments/e0_oracle_bootstrap` exhaustively checks the binary alphabet
through length five. This finite check covers implementation agreement and is
not used as the universal proof.
