# GUI.Forms design-language system model

Date: 2026-08-13

Status: **GIVEN direction with CANDIDATE execution model; not an accepted
replacement for the Web.Forms or GUI.Forms architecture records**.

## 1. Diagnosis

**GIVEN:** the recurring failure is not peculiar to GUI.Forms and is not a
second failure mode distinct from the elaborate theme work historically needed
by KDE, GTK, WinCustomize, and similar systems. It is the same failure:
richness does not rinse and repeat because the visual anatomy and relationships
inside controls are not recursively and compositionally addressable.

A renderer may be capable of gradients, images, shadows, masks, animation, and
arbitrary paths while themes remain effectively limited to trivial color
changes. A toolkit may expose thousands of hooks while every ambitious theme
still requires extensive control-by-control manual design. Both outcomes have
the same cause: rendering power exists without a sufficiently compressed,
reusable design-language grammar.

The current GUI.Forms failure is concrete. A monolithic control can know how to
paint segments, yet the authored design language cannot independently and
relationally address its face, cap, overlap, seam, depth, state, terminal
operator, disclosure, and projected history. Adding more bespoke C++ paint
properties reproduces the failure instead of resolving it.

## 2. Governing direction

**GIVEN:** an ideal theme operates nearly entirely as CSS.

**GIVEN:** structural purpose, elements, nesting, and relationships live in the
HTML authored through the bounded Web.Forms profile. The design language lives
in CSS and admitted local style images. The compiler interprets that source and
forms the styled retained control system.

**GIVEN:** every control is itself a system, and systems nest inside other
systems. A button is not an indivisible renderer atom. It is an authored custom
system containing a surface, content, focus indication, optional glyphs, and
state. A chevron breadcrumb segment is a button system placed into a parent-
authored overlap relationship. A file picker is a system containing title,
navigation, location, object, filtering, selection, and command systems.

This is similar in principle to the Codex pet system, but expanded into a large
recursive UI vocabulary: named elements, nested systems, stable state,
scriptable semantic surfaces, and separately authored presentation.

## 3. Three views of one retained system

The same compiled retained tree must support three deliberately different
views.

### 3.1 Structural authoring view

Web.Forms HTML describes:

- semantic purpose;
- containment and sibling relationships;
- named parts and slots;
- repeated and application-projected content hosts;
- commands, state exposure, labels, and accessibility relationships;
- stable binding points required by program code.

HTML must not collapse a rich composed system into a single opaque control name
when its meaningful internal parts and relationships are required for styling.

### 3.2 Design-language view

Web.Forms CSS and admitted local images describe:

- layout and spacing;
- surfaces, images, borders, seams, shadows, masks, and materials;
- typography and glyph presentation;
- state-dependent appearance;
- parent/child overlap and attachment relationships;
- transitions and responsive presentation;
- theme variation over the same structure and behavior.

The exact selector and property profile remains Web.Forms-owned. “Nearly
entirely CSS” does not admit a browser runtime, arbitrary CSS, JavaScript, remote
resources, or executable themes. It requires that supported richness be data-
authored and compositionally reusable instead of reimplemented in C++.

### 3.3 Program view

Generated C++ exposes stable semantic handles and typed operations such as:

```text
open_dialog.location
open_dialog.objects
open_dialog.selection
open_dialog.accept_command
open_dialog.cancel_command
```

The program view does not need a handle for every decorative shadow or chevron
tip. Program addressability and style addressability are different sets over
the same retained system:

- C++ receives stable semantic state, content, commands, collections, and
  events;
- CSS receives structural elements, presentational parts, relationships,
  states, and generated decoration;
- inspection retains enough source and lowering identity to relate both views
  to the executed nodes and display operations.

CSS cannot grant filesystem, service, or product authority. C++ cannot silently
replace authored visual anatomy with a product-specific paint routine.

## 4. Compiler and runtime boundary

The candidate execution direction is:

```text
bounded HTML structure + bounded CSS design language + local style assets
                              |
                              v
             Web.Forms validation and typed lowering
                              |
                              v
       generated semantic handles + retained composition recipes
                              |
                              v
 GUI.Forms retained nodes, layout, input, semantics, inspection, display chunks
                              |
                              v
                    private CPU renderer + native host
```

GUI.Forms provides reusable primitives, deterministic retained state, layout,
input, accessibility, animation clocks, inspection, and renderer-neutral
display operations. Web.Forms provides the round-trippable authored document
and design language. Product C++ attaches application state and behavior.

If Web.Forms lowers `<breadcrumb>` directly to one monolithic C++ painter whose
internal visual anatomy is not represented or selectable, HTML-shaped syntax
has merely hidden the same failure.

## 5. Theme goal

A serious theme should primarily replace or extend CSS and local style assets
over a conserved structural vocabulary. It should not require:

- subclassing every control;
- writing a paint callback for every state;
- reproducing hit testing or accessibility;
- rebuilding application command wiring;
- modifying generated C++;
- knowing private renderer or host types.

This does not imply that every theme can radically alter arbitrary application
structure. It means a control system exposes enough conserved structure,
relationships, and state that radically different admitted presentations can
be expressed without restating its behavior control by control.

## 6. Falsification conditions

The model has failed if any of these remain ordinary practice:

1. Prototype fidelity requires adding a new product-specific C++ paint branch.
2. A theme cannot restyle a meaningful internal part because the runtime exposes
   only the outer control class.
3. Sibling overlap or popup transitions are inferred from incidental paint or
   hit-test traversal rather than authored parent relationships.
4. Generated C++ owns visual constants that existed in the source stylesheet.
5. A visual regression can pass structural tests because no source-to-display
   conservation record names the lost microfeature.
6. Two rich themes require independently rebuilding the same behavioral control
   system.

## 7. Unresolved edges

- The exact minimum element/part/state/relationship vocabulary is proposed in
  `CONTROL_DNA_AND_STYLE_ADDRESSABILITY.md`; it is not yet frozen.
- Parent authority, overflow, projection, and independent render/hit order are
  proposed in `AUTHORITATIVE_CONTROL_REGIONS.md`.
- The source grammar and CSS limits require a Web.Forms proposal and decision;
  this GUI.Forms record does not amend them.
- Theme package compatibility, migrations, asset provenance, parser budgets,
  and untrusted-theme policy remain separate gates.
- The relationship between stock Forms compatibility controls and fully
  authored internal systems needs an explicit migration strategy.
