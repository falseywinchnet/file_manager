# ADR-020: File Manager trusted local actions, previews, and picker package

Status: **accepted for File Manager 1.0 protected-root implementation and M4
dogfood**.

Date: 2026-08-10.

Owner approval: the grand architect opened File Manager implementation,
authorized adjacent repository work required by the frontend, cleared the
blocking gates, and directed a working 1.0 on the M4. This decision does not
promote the deferred dynamic handler/plugin registries or daily personal roots.

## Question

What first-party boundary should File Manager use for ordinary document Open,
Terminal Here, SHA-256 inspection, safe previews, and the reusable Document
Picker without introducing a shell, following links, loading unbounded bytes,
or linking future applications to the File Manager executable?

## GIVEN constraints

- Exact filesystem identity and the selected path revision remain
  authoritative. Engine results are never operation authority.
- Trusted File Manager built-ins are not plugins. Deferred `ORC-HND-001` and
  `ORC-CMD-001` cannot be described as implemented dynamic registries.
- File Manager may use explicit first-party platform adapters, but it may not
  embed a shell, inject environment, elevate, execute a repository command, or
  perform automatic network lookup.
- Preview and checksum workers use bounded memory, support cancellation, and
  report replacement/change instead of publishing a stale result.
- The reusable picker shares navigation and identity semantics, not the whole
  File Manager application. Engine absence must not block ordinary browsing;
  an invalid Orchestrator selection session must block acceptance.
- GUI.Forms remains the only native control/runtime surface. Web.Forms remains
  build-time browser-valid authoring under ADR-016.

## Candidates

### A. Delegate all actions and selection to native common dialogs/Finder

This supplies useful platform behavior quickly but does not produce the owned
picker surface, cross-application semantic package, protected-root identity
law, or exact checksum/preview failure states.

### B. Run shell command strings and external checksum/preview utilities

This appears small but creates quoting, option, environment, executable
selection, output parsing, cancellation, and privilege ambiguity. It also
makes the trusted operation depend on mutable external command behavior.

### C. Use bounded first-party C++ models/adapters and export the picker

Implement SHA-256 in the frontend model using one 256 KiB stream buffer;
revalidate the opened descriptor and visible path before publishing. Admit
bounded UTF-8 text and encoded PNG previews only. Launch macOS Open and
Terminal with a fixed `/usr/bin/open` executable and explicit argv vector. Put
the selection controller and GUI.Forms view in separately installed CMake
targets, with File Manager remaining only one consumer.

## Measurements

- **MEASURED:** SHA-256 empty and `abc` known-answer vectors pass; cancellation
  publishes no partial digest; stale revision, symbolic link, and out-of-root
  fixtures fail closed.
- **MEASURED:** fixed-argv tests preserve spaces, Unicode, quotes, and leading
  dashes as one path argument. Replaced objects, file-as-terminal-directory,
  and symbolic-link routes are refused before launch.
- **MEASURED:** UTF-8/PNG preview routing, invalid text, unsupported format,
  replacement, link, and root-containment fixtures pass. Text reads are capped
  at 64 KiB and encoded PNG reads at 16 MiB.
- **MEASURED:** eight M4 frontend suites pass after the picker view was added.
  Open-one, bounded multi-open, folder, save-as, export, extension correction,
  overwrite, hidden policy, type filter, cancellation, stale selection,
  session loss, and GUI.Forms Accept/disabled states are covered.
- **MEASURED:** an out-of-tree CMake consumer found the installed
  `FileManagerDocumentPicker` 1.0 package and linked
  `FileManager::DocumentPickerView` without linking the File Manager
  executable.
- **MEASURED:** installed Orchestrator/Engine search returned a catalogue page
  with one result and a source-bound cursor; the next C++ request consumed that
  cursor and completed with the second result.

## Decision

Choose C for the File Manager 1.0 protected-root profile.

`checksum_sha256` accepts one regular file, never follows a symbolic link,
streams through 256 KiB, publishes progress/cancellation, compares an optional
exact expected digest, and offers clipboard copy only after a stable terminal
digest exists. SHA-256 proves equality under that algorithm, not trust,
publisher identity, malware safety, or provenance.

The built-in preview lane admits only UTF-8 text and PNG. Unsupported formats
remain an explicit unavailable state. GUI.Forms owns PNG decoding and the
retained `PictureBox`; File Manager supplies only bounded encoded bytes.
Settings can disable the first-party preview and hide checksum/terminal
commands through the existing typed `ORC-SET-001` transaction.

The macOS adapter spawns only `/usr/bin/open` with a fixed argv shape. Default
Open passes the exact absolute path. Terminal Here adds only `-a`, `Terminal`,
and the exact directory path. A worker revalidates identity immediately before
spawn. No shell, shell command, output capture, environment mutation, elevation,
or embedded terminal exists.

The installed picker package exports:

- `FileManager::FrontendModel` for bounded filesystem identity/navigation;
- `FileManager::DocumentPicker` for request, profile, filter, selection,
  overwrite and acceptance semantics; and
- `FileManager::DocumentPickerView` for the bounded GUI.Forms composition.

The first profiles are open one, open bounded many, select folder, save as,
import, and export. Save/export return a destination observation and explicit
overwrite decision; the host application still owns its write. Browsing stays
available without Engine. Acceptance revalidates the object and current
Orchestrator session. Native fallback is returned as policy state, never
silently taken by the controller.

Engine pagination retains the source/value cursor as a typed C++ object.
Continuations cannot silently switch between catalogue and live-filesystem
lanes. File Manager appends de-duplicated, freshly observed in-root results and
keeps More Results disabled during a request or after completion.

## Failure modes

- An unavailable, changed, linked, non-regular, or out-of-root checksum/preview
  object produces no digest or preview bytes.
- GUI.Forms PNG rejection returns the generic file presentation and a precise
  decode failure; it does not fall through to a second decoder.
- A native launch failure displays the exact path and code for copying; it does
  not retry with a shell.
- Picker navigation failure retains a non-accepted state. Session loss disables
  the GUI accept action and makes controller acceptance unavailable.
- Save collision returns `overwrite_confirmation_required`; the picker never
  writes or truncates the destination.
- A stale or wrong-source Engine cursor is rejected by the closed contract
  instead of restarting in another source lane.

## Rejected options

- Reject A as the only picker path. Retain native dialogs only as an explicitly
  allowed fallback selected by fresh policy.
- Reject B completely for trusted built-ins. Fixed argv is not a shell template.
- Reject whole-file hashing/previews, automatic reputation lookup, plugin
  fallback inside the frontend process, and directories-as-files.
- Reject linking Paint, Text Editor, or another host to the File Manager
  executable. Those projects retain their independent implementation gates.

## Reversal path

Consumers may link only `FileManager::DocumentPicker` and supply another
GUI.Forms view without changing the request/result semantics. A later
Orchestrator file-selection session object can replace the current validity
input while preserving acceptance failure on session loss. A platform launch
intent may replace the macOS adapter after it preserves fixed-argument,
identity, and no-elevation behavior. Additional preview formats or checksum
algorithms require independent bounds and evidence; removing one leaves the
explicit unsupported state.

## Unresolved edges

- dynamic `ORC-HND-001` Open With and `ORC-CMD-001` registry snapshots;
- first-party application registry and native suite menu publication;
- owned picker top-level modality/focus evidence inside Paint/Text Editor after
  those projects' source gates open;
- VoiceOver, high-contrast, scale, and reduced-motion promotion evidence;
- signed/notarized distribution, Windows/Linux adapters, and daily-root
  promotion.
