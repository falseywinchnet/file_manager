# Adaptive and demand-driven preview delivery

Published prerelease:
[v0.001-alpha.7be8b8a](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.7be8b8a).
Release ID `402781635`; published 2026-10-04 02:08 UTC.

This portable checkpoint includes the demand-aware selected-preview lifecycle,
adaptive inspector content dimensions and monospace excerpt, and the preceding
Windows brush-cache work. It retains bounded UTF-8/PNG format coverage. The
private prepared-document development is not activated as a new public viewer.

**MEASURED:** exact source `7be8b8a1a1821b21dc41729a026a6ee43ae848ec` passed all
four checks in native push run `37169104924`: house style, Windows x64, macOS
arm64 and Linux x64. PR36 was rebase-merged as
`5995c0f576b9afbdfcd3b28f6025be39b137daeb`; complete tested/merged trees match
`f7cea74adfa9246e76e48186134cb3e0b3a9980e`. An older merge-ref run is not used
as the exact-head acceptance receipt here. Earlier PR35 already passed both its
native matrices before integration.

Frontend suites passed 15 on Windows, 16 on Mac and 14 on Linux. Native logs,
Mac text/image screenshots and the clean Mac source receipt are retained here.
Root visually inspected actual AppKit text and PNG content, the separate
coverage caption and the stable properties layout. Synthetic input also checks
Details and ordinary Create/Rename/Undo. This is not physical dogfood on the
owner's Mac or a whole-application latency measurement.

Root independently verified all three downloaded archives: clean exact source,
SHA-256 sidecars, every receipt-listed file hash and Unix executable permissions.
The verification JSON records 44 Windows, 42 Mac and 40 Linux files. All six
remote archive/sidecar digests matched local bytes before draft publication.
The initial tag-based API lookup returned 404 for the draft; lookup by its
confirmed release ID succeeded. No asset was published before verification.

| Platform | Archive SHA-256 |
|---|---|
| Windows x64 | `16afb56f071bca98e204a7ac52ec2f397a9b1e82ad42eeb9232b6687485e3ef2` |
| macOS arm64 | `088db2be0d34dc96e45fbf376fa1cdd201215be523c53297dda369fb7ee8140a` |
| Linux x64 | `576de022cabce903b7460bd75877d6f0f5f58d00586f77740d4efc23c563a0f6` |

The archive receipt records a five-second packaged startup on a generated empty
root. These remain portable alpha archives, not finished installers. Mac needs
Apple Silicon and macOS 26 or later, with ad-hoc signing and no Developer ID
notarization; the actual CI host was macOS 26.6.2. Windows is unsigned x64.
Linux targets Ubuntu 24.04 with X11/XWayland and its named system libraries.

Ordinary New Folder, same-parent Rename and one-step Undo are available.
Ordinary Copy/Move/Delete, broad preview formats, indexed thumbnails, folder
aggregates, automatic search setup and remaining interview refinements are
unfinished. Selecting a hidden preview now defers work; cancellation still
cannot interrupt an OS read already entered. Visible large-PNG admission and
end-to-end latency remain open.

This delivery record and copied evidence were reviewed for source identity,
accurate scope and distinction between automated and physical verification.
No implementation changed in the delivery follow-up. Existing verification
helper code was used unchanged and is not newly style-certified here.
