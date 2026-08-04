# M2c bounded PNG resource registry evidence

Date: 2026-08-04

Status: **OBSERVED implementation evidence for the bounded M2c proving slice**.
This record does not admit external resource packs, freeze public resource ABI,
select the M2 renderer, or expand the sole core image format beyond PNG.

## Scope and evidence labels

- **GIVEN**: untrusted PNG dimensions, row/stride calculations, color metadata,
  and compressed payloads must cross explicit bounded validation before use.
- **GIVEN**: the renderer-free core owns resource identity and encoded lifetime;
  the private Skia adapter consumes validated resources and does not invent IDs.
- **CANDIDATE**: the exact quotas in this slice are safety controls selected for
  proving work. They are not accepted compatibility or production limits.
- **CANDIDATE**: decoded output is eagerly materialized as premultiplied sRGB
  RGBA8. Compressed ICC profiles are rejected because they introduce another
  decompression and allocation boundary not yet admitted by this slice.

## Registry contract

`ImageRegistry` is renderer-neutral and included in `gui_forms_core` when Skia,
AppKit, the Gallery, and tests are all disabled. A `Window` owns one registry,
and its load, replacement, and removal entry points enforce the M1 UI-thread
contract.

Image IDs pack a 48-bit generation and 16-bit slot identity. Successful
replacement advances the generation atomically; removal followed by slot reuse
also advances it. A stale ID cannot resolve, replace, or remove the current
resource. Failed validation, quota checks, and allocation leave the prior
resource and registry revision unchanged.

Default proving bounds:

- 32 MiB encoded per image;
- 4,096 by 4,096 dimensions and 16,777,216 pixels;
- 64 MiB decoded RGBA8 per image;
- 4,096 PNG chunks per image;
- 1,024 resident resources;
- 64 MiB aggregate encoded bytes;
- 256 MiB aggregate decoded bytes.

Snapshots expose revision, live count, owned encoded bytes, and maximum decoded
allocation represented by the live set. Removal releases the encoded allocation
and decrements both aggregate accounts synchronously.

## Parser and decoder boundary

The allocation-free parser verifies the PNG signature, bounded chunk framing,
ASCII/reserved chunk-type bits, every chunk CRC, IHDR order and uniqueness,
legal bit-depth/color-type pairs, dimensions, source row bytes, RGBA8 decoded
bytes, PLTE/tRNS relationships, bounded fixed-size sRGB/gAMA/cHRM metadata,
contiguous IDAT chunks, IEND, absence of trailing bytes, and rejection of
unknown critical chunks. Errors are typed and named; malformed input does not
throw.

Structural validation deliberately does not inflate IDAT. The Skia adapter
therefore performs an eager bounded decode, verifies decoded dimensions and
RGBA8 byte size against registry metadata, and only publishes a raster image
after `getPixels` succeeds. A structurally valid PNG with a repaired chunk CRC
and invalid zlib payload is accepted by the registry and rejected by this eager
decoder gate without becoming drawable.

## Negative result retained

The pre-M2c one-pixel renderer fixture carried a bad IDAT CRC. The former Skia
path accepted it because decode was deferred; the new core parser rejected it.
Repairing only the CRC then exposed that its compressed payload also failed an
eager decode. The fixture was replaced with a standards-valid, eagerly decoded
PNG, while explicit corrupt-payload coverage was retained as a negative test.

## Gallery demonstration

The Gallery loads a compiled 16 by 16 sRGB RGBA status mark through
`Window::load_png`, retains its generational `ImageId`, records it into the
display chunk for the command strip, and has Skia synchronize the validated
registry before replay. The visible strip reads “bounded PNG registry”; the
diagnostic renderer line reads “Skia CPU m152 · PNG registry · Rapids UI”.
Portsmouth Rapids remains restricted to titles and controls; field text remains
Lucida Grande.

## Verification

Environment: macOS arm64, AppleClang 16.0.0.16000026.

- Full Gallery/Skia/AppKit Debug: 15/15 tests passed.
- Renderer-free Debug/Werror: 11/11 tests passed.
- Renderer-free ASan+UBSan: 11/11 tests passed with unsupported Apple leak
  detection disabled.
- Renderer-free TSan: 11/11 tests passed.
- Renderer-free Release: 11/11 tests passed.
- AppKit active-surface native close under ASan+UBSan: passed with Apple leak
  detection and the Skia-incompatible vptr sanitizer disabled.
- Parser mutation oracle: two byte-identical passes over every byte of the
  valid fixture.
- Fuzz entry: canonical `LLVMFuzzerTestOneInput` implemented. The installed
  Apple toolchain lacks `libclang_rt.fuzzer_osx.a`, so CMake automatically built
  and passed the ASan+UBSan standalone fallback over 100,000 deterministic
  truncation, extension, and bit-mutation cases. This is not claimed as a
  replacement for coverage-guided fuzzing on a toolchain with libFuzzer.
- Eager Skia smoke: valid PNG drew successfully; repaired-CRC invalid zlib data
  was rejected before publication.
- Canonical lifecycle trace remains 42 lines and 1,959 bytes, SHA-256
  `7fd93035ae50971f7f8fa04f8a0dc3e6164fc8a9bd23978d36ca78ad55d0f99a`.
- Computer-use inspection observed the blue validated-resource badge, Windows-
  inspired command strip, Portsmouth control typography, Lucida field text,
  active custom instrument, and the bounded registry labels in the running app.

## Unresolved edges

- PNG ancillary-chunk policy needs a broader compatibility corpus. This slice
  rejects compressed ICC profiles and does not claim complete PNG conformance.
- `ImageRegistry` views are invalidated by mutation and currently assume the
  owning UI domain. Cross-domain immutable-resource ownership remains a later
  contract.
- Resource-to-display-chunk reverse dependencies are not indexed. Replacement
  and removal conservatively damage the full window; precise resource damage is
  later M2/M9 work.
- External packs, file lookup, scale/color variants, cache eviction, fallback
  assets, and trust/signature policy remain later resource milestones.
- M2d still owes chunk-native Skia adapter measurements, 1/10/100-percent damage
  and 30 Hz bitmap-band benchmark records, symbol/dependency audit, a credible
  renderer comparison lane, and the renderer decision gate.
