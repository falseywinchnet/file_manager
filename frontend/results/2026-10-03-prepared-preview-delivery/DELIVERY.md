# Preview responsiveness and copy dates: delivery

Published prerelease:
[v0.001-alpha.0372733](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.0372733).

This portable alpha includes the previously merged fixed-size Label measurement
and repeated paragraph-width reuse changes, copied file/directory date repairs,
and the Engine catalogue/observation status capture repair. The measured Windows
wrapping improvement is scoped to the workloads in
`../../../gui_forms/results/2026-10-03-label-wrap-reuse/README.md`; it is not a
Mac or complete input-latency claim. Ordinary New Folder, same-parent Rename and
one-step Undo remain available. Ordinary Copy/Move/Delete remain closed; the
metadata repairs apply to the existing protected development Copy profile.

**MEASURED:** original source `0372733898fdf16889ee90f9004b977d1e51438b`
passed both complete native matrices, push run 37122027342 and PR run 37122068315.
The merged main commit `568d7fbde2cc173cb5f857431aec53400ee1b550` has the exact
same tree, `2b87aa66ebd3272b20d38c58734ec567880cb255`. The private shaper/session
implementation is tested but not activated in the packaged frontend or normal
SDK. Neither this release nor its source acceptance establishes finished async
preview delivery, thumbnails or wider format support.

Root downloaded all three push-run archives and independently verified clean
source revision, archive sidecar SHA-256, every receipt-listed file hash, and Unix
executable mode. The retained verification JSON records 44 Windows, 42 Mac and
40 Linux files. All six remote archive/sidecar digests matched the local bytes
before the draft was published. GitHub release ID: 402504432.

| Platform | Archive SHA-256 |
|---|---|
| Windows x64 | `7cc1f7eb658f3c775144c46427b3b7d690bc3a7d3a217f934fce01d8718573a2` |
| macOS arm64 | `bca6304f0895932eb6161da918e05acb8341a9ad683570fefe03ecb82f1a3753` |
| Linux x64 | `0e734af31f6e721ae6fa104a2244e22e1fa1bacceeac0645c9ca4aeebc8c32bc` |

Root inspected the real AppKit text-preview screenshot and frontend log.
All sixteen Mac frontend suites passed; the native fixture records readable
selected text, coverage caption outside the body, four Details columns and
Create/Rename/Undo. Synthetic native input does not establish physical dogfood
or smoothness under heavy disk load. Package startup checks only five seconds
alive on a generated empty root.

These are portable archives, not finished installers. The Mac archive requires
Apple Silicon and macOS 26 or later, with ad-hoc signing rather than Developer ID
notarization. Linux targets Ubuntu 24.04 with X11/XWayland. Wider previews,
thumbnails, indexed folder facts, search setup, everyday transfer fidelity,
accessibility completion and interface refinement remain open.

This record and copied evidence were reviewed for accurate scope, explicit
status, source identity and distinction between synthetic verification and
physical dogfood. No implementation-style certification is added here; each
source slice retains its own exact review record. Existing archive-verification
helper code was used unchanged and is not newly style-certified by this record.
