# Native File Manager paint baseline — Shadow, 2026-10-03 UTC

**MEASURED:** existing `file_manager_application_latency_benchmark` against
the local installed development SDK, real repository navigation, 1340×850
logical window, Win32 DIB CPU / Uniscribe-GDI / WIC renderer. Three separate
processes were run sequentially: partial invalidation, full invalidation, then
instrumented full invalidation. Each completed all five presentation samples,
closed normally and reported zero callback faults. No product code was changed.

This is a narrow rendering diagnostic, not owner dogfood or physical-input
latency. The timer explicitly invalidates either the Back control or root after
each observed presentation. It does not scroll, type, resize or choose previews.
Initial navigation and asynchronous startup can invalidate additional areas.

## Observations

- Partial run: first presentation 130.962 ms; second 60.879 ms with nearly whole-
  window damage; subsequent three 0.598 / 0.370 / 0.402 ms, each adding 960 logical
  units squared of painted damage. Total 193.211 ms. This is evidence that the
  settled small-damage path exists, not a stable percentile estimate.
- Full run: first presentation 109.442 ms; subsequent 40.686 / 25.965 / 26.367 /
  28.694 ms, each painting the whole 1,139,000-unit window. Total 231.154 ms.
  The 16.67 ms period at 60 Hz is a useful comparison, not a measured input SLA.
- Instrumented full run: total 252.428 ms. Existing optional category timers
  recorded fill 22.809 ms, linear gradients 27.158 ms, radial gradients 45.226 ms,
  lines 10.073 ms, text draw 47.749 ms, text metrics 138.748 ms and images 52.051 ms.
  These include work outside presentation (notably layout) and can overlap.
  Do not add them or interpret them as exclusive percentages of frame time.
  Instrumentation changes cost; use `full.txt` for the uninstrumented observation.

**CANDIDATE next diagnostic:** attribute settled full-frame cost by phase and
separate initial text/font/gradient/image preparation. Compare the same rendered
scene before and after any change, including pixels and Unicode/fallback cases.
Prefer reducing measured repeated work or excessive invalidation; these three
runs do not justify a new renderer, cache policy or queue scheduler by themselves.

## Reproduction and provenance

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake --build .build/details-consumer --target file_manager_application_latency_benchmark --parallel 2
.build/details-consumer/file_manager_application_latency_benchmark.exe C:/Users/Shadow/file_manager --native-partial
.build/details-consumer/file_manager_application_latency_benchmark.exe C:/Users/Shadow/file_manager --native
$env:GUI_FORMS_PROFILE_PAINTER = '1'
.build/details-consumer/file_manager_application_latency_benchmark.exe C:/Users/Shadow/file_manager --native
Remove-Item Env:GUI_FORMS_PROFILE_PAINTER
```

Environment: Windows 11 Home 10.0.22621, Shadow VM, EPYC 9354 presented as 4 cores /
8 logical processors, 16757176 KiB visible RAM, GNU/MinGW 16.2.0 Release. The
benchmark was rebuilt at `f633358` (its application code equals PR10 `8a13ba1`)
against the existing `.build/details-sdk`. This local SDK is not a fresh,
complete source-coherent release build; independent native CI is release proof.
Adjacent Plan Paint compilation dependencies were read only. GUI.Forms sibling
prepared-window source was neither changed nor incorporated into this build.

SHA-256:

- benchmark executable: `d45a263ece6dd5171d0e20d2b2580c900d9eb19d6da68811866886f597d596be`
- loaded-directory `libgui_forms_application.dll` (matches installed SDK copy):
  `42d5c96fd2500ebd4194b6c2fa06548e0a138f4af7ae4f7508a51a369e4c9a3c`
- installed `libgui_forms_core.a`:
  `35ebd435bbd4aae240b3e670ec666d18dad019c5be179664334eec8b72344a8a`
- installed `libgui_forms_controls.a`:
  `fc48a737854126beb5d214c6c04cb26afb246fd99f8715cf26ff2bdabd5ec313`

Source inspection scope: `NativeBenchmark` scheduling/metrics in
`frontend/tests/application_latency_benchmark.cpp`, `DibPainter::PaintTimer` and
the existing instrumented paint/metrics entry points in
`gui_forms/src/host/windows/application/windows_host.cpp`. No first-party source
was authored or changed for these runs and no whole-file house-style compliance
is claimed. Uninitialized legacy locals and allocating text-run copies visible
in those existing bodies need separate scoped review before any modification.
