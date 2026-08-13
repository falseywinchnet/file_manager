# Independent C++ Orchestrator client

Status: **OBSERVED Unix/macOS Core 1.0 conformance consumer; Windows transport
projection remains open**.

This C++17 library and probe executable implement ADR-009 without linking Rust,
sharing Rust layout, or depending on frontend/GUI.Forms code. The client:

- validates the private discovery directory and endpoint objects;
- negotiates and authenticates `orchestrator.local` 1.0;
- enforces the `ORC1` one-MiB framing ceiling;
- rejects malformed, duplicate-key, deeply nested, incompatible, mismatched,
  disconnected, or non-success responses;
- materializes typed version, release, lifecycle, and availability snapshots;
- obtains contracts, availability, routing/fallback, and service controls in one
  atomic `ORC-FE-001` bootstrap read;
- invokes the single `orchestrator.search` operation and parses common result
  names/source/completeness plus successful or partial terminal state without
  selecting catalogue versus live traversal;
- verifies one daemon/cache generation and computes only the Orchestrator-owned
  Frontend 001 gate from the ready release evidence;
- materializes the separately attributed GUI.Forms and architect gates without
  claiming authority to satisfy or combine them.

The library creates no worker thread and invokes no callback. Calls are bounded
and synchronous; the File Manager adapter owns its I/O worker and transfers the
completed value snapshot through its UI-thread queue. The public C++ surface is
a source wrapper over the wire, not a frozen cross-compiler binary ABI.

It intentionally lives in Orchestrator's conformance tree. It is evidence for
the future frontend edge, not authorization to begin Frontend 001.

The reusable snapshot method is `orchestrator_gate_ready()`. There is
intentionally no `ready_for_frontend_001()` convenience: implementation may
open only after the GUI.Forms go-ahead and architect direction are recorded by
their own authorities as well.

Build and run:

```sh
cmake -S orchestrator/conformance/clients/cpp \
  -B /tmp/fileman-orchestrator-cpp-build
cmake --build /tmp/fileman-orchestrator-cpp-build
/tmp/fileman-orchestrator-cpp-build/orchestrator-cpp-client \
  /absolute/private/runtime-leaf probe
/tmp/fileman-orchestrator-cpp-build/orchestrator-cpp-client \
  /absolute/private/runtime-leaf search docs needle
/tmp/fileman-orchestrator-cpp-build/orchestrator-cpp-client \
  /absolute/private/runtime-leaf criteria docs kind file 3
```

On macOS the runtime argument may be omitted to use the same stable Application
Support default and activation retry as the Rust CLI:

```sh
/tmp/fileman-orchestrator-cpp-build/orchestrator-cpp-client probe
```

The reusable public header is
`include/fileman_orchestrator/client.hpp`. Windows currently fails explicitly;
its implementation waits for the named-pipe/ACL projection rather than
pretending Unix socket semantics are portable.

A CMake consumer may add this directory and link
`fileman::orchestrator_client`. Set
`FILEMAN_ORCHESTRATOR_BUILD_PROBE=OFF` when only the source client is needed.
