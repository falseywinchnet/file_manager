# T-GUARD-COVERAGE-1: independently addressable protection and retrieval

Status: **HYPOTHESIS with complete finite recall/cost proof and least
provenance obstruction; empirical allocation unselected**.

## Two address roles

For each exact certificate pattern `p`, let

```text
g(p) in G    protected guard cell,
c(p) in C    ambient coverage cell.
```

The guard is compiled against named mutation differences. The coverage address
is selected against candidate tails and unseen-direction attacks. Their dense
support per certificate is additive:

```text
|G| + |C|,
```

not the product `|G||C|` required by the coupled pair cell `(g(p),c(p))`.

## Recall law

If two sources share an exact descendant pattern `p` in a certificate, their
guard masks both contain `g(p)` and their coverage masks both contain `c(p)`.
Therefore any of the following retains the common-descendant candidate:

1. coverage overlap alone;
2. guard overlap alone;
3. requiring both overlaps separately;
4. overlap of the coupled pair cell.

Projection can still only add candidates. Guard evidence used only after
coverage generation cannot cause a candidate false negative.

## Separate overlap is not coupled witness evidence

Requiring guard and coverage overlaps separately does not prove that one exact
pattern supplied both. The least obstruction needs two patterns on each side.
Choose `a,b` in the left set and `u,v` in the right set such that

```text
g(a)=g(u),       c(b)=c(v),
(g(p),c(p)) != (g(q),c(q)) for every left p and right q.
```

The guard intersection is witnessed by `a,u`; the coverage intersection is
witnessed by `b,v`; the coupled pair intersection is empty. One pattern per
side cannot fail because both separate equalities would apply to that sole
pair. Thus two is least.

Consequently the additive support route trades exact common-witness provenance
for smaller dense support. It must expose this residual and end in exact
verification.

## Legitimate compositions

Three compositions have honest semantics:

- **coverage generator + guard evidence:** coverage postings generate the
  candidate; guard overlap/energy explains or ranks it after retrieval;
- **separate guard-and-coverage intersection:** a stricter recall-safe front
  index with an explicit provenance-mixing residual;
- **coupled guard/coverage cell:** common projected witness, at multiplicative
  dense support.

Calling the second form “joint pattern evidence” is rejected.

## Support comparison

For binary depths `d_g,d_c`, separate masks cost

```text
2^d_g + 2^d_c
```

bits per certificate, while the coupled concatenated cell costs

```text
2^(d_g+d_c).
```

At equal 16-bit dense support, two independently addressable three-bit cells
cost `8+8=16`; their fully coupled six-bit cell costs 64. This is the concrete
support/residual trade tested in E12.

## Boundary

The theorem does not decide whether guard evidence should veto, rank, explain,
or merely audit a candidate. A veto requires a product policy proving that the
guard's zero response is never an intended invariance. Until then coverage owns
recall and exact records own the final decision.
