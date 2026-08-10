# Web.Forms cross-project interface negotiation

Date: 2026-08-10

Status: **proposal 001; no schema, compiler API, or ABI frozen**.

Participants: Web.Forms source/compiler authority, GUI.Forms retained-control
provider, and Orchestrator contract registry. Proposed family: `ORC-GUI-002`.

Web.Forms introduces no runtime Orchestrator service and no product-process
edge. The registered cross-project edge is build-time: the Web.Forms compiler
must validate and generate against an explicit GUI.Forms capability/schema
manifest instead of scraping headers, depending on private C++ types, or
assuming every named control/property is behaviorally complete.

## Web.Forms proposal 001

GUI.Forms should eventually publish a versioned, machine-readable authoring
manifest containing:

- control/component kind and public construction identity;
- property name, type, default, reset/serialization rule, mutability,
  initialization order constraints, and declared invalidation effects;
- event/command name, payload type, delivery domain, and lifetime rules;
- ownership, child/content slots, popup/top-layer relationships, and semantic
  hooks;
- admitted layout families/properties, nested ambient-context contributions,
  surface ownership/reveal rules, and exact value domains;
- style roles, baked/exposed property capability, intrinsic/effective state
  matrices, backplane/drawing capabilities, and resource types;
- runtime stable-ID and generated-handle projection;
- supported/unavailable/incompatible capability state with evidence locator;
- C++ package/version, C++17-compatible orthodox generation seams, and optional
  C ABI compatibility range.

The manifest describes public semantics. It exposes no Skia, host, private C++
layout, pointer, allocator, or renderer object.

## Web.Forms obligations

- own HTML/CSS source syntax, diagnostics, bounds, cascade semantics, and source
  compatibility;
- lower only capabilities the selected GUI.Forms manifest truthfully supports;
- preserve source identity and accepted initialization/event order;
- preserve compiled containment, ambient surface/state context, and style
  exposure without runtime selector matching;
- emit generated source against public package seams;
- record compiler/profile/manifest digests in generated metadata;
- reject a profile/manifest mismatch rather than silently substituting a
  different control or style.

## Required first fixtures

1. unknown control/property/state rejection;
2. supported versus present-but-unimplemented distinction;
3. duplicate/missing/mismatched hierarchical IDs;
4. invalid ownership and popup parentage;
5. ordered initialization plus one coalesced layout/paint transaction;
6. identical style-record interning without identity loss;
7. parent-surface reveal versus owned-surface fixtures with no default-color
   holes;
8. generated C++ package/version and orthodox-profile mismatch;
9. File Manager Folder slice capability report with no private-header reach.

## Non-edge

Orchestrator does not receive HTML, CSS, GUI.Forms controls, generated handles,
native windows, or browser state. `ORC-GUI-002` records versioned project
semantics and compatibility evidence only.
