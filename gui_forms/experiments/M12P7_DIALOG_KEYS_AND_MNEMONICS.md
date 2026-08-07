# M12-P7 dialog keys and mnemonics

Status: **MEASURED PARTIAL**. Date: 2026-08-06.

## Question

Can GUI.Forms express the useful WinForms dialog-command center without a
platform message loop or a concrete renderer dependency: ampersand mnemonics,
programmatic button activation, Label/GroupBox focus transfer, and Form-style
accept/cancel commands?

## Retained contract

- `parse_mnemonic_text` treats one `&` as a marker, `&&` as a literal
  ampersand, and retains the marked Unicode scalar. `is_mnemonic` compares
  Unicode identity with ASCII case folding.
- Label, GroupBox, ButtonBase, Button, CheckBox, RadioButton, and LinkLabel use
  the marker-free text for measure, paint, and semantics when `UseMnemonic` is
  enabled. This is framework behavior, not showcase string preprocessing.
- Window sends Alt+letter/digit to the active focus-scope root only after the
  focused preview/target/bubble route and accelerators decline the key.
  Effective visibility/enabled state and stable retained child order decide the
  first match.
- Label and GroupBox focus the next selectable control in retained tab order.
  ButtonBase `perform_click` shares one availability/validation gate with
  mnemonic and semantic activation.
- Window owns weak accept/cancel targets. Accept assignment transfers the
  default cue. Enter/Escape run after the focused route, do not steal focus,
  remain inside the active focus scope, and clear synchronously when a target
  subtree detaches or disposes.
- `DialogKeySnapshot` separates mnemonic, accept, and cancel attempts from
  activated commands and validation rejections. A recognized live target still
  consumes its key when validation rejects activation, preventing command or
  host leakage.

## Evidence

`gui_forms_basic_controls_tests` proves escaped display text, ASCII-folded
matching, Alt mnemonic activation, Label focus transfer, validation rejection,
`CausesValidation=false`, Enter/Escape routing without focus theft, default cue
transfer, disposal cleanup, and exact counters. The existing validation,
focus-scope, animation, and 48-cycle Ranges-to-Animation showcase gates remain
green after integration. Focused normal, ASan/UBSan, renderer-free, strict
MinGW PE64 link, and Wine executions pass for basic controls, validation, and
focus scopes.

The Complete Showcase uses authored mnemonic strings and assigns its default
and cancel specimens through Window, so their visual cue and keyboard behavior
exercise the public library contracts. Native AppKit dogfood changed the Toggle
checkbox through Alt+T, then changed the filled slider from 52 to 71 and opened
Animation successfully; the animation semantic value continued to advance and
the rebuilt application remained visible.

## Honest boundary

This is not complete WinForms key preprocessing. Duplicate mnemonic cycling,
locale-sensitive Unicode case folding, keyboard-cue underline policy,
ToolStrip collision arbitration, native `DialogResult`/modal close projection,
managed protected-call identity, and C ABI exposure remain open. A command is
presentation-local; it does not confer Orchestrator authority or capability.
