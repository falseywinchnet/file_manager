# Private prepared-window session: native acceptance

**MEASURED correctness:** source `0372733898fdf16889ee90f9004b977d1e51438b`
passed all Windows x64, macOS arm64, Linux x64 and house-style spelling checks in
[push run 37122027342](https://github.com/falseywinchnet/file_manager/actions/runs/37122027342)
and [PR run 37122068315](https://github.com/falseywinchnet/file_manager/actions/runs/37122068315).
The private session suite passed on each platform: Windows 0.03 s, Mac 0.01 s,
Linux 0.02 s in the push run. These are test durations, not latency guarantees.
The development stage ran nine suites on Windows and twelve on Mac/Linux;
separate normal-SDK, frontend and native-host stages also passed.

After PR31's rebase merge, changing PR32's base exposed an ancestry conflict.
Root checked that its old parent and new main had exactly the same source tree,
then reparented the session commit without changing any file. The resulting
commit was `2b511bb55dd6de096e094d518621f8565019d55b`. Root fetched the fresh
PR merge ref and checked both parents, rather than accepting the stale merge
ref from the previous base. Original, reparented and fresh merge-ref trees
were identical: `2b87aa66ebd3272b20d38c58734ec567880cb255`.

PR32 was rebase-merged as `568d7fbde2cc173cb5f857431aec53400ee1b550`, with
that same complete tree. Acceptance used the completed matrices for the exact
original source tree; new ancestry-only runs were still pending at merge time.
No passing result is inferred for those later runs.

The normal installed frontend and SDK do not activate this development-only
session. Native readiness, controller delivery, retained batch rendering and
physical-input verification remain open. See the
[package delivery record](../../../frontend/results/2026-10-03-prepared-preview-delivery/DELIVERY.md).
