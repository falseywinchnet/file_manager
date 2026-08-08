# ADR-010: Orchestrator Core 1.0 compatibility horizon

Status: **accepted and conformant for the first macOS Core 1.0 artifact**.

Date: 2026-08-05.

Owner approval: the grand architect directed continued work toward Orchestrator
1.0 and delegated Orchestrator integration decisions. ADR-006 already requires
the first supported platform rather than every target platform to precede
Frontend 001.

## Question

What exact compatibility promise turns the pre-release Core projections into a
1.0 line without making unfinished Windows/Linux packaging a false macOS
blocker or preserving accidental 0.x wire shapes forever?

## GIVEN constraints

- Core 1.0 must be a live, headless frontend bootstrap authority.
- macOS, Windows, and Linux are product targets, but ADR-006 requires documented
  discovery/authentication on the first supported platform.
- Contract families, local wire, CLI grammar, and C++ source API remain
  independently versioned.
- Required but absent provider capabilities may remain explicitly degraded,
  unavailable, deferred, negotiating, or stubbed.
- Unknown critical semantics fail closed; optional additive fields may be
  ignored.

## Workloads and failure modes

The horizon covers an older 1.0 frontend talking to a later Core 1.x daemon,
newer clients meeting an older daemon, a pre-release 0.1 peer, daemon restart,
and platform features absent from the first artifact. It must not convert a
provider's availability into protocol incompatibility or silently reinterpret
a field inside one major line.

## Candidates

### A. Call the release 1.0 while retaining wire/contracts 0.1

This keeps current fixtures unchanged but advertises a stable product over an
explicitly experimental compatibility namespace.

### B. Support every 0.x development projection indefinitely

This protects no shipped consumer and freezes pre-release mistakes before the
frontend exists.

### C. Start the stable major at 1.0 and retain 1.0 semantics throughout Core 1.x

Move the installed wire and required Core semantic projections to major 1,
minor 0. Keep 0.1 as an incompatible negative fixture. Permit additive optional
fields and new methods in later 1.x minors, but require a new major for removed,
renamed, retyped, authority-changing, or newly must-understand behavior.

## Evidence and measurements

- **OBSERVED:** Rust and separately built C++ clients complete one authenticated,
  atomic `ORC-FE-001` release/contract/availability/routing/control bootstrap
  and restart on macOS.
- **OBSERVED:** live tests reject an incompatible major and contain malformed,
  truncated, oversized, fragmented, stalled, and saturated sessions.
- **OBSERVED:** fixtures reject must-understand extensions, explicitly reject
  cancellation on atomic Core calls, and tolerate unknown ordinary fields.
- **UNMEASURED:** Windows named-pipe and Linux supervisor projections are not
  first-platform evidence and remain separate promotion gates.

## Decision

Choose C.

The first Core 1.0 platform is macOS. `ORC-COM-001`, `ORC-LIF-001`,
`ORC-FE-001`, and `ORC-CLI-001` begin their stable compatibility line at 1.0.
The installed `orchestrator.local` wire and structured CLI protocol likewise
move to 1.0 before product release. No released consumer depends on 0.1; it is
retained only as an explicit incompatible-major corpus.

Within Core 1.x:

- a 1.0 client remains accepted by later 1.x servers for the Core bootstrap
  operations and terminal meanings;
- a client minor greater than the server's supported minor is rejected;
- optional fields and new methods may be additive;
- `critical_extensions` carries must-understand names and fails closed when
  unsupported;
- removal, rename, type/authority change, or a new mandatory interpretation
  requires a new major;
- provider availability changes do not change protocol compatibility.

Core 2.x support policy is deliberately not promised by this record. It must be
decided with real 1.x consumers and migration evidence.

macOS release readiness is evaluated against the macOS artifact. Windows and
Linux capabilities remain truthful follow-on rows and cannot be labeled
available from macOS evidence. This does not reduce their product-target status.

The first-platform authentication boundary is the private `0700` runtime leaf,
same-owner `0600` credential/discovery/socket objects, a fresh 256-bit token per
daemon instance, and instance verification. All local processes of that user
share the CLI authority defined by ADR-003, so a native peer UID check would not
separate an additional policy principal on macOS. The implementation now
enforces that kernel-vouched UID as defense in depth; it does not create a new
authority class or Core 1.0 policy boundary. Root compromise and
same-user token theft are outside this boundary and must not be described as
contained.

## Why the other candidates lost

- A makes “1.0” a label without a stable protocol horizon.
- B pays permanent compatibility cost for an unreleased development surface.
- Requiring all platform packages before the first platform contradicts the
  staged release predicate already accepted in ADR-006.

## Consequences

- Every current Rust/C++ fixture and client moves in lockstep to 1.0 before
  frontend implementation opens.
- The 0.1 hello remains useful only for rejection testing.
- Contract stages may advance through implemented/conformant to stable only
  when their executable macOS evidence passes; this ADR does not itself mark
  them stable or flip release readiness.
- Platform identity is added to release evidence before final promotion.

## Reversal and migration path

Before Core reports ready, 1.0 fixture details may still change with an explicit
digest and ADR correction. After readiness, incompatible changes require a new
major and a separately specified migration/dual-read interval.

## Unresolved edges

- Windows named-pipe and Linux supervisor compatibility records;
- later-family and long-run differential/resource suites beyond the Core macOS
  bootstrap corpus;
- Core 2.x support duration.

## First-artifact promotion evidence — 2026-08-07

ADR-011's installed LaunchAgent trial passed activation, authenticated status,
shutdown, supervisor reactivation within the five-second client bound with a
fresh instance, bootout, and exact removal on macOS 14.8.7 arm64. The executable release manifest now
reports every Core requirement satisfied. This promotes the first macOS Core
1.0 artifact without claiming Windows, Linux, later providers, or package
signing.
