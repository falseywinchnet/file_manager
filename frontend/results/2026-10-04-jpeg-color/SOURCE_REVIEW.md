# Independent read-only source review

Review by the authorized visible audit sibling. The raw report below predates the const correction and final marker-test review; follow-up acceptance must be recorded separately.

**No blocking ownership, raster-extent, or conversion-order defect found in the reviewed ICC conversion core. One concrete house-style correction remains.** The integrated experiment changed during this review as marker tests were added, so this is scoped acceptance of the conversion/fixture core—not final acceptance of the evolving complete diff.

**Required house-style correction:** mark the read-only by-value parameters `const` in [icc_color.cpp](C:/Users/Shadow/file_manager/frontend/experiments/jpeg_decode/icc_color.cpp:20):

- `report_error`: `context` and `code`;
- `Profile` constructor: `acquired`;
- `Transform` constructor: `acquired`.

For handle typedefs, top-level `const` fixes the local handle value; it does not make the referenced vendor object immutable. This follows the house style’s explicit instruction to mark read-only inputs `const`. It is not a functional defect.

The conversion’s ownership and failure handling are sound in the inspected source:

- `ColorContext::error` is initialized before context creation. The named, non-throwing error callback borrows that invocation-local state. No process-global handler is changed.
- Transforms are destroyed before profiles, and profiles before their context. Fixture cleanup likewise releases profile and curve before context. Failed acquisition or conversion unwinds the owners.
- The profile span and source raster are synchronous borrows. Output has separate owned storage; failures do not modify either input or publish a partial result.
- Raster dimensions and stride establish the byte extent before traversal. The 1,024-square bound makes the pixel-count conversion to `cmsUInt32Number` safe.
- Gray extraction allocates once before its loop and checks channel equality. Output alpha is initialized; the RGB path explicitly copies the validated opaque alpha.
- Color conversion occurs before orientation, and replacing the raster transfers ownership explicitly.

The oracle is appropriately independent **within its tested domain**. The direct 256-level neutral ramps use a scalar sRGB transfer equation rather than asking LittleCMS to generate expected output. Embedded baseline/progressive fixtures exercise conversion followed by all eight supplied orientations, with separate tolerances. The retained optimized-gray failure justifies the localized `cmsFLAGS_NOOPTIMIZE` choice for this pinned experiment. It does not establish a general LittleCMS optimizer defect or prove that every gray profile needs that flag.

Important evidence limits remain:

- The generated profiles and tests establish sRGB identity and linear RGB/gray transfer behavior. They do not establish accuracy for arbitrary LUT profiles, wide-gamut photographic profiles, unusual adaptation data, or perceptual quality.
- The embedded color test supplies orientation to `decode`; it does not combine embedded ICC and embedded EXIF parsing in the same end-to-end fixture.
- The refusal cases cover several useful boundaries, but are not complete profile validation or resource-containment evidence.
- WIC’s existing control does not become color-policy-equivalent merely because the portable path now converts ICC profiles.
- The existing timing fixtures contain no ICC profile, so their measurements do not measure this conversion stage.

Two boundary details should stay explicit in the final documentation and fixture review:

1. **The 1 MiB profile cap is enforced after the codec’s header processing.** The pinned TurboJPEG source can extract and retain ICC bytes during `tj3DecompressHeader`; the later query and cap prevent the specimen’s additional profile copy and conversion from accepting oversized profiles. This is an accepted-profile limit, not a guarantee that the codec never allocates more than 1 MiB for metadata. The header already correctly disclaims whole-process memory/deadline protection.

2. **Malformed marker refusal depends on the checked header result as well as the later ICC query.** The pinned source emits warnings for several inconsistent ICC sequences, and the specimen enables `STOPONWARNING` and checks the header status. I therefore did not establish a malformed-sequence-to-absence defect. The marker checks being added should verify that behavior; the “absent profile” result alone is not sufficient evidence.

The build inputs are consistent with the stated research scope. The fetch script pins archive bytes, while `find_package(lcms2 2.19 EXACT)` reflects the pinned source’s actual package version. The workflow disables shared-library, tool, test, fast-float-plugin, and threaded-plugin builds for the color dependency and installs it under the research prefix. A manually supplied package reporting `2.19` is not independently proven to be release `2.19.1`; final execution receipts should retain the resolved package path and archive provenance. No application integration is introduced.

**Exact reviewed hashes**

| File | SHA-256 |
|---|---|
| `icc_color.hpp` | `76EB32207A7809A4EC4CFC72BD94E3E11DDA708B55FEDB123CCBF981E3F53996` |
| `icc_color.cpp` | `7035A87BCD14FEFF021679575CFBFD85B847F529F342DFCB3681A79E548DC343` |
| `icc_fixtures.hpp` | `33F76004792F45D62008534F841AB5504CD40F20248A739946AF2F79E1DA07F5` |
| `icc_fixtures.cpp` | `C91E4831A509D8020233EAF09ED1FD661C1F05FED953AD3264B48868AF29A8E9` |
| `CMakeLists.txt` | `6809642AE926EAFBB2266C90C72C71186338B547EA47F112A4FB4E3B5FAB80A2` |
| `fetch_color.cmake` | `A54A0C4922A2EA933A46B71918E0B197EADFB0FA5E96E95F61A47CFE7533E684` |
| `.github/workflows/jpeg-research.yml` | `E4B821C062884EA39AF1005E8DE222CC1C0C540ECD2E8149D84248EC43522C9E` |

The inspected `jpeg_experiment.cpp` integration initially hashed `34E1370EFAAC52230FFC1C56B94A9939E10742BE5B1A7F8DA6277888DFE6F737`; it changed to `419E5C5499C20FF8C5AA520B517EDD2E2D6F623965B7959E0A59850C4AB86BCC` before the final hash check. **The latter snapshot is not covered by this completed integration review.**

No edits, builds, Git mutations, or workers were used.
