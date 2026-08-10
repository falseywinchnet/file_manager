# Bounded data projection 001

Date: 2026-08-10

Status: **CANDIDATE answer to WF010; not accepted**.

## Recommendation

Do not admit a general binding language in 0.1. Admit, if the first atlas slice
requires it, one narrow one-way item-projection facility for virtual lists,
trees, grids, search results, and object-field rows.

Ordinary controls remain imperative: application C++ reads and writes generated
typed handles. A virtual collection may reference one static item template:

```html
<template id="file-manager.objects.item"
          data-wf-item-template="FileObjectProjection">
  <img data-wf-field="icon" alt="">
  <span data-wf-field="name"></span>
  <span data-wf-field="size"></span>
</template>
```

The named projection schema supplies explicit typed fields and one stable item
key. The compiler creates one native item recipe; the virtual host applies it to
visible snapshots. It does not clone a DOM node or invent source IDs for rows.

## Hard exclusions

No expressions, dotted property traversal, reflection, converters, arithmetic,
conditionals, loops, function calls, observers, automatic dependency tracking,
two-way writes, or application side effects are admitted.

Finite Boolean/enum state fields may select precompiled style/state variants.
Activation or editing emits `(host ID, stable item key, command ID, typed
value)` to a named C++ handler. C++ validates and applies the operation, then
publishes a replacement snapshot. Markup never writes the model.

## Why this narrow lane may be necessary

Requiring handwritten C++ row composition for every virtual tree, object field,
criteria result, or correspondence row would reintroduce the slow visual grind
Web.Forms exists to remove. A general binding language would instead recreate a
script/runtime model. Static compiled projection is the intermediate.

## Gate and unresolved edges

Before admission, fixtures must close schema ownership, optional/null fields,
localization, editor commit/cancel, virtualization recycling, accessibility,
and conditional visibility. If a representative dynamic atlas collection can
retain WYSIWYG without this facility, defer it from 0.1.

