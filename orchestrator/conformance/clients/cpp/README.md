# Independent C++ Orchestrator client

Status: **OBSERVED Unix/macOS Core 1.0 conformance consumer; Windows transport
projection remains open**.

This C++17 library and probe executable implement ADR-009 without linking Rust,
sharing Rust layout, or depending on frontend/GUI.Forms code. The client:

- validates the private discovery directory and endpoint objects;
- negotiates and authenticates `orchestrator.local` 0.1;
- enforces the `ORC1` one-MiB framing ceiling;
- rejects malformed, duplicate-key, deeply nested, incompatible, mismatched,
  disconnected, or non-success responses;
- materializes typed version, release, lifecycle, and availability snapshots;
- verifies one lifecycle generation across the bootstrap read.

It intentionally lives in Orchestrator's conformance tree. It is evidence for
the future frontend edge, not authorization to begin Frontend 001.

Build and run:

```sh
cmake -S orchestrator/conformance/clients/cpp \
  -B /tmp/fileman-orchestrator-cpp-build
cmake --build /tmp/fileman-orchestrator-cpp-build
/tmp/fileman-orchestrator-cpp-build/orchestrator-cpp-client \
  /absolute/private/runtime-leaf probe
```

The reusable public header is
`include/fileman_orchestrator/client.hpp`. Windows currently fails explicitly;
its implementation waits for the named-pipe/ACL projection rather than
pretending Unix socket semantics are portable.
