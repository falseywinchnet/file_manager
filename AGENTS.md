# File Manager operating instructions

This repository has accepted several program-spine decisions but remains
pre-architecture in unresolved areas. Do not silently turn research candidates
into architecture decisions, and do not reopen an accepted ADR through older
candidate language.

Before planning or implementation, read:

1. `planning/README.md`
2. `planning/PROGRAM_MAP.md`
3. `planning/PREPLAN_CHARTER.md`
4. `planning/EVIDENCE_REGISTER.md`
5. `planning/DECISION_PROTOCOL.md`
6. `decisions/README.md`
7. the relevant component `AGENTS.md` and contract registry entries
8. `planning/PROGRAMMING_HOUSE_STYLE.md`, the owner's complete coding standard

## House-style acceptance

The owner's 2026-09-30 correction requires the supplied house style in all
first-party implementation, tests, and authored tooling. Every delegated
assignment must name this document and include source review against it.
Passing functional tests does not establish style compliance. Review explicit
types, named executable behavior and retained callback state, ownership and
borrow lifetimes, operation order, initialization, conversions, failure states,
and repeated-loop storage/work. Apply the C++ spelling table to C++; apply
language-neutral requirements to other languages without confusing their syntax
with C++. Generated-code restrictions apply only to the named generated profile.
Do not rewrite vendored dependencies or claim legacy code compliant without
reviewing it. Record the exact reviewed scope and remaining violations.

## Scope boundaries

- The core product is local-machine software. Web search, web stores, cloud
  accounts, and remote content discovery are not core capabilities. If ever
  admitted, they enter through explicit plugins and user-controlled policy.
- Treat Zeta only as a source for indexing and retrieval research. Do not import
  its zeta mathematics or unrelated algorithms into this project.
- Treat BFFT only as a model for research discipline: label claims, keep failed
  experiments, measure against controls, and distinguish reasoned proposals
  from measured results.
- File Manager may not rely on .NET, Java, Godot, or a bundled web engine.
  GUI.Forms and the frontend are disciplined C++; the indexing/search engine is
  Go; Orchestrator and hostile plugin supervision are Rust. C# bindings may target a
  native UI engine without making .NET part of File Manager.
- Modern.Forms is fetched source material and a possible compatibility/API
  frontend over a new native retained engine. Its present .NET implementation is
  not an admitted File Manager runtime dependency.
- Do not expand features merely because an operating system file manager has
  them. Absence is a first-class design decision here.

## Program boundaries

- `orchestrator/` is the active Rust Orchestrator repository and canonical
  integration authority for cross-project API/ABI semantics, capability needs,
  and availability. New edges enter its registry and project-local negotiation
  process before adapters freeze.
- `frontend/` is the end-user application project and Orchestrator's GUI for
  settings/service controls. Frontend 001 opens only after Orchestrator Core 1.0
  is available, the named GUI.Forms go-ahead is recorded, and the architect
  explicitly directs implementation to begin.
- `engine/`, `gui_forms/`, and `kolmogrov/` remain independently buildable and
  own their private implementations.
- `malkuth/` is the planning-only suite release, installer, documentation, and
  website program. It does not absorb component versions or open publishing
  implementation before its release gates.
- `paint/` and `text_editor/` are interview-first planning subprojects. Their
  planning gates are open; source/build implementation remains closed until
  their local dependency gates and explicit architect start directions pass.
- `games/` and `lexicon/` are planning-only future projects. Research/interview
  work is open; source/build/provider implementation remains closed until their
  local gates and explicit architect start directions pass.
- `plugin_runtime/` is frozen legacy research input. New runtime implementation
  belongs to Orchestrator's plugin-supervisor subsystem after its gate opens.
- `web_forms/` is the planning-stage Web.Forms authoring/compiler project. It
  may define a bounded browser-valid HTML/CSS source profile and generated C++
  contract over public GUI.Forms; it may not introduce JavaScript, a runtime
  browser/DOM/CSS engine, or implementation before its local gates pass.
- Kolmogrov similarity is a gated core engine candidate channel. It is distinct
  from AI interpretation and personal semantic memory, which belong in
  Orchestrator-managed hives beyond the engine's critical boundary.
- Persistent left/right panels are reserved to File Manager and its embedded
  picker/browser projection. Other first-party applications use owned popup
  dialogs for secondary tools.

## Epistemic labels

Use these labels in planning and design records:

- **GIVEN** — directly required or excluded by the grand architect.
- **OBSERVED** — directly present in a named source or code path.
- **MEASURED** — reproduced by a named benchmark with environment and data.
- **HYPOTHESIS** — falsifiable proposed explanation or mechanism.
- **CANDIDATE** — an option admitted for comparison, not selected.
- **REJECTED** — failed a declared gate; retain the reason and evidence.
- **DECIDED** — accepted by an explicit decision record with reversal cost.

