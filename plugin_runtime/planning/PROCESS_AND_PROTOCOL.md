# Process isolation, IPC, ABI, and protocol plan

Status: **candidate dossier**.

## Candidate topology

```text
C++ GUI / Go indexer / CLI
          |
   narrow host adapter
          |
  Rust supervisor (trusted policy and validation)
          |
  authenticated local channel
          |
 disposable or pooled worker
          |
 native adapter OR WASM guest
```

The supervisor may ultimately be an executable service or a per-client helper.
That choice is not decided. It must not render UI or interpret File Manager
control state.

### Worker-lifetime candidates

| Candidate | Gain | Loss/risk | Rejection gate |
|---|---|---|---|
| One process per job | strongest state reset; simple kill semantics | cold-start and memory cost | reject if representative p95 cannot fit interaction budget after prewarm experiments |
| One process per plugin | amortized runtime/model load | cross-job leakage, stuck global state | reject on leakage fixture, irreclaimable memory growth, or cancellation failure |
| Small quarantined pool | bounded concurrency and amortized startup | most supervisor complexity | reject if benefits do not exceed process-per-job by declared budget |
| In-process WASM instance | low process cost with language-neutral guest | runtime attack surface and imperfect OS-resource isolation | never use as sole boundary until escape and resource tests pass; native guests excluded |

**CANDIDATE layered policy:** OS-sandboxed worker for every guest; WASM may be an
additional inner boundary for portable plugins. This is not yet selected.

## OS enforcement map

### macOS candidates

- App Sandbox/XPC service with minimal entitlements and supervisor-transferred
  file access; no network entitlement by default.
- Security-scoped URL/bookmark transfer only when persistent user-approved scope
  is actually required; prefer ephemeral descriptors/streams for one job.
- Separate signed helper profiles for contracts whose entitlements differ.

