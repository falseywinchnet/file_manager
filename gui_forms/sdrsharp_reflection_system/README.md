# SDRSharp reflection system

This directory is the complete, sanitized recipe for running a separately
obtained SDRSharp installation through the generated GUI.Forms compatibility
facades on the nearby M4 Mac mini under Wine.

It contains no SDRSharp binaries, extracted bundle entries, configuration,
layout, decompiled/disassembled output, private-field or method inventories,
runtime logs, or reverse-engineering evidence. The runner performs only four
operations: validate and unpack a user-supplied .NET single-file bundle into a
local ignored directory, substitute the GUI.Forms and System.Drawing facades,
resolve runtime/native libraries, and invoke the application entry point.

## Known working arrangement

- The Neo checkout is authoritative and is mirrored to the M4 with `m4build`.
- M4 repository mirror:
  `$HOME/Developer/CodexBuilds/file_manager-2d80cdb86b7d`
- M4 private runtime root:
  `$HOME/Developer/CodexRuns/gui-forms-sdrsharp`
- User-supplied application files:
  `$HOME/Developer/CodexRuns/gui-forms-sdrsharp/specimen`
- Ignored profile and extraction directories are siblings named `profile` and
  `extract`.
- Native build output:
  `gui_forms/.build/windows-x64-skia-make`
- Native .NET SDK: `10.0.105`; Windows Desktop runtime/reference pack: `10.0.5`.
- Wine executable:
  `/Applications/Wine Devel.app/Contents/Resources/wine/bin/wine`
- The working bundle SHA-256 is pinned in `run_reflected.sh`. To try a different
  user-supplied build, explicitly set `GUI_FORMS_REFLECTION_EXPECTED_SHA256`.
- MME audio does not work in this Wine arrangement. WASAPI device pinning is
  mandatory.

The two PanView display controls are routed through the GUI.Forms direct HWND
surface lease by `GUI_FORMS_DIRECT_HWND_TYPES`; all ordinary controls remain on
the retained facade path. This is runtime routing configuration, not an
inspection of application methods.

## Build from the Neo

The M4 alias and mirroring helper are documented in the repository `AGENTS.md`.
From the File Manager repository on the Neo:

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh gui_forms/sdrsharp_reflection_system/build_on_m4.sh gui_forms
```

If the native Windows build is absent or stale, configure/build it first:

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh gui_forms/tools/configure_windows_mingw_m4.sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/cmake --build gui_forms/.build/windows-x64-skia-make -j8
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh gui_forms/tools/stage_mingw_runtime.sh \
  gui_forms/.build/windows-x64-skia-make
```

Do not put the application, extracted files, profile, or logs in the Git tree.
Place the legitimately obtained application directory at the private runtime
path shown above.

## Run on the M4

From a Terminal visible in the M4 GUI session:

```sh
cd "$HOME/Developer/CodexBuilds/file_manager-2d80cdb86b7d/gui_forms"
./sdrsharp_reflection_system/run_on_m4.command
```

Launching inside the logged-in Aqua session matters; Wine GUI programs may not
create windows when started from a plain background SSH session. SSH remains
useful for read-only checks:

```sh
ssh m4mini-awdl 'pgrep -afil "SdrSharpReflectionRunner|SDRSharp|wine"'
ssh m4mini-awdl 'tail -n 120 "$HOME/Developer/CodexRuns/gui-forms-sdrsharp/reflection-launch.log"'
```

To stop only this build without resetting the shared Wine environment, close
the application window normally. If its window closes but the runner remains,
identify the exact `GuiForms.SdrSharpReflectionRunner` PID with the first
command and send that PID `TERM`. Do not run a broad `wineserver -k` while other
Wine work is active.

## Screen viewing and interaction

The M4 has macOS Screen Sharing enabled. Open the `Screen Sharing` app on the
Neo and connect to the M4; VNC is also acceptable when its password is
available. The remote framebuffer is mostly opaque to accessibility APIs, so
interaction is coordinate-based:

1. Take a fresh screenshot before each click or key sequence.
2. Click the M4 Terminal and run `./sdrsharp_reflection_system/run_on_m4.command`.
3. If Wine is active but its window is behind other apps, click the Wine/.NET
   icon in the remote Dock.
4. Keep bandwidth and display resolution modest. AWDL traffic and full-screen
   updates share the same physical wireless adapter, so high screen-sharing
   bandwidth can look like application lag.
5. For radio operation, lower bandwidth/increase buffers as needed and pin the
   desired WASAPI input/output devices.

The screenshots in `screenshots/` record the successful reflected state from
2026-08-10. They are visual results only and contain no extracted program data.

## Expected result

- Main UI, spectrum, waterfall, menus, dialogs, docking panels, and plugin
  controls render through GUI.Forms.
- Plugin windows can be opened, dragged, and closed with their title-bar close
  control.
- The right-side Zoom/Contrast/Range/Offset container and plugin content use the
  same dark application surface as the top toolbar.
- The M4 rendering is smooth when Screen Sharing bandwidth is controlled.
