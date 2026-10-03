# Regular-file modification dates: native acceptance

**MEASURED:** tested source `5d86e949ad9f05c250615817997250380cc97926` passed
Windows x64, macOS arm64, Linux x64 and house-style checks in both
[push run 37116072831](https://github.com/falseywinchnet/file_manager/actions/runs/37116072831)
and [PR run 37116096848](https://github.com/falseywinchnet/file_manager/actions/runs/37116096848).
This records completed Actions results, including the frontend suites; it is
not an independent archive or physical-input acceptance claim.

After PR26 merged, root retargeted
[PR27](https://github.com/falseywinchnet/file_manager/pull/27) to main and fetched
its proposed merge commit. That merge tree exactly equaled the already-tested
`5d86e94` tree. PR27 then rebase-merged as
`8dd25cb929c1f5965598a790a543a1518b75ac44`; root fetched main and again verified
exact equality with the tested tree. No source changes were introduced by
retargeting or rebasing.

This supersedes the pending native-code verification in this directory's README.
The published `v0.001-alpha.31d1319` checkpoint predates this repair and does not
include it. Complete metadata fidelity, ordinary Copy admission and the separate
directory-copy repair retain their own work and acceptance requirements.
