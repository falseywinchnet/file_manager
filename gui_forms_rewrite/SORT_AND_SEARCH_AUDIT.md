# Sorting and binary-search audit

Status: **MEASURED on the M4 Release build; one bounded house-sort migration,
all production binary-search-family calls migrated, negative results retained**.

The laboratory implements the architect-selected candidates in
`include/gui_forms/detail/algorithm/sort.hpp`:

- stable insertion sort;
- natural-run adaptive stable merge sort;
- explicit-stack introsort with a private heap fallback at its depth limit.

`std::sort` and `std::stable_sort` are the controls. Candidate existence is not
admission: a production call site changes only when its actual contract and the
measured workload support the choice.

## Production call-site decisions

| Site | Data/contract | Selection | Evidence and reason |
|---|---|---|---|
| `error_provider.cpp` | UTF-8 stable IDs for snapshot determinism | keep `std::sort` | The 32-element string corpus favored the standard control, 339.16 ns/sort versus 656.06 insertion and 398.90 house introsort. |
| `list_box.cpp` duplicate-ID validation | growable string IDs | keep `std::sort` | String movement/comparison favored the standard control; no bounded small-size contract exists. |
| `list_box.cpp` selected indexes | growable integer selection, often plausibly ordered | keep `std::sort` | Insertion won the synthetic nearly ordered 16/64 cases, but lost the random-64 case by 2.5x. No production distribution was measured, so a data-dependent cliff is not admitted. |
| `property_grid.cpp` visible descriptors | growable descriptor list and named comparator | keep `std::sort` | Comparator/key costs and actual rebuild distributions are not measured; no house winner is established. |
| `correspondence_view.cpp` expanded indexes | at most three roles: pinned, hover-expanded, focused | use house stable insertion | The maximum size is an exact code contract. At size three insertion measured 1.15 ns/sort versus 2.14 for `std::sort`, with equivalent output and no auxiliary allocation. |
| `text_store.cpp` style spans | growable stateful spans, normalization path | keep `std::sort` | Actual order/overlap distribution is not measured; no specialized winner is established. |
| `damage_region.cpp` x edges | duplicate-heavy numeric edges, frame-sensitive | keep `std::sort` | Insertion won the synthetic duplicate-128 corpus, but on random 512 edges it was 18.51 us versus 1.91 us for the standard control. Actual damage distributions are not yet sufficient to choose safely. |
| `damage_region.cpp` y intervals | repeated growable numeric pairs | keep `std::sort` | Same data-dependent risk as x edges; algorithm replacement would precede evidence. |
| `harfbuzz_font_engine.cpp` face tiers | stable registration order within preference tiers | keep `std::stable_sort` | Standard stable sort won: 16.52 ns versus 18.17 insertion and 28.57 adaptive. |
| `control.cpp` two tab/mnemonic traversals | stable equal-tab insertion order | keep `std::stable_sort` | Standard control won the stable size-32 corpus, 63.28 ns versus 75.65 insertion and 139.89 adaptive. |
| `window.cpp` two tab/mnemonic traversals | stable retained-child order | keep `std::stable_sort` | Standard control also won size 256, 856 ns versus 3.07 us insertion and 1.10 us adaptive. Caching/traversal consolidation would be a different lifecycle/invalidation change. |
| File Manager demoboard product ordering | stable presentation ordering | keep `std::stable_sort` | First-party consumer behavior stays at its standard control; no consumer-specific winner was measured. |

Support uses deliberately retain the standard operations: binary-search and
sort tests need independent controls; `sort_lab.cpp` is the tournament;
dispatcher tests canonicalize expected output; the renderer benchmark orders
measurement samples; and the house-policy checker sorts its emitted ledger and
rewrite plans deterministically. These are not production migrations waiting
to happen.

## Measured tournament

The complete result table is `SORT_LAB_M4_RESULTS.csv`. It records best total
time, iterations, time per sort, and a checksum for every algorithm/corpus pair.
Important outcomes are:

- stable insertion has a real small/nearly-ordered numeric niche;
- stable insertion has a severe random-medium regression and loses on the
  measured string corpus;
- house introsort did not beat `std::sort` in any measured corpus;
- the adaptive stable candidate did not beat `std::stable_sort` in any stable
  corpus;
- the standard library remains selected everywhere evidence is absent or a
  house candidate loses.

These are representative laboratory corpora, not claimed p50/p95 production
telemetry. The bounded three-role correspondence site needs no distributional
guess; other sites stay standard precisely because they do.

`tests/sort_algorithm_tests.cpp` verifies ordering equivalence across empty,
small, ordered, reverse, duplicate-heavy, deterministic generated, and larger
inputs. It separately verifies stable equal-key order for both house stable
algorithms.

## Binary search

All production `std::lower_bound`, `std::upper_bound`, and
`std::binary_search` calls were replaced by the contiguous index-returning
house core in `include/gui_forms/detail/algorithm/binary_search.hpp`.
First-party `std::*` occurrences remain only as independent test controls.

The house operations cover:

- lower-bound index;
- upper-bound index;
- exact membership;
- explicit comparator types and heterogeneous values;
- midpoint calculation as `first + count / 2`, avoiding `first + last`
  overflow.

`tests/binary_search_tests.cpp` proves empty/singleton/boundary/duplicate,
custom-comparator, constexpr, and deterministic generated equivalence against
the standard controls.

## Negative rule retained

No algorithm is selected because it is theoretically fashionable or because a
private implementation now exists. The losing candidates and their checksums
remain in the laboratory as negative evidence. A later production migration
needs a named workload and a win without material regression on the admitted
input domain.
