# Pinned style references

Status: **OBSERVED source evidence plus bounded architect interpretation**.

These references inform style. They are not dependencies and are not complete
GUI.Forms specifications.

## BFFT heap array

Pinned source:
[BFFT `bruun_simd_backend.hpp` at f5fb842, line 222](https://github.com/falseywinchnet/bfft/blob/f5fb8420eb12d555c9413e7489d93054c17b2009/src/detail/bruun_simd_backend.hpp#L222).

Observed properties of `bruun::heap_array<T>`:

- explicit `T*`, capacity, and length members;
- move-only ownership;
- explicit destruction and release;
- 64-byte aligned allocation through `_aligned_malloc` or `posix_memalign`;
- checked element-count multiplication and capacity doubling overflow;
- Boolean allocation failure from `resize`, `reserve`, `assign`, and
  `push_back`;
- placement construction and explicit element destruction;
- direct pointer, index, begin/end, length, and capacity-like mechanics;
- no attempt to reproduce the complete standard-container contract.

Architect interpretation:

- Use/copy/adapt this style only for a presently large, non-growing heap
  allocation that is subsequently edited in place.
- Do not use it for small arrays or a collection whose contract requires
  growth.
- Do not make it the default growable collection and do not replace
  `std::vector` wholesale.
- GUI.Forms must not gain a link dependency on BFFT.
- Before adoption, establish the call site's size distribution and memory
  footprint, then audit element requirements, move exception behavior,
  alignment, fixed-size semantics, OOM path, and in-place editing behavior.

The implementation is evidence and a useful starting point, not a license to
copy it without tests or adapt it into a general Mini-STL.

## Cleanup header

Pinned source:
[Cleanup `Release/cleanup.h` at ac5be0f](https://github.com/falseywinchnet/Cleanup/blob/ac5be0f64b00495d4e69edf951ec8f22dd534fe9/Release/cleanup.h).

Observed useful style:

- explicit aliases for problem-domain scalar/index types;
- named functions and direct loops;
- compile-time fixed `std::array` workspaces;
- explicit local scalar types and casts;
- standard named numerical operations such as `accumulate` and
  `inner_product` where their mathematical meaning is clear;
- ordinary `std::sort` in places where ordering is required, which supports the
  decision to audit sort by workload rather than ban all standard algorithms;
- direct, inspectable staging buffers rather than opaque expression machinery.

Bounded interpretation:

- Copy the epistemic stance, not formatting quirks or every implementation
  detail.
- Fixed arrays are appropriate only when the bound is a real domain constant.
- Direct loops are preferred when they expose meaningful control flow; standard
  named algorithms remain preferred when the name is the clearest complete
  description and implementation choice does not matter.
- Large compile-time tables and monolithic class organization are not mandates
  for GUI.Forms.

## Combined house-style reading

The shared value is explicit execution and storage, not an anti-STL ideology.
GUI.Forms may use ordinary standard facilities and explicit low-level machinery
side by side. The selection test is whether the source makes the relevant type,
lifetime, allocation, ordering, and work knowable at reasonable cost.
