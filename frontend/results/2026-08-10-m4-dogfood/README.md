# File Manager M4 dogfood 001

Status: **REJECTED as a 1.0 claim; retained as 0.001-alpha subsystem and
negative visual/interaction evidence**.

Date: 2026-08-10.

The requirement-by-requirement release check is in
[`COMPLETION_AUDIT.md`](COMPLETION_AUDIT.md).

The corrective protected-root 1.0 run is recorded separately in
[`../2026-08-11-m4-dogfood/README.md`](../2026-08-11-m4-dogfood/README.md).
It does not rewrite or erase this rejected baseline.

The grand architect's 2026-08-11 direct inspection found that most visible
controls were inert, the program did not resemble or behave like the accepted
prototype, and synthetic `Places`/`Recent locations` language was incorrect.
That observation supersedes the earlier promotion wording. Green subsystem
tests below remain valid only for the behavior they actually cover.

Host: `Joshuah's Mac mini`, Apple M4, macOS arm64. Source remained authoritative
on the Neo and was mirrored with `m4build` to
`$HOME/Developer/CodexBuilds/file_manager-2d80cdb86b7d`.

## Built object

- Web.Forms accepted `frontend/ui/boards/file_manager/file_manager.wf.html`:
  105 retained runtime nodes, 77 rules, 46 state variants, zero decorations.
- Stage 2 emitted the full fail-closed GUI.Forms public C++ tree into the
  frontend build directory.
- The independently buildable macOS bundle linked only public
  `GUIForms::Application` plus the Orchestrator C++ source client and the
  frontend model.
- The final bundle contains and loads
  `Contents/Frameworks/libgui_forms_application.0.dylib`; its only runtime
  rpath is `@executable_path/../Frameworks`.
- The installed release bundle reports version `1.0.0`, is arm64, passes strict
  deep ad-hoc signature verification as `local.filemanager.frontend`, and its
  executable SHA-256 is
  `a761d4ee9e405b1033e3a48f6425b7602178354d3c4fe9e1d731c4f728f9297c`.
- The Mini's ordinary Internet default route remained `192.168.10.1` on `en1`;
  the AWDL alias was used for source transfer/build control, not as its update
  route.

## Screen Sharing observations

The browser and native program were launched from Terminal inside the M4 Aqua
desktop, not on the Neo.

- `web-forms-authored-source.png` shows the actual browser-valid File Manager
  authoring source maximized in Brave on the M4.
- `native-file-manager-root.png` shows the retained native app reading 24 real
  objects from the protected mirrored repository with live
  `Core ready · GUI.Forms available` status.
- `native-file-manager-icon-view.png` shows the installed self-contained bundle
  switching the same 24-object retained tree from details to icon layout and
  publishing `Icon view` in the status line.
- Selection updated the right inspector, folder double-click entered
  `.gitbooks`, Back restored the root generation, and committing `front` in the
  current-folder filter reduced the object/tree views to `frontend`.
- Closing the installed app removed its process completely after the frontend
  worker joined; no `File Manager.app/Contents/MacOS/File Manager` process
  remained.
- `native-protected-rename-editor.png` shows the retained inspector rename
  editor with basename selection and explicit Enter/Escape/collision guidance.
- `native-protected-delete-undo.png` shows the disposable corpus after a
  two-step Delete moved `read-me.txt` to the separate quarantine and exposed
  one exact Undo command. Remote inspection observed the source absent and
  `fm-q-00000006-00000-read-me.txt` present in the quarantine; Undo restored
  the source and emptied the user-visible undo state.
- New Folder selected the created object, Undo removed it, Rename preserved the
  selected device/inode identity, and Undo restored the original basename.
- `native-protected-copy.png` shows a staged Copy of `read-me.txt` published in
  `Documents` and selected after refresh. Remote `stat` observed source inode
  `296832604`, copy inode `296851379`, the same device `16777234`, and no
  remaining `.fm-stage-*` object.
- `native-protected-move.png` shows `ledger.csv` published in `Images` with the
  exact Undo command visible. `native-protected-move-undo.png` shows the
  destination empty again; remote `stat` observed the restored source at its
  original inode `296832602` on device `16777234`.
- `native-protected-collision-dialog.png` shows a real second Copy into the
  occupied `Documents` destination being refused by the native GUI.Forms host
  dialog. It names `destination_exists`, the source, the destination, and the
  no-overwrite law; dismissing it returned to the still-operational browser.

These screenshots are observations, not proof by themselves. They are paired
with the tests below.

## Automated evidence

- Web.Forms: 27/27 tests passed locally; a fresh build of the product source
  emitted deterministic IR, descriptor C++, and the complete GUI.Forms tree.
