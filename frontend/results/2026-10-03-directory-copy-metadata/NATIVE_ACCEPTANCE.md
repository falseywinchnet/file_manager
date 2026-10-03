# Directory-copy metadata: native acceptance

**MEASURED:** tested source `c5fb4c69fa27bf323ba33f100ce5b151e84b7e5c` passed
Windows x64, macOS arm64, Linux x64 and house-style checks in both
[push run 37117160449](https://github.com/falseywinchnet/file_manager/actions/runs/37117160449)
and [PR run 37117197453](https://github.com/falseywinchnet/file_manager/actions/runs/37117197453).
This supersedes the pending native verification in this directory's README.
The Unix no-owner-write source-directory fixture now has native execution
evidence; the Windows-only local run did not establish that branch.

Root fetched PR28's explicit proposed merge ref and verified its complete tree
against the tested source. After rebase merge, main
`d08e45c4b91d8dc3c62a790bf36d884ddbf20f5b` again had the identical tree
`cdcb35f33a037bb7d5ef0c5e835c52c03ab1a879`. An initial check using the ambiguous
multi-ref `FETCH_HEAD` selected main instead of the PR merge ref and stopped
before mutation; the subsequent comparison used the explicit stored PR ref.

The latest published `v0.001-alpha.31d1319` predates both regular-file and
directory metadata repairs. This record is source/native-test acceptance,
not an independent package verification or physical dogfood claim. Complete
metadata fidelity, read-only-stage failure cleanup and ordinary Copy admission
remain unfinished.
