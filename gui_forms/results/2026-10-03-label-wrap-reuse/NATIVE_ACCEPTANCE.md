# Paragraph width reuse: native acceptance

**MEASURED:** source `37510a27d4db8ca353113dd3c5ee89a7cff0a936` passed
Windows x64, macOS arm64, Linux x64 and house-style checks in both
[push run 37120369490](https://github.com/falseywinchnet/file_manager/actions/runs/37120369490)
and [PR run 37120372054](https://github.com/falseywinchnet/file_manager/actions/runs/37120372054).
The original source `7ce3db1b7add076ad9a22f9f60eb8e6f6ced08e3` had also
passed both native matrices before the ancestry-only rebase. Their complete
source tree is identical: `4b8c3e9d1e5971d4fd189f38becb2cb5038b726d`.

Root fetched and checked the current explicit PR30 merge ref before merging.
The rebase-merged main commit `11116c2da5121a9da8a3c5e011b15a3bc8e3e67e`
has that same tree. This records integration/correctness, not native-platform
timings or physical-input dogfood. The benchmark claims remain limited to the
workloads and Windows environment in the README.

The published `v0.001-alpha.31d1319` predates this change. A later private-batch
CI run exposed an existing Engine status-sampling race; its repair and package
validation are being completed before the next coherent dogfood release.
