# Independent operation worker: native acceptance

**MEASURED:** all checks passed for tested source
`31d131949cf80ad82ea125ce1fa440c53ad199af` in both
[push run 37115417394](https://github.com/falseywinchnet/file_manager/actions/runs/37115417394)
and [PR run 37115442122](https://github.com/falseywinchnet/file_manager/actions/runs/37115442122).
Each matrix passed Windows x64, macOS arm64, Linux x64 and house-style spelling
regression checks. The native jobs include frontend, native-host and package
startup checks. This receipt records Actions results; their archives have not
yet been independently downloaded and promoted as a release for this source.

[PR26](https://github.com/falseywinchnet/file_manager/pull/26) was rebase-merged
as `793d326bf4e6f2d9b0ca7ed068a6db5a3489c242`. Root fetched main and verified
its tree is exactly the tested source tree,
`cdc6cbc0de8555459afc81192df566d782e58261`.

This supersedes the pending native-matrix statement in this directory's README.
It does not establish physical-input acceptance, latency under disk contention,
ordinary transfer admission or complete read-worker responsiveness. The following
Copy modification-date and directory-metadata repairs have independent source
revisions and evidence.