Never describe a candidate as revolutionary, fast, native, lightweight, or
semantic without naming the measurement or the deliberately unverified status.
Approximate indexes may propose candidates; exact filesystem identity and exact
stored records remain authoritative.

The GUI must not be immediate-mode. Keep authoring style, serialized description,
runtime state retention, layout, native-control wrapping, and rendering strategy
as separate axes. The live surface hypothesis is imperative authoring compiled
through a PTP-like declarative schema into a retained, custom-rendered native
core; it remains a hypothesis until `planning/SURFACE_PIPELINE.md` is resolved.

## Decision hygiene

Architecture decisions live in numbered records under `decisions/`.
Each must state the question, constraints, candidates, measurements, failure
modes, chosen option, rejected options, reversal path, and owner approval.

Performance proposals require a baseline and workload. Preserve negative
results. Optimize the measured bottleneck. A clever data structure is not a
relevance model, and a relevance model is not an identity store.

## M4 Mac mini remote build and GUI operation

### Current Shadow desktop bring-up (2026-09-29 owner direction)

The active Windows checkout is now `C:\Users\Shadow\file_manager`. Work in this
checkout directly; isolated worktrees are not required. Borrow the adjacent
Plan Paint compilation toolchain read-only and keep File Manager build outputs
under its own `.build/` directories. The owner authorized visible sibling chats
for staged work and prohibited worker/subagent creation. Coordinate directory
ownership before edits. Native tools are selected by
`tools/Enter-WindowsToolchain.ps1`; see the Windows bring-up record for results.

The Neo/AWDL instructions below are historical remote-build guidance. Do not
assume that their absolute helper path or SSH alias exists on Shadow. Confirm
the remote route before using it. Ordinary real-world development is authorized;
fault-injection tests continue to use their own generated fixture data.

For the historical Neo workflow, the Neo checkout is the authoritative working
tree. From that configured host, use the passwordless `m4mini-awdl` SSH alias for direct inspection
and `/Users/ultimussecundai/.local/bin/m4build` for compute work. `m4build`
rsyncs the local tree to a deterministic directory below the Mini's
`$HOME/Developer/CodexBuilds/`, prints that resolved directory, and then runs the
given command there. A useful orientation sequence is:

```sh
ssh m4mini-awdl '/usr/bin/sw_vers; /usr/bin/uname -m'
/Users/ultimussecundai/.local/bin/m4build --sync-only
/Users/ultimussecundai/.local/bin/m4build -- <command> [arguments...]
```

Do not edit the mirrored source as the primary copy. Make source changes on the
Neo and run `m4build` again. The helper intentionally excludes `.git/` and
preserves remote `build/`, `.build/`, `target/`, `DerivedData/`, and
`node_modules/` directories, so large build products survive source syncs and
are not copied back. Use direct SSH for read-only checks of remote products,
processes, and logs. Quote remote commands as one shell argument so local path
expansion does not leak into them.

The private AWDL route shares the Mini's physical wireless adapter with Screen
Sharing. The Mini also has ordinary Wi-Fi Internet connectivity; AWDL is the
private build/control route, not its Internet uplink. Prefer fetching pinned
source dependencies on the Neo and mirroring them when the repository workflow
requires authoritative local bytes, but do not diagnose the Mini as offline
without checking its ordinary Wi-Fi route. Expect high-volume sync/build
traffic and frequent screen captures to contend with the interactive display;
avoid needless live refreshes while a build is moving large files.

### GUI operation through Screen Sharing

An SSH process does not inherit the Mini's logged-in Aqua GUI session. SSH is
therefore reliable for builds and noninteractive tests but is the wrong place
to launch a Wine program that must be seen or manipulated. Use this sequence:

1. Keep a relay terminal open on the Neo:

   ```sh
   ssh -N -o ExitOnForwardFailure=yes \
     -L 5901:127.0.0.1:5900 m4mini-awdl
   ```

2. In another Neo terminal, open `vnc://127.0.0.1:5901`, for example with
   `open 'vnc://127.0.0.1:5901'`. Screen Sharing may request the Mini login or
   VNC password. Let the user enter it; never record a credential in this
   repository or a launch script.
3. Open Terminal *inside the remote desktop* and launch the interactive program
   there. A `.command` file is convenient because it runs in the correct GUI
   session and keeps lengthy Wine environment setup out of keyboard entry.
4. Keep shell inspection and builds in ordinary local terminals. Use Screen
   Sharing only for interaction and visual comparison.

