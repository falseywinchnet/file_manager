# First-party Image Converter plugin plan

Date: 2026-08-06.

Status: **GIVEN future controlled-transform proof; implementation closed**.

## Mission

Prove the smallest safe create-new plugin transformation: convert selected
AVIF/WebP and later admitted image inputs to PNG or JPEG in the same folder or a
trusted user-selected destination without granting the decoder arbitrary
filesystem authority.

## Product behavior

- Installed by an admitted Malkuth package but disabled until the user enables
  it under the final release policy.
- Host-rendered context command **Convert Image...** appears only for supported
  inputs while enabled.
- An owned trusted options dialog selects output type, quality/matte and bounded
  batch destination policy.
- Default output is a new sibling file; source replacement/deletion is not
  offered by the first version.
- Collision resolution belongs to File Manager. Output is staged, validated and
  atomically published by trusted code.

## Transform profile

- PNG is the lossless alpha-preserving default.
- JPEG requires quality and an explicit matte when source transparency exists.
- Orientation, color profile, animation/multiple frames, metadata retention,
  bit depth and HDR behavior must be decided and shown before conversion.
- Unsupported animation or profile features produce a refusal/warning rather
  than silent flattening unless an explicit profile permits it.
- A bounded batch shares one declared profile but returns per-file terminal
  results.

## Reef

The plugin receives only selected read handles/streams, declared limits and
host-owned output streams. It cannot enumerate the folder, select a destination,
overwrite/delete inputs, add controls, call Engine writes, use the network or
remain resident while disabled. General decoders stay outside GUI.Forms and the
trusted frontend.

## Gates

1. Exact formats/decoders and licenses selected through security review.
2. Host-mediated transform/publication contract reconciled.
3. Color/profile/metadata/animation behavior approved.
4. Malformed-image and resource-exhaustion corpus passes in containment.
5. Native File Manager context/options/collision flow passes.
6. Grand architect explicitly opens implementation.

## Evidence

Golden pixel/color/alpha/orientation fixtures, corrupt/truncated/bomb images,
huge dimensions, batch partial failure, collision, cancellation, plugin crash,
disabled-zero-activity, throughput/RSS/staged bytes and source-preservation.

## Deferred consumers

OCR may later consume the same create-new contract for text output. It is not
enabled by this plan and would require its own model/data, privacy, language,
quality and provenance gates.
