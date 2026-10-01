# Window cursor interaction validation — 2026-10-01

This development slice adds a move-only, UI-thread cursor-hide lease and checked client-DIP placement to Window/HostServices. Windows implements the native operations. Other backends report unsupported through default hooks. This is not native visible-interaction or cross-platform release validation.

Base: `6ebd337f9a50589114dffe16f51b9043a98a072a`. Authored scope is the cursor_interaction header/core/test, cursor-only declarations in Window and HostServices, invalidation in Window lifecycle and HostSession attach/shutdown, Windows service/message routing and coordinate helpers, and windows_cursor_tests. The root-owned CMake source/test registration is required but excluded from this contributor's commit. Prepared-text/provider changes are unrelated.

The lease records active/released/revoked phase, terminal cause and restoration result separately. Wrong-thread explicit release does not mutate ownership. Lease destruction is required on the creating UI thread. The host retains the state and clears its observer before native teardown. Failed restoration prevents another hide. Shutdown retries cleanup once while the native owner remains valid, including when the first failure occurs during shutdown; the original failure stays observable. Base destruction performs no virtual native cleanup.

Window identity is process-unique with exhaustion refusal, and metrics generations invalidate on resize, scale and host attachment changes. Placement validates the complete current snapshot, finite half-open client coordinates, pixel conversion, actual native client bounds, screen addition and clipping. Native foreground/focus/client authority is checked again before SetCursorPos. Placement does not synthesize buttons or capture.

Windows uses SetCursor(NULL) and WM_SETCURSOR routing, not a transparent bitmap or the process ShowCursor counter. TrackMouseEvent is admitted before hiding, and mouse leave, focus/capture loss, modal entry, hiding and teardown revoke the lease. Restoration follows current cursor policy. Microsoft documents [SetCursor](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setcursor) and [WM_SETCURSOR](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-setcursor); these describe the native mechanism, not validation of this implementation.

Validation: Windows x64, GCC 16.2, Release, two build jobs. Source build uses Audio, Skia and HarfBuzz disabled, Windows host and tests enabled. In Games' ignored build directory:

```powershell
cmake --build .build/cursor-build --target gui_forms_cursor_interaction_tests gui_forms_windows_cursor_tests gui_forms_host_protocol_tests gui_forms_host_windows -j 2
ctest --test-dir .build/cursor-build --output-on-failure -R '^(cursor_interaction|gui_forms_windows_cursors|gui_forms_host_protocol_tests)$'
```

Final focused result: 3/3 passed, 0.35 s. Policy checks cover duplicate/idempotent leases, current-policy restoration, wrong-thread refusal, cross-window and forged metrics, nonfinite/boundary/DPI coordinates, revocation on focus/capture/detach, unsupported/denied/native failures, retained terminal handles, direct shutdown failure with exactly one retry before shutdown_impl, repeated shutdown idempotency, and restoration before modal backend entry. Windows tests use a hidden owned HWND to exercise refusal and coordinate helpers, stale handles, and existing resource lifecycle tests. They never request focus, hide the user's pointer or move it. Host protocol regression tests pass.

Review limitation: visible hide/restore/warp, native modal/focus/capture interactions, multiple visible windows and DPI transitions are not established by these tests. macOS/Linux cursor backends remain unsupported. The complete Application SDK has not been rebuilt or adopted by Games for this slice.

## Coordinator integration review

The coordinator reviewed the complete new cursor header, core policy and tests,
all cursor-related changes in the ten existing source/header/test files listed
below, and the CMake registration against PROGRAMMING_HOUSE_STYLE.md. Review
covered explicit types and initialization, named behavior, native observer and
lease lifetimes, operation order, conversions, failure retention and repeated
work. No remaining blocking finding was identified in this authored scope;
unmodified legacy code and native visible behavior are not certified.

The initial review found that restoration failing for the first time during
shutdown skipped the intended retry before native teardown. The corrected
common closing path now performs at most one retry and retains the first
failure. Focused tests verify this path and modal-entry revocation. Independently
building the four focused targets found them current; the coordinator reran all
three focused tests successfully in 0.21 seconds on the source hashes below.
No desktop pointer movement or hiding was performed by those tests.

Author: Astra  
Sponsor: Rainstar

## Exact reviewed source bytes

gui_forms/include/gui_forms/host/cursor_interaction/cursor_interaction.hpp SHA256 af1857c8ab1e74aa44d6a7b0edd22bf1d1afe0c054840e60cfd585cfaf613587
gui_forms/src/core/host/cursor_interaction/cursor_interaction.cpp SHA256 ad91adc5a98332428cfea4623f93fe2d34259912814e8df9eb04a939ab3b0920
gui_forms/tests/cursor_interaction_tests.cpp SHA256 db01f68230dea16f3040ad14a157b0e3c93c72b3ad6a16d8ba98dbb8101150d3
gui_forms/include/gui_forms/host/services/host_services.hpp SHA256 7b31e0ceee761709ae889145ed0bd8e737faa151ea79ad7c04f63717c676656e
gui_forms/src/core/host/services/host_services.cpp SHA256 c1228a7ed3673bba46e35a18f33cd309083887874930ac6896c59b1543b10078
gui_forms/include/gui_forms/window/window.hpp SHA256 e8bc60142de104340f48f7905b89913c6a8cb65b7366e453df3279d9b6e4cadb
gui_forms/src/core/window/window.cpp SHA256 527bc879a5b79ae76bb75c86763363d676554da4f1b1ab19b9ebe081221a7026
gui_forms/src/core/window/lifecycle/window_lifecycle.cpp SHA256 f44818ea2ee3422554a36f30bdb95862ff852817634591b8e4d01d2191aae877
gui_forms/src/core/host/session/host_session.cpp SHA256 a008a95b845ad60dd44eb3da7df83ca15d4026db98c99c5255bd02df5c6971f2
gui_forms/src/host/windows/services/windows_host_services.hpp SHA256 99640ad14711933f2b7105d799c8769005251d1d78e5e5b145efdb312c1d56d0
gui_forms/src/host/windows/application/windows_host.cpp SHA256 e8c811ac9bd45d9cf9a308f8098fae8fb583bb8700993c77ba3c87d40e497db0
gui_forms/src/host/windows/services/windows_cursor.hpp SHA256 4f459ca241680c3af6a5e22124c3f906b3de1197b0d06d6412ce62d8d44acf4d
gui_forms/tests/windows_cursor_tests.cpp SHA256 d62b767082f700775cae89b181ca969d2ca8c76256065a18a46d8976af611911
