# Selection and Copy path delivery

Published prerelease:
[v0.001-alpha.9042dd4](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.9042dd4).
Release ID `402865646`; published 2026-10-04 05:55:15 UTC.

The application now presents observed multi-selection counts/common facts and
regular-file logical-size totals, and Copy path publishes the complete selected
path list. Missing/zero/overflow/folder-size states remain explicit. Copy path
does not copy file contents and does not introduce shell quoting. Prior adaptive,
demand-driven UTF-8/PNG previews and four-column Details remain included.

**MEASURED:** exact clean source `9042dd490e2226d385bdbb74d9883b5cddb19be7`
passed all four checks in native push run
[37180370974](https://github.com/falseywinchnet/file_manager/actions/runs/37180370974)
and PR run
[37180373518](https://github.com/falseywinchnet/file_manager/actions/runs/37180373518).
Windows passed 15 frontend suites, Mac 16 and Linux 14. Raw frontend logs are
retained here. The Mac harness invoked the real Commands / Copy path menu and
independently read three exact selected paths from NSPasteboard. Its native
selection capture was visually inspected; physical input and the owner's Mac
remain separate evidence. The long Location value still clips.

PR45 merged as `0842bdaffcbfa5d0546ba3cd21ffc914cd4a58cd`, identical to its
tested source `76af0ee8e80ab837f6eef4c3e19cd994d514035b`. PR46 was then rebased
without changing its entire tree and merged as
`a3f7732a8cabcd54a54299c53d7e79a2ff03d0e2`. Tested and final merged tree:
`5be044dbf538ff355cf20c19405a0ffc5a400133`.

Independent archive verification checked source identity, clean-source receipt,
archive SHA-256 sidecars, all 44 Windows / 42 Mac / 40 Linux receipt-listed file
hashes, and Unix executable permissions. Before publishing the draft, all six
uploaded archives/sidecars matched local size and SHA-256. The report and
published asset receipt are retained here. Existing verification tooling was
used unchanged; this record does not newly certify its source style.

| Archive | SHA-256 |
|---|---|
| Windows x64 | `bb97cc7018cf33a344f9a92843c6d13b2700d45d5cdcee73aff4cee715125cfd` |
| macOS arm64 | `1f3846c3f82feca7361322926a4ccce61292e0af0f117405cd930fc30910b433` |
| Linux x64 | `d867deefe34fc1b95c7ed9458af17dbb7a193c5d4c90ffa38a01cb53ff728bb4` |

Each package stayed alive for five seconds on a generated empty root. These are
portable alphas, not completed installers. Mac requires Apple Silicon/macOS 26+
and is ad-hoc signed, not notarized; Windows is unsigned x64; Linux targets
Ubuntu 24.04 with X11/XWayland and its named system dependencies.

Ordinary New Folder, same-parent Rename and one-step Undo remain available.
Ordinary Copy/Move/Delete, broad preview formats, indexed thumbnails, dynamic
folder aggregates and automatic search setup remain unfinished. The research
decoder/supervision executables are not activated as application preview formats.
No whole-application responsiveness claim follows from these correctness checks.
