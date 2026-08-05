# Orchestrator conformance corpus

Status: **bootstrap fixtures executable; cross-language peers pending**.

`fixtures/bootstrap/` contains canonical JSON request/response examples for
version, status, semantic-fact stub state, and unknown methods. Rust integration
tests execute the same requests against the kernel and compare semantic JSON
equality.

Later contract families add every terminal error, unknown critical fields,
version downgrade, fragmentation/coalescing, EOF, cancellation, backpressure,
quota, stale generation, capability denial, and partial-provider case required
by `../planning/CONFORMANCE_AND_VERSIONING.md`.

The JSONL fixture layer is inspectable laboratory interchange. It is not a
decision that bulk plugin results or the final local daemon transport use JSON.
