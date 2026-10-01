# Private loop scheduling model — 2026-10-01

This is Stage 1 development evidence for a serial model, not an exported Audio capability. It contains no native callback, miniaudio node, PCM mixer, atomic command publication or shared clip owners. Inert indices model bounded payload retention and retirement. No listening, concurrency, callback quiescence, backend destruction or installed SDK result is claimed.

Scope: `src/audio/loop_transport/scheduler.hpp`, `scheduler.cpp`, and `tests/audio/audio_loop_scheduler_tests.cpp`. Root owns the tests-only CMake registration. Existing Audio implementation and public headers are untouched.

The model admits at most 16 commands at each explicit `ingest()` boundary. Successful enqueue and render-side admission/application have separate receipt phases. Unpaused output frames advance even when empty or stopped; pause freezes the frame and source positions while commands can still be admitted. Stop preserves the pause flag and monotonic frame. Epoch zero and exhausted request IDs refuse authority rather than recycling it.

An incoming clip carries its own bar grid, adopted only when it becomes current. Transition timing uses the current clip's grid and actual loop length. A boundary must be strictly after admission plus lead, and no earlier than the end of an existing outgoing fade. A pending change during that fade keeps its original cutoff. Loop end is an additional boundary, preserving Four Pegs tier one's eleven-frame residual. Empty playback starts at its first unpaused sample without a source-grid delay. Source arithmetic skips complete cycles by division and checks additions.

At switch sample B, incoming frame zero has full master gain. Outgoing fade N=0 or N=1 is absent at B. For N>=2, outgoing sample B+j has gain `1-double(j)/double(N-1)` for j in [0,N). Thus first and last gains are exactly one and zero. Overlap is not a constant-power crossfade and a future mixer may clip it. The model does not read or mix PCM. Master gain is finite [0,1]; invalid input preserves the previous value.

Storage is fixed: 16 commands, 32 payload indices and 64 receipts. Terminal receipts can expire; nonterminal receipts cannot be evicted. Cancellation of a retained applied request is too late; expired history reports an expired target instead of guessing its outcome. Exhaustion refuses new work without cancelling a previous pending change. Retired payload indices cannot be reused until explicit producer collection. A returned sample plan is consumed before the next model operation; the final outgoing plan retains its index until the next sample step. These serial ordering tests are not a concurrent lifetime proof.

Validation on Windows x64/GCC 16.2, Release:

```powershell
cmake --build .build/cursor-build --target gui_forms_audio_loop_scheduler_tests -j 2
ctest --test-dir .build/cursor-build --output-on-failure -R '^gui_forms_audio_loop_scheduler_tests$'
```

Result: 1/1 passed, 0.20 s total. A separate `g++ -std=c++20 -O2 -Wall -Wextra -Werror` compilation and run also passed before the final enum-input guard. The CTest run includes that guard. There are 470,028 independent comparisons against a frame-by-frame boundary oracle across short regular and irregular loops, every source position, zero and multi-loop lead, and deferred availability. Focused cases cover strict cutoff minus/equal/plus one, the real eleven-frame residual, arithmetic overflow, malformed coordinates, first sample application, N=0/1/2/3/4 fade endpoints, differing incoming/current grids, pending replacement, original cutoff through fade, paused initial start, cancellation, stop, gain rejection, queue/payload backpressure, terminal receipt eviction, retained pending receipts, ID/frame overflow and preservation of earlier work when a later change cannot be scheduled.

Negative evidence: identical enqueue requests with different admission frames deliberately produce different boundaries (20 versus 40 in the fixture). Callback chunk independence cannot be claimed unless admission frames and command order are held equal. The serial model cannot validate the proposed atomic receipt protocol, source-node attachment, shared-pointer destruction or device-thread shutdown. Those require separately reviewed Stage 2 work.

Exact-scope house-style review: explicit C++20 types and named procedures; no lambdas, arrow access, structured bindings or fast-math. Double fade computation; checked integer sample arithmetic. No allocation, locks, callbacks or custom allocator. Stateful test operations are outside assertions. Fixed-capacity scans use named active counts or declared storage bounds. `tools/check_house_style.py` on all three C++ files reports zero spelling findings; this is separate from the semantic review above.

## Coordinator review and independent rerun

The coordinator read all three authored files and this receipt against the house
style, including boundary arithmetic, queue and receipt state transitions,
retirement order, initialization, failure preservation and repeated work. No
blocking issue was found in this serial experiment. The three source hashes
below were independently matched. The focused target was current and its CTest
passed independently in 0.07 seconds, including the 470,028 oracle comparisons.
This does not validate concurrency, actual PCM output or native callback lifetime.

Author: Astra

Sponsor: Rainstar

Exact source bytes:
gui_forms/src/audio/loop_transport/scheduler.hpp SHA256 e65262d9802b1f9af57659b6102737348b8e606974c018617fd061cd9f354b4c
gui_forms/src/audio/loop_transport/scheduler.cpp SHA256 b491cbfa8a6b10d1422429feaeb99681c90c0baf149edb2db2d0ce134f98c19a
gui_forms/tests/audio/audio_loop_scheduler_tests.cpp SHA256 e458e0129ee50e5c68ad313930629e6138ae776fe8b23a5e7841a189f39df50e
