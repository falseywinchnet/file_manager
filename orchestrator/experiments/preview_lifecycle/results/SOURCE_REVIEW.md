# Independent lifecycle source reviews

The authorized visible audit sibling performed read-only review against the full programming house style. The raw initial report and final workflow/line-ending follow-up follow in order.

**Scoped source acceptance: no blocking state-machine, ownership, contract-authority, or house-style finding in the inspected lifecycle laboratory.** I found no path that accepts an obsolete ticket or prematurely reuses the process slot under the documented host-ordering contract.

The important transitions are consistent:

- **Replacement during `Running`** issues one stop effect and enters `Stopping`. Further selections replace only the client’s pending record. They neither issue repeated stops nor overwrite the original stop reason.
- **`Stopping` retains the global slot** until the matching ticket is reaped. A wrong ticket cannot release it, and a matching reap discards pixels regardless of the supplied completion.
- **Reaping a running request** checks the trusted source observation and raster descriptor before creating an offer. Decode failure, invalid output, stale source, and acknowledgement-clock overflow return the slot to `Idle`.
- **Offer expiry** retires the offer and emits its terminal reason. Late or wrong acknowledgements cannot acknowledge another request or free its slot.
- **Disconnect/reconnect** can reuse a window number, but subsequent requests receive different nonces. A prior stopping request must still be reaped before dispatch resumes.
- **Deadline arithmetic** is checked before mutation. Backward time is rejected. Dispatch-time overflow preserves the pending request and leaves the slot idle; post-reap expiry overflow discards the result rather than stranding the slot.

The fixed-storage and accounting claims are supported by the source. There are eight client slots, at most one pending record and one display descriptor per slot, and one running/stopping/offered slot. Descriptor validation bounds dimensions before multiplication; eight accepted displays cannot exceed the model’s 32 MiB accounting limit. Model transitions allocate no dynamic storage.

The **effects boundary is explicit but external**. The broker may enter `Idle` before a host physically releases pixels or grants; the draft correctly requires retirement effects to execute before replacement admission. Likewise, a reap event is a host assertion of completed exit/cleanup—not something this crate verifies. The laboratory cannot prove effect execution, descendant cleanup, actual memory release, authentication, or timely clock advancement. These are appropriately retained as adapter obligations rather than claimed capabilities.

The clock rule is also an important precondition: expiration wins at equality **when the host advances the clock before processing the event batch**, as documented. The API does not independently consult time inside `reaped()` or `acknowledge()`. I found no contradiction between that model and the draft’s stated evidence limits.

The ten tests exercise meaningful behavior beyond implementation mirroring: repeated replacement while stopping, wrong-session tickets, exact deadline and expiry boundaries, late acknowledgements, source/raster refusal, reconnects, maximum display accounting, round-robin progress under a 10,000-update selection storm, and clock exhaustion. They do not execute effect owners or prove physical allocation release, and the documentation does not say otherwise. Nonce exhaustion is source-checked through `checked_add`; it is not directly exercised by the current public-API fixtures.

The authority language is supported by the repository:

- [ADR-003](C:/Users/Shadow/file_manager/decisions/ADR-003-ORCHESTRATOR-INTEGRATION-AUTHORITY-AND-BOOTSTRAP.md:107) opens provider-independent kernel and laboratory work.
- [B0](C:/Users/Shadow/file_manager/orchestrator/planning/DELIVERY_SEQUENCE.md:6) is open.
- The [contract ladder](C:/Users/Shadow/file_manager/orchestrator/planning/CONFORMANCE_AND_VERSIONING.md:52) permits a disposable executable laboratory for a fixture draft.

The registry and negotiation additions preserve that limited status. They do not assign a complete contract, advertise a worker, admit a codec, or open source-read/transport/provider authority.

I applied the full [house style](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md) using its language-neutral requirements. The Rust uses named operations, explicit state records, initialized storage, checked counters, immutable copied fixture data, exclusive mutation through `&mut self`, and no retained borrows, closures, foreign callbacks, or hidden scheduling. Rust enum destructuring and inert-value derives are not C++ spelling violations. No concrete remaining violation was identified in the reviewed scope.

**Exact reviewed hashes**

| File | SHA-256 |
|---|---|
| `orchestrator/experiments/preview_lifecycle/Cargo.toml` | `74071C127311D3B056919F7B0590892B57165C6E6A3AC95C380182DE646575C2` |
| `orchestrator/experiments/preview_lifecycle/src/lib.rs` | `414A1C9625FAE3C2247DAC074C1B74A2D487BC7B789F3EEC4090D80947F3FC5B` |
| `orchestrator/experiments/preview_lifecycle/tests/lifecycle.rs` | `64A9FDF809A5CD3BF77364A71A1BE54142B7078F05F85CB718B016FDAAAB303E` |
| `orchestrator/experiments/preview_lifecycle/README.md` | `74033B27825A10C6B3E2285E5A495A097A17F0F60C3E50E10907ED63D4008C5D` |
| `.github/workflows/preview-lifecycle.yml` | `D88C95E7105DE52EFC5DE6C3FF8CD584152A19F2EB5C4B9AA33F47C297F27AEF` |
| `orchestrator/spec/SELECTED_PREVIEW_LIFECYCLE_DRAFT.md` | `1808729D58BD7778FD840076566DDC32527B9BBDBF246EC2E4157F23D6ED07B7` |
| `orchestrator/spec/CONTRACT_REGISTRY.md` | `135FC7DC043B67AD0BA2FCEA4BA47B58CBA7910D188130045478431E6E7027F1` |
| `orchestrator/negotiations/FILE_MANAGER_PREVIEW_2026-10-04.md` | `0CDD5549310D3C3235F7B299ACF1B413417B9E48E96A21D40C2A124958C471E9` |
| `frontend/planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md` | `286CCE63CF05EA4FB1DC4F2D5E09515AC5FC08C3E3E2EF720E9EF09DF51253DF` |

For the three broader registry/negotiation files, acceptance covers the lifecycle additions, not unrelated historical content. The reported test and Clippy passes are root’s evidence; I ran neither. No edits, builds, Git operations, or workers were used.

---

**Final workflow accepted; the prior lifecycle source acceptance remains valid.** No new blockers or house-style findings.

I verified that the current `lib.rs` and `tests/lifecycle.rs`, normalized to LF **in memory only**, hash exactly to the snapshots from the initial review. Their changes are line-ending normalization, not semantic edits.

The revised workflow explicitly installs **Rust 1.87.0** with `rustfmt` and `clippy`, then uses `cargo +1.87.0` for formatting, tests, and Clippy on all three platforms. This checks the declared minimum Rust version instead of relying on the runner’s default toolchain. Locked/offline dependency resolution, the two-job limit, and the isolated `.build/` target directory remain intact. Workflow execution is still separate evidence.

| Final reviewed file | Current SHA-256 |
|---|---|
| [src/lib.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_lifecycle/src/lib.rs) | `2413E8181D54CDA18FC24E31A105CDC6147DDF0D84ABA039D5D13AC2B68CB8BB` |
| [tests/lifecycle.rs](C:/Users/Shadow/file_manager/orchestrator/experiments/preview_lifecycle/tests/lifecycle.rs) | `469853BEEC36CF5F792E5D6BBE63CE140EF74E361A60A905A49AB5C5EB3A382A` |
| [preview-lifecycle.yml](C:/Users/Shadow/file_manager/.github/workflows/preview-lifecycle.yml) | `E8D6D9B534E35ED9012D59C06561C700C6E51DB154E497BF81D04F5046873F45` |

No edits, builds, Git operations, or workers were used.
