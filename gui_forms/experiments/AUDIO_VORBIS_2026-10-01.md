# Audio decoder development receipt - 2026-10-01

This is development validation for the optional GUI.Forms Audio component on Windows x64 with GCC 16.2. The complete Application SDK, native listening, macOS and Linux remain unvalidated. Audio implementation commit: `00dca7c6d057cd9a0919ea33ae7c6abef53631cb`.

Dependency: miniaudio git f40cf03f80cdb7e741d43e53b7e706e8c1394bcf; extras/stb_vorbis.c v1.22 LF-normalized SHA256 4c7cb2ff1f7011e9d67950446b7eb9ca044f2e464d76bfbb0b84dd2e23e65636. CMake verifies both pinned source hashes. Full upstream source including Alternative A MIT Sean Barrett 2017 notice installed with licenses. Vendor bytes unchanged.

Allocation review: supplied aligned arena is 16 MiB; owned encoded input <=64 MiB. setup_malloc/setup_temp_malloc/temp_alloc use arena branches (source 930-980). Dynamic alloca branch is not selected with supplied arena. Debug MDCT malloc calls are in #if 0; convenience malloc decoders disabled by NO_INTEGER_CONVERSION and stdio/push APIs disabled. start_decoder checks setup plus permanent decoder plus calculated runtime temporary requirement before first frame (4190). Native scalar/local arrays and fixed 8192-float output block remain stack storage; no claim that total thread stack is part of the arena. libc qsort may allocate outside arena, so the two upstream sorting calls use a private nonrecursive in-place heapsort at include boundary; comparator and caller types preserved, constant scratch, no vendor edits. Codec float32 arithmetic retained upstream; no fast-math.

Global state: crc_table declared 991, sole writer crc32_init 992 called start_decoder 3707. Readers crc32_update called only push page search 4384/4387/4417 (disabled) and vorbis_find_page 4592/4596/4601 (seek/length paths not called by public wrapper). open_memory including initial pump is serialized; get_info/get_samples_float_interleaved/get_error/close/deinit do not touch CRC. DIVIDE_TABLE macro remains disabled. Other static lookup tables are read-only under chosen macros. No promise for foreign direct use of unadvertised stb APIs. Four concurrent workers each open/decode 12 times and compare every sample to an independently owned reference.

Ogg validator: linear scan with compile-time immutable 1 KiB CRC table, no per-page allocation; <=64 MiB input. Single serial, sequential page numbers, valid lacing, continuation consistency, CRC, monotonic known granules <=28.8M, final EOS ending file, no chained/trailing stream. CRC is corruption detection, not proof of packet validity; codec validates packets and test includes valid-CRC malformed packet. Output size is checked and reserved against aggregate 512 MiB before allocation. Actual decoded frames are counted per <=4096-frame request and checked against both per-clip bound and declared extent; only complete exact count published. Nonfinite rejected, Vorbis overshoot explicitly saturated [-1,1].

Cancellation: stop_token checked between <=64 KiB reads, pages, decode requests and publication. Foreign open/decode and open mutex wait are noninterruptible. Failure RAII closes decoder before releasing arena/input, destroys partial output and returns reservation; old clip owners survive. Games loader has bounded queue32 plus one active job, one worker, per-slot cancellation, cancellation/draining/join at loader destruction; stale futures cannot republish after slot clear. Legacy WAV still supported, cancellation around entire legacy WAV call only.

Tests: source CTest includes WAV/PCM compatibility, engine controls/quotas/revocation, exact Vorbis5760 frames, finite nonzero tone, four-worker determinism, cancellation before work and after first decode chunk, 64-byte test arena exhaustion, aggregate quota refusal, reservation release and old-owner preservation/retry, checksum corruption, four truncation cases, mono/rate refusal, no EOS, oversized granule, valid-CRC malformed packet, chained stream and oversized encoded file. Test-only seams not defined in installed build.

Installed consumer: the Games checkout's `.build/audio-sdk-build` (Audio enabled, tests OFF; not full native Application SDK), installed .build/audio-sdk. .build/audio-consumer-build CTest 4/4 passes: installed_audio_api, prepared_audio_assets (243 WAV), compressed_audio_assets (243 Ogg, all17 loop frame counts exact, playback seam error0), game_audio_policy (disabled music no engine/read, async missing-file retry, pause/resume, effect EOF/restart, master/local mute and hidden/pending shutdown). All6 game audio adapter translation units compile against imported GUIForms::Audio. Candidate runtime Ogg set 54,735,409 bytes vs PCM1,169,418,011; source audio76,417,500. Vorbis quality6 is lossy; loop seam test proves playback adds no padding/discontinuity, not perceptual equivalence to original source.

