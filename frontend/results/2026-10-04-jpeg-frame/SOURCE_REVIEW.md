# Independent source review

2026-10-04 UTC, visible audit thread `01a0fb48-1437-7302-8b6f-f3bcf02e7319`.
Verbatim completed report. Root separately reviews authored documentation and
the eventual native results against their actual evidence.

**No blockers or remaining house-style violations found in the requested joined JPEG/frame changes.** The source supports a closed generated-fixture experiment that sends real decoded JPEG pixels to a separate Rust process.

- **Writer bounds and ownership:** `write_preview_frame` borrows the raster synchronously. It validates nonzero tickets, positive axes bounded to 1024, tight stride, exact payload extent and every alpha byte before writing anything. Those axis bounds make the stride/product calculations and subsequent `uint32_t` conversions safe. It uses one fixed, zero-initialized header and retains no raster borrow.
- **Wire representation:** the writer explicitly encodes 32-bit and 64-bit fields little-endian. The zero-initialized header correctly supplies ready status, zero reserved bytes, and the high bytes of version/header size. Payload is the validated tight BGRA byte vector, with no native structure serialization.
- **Output failures and EOF:** emit modes reach the writer without diagnostic text on stdout; caught exceptions go to stderr and produce a nonzero exit. Writes and flush are checked. An output failure can leave a partial frame, which the receiver rejects at EOF. If a complete frame reaches the consumer before the producer reports failure, workflow `pipefail` still fails the joined experiment. This is appropriate fixture orchestration; it is not production reap/publication enforcement.
- **Generated EXIF helper:** the extracted helper bounds source extent before adding the 36-byte APP1 segment, reserves the complete destination capacity, and preserves encoded image and ICC bytes. The orientation edit targets the intended TIFF value. Its callers supply generated JPEG bytes; this is not an arbitrary-file injection interface.
- **Independent oracle:** the Rust checker derives expected dimensions and EXIF quadrant placement independently of the C++ transform implementation. Its scalar linear-to-sRGB curve matches the generated profile’s linear transfer function, D65 white and sRGB primaries. It compares three channels at four interior anchors with a declared ±4 tolerance. This establishes those anchors and geometry, not pixel-for-pixel correctness across the entire image.
- **Gated scope:** the two closed modes accept only a one-character orientation from 1 through 8. They generate their input internally and cover baseline/no-ICC and progressive/linear-ICC paths, each with embedded EXIF. No real-file authority, supervisor, provider registration or GUI publication is introduced.

I applied the complete [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md), including ownership, borrows, operation order, initialization, conversions, failure states and repeated-loop storage. The new writer and checker keep their storage and execution explicit; the checker computes its color expectations before inspecting raster anchors.

Exact acceptance scope:

- Full new [preview_frame.hpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/preview_frame.hpp) and [preview_frame.cpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/preview_frame.cpp).
- Changes in [jpeg_experiment.cpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/jpeg_experiment.cpp): `with_orientation`, its existing-test integration, `emit_generated_frame`, the new include, and closed emit-mode dispatch.
- The writer target addition in [CMakeLists.txt](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/CMakeLists.txt).
- Full new [check_jpeg.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_frame/src/bin/check_jpeg.rs).
- Changes in [jpeg-research.yml](C:/Users/Shadow/file_manager/.github/workflows/jpeg-research.yml): path triggers, directory spelling-check scope, Rust consumer build, and sixteen streamed checks.

I also inspected the existing pixel type, fixture generation/decode path, ICC fixture primaries, exception boundary and scanner directory handling as supporting context. This does not extend acceptance to all unchanged code in those files.

No edits, builds, tests or Git mutations were performed. The sixteen local passing streams, existing JPEG checks and Clippy result remain root-reported evidence; native CI and the forthcoming documentation/results are outside this review.
