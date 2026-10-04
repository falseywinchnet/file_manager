# Selected-preview result bytes and source boundary

2026-10-04 UTC. **CANDIDATE fixture draft**, companion to
`SELECTED_PREVIEW_LIFECYCLE_DRAFT.md`. Not a frozen wire contract, new runtime
operation or opened plugin capability. The disposable `preview_frame` laboratory
tests receiver semantics independently of codec, source platform and transport.

## Authority retained by the host

**OBSERVED:** the frontend's current `NativeReadFile` reads and observes one
opened object, while `load_preview` compares the selected revision before read,
then the opened descriptor and visible path afterward. Its stat/file-information
revision is an observation, not proof of an atomic immutable filesystem snapshot.
Path-route checks and final-component no-follow flags are not proof that every
ancestor stayed unchanged throughout access. The new profile must not inherit
stronger authority guarantees merely by copying that code.

**CANDIDATE source route:** the trusted host admits a current selected regular
file through the negotiated native read boundary, owns one encoded snapshot
(the JPEG experiment currently accepts at most 16 MiB), and sends only those
bytes to the closed decoder. No user path or ambient directory authority enters
the decoder request. Compare this bounded-copy route against a separately
restricted read handle before implementation; neither is selected here.

Required source semantics include native object identity and root/revocation
association, before/after read observations, short-read/change handling, and
revalidation before publication. An immutable received byte array is stable for
decoding; it cannot retroactively prove an atomic source read. Concurrent changes
that restore the observed metadata, filesystem observation limits, ancestor
replacement and atomic-snapshot/locking alternatives remain explicit unresolved
edges. No claim of hostile-filesystem containment is made by this fixture.

The host, not the helper, associates a ticket with source evidence and the
admitted provider build/profile. An echoed ticket is correlation, not
authentication or source authority. A later launch/transport must establish its
private peer and forbid unrelated processes from supplying bytes. The host
rechecks current demand, source/revocation state and lifecycle after process
exit before offering a result. Source changes discard even a well-formed frame.

## Experimental result frame

Exactly one 64-byte header followed by its exact payload, then transport EOF.
Every integer uses little-endian encoding, not native record layout. No optional
fields, extension chain, variable metadata, compression or embedded strings.

| Offset | Bytes | Meaning |
|---|---|---|
| 0 | 8 | ASCII `FMPREV01` |
| 8 | 2 | Experimental version 1 |
| 10 | 2 | Header extent 64 |
| 12 | 4 | Status: 0 ready, 1 unsupported, 2 invalid input, 3 resource limit, 4 decode failed |
| 16 | 8 | Nonzero host-session fixture identity |
| 24 | 8 | Nonzero request nonce |
| 32 | 4 | Width |
| 36 | 4 | Height |
| 40 | 4 | Stride |
| 44 | 4 | Payload byte count |
| 48 | 4 | Pixel profile: 1 for ready, 0 for terminal |
| 52 | 12 | All zero; unknown fields are rejected |

A ready result has positive axes no larger than 1024, stride exactly `width*4`,
payload exactly `stride*height`, and profile 1: top-to-bottom opaque sRGB BGRA8
with orientation already applied. Maximum live payload is 4 MiB. Dimensions
are validated before products and allocation. Every alpha byte must equal 255.
The receiver verifies shape, profile ID and alpha bytes; it cannot infer correct
color conversion or orientation from arbitrary RGB values. Provider correctness
and build binding are separate admission evidence.

A terminal result has zero width, height, stride, payload and profile. Unknown
status/version, reserved bytes, wrong ticket, malformed shape, truncated header
or body, trailing bytes and a second frame are protocol failures. Helper status
does not override a host timeout/cancellation; the host owns those outcomes.

## Incremental ownership and publication

The receiver has a fixed header and one owned pixel vector. A complete validated
header precedes reservation and zero-initialization of the exact live payload.
Chunks are borrowed synchronously, copied into fixed storage, and not retained.
No allocation or vector growth occurs per chunk. Allocation refusal is explicit.
One live payload is bounded; allocator overhead/reservation behavior and host
source/IPC/GUI copies are separate accounting. This class does not impose a
global receiver count; the lifecycle slot/admission owner must do that.

`push` consumes the whole supplied chunk or poisons the receiver and releases
its pixels. Future input to a poisoned receiver is refused. Empty input is not
EOF. `finish` consumes the owner at actual EOF, requires complete extent and
opaque pixels, and moves the one vector into a valid result. On every failed
finish the owner drops its storage. No raster borrow escapes prematurely.
Blocking reads, peer shutdown, bounded chunk sizes and deadlines are host work;
the small CLI consumer is a conformance fixture, not a supervised service.

Receiving a frame or EOF does not establish process exit, source stability or
permission to publish. The lifecycle's matching reap event, current ticket,
source validation, result admission, offer expiry and frontend acknowledgement
remain mandatory. The decoder may exit without a frame; that becomes a host
failure. A success frame followed by a failed process exit cannot publish.

## Evidence and unresolved integration

Rust fixtures exercise every split of the six-pixel golden frame, every short
extent, single-byte delivery, stale tickets, malformed geometry/extent/version,
reserved fields, non-opaque alpha, duplicate/trailing input, terminal results and
maximum raster size. A separately authored C++ producer writes the golden bytes
to a Rust consumer through an actual stream. No common encoder makes that
producer and consumer agree by construction. Neither endpoint processes files,
codecs or GUI objects. The full house style applies to both and their tooling.

Source acquisition, OS confinement/resource policy, actual codec transport,
authentication, provider-build binding, global copy accounting and independent
frontend consumption are still unresolved before a complete preview family can
freeze. The native Mac process experiment establishes additional mapping
admission only; it does not satisfy the earlier candidate hard physical-memory
budget. This draft does not silently relax that unresolved requirement.

## Decoder-only input feasibility

The JPEG research now separates generated input from the decoder entry point.
Its experimental envelope is eight ASCII bytes `FMJPEG01`, a little-endian u64
length admitted only in 1..16 MiB, that exact body, then EOF. Header/length
validation precedes allocation; incomplete or trailing input cannot decode.
The executable takes no path argument but accepts caller-supplied bytes and is
not a sandbox. Its fixed result ticket 7/11 is conformance data, not an
authenticated runtime request. No source grant, revision, root association or
production request schema is established by this envelope.

This helps measure decoding separately from fixture encoding and exercises
the bounded-copy candidate. It does not select that candidate or freeze input
transport. The synchronous reader can block on a stalled sender, and process
limits/cancellation must be established by a supervising host before adoption.
See `../../frontend/results/2026-10-04-jpeg-decoder-input/README.md`.