Exact-scope semantic review: named explicit types/callbacks, visible ownership/release order, no auto/lambdas/arrows/structured bindings, stateful operations separate from test assertions, static format selected before PCM kernels, no allocation or format selection inside sample loop, checked sample/frame arithmetic and no partial publication. `tools/check_house_style.py` on the three C++ files: zero spelling findings. git diff --check clean (line-ending notices only).

gui_forms/include/gui_forms/audio/audio.hpp SHA256 f36963b65302eb6f33298d2088bbd01a004265fb85310d752da6b8ab6e051dd2
gui_forms/src/audio/audio.cpp SHA256 23091b14e7c6b889f375964cb2f5839e598ccf96c71897d90f20e23fccbd6cd2
gui_forms/tests/audio/audio_service_tests.cpp SHA256 658dc474539655c09f7aab9ee150b712486328a81479791bdc98f03cf319a7de
gui_forms/cmake/Audio.cmake SHA256 91e10996489ce637ddddfbc786c51e6eb2dc7cc0560ed1566d800022f6a342b9

## Reproducible test scope and captured results

The source module registers `gui_forms_audio_tests` when `GUI_FORMS_BUILD_TESTS`
is enabled. Build its target and run `ctest -R gui_forms_audio_tests
--output-on-failure`. The decoder fixture is embedded in
`gui_forms/tests/audio/audio_service_tests.cpp`; it needs no external encoder,
asset download, audio device or network at runtime. Private failure seams are
compiled only for the source test configuration.

For the installed boundary, install with `GUI_FORMS_BUILD_AUDIO=ON` and
`GUI_FORMS_BUILD_TESTS=OFF`, then build the same test source in an independent
consumer using `find_package(GUIForms CONFIG REQUIRED COMPONENTS Audio)` and
`GUIForms::Audio`. Do not define `GUI_FORMS_AUDIO_TESTING` in that consumer.
This verifies the production library and public headers rather than private seams.

The Games-specific evidence uses `tests/audio_tests.cpp` with a prepared asset
directory and `wav` or `ogg` argument, plus `tests/audio_policy_tests.cpp` linked
to its portable audio adapters. The asset preparer preserves original audio and
records source/output fingerprints and exact loop counts in its manifest.
These checks are consumer evidence, not toolkit source tests.

Captured final source result:

```text
1/1 Test #1: gui_forms_audio_tests ............ Passed 0.35 sec
100% tests passed, 0 tests failed out of 1
```

Captured final independent installed-consumer result:

```text
1/4 installed_audio_api .............. Passed 0.31 sec
2/4 prepared_audio_assets ............ Passed 2.67 sec
3/4 compressed_audio_assets .......... Passed 8.12 sec
4/4 game_audio_policy ................ Passed 0.18 sec
100% tests passed, 0 tests failed out of 4
```

The compressed corpus reported `Full audio decode: 243/243 files.` All seventeen
music tracks reported seam error zero. Example exact counts: menu 4,388,608;
klondike 6,582,784; switchbox 4,680,000; sticks_stones 5,068,800 stereo frames.
This checks playback against the decoded clip; it does not compare lossy Vorbis
samples to their PCM source or establish subjective audio quality.

This receipt makes no claim of a total-process memory bound or hard real-time
behavior. Aggregate clip payload accounting excludes encoded inputs, per-load
arenas, ordinary stream/control allocations, foreign and caller stacks, backend
state and allocator overhead. Applications must bound concurrent loads; Games
uses one loader thread. Release/native validation remains outstanding.

## Coordinator verification after review corrections

The coordinator independently reviewed the complete authored integration diff
for implementation commit `00dca7c` and the following two corrections: spell
`std::barrier<>` explicitly in the concurrency test; calculate the checked
sample count once before the sample-copy loop. Receipt encoding was also
corrected and the source hashes above reflect that final source.

The coordinator rebuilt and reran the source suite (1/1, 0.34 seconds), rebuilt
the tests-disabled Audio library, reinstalled into a separate development SDK
prefix, rebuilt its independent consumers and reran all four tests (4/4,
10.91 seconds). The coordinator found no blocking issue in the authored
integration scope. This is separate verification of those tests and that source
review; it is not an independent full vendor audit, physical audio listening,
complete application SDK or additional platform validation.
