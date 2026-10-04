# Native admission measurements

**MEASURED** by the installed-Core consumer at source
`6105a31bcf9f7faf8c4a1fecfa986729683e14e5` in push run
[37182015418](https://github.com/falseywinchnet/file_manager/actions/runs/37182015418).
The separate PR run
[37182017056](https://github.com/falseywinchnet/file_manager/actions/runs/37182017056)
also passed the complete house-style/Windows/macOS/Linux matrix. Raw samples
here come from the push run only. Both geometry orders and all three format
controls passed the ownership, stale-ID and logical-retirement checks.

The rebase and final merge `0a2e47dfceba1af1a66eda4c2c485b360affd21e` retain
the identical complete tree `7aef45861227bb2c9573da44e3ea52c90c05de58`.
Each downloaded source receipt records a clean checkout. Compiler excerpts are
from each probe's CMake configure log; the complete logs remain in the linked
run's diagnostic artifacts. Build mode is Release, C++20 with extensions off.

| Runner / compiler | Diagnostic artifact ID |
|---|---:|
| windows-2022 x64 / MSYS2 GCC 16.2.0 | 11295892183 |
| macos-26 arm64 / Apple Clang 21.0.0 | 11295279437 |
| ubuntu-24.04 x64 / GCC 13.3.0 | 11296145191 |

For the 1024 by 1024 generated solid-color control, median milliseconds are
shown as forward / reverse geometry order:

| Runner | Raw BGRA, 4,194,304 bytes | Stored PNG, 4,195,716 bytes | Deflated PNG, 6,504 bytes |
|---|---:|---:|---:|
| Windows | 5.964900 / 5.962700 | 16.101600 / 16.082900 | 0.025700 / 0.025700 |
| macOS | 7.324000 / 7.491584 | 22.831042 / 21.560708 | 0.033625 / 0.034041 |
| Linux | 5.228289 / 5.244102 | 14.771565 / 14.800824 | 0.023051 / 0.023137 |

These are synchronous admission times, excluding PNG decode, renderer image
synchronization, drawing, input scheduling, source I/O and IPC. They are not
comparative hardware scores: runner hardware and shared-runner load differ,
and this run did not record CPU model or utilization. Raw admission consumes
several milliseconds even before paint, while the highly compressible PNG
control defers much of its eventual work. Neither result selects a product
transport or proves complete preview responsiveness. The forced-native-paint
consumer is the next separate measurement.

The workload, sample ordering, percentile method, ownership semantics and
limitations are in the [experiment protocol](../../../experiments/preview_admission/README.md).
`sha256.json` fingerprints the copied raw logs, source receipts and compiler
excerpts. Earlier Shadow measurements remain in the parent evidence directory.
