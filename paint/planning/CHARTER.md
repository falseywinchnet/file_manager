# Paint charter

Status: **GIVEN mission skeleton; exact tools/formats/interactions remain open**.

## Mission

Make the quickest pleasant path from an idea, screenshot, file, or piece of
clipart to a small finished image. Paint should feel like a direct physical
instrument: obvious tools, immediate marks, useful transparency, and reusable
composites—without becoming a professional layer stack, revision database,
asset-management suite, or generative service.

## GIVEN direction

- classic MS Paint is the primary interaction-scale reference, not its precise
  Win32 implementation or every historical bug;
- selected Kid Pix-like playfulness/composition may enrich the experience;
- no user-facing layers product and no general revision-tracking system;
- reusable alpha-bearing clipart/composites are first-class;
- File Manager file or preview objects can be dragged into an active canvas;
- Paint can save canvas objects/composites for later reuse;
- open/save-as uses the shared Malkuth Document Picker;
- application help is local, contextual, and registered through Orchestrator;
- no persistent left or right panels; secondary tools are owned popup dialogs;
- detailed Color is a Paint-owned selector/converter dialog, not a palette
  sidebar;
- Paint is a first-party backbone proof, not permission to place GUI policy in
  Orchestrator or product behavior in GUI.Forms.

## Product shape

- one image/canvas per ordinary window; document tabs are not presumed;
- compact menu/tool/palette/status vocabulary on the primary surface, with
  Attributes, Resize, detailed Color, Help and other secondary operations in
  owned dialogs;
- direct pointer and keyboard operations with touch refinement later in the
  Malkuth 3.0 horizon;
- transparent and opaque image workflows;
- ordinary file import/export plus a native representation only if it preserves
  real value not expressible by PNG;
- bounded undo may exist, but its depth, persistence and promised operations
  require architect approval;
- no online content browser, template store, cloud canvas, AI generation, or
  automatic account/sync.

## Authority

- Paint owns canvas pixels, placed-object semantics, tools, selection, palette,
  import/export and safe-write policy.
- GUI.Forms owns retained UI/drawing controls, modal/help/input/accessibility,
  host presentation, clipboard and drag mechanics.
- Orchestrator owns application profile, picker/help/handler/transfer
  registration and capability availability.
- File Manager supplies the shared picker/browser surface and file-object drag
  source, not Paint canvas behavior.
- Plugins may later decode or provide information under admitted capabilities;
  they do not inject Paint tools, controls, native windows, or trusted code.

## Explicit non-goals

- Photoshop/GIMP/Krita parity;
- animation timeline;
- page layout/desktop publishing;
- layer panel, adjustment layers, nondestructive-filter graph;
- version-control/history browser;
- RAW/photo-development pipeline;
- arbitrary plugin UI;
- docked/persistent tool, color, history, layer, property or help side panels;
- web canvas/collaborative cloud document;
- GPU rendering requirement.

“Collaboration” remains open because the architect used “compositional/collab”:
it may mean playful composition between files/objects rather than simultaneous
network editing. No network collaboration is inferred.
