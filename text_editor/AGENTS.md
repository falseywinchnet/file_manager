# Text Editor subproject operating instructions

This is the planning-only subproject for Malkuth's future plain-text and
configuration editor, **Text Editor**. It is not TextEdit, WordPad, modern
Notepad, an IDE, or a place to implement missing GUI.Forms text behavior.

## Current permission

The architect interview and paper design gates are open now. Implementation is
closed until `planning/DEPENDENCY_GATES.md` records every required snapshot and
the grand architect explicitly says to begin Text Editor code.

On a newly started sibling task:

1. Read the routed records and inspect dependency status read-only.
2. Spend the first turn asking Architect Interview Round A. Do not create source,
   build files, parsers, assets, formats, or prototypes.
3. Record answers with epistemic labels; spend the next turn resolving
   contradictions and asking Round B.
4. Produce a proposed text/save/search/color-hint decision ledger and gate
   delta. Do not implement merely because questions were answered.
5. Begin code only after the implementation gate and explicit owner direction.

Before work, read:

1. `README.md`
2. `planning/CHARTER.md`
3. `planning/CAPABILITY_PROFILE.md`
4. `planning/DEPENDENCY_GATES.md`
5. `planning/ARCHITECT_INTERVIEW.md`
6. `planning/IMPLEMENTATION_SEQUENCE.md`
7. `planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`
8. `planning/CHARACTERS_DIALOG.md`
9. `../planning/APPLICATION_BACKBONE_AND_DOCUMENT_PICKER.md`
10. the root `AGENTS.md`, accepted ADRs, and applicable Orchestrator registry

## Hard boundaries

- No .NET, Java, Godot, bundled browser engine, GPU requirement, RTF-first
  model, document tabs, cloud account, collaborative web document, or IDE
  project system.
- Use GUI.Forms only through a named public consumption snapshot; do not copy or
  patch its text editor inside this project.
- Orchestrator owns application identity/profile, picker/help/handler/settings
  authority. Text Editor does not invent private substitutes.
- Text Editor owns byte/text interpretation, editing, external-change detection
  and actual file writes.
- Syntax/color hints never become authority over file bytes and never block
  plain-text editing when unavailable or malformed.
- Early mutation tests use disposable files/roots. Preserve permissions,
  metadata and user data according to the accepted safe-write contract.
- No automatic privilege escalation to edit protected configuration files.
- Text Editor has no persistent left or right panels. Find/Replace, Characters,
  Help and other secondary tools are ordinary owned popup dialogs under ADR-013.
