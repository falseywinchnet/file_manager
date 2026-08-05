# T-DELETE-1: exact single-deletion witness law

Status: **HYPOTHESIS with complete paper proof; independent review and formal
encoding absent**.

## Statement

Let `x` have length `n >= 1`, and let `y` be obtained by deleting source index
`r`. For every degree `k` in `[1,n]`:

1. exactly `C(n-1,k)` degree-`k` occurrence witnesses survive;
2. exactly `C(n-1,k-1)` are destroyed;
3. every survivor preserves its content pattern and order;
4. every survivor's source and target gap vectors differ by `+1` in exactly one
   source coordinate and zero elsewhere, hence gap `L1` distance is exactly one;
5. for every pattern `p`, its target multiplicity is no greater than its source
   multiplicity;
6. total pattern-multiplicity excess is `C(n-1,k-1)`;
7. the fraction of source occurrence mass destroyed is exactly `k/n`.

When `k <= n-1`, exact occurrence-mass containment of `y` in `x` is one.

## Proof

A source occurrence survives exactly when its `k` selected indices avoid `r`.
Choosing all `k` indices from the other `n-1` positions gives `C(n-1,k)`
survivors. A destroyed occurrence includes `r` and chooses its remaining
`k-1` indices from the other positions, giving `C(n-1,k-1)`.

Deletion preserves the relative order and symbols at every surviving index.
The deleted position is unselected for such an occurrence, so it lies in
exactly one of the `k+1` gap regions: before, between, or after selected
indices. Deleting it subtracts one from exactly that gap and leaves all other
gaps unchanged. Thus the lifted source gap vector minus the target vector is a
standard basis vector and their `L1` distance is one.

Every target occurrence lifts uniquely to the source occurrence obtained by
adding one to indices at or after `r`; therefore target pattern occurrences
inject into source pattern occurrences. This proves coordinatewise
multiplicity containment. Summing source minus target multiplicities gives the
difference in total occurrence counts:

```text
C(n,k) - C(n-1,k) = C(n-1,k-1).
```

Finally,

```text
C(n-1,k-1) / C(n,k) = k/n.
```

For `k <= n-1`, the target has positive degree-`k` mass, all of which occurs in
the source, so directional occurrence-mass containment is one. QED.

## Scope

This law counts occurrence multiplicity, not merely distinct patterns. It is
exact for one deletion and does not yet state bounds for substitution,
transposition, block motion, multiple edits, projected gaps, or compact hashes.
