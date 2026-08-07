# M12-P4 guidance-provider tranche

Status: **MEASURED PARTIAL**
Date: 2026-08-06

## Claim and boundary

**HYPOTHESIS:** reusable validation and help extenders can be expressed as
renderer-neutral retained components without subclassing every target control,
running an idle timer, hiding missing binding behind no-ops, or embedding
browser/network policy.

The family was selected mechanically from the pinned LibreWinForms/.NET 10
member ledger. This tranche covers the ErrorProvider and HelpProvider behavior
centers. It does not claim ErrorProvider data binding, Help TopicId/raw managed
enum parity, ToolTip balloon/title/icon breadth, DML/C ABI projection, or native
accessibility publisher completion.

## Mechanism

- `ErrorProvider` retains one mapping per target and one independent popup-root
  error glyph for each visible mapped target. Glyphs use the ordinary overlay
  plane, damage, hit testing, ToolTip, semantic, popup ownership, and lifecycle
  systems.
- Six alignments, signed bounded padding, provider RTL mirroring, and root/target
  arranged-bound subscriptions produce stable logical geometry.
- `BlinkIfDifferentError` performs six phase transitions, `AlwaysBlink` retains
  a continuing active surface, and `NeverBlink` schedules nothing. The Window
  scheduler suppresses active surfaces while occluded; reduced motion settles
  the glyph visible and revokes its lease.
- Final semantic projection enriches every subclass descriptor after the
  control authors its stock state. Errors set `invalid`; help/error text is
  appended in stable provider-ID order. An otherwise unexposed mapped container
  becomes an accessible group rather than dropping the metadata.
- `HelpProvider` retains string, keyword, navigator, namespace, and an optional
  ShowHelp override. Absent an override, nonempty string/keyword metadata makes
  help effective. Reset returns to that automatic rule.
- F1 is a tokenized Window accelerator. It walks from focused control through
  ancestors, emits the mapped Control's mutable request first, then provider
  policy only if unhandled. No browser, help file, or network action occurs in
  GUI.Forms.

## Deterministic fixture

`gui_forms_guidance_provider_tests` proves:

- error string, HasErrors, invalid semantics, deterministic descriptions, and
  multiple-provider isolation;
- right/left/top/middle/bottom placement, padding, RTL mirroring, movement after
  retained layout, ancestor hide/show revocation and restoration;
- changed-error bounded completion, continuous and disabled modes, zero
  occluded/reduced-motion wake, and a visible settled phase;
- provider and target disposal cleanup with no scheduled remainder;
- help string/keyword/navigator/namespace transport, automatic/explicit/reset
  ShowHelp, focus-ancestor F1, control-before-provider order, handled
  termination, semantic projection, and request counters.

The Complete Showcase timing/provider board dogfoods an invalid endpoint field,
live error adornment, clear/restore command, and F1 help request through public
library classes. No provider behavior lives in the showcase.

Focused gates passed in the normal macOS build, renderer-free build,
warnings-as-errors build, and AddressSanitizer build (`detect_leaks=0`; the
platform runtime rejects leak detection). Win64 compiled the controls, Win32
host, and Complete Showcase with private CPU Skia; the strict Win64 build
compiled controls and host with warnings as errors.

Native AppKit computer-use dogfood then opened the rebuilt board, observed the
red error glyph outside the endpoint field, focused the field, dispatched F1,
and received the authored help in the retained status surface. Clear removed
the glyph and invalid semantic text; Restore returned both. This inspection
found and corrected a redundant `Help: Help:` accessibility label caused by
prefixing an already labelled native description. The rebuilt tree now exposes
one `Help:` label with the plain authored payload.

## Result and inference limit

The hypothesis survives the renderer-free behavioral fixture and headless
showcase dogfood. The resulting family is genuine retained toolkit capability,
not a compatibility no-op or specimen passthrough.

This does not establish native screen-reader relation quality, managed facade
exception/event parity, localization refresh ordering, high-density icon raster
quality, or binding-driven validation. Those remain ledger rows and promotion
gates.
