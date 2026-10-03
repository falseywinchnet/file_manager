# Copy progress: native acceptance follow-up

**MEASURED:** all checks passed for tested PR25 head
`8da1f66140e3b06d620a7681b98ed0227b7cfdb0` in
[push run 37113768690](https://github.com/falseywinchnet/file_manager/actions/runs/37113768690)
and [PR run 37113771209](https://github.com/falseywinchnet/file_manager/actions/runs/37113771209):
house-style spelling regression checks, Windows x64, macOS arm64 and Linux x64.
The native jobs include the complete frontend suite and packaged startup checks.

Root downloaded the Mac and Linux evidence from the push run and inspected
their frontend CTest logs. Both passed all 16 suites, including native empty,
small and multichunk copies, bounded monotone progress, contents/identities,
progress-driven partial cancellation, observer failure and stage/close checks.
Windows job success is recorded by Actions; its artifact was not independently
downloaded during this follow-up.

[PR25](https://github.com/falseywinchnet/file_manager/pull/25) was rebase-merged
as `b328529aa9cb7074221136c801b23831c53e68df`. Root fetched main and verified
the merged and tested trees both equal
`6e94b4f7444a375a1fa92dd30651ec24c0dc947e`.

This supersedes the earlier pending native-build statement in this directory's
README. It does not establish physical-input dogfood, ordinary Copy admission,
metadata fidelity or smoothness under disk load. No release was promoted for
PR25 alone; the preceding `v0.001-alpha.fc6312c` release excludes these changes.
