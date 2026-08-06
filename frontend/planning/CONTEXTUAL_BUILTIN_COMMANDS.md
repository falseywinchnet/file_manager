# Contextual built-in commands

Date: 2026-08-06.

Status: **GIVEN product scope; exact command schemas/platform implementations
remain CANDIDATE**.

## Classification law

Checksum inspection and `Open Command Line Here` are trusted File Manager
built-ins. They are not plugins merely because plugins may also register context
commands. Orchestrator owns stable command identity, settings/availability and
audit meaning; File Manager owns contextual visibility and dialogs; first-party
platform code performs the operation.

Contextual visibility is not execution. No hashing, process launch or directory
inspection beyond ordinary context classification begins while rendering a
menu.

## Checksums

### Default visibility

The checksum command is prominent for:

- executables and executable packages;
- archives;
- disk images and installer images.

For other regular files it may live in an expanded command set or be exposed by
the user's context-menu configuration. Directories, virtual results and
unavailable remote objects require separate semantics and are not hashed by
accident.

### Operation

An owned modeless or modal **Checksums** dialog displays:

- exact selected path/object snapshot and current file size;
- selected algorithm(s), progress, bytes read, cancellation and terminal state;
- computed digest with copy command;
- optional expected digest entry/paste and exact match/mismatch result;
- file changed/replaced/unavailable detection;
- algorithm identity and implementation version where useful.

The first algorithm set is an interview/benchmark choice. SHA-256 is the
interoperability baseline candidate. Additional SHA-2/SHA-3/BLAKE-family or
legacy verification algorithms must be explicitly justified; insecure legacy
digests are labelled and never presented as proof of authenticity.

### Correctness and resource law

- Hash on request only; never precompute merely because a menu opened.
- Stream through bounded buffers with cancellation and no whole-file load.
- Observe file identity/size/timestamps before and after; report changed-during-
  read rather than presenting a stable claim.
- Multiple selected files produce per-file records under a declared batch cap.
- A digest proves byte equality under the named algorithm, not publisher trust,
  malware safety or provenance.
- No automatic web lookup or reputation service.

### Evidence

Golden known-answer vectors, empty/small/huge/sparse files, mid-read mutation,
replacement, permission failure, disappearing volume, cancellation, batch cap,
throughput/CPU/RSS and UI responsiveness.

## Open Command Line Here

### Default visibility

The command is prominent when the current/selected directory is:

- inside a recognized version-control repository, initially Git;
- hidden;
- classified by a first-party platform adapter as a system or configuration
  directory.

It may be user-enabled everywhere or globally hidden. Recognition changes menu
visibility only and grants no extra authority.

### Operation

- Launch the user's configured native terminal application with working
  directory set to the exact selected/current directory.
- File Manager does not embed a terminal, host a shell, parse shell commands,
  quote user commands, or capture command output.
- No automatic elevation, `sudo`, administrator request, login-shell mutation,
  environment injection or repository command is performed.
- If terminal selection or platform launching is unavailable, show one clear
  error and the directory path for copying.
- Symlink/path identity and unavailable-directory behavior are explicit.

### Settings

Settings may choose terminal application, contextual visibility and whether the
command appears outside developer/system contexts. Unknown terminal profiles do
not accept arbitrary command templates without a separate injection/security
design.

### Evidence

Disposable repositories, hidden and ordinary folders; paths containing spaces,
quotes, Unicode and leading dashes; deleted/unmounted directory; configured
terminal absent; no elevation; correct working directory on physical macOS,
Windows and Linux.

## Contract intake

Both commands use `ORC-CMD-001` for stable identity, availability and settings
projection, but invocation remains first-party. Plugin command declarations
cannot impersonate these IDs, override their dialogs, or gain their trust.
