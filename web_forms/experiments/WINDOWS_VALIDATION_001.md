# Windows developer validation 001

Date: 2026-09-29

Status: **MEASURED Windows compiler/test and build-time browser harness;
native Windows raster comparison remains unmeasured in this run.**

Source base: `7ce5cf44d5fa35b89b182f106ca79c8ab3f44dde` plus the local
Web.Forms browser discovery/shutdown changes described here. Other component
work was present in the shared checkout and was not changed by this task.

## Environment and replay

**OBSERVED:** Windows 11 Home, version `10.0.22621`; shared MSYS2 MinGW64
Python `3.14.7` and GCC `16.2.0` (Rev4); installed Brave executable product
version `154.1.96.59`. DevTools reported `Chrome/154.0.8037.58`, protocol `1.3`,
revision `@08e68f356f25ee2c2d980ff9e3f124a387b0901`.
No browser or Python package was downloaded.

Run from the repository root in PowerShell:

```powershell
$env:PATH = 'C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin;' + $env:PATH
$env:CMAKE_BUILD_PARALLEL_LEVEL = '2'
python -m unittest discover -s web_forms/tests -p test_fidelity.py -v
python -m unittest discover -s web_forms/tests -v
```

Unittest executes serially and the generated C++ checks invoke one compiler at
a time. The CMake cap also bounds any subsequent CMake build in this shell.
This suite does not require building all of GUI.Forms or installing a runtime
browser dependency.

An explicit, repeatable build-time browser capture is:

```powershell
python web_forms/tools/webforms.py capture-browser-fidelity `
  web_forms/boards/controls/button/button.wf.html `
  --viewport 800x600 --text-scale 1.5 `
  --interaction pressed --interaction-target button-study.stage.archive `
  --chrome 'C:\Program Files\BraveSoftware\Brave-Browser\Application\brave.exe' `
  --output web_forms/.build/windows-validation/button-pressed.json
```

The JSON records browser product, executable path, DevTools version metadata,
user agent, viewport, device/text scale, font status, stable IDs, geometry,
materials, and raster probes. Browser upgrades change the measured environment;
retain this metadata when comparing captures.

## Harness changes and retained negative evidence

**OBSERVED:** the original automatic discovery searched only macOS application
paths and the `google-chrome`/`chromium` PATH names. All three real-browser tests
therefore failed before capture on this Windows installation.

Discovery now checks Windows Program Files, Program Files (x86), and Local App
Data for Chrome, Chromium, Brave, and Edge, then Chromium-family PATH names.
Explicit `--chrome` paths take precedence and an invalid explicit path fails
with `WFV004`, rather than silently selecting a different measurement browser.

Version evidence comes from `Browser.getVersion` on the actual capture process.
The harness no longer starts a separate `--version` browser process, whose
console behavior is not portable to Windows GUI executables. Each capture uses
its own temporary profile and requests `Browser.close` before process wait,
with bounded termination fallback.

**MEASURED negative result:** the first patched focused run passed four of five
tests but hit `WinError 32` deleting the profile lock after the pressed capture.
Windows browser children can release profile handles after the launcher exits.
Cleanup now retries `PermissionError` for at most five seconds, then raises;
cleanup failure is not suppressed and fidelity assertions are unchanged.

## Results and scope

**MEASURED:** focused fidelity suite: **8/8 passed**, including three real Brave
captures and three added browser-discovery regressions. The captures exercise
the exact viewport, loaded fonts, seven stable nodes, raster probes, scaled
pressed state, and comparator dimension separation. These are build-time
reference measurements, not proof of native raster equivalence.

**MEASURED:** full unittest discovery: **40/40 passed in 61.302 seconds**,
with no skips. This includes all original 37 tests and the three discovery
regressions; generated C++17 descriptors, C++20 public GUI.Forms projections,
and the material/state execution check passed. The explicit CLI capture above
also succeeded, with loaded fonts and the requested 800x600 viewport and 1.5
text scale. `git diff --check -- web_forms` passed. No capture-owned Brave
process remained at the post-capture process check.

The standalone `build_native_fidelity_probe.py` and `build_native_tree_probe.py`
helpers still contain macOS-specific compiler/link assumptions. In particular,
the fidelity probe links CoreFoundation/CoreGraphics/CoreText and a named Skia
archive layout. The full native Windows fidelity matrix needs a separately
validated Windows GUI.Forms/Skia package and corresponding probe link recipe.
The native build owner confirmed the current Windows package uses GDI and has
no Windows Skia prebuilt. This run does not establish that matrix or promote a
production generation ABI.
The no-runtime-browser rule remains unchanged.
