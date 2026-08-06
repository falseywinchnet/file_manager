# Host-mediated file capabilities

Date: 2026-08-06.

Status: **GIVEN security boundary; CANDIDATE contract decomposition**.

## Governing law

A plugin may inspect or transform only objects explicitly introduced by the
trusted host. It never receives ambient directory authority merely because the
user invoked it from File Manager.

```text
trusted selection
  -> capability-scoped input handle
  -> supervised worker / bounded external process
  -> typed virtual result or bounded output stream
  -> trusted validation, collision and destination decision
  -> atomic host publication
```

## Input grant

- exact job and plugin identity;
- one or a bounded list of selected object snapshots;
- read-only handle/stream or staged immutable copy;
- declared size/type and stale/change behavior;
- byte/read/time/memory/process quotas;
- cancellation/expiry;
- no parent-directory enumeration unless a separate virtual-view job explicitly
  provides bounded child entries.

## Output grant

Plugins return one of:

- bounded structured metadata;
- validated preview/thumbnail bulk data;
- paged virtual hierarchy entries plus entry-open requests;
- bounded create-new output stream(s) with declared type/name suggestion;
- progress/password/choice request as typed data rendered by the host.

The trusted host chooses the final destination through its picker/policy,
validates names/types/limits, handles collisions, and publishes atomically. The
plugin cannot replace/delete the source by default or open a path behind the
host's back.

## External-engine broker

Where a plugin wraps 7z or another engine, the supervisor supplies an executable
identity, fixed argument schema, sanitized environment, private working
directory, inherited handles rather than arbitrary paths where possible,
stdout/stderr/result bounds, process-tree kill, deadline and exit interpretation.
No shell command string is constructed or evaluated.

## UI law

Plugins supply typed information and requests, never GUI.Forms controls, native
windows, styles, callbacks or arbitrary markup. File Manager/Document Picker
renders virtual lists, destination selection, password, progress, collision and
error dialogs. Plugins cannot add persistent panels; admitted context commands
are host-rendered from declarations.

## Disabled state

Installed-but-disabled means no worker process, decoder load, context command,
file read, background scan, external-engine invocation, network or update
activity. Package metadata may remain visible in trusted settings.

## Candidate contract split

- extend `ORC-PLG-002` for bounded worker/process jobs;
- extend `ORC-PLG-004` or introduce a separately reviewed virtual-hierarchy
  family for archive browsing;
- propose a transform/publication family only after destination and atomic-write
  semantics close;
- reuse `ORC-PCK-001` for trusted destination selection, never from inside the
  worker;
- use `ORC-CMD-001` for declarative context commands.

No new runtime ID is frozen by this proposal.
