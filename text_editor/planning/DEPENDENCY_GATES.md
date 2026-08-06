# Text Editor dependency gates

Status: **mandatory; planning gate open, implementation gates closed**.

## TE0 — architect interview closure

Required:

- Round A/B answers and labelled decision ledger;
- byte/encoding/BOM/newline and malformed-input model;
- safe-write, external modification, permissions/metadata and recovery policy;
- document/window, undo, wrap/status/line-number behavior;
- literal/wildcard find/replace grammar;
- first color-hint formats/provider boundary;
- large-file behavior, accessibility, help and daily workflow;
- Characters dialog data/font/encoding behavior and popup lifecycle;
- no unresolved contradiction that changes the source-text model.

## TE1 — GUI.Forms text consumption gate

A named public package must pass the admitted multiline editing, IME, shaping,
bidi, selection/clipboard/undo, decoration, scrolling, accessibility text-range,
modal/modeless owned-dialog/help, focus, headless and native-host scenarios. A constructor called
TextBox is not sufficient.

## TE2 — Orchestrator application-services gate

The Text Editor-used portions of application, picker, help, handler, settings
and optional hint-provider contracts are reconciled, versioned and fixture-
backed. The basic editor may omit optional providers, but it may not create a
second settings/handler/picker authority.

## TE3 — shared Document Picker gate

- reusable package and live no-Engine navigation;
- hidden-file app profile and remember/session behavior;
- open/open-many/save-as, access grant, type-filter and native fallback fixtures;
- consumption without File Manager private sources or executable linkage.

## TE4 — owner start gate

The grand architect explicitly directs Text Editor implementation to begin and
names the first platform/sandbox. Technical readiness alone never starts code.

## TE5 — Malkuth release inclusion gate

Requires native platform dogfood, installers, handler associations, local help,
versioned docs/website captures, safe-write corpus, encoding/newline and large-
file evidence, licenses/SBOM and accepted known issues. Planning or dogfood does
not make Text Editor a Malkuth 1.0 blocker.
