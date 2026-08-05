# Local wire v0 fixtures

Status: **fixture-draft under ADR-009**.

Each JSON file is the payload of one frame:

```text
4 bytes ASCII ORC1
4 bytes unsigned big-endian payload length
1..1,048,576 bytes UTF-8 JSON
```

`client_hello.json` contains a deliberately public all-zero fixture credential;
it is never valid production authentication material. Real credentials contain
32 operating-system-random bytes, live in a private `0600` file beneath the
`0700` runtime leaf, and are never checked into conformance data.

The first client frame is the hello, the first server frame is the selected
hello, and subsequent frames carry `ORC-COM-001` requests and responses.
