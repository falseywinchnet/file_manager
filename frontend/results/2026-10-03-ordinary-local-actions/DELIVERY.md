# Ordinary local actions native delivery

**MEASURED:** clean source `fc6312c02daf5608cde144c64716a3d158088363` passed
both complete native matrices: push `37112362735` and PR `37112365392`.
Windows x64, macOS arm64, Linux x64 and house-style jobs all passed.
PR #23 rebase-merged as `948b3941d5d885bf56e310382c03ef5d4caafc00`.
Root fetched and verified exact tested/merged tree equality:
`a091e1032ef3d3f24a7614c484d06e32b77cf7b3`.

Published prerelease:
[v0.001-alpha.fc6312c](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.fc6312c).

Normal launch enables New Folder, inline/property Rename and their one-step
identity-checked Undo without a quarantine directory or search service.
Explicit read-only launch remains available. Details semantic item actions now
retire header focus so F2 reaches Rename. Same-identity directory refresh updates
inspector facts and preview bytes after Rename, Undo and F5.

Root independently checked all three push-run archives for clean source and
exact revision, sidecar SHA-256, every receipt-listed file hash and executable
mode where applicable. The retained verification JSON records 44 Windows,
42 Mac and 40 Linux files. GitHub's uploaded asset digests match:

| Platform | Archive SHA-256 |
|---|---|
| Windows x64 | `a4cd592476b2a3762e7984b383f1072abf36b8e06bae039ef291369b9918c2cc` |
| macOS arm64 | `78174859a38c343e4f4f74d8a4b62a3073ba1c3b3397fdc45931b779155cf2fc` |
| Linux x64 | `eb81606a2b0a4d4ef57bb4696e838401dc53412265cf4a9ab3624a2109081a28` |

Both Mac screenshots were visually inspected. The selected Details row,
preview heading and Name property all display `renamed-preview.txt`, with
readable text and its coverage caption. `MacAcceptedInspector.png` preserves
the push-run image; `MacAcceptedLastTest.log` retains all 16 frontend passes,
including menu Create/Undo, F2 Rename/Undo and both inspector-name checks.
The earlier rejected screenshot remains alongside the repair record.

Portable alpha only: macOS arm64/macOS 26+, ad-hoc signed without Developer ID
notarization; Windows x64; Linux x64 on the documented Ubuntu 24.04 and
X11/XWayland/library baseline. Packaged launch checks prove five seconds alive
on an empty generated root. Mac native interaction is synthetic delivery through
a real AppKit window, not physical-input or full packaged-workflow acceptance.
Ordinary Copy/Move/Delete, indexing setup, wider previews, thumbnails, folder
aggregates, interface refinement, installers and broader accessibility remain
unfinished. The separate Copy-progress work is not included in this release.
