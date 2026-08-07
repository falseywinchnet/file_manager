# M11h-P1 — compatibility-surface drains

Status: **MEASURED PARTIAL**. Date: 2026-08-06.

## Question

Can admitted Win32/GDI compatibility drawing update a private backing surface
many times while GUI.Forms imports and presents only the newest coherent result,
without losing raw GDI support or allowing paint reentry?

## Implemented

The generated Drawing/Forms bridge now gives every admitted direct-HWND surface:

- content and captured revisions plus a surface epoch;
- one queued-drain bit, one active-drain bit, and one dirty-after-drain bit;
- queued, coalesced, started, committed, and unchanged counters;
- deterministic `clean`, `dirty_queued`, `rendering`, `rendering_dirty`, and
  `retired` diagnostics;
- resize/visibility epoch invalidation and stale-lease rejection; and
- one target-scoped synchronous drain used by `Control.Update()`/`Refresh()`.

`Graphics.FromHwnd` captures once into its private GUI.Drawing bitmap. Later
`Graphics.Flush()` calls execute only new recorder commands into that bitmap and
copy the completed backing result into the isolated offscreen compatibility
HWND; they no longer recapture the HWND before every flush. The resulting
private-window message marks the owning retained surface dirty and queues one
import. `ReleaseHdc` now commits a bitmap-backed native lease to the same
surface and signals the same path. `WM_PAINT`/`EndPaint`, admitted input/text
callback return, resize, visibility, and explicit `Invalidate` are also touch
boundaries.

The queue drains at the outer callback boundary or through one posted owner-
thread operation. A touch during a drain sets one deferred follow-up. No
revision job list exists. Explicit boundaries disable fallback polling.

Foreign code may retain an HDC or use raw `GetDC`/`ReleaseDC`, which cannot be
intercepted reliably. Those surfaces retain a one-shot hash probe. An unchanged
surface backs off from 33 ms through a bounded 250 ms maximum; a change restores
33 ms cadence. The hash is therefore a compatibility detector of last resort,
not the normal dirty mechanism.

Managed owner paint now uses the same bounded discipline. Protected
`DoubleBuffered`, `GetStyle`, `SetStyle`, `OnPaintBackground`, and
`InvokePaintBackground` are present on the generated nominal surface. With
buffering enabled, one size-matched PArgb bitmap is reused across paints;
background and foreground receive the same `Graphics`. A resize during a
callback invalidates the surface epoch, abandons the stale candidate, and posts
one replacement pass. Disabling buffering retires the persistent bitmap while
retaining a coherent ephemeral paint transaction. `Update()` called from
`OnPaint` returns without recursion and publishes one follow-up after the
callback boundary. A throwing callback abandons its candidate and retains the
last published raster; it does not schedule an unbounded retry loop.

All six nominal `Control.Invalidate` overloads now enter one damage path.
Rectangle input clips to the client, repeated requests union behind one queue
bit, `Invalidate(Region)` consumes GUI.Drawing's native retained region bounds,
and `invalidateChildren=true` intersects in parent coordinates then translates
into each child. `NotifyInvalidate` raises `OnInvalidated` and `Invalidated`
synchronously without itself scheduling paint. Persistent buffered paint sets
both `PaintEventArgs.ClipRectangle` and the Graphics clip to the captured damage.
A failed lease restores the damage it took so the next real touch merges with
it; ephemeral paint deliberately promotes to full-surface work because it has
no prior bitmap pixels to preserve.

Managed pointer, key, and text ingress now shares one UI-thread-local queue
while any generated owner-paint lease is active. The queue is explicitly
bounded at 1,024 entries and drains only after the outermost lease returns.
Delivery order is retained, disposed targets are abandoned without application
callbacks, and invalidation caused by delivered input enters the ordinary
post-callback paint flush. Disposing from `OnPaint` also abandons the paint
candidate without querying the retired native peer; stale resize/fault paths
continue to restore damage while live.

## Measured gates

Environment: Wine devel 11.10, Windows Desktop Runtime 10.0.5, generated .NET 10
facade, x64 MinGW GUI.Forms/GUI.Drawing libraries.

- Generator and managed build: 1,104/1,104 captured identities resolved, 251
  facade types emitted, zero build warnings after queue state was moved out of
  the MarshalByRef-derived Control object.
- `native-surface`: sixteen distinct `Graphics.Flush()` backing updates plus one
  bitmap-backed `ReleaseHdc` mutation remain `dirty_queued` with one queue bit;
  callback release produces exactly one capture/import commit, with at least
  sixteen merged requests and equal content/captured revisions.
- `native-surface-fallback`: one otherwise unobservable raw GDI mutation is
  discovered by the bounded adaptive probe and produces exactly one changed
  import.
- `paint-reentry`: `Update()` from `OnPaint` reaches maximum application-paint
  depth one and produces exactly one later pass after callback return.
- `managed-double-buffer`: protected state is readable and style-reflected;
  sixteen invalidations coalesce into one later render on the same 96 x 48
  bitmap; resize in `OnPaint` abandons one stale epoch and produces one 128 x 64
  replacement; background/foreground share one `Graphics`; disabling buffering
  retires persistence and completes one coherent ephemeral paint; a deliberate
  callback fault increments abandonment once and remains unqueued on the next
  dispatcher boundary.
- `managed-damage`: two rectangles synchronously notify and union into one
  clipped lease; Region bounds round outward; child propagation translates the
  intersection; a failed partial lease restores damage; and unbuffered paint
  uses a coherent full clip.
- `paint-input-deferral`: pointer then key ingress during `OnPaint` produces no
  application input callback inside the lease, drains in the same order after
  release, and causes one localized ordinary follow-up. A second control
  disposed from `OnPaint` abandons both queued inputs, retires its paint lease,
  and leaves the queue empty without touching its dead peer.

The six modes are permanent Wine gates in
`tools/run_facade_surface_smoke.sh`.

## Honest boundary

This closes the behavioral center of M11h-P1 but not every compatibility detail.
Still open:

- rectangle-aware `Invalidate(Rectangle)` and useful dirty-region projection for
  direct GDI surfaces; managed owner paint is localized, but releasing an
  opaque HDC remains conservatively full-surface;
- exact nonrectangular Region damage and Graphics-transform/infinite-region
  parity; the current invalidation boundary uses a conservative Region bound;
- full .NET/Wine ordering and exception corpus for every `Update`, `Refresh`,
  `BeginPaint`/`EndPaint`, `GetHdc`, and `ReleaseHdc` overload;
- presentation acknowledgement from the retained host back into the managed
  surface diagnostic; and
- M12 atomic candidate-raster swap, input deferral during retained-core/native
  raster replay, key-preview return-value parity, bounded queue pressure, and
  sustained producer/stall/occlusion soak.

No claim is made that the fallback can infer exact damage from arbitrary foreign
GDI. Its bounded full-surface probe is the explicit cost of admitting an
unobservable external writer.
