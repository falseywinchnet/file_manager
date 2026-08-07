# WinForms catalogue generator

This tool turns two pinned reference surfaces into the conservative GUI.Forms
implementation ledger:

- LibreWinForms `PublicAPI.Shipped.txt` at commit
  `1457ed5beef24a7d7e3688a4725c7487da09791e`;
- .NET 10.0.3 `System.Drawing.Common.xml` and
  `System.Drawing.Primitives.xml` reference documentation.

It records every published type/member row and applies only the exclusions in
the accepted GUI.Forms boundary. It does not infer support from name similarity.
Rows associated with a native retained type remain `partial_native` until exact
behavioral evidence promotes them.

M12-P4 promotes the measured ErrorProvider/HelpProvider visual and guidance
centers. M12-P5 promotes retained binding/currency. M12-P6 promotes the exact
validation enums/events/container calls and binding-aware ErrorProvider calls.
M12-P7 promotes renderer-free mnemonic parsing/routing, programmatic button
commands, and Window/Form accept/cancel dialog-key behavior.
M12-P8 promotes stable duplicate arbitration, MenuStrip/popup mnemonic
behavior, exact DialogResult/IButtonControl centers, and Click-before-result
propagation while retaining native modal closure as open host work.
M12-P9 promotes the base Control geometry/order family: exact geometry enums,
masked bounds, preferred/min/max/AutoSize sizing, client/window transforms,
direct-child filtering, nested tab traversal, and topmost-first collection
semantics. True portable screen origin/multi-monitor conversion remains host
contract work rather than being inferred from AppKit or Win32.
M12-P10 promotes the bounded scrolling family: `ScrollableControl`,
`ScrollProperties`, horizontal/vertical properties, exact scroll enums/events,
`Control.AutoScrollOffset`, automatic/manual axes, viewport/display geometry,
and control reveal. RTL/scaling and exhaustive independent WinForms ordering
remain explicitly open.
M12-P11 promotes per-control layout transactions: nested `SuspendLayout`, both
`ResumeLayout` forms, both `PerformLayout` forms, exact `LayoutEventArgs`
payloads, committed geometry while suspended, retained ABI 0.21 state, bounded
re-entry, and fault recovery. Exhaustive independent WinForms event ordering
and designer-scale convergence remain explicitly open.
The generator deliberately leaves arbitrary managed IDataErrorInfo/reflection,
true nested object traversal, Help TopicId/raw enum projection, protected
disposal/site shapes, and unproved constructors partial or missing rather than
inferring closure from the public type name.

M12-P20 adds native instance-owned converter/editor registries and retained
custom-editor ownership, but deliberately promotes no managed `PropertyGrid`,
`GridItem`, `TypeConverter`, or component-editor row. Those identities remain
missing until their exact generated facade behavior is projected and measured.

```sh
python3 tools/winforms_catalogue/generate_catalogue.py \
  --forms-api /path/to/LibreWinForms/src/System.Windows.Forms/PublicAPI.Shipped.txt \
  --drawing-xml /path/to/System.Drawing.Common.xml \
  --drawing-xml /path/to/System.Drawing.Primitives.xml \
  --output planning/generated/WINFORMS_API_CATALOGUE.tsv \
  --summary planning/WINFORMS_API_CATALOGUE.md
```

The full TSV is intentionally generated and checked in: it is the inspectable
member-level backlog. The summary contains counts and the policy boundary.
