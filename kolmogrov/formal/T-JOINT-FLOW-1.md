# T-JOINT-FLOW-1: the least failure of degree-two occurrence-flow closure

Status: **HYPOTHESIS with complete paper proof; independent review absent**.

## Observation

For a binary string `x`, let `O_2(x)` contain its length and the multiplicities
of every ordered subsequence pattern of degrees one and two. Let the visible
actions be three edit types, with edit location hidden:

- delete one uniformly random position;
- flip one uniformly random position;
- transpose one uniformly random adjacent pair.

The empty or too-short cases act as identity where the edit has no valid
location.

## Closed coarse control

The weaker observation `O_1=(length,Hamming weight)` is strongly lumpable for
all three averaged actions. From length `n` and weight `w`, deletion and flip
probabilities depend only on `n,w`; transposition never changes them.

Thus averaged edit type alone does not force literal support.

## Least obstruction for `O_2`

The two length-four strings

```text
x = 0110
y = 1001
```

have the same degree-one counts and the same degree-two counts:

```text
00:1, 01:2, 10:2, 11:1.
```

Their uniformly deleted target multisets are

```text
D(x) = {110, 010, 010, 011}
D(y) = {001, 101, 101, 100}.
```

`O_2` is injective on length-three binary strings, so these disjoint target
multisets induce different distributions over `O_2` cells. Therefore the
`O_2` partition is not lumpable even when deletion position is hidden. The
joint edit family fails closure because one of its actions fails closure.

## Minimality

No shorter obstruction exists:

- lengths zero and one are literal from degree one;
- at length two, the unique degree-two occurrence is the whole string;
- at length three, strings of weight zero or three are trivial. With one `1`,
  its position is distinguished by degree-two counts `10:2`,
  `01:1,10:1`, or `01:2`; the weight-two cases follow by exchanging symbols.

Hence `O_2` is injective through length three. At length four, weights zero,
one, three, and four are reconstructible as above. For weight two, the six
strings have `01`/`10` pair counts respectively

```text
0011:4/0, 0101:3/1, 0110:2/2,
1001:2/2, 1010:1/3, 1100:0/4.
```

Thus its only non-singleton binary cell is `{0110,1001}`. Any exact closed
refinement must split that cell, and doing so makes every cell through length
four literal and therefore closed. One additional binary distinction is
necessary and sufficient on this domain.

## Restricted theorem

Low-degree occurrence counts can be a closed state for some averaged edit
contracts, but increasing occurrence degree does not monotonically preserve
closure. A finer observation can expose hidden transition differences and
thereby require more state support.

## Falsification gate

For a proposed finite occurrence state, compute the coarsest flow-closed
refinement. The proposal is insufficient if a same-cell pair has different
next-cell laws; it uses excess exact support if it strictly refines the
coarsest closed partition without reducing another declared loss.
