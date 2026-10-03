# Native delivery — New Folder and search source records

**MEASURED:** source `d4dcb2450e1d566cc218810edb99e7fb3c7d9d50` passed both
native workflows: push `37103936383` and PR `37103959771`, each with Windows x64,
macOS arm64, Linux x64 and house-style jobs. PR #18 rebase-merged as
`2be38d5d5dddc550b880fe935d68e5624bd57561`. Root fetched the merge and verified
complete tree equality `ca0c5a05e266da2bbf183429ed762e5a1dbab53e`.

Release: [v0.001-alpha.d4dcb24](https://github.com/falseywinchnet/file_manager/releases/tag/v0.001-alpha.d4dcb24).
It includes the preceding New Folder naming change. It does not include the
subsequent bounded-reader experiment or reserved substring-cursor guard.

Root independently downloaded all three push-run archives, checked clean source
receipts and SHA-256 sidecars, and verified every receipt-listed file and packaged
executable: 40 Linux, 42 Mac, 44 Windows. The committed independent verification
JSON retains exact source/SDK hashes and the package-startup qualification.
GitHub's published archive digests agree with these local hashes:

| Archive platform | SHA-256 |
|---|---|
| Linux x64 | `1990b118ea21a9a1de0af132171237d32da1fe2451ca17830f77c45af1314410` |
| macOS arm64 | `598e7e97ee0fc0e52b9ba3d7faa821654b7fb72f028816a49168cae79f7cdc61` |
| Windows x64 | `05becc7194acbfa19b2eadd9eca33b259335a3138ccaee71a754e8116ca4ce46` |

The archived Mac application images were opened and inspected: the text preview
and separate coverage caption are visible, and Details shows Name, Type, Size
and Date modified headers with intact selection. The test log records 16/16
native frontend suites, including source-projection checks and real Mac window
preview/Details probes. These images do not show physical-keyboard New Folder
naming, active copy cancellation, or an indexed search session.

Portable alpha only: arm64 macOS 26+ ad-hoc signed, Windows x64, and Linux x64
with the documented Ubuntu 24.04/X11 or XWayland/system-library baseline.
Normal browsing is read-only; explicit mutation/search profiles are documented
in the packages. No installers, Developer ID/notarization, broad-format previews,
thumbnails, folder aggregates, accelerated catalogue substring search, complete
everyday operations or whole-product smoothness are claimed.
