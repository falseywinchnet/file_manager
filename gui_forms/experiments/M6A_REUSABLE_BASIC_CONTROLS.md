# M6a reusable basic controls evidence

Date: 2026-08-04

Status: **OBSERVED implementation evidence for the first bounded M6 control
extraction**. M6 is not complete. Text editing, scrolling, collection controls,
providers, command models, default/cancel form routing, mnemonics, owner draw,
the C ABI, and the remaining Gallery-private controls stay **OPEN**.

## Boundary and dependency choice

- **GIVEN:** the public retained core is platform-neutral. AppKit and Skia stay
  behind private adapters.
- **GIVEN:** default presentation is professionally Windows 7/10 inspired.
  Portsmouth Rapids is for titles/control chrome; field content keeps the
  content role currently resolved to Lucida Grande by the macOS renderer.
- **CANDIDATE:** `BasicControlStyle` is a provisional plain value record.
  Its palette is not a frozen M9 theme decision.
- **OBSERVED:** the completeness matrix orders panel/label/button/check/radio
  extraction before text, range, and collection families. This slice follows
  that dependency boundary and adds `GroupBox` and `LinkLabel` as narrow peers.

## Public reusable surface

`gui_forms_controls` now exports renderer-neutral `Panel`, `GroupBox`, `Label`,
`ButtonBase`, `Button`, `CheckBox`, `RadioButton`, and `LinkLabel`. Their public
state covers provisional style values, panel borders, label alignment and font
roles, default-button cue state, checked/indeterminate state, radio grouping,
auto-check policy, and visited-link state.

All property mutations retain the core UI-thread and lifetime guard. State
changes invalidate retained measure/paint/semantic state as needed. Text,
click, check-state, checked, and radio-checked notifications use tokenized
events. Callback disposal is explicitly safe: a checkbox disposed from its
first state callback publishes neither later checked nor click callbacks.

The deterministic interaction order is:

- button: matching press/release or normalized Space/Enter release, then click;
- checkbox: mutate, `check_state_changed`, conditional `checked_changed`, click;
- grouped radio: uncheck direct-parent/group peer, check self, click;
- link: set visited, then click.

Radio exclusion is deliberately scoped by direct logical parent and explicit
group name. This is GUI.Forms' declared order, not a claim that every historical
WinForms release used the same order.

## Host and Gallery adoption

The macOS adapter now translates common AppKit hardware positions to USB HID
usage IDs before `KeyEvent` crosses the host boundary. Text continues through
`NSTextInputClient`; native key codes no longer leak into reusable-control
activation. Unmapped vendor/media positions report reserved usage zero.

The Gallery instantiates reusable labels, buttons, checkboxes, radio buttons,
and a link label. Demo-specific model coupling lives in thin subclasses rather
than in the public controls. A visible indeterminate three-state checkbox and a
visited `LinkLabel` contract probe extend the professional basic-control panel.
The command PNG label and font-policy readout are specialized `Label` painters.
The remaining text/range/list/category/instrument/container nodes still use the
Gallery-private control and therefore prevent the M6 exit claim.

## Verification

- Normal build including both private renderers and AppKit: **20/20 passed**.
- Strict Release (`-Wall -Wextra -Wpedantic -Werror`): **19/19 passed**.
- AddressSanitizer build: **19/19 passed**.
- Renderer-free UndefinedBehaviorSanitizer build: **14/14 passed**.
- Renderer-free ThreadSanitizer build: **14/14 passed**.
- `gui_forms_basic_controls_tests` covers retained painter commands and font
  roles, non-intercepting labels, pointer and normalized-key activation,
  tri-state ordering, radio scope/order, visited links, callback disposal, and
  wrong-thread mutation rejection.
- Gallery conformance still proves complete visible-child containment,
  localized display-chunk invalidation, one validated PNG resource, field vs.
  control typography roles, retained state, typed drop acknowledgement, and
  ancestor clipping.
- **OBSERVED manual AppKit smoke:** the rebuilt native Gallery opened at
  900×660 with the new fourth row contained inside `Basic controls`; clicking
  `Show control contract` changed it to the visited color and updated the
  retained command banner. The demo was left running.

## Honest next dependency

The next control expansion should not fake editing or collection semantics in
Gallery code. M4 owns the Unicode editor/shaping prerequisites; M5 owns reusable
layout, scrolling, and virtualization. Within M6, the next independent slice
can extract container identity and a renderer-neutral progress/range value
contract, while `TextBox`, `ListBox`, `ComboBox`, and virtual collections wait
for their declared foundations.
