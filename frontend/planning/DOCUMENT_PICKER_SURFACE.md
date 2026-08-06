# File Manager reusable Document Picker surface

Date: 2026-08-06.

Status: **GIVEN product direction; CANDIDATE package/process design; planning
only while Frontend 001 remains closed**.

## Purpose

File Manager will produce a bounded reusable browsing and selection surface for
its own open/save workflows and for first-party applications such as Paint and
Text Editor. It shares File Manager's navigation/object semantics and house
rendering without linking, launching, or pretending to embed the entire File
Manager application.

The surface consumes GUI.Forms in-process and an Orchestrator File Selection
Session. It can navigate live filesystem locations when Engine is absent. It
must not duplicate Engine storage, Orchestrator policy, GUI.Forms controls, or
host-application document logic.

Parent direction:
[`../../planning/APPLICATION_BACKBONE_AND_DOCUMENT_PICKER.md`](../../planning/APPLICATION_BACKBONE_AND_DOCUMENT_PICKER.md).

Under ADR-013, this bounded embedded File Manager browser is the only reusable
application surface permitted to retain File Manager-like side-panel
composition. Paint, Text Editor, Games, and other host applications do not gain
persistent panels by linking the picker; their non-picker secondary tools remain
owned popup dialogs.

## Shared objects

The initial reusable boundary should separate:

- `FileBrowserModel` — live location, entries, identity/address snapshots,
  filtering, sorting, selection, and availability;
- `FileSelectionController` — purpose, cardinality, type filters, filename,
  effective visibility, validation, acceptance, and cancellation;
- `DocumentPickerView` — GUI.Forms composition, focus, keyboard, modality,
  accessibility, house theme, and error/degraded presentation;
- host adapter — obtains the Orchestrator session, owns the picker window/sheet,
  and returns the terminal result to Paint, Text Editor, or File Manager.

Names are **CANDIDATE**. The separation is the requirement.

## Surface profiles

The caller selects a registered purpose profile and bounded parameters; it does
not override controls or layout.

| Profile | Selection | Minimum visible composition | Mutating commands |
|---|---|---|---|
| Open file | one file | owner/title, breadcrumb/path, object field, type filter, Open/Cancel | none |
| Open files | bounded multiple files | same plus selection count | none |
| Select folder | one folder | breadcrumb/path, folders, Select/Cancel | optional create-folder |
| Save as | destination folder plus name | breadcrumb/path, object field, name/type fields, Save/Cancel | optional create-folder; overwrite is explicit |
| Import clipart | one or bounded many admitted objects | open-file profile plus admitted type summary | none |
| Export | destination plus type | save-as profile plus exporter options owned by host outside the file field | optional create-folder |

Rename, delete, preview, search, criteria views, plugin commands, and properties
are absent by default. Each must be admitted per profile; “it exists in File
Manager” is not sufficient.

## Recommended first slice

- owned window or platform-appropriate sheet;
- breadcrumb/path editor and one virtualized list/small-icon object field;
- current folder, back/up, type filter, filename where applicable;
- keyboard navigation, selection, accept/default, cancel/Escape, focus restore;
- hidden-object toggle when the host profile permits it;
- live directory enumeration without Engine;
- exact unavailable/permission/stale-volume/validation presentation;
- native common-dialog fallback when Orchestrator explicitly selects it;
- headless fixtures for every profile and terminal state.

The first slice need not carry the full File Manager tree, preview pane, command
shelf, recent breadcrumb matrix, indexed thumbnails, semantic search, plugins,
or folder-size badges. Reuse is semantic and structural, not a demand to squeeze
the entire main window into a modal.

## Hidden-file behavior

The effective value comes from the Orchestrator selection session and is stored
per stable host application. The picker may expose a session toggle. Remembering
the change is an explicit app-scoped settings operation; it never silently
changes File Manager's own view preference.

Filtering, completion, search, recent locations, selection restoration, and
validation all consume the same effective visibility. Hidden entries cannot be
omitted visually while leaking through a secondary picker feature.

## Engine and Orchestrator loss

- Engine absence never prevents directory enumeration, filtering by ordinary
  visible type/name, or acceptance of a directly navigated object.
- Indexed thumbnails, semantic/provider evidence, and indexed search disappear
  honestly when unavailable.
- If Orchestrator fails after presenting an in-process picker, the surface may
  continue browsing but cannot accept under an invalid policy/session. It must
  reconnect/revalidate or terminate as unavailable.
- Native fallback occurs only if the same request allowed it and a fresh policy
  decision or durable route grant remains valid.

## Save boundary

The picker reports a destination observation and overwrite decision. It does not
write Paint or Text Editor bytes. The host revalidates and performs its own safe
write. File Manager may supply shared first-party write utilities later, but
that is a separate file-operation contract and cannot be inferred from picker
acceptance.

## Reuse and build boundary

**CANDIDATE recommendation:** File Manager's eventual build exports the browser
model, selection controller, and picker view as an independently consumable
first-party package. Paint and Text Editor link that package against the same
compatible GUI.Forms ABI. The shipping File Manager executable is not a library
dependency.

A later out-of-process Picker Host may wrap the same package. Its existence must
not force cross-process native child-window embedding or change the semantic
request/result.

## Required frontend fixtures

1. open one ordinary visible file without Engine;
2. open bounded multiple selection;
3. Text Editor app profile reveals and opens a hidden dotfile;
4. Paint profile hides the same object and does not leak it through completion;
5. save new, save existing, extension correction, cancel, and revalidation;
6. unavailable volume and permission denial;
7. owner close, Orchestrator restart, incompatible GUI.Forms capability, and
   native fallback;
8. keyboard-only, VoiceOver, scale, high-contrast, reduced-motion, and
   deterministic modal-focus restoration;
9. no Engine, stale Engine, and provider-limited search states;
10. Paint import/drag handoff without plugin UI or unbounded eager bytes.

## Open frontend decisions

- exact shared-package location and versioning;
- whether the first picker includes a folder tree or only breadcrumb/object
  field;
- search admission in the first slice;
- overwrite-dialog rendering ownership;
- whether preview is ever admitted in a picker profile;
- standalone Picker Host timing;
- theme/locale selection when host and File Manager preferences differ.