For Codex desktop sessions, load the `computer-use` skill before operating the
Screen Sharing app. Target `Screen Sharing` and inspect its screenshot after
each meaningful action. The remote framebuffer and Wine controls may expose
little or no accessibility tree, so coordinate clicks and key presses based on
the fresh screenshot are sometimes necessary. Do not reuse coordinates after a
window moves, a menu opens, or the remote resolution changes. Screen Sharing
keyboard synthesis has occasionally dropped underscores; paste paths when
possible, or make a short temporary symlink below `CodexRuns/` and remove it
afterward. Never rename the authoritative or mirrored source tree as a typing
workaround.

The Screen Sharing session is part of GUI verification, not proof by itself.
Compare the stock and reflected program, exercise controls, close and reopen
owned/plugin windows, and pair that observation with focused automated tests.
Avoid resetting Wine while another useful instance is running; a wineserver
reset ends every process in that Wine session.

### GUI.Forms build details

The `m4mini-awdl` relay shell does not include Homebrew in `PATH`, and the
relay is noninteractive. Use absolute Homebrew tool paths. Fetch the pinned
ignored dependencies into the authoritative local tree first:

```sh
/bin/sh gui_forms/third_party/fetch_text_stack.sh
/bin/sh gui_forms/third_party/fetch_skia_cpu.sh
```

`m4build` deliberately omits nested `.git` directories. Build the already
fetched and patched Skia sources directly into the persistent remote build
directory, then configure GUI.Forms to consume those archives:

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  gui_forms/third_party/build_skia_cpu.sh \
  gui_forms/build/skia-cpu-release
/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/cmake -S gui_forms -B gui_forms/build \
  -DCMAKE_BUILD_TYPE=Release \
  -DGUI_FORMS_SKIA_PREBUILT=ON \
  -DGUI_FORMS_SKIA_OUT=gui_forms/build/skia-cpu-release
/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/cmake --build gui_forms/build --parallel
/Users/ultimussecundai/.local/bin/m4build -- \
  /opt/homebrew/bin/ctest --test-dir gui_forms/build \
  --output-on-failure --parallel 8
```

The Windows/MinGW renderer build uses persistent `.build/` projections because
`m4build` preserves that directory across synchronizations. The recorded Skia
patch selects only the MinGW-compatible Windows file/logging ports; it does not
enable Skia GPU backends or WIC:

```sh
/bin/sh gui_forms/third_party/fetch_skia_cpu.sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh gui_forms/third_party/build_skia_cpu_windows_mingw.sh \
  gui_forms/.build/skia-windows-mingw
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh gui_forms/tools/configure_windows_mingw_m4.sh \
  gui_forms/.build/windows-x64-skia-make \
  gui_forms/.build/skia-windows-mingw
/Users/ultimussecundai/.local/bin/m4build -- \
  /usr/bin/env PATH=/opt/homebrew/bin:/usr/bin:/bin \
  /opt/homebrew/bin/cmake --build \
  gui_forms/.build/windows-x64-skia-make --parallel 10
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh gui_forms/tools/stage_mingw_runtime.sh \
  gui_forms/.build/windows-x64-skia-make
```

The Mini's native arm64 .NET 10 SDK is user-scoped at
`$HOME/.local/share/dotnet-sdk-10.0.105`; the separate Windows x64 SDK/runtime
used by Wine and as the Windows Desktop reference source is at
`$HOME/.local/share/dotnet-win-x64-sdk-10.0.105`. Build the checked-in generated
facade and its two managed consumers in one remote invocation (ordinary
`bin/`/`obj/` products are intentionally not persistent across later
`m4build` synchronizations):

```sh
/Users/ultimussecundai/.local/bin/m4build -- \
  /bin/sh -c '
    export DOTNET_ROOT="$HOME/.local/share/dotnet-sdk-10.0.105"
    export GUI_FORMS_WINDOWS_DESKTOP_REF="$HOME/.local/share/dotnet-win-x64-sdk-10.0.105/packs/Microsoft.WindowsDesktop.App.Ref/10.0.5/ref/net10.0"
    gui_forms_dotnet="$DOTNET_ROOT/dotnet"
    "$gui_forms_dotnet" build gui_forms/generated/facade-v1/System.Windows.Forms/System.Windows.Forms.csproj -c Release
    "$gui_forms_dotnet" build gui_forms/tools/facade_smoke/GuiForms.FacadeSmoke.csproj -c Release
    "$gui_forms_dotnet" build gui_forms/tools/facade_behavior_smoke/GuiForms.FacadeBehaviorSmoke.csproj -c Release
  '
```

Proprietary specimens, writable profiles, extracted bundles, and runtime logs
stay outside Git under the Mini's `$HOME/Developer/CodexRuns/`; never place them
in the mirrored source tree. Keep MME out of radio-consumer profiles: it did not
work in this Wine setup. Enumerate the actual host endpoints and pin the
applicable WASAPI input/output names.

## Voice

Be direct, curious, and exact. Name the object, its status, its evidence, and
its unresolved edge. Prefer a precise exclusion over speculative feature creep.
