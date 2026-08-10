# Sorting and binary-search audit

Status: **initial textual inventory from the dirty 2026-08-10 tree; workloads
and measurements pending**.

The implementation sibling must refresh this inventory from the exact starting
snapshot. No call-site winner is selected here.

The architect selected the laboratory candidate set: optimal house-style forms
of stable insertion sort, a run-aware stable merge/adaptive sort, and an
explicit-stack unstable partition/introspective sort. Standard-library sorts
remain controls and remain selected wherever no house candidate wins a declared
dimension without material regression.

## Current sort sites

### Ordinary sort

| Site | Data and semantic purpose | Initial workload hypothesis | Audit emphasis |
|---|---|---|---|
| `error_provider.cpp` snapshot | error icons ordered by stable UTF-8 ID | usually small; snapshot/report path | string comparison cost, determinism, maximum attached errors |
| `list_box.cpp` stable-ID validation | copied item IDs sorted to detect duplicates | can scale with item count; mutation-time | string sizes, duplicate density, already-ordered inputs |
| `list_box.cpp` selection normalization | integer indexes sorted then uniqued | often small and nearly ordered | insertion/adaptive sort versus standard sort |
| `property_grid.cpp` visible descriptors | alphabetical or category/name ordering | moderate; rebuild/refresh path | stability need, canonicalization cost, precomputed keys |
| `correspondence_view.cpp` expanded indexes | at most pinned, hover-expanded, and focused indexes | bounded to at most three current roles | direct ordered insertion or tiny fixed sorting logic |
| `text_store.cpp` style-span normalization | spans ordered by start/end/style | bounded content; often authored in order | nearly-sorted behavior, duplicate/overlap patterns, stable requirement |
| `damage_region.cpp` x edges | two numeric edges per nonempty rectangle, then unique | damage count dependent; potentially frame-sensitive | small-array behavior, duplicates, hotness |
| `damage_region.cpp` y intervals | numeric pairs sorted for every x strip | repeated inside area computation; potentially expensive | comparator/move cost, repeated work, possible specialized sweep design |

### Stable sort

| Site | Data and required stability | Initial workload hypothesis | Audit emphasis |
|---|---|---|---|
| `harfbuzz_font_engine.cpp` face candidates | role/content/other tiers; registration order retained within tier | small fallback-face set | stable insertion/adaptive sort; tier precomputation |
| `control.cpp` next-control traversal | children by `tab_index`, insertion order for equals | small/moderate retained child lists | frequency, existing order, stable semantics |
| `control.cpp` mnemonic traversal | retained children by `tab_index` | small/moderate; input path | repeated sort versus maintained/cached order |
| `window.cpp` traversal | eligible children by `tab_index` | navigation path | repeated tree sorting, allocation, stability |
| `window.cpp` mnemonic traversal | children by `tab_index` | keyboard input path | repeated sorting/caching and exact invalidation |
| `window.cpp` focus candidates | children by `tab_index` | navigation/focus path | stable order, tree size, already-sorted frequency |

The repeated stable tab-order sites may represent one semantic operation copied
across implementations. Audit consolidation or a shared named traversal helper,
but do not change public APIs, tree semantics, or invalidation rules.

## Current binary-search family

House replacements use a contiguous range/index core with named lower-bound,
upper-bound, and exact-search operations, explicit value/comparator types, and
no lambdas at call sites.

| Operation | Current uses |
|---|---|
| exact binary search | selected ListBox indexes, CheckedListBox selection, grapheme boundaries |
| lower bound | text layout offsets, text layout positions, ABI field-control positions, grapheme offsets |
| upper bound | generated Unicode ranges, style-span lookup |

The house implementation must prove:

- empty and singleton ranges;
- lower/upper edge insertion points;
- duplicates;
- custom named comparator behavior;
- no overflow in midpoint computation;
- iterator/range requirements stated explicitly;
- equivalence to the current standard operation over representative and fuzzed
  inputs.

## Measurement corpus required before sort selection

For each call site record:

- p50/p95/p99 and maximum element count;
- initial-order classification: sorted, nearly sorted, reverse, random, runs;
- duplicate/equal-key density;
- element size and move/copy cost;
- comparator calls and comparator cost;
- scratch allocation and bytes;
- invocation frequency and whether it occurs in input/frame paths;
- required stability;
- results under the actual libc++ and Windows toolchains.

Call-site winners are chosen after this data. The selected laboratory candidates
are stable insertion, adaptive/run-aware stable merge, and a nonrecursive
explicit-stack partition/introspective sort. Specialized bounded numeric/pair
ordering may still be measured for a concrete site. Each remains controlled by
`std::sort`/`std::stable_sort` and exact output equivalence.

## Negative rule

Do not choose an algorithm because it is fashionable, theoretically optimal in
the abstract, or already implemented elsewhere. Choose the smallest named set
that wins or materially clarifies the actual GUI.Forms workloads.
