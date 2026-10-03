# Fixed-size Label measurement: native acceptance

**MEASURED:** tested source `95b76f6dec3f6ee436ab95acf40f629ee11d3d77` passed
Windows x64, macOS arm64, Linux x64 and house-style checks in both
[push run 37118311527](https://github.com/falseywinchnet/file_manager/actions/runs/37118311527)
and [PR run 37118326978](https://github.com/falseywinchnet/file_manager/actions/runs/37118326978).
This supersedes the pending native integration statement in the README. The
native runs are correctness/integration evidence, not cross-platform timings.

Root verified the explicit PR29 merge ref against the tested source before
rebase merge. Main `b5c257854a2c31a3eea870601065aa80678d7bfd` has the same tree
`c099cac529d134b1201cc9a2df0ef2f0ca5ce525`. No source change was introduced by
the merge. The separately measured wrapping reuse change is in PR30 and is not
part of this acceptance.

The latest published `v0.001-alpha.31d1319` predates this repair. Package
publication and physical-input dogfood remain separate evidence requirements.
