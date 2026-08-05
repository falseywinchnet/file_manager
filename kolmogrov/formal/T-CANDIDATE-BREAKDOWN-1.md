# T-CANDIDATE-BREAKDOWN-1: exact attribution of certificate candidate excess

Status: **set decomposition PROVED; E13 instantiation MEASURED**.

## Nested candidate sets

Fix a query, source-length plan, exact observation view family, and deletion
radius. Define:

```text
T   exact declared descendant relation;
C   complete-schedule exact certificate acceptance;
B   bounded-schedule exact certificate acceptance;
PB  bounded-schedule projected certificate acceptance.
```

The certificate recall theorems and schedule omission give

```text
T subseteq C subseteq B subseteq PB.
```

Let `E` be the parent engine's exact comparison predicate, such as its chosen
transposition/edit tests. `E` need not equal `T`; it is an external consumer of
the hash candidate set.

## Disjoint breakdown

Every projected bounded candidate belongs to exactly one of:

```text
T intersection E         declared relation and parent exact predicate;
T \ E                    relation breadth beyond that parent predicate;
C \ T                    incompatible certificate histories;
B \ C                    omitted certificate directions;
PB \ B                   projection collision on retained directions.
```

The five sets are pairwise disjoint and their union is `PB`. This follows
immediately by telescoping the nested sets and splitting `T` by `E`.

Consequently a candidate tail can be attributed without a relevance judgment.
Only the final split `T intersection E` versus `T\E` depends on the parent
engine predicate. The other three excess classes are failures or deliberate
relaxations of the hash method itself.

## Capability consequence

- More projection cells can reduce only `PB\B`.
- More certificate directions can reduce `B\C` and some projected excess, but
  not `C\T`.
- Only history-coupled/global evidence can reduce `C\T` without using the full
  descendant relation.
- Changing ranking policy changes none of these sets.

This is why E12's 16-to-64-cell plateau points away from row-only work, but it
does not by itself identify which nonprojection class dominates.

## E13 instantiation

On the 216-query evaluation split, all nested-set assertions held. Aggregate
counts were 336 in `T intersection E`, 12 in `T\E`, zero in `C\T`, 696 in
`B\C`, and 323 in `PB\B`. The measured certificate residual was therefore
schedule omission plus projection collision, not complete-certificate
incompatible histories. The decomposition corrected the earlier causal guess
without invoking relevance.

## Boundary

The decomposition measures candidate mechanics, not result relevance. Parent
ngram, transposition, edit, or field-specific methods may consume `PB` in any
order while exact external records remain authoritative.
