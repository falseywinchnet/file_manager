# Paint implementation sequence

Status: **post-gate candidate plan; no implementation authorized**.

## P0 — reference document/canvas model

Implement the accepted pixel/object state machine without GUI: dimensions,
alpha/color, selection/placement, commit/flatten, undo if admitted,
serialization/reference vectors and allocation limits. Differential fixtures
prove every edit against a simple exhaustive reference.

Exit: byte-stable fixtures, malformed/oversized rejection, deterministic edit
digests and no filesystem access outside disposable test files.

## P1 — GUI.Forms canvas laboratory

Build one window with canvas, accepted minimum tools, palette/status, zoom/pan,
selection, keyboard/focus/accessibility and damage instrumentation using only the
named public GUI.Forms package.

Exit: headless interaction/damage/semantic traces plus live first-platform
inspection; no GUI.Forms private source or perpetual redraw.

## P2 — open/save/export and recovery

Connect the real Orchestrator app profile and Document Picker; implement PNG and
accepted native/clipart formats, atomic safe writes, overwrite/external-change/
failure behavior and local recent state through admitted settings.

Exit: disposable-root corpus for cancel, malformed, huge, unavailable,
read-only, collision, partial-write, crash and reopen.

## P3 — clipart/composite dogfood

Create, select, save, browse, preview, drag and reuse alpha clipart/composites.
Prove native format migrations and flattened export do exactly what the
architect approved.

## P4 — File Manager and clipboard transfer

Dogfood File Manager → Paint file/PNG/composite drag, Paint → File Manager saved
objects, clipboard peers, lazy payload cancellation/expiry, incompatible flavor
fallback and cross-window focus behavior.

## P5 — help, polish and breadth

Complete the detailed Color dialog, local HelpProvider topics/greaseboard,
menus/owned dialogs, accepted tools,
keyboard/accessibility, high contrast, scale, sounds, resource packs, crash
diagnostics and performance budgets.

## P6 — native three-platform dogfood

Run the same image workflows on physical macOS, Windows and Linux; close host
input/dialog/clipboard/drag/accessibility and installer gaps. Compatibility
layers do not count as native support.

## P7 — Malkuth release candidate

Publish application package, handler/file formats, installer integration,
versioned documentation/local help, website captures, migrations, licenses/
SBOM, security/fuzz record and accepted known issues into a Malkuth manifest.

## Cross-cutting measurements

- startup/first correct canvas, RSS and idle wakeups;
- input-to-mark p50/p95/p99/worst and visible missed frames;
- local-damage pixels/work and allocation per stroke;
- large canvas/open/save/flatten/undo memory and latency;
- clipboard/drag payload bytes, startup and cancellation;
- malformed format and resource-ceiling behavior;
- deterministic output digests and color/alpha round trips.
