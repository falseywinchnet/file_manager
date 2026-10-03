# Independent navigation and operation progress: delivery

Published prerelease:
[v0.001-alpha.31d1319](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.31d1319).

Source `31d131949cf80ad82ea125ce1fa440c53ad199af` passed both complete native
matrices and rebase-merged with exact tested/merged tree equality; see
`NATIVE_ACCEPTANCE.md`. This release includes PR25's confirmed Copy progress
and PR26's independent operation worker, pending-navigation/history repair and
deferred directory refresh after Settings. Ordinary New Folder/Rename/Undo from
the preceding release remain available; ordinary Copy/Move/Delete remain closed.

Root downloaded all three push-run archives and independently verified exact
clean source revision, archive sidecar SHA-256, all receipt-listed file hashes,
and executable mode where applicable. `independent-archive-verification.json`
records 44 Windows, 42 Mac and 40 Linux files. All six uploaded archive/sidecar
digests were checked against local bytes before publishing the draft release.

| Platform | Archive SHA-256 |
|---|---|
| Windows x64 | `db3161340fa2fa72bf50a9e89e0f575e00666278e2d8d4e846fac71ee01fdc51` |
| macOS arm64 | `77510030903d20db9c1f0ccaa8f8c22c61808675a83805c181e286229787bcfd` |
| Linux x64 | `d66d860beed7fb9f65b96fc0077dc0e1b8a310be66f5aeae678080d51d4c72fa` |

Root inspected the Mac frontend CTest log (all 16 suites passed) and the native
ordinary-action screenshot. It shows readable selected-file text, four Details
columns and matching `renamed-preview.txt` row, preview heading and Name property.
The log records Create/Undo and F2 Rename/Undo with inspector names passing.
`MacAcceptedLastTest.log` and `MacAcceptedInspector.png` preserve this evidence.
This uses synthetic delivery through a real AppKit window, not physical input
or complete packaged daily-workflow acceptance.

Portable alpha: Windows x64; macOS arm64 with load-command minimum 26.0 and
ad-hoc signing, without Developer ID notarization; Linux x64 on the documented
Ubuntu 24.04/X11 or XWayland baseline. Packaged startup proves five seconds alive
on a generated empty root. Installers, broader previews, thumbnails, indexed
folder facts, ordinary transfers/Trash, search setup and interface refinement
remain unfinished. No heavy-disk-load latency claim is made. The later regular
file and directory modification-date repairs are not included in this release.
