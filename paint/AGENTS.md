# Paint subproject operating instructions

This is the planning-only subproject for Malkuth's future classic direct image
editor, working name **Paint**. It is a first-party GUI.Forms/Orchestrator/File
Manager-backbone consumer, not a GUI.Forms demo and not an implementation of
missing framework or picker capabilities.

## Current permission

The architect interview and paper design gates are open now. Implementation is
closed until `planning/DEPENDENCY_GATES.md` records every required snapshot and
the grand architect explicitly says to begin Paint code.

On a newly started sibling task:

1. Read every file routed below and inspect dependency status read-only.
2. Spend the first turn asking Architect Interview Round A. Do not create source,
   build files, assets, formats, or prototypes.
3. Record answers with epistemic labels; spend the next turn resolving
   contradictions and asking the necessary Round B questions.
4. Produce a proposed Paint decision ledger and gate delta. Do not implement
   merely because the questions were answered.
5. Begin code only after the implementation gate and explicit owner direction.

Before work, read:

1. `README.md`
2. `planning/CHARTER.md`
3. `planning/CAPABILITY_PROFILE.md`
4. `planning/DEPENDENCY_GATES.md`
5. `planning/ARCHITECT_INTERVIEW.md`
6. `planning/IMPLEMENTATION_SEQUENCE.md`
7. `planning/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`
8. `planning/COLOR_DIALOG.md`
9. `../planning/APPLICATION_BACKBONE_AND_DOCUMENT_PICKER.md`
10. the root `AGENTS.md`, accepted ADRs, and applicable Orchestrator registry

## Hard boundaries

- No .NET, Java, Godot, bundled browser engine, or GPU capability.
- Use GUI.Forms only through a named public consumption snapshot; do not copy or
  patch GUI.Forms internals from this project.
- Orchestrator owns application identity/profile, picker/help/handler/transfer
  contract authority. Paint does not invent local substitutes.
- Paint owns canvas/document semantics and actual file reads/writes. Orchestrator
  and File Manager do not own canvas bytes.
- General non-PNG image decoding remains outside GUI.Forms. Paint decoder scope
  requires an explicit threat/capability decision.
- “No layers product” does not authorize a misleading file format. Internal
  compositional objects, flattening, and export behavior must be explicit.
- Preserve user files. All early mutation tests use disposable roots and files.
- No web search, web store, account, or ambient network collaboration.
- Paint has no persistent left or right panels. Secondary tools—including
  detailed Color and Help—are ordinary owned popup dialogs or bounded
  greaseboard overlays under ADR-013.
