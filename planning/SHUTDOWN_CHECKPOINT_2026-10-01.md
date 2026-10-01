# Shadow shutdown checkpoint — 2026-10-01

The owner announced shutdown in five minutes. This checkpoint preserves ongoing
development; it does not accept unfinished interfaces or certify release readiness.
Plan Paint is outside this checkpoint and must remain untouched.

## File Manager restart

- Previous reviewed checkpoint: `8e5368f` (serial audio scheduler and records).
- Prepared-text A2 service/raster source and focused tests are preserved with
  receipts under `gui_forms/experiments/PREPARED_TEXT_A2_*`. The coordinator
  reviewed the initial implementation and independently ran six passing suites.
  That review found cross-service authority collisions and an unlocked raster
  publication interval. The provider corrected both and reports six passing
  suites on the corrected source. Independent review of those corrections is
  still required. Process-wide nonreused session identities and publication under
  the authority mutex are the intended corrected laws.
- Audio Stage 2 transport is unfinished development source. The owning sibling
  reports initial PCM/concurrency tests passing; coordinator source review,
  shutdown/quiescence proof, packaging verification and full house-style review
  remain pending. Do not infer these from passing tests.
- Both prepared text and loop transport are opt-in, OFF by default. Prepared
  headers are excluded from ordinary SDK installation. Development loop transport
  installation is explicitly refused. Neither is an available released SDK API.
- Root owns `gui_forms/CMakeLists.txt` and `gui_forms/cmake/Audio.cmake`; the
  prepared-text sibling owns prepared text and its bounded HarfBuzz changes;
  Games owns the transport slice. Coordinate before resuming edits.
- Window/Painter/display-command/DIB integration for A2 is unassigned. Dynamic
  SwiftEdit windows remain a negotiation, not an implemented capability.

## Evidence and review limits

Use `planning/PROGRAMMING_HOUSE_STYLE.md` for every first-party source review.
Preserved source is not automatically house-style accepted. Prepared-text
receipts identify reviewed scope and unresolved proof limits. Audio Stage 2
requires independent review of actual PCM ownership, SPSC publication, receipt
reuse, engine shutdown, callback allocation/destruction and executor enforcement.
No new native device or visible desktop test is implied by this checkpoint.

SwiftEdit and Games are separate repositories; their owning sibling chats are
responsible for committing and pushing their checkpoints. Build directories and
ignored dependencies remain local and can be regenerated. Resume from Git source
and recorded receipts, not from assumptions about surviving build processes.
