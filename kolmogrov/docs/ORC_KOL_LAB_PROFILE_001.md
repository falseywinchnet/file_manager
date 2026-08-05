# ORC-KOL-001 conformance-laboratory profile 001

Status: **CANDIDATE fixture profile for contract reconciliation and experimental
engine adapters; not a production hash configuration**.

## Purpose

This profile is the earliest honest cross-project artifact Kolmogrov can supply.
It lets Orchestrator register configuration/availability semantics and lets the
Go engine implement a reference candidate-posting adapter without claiming that
File Manager's production feature policy, hash family, or resource budget has
been selected.

This binary certificate fixture remains the K0 conformance control. It is not
the current filename dogfood recommendation and is not superseded in place.
Engine experiments with the coupled filename-history hash use the separate
sealed candidate guidance in `HISTORY_HASH_ENGINE_CONTRACT_001.md` and
`conformance/orc_kol_001/history_hash_usage_guidance_001.json`.

The profile exercises the semantic spine that is already stable:

- a configuration descriptor is authoritative and versioned;
- independently addressable channel bytes are deterministic;
- a query produces bounded candidates, never identity;
- candidate evidence maps to exact engine records and requires verification;
- configuration mismatch makes the projection unavailable/rebuild-required;
- the conventional engine control remains available when Kolmogrov is absent.

## Lab family

```text
family_id:       org.filemanager.kolmogrov.symbolic-delete-certificate
family_version:  0.1.0-lab.1
profile_id:      binary-n10-t3-affine-q16
domain_policy:   sequence of exactly 10 atoms from ordered alphabet ["a","b"]
transformation:  exactly 3 deletions
target_length:   7
certificate_degree: 4
certificate_count: 16
channel_id:      deletion.certificate.membership
channel_algebra: idempotent-pattern-membership
pattern_encoding: base-2, slot-major, a=0, b=1
mask_width:      16 bits/certificate
channel_width:   256 bits
```

The target-position subsets are part of configuration identity and are frozen
explicitly rather than reconstructed from the private schedule compiler:

```text
[0,1,2,6] [3,4,5,6] [0,1,4,5] [0,2,3,6]
[1,2,4,5] [0,3,4,6] [1,2,5,6] [0,1,3,5]
[1,2,3,4] [0,4,5,6] [0,2,3,5] [1,2,4,6]
[1,3,4,6] [0,2,4,5] [1,3,5,6] [0,1,2,3]
```

Each certificate admits the 35 weakly increasing offset vectors

```text
0 <= d_0 <= d_1 <= d_2 <= d_3 <= 3.
```

For each vector, source positions `subset_i+d_i` form one observable pattern.
The corresponding mask bit is set once regardless of history multiplicity.

## Byte encoding

Certificate order is the listed subset order. Each 16-bit mask is serialized as
two bytes, least-significant byte first. Pattern code zero occupies the low bit;
pattern code fifteen occupies the high bit. The channel payload is the 16 masks
concatenated without padding.

This byte rule is stable only for this lab family. It does not select the future
numeric jet representation, general content projection, or production payload.

## Reference vector 001

```text
object_id: lab-source-alternating-10
symbols:   ["a","b","a","b","a","b","a","b","a","b"]
channel:   deletion.certificate.membership
bytes_hex:
ffbffeffff7ffffef7ffffbffeffffdffefffffefffdfbffbffffff7efffff7f
```

Query fixtures:

| Query | Expected certificate candidate | Exact descendant |
|---|---|---|
| `abababa` | yes | yes |
| `bababab` | yes | yes |
| `bbbbbbb` | no | no |
| `aaaaaaa` | no | no |

The adapter still maps every positive candidate to the external exact object
anchor and invokes exact catalogue verification. The fixture's agreement does
not allow a client to skip that rule.

## Canonical fixture envelope proposal

Reference fixtures use canonical UTF-8 JSON values with explicit arrays and
lowercase hexadecimal payloads:

```text
schema
configuration_descriptor
configuration_id
input {object_id, symbols}
addresses [{channel_id, bit_count, bytes_hex}]
queries [{query_id, symbols, limits, expected_candidate_ids}]
expected_evidence
expected_terminal_status
```

The canonical descriptor, not its digest string, is semantic authority. The
digest algorithm and JSON canonicalization profile remain for Orchestrator
reconciliation; an implementation may not treat two unavailable descriptors as
compatible merely because a field was omitted.

## Status and error behavior

The lab adapter reports one of:

```text
available_experimental
unavailable
configuration_mismatch
rebuild_required
unsupported_input
budget_exceeded
cancelled
corrupt_projection
internal_failure
```

An absent or incompatible Kolmogrov projection never becomes an empty successful
candidate set. The engine reports the active fallback/control family separately.

## Bounds

- source length: exactly 10 atoms;
- query length: exactly 7 atoms;
- alphabet: exactly `a,b`;
- candidate count, posting work, and elapsed work: caller-bounded by the engine;
- continuation: engine generation plus configuration identity plus query plan;
- cancellation: inherited from the engine request context;
- projection: disposable and rebuilt on any descriptor incompatibility.

The profile supplies deterministic semantics but no production latency promise.

## Deliberately opaque/private

The following do not cross ORC-KOL-001:

- Python classes, Go structs, Rust enums, native object layouts, or pointers;
- dynamic-programming slab layout and reachable-cell schedule;
- run-block matrices, phase-generation machinery, SIMD packing, or allocators;
- internal theorem proof representation;
- posting codec, segment layout, cache, compaction, or engine ordinals.

## Promotion boundary

This profile can open an Orchestrator fake provider and an engine experimental
adapter. It cannot advertise production fuzzy similarity. Production admission
still requires:

1. a canonical File Manager feature policy for names/paths or another declared
   source view;
2. a rich-content pattern projection and rescue rule;
3. frozen length bands, radii, schedules, widths, quantization, and numeric-jet
   disposition;
4. deterministic cross-platform vectors for the selected profile;
5. equal-bit held-out quality and hard-negative wins over engine controls;
6. posting bytes, build/update work, query tails, cancellation, and exact
   verification under engine guard workloads;
7. migration/rebuild and evidence schemas reconciled by Orchestrator.