**OBSERVED:** Apple documents App Sandbox as kernel-enforced damage containment
over files, network, and resources, and documents sharing selected file access
between processes. Sources:
[App Sandbox](https://developer.apple.com/documentation/security/app-sandbox),
[file access](https://developer.apple.com/documentation/security/accessing-files-from-the-macos-app-sandbox).

Gate: prove the intended non-App-Store distribution/signing path can launch each
profile and that denied access remains denied on every minimum supported macOS.

### Windows candidates

- AppContainer or LPAC process token, capability SID ACLs for staged input, Job
  Object for process/memory/CPU limits and tree kill, restricted handle list,
  mitigation policies, and win32k denial for headless workers.
- Named pipe or ALPC-family local channel authenticated against expected process
  identity; no inherited ambient handles.

**OBSERVED:** Microsoft describes AppContainer as a security boundary restricting
files, processes/windows, devices, registry, network, and credentials; LPAC is
stricter. Source: [Launch an AppContainer](https://learn.microsoft.com/en-us/windows/win32/secauthz/implementing-an-appcontainer).

Gate: test unpackaged desktop deployment, minimum Windows version, file-handle
transfer, process-tree death, loopback denial, and absence of unintended broker
capabilities. New experimental APIs may be measured but cannot be the sole path
without support-lifetime evidence.

### Linux candidates

- New user/mount/PID/network namespaces where available; `no_new_privs`;
  seccomp-BPF syscall allowlist; Landlock filesystem/network restrictions;
  rlimits/cgroup v2 for resources; sealed `memfd`/Unix socket/pipe handles.
- A portal or distribution-specific sandbox (for example, Flatpak) may add a
  layer but cannot be the only portable Linux contract.

**OBSERVED:** current kernel documentation defines Landlock as unprivileged,
stackable restriction of ambient filesystem/network rights, inherited by
descendants, and seccomp-BPF as syscall filtering. Sources:
[Landlock userspace API](https://docs.kernel.org/userspace-api/landlock.html),
[seccomp filter](https://docs.kernel.org/userspace-api/seccomp_filter.html).

Gate: runtime-probe every primitive. If the minimum contract cannot be enforced,
disable third-party execution rather than silently degrade. Distribution/kernel
coverage must be specified before 1.0.

## Host bridges

### C/C++

**CANDIDATE:** a small `extern "C"` supervisor client ABI with opaque handles,
fixed-width integer types, explicit buffer ownership, status codes, caller-owned
allocators or paired free functions, and asynchronous completion through a
pollable queue—not foreign-language callbacks on arbitrary threads.

No Rust enum layout, `Vec`, `String`, panic, allocator, or unwinding crosses the
boundary. Header generation is checked into release artifacts and ABI-tested.

### Go

Candidates:

1. Go speaks the authenticated local protocol directly to a supervisor process.
2. cgo wraps the same C ABI used by C++.

Direct protocol avoids cgo/runtime entanglement; C ABI shares one client
implementation. Measure startup, cancellation, packaging, race behavior, and
cross-compilation before choosing.

### Plugin SDKs

The Rust guest SDK should be convenience over generated protocol types, not the
protocol definition. Future C/C++ guest SDKs can target native workers; a WASI
component SDK is a separate candidate. SDK version and protocol compatibility
must be independently testable.

## Logical protocol, independent of codec

Every frame has:

```text
magic, framing_version, header_length, payload_length,
contract_id, contract_major, contract_minor,
message_kind, flags, request_id, nonce,
plugin_instance_id, sequence, payload_digest
```

Rules:

- read a fixed small header before allocating payload;
- reject overflow, duplicate fields, invalid UTF-8 where text is required,
  unknown critical flags, out-of-order states, and messages above negotiated
  maxima;
- separate control and bulk-data channels so a full pixel stream cannot block
  cancellation or heartbeat;
- one terminal response per request: success, denied, unsupported, invalid
  input, resource limit, timeout, cancelled, crashed, protocol fault;
- cancellation is idempotent; deadlines are supervisor-authoritative;
- protocol minor versions may add optional fields; incompatible semantics need
  a major contract; negotiation chooses the intersection, never the newest
  unilaterally;
- plugin manifest schema, package format, host ABI, framing, contract, and
  extension schemas all carry independent versions.

### Codec candidates

| Candidate | Strength | Risk/question |
|---|---|---|
| Protobuf | mature evolution/tooling across Rust/C++/Go | canonicalization and unknown-field policy must be made explicit |
| Cap'n Proto | low-copy model and mature multi-language bindings | verifier/limits and platform packaging need hostile-input evaluation |
| FlatBuffers | random access, generated types | verifier use is mandatory; mutation/ownership ergonomics differ |
| bounded CBOR with generated schema | inspectable/flexible and good lab codec | generic decoders and canonical forms risk ambiguity/allocation |
| WIT component contracts | natural for WASM components | does not replace native worker IPC and runtime is another TCB |

JSON is permitted only as a human-inspectable laboratory codec and fixture
format. It is not a production candidate for bulk previews.

## Preview surface transport candidates

1. **Encoded PNG:** simple cross-platform output with a narrow decoded result;
   extra decode/copy and invocation of the core PNG path remain costs.
2. **Validated shared raster:** worker writes a supervisor-created, fixed-size,
   sealed/shared buffer; supervisor validates dimensions, stride, format,
   premultiplication, color space, and byte coverage before host sees it.
3. **Chunked raw raster stream:** simple ownership; higher copies and IPC volume.
4. **Platform IOSurface/D3D shared surface/dmabuf:** low-copy potential but
   increases per-platform API, GPU, synchronization, and handle attack surface.

Initial experiment order: raw stream baseline, shared CPU raster, PNG. Platform
GPU surfaces are deferred until a measured copy bottleneck exists. Plugins never
receive a GUI.Forms canvas, native window handle, display-server connection, or
arbitrary display list. Vector/structured preview formats require separate
parsers and are not implied by raster support.

## Lifecycle state machine

```text
discovered -> verified -> installed -> disabled
                         -> enabled -> spawning -> handshaking -> ready
                                                   -> busy -> ready
                                                   -> draining -> stopped
                                                   -> faulted -> quarantined
```

Every transition is journaled locally with redacted identifiers. A package
change invalidates running instances. Protocol violations terminate the worker;
retries never reuse the same input/output buffer state.
