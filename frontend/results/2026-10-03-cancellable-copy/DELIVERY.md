# Native verification and delivery

**MEASURED:** Both complete native matrices passed at
`137151409ce75d106ffad059fe21f66f76f8d55b`:

- [Push 37101430324](https://github.com/falseywinchnet/file_manager/actions/runs/37101430324)
- [PR 37101432514](https://github.com/falseywinchnet/file_manager/actions/runs/37101432514)

[PR 16](https://github.com/falseywinchnet/file_manager/pull/16) rebase-merged as
`631ea56db5adb326f48ff9d960360afb108abcab`. Complete tested and merged trees match
`d0b4c026f45f1e9403cc4dd839070ab0bb1d3a89`.

The macOS test output explicitly passes post-write cancellation, contained
callback exceptions, ordinary/read-only permissions, and both metadata cases:
ordinary xattr absent in baseline and candidate; generated quarantine present
and equal in both. Neither case was skipped as UNMEASURED. This closes the
native helper adoption gate recorded in the initial receipts; it does not
select a broader metadata-preservation policy.

[Release v0.001-alpha.1371514](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.1371514)
contains Windows x64, macOS arm64 and Linux x64 portable packages. Independent
download verification checked clean source receipts, all archive checksum
sidecars and all 44 Windows / 42 Mac / 40 Linux listed files. Initial verification
correctly refused the two-archive download before the Windows artifact was
available; downloading the completed Windows artifact and rerunning verified
all three. No checksum or receipt rule was relaxed.

The saved native Mac text-preview and Details screenshots were inspected.
Text/caption and four headers are visible. Those images do not show active
cancellation: its current evidence is the semantic/layout application test,
not a native visual or physical-input acceptance claim.

Archive SHA-256:

| Platform | SHA-256 |
|---|---|
| Linux x64 | `7c50a49e732a703017259c381562c5d7733c56549095bcc57f5bb8cbe9c622b1` |
| macOS arm64 | `64d6ca86792def7bdd07b00bfdb6de7a0018a807f420ec214459b4d779cca62f` |
| Windows x64 | `b3b1bb810ec6ca08ed65a165443a817f32c51e58068067b0d0baf12102dbd5d1` |

This supersedes the rename/no-replace release `v0.001-alpha.21444bb`, which was
also published after both complete matrices and independent archive checks.
Both releases retain the portable-alpha, protected-mutation, macOS 26+ arm64
ad-hoc-signing and documented Linux baseline limits. Neither contains the later
New Folder naming change. Byte progress, native active-cancellation screenshots,
hard I/O deadlines, metadata policy and wider product acceptance remain open.
