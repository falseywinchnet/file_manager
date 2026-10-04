# Independent scoped source reviews

Retained verbatim from the authorized visible audit sibling. Its execution qualifications reflect review time; the final root evidence is in README.md. The timing review predates the final malformed-header/enlargement/recovery assertions, which root reviewed separately.

**Scoped source acceptance: no blocking correctness, COM-lifetime, extent, or house-style finding in the inspected WIC research additions.** Compilation and execution remain unverified by this review.

The ownership chain in [wic_decode.cpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/wic_decode.cpp) is sound:

- `ComApartment` accepts both successful COM initialization results and balances them with `CoUninitialize`. An incompatible apartment causes construction to throw, so it does not incorrectly uninitialize an apartment it failed to initialize. This matches Microsoft’s requirement to balance successful calls, including `S_FALSE`. [CoInitializeEx documentation](https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex)
- Every COM interface starts null and has one stack owner. Destruction releases converter, scaler, factory, frame, decoder, and stream before the apartment ends. Failed calls unwind through the same owners; no partial raster escapes.
- Encoded bytes remain borrowed for the synchronous call. The memory stream is initialized from them and remains alive throughout decoding; the returned vector owns independent pixels. No deferred callback or retained frontend object is introduced. [SHCreateMemStream documentation](https://learn.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-shcreatememstream)
- The `void**` conversions are confined to COM acquisition boundaries. The wrapper does not expose COM ownership to the shared pixel type.

The arithmetic is bounded before allocation or copying. The 16 MiB input cap makes the `UINT` conversion safe. Source geometry is checked with a 64-bit product. Requested output dimensions are positive and at most 1,024, so `output_width * 4`, the `UINT` stride, and `stride * target_height` fit their domains and cap the destination at 4 MiB. `CopyPixels` receives that same extent. The no-enlargement check applies to both axes.

The research comparison is useful but narrower than a complete decoder-policy comparison:

- Both decoders receive the same generated baseline/progressive bytes and the same requested output geometry.
- WIC’s output is checked against independent color/alpha anchors rather than byte equality with libjpeg-turbo.
- **The orientation checks test WIC-decoded pixels followed by the shared first-party `orient()` function. They do not test WIC EXIF extraction or native orientation handling.**
- WIC’s geometry is supplied from the portable decoder’s selected dimensions. This compares rendering at matched dimensions; it does not independently validate WIC output-size selection.
- The WIC wrapper does not enforce the portable specimen’s identical precision, lossless, ICC, or color-space policy. That is acceptable for these generated fixtures, but the comparison must not be described as equivalent admission behavior for arbitrary JPEGs.
- No WIC timing mode, process limit, cancellation, photographic corpus, or application presentation result is established here.

The new test deliberately performs fresh WIC decodes before each shared transform. That is repeated integration-test work, not a reusable-buffer or allocation-free path. Its scope is explicit, and I found no allocation inside a pixel kernel. The marker-count fixture now allocates its known extent before indexed writes, correcting the previously identified house-style violation.

The [shared pixel header](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/jpeg_pixels.hpp) exposes owned contiguous pixels and a synchronous research-only function. The [CMake changes](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/CMakeLists.txt) add WIC sources, libraries, and the test only on Windows, with no product install or application integration.

Before retaining the execution receipt, update the research README to describe `--verify-wic` and the shared-transform limitation. Its current statement that WIC merely remains a comparison candidate predates this control implementation. Likewise, root’s confirmed historical three-platform run should be recorded separately from the new, not-yet-compiled WIC additions.

I reviewed the complete changed files against the full [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md), including named execution, explicit types, foreign-resource ownership, synchronous borrows, operation order, checked extents, initialization, cleanup, and repeated work. **No remaining concrete violation identified in this changed scope.** Dependencies and unrelated legacy code are not certified.

| Reviewed file | SHA-256 |
|---|---|
| `wic_decode.cpp` | `D50CE654C7A5F78F512CB645C0CA081AE31B26AA8B694F19743C021E34B460D1` |
| `jpeg_pixels.hpp` | `C39C4FC50E540F626F74763E422F84DD06C5EFCC62DF5D6D2A2020C41AE8ABF5` |
| `jpeg_experiment.cpp` | `ABD8F71753C24BA3EF615E8BF136093705870D0FFD64F9EC0762593816E491EC` |
| `CMakeLists.txt` | `B9488C4D71A46AA8DA4A6C4DDA26D150903C2A995C4733130940B31E9C60DA51` |
| `exif_orientation_tests.cpp` | `7D3CA1CFCB4D08045DCD367854E16991C9CBA84A81AFB545FC81F891ED73EF7D` |

No edits, builds, Git operations, or workers were used.

---

**Scoped acceptance: no blocking correctness or house-style finding in `measure_wic()` and the `--measure-wic` dispatch.** The timing boundaries match the stated comparison.

- Output geometry is obtained before timing; the portable decoder is not called inside the WIC timed interval.
- The interval includes fresh WIC owners, COM initialization/uninitialization, the memory-stream copy, decoding/scaling/conversion, and the shared orientation transform.
- The timestamp is taken before validation, CSV output, and destruction of the final raster. For rotated output, destruction of the temporary unrotated allocation occurs inside `orient()` and is included—consistent with the portable transform path.
- The WIC measurement command runs its correctness checks before emitting samples. The existing CSV schema and four fixture/orientation groups remain compatible with the summarizer.
- Explicit types, named behavior, initialized counters, ownership transfer, and synchronous lifetimes follow the complete house style. Fresh allocation per sample is intentional measurement scope, not a reusable-workspace claim.

One **nonblocking comparison detail** should be recorded: `const Pixels geometry` retains its complete portable-decoded raster throughout both WIC sample loops, although only its dimensions are used. Thus the WIC run has an additional resident raster outside the timed work. It does not invalidate the elapsed-call measurement, but the runs do not have identical resident-memory baselines. For a future memory comparison, retain only the dimensions and release that raster before sampling; do not infer a measured latency effect from its presence.

Keep the results described as **fresh-call implementations at matched output geometry**, not isolated codec speed: WIC includes stream copying and COM setup/teardown, while the portable implementation borrows encoded input. Neither run establishes EXIF-policy equivalence, color equivalence, cold-process behavior, or product latency.

Reviewed [jpeg_experiment.cpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/jpeg_experiment.cpp), SHA-256:

`CD2EA013AA571AF2BBFA54E42C18A4D90C4DAC60F6961EC9594D9AB0F76C8B2D`

The reported 3/3 correctness pass is root’s execution evidence. No builds, edits, Git operations, or workers were used in this review.
