# Selectable inspector Location

**OBSERVED problem:** the native selection-facts capture recorded in
`../2026-10-04-selection-facts/NATIVE.md` clips long Location values. That row
previously painted a string without a text-selection or text-navigation control.

The application now uses the existing public PropertyList text editor for that
row and marks its TextBox read-only before exposing the window. PropertyList
retains ownership and synchronizes the same observed location values. A missing
editor rejects construction. No new host API, callback, retained path copy or
filesystem operation is introduced. The complete string remains available for
caret navigation, Select All and Copy. Multiple selection continues to show its
common browsing location, or the existing multiple-path explanation in search.
Empty selection clears the old path.

**MEASURED locally:** GCC 16.2.0, Windows x64 Release, one compiler. The complete
15-suite frontend run passed in 7.25 seconds. The existing selection-facts test
now drives the real Window key/text route: selecting and copying a full path
through a fake clipboard host, rejecting typing/Backspace/Delete changes, then
replacing the location after multiple/empty selection. This is controller-level
evidence, not physical keyboard or native clipboard evidence. The local consumer
uses the installed SDK with the reviewed PR50 Windows queue correction.

The Mac harness additionally exercises the actual host clipboard via the field's
Cmd+A/C key route, compares the native pasteboard with the full common path, and
navigates to the end before saving the inspector capture. Those native results
remain pending. No claim that all path characters fit simultaneously in a narrow
inspector, or that the full interface is visually accepted, follows from this fix.

The house-style scanner found zero spelling findings in the three touched
source files. Semantic review is recorded separately with its exact scope; this
does not certify unchanged controls or entire legacy source files.
