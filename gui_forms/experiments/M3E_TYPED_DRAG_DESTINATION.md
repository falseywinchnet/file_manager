# M3e typed drag destination evidence

Date: 2026-08-04

Status: **OBSERVED implementation evidence for the bounded inbound half of
M3e**. Outbound drag-source initiation remains **OPEN**. H6 and Gate H1 remain
**CANDIDATE/open** because Windows/Linux adapters, complete IME, and the other
declared M3 host services are not complete.

## Given constraints

- **GIVEN:** portable core and host protocol treat Windows, macOS, and Linux
  equally. AppKit types and conventions may exist only inside the macOS adapter.
- **GIVEN:** GUI.Forms defines one pragmatic deterministic event order; it does
  not reproduce every historical WinForms ordering variant.
- **GIVEN:** the current Gallery is not the complete Modern.Forms-equivalent
  control set. Control-family completion remains governed by the 51-family
  completeness matrix and M6 onward.

## Entrance defect and correction

The reported bottom-widget overflow was **OBSERVED** in two places. The values
group declared 66 logical pixels of content for 82 pixels of children, and the
collection list inserted row gaps that exceeded its inner height. More
generally, retained painting passed window damage—not the current ancestor
intersection—to descendants.

The Gallery group heights and list spacing now contain their declared children.
The retained painter passes the current intersection recursively. A geometry
test checks every effectively visible Gallery child against its parent, and an
intentional overflow probe confirms that a 30×30 child crossing a 50×50 parent
receives only a 10×10 clip.

## Protocol 0.4 contract

- `DragDataItem` is a closed portable variant: UTF-8 text, a UTF-8 file-path
  list, or bytes carrying a UTF-8 media type. No pasteboard, COM data object,
  URI-list convention, AppKit operation, or native handle enters the contract.
- A drag has a nonzero session identity, logical position, modifiers, declared
  allowed effects, ordered items, and one of enter/over/leave/drop.
- The boundary rejects malformed UTF-8, embedded NULs, unknown effect bits,
  zero identities, non-finite positions, empty actionable drags, more than 16
  items, more than 4,096 paths in one item, and more than 16 MiB total payload.
- Controls opt in with `allow_drop`. Hit testing finds the visual leaf and then
  climbs to the nearest eligible opt-in ancestor.
- Target crossing order is fixed: leave the old target, enter the new target,
  then deliver the current over event. Drop delivers once and clears session
  state. Disable, hide, detach, dispose, close, shutdown, and modal owner
  suppression clear or block drag ownership through ordinary retained rules.
- Preview, target, and bubble routing follows the retained route. A returned
  effect must be one allowed single effect; invalid or combined results collapse
  to none.

## Reference and AppKit adapters

Headless advertises the capability and records event/drop counts plus the
returned effect in byte-stable traces. Its tests preserve variant order and
bytes, cross between two targets, revoke a live target, and exercise boundary
rejections.

The AppKit view registers as an `NSDraggingDestination`. It translates file
URLs to paths, strings to UTF-8, `public.data` to media-typed bytes, operation
masks to portable effects, and view coordinates to logical points. The adapter
applies the shared count/byte limits while extracting and returns only the
portable result selected by retained code. AppKit declarations remain under
`src/host/macos` and the shared boundary audit stays authoritative.

The Gallery collection opts in as the live consumer. It paints a restrained
blue drop cue and reports accepted file/text/data counts in the command bar.
Its default banner is `Portable core · Host 0.4 · drop-ready`.

## Verification coverage

- Normal build and AppKit smoke: **19/19 passed**.
- Strict Release (`-Wall -Wextra -Wpedantic -Werror`): **18/18 passed**.
- Full AddressSanitizer build: **18/18 passed**.
- Renderer-free UndefinedBehaviorSanitizer build: **13/13 passed**.
- Renderer-free ThreadSanitizer build: **13/13 passed**.
- Headless protocol tests cover exact routing order, payload preservation,
  accepted-effect filtering, malformed UTF-8, zero session identity, aggregate
  size rejection, accounting, and eligibility cleanup.
- Gallery tests cover complete visible-child containment, inherited clipping,
  a retained typed file drop, and command-bar acknowledgement.
- The AppKit smoke verifies capability publication and compiles/launches the
  native destination implementation. A real external drag is not automated;
  that remains a platform interaction test for the next native harness.
- **NEGATIVE/UNRESOLVED:** one computer-control attempt dragged the checked-in
  `gallery.dml` from Finder toward the live collection, but the command bar did
  not transition. The automation could not distinguish a missed screen target
  from rejected native delivery, so this is not counted as AppKit conformance
  and the result is retained rather than silently promoted to success.

## Honest boundary

This slice proves inbound destination translation and retained routing. It does
not yet expose a portable begin-drag service, negotiate delayed data providers,
promise non-UTF-8 Linux filenames, or prove Windows/Linux adapters. The broad
Protocol 0.4 exposes distinct `typed_drag_destination` and
`typed_drag_source` bits. Headless and AppKit advertise only the implemented
destination bit; the source bit remains unadvertised until its service exists.
