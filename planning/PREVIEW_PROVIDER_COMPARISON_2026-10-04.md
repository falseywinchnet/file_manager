# Preview provider comparison

Status: **CANDIDATE evaluation**, 2026-10-04 UTC. No product provider selected.
This supplements [the proposed slice](PREVIEW_AND_THUMBNAIL_NEXT_SLICE.md) and
[Orchestrator's intake](../orchestrator/negotiations/FILE_MANAGER_PREVIEW_2026-10-04.md).

## Existing source and new local evidence

**OBSERVED:** preserved File Manager research commit `7e40e111` contains a
libjpeg-turbo 3.2.0 experiment, allocation-free bounded EXIF orientation reader,
independent orientation/color anchors and pinned scalar builds. Root confirmed
its exact-head push run `37111495248` and PR run `37111498441` passed on Windows,
macOS and Linux. It was deferred for missing production boundaries, not rejected
as a demonstrated decoder failure. The branch and stash remain intact; only its
research directory, historical evidence and dedicated workflow were restored.

**MEASURED:** the new generated-fixture Windows WIC comparison passes its
matched-geometry, shared-transform, color/alpha, grayscale and refusal checks.
The portable implementation has lower medians on both declared workloads in
two reverse-order runs; tails vary. Exact data, timing boundaries and source
review are in
[`2026-10-04-jpeg-wic-control`](../frontend/results/2026-10-04-jpeg-wic-control/README.md).
This supports continued portable evaluation, not final provider selection.
There is no photographic corpus, identical color/admission policy, process
containment or selection-to-native-paint measurement in that comparison.

## Decoder candidates and integration cost

| Candidate | Primary-source capability and observed local state | Remaining experiment |
|---|---|---|
| libjpeg-turbo 3.2.0 | Already available read-only in the adjacent toolchain. Its API supplies scaled decode, pixel and progressive-scan limits, and an intermediate-buffer limit; the latter is not a process-memory cap. [API header](https://github.com/libjpeg-turbo/libjpeg-turbo/blob/3.2.0/src/turbojpeg.h) | Resolve ICC/CMYK/precision policy and source-to-output scaling when codec factors alone cannot meet the requested extent. Current research rejects profiles rather than converting them. |
| Windows WIC JPEG | Supports frame/scaler/format-converter composition. WIC can delegate transforms to a decoder that implements its source-transform interface. The new control names the JPEG CLSID and avoids content-based codec discovery. [Microsoft transform overview](https://learn.microsoft.com/en-us/windows/win32/wic/-wic-imp-iwicbitmapsourcetransform) | Generated control is now measured; arbitrary-file policy, native color conversion, metadata discovery, OS-version variability and containment remain unproved. A CLSID names a registration, not a cryptographic guarantee of installed code. |
| macOS ImageIO | Apple's image-source thumbnail function returns an owned image and reports errors with null; it accepts an options dictionary. [Apple API](https://developer.apple.com/documentation/imageio/cgimagesourcecreatethumbnailatindex(_:_:_:)) | Run an equivalent bounded, oriented/color-controlled corpus. Do not infer its memory use or output equality from the API name. No new Mac ImageIO implementation was built in this slice. |
| Portable PDFium | Public embedder API supports document loading and bitmap rendering. Calls must be single-threaded or externally serialized. [Public header](https://github.com/chromium/pdfium/blob/main/public/fpdfview.h) | First-page CropBox/rotation, fonts, transparency, encryption and limit enforcement need fixtures. A renderable page does not establish identical layout across OS fonts. |
| Platform PDF adapters | Windows exposes `PdfDocument`; macOS Core Graphics exposes data-provider documents, pages, boxes and encrypted-document state. [Microsoft](https://learn.microsoft.com/en-us/uwp/api/windows.data.pdf.pdfdocument), [Apple](https://developer.apple.com/documentation/coregraphics/cgpdfdocument) | No common Linux platform implementation established here; separate adapters must preserve the same page/color/error semantics and failure containment. No claim that three host APIs eliminate portable packaging work. |

**OBSERVED PDFium build constraints:** upstream uses Chromium's build tooling,
GN/Ninja and Clang. JavaScript and XFA are enabled by default; a proposed
noninteractive decoder build must explicitly disable both. Only public headers
are intended for embedder use. This is a standalone PDF-library candidate, not
a proposal to bundle a browser or JavaScript engine.
[Upstream build instructions](https://github.com/chromium/pdfium/blob/main/README.md).
No PDFium revision, binary supplier or transitive dependency/license inventory
is accepted yet. Existing JPEG evidence does not justify skipping that work.

## Resource enforcement cannot use one misleading memory number

The proposed 256 MiB worker limit still needs a precise accounting class and
platform proof. Codec scratch, private commit, address space, resident bytes and
shared mappings are different quantities.

- Windows Job Objects can manage a process group, enforce configured limits and
  terminate its members. Closing the last handle can kill members when the
  corresponding flag is set. Security restrictions remain separate from job
  management; notification delivery alone is not enforcement evidence.
  [Microsoft Job Objects](https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects).
- Linux `RLIMIT_AS` bounds virtual address space and can cause allocation calls
  or stack growth to fail. It is not an RSS measurement or a filesystem/network
  capability sandbox. [Linux manual](https://www.man7.org/linux/man-pages/man2/setrlimit.2.html).
- Current published XNU source handles `RLIMIT_AS` through
  `vm_map_set_size_limit` and rejects lowering below current usage. This is
  source evidence, not proof about the installed macOS runner/kernel or a
  portable 256 MiB resident-memory ceiling.
  [Apple XNU source](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/kern/kern_resource.c).

**CANDIDATE next experiment:** a generated-input helper with declared startup
mapping footprint, scoped bytes, deadline, crash and resource-rejection cases
on each native OS. It must demonstrate termination/reaping and clean ownership
before any real-file adapter opens. First-party status does not silently remove
hostile-parser containment requirements. The closed-profile placement still
needs canonical reconciliation; this comparison does not activate ORC-PLG.

## Next choice to earn

Continue portable JPEG color/scaling evaluation using the retained EXIF corpus,
with WIC as a measured Windows control. Do not make the selected-preview job wait
for thumbnail storage/Engine identity work. Establish the smallest first-party
process profile and its limits before the application can claim another format.
PDF's dependency/build and page-rendering proof can proceed independently; JPEG
is not evidence that PDF parsing meets the same resource envelope.
