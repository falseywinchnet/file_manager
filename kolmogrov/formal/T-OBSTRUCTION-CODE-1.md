# T-OBSTRUCTION-CODE-1: task-relative width from a protected-separation graph

Status: **HYPOTHESIS with complete finite paper proof; protected File Manager
obstructions not yet frozen**.

## Protected graph

Let `S` be a finite oracle state set. Put an edge `{x,y}` in graph `G` exactly
when the compact candidate code is forbidden to merge that pair. Edges may come
from required observations, flow-closure blocks, contradiction pairs, or named
hard negatives; their provenance remains separate.

A `B`-bit code `h:S->{0,1}^B` satisfies the protected separation contract when

```text
{x,y} in E(G) implies h(x) != h(y).
```

## Statement

Such a code exists exactly when

```text
chi(G) <= 2^B,
```

where `chi(G)` is the chromatic number. Therefore the minimum exact protected
width is

```text
B_min = ceil(log2 chi(G)).
```

## Proof

The codewords used by `h` are color names, and the separation condition is
precisely proper coloring. A proper coloring using at most `2^B` colors can be
injected into the available codewords, and every valid code induces such a
coloring. QED.

## Least width obstruction

The triangle on three oracle states has chromatic number three and requires two
bits, although three states fit inside a one-dimensional real coordinate. Any
graph on at most two states has chromatic number at most two and fits in one
bit. Thus the triangle is the least obstruction to a one-bit protected code.

## Robust distance extension

If every protected edge must have Hamming distance at least `delta`, define

```text
B_min(G,delta) = min B admitting such a binary labeling.
```

The coloring theorem is the `delta=1` case. For `delta>1`, width depends on both
the obstruction graph and the required margin; cardinality alone is
insufficient.

## Consequence

Hash width is not a function of content length alone. It is a certificate for a
declared protected-obstruction graph and, when required, a distance margin. A
new protected pair can raise the minimum width even when the corpus size and
object lengths do not change.

This theorem governs only pairs that may not collide. Perceptual positives,
ranking order, recall outside the protected graph, and random collision rates
remain separate contracts.

## Falsification gate

A claimed width is impossible when `chi(G)>2^B`, or when a declared protected
clique cannot be assigned the required Hamming-separated codewords. A width
claim is under-specified when it does not version the graph construction,
protected classes, and distance margin.
