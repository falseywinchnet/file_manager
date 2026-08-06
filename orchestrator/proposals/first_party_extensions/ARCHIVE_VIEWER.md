# First-party Archive Viewer plugin plan

Date: 2026-08-06.

Status: **GIVEN future plugin proof; implementation closed**.

## Mission

Present an archive as a bounded virtual hierarchy using File Manager's trusted
embedded browser/Document Picker surface, while an isolated first-party wrapper
invokes an external cross-platform archive engine such as 7z. Viewing and
extracting hostile archives must not place the decoder or arbitrary filesystem
writes inside trusted File Manager or Orchestrator processes.

## Product behavior

- Context command **Open Archive** appears for admitted archive types when the
  plugin and engine are enabled/available.
- File Manager opens a virtual archive location using its existing tree/content
  and optional preview composition; the plugin supplies data, not controls.
- Entries expose path, kind, logical/compressed sizes, times, attributes,
  encryption state and integrity warnings when the engine reports them.
- Open/preview one entry materializes only that bounded entry through a staged
  handle and existing handler/preview path.
- Extract Selected/All opens the trusted Document Picker for destination and
  then host-rendered collision/options/progress dialogs.
- Cancellation kills the process tree and deletes incomplete staged output.

## Virtual hierarchy model

Entry identity includes archive object snapshot, archive-generation digest,
engine identity/version and canonical internal path/ordinal. Listings are paged
and bounded; directory trees are assembled from sanitized internal paths.
Duplicate names, case collisions, absolute paths, `..`, alternate separators,
links, devices and malformed encodings remain representable as warnings without
becoming host paths.

The archive is revalidated before entry-open/extract. If it changed, the virtual
generation becomes stale and the host requests reopen rather than mixing lists
and bytes.

## Extraction reef

- worker/engine writes only to a private staging root or host-provided output
  handles;
- trusted broker validates every relative output name and object kind;
- reject path traversal, absolute paths, device nodes and unsupported link
  targets by default;
- enforce entry count, per-entry bytes, total expanded bytes, ratio, depth,
  CPU/time/memory/disk and nested-archive limits;
- never overwrite existing destinations without a trusted host decision;
- publish validated outputs atomically where possible and report partial
  publication precisely;
- password bytes are entered in a trusted host dialog, sent only to the job,
  redacted from logs and released after terminal state.

## Engine packaging candidates

1. Discover a compatible user-installed 7z executable.
2. Bundle a reviewed engine/library under its license and platform package.
3. Support several external engines through separately versioned adapters.

The first choice reduces package size but varies capability/version. The second
improves reproducibility but expands security/update/SBOM burden. The third is
premature until one complete path passes. Selection requires an ADR and native
evidence; “such as 7z” does not yet freeze an executable.

## Gates

1. Archive types and engine route selected; licenses/threat model approved.
2. Plugin supervisor, external-process broker and host-mediated file capability
   fixtures pass.
3. Virtual hierarchy and entry-stream contract reconciled.
4. Reusable File Manager embedded browser and Document Picker are available
   without plugin UI injection.
5. Disposable-root hostile corpus passes before real user folders.
6. Grand architect explicitly opens implementation.

## Hostile corpus

Path traversal, absolute names, case collisions, duplicate entries, symlinks/
hardlinks/devices, zip/tar bombs, deep paths, huge counts, truncated headers,
wrong password, encrypted names, nested archives, changing source, engine hang/
crash/spam, output quota exhaustion, cancellation and destination unmount.

## Measurements

Cold/warm open, first page, pagination, entry preview, extraction throughput,
CPU/RSS/process count, staged bytes, cancellation latency, File Manager input
latency and zero worker activity while disabled.

## Exclusions

No archive editing/update in the first proof, shell command construction,
ambient extraction, plugin controls, in-process third-party decoder, web archive
service or silent recursive nested opening.
