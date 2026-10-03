# Catalogue and observation capture: native acceptance

**MEASURED correctness:** repaired source
`8d49f19956e1e5ef1720d25956c8ebd57df19b1f` passed Windows x64, macOS arm64,
Linux x64 and spelling checks in both
[push run 37121817123](https://github.com/falseywinchnet/file_manager/actions/runs/37121817123)
and [PR run 37121819280](https://github.com/falseywinchnet/file_manager/actions/runs/37121819280).
Root inspected the native service-test and private batch-shaping results.
The failed earlier Linux run and deterministic rejected control remain retained
in `../rejected/2026-10-03-status-sampling/`; these passes do not erase them.

PR31 was rebase-merged as `37b4708bb5960ce90e36629c7294b05973a9e223`.
Root verified the tested head, fresh PR merge ref and merged main all had tree
`c65f8dc8753701a37b3f56a6fac544de28175143`. This also accepts the private batch
shaper in the same PR, without claiming public GUI availability or throughput.
The full Windows race run and semantic house-style scope are in the README.
The native workflow runs ordinary Go correctness tests; it is not evidence of
native-platform race-detector runs. Administrative root-transition and separately
sampled lifecycle-counter boundaries remain as documented there.
