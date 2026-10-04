# Native selection-facts inspection

**MEASURED:** macOS arm64 job `111369495986` in
[run 37179652389](https://github.com/falseywinchnet/file_manager/actions/runs/37179652389)
passed at clean source `76af0ee8e80ab837f6eef4c3e19cd994d514035b`.
All 16 frontend suites passed, including the actual AppKit harness's three-file
count, logical-size total and mixed-type assertions. Copied evidence includes
the source receipt, frontend test log and `native-selection-facts.png`.

**OBSERVED visual inspection:** the primary thread opened the native capture.
Three selected rows remain highlighted; the inspector shows three selected
objects, three files, mixed kinds, 64 KB in files and multiple dates. These facts
are visible in the PropertyList with preview collapsed. No full-interface visual
acceptance is claimed: the long Location value clips at the right edge, and the
verification editor remains present for a multiple selection. Physical input,
the owner's Mac and large-folder responsiveness are unverified by this capture.

The harness also passed existing text/PNG/unsupported preview, four-column Details,
synthetic header sort, and ordinary New Folder/Rename/Undo checks. This is not
broader preview-format, thumbnail, recursive-folder-size or ordinary transfer
acceptance. Both full native runs (37179652389 and 37179654337) subsequently
passed Windows, Mac, Linux and house style. The combined portable release is
recorded in `../2026-10-04-copy-path/DELIVERY.md`.