- GUI.Forms: an initial full M4 run reached 61/62 before
  `gui_forms_dispatcher_tests` stopped making progress. The stuck test process
  was terminated; the isolated dispatcher test passed immediately, and a
  second full run with a 60-second per-test bound passed 62/62 in 2 seconds.
  The first hang is retained as negative evidence rather than erased.
- GUI.Forms FM0 clean external install/consume: configured, linked, and
  executed `--model-only` on the M4.
- Frontend: 8/8 Release suites passed both in the product build and in a fresh
  M4 configure/build directory. Navigation covers ordering,
  filtering, identity retention, cancellation, no-follow direct navigation,
  and out-of-root refusal. Operations cover opt-in, deterministic create,
  rename/undo, replacement and collision refusal, symlink-leaf quarantine,
  directory quarantine, occupied restore, staged regular-file and directory
  copy, copied symlink leaves, copy cancellation/cleanup, recursive and
  collision refusal, mid-traversal cancellation cleanup, injected partial
  disk-full cleanup, exact permission denial, cross-volume move refusal,
  same-volume move/undo, and one-step undo replacement. Fault injection is an
  explicit constructor-owned test seam and is absent from product launches.
- The added suites cover internal drag validation; streamed SHA-256 known
  answers, cancellation, replacement and no-follow refusal; fixed-argv Open and
  Terminal Here with hostile path names; bounded UTF-8/PNG preview; all picker
  controller profiles; and the GUI.Forms picker composition. Failed picker
  navigation retains the last usable location and selection while publishing
  the refusal.
- A clean external CMake consumer configured against the installed
  `FileManagerDocumentPicker` package, linked
  `FileManager::DocumentPickerView`, and printed
  `File Manager Document Picker 1.0 consumer linked` without linking the File
  Manager executable.
- A fresh AppleClang AddressSanitizer/UndefinedBehaviorSanitizer build passed
  the same 8/8 suites with halt-on-error. The first sanitizer invocation had
  requested leak detection; Apple's arm64 ASan runtime rejected that unsupported
  option and aborted all tests before product code, so the corrected run used
  `detect_leaks=0`. No leak-sanitizer claim is made.
- Engine `go test ./...` passed across every package. The contained installed
  Engine executable SHA-256 is
  `7c3a99ef52f1b3bbdf865c96387f53938505b33c17897cdc590120245c4e32b3`.
- Orchestrator: all Rust unit/integration/C++/fixture/live-search/hostile suites
  passed, generated service documentation was current, and Clippy passed with
  warnings denied using the installed stable M4 toolchain. `ORC-GUI-001` now
  projects the exact named FM0 manifest as `available`. The final typed C++
  probe rebuild also rejects a result bound outside the closed `1..1000`
  contract before issuing a request.
- The final rollback-safe LaunchAgent update installed artifact
  `0773b13cfc81018891cbbcfd93a08baadaedd1cefd44613065a5f9f17f8443b2`.
  Its live bootstrap—not a fixture—reported release ready, 25 contracts, 28
  capabilities, 16 settings fields, two services, zero opening blockers, and a
  ready contained Engine. The typed C++ client fetched a one-result catalogue
  search page with a source-bound cursor and fetched the complete second page.

## Honest boundary after this run

Available now: self-contained native protected-root launch; generated retained
shell; live bootstrap; bounded asynchronous navigation; history/up/root;
direct protected path entry; folder tree; details/icons; installed Engine
search through Orchestrator with source-explicit pagination; transactional
settings and identity-bound service controls; bounded built-in text/PNG
preview; default Open; Copy Path; fixed-argv Terminal Here; streamed SHA-256;
internal drag; and an explicit disposable-root mutation profile with New
Folder, inline Rename, staged Copy, same-volume Move, destination Paste,
two-step recoverable Delete, and one-step Undo. The reusable installed picker
package covers open-one, bounded open-many/import, select-folder, save-as, and
export semantics without requiring Engine for browsing.

Still gated: crash/restart operation recovery and physical-volume confirmation
of deterministically injected transfer faults; dynamic `ORC-HND-001` Open With;
the dynamic `ORC-CMD-001` registry; plugin previews; native application menus;
general audit/event replay; VoiceOver promotion; Developer ID signing,
notarization and installer/update flow; daily-root admission; Windows; and
Linux. Mutation controls remain visibly disabled on ordinary read-only
launches, and the ad-hoc signature is not a distribution signature.

A direct launch through the temporary binary symlink painted geometry but no
text because bundle resource discovery was bypassed. That route is **REJECTED**;
all counted GUI dogfood used the `.app` bundle from Terminal inside the M4 Aqua
desktop.

Browser/native raster correspondence remains **UNMEASURED**. The two lanes are
visually close enough to continue implementation, but no pixel metric is
claimed by this record.
