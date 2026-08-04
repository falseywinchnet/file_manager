# Threat and capability model

Status: **HYPOTHESIS and CANDIDATE design; no accepted ADR**.

## Assets to protect

1. File contents, names, paths, metadata, search history, and private-root policy.
2. Integrity of files, index records, handler registry, plugin grants, and hive.
3. GUI/indexer availability and interactive latency.
4. User identity, credentials, keychain/secrets, clipboard, cameras, microphones,
   screen contents, process memory, and other applications.
5. Provenance: which plugin/version produced every derived field or pixel.
6. Update and package authenticity, rollback safety, and local development state.

## Adversaries and fault sources

| Actor/input | Representative attack or failure |
|---|---|
| Malformed file | decoder memory corruption, decompression bomb, deep recursion, parser hang |
| Malicious native plugin | arbitrary syscalls, path probing, network exfiltration, fork bomb, IPC spoofing |
| Buggy plugin | crash, leak, deadlock, invalid dimensions, stale replies, corrupt cache data |
| Malicious package | dependency confusion, path traversal, symlink escape, executable install hooks |
| Compromised publisher/update | correctly signed hostile release or downgrade |
| Other local process | socket impersonation, shared-memory tampering, package replacement |
| Curious plugin | infers private paths or cross-job data through ambient filesystem or reused memory |
| Host bug | over-grants a directory, trusts declared MIME, accepts output before identity/version check |

Compromised kernel, firmware, administrator/root, and physical attacks are out of
the first containment claim. They must be stated as outside scope, not implied
to be solved.

## Security invariants proposed for experiments

- **HYPOTHESIS T1:** a plugin can receive the bytes/metadata for one job without
  learning an ambient source path.
- **HYPOTHESIS T2:** with no network grant, DNS, loopback, LAN, Internet, Unix
  sockets outside the job channel, and inherited connected sockets are denied.
- **HYPOTHESIS T3:** killing a worker ends its access; no descendant or inherited
  handle survives.
- **HYPOTHESIS T4:** a crash, panic, abort, timeout, output bomb, or protocol
  violation produces one typed failure and leaves GUI/indexer alive.
- **HYPOTHESIS T5:** a worker cannot mutate source data even when it parses a
  hostile file and even if a path is discoverable.
- **HYPOTHESIS T6:** pooled workers do not disclose prior-job bytes. This must be
  falsified before pooling is allowed for sensitive inputs.
- **HYPOTHESIS T7:** every accepted result is bound to request ID, source identity
  snapshot, plugin ID/version, contract version, and grant digest.

## Trust-tier candidates

Trust changes verification and grant UX; it must not remove process isolation.

| Tier candidate | Intended source | Maximum default authority |
|---|---|---|
| T0 built-in adapter | shipped and signed with File Manager | fixed contract; still separate worker for hostile parsing |
| T1 verified publisher | signature chains to an admitted publisher | declared fixed contracts; no network or write by default |
| T2 local signed | user imports/checksum-pins a package | same runtime denial; explicit local identity shown |
| T3 developer | unsigned/debug package under conspicuous mode | test roots only by default; no silent persistence/update |
| T4 legacy native bridge | OS/native preview provider | single-purpose disposable worker and strictest budgets |

**CANDIDATE alternative:** omit publisher tiers entirely for 1.0 and distinguish
only built-in, user-installed, and developer. Gate: fewer policy states without
weakening runtime confinement.

## Capability record

Each supervisor-issued grant should contain at least:

```text
grant_id, plugin_id, package_digest, contract, operation,
resource_handle_or_scope, access_mode, byte/range limit,
deadline, cpu_budget, memory_budget, output_budget,
network_scope, persistence, user_decision_provenance, expiry, nonce
```

Strings in a package manifest are requests. They become capabilities only after
policy intersection:

```text
effective = contract maximum
          ∩ package request
          ∩ user grant
          ∩ installation policy
          ∩ invocation scope
          ∩ platform-enforceable subset
```

If the platform-enforceable subset is weaker than the contract's minimum,
launch fails closed and explains the missing primitive. It does not run with a
warning.

## Capability vocabulary candidates

- `input.read_stream` — bounded read of a supervisor-opened source snapshot;
- `input.read_range` — bounded random access without path disclosure;
- `metadata.read_declared` — selected exact metadata fields;
- `result.emit_preview` — validated raster/vector-neutral result descriptor;
- `result.emit_thumbnail` — one bounded raster plus scale/color metadata;
- `result.emit_fields` — namespaced typed fields with confidence/provenance;
- `search.read_query` / `search.emit_candidates` — query plan fragment in,
  attributed record references out;
- `virtual.enumerate` / `virtual.open_stream` — opaque provider IDs, not forged
  local paths;
- `cache.read_own` / `cache.write_own` — quota-limited, version-namespaced store;
- `network.connect` — **not admitted**; reserve no production numeric ID until a
  decision accepts semantics and user policy;
- generic `filesystem.read_path`, `filesystem.write`, `spawn`, `shell`, `ui`,
  `clipboard`, `credentials`, and `device` — **REJECTED from the public model**.

## Confused-deputy defenses

- Host opens inputs after scope/policy checks; guest cannot convert a display
  path into access.
- Each reply echoes an unpredictable request nonce and is accepted once.
- Handles are single-job, least-rights, non-inheritable except at controlled
  spawn, and closed before process reuse.
- Virtual-provider object IDs carry provider namespace and generation; they can
  never be parsed as local filesystem IDs.
- Plugin search candidates are resolved through host authority and labeled with
  source/provider; missing/stale objects remain missing rather than reopening a
  guest-provided path.
- The supervisor never asks a plugin whether the plugin is allowed to act.

