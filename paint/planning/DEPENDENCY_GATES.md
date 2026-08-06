# Paint dependency gates

Status: **mandatory; planning gate open, implementation gates closed**.

## PA0 — architect interview closure

Required:

- Round A and B answers recorded with GIVEN/DECIDED/CANDIDATE/OPEN labels;
- first release tool set and explicit exclusions;
- canvas/composite/clipart and flattening semantics;
- file formats, decoder boundary and safe-write behavior;
- undo, collaboration, help, accessibility and platform scope;
- popup-dialog topology and detailed Color conversion/profile behavior;
- a daily workflow and failure corpus;
- no unresolved contradiction that changes the document model.

## PA1 — GUI.Forms consumption gate

A named snapshot must supply the admitted controls, drawing/bitmap operations,
text tool substrate, modal/modeless owned dialogs, help, clipboard, bidirectional drag/lazy
payload, accessibility, headless traces, clean package and native first-platform
host. Capability presence alone is insufficient; Paint scenarios must pass.
No gate is satisfied by replacing required dialogs with persistent panels.

## PA2 — Orchestrator application-services gate

The Paint-used portions of `ORC-APP-001`, `ORC-PCK-001`, `ORC-HLP-001`,
`ORC-XFR-001`, handler and settings families are reconciled, versioned, fixture-
backed and available through the real local service. Missing families may be
removed from the first slice only with an explicit reduced profile; they may not
be silently reimplemented in Paint.

## PA3 — File Manager picker/transfer gate

- reusable Document Picker consumption snapshot;
- no-Engine live navigation;
- open/save/import/export profiles required by Paint;
- File Manager → synthetic Paint file/PNG transfer;
- cancellation, hidden policy, route fallback and unavailable fixtures;
- package can be consumed without File Manager private sources/executable.

Full File Manager release is not required if this exact shared snapshot is
already dogfooded. Conversely, an attractive File Manager window does not pass
PA3 without reusable package evidence.

## PA4 — owner start gate

The grand architect explicitly directs Paint implementation to begin and names
the first platform/sandbox. Passing technical gates never starts code
automatically.

## PA5 — Malkuth release inclusion gate

Paint joining a Malkuth release requires native platform evidence, installer,
documentation, help, handler/file-format registration, migrations, licenses/
SBOM, screenshots and accepted known issues. Paint planning or dogfood does not
make it a Malkuth 1.0 blocker.
