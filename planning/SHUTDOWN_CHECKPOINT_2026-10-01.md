# Shutdown checkpoint — 2026-10-01

The owner announced a machine shutdown in five minutes. This checkpoint preserves
work in progress; it does not declare a release or new runtime availability.
Plan Paint is excluded from this push.

## Verified foundation

Windows candidate/committed DIB development transaction is committed at f26b143.
The optional GUIForms::Audio PCM foundation is OFF by default. Coordinator source
review covered its public header, implementation, tests, Audio.cmake and package
wiring against planning/PROGRAMMING_HOUSE_STYLE.md. Corrections include staged
test effects, named returns, format selection before sample loops, loader failure
boundaries, and callback status that cannot overwrite closed state. Vendored
miniaudio is not claimed house-style compliant.

MEASURED on Shadow Windows immediately before checkpoint: source audio CTest 1/1
passed (0.05 seconds); independent installed consumer CTest 2/2 passed (4.04
seconds), including complete prepared asset decoding. Build directories are
C:/Users/Shadow/games/.build/audio-build and audio-consumer-build. These are
focused offline checks, not physical audio listening or macOS/Linux validation.

Dependency: miniaudio 0.11.23 commit f40cf03f80cdb7e741d43e53b7e706e8c1394bcf;
LF-normalized miniaudio.h SHA256
7e4f3f13c8fe66df2080ac3dd12a89193e3c2463cb7f067c798abd7331cd8ee6.
Upstream license is MIT-0 or Unlicense; unchanged notice is installed. Mixing
uses ma_engine. Decode/resource-manager/generation features are disabled.
Native backend definitions exist for WASAPI, CoreAudio and ALSA; only Windows
was exercised here. PCM input is stereo 48 kHz, at most 600 seconds per clip;
aggregate live clip payload is 512 MiB. These are controlled payload bounds,
not process RSS or hard real-time guarantees. Native-device tests remain open.

## Unfinished work and next steps

- Bounded grapheme and bounded HarfBuzz shaping work is preserved in source.
  Provider reports grapheme tests passed; coordinator acceptance of this new
  bounded text scope remains pending. Bounded shape tests and CMake integration
  may be incomplete. Resume with provider handoff and compile/test before use.
- A2 public prepared text service, typed Painter integration and updated SDK
  remain unfinished. Do not describe existing installed SDK as supporting A2.
- Orchestrator prepared text and Games cursor/audio records are development
  negotiations. Cursor and compact decoder proposals are not availability.
- Games owner requested compact portable audio decoding. Vorbis is a candidate;
  admission, pinned source/license review and bounded decode evidence remain
  open. Do not silently freeze PCM-only release packaging as the final answer.
- SwiftEdit owner requests downloadable MacBook builds. POSIX adapters and
  coherent public GUI.Forms/picker SDK distribution need completion. No Mac
  artifact is claimed by this checkpoint.
- Preserve the frozen Windows SDK. New provider builds require new prefixes.
- Resume exact source review, component gates and house-style checks before
  declaring any unfinished checkpoint scope complete.

Build products and fetched dependencies remain ignored local data, not Git
source. Reconstruct them through the checked-in build tools after restart.
