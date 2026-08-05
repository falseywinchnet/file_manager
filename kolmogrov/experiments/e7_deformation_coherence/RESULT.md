# E7 deformation coherence and algorithm-collapse result

Status: **MEASURED on the declared exhaustive binary corpus and Python timing
environment; compact hash not selected**.

Date: 2026-08-05.

Environment: Darwin arm64, Python 3.14.0.

## Coherence measurement

The exact common-history predicate was compared with unlabelled degree-one
positional marginals through binary source length eight and deletion radii one
and two.

- Least unlabelled failure: source `aba`, query `bb`, radius one. Each target
  position is supported by a different deletion; no deletion produces `bb`.
- At length eight/radius one, exact descendants numbered 1,152 while unlabelled
  marginals accepted 4,374 pairs, including 3,222 false candidates.
- Exact pairwise history intersection removed every radius-one marginal false
  candidate through length eight. T-PAIRWISE-HISTORY-1 now proves this for all
  one-deletion source/query pairs because the support sets are intervals.
- Pairwise intersection is not sufficient at radius two. The least failure is
  source `ababa`, query `bbb`: descendants `bba`, `bab`, and `abb` support every
  query-position pair, but no history supports all three. At length eight/radius
  two, pairwise coherence retained 442 false candidates beyond 2,368 exact
  descendants.

For source length eight, positional moment modulus eleven:

- radius one has 8 histories and 11 full characters. Eight greedily selected
  characters reached observed pair margin `-0.039735`; all 11 characters gave
  exact coherence and margin `1`;
- radius two has 28 histories and 121 full characters. Thirty-two selected
  characters still had observed pair margin `-3.302667`; all 121 gave exact
  coherence and margin `1`.

Small selected character families therefore do **not** yet support a universal
degree-one coherence claim, especially at radius two. Full character coherence
works exactly but is too large to be the intended compact hash.

## Algorithm equivalence and scaling

Descendant enumeration, occurrence-product evaluation, and the streaming
degree/radius recurrence produced identical modular address coefficients on all
benchmark cases. Named loop bodies and median build times were:

| Length | Oracle iterations | Product iterations | Streaming cells | Oracle range | Product range | Streaming range |
|---:|---:|---:|---:|---:|---:|---:|
| 8 | 933 | 840 | 77 | 1.491–2.692 ms | 0.144–0.173 ms | 15.816–16.944 us |
| 12 | 10,199 | 5,940 | 125 | 17.414–18.943 ms | 0.955–1.050 ms | 26.023–26.776 us |
| 16 | 51,657 | 21,840 | 173 | 91.971–144.690 ms | 3.549–3.561 ms | 36.968–37.631 us |
| 20 | 175,771 | 58,140 | 221 | 319.279–320.040 ms | 9.654–9.930 ms | 48.685–50.007 us |

The streaming path collapses both combinatorial loops into one object scan over
small degree/radius state. At length twenty it was 6,385–6,574 times faster than
the descendant oracle and 198.3–198.6 times faster than occurrence products
across two consecutive Python timing runs.

## Interpretation boundary

The timing establishes the reference algorithm's iteration shape, not native
production latency. It uses one modular probe and excludes quantization,
indexing, candidate scoring, and exact verification.

The coherence result says where refinement must act: reduce incompatible
history multiplicity before or during projection. The immediate candidates are
higher-degree occurrence atoms, history equivalence by identical descendant,
and selected phase families optimized against declared protected differences.
Increasing character count without changing that structure is not accepted as
the only repair.
