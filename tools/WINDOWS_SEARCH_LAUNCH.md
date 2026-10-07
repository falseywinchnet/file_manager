# Explicit Windows search launch

Extract the whole development package. In its folder, launch against one
existing local root you choose:

```powershell
.\launch_windows_search.ps1 -Root 'C:\chosen\folder'
```

The launcher opens File Manager with real, separate Engine and Orchestrator
processes. Ordinary text searches filenames and root-relative paths without a
catalogue. New Folder, same-parent Rename and their one-step Undo use the
ordinary application policy independently of search. Copy, Move and Delete
remain unavailable in ordinary mode. Closing this File Manager
window gracefully stops both services created for this run; other processes
are not touched. Keep the launcher running until the application closes.

To explicitly build a persistent catalogue for exact metadata criteria:

```powershell
.\launch_windows_search.ps1 -Root 'C:\chosen\folder' -IndexEnabled
```

This opts into an initial scan of that root before the application opens.
Ordinary text still uses bounded live name/path matching; indexed substring
acceleration is not implemented. Exact criteria use the catalogue. Currentness
is manual reconciliation, not a promise of continuous filesystem observation.

State is created in a new private current-user directory outside the searched
root. The path is printed. `-StateDirectory 'D:\chosen-new-state-folder'` selects
a new state leaf whose parent already exists. Existing state is never overwritten
or silently reused. Each invocation has distinct process credentials and state.
State/logs, and an opted-in index, remain for diagnosis after shutdown; no source
files are modified by the services. No root is inferred from Home or from a drive.

The script makes no persistent environment change, service installation,
autostart registration, elevation or execution-policy change. Failure to admit
the exact root or secure private state stops startup. The chosen root and private
logs can contain local path metadata; review them before sharing.

`-CheckOnly` tests admission, service startup and authenticated Orchestrator
status, then stops both without opening the application. It is not a frontend
interaction test. From the source checkout, identify an extracted package:

```powershell
./tools/launch_windows_search.ps1 -PackageDirectory 'C:\build\FileManager' `
  -Root 'C:\chosen\folder' -CheckOnly
```

The package must contain `File Manager.exe`, its DLL/fonts closure, and
`components/fileman-engine.exe` plus `components/orchestrator.exe` from the same
tested checkpoint. The script alone does not update older packaged binaries.

**MEASURED on Shadow:** live and indexed CheckOnly launch/teardown passed against
the clean 21a89b5 package using generated roots and state paths with spaces;
Unicode Omega paths also passed. The ordinary GUI path opened its owned packaged
frontend, stayed alive, accepted an owned-window close, exited zero and removed
both service discovery records. This tests process/environment/lifetime wiring;
actual frontend live/indexed query evidence is recorded separately in
`../backend/orchestrator/conformance/evidence/SHADOW_WINDOWS_PIPES_2026-09-29.md`.
