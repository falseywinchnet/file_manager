# Fixed extension contracts

Status: **contract candidates bounded by GIVEN extension classes**.

## Universal job envelope

All jobs receive:

- host-generated request ID and nonce;
- exact source identity token plus immutable metadata snapshot version;
- contract and plugin versions;
- scoped input handle/stream, never ambient filesystem authority;
- locale, scale, color-space, requested bounds, and declared type hints as
  untrusted hints where relevant;
- deadline, cancellation, byte/CPU/memory/output budgets;
- privacy class and cache permission;
- no UI object, host pointer, GUI callback, or writable index handle.

All results carry plugin/package digest, extractor version, contract version,
source snapshot, confidence where derived, warnings, elapsed/resource counters,
and cache key material. The host validates and can discard them without asking
the plugin.

## Preview provider

Input candidates: bounded random-access reader or streaming reader plus exact
metadata. Output candidates:

- one or more validated raster pages/frames with dimensions, stride, pixel
  format, orientation, color-space declaration, and page/frame identity;
- fixed-schema property groups containing typed read-only values;
- declared navigation among pages/frames through host-owned controls.

The plugin cannot create commands, edit controls, hyperlinks that bypass host
policy, scripts, audio autoplay, native windows, or event loops. Editable
properties are core/handler operations, not preview-plugin callbacks unless a
future capability explicitly admits them.

Required adversarial cases: truncated files, huge claimed dimensions, zip/decode
bombs, recursive containers, malformed profiles/fonts, slow stream, seek beyond
end, cancellation during parse/render, crash after partial surface, and stale
reply after new selection.

## Thumbnail provider

- Only invoked after host attests the source belongs to an indexed tree.
- Output is one bounded image at requested logical size/scale with crop policy,
  orientation, alpha, color-space, and generator provenance.
- Cache key includes source identity/version, plugin/extractor version, request
  parameters, theme-independent rendering parameters, and policy version.
- Thumbnail failure never blocks folder enumeration; late results are accepted
  only if source/selection identity remains valid.

Animated thumbnails, network fetch, file mutation, and hidden background scans
are not admitted.

## Search provider

The contract is evidence exchange, not a raw index database ABI.

Input candidate:

- parsed query constraints and current-folder scope expressed in a versioned
  neutral query IR;
- requested result limit/deadline;
- provider-specific opaque continuation token;
- user-approved scope token.

Output candidate:

- opaque provider object ID or authoritative host record reference;
- score channels, not one unexplained universal score;
- exact match evidence and source anchors;
- provider/extractor/model/version provenance;
- ambiguity and confidence;
- stable ordering key within one unchanged provider snapshot.

The host owns late fusion, provenance display, exact-record resolution, result
limits, and cancellation. A plugin may not overwrite file facts, alter core
index records, impersonate a local result, or return a path as authority. Remote
providers are outside the current core and require a future network decision.

## Virtual-system provider

Virtual objects remain distinct types, never fake local paths. A candidate
object descriptor includes:

```text
provider_id, object_id, generation, parent_id, display_name,
kind, capabilities, size/time/type fields with known/unknown state,
icon/thumbnail reference, stream availability, provenance
```

Current admitted operations are enumerate and read/open stream for display or
copy through host-controlled operations. Mutation, rename, delete, credentials,
mounting, and remote connection are not inferred. Pagination and cancellation
must be deterministic; cycles, duplicate IDs, disappearing parents, offline
providers, and a million-child directory are guard fixtures.

## Handler, icon, file-type, and metadata seams

The user requested planning for these hooks, but current GIVEN plugin classes do
not independently admit general handler, icon, or metadata plugins. Therefore:

- **CANDIDATE within preview/thumbnail:** a provider may declare supported type
  predicates and return its own preview/thumbnail results.
- **CANDIDATE within virtual systems:** virtual objects may provide validated
  icon/thumbnail references or bytes through that provider's namespace.
- **NOT ADMITTED:** plugin-installed OS associations, plugin shell execution,
  arbitrary executable handlers, global icon-theme replacement, core column
  injection, or unbounded metadata extraction.
- **OPEN:** a future fixed `type_evidence` result could return MIME/UTI-like
  evidence, magic-match confidence, and friendly label without becoming the
  authoritative handler decision.
- **OPEN:** a future `extracted_fields` contract may feed namespaced index data;
  it must distinguish content from derived metadata, carry provenance, and pass
  privacy/index-consent gates.

Unknown types continue to use native selection dialogues according to the host
product contract. File Manager's registry remains internal and platform defaults
are untouched.

## Configuration and settings

**CANDIDATE:** plugins publish a declarative settings schema with stable field
IDs, types, bounds, enum choices, defaults, sensitivity, restart requirement,
and localized description keys. File Manager renders it with house controls.

- Values live in a supervisor-owned, plugin-ID/version-namespaced store.
- Plugins receive validated values, never arbitrary settings-file paths.
- Secret values, if ever admitted, use a host secret service and opaque handles;
  they never appear in logs or ordinary configuration export.
- Schema migration is a pure, versioned transformation run without file/network
  capabilities, with backup and rollback.
- Unknown fields are preserved or rejected according to declared schema rules;
  they do not silently grant powers.

