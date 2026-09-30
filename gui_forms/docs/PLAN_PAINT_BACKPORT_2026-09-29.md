# Plan Paint platform backport

Status: **MEASURED Windows development build/install and 64/64 native tests passed;
macOS/Linux rebuilds not performed in this run**.

The owner directed File Manager to reuse the compilation environment being
installed for adjacent Plan Paint and backport reusable GUI logic while
preserving File Manager's styling. Work uses the Shadow checkout directly.
It preserves File Manager's Web.Forms source and component ownership.

## Source and scope

The exact archive and six patches are recorded in
[`../manifests/plan-paint-backport-2026-09-29.json`](../manifests/plan-paint-backport-2026-09-29.json).
Paint's fetcher checksum-verified the archive and each patch before integration.
Source commit: `64248bcc06a15ccf0126f30e49d52fd0b86f9c56`.

Imported library code includes the portable Application interface, Windows
application package, Linux X11 host, clipboard/image/drop services, popup focus
and ownership fixes, event/dispatcher lifetime fixes, text fallback and mapped
fonts, and Windows/Linux accessibility adapters. Associated tests, pinned
dependency recipes, vendor notices and fonts travel with the code. Existing
source paths were preserved; no File Manager composition or authored styling
was copied from Paint.

The host protocol is now 7. The historical macOS FM0 manifest describes an older
built artifact and remains unchanged. A new development build must not identify
itself as that historical ready artifact. Compatibility and release status
remain separate from a successful development build.

## File Manager wake bridge

File Manager has an existing background-result queue drained on the UI thread.
Paint's portable application entry discarded the host wake callback and did not
project the native dispatch_pending callback. Additive
ApplicationWindowOptions::wake_ready and dispatch_pending fields preserve that
mechanism without exposing native handles.

The host publishes wake on its UI thread before the application ready callback.
Workers may call wake while the window lives; they must stop before it closes.
Wake schedules work; dispatch runs on the UI thread. Exceptions from either
callback are captured by the portable application boundary and cause orderly
close. The native application test exercises a worker wake, UI-thread dispatch
and dispatch-exception containment. Linux projects the same drain callback
through its event-driven update path.

This change does not invent a portable custom-titlebar implementation. File
Manager retains its macOS presentation adapter for titlebar/drag controls;
Windows initially uses the supported native frame.

## Build profile and limits

Shadow uses Paint's MinGW64 installation, File Manager build output at
gui_forms/.build/shadow-windows, and SDK at gui_forms/.build/shadow-sdk. It is a
Windows GDI profile with Skia and HarfBuzz disabled, following Paint's Windows
build. macOS/Linux retain CPU Skia source paths; this Windows run has not rebuilt
them.

From the repository root:

```powershell
. .\tools\Enter-WindowsToolchain.ps1
.\tools\Build-Windows.ps1 -Component Toolkit -Jobs 3 -Test
```

The environment helper changes only process PATH. Override
FILE_MANAGER_MINGW_BIN for another compatible MinGW64 installation. The build
helper stops on command failure and does not install after failed requested
tests. The shared Paint toolchain is read only to this workflow.

Windows accessibility is MSAA. Linux source is X11/XWayland, not native Wayland
or mixed-DPI. Building these sources does not establish visual equivalence,
screen-reader usability, filesystem portability or release readiness. Record
native test results and failures against the actual platform.

## Windows result

GCC 16.2.0, CMake 4.4.3 and Ninja 1.13.2 built the library and all test targets.
A second incremental build refreshed all dependencies after the wake-bridge
header changed. CTest then passed 64/64 native Windows tests in 28.96 seconds,
including worker wake/UI dispatch and exception containment, window lifecycle,
fonts, canvas, accessibility, ABI and host-boundary checks. The SDK was installed
from that consistent build. The DLL hash is recorded in the separate Shadow
Windows development consumption manifest; test output remains in
`.build/shadow-windows/ctest-windows.txt` relative to GUI.Forms.

## MSAA self-reference repair

The first File Manager desktop inspection rendered Home, then appeared to hang
on folder navigation. A separate directory reader returned 22 objects in 12 ms.
The Paint chat reproduced a similar live accessibility hang and captured a GDB
stack in WindowsAccessibility::Accessible::get_accFocus. Its variant helper
returned a fresh VT_DISPATCH wrapper even when the requested runtime identity
was its own. MSAA focus descent therefore repeatedly visited self wrappers.

Paint supplied windows-accessibility-self-reference.patch, now recorded as a
seventh patch in the backport provenance. Self references return VT_I4 with
CHILDID_SELF; actual descendants still return VT_DISPATCH. The focused native
regression checks both descendant focus termination and leaf hit testing. Paint
reported the test failing before the fix and passing afterward. This patch was
applied to File Manager's source and rebuilt locally.

The user stopped Computer Use before a second File Manager desktop run. Native
automated coverage of the repair must be distinguished from an unperformed
post-fix File Manager navigation/preview/popup dogfood session.

After applying the repair, all 64 File Manager toolkit CTests passed again in
14.26 seconds, including the new focus/hit-test assertions. The SDK was
reinstalled; the updated DLL hash is in the Windows consumption manifest.
Log: .build/shadow-windows/ctest-windows-accessibility-fix.txt.

