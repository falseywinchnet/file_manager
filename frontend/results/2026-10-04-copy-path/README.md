# Copy selected paths

Status: **OBSERVED implementation; MEASURED Windows verification; native CI pending.**

The old command used `selected_entry()`, which returns no entry for a multiple
selection, and silently copied the browsing directory instead. The shared
command now prepares all selected paths in the ObjectView's normalized item
order. Only an empty selection falls back to the browsing directory.

The private preparation reads existing path observations without filesystem I/O.
It counts exact encoded bytes and separators before allocating one destination.
POSIX paths copy native bytes; Windows uses strict WideCharToMultiByte sizing
and conversion into that destination. The conversion pass makes no per-path
string allocation or destination growth. Complete preparation precedes one
clipboard call. Missing entries, output beyond the existing 16 MiB HostServices
limit, and invalid Windows UTF-16 return no text and make no clipboard call.
Backend refusal is reported rather than labelled as success.

Plain paths are separated by LF, with no final LF and no shell quoting. Embedded
newlines are preserved; this is readable text, not a reversible filename-list
serialization or file-transfer payload. Invalid POSIX UTF-8 remains subject to
the HostServices validation boundary. Copying an observed path does not revalidate
filesystem identity or grant file access.

## Evidence

- Full Windows GCC 16.2 Release frontend build, one compiler job, and all 15
  CTest suites passed in 7.12 seconds. Raw output: `Windows-LastTest.log`.
- Actual shared-command/HostSession fixture: current directory, single file,
  multiple file/folder selection, one publication, backend refusal, missing
  selected binding with no partial clipboard publication.
- Independent builder expectations: Unicode including a supplementary character,
  spaces, apostrophe, embedded newline, exact byte cap and one byte over it,
  missing entry, Windows unpaired surrogate.
- The macOS native harness invokes Commands / Copy path on three selected
  generated files and compares the actual NSPasteboard bytes with independently
  prepared expected paths. Native execution is pending, not proved by Windows.
- Complete independent house-style review accepted the revised scope. The first
  review rejected repeated conversion-temporary allocations; the final code
  removes them. See `SOURCE_REVIEW.md` and reviewed source hashes.

`Windows-rejected-order-assumption.log` preserves the first assembled failure.
The test expected insertion order, but ObjectView normalizes selected IDs to item
order. Correcting the expectation to Documents followed by root.txt preserved the
product's established order and the exact-path assertion. No application logic
was changed to accommodate that test mistake.

These checks establish correctness, not a measured large-selection latency gain,
physical input verification, new file operations or a released package.
