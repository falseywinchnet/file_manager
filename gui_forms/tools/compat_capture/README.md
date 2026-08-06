# GUI.Forms compatibility Capture-0

Capture-0 produces a deterministic, versioned static-usage manifest from a
managed PE or a .NET single-file bundle. It reads PE, CLI metadata, method-body
IL operands, and `ImplMap` declarations without loading an inspected assembly
into the CLR and without executing target code.

This is an opt-in compatibility laboratory. It is not referenced by the native
CMake build, linked into GUI.Forms, or required by File Manager. The tool uses
only the .NET 10 SDK/runtime libraries; it has no NuGet dependencies.

## Run

```text
dotnet build tools/compat_capture/GuiForms.CompatCapture.csproj -c Release
dotnet run --project tools/compat_capture/GuiForms.CompatCapture.csproj \
  -c Release --no-build -- \
  --input /path/to/application.exe \
  --include-dir /path/to/application/Plugins \
  --output /path/to/capture-v0.json \
  --label stable-specimen-label \
  --expected-sha256 64-hex-character-authoritative-hash
```

The exact input hash is checked before output is written. `--include-dir`
recursively considers `.dll` and `.exe` files, records native files as inputs,
and scans only files carrying CLI metadata. A trailing `*` in an
`--assembly-prefix` value selects prefix matching; other values match an
assembly name exactly.

The checked-in schema is
[`capture-manifest-v0.schema.json`](capture-manifest-v0.schema.json).

## Privacy and evidence boundary

The manifest contains hashes, assembly identities, public external API names,
aggregate IL-operand counts, redacted application-defined control IDs, and
native import declarations. It deliberately excludes:

- IL bytes and decompiled source;
- string literals and embedded resources;
- application-defined type and method names;
- absolute input paths; and
- timestamps or machine-specific output paths.

An IL occurrence proves that a compiled method contains an operand referring to
the API. It does not prove the path executed. A metadata-only reference is kept
separate. Reflection capture remains a coverage oracle; dynamic GUI.Forms traces
remain the behavioral oracle.

Every observed API begins with disposition `unclassified`. Applying
`required`, `deferred`, `excluded`, or `application_side_port` is a separate,
reviewable compatibility decision rather than a scanner inference.

Capture-1 applies that separate policy with `apply_dispositions.py`; it never
rewrites Capture-0. The authoritative policy, generated catalogue, provenance,
and closure test are documented in the private specimen directory under
`compatibility/`.

## Test

```text
tests/run_compat_capture_tests.sh
```

The fixture includes a module initializer that writes a sentinel if loaded. The
test requires the sentinel to remain absent, compares two byte-identical
manifests, verifies Forms calls and transitive custom-control inheritance,
checks private-name redaction and P/Invoke capture, and rejects a wrong input
hash.
