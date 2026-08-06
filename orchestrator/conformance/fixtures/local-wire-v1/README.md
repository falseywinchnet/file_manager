# Local wire 1.0 fixtures

These are the current Core 1.0 `orchestrator.local` hello and request payloads.
Each payload is carried behind the eight-byte `ORC1 || u32be(length)` header.
The all-zero credential is inert public fixture data and must never be used by a
live endpoint.

`../local-wire-v0/` is retained as the incompatible pre-release 0.1 corpus.
