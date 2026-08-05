# Future Oracle task handoff

Do not use this prompt until `DELIVERY_SEQUENCE.md` Gate O0 is satisfied and the
grand architect opens implementation.

> Work only in `/Users/quentinkuttenkuler/file_manager/orchestrator`. Read its
> `AGENTS.md`, every routed planning document, the master contract registry, and
> the accepted root ADRs before editing. Verify Gate O0 against named GUI.Forms
> and engine snapshots; if it is not satisfied, improve paper contracts and
> fixtures only—do not create Rust code.
>
> The component is The Oracle: a Rust control plane and interoperability
> authority, not a monolithic replacement for GUI.Forms, the Go engine, the C++
> frontend, Kolmogrov, or sandboxed plugins. The semantic contract registry is
> authoritative. No implementation may invent an unregistered boundary or pass
> native Rust/Go/C++ layouts across processes/languages.
>
> Begin with Gate O1: common types and error vocabulary, lifecycle state machine,
> fake peers, deterministic message traces, capability evaluation, quotas,
> deadlines, cancellation, backpressure, restart, and hostile fixtures. Use no
> real plugin, database, model, platform integration, or attractive bulk codec
> until those semantics are executable and cross-language fixtures exist.
>
> Preserve the reef: third-party code is always outside trusted processes;
> plugins receive scoped handles and publish bounded data; hives and registries
> are Oracle-owned; exact engine facts and user-authored memory cannot be
> overwritten by provider claims; file mutation is not smuggled through search,
> hive, handler, or command APIs.
>
> At each milestone report contract IDs changed, provider/consumer sides,
> compatibility effect, generated artifacts, commands/tests, hostile failures,
> resource measurements, rejected alternatives, and the next gate. Stop rather
> than weakening a contract or silently coupling to a subproject's private
> implementation.
