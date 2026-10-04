# Independent source review

2026-10-04 UTC, visible audit thread `01a0fb48-1437-7302-8b6f-f3bcf02e7319`.
Verbatim report below. Subsequent rebase preserved the entire working tree;
root separately reviewed the documentation and evidence record additions.

**No blockers or remaining house-style violations found in the requested decoder extraction and input-boundary changes.** The standalone executable now accepts caller-supplied encoded bytes through stdin; its research status must therefore be described as an unsupervised decoder with bounded input, rather than as accepting only internally generated fixtures.

The reviewed implementation handles the boundary correctly:

- **Bounds before allocation:** the reader first fills a fixed 16-byte header, checks `FMJPEG01`, and decodes the length explicitly as little-endian `uint64_t`. It rejects zero and lengths above 16 MiB before conversion to `size_t` or vector allocation. The body uses one initialized vector with no per-read growth.
- **Exact body and actual EOF:** `read_exact` advances only by bytes returned from `fread` and rejects truncation/read errors. After the declared body, `fgetc` plus the stream-error check requires actual EOF and rejects any trailing byte. A sender that leaves stdin open can block this operation; the executable supplies no deadline, as expected for this presently unsupervised research boundary.
- **Ownership and failure:** encoded bytes remain owned through decoding. Codec and compressed-profile owners are noncopyable and release their foreign resources during normal return or exception unwinding. No input borrow escapes. Envelope and decode failures occur before the frame writer is invoked, so those failures emit no preview bytes. Output-write failures can still leave a partial frame; they return failure through the existing exception boundary.
- **Extraction and linkage:** the moved decode/orientation implementation retains its established validation, allocation and transformation sequence. The new declarations state input borrows and the validated-raster precondition for `orient`. The standalone target links the private research decode library; fixture generation and WIC remain in the experiment executable. The reviewed CMake file adds no install rule or application/GUI linkage.
- **Python conformance tooling:** subprocess arguments are passed as a list without shell interpolation. Captured stdout is compared byte-for-byte for the direct/separate baseline, and each of the nine refusal cases requires exit code 1 and empty stdout. The recovery check launches a fresh process. The 15-second subprocess timeout bounds these fixture invocations; the script correctly states that checking captured output size afterward is **not** a hard output-memory limiter.
- **Workflow:** the existing `pipefail` setting covers the added generator → decoder → Rust-consumer pipelines. Both encoded modes traverse all eight EXIF orientations. The independent Rust oracle remains distinct from the Python same-implementation equivalence check.

I applied the complete [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md), including explicit types, named behavior, ownership and borrow lifetimes, initialization, conversions, operation order, failure handling and repeated-loop storage. The Python review applied the language-neutral requirements rather than C++ syntax rules.

Exact acceptance scope:

- Full new [codec_owner.hpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/codec_owner.hpp).
- Full new [jpeg_decode.hpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/jpeg_decode.hpp) and [jpeg_decode.cpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/jpeg_decode.cpp).
- Full new [encoded_stream.hpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/encoded_stream.hpp) and [encoded_stream.cpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/encoded_stream.cpp).
- Full new [decoder_main.cpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/decoder_main.cpp).
- Full new [verify_decoder_input.py](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/verify_decoder_input.py).
- Extraction, aliases/includes and encoded emit-mode changes in [jpeg_experiment.cpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/jpeg_experiment.cpp).
- Target/linkage changes in [CMakeLists.txt](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/CMakeLists.txt).
- Separated pipelines and refusal-check invocation in [jpeg-research.yml](C:/Users/Shadow/file_manager/.github/workflows/jpeg-research.yml).

The 16 MiB envelope limit is not a whole-process memory ceiling, and the inert `7/11` ticket establishes no authentication or source authority. Confinement, deadlines, supervision and publication remain outside this implementation. Forthcoming documentation/results were not reviewed.

I performed no edits, builds, tests or Git mutations. Local passing suites, sixteen separated streams, nine refusals, recovery, and the byte-identical extraction comparison remain root-reported evidence.
