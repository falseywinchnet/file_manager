# Windows Forms control and component inventory

Status: **evidence collection for the GUI.Forms bridge; not an implementation
commitment**.

Date: 2026-08-03.

## 1. Scope and terminology

**GIVEN:** GUI.Forms is to preserve reasonable Windows Forms calls and expected
outcomes while using a new native implementation. This document inventories the
surface that such a compatibility project must classify before it chooses a
supported subset.

“Every Windows Forms control ever made” cannot literally include every custom,
commercial, ActiveX-generated, or application-private control. That universe is
unbounded. The bounded inventory here is:

1. every public `Control` descendant found in Microsoft's current
   `System.Windows.Forms` source, including abstract bases and public
   infrastructure types;
2. every inbox Forms `Component` normally visible either in the Visual Studio
   toolbox, the component tray, or a composite-control designer;
3. the default non-Forms components included by the classic Windows Forms
   toolbox;
4. Microsoft satellite controls that materially expanded the historical Forms
   toolbox; and
5. legacy/obsolete families retained for compatibility.

These terms are not interchangeable:

| Kind | Test | Examples |
|---|---|---|
| Control | derives from `System.Windows.Forms.Control`; has bounds, parenting, input, paint and focus behavior | `Button`, `TreeView`, `Panel` |
| Contained control | is a `Control`, but is normally created/owned by another control rather than dragged independently | `TabPage`, `SplitterPanel`, `MdiClient` |
| Hosted visual component | does not derive from `Control`, but has visual/input behavior inside a composite owner | `ToolStripItem`, `DataGridViewColumn`, `TreeNode` |
| Nonvisual component | participates in `IComponent`/`IContainer`, lifetime and designer serialization; normally appears in the component tray | `Timer`, `ImageList`, `BindingSource` |
| Dialog component | is a component whose primary contract is an owned modal interaction and `DialogResult` | `OpenFileDialog`, `ColorDialog` |
| Service/support type | is neither a control nor a component, but is part of the programming contract | `MessageBox`, `TaskDialog`, `Screen`, `ApplicationContext` |

## 2. Candidate for the remembered exhaustive GitHub listing

### 2.1 Strongest match

**OBSERVED:** [`mirumirumi/mirumi-tech-content`,
`posts/dotnet-control-list.md`](https://github.com/mirumirumi/mirumi-tech-content/blob/main/posts/dotnet-control-list.md)
is titled “all controls used with C# or VB” and contains 66 alphabetized entries,
with original screenshots for visual controls and a common placeholder image for
nonvisual component-tray entries. It states that its inventory was taken from
Visual Studio Community 2019 using .NET Framework 4.7.2 on Windows 10. It includes
the unusual toolbox components (`DirectoryEntry`, `EventLog`, `MessageQueue`,
`PerformanceCounter`, `Process`, `SerialPort`, and `ServiceController`) as well as
the ordinary controls.

This is the strongest match to the architect's memory because it is a GitHub
page that explicitly presents itself as an exhaustive, screenshot-backed Forms
toolbox list. Its exact 66 headings are:

```text
BackgroundWorker, BindingNavigator, BindingSource, Button, CheckBox,
CheckedListBox, ColorDialog, ComboBox, ContextMenuStrip, DataGridView, DataSet,
DateTimePicker, DirectoryEntry, DirectorySearcher, DomainUpDown, ErrorProvider,
EventLog, FileSystemWatcher, FlowLayoutPanel, FolderBrowserDialog, FontDialog,
GroupBox, HelpProvider, HScrollBar, ImageList, Label, LinkLabel, ListBox,
ListView, MaskedTextBox, MenuStrip, MessageQueue, MonthCalendar, NotifyIcon,
NumericUpDown, OpenFileDialog, PageSetupDialog, Panel, PerformanceCounter,
PictureBox, PrintDialog, PrintDocument, PrintPreviewControl,
PrintPreviewDialog, Process, ProgressBar, PropertyGrid, RadioButton,
RichTextBox, SaveFileDialog, SerialPort, ServiceController, SplitContainer,
Splitter, StatusStrip, TabControl, TableLayoutPanel, TextBox, Timer, ToolStrip,
ToolStripContainer, ToolTip, TrackBar, TreeView, VScrollBar, WebBrowser
```

**LIMIT:** it is an inventory of one Visual Studio toolbox configuration, not an
API oracle. It omits base classes, hosted `ToolStripItem` and grid-column types,
legacy menus/toolbars/status bars, `Chart`, WPF interop, ActiveX hosts, and types
not placed on that toolbox by default.

### 2.2 Other broad catalogues found

- **OBSERVED:** [`ga-lucas/CanvasForms`,
  `COMPATIBILITY_REVIEW.md`](https://github.com/ga-lucas/CanvasForms/blob/main/COMPATIBILITY_REVIEW.md)
  is an independent reimplementation ledger organized control by control. It
  records implemented, partial, and missing behaviors for most standard and
  legacy controls. It is unusually useful as a behavioral checklist, but it was
  created in 2026 and is not evidence of the historical toolbox the architect
  remembers.
- **OBSERVED:** the official [Avalonia Windows Forms migration
  page](https://docs.avaloniaui.net/docs/migration/winforms/) maps only a small
  number of concepts and explicitly says that migration is not a one-to-one
  mapping. It is not an exhaustive WinForms specification.
- **OBSERVED:** Avalonia's [controls reference](https://docs.avaloniaui.net/controls)
  inventories Avalonia controls, not the WinForms API surface.
- **CANDIDATE, provenance not authoritative:**
  [`wieslawsoltes/development-plugin-for-avalonia`, WinForms-to-Avalonia
  conversion index](https://github.com/wieslawsoltes/development-plugin-for-avalonia/blob/main/references/63-winforms-to-avalonia-modern-ui-conversion-index.md)
  claims a generated exhaustive reference lane. It is a third-party development
  plugin, not Avalonia's framework specification. It can supply test ideas but
  cannot define WinForms behavior.

## 3. Authoritative cross-checks

- **OBSERVED:** Microsoft's [Controls to Use on Windows
  Forms](https://learn.microsoft.com/en-us/dotnet/desktop/winforms/controls/controls-to-use-on-windows-forms)
  is the official alphabetic control/component catalogue.
- **OBSERVED:** Microsoft's [Windows Forms controls by
  function](https://github.com/dotnet/docs-desktop/blob/main/dotnet-desktop-guide/winforms/controls/windows-forms-controls-by-function.md)
  supplies the functional grouping used by the classic documentation.
- **OBSERVED:** Microsoft's [Windows Forms/WPF equivalence
  table](https://learn.microsoft.com/en-us/dotnet/desktop/wpf/advanced/windows-forms-controls-and-equivalent-wpf-controls)
  is a second independent Microsoft list, useful for finding controls with no
  obvious compositional equivalent such as `PropertyGrid` and
  `BindingNavigator`.
- **OBSERVED:** [`dotnet/winforms`](https://github.com/dotnet/winforms) at
  `480ddfbd62ede94f539cc7f1750d5bd1c6bd08d3` was mechanically scanned. The
  public source graph contains 69 `Control` descendants: 62 concrete and seven
  abstract. This count includes palette controls, contained controls, legacy
  types, editor controls, and public infrastructure; it is deliberately broader
  than a Visual Studio toolbox screenshot.

## 4. Complete `System.Windows.Forms` control graph

### 4.1 Abstract control primitives

**OBSERVED:** these seven public abstract classes are `Control` descendants in
the pinned source. They are extension contracts, not toolbox widgets.

| Type | Base | Contract family |
|---|---|---|
| `AxHost` | `Control` | generated ActiveX wrapper host, COM persistence, verbs and activation |
| `ButtonBase` | `Control` | click/keyboard activation, image/text layout, flat styles |
| `ComponentEditorPage` | `Panel` | design-time component editor page |
| `ListControl` | `Control` | `DataSource`, display/value members, selection currency |
| `ScrollBar` | `Control` | range/value/small-large change and scroll event protocol |
| `TextBoxBase` | `Control` | selection, clipboard, undo, multiline and native edit protocol |
| `UpDownBase` | `ContainerControl` | editable value area plus spinner buttons |

### 4.2 Concrete authoring and window primitives

| Type | Normal role | Essential compatibility contract |
|---|---|---|
| `Control` | custom-control base | parent/child tree; bounds/client area; visibility/enabled inheritance; focus/tab order; input, drag/drop, painting and invalidation; UI-thread affinity; handles; accessibility; data bindings |
| `ScrollableControl` | scrollable custom-container base | display rectangle, auto-scroll extents/position, child scrolling |
| `ContainerControl` | focus/validation container base | active control, validation traversal, auto-scaling and mnemonic/dialog-key routing |
| `UserControl` | reusable composite | designer-authored child composition and load/scale lifecycle |
| `Form` | top-level/owned/modal/MDI window | owner and modal loop, accept/cancel buttons, close reasons, chrome/window state, activation, menu binding and DPI behavior |

### 4.3 Standard visual and container controls

Every type in this table is a concrete `Control`. “Contained” means that the
designer normally creates it through its owner rather than as an independent
top-level toolbox item.

| Family | Concrete types | Behavioral center |
|---|---|---|
| Commands/choices | `Button`, `CheckBox`, `RadioButton` | keyboard and mnemonic activation; click/check event order; default/cancel semantics; radio grouping; three-state check behavior |
| Static/action text | `Label`, `LinkLabel` | text measurement, mnemonic target, autosize/ellipsis; link spans, visited/enabled state and hit testing |
| Text editing | `TextBox`, `MaskedTextBox`, `RichTextBox` | caret/selection, clipboard and undo; single/multiline; IME; mask provider and validation; RTF formatting, links and protected ranges |
| List selection | `ListBox`, `CheckedListBox`, `ComboBox` | item collection, selection modes, data binding, owner draw, type/search and autocomplete; dropdown commit/cancel behavior |
| Spin/value input | `NumericUpDown`, `DomainUpDown` | range/increment/formatting or ordered string domain; edit-to-value validation; wrapping and acceleration |
| Date/time | `DateTimePicker`, `MonthCalendar` | culture-aware calendar/date format, nullable checkbox behavior, selection ranges and bold dates |
| Scroll/range/progress | `HScrollBar`, `VScrollBar`, `TrackBar`, `ProgressBar` | effective range rules, orientation, keyboard/wheel input, ticks and determinate/marquee progress |
| Media | `PictureBox` | image ownership/loading and normal/stretch/autosize/center/zoom layout |
| Collections | `TreeView`, `ListView` | node/item identity, selection, keyboard navigation, labels, images, checks, owner draw; ListView modes/groups/columns and virtual mode |
| Basic containers | `Panel`, `GroupBox` | child clipping, borders/caption and radio-button grouping |
| Layout containers | `FlowLayoutPanel`, `TableLayoutPanel` | flow direction/wrap/break; row/column size modes, spans and cell lookup |
| Split/tab containers | `SplitContainer`, contained `SplitterPanel`, `TabControl`, contained `TabPage` | splitter constraints/collapse/fixed panels; selected page, tab ordering, page collection and keyboard navigation |
| Data/inspection | `DataGridView`, `PropertyGrid` | grid cell/row/column model, edit controls, virtual mode, sorting and data errors; reflection/property descriptors, editors, categories and value commit |
| Command surfaces | `ToolStrip`, `MenuStrip`, `ContextMenuStrip`, `StatusStrip`, `BindingNavigator` | item ownership, overflow, dropdowns, shortcuts/mnemonics, merge, renderer, strip layout; binding currency navigation |
| Strip layout | `ToolStripContainer`, `ToolStripPanel`, contained `ToolStripContentPanel` | docking/rafting of strips and central content ownership |
| Printing UI | `PrintPreviewControl`, `PrintPreviewDialog` | page rendering, zoom/navigation, print-document binding and modal preview shell |

### 4.4 Public concrete controls that are infrastructure or legacy

These complete the 62-concrete-type source sweep. Their public status does not
mean each deserves a GUI.Forms palette entry.

| Role | Types | Notes |
|---|---|---|
| Composite infrastructure | `MdiClient`, `ToolStripDropDown`, `ToolStripDropDownMenu`, `ToolStripOverflow` | created/managed by MDI or ToolStrip owners |
| Grid editing infrastructure | `DataGridTextBox`, `DataGridViewComboBoxEditingControl`, `DataGridViewTextBoxEditingControl` | hosted editors with grid notification contracts |
| Designer/error infrastructure | `ComponentEditorForm`, `ThreadExceptionDialog` | framework/design-time shells, not application palette widgets |
| Legacy replacements retained | `DataGrid`, `Splitter`, `StatusBar`, `ToolBar` | superseded respectively by `DataGridView`, `SplitContainer`, `StatusStrip`, `ToolStrip` |
| Legacy browser host | `WebBrowser`, `WebBrowserBase` | Internet Explorer/ActiveX navigation and DOM interop; `WebBrowserBase` is infrastructure |

**COUNT CHECK:** 5 concrete authoring/window primitives + 42 concrete standard,
contained, and printing types + 15 infrastructure/legacy types = 62 concrete
types. Adding the seven abstract bases produces the pinned source total of 69.

## 5. Hosted visual components and item models

These types are easy to miss in a “controls” list because many do not inherit
`Control`. A source-compatible API still encounters them directly.

### 5.1 ToolStrip family

| Kind | Types | Contract |
|---|---|---|
| Abstract bases | `ToolStripItem`, `ToolStripDropDownItem`, `ToolStripControlHost` | owner/placement, bounds, available/visible/enabled state, input, image/text layout, accessibility and dropdown or hosted-control lifecycle |
| Commands/content | `ToolStripButton`, `ToolStripLabel`, `ToolStripStatusLabel`, `ToolStripSeparator` | click/check behavior, labels/links, spring status sizing, separator layout |
| Menus/dropdowns | `ToolStripMenuItem`, `ToolStripDropDownButton`, `ToolStripSplitButton` | nested item trees, shortcuts, checked state, primary-vs-dropdown activation |
| Hosted controls | `ToolStripComboBox`, `ToolStripTextBox`, `ToolStripProgressBar` | bridge a real child control into item measurement/focus/overflow rules |
| Infrastructure | `ToolStripOverflowButton`, `ToolStripPanelRow` | owner-created overflow and rafting layout objects |

### 5.2 Tree/ListView models

| Owner | Public models that must be inventoried | Contract |
|---|---|---|
| `TreeView` | `TreeNode`, `TreeNodeCollection` | parent/tree ownership, expand/check/select, state images, labels, cloning/serialization |
| `ListView` | `ListViewItem`, `ListViewItem.ListViewSubItem`, `ColumnHeader`, `ListViewGroup`, their collections, `ListViewInsertionMark` | view-independent item identity, columns/subitems/groups/images, label edit, virtual retrieval and insertion feedback |

### 5.3 DataGridView model

`DataGridViewColumn` implements `IComponent` directly. Columns are designer
components even though they are not `Control` objects.

| Layer | Public concrete families |
|---|---|
| Columns | `DataGridViewTextBoxColumn`, `DataGridViewCheckBoxColumn`, `DataGridViewComboBoxColumn`, `DataGridViewButtonColumn`, `DataGridViewImageColumn`, `DataGridViewLinkColumn` |
| Cells | corresponding `TextBoxCell`, `CheckBoxCell`, `ComboBoxCell`, `ButtonCell`, `ImageCell`, `LinkCell`; `DataGridViewHeaderCell`, `ColumnHeaderCell`, `RowHeaderCell`, `TopLeftHeaderCell` |
| Rows/bands | `DataGridViewRow`, `DataGridViewBand`, row/column/cell collections and selected-object collections |
| Editing | `IDataGridViewEditingControl` plus the concrete text/combo editing controls listed in section 4.4 |
| Style/state | `DataGridViewCellStyle`, `DataGridViewAdvancedBorderStyle`, sort/selection/auto-size enums and event-argument families |

### 5.4 Legacy hosted models

- `DataGridTableStyle`, abstract `DataGridColumnStyle`,
  `DataGridTextBoxColumn`, and `DataGridBoolColumn` configure the legacy
  `DataGrid`.
- `ToolBarButton` is the hosted command object for legacy `ToolBar`.
- `StatusBarPanel` is the hosted panel for legacy `StatusBar`.
- abstract `Menu` with `MainMenu`, `ContextMenu`, and `MenuItem` is the
  component-based pre-ToolStrip menu model.

## 6. Inbox nonvisual Forms components and dialogs

| Family | Concrete types | Central behavior |
|---|---|---|
| Binding | `BindingSource` | currency, add/remove/edit, sorting/filtering where supported, list-change propagation |
| Extender providers | `ErrorProvider`, `HelpProvider`, `ToolTip` | attach per-control properties through `IExtenderProvider`; provider-owned rendering/lifetime |
| Assets and shell | `ImageList`, `NotifyIcon` | keyed/indexed scaled image store; tray icon, context menu, balloon and mouse events |
| Scheduling | `Timer` | UI-message-loop ticks; interval/enabled and UI-thread event delivery |
| File dialogs | `OpenFileDialog`, `SaveFileDialog`, `FolderBrowserDialog` | owned modal selection; filters, initial/selected path, validation and result state |
| Appearance dialogs | `ColorDialog`, `FontDialog` | owned modal selection with cancel preserving caller state |
| Printing dialogs | `PageSetupDialog`, `PrintDialog` | bind `PageSettings`, `PrinterSettings` or `PrintDocument`; owned modal result |

Abstract component bases include `BindableComponent`, `CommonDialog`, and
`FileDialog`. They establish behavior but are not palette entries.

## 7. Classic Visual Studio component-tray entries outside Forms

**OBSERVED:** the strongest-match 66-entry GitHub catalogue includes these
default toolbox objects. They are not `System.Windows.Forms` controls, and most
belong to other framework assemblies. GUI.Forms should not accidentally model
them as widgets.

| Type | Framework namespace/family | Why it appeared in Forms designer |
|---|---|---|
| `BackgroundWorker` | `System.ComponentModel` | worker/progress/completion event component |
| `DataSet` | `System.Data` | designable in-memory relational data component |
| `DirectoryEntry`, `DirectorySearcher` | `System.DirectoryServices` | Active Directory binding/query components |
| `EventLog`, `PerformanceCounter`, `Process` | `System.Diagnostics` | OS diagnostics/process components |
| `FileSystemWatcher` | `System.IO` | filesystem event component |
| `MessageQueue` | `System.Messaging` | MSMQ component |
| `PrintDocument` | `System.Drawing.Printing` | application-supplied page rendering and print lifecycle |
| `SerialPort` | `System.IO.Ports` | serial I/O event component |
| `ServiceController` | `System.ServiceProcess` | Windows service control component |

Microsoft's official control catalogue also documents `SoundPlayer`
(`System.Media`) as a Forms-adjacent sound service, though it is not one of the
66 headings in that captured toolbox inventory.

## 8. Microsoft satellite and historical controls

These are not part of the core `System.Windows.Forms.dll` graph, but omitting
them would make “Microsoft-era WinForms” narrower than the real designer
ecosystem.

| Surface | Status and source | Compatibility significance |
|---|---|---|
| `Chart` | **OBSERVED:** `System.Windows.Forms.DataVisualization.Charting.Chart` derives from `Control` in `System.Windows.Forms.DataVisualization.dll`; see [Microsoft API reference](https://learn.microsoft.com/en-us/dotnet/api/system.windows.forms.datavisualization.charting.chart) | root chart control with series, chart areas, axes, legends and annotation object model |
| `ElementHost` | **OBSERVED:** WindowsFormsIntegration control for hosting a WPF element; see [Microsoft API reference](https://learn.microsoft.com/en-us/dotnet/api/system.windows.forms.integration.elementhost) | an interop host, not a native Forms rendering primitive |
| `DataRepeater` | **OBSERVED:** Visual Basic Power Packs `ContainerControl`; see [archived Microsoft reference](https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2013/bb894815%28v%3Dvs.120%29) | repeated designer template with binding and virtual item-value events |
| `LineShape`, `OvalShape`, `RectangleShape`, `ShapeContainer` | **OBSERVED:** Visual Basic Power Packs designer shapes; see [archived `LineShape` reference](https://learn.microsoft.com/en-us/previous-versions/bb907131%28v%3Dvs.140%29) | design-time drawing objects, not ordinary child HWND controls |
| `PrintForm` | **OBSERVED:** Visual Basic Power Packs nonvisual component; see [archived Microsoft reference](https://learn.microsoft.com/en-us/previous-versions/bb882742%28v%3Dvs.140%29) | prints a rendered image of a Form |
| `InkEdit`, `InkPicture` | **OBSERVED:** Tablet PC managed/ActiveX controls; see [Microsoft ink controls](https://learn.microsoft.com/en-us/windows/win32/tablet/ink-controls) | handwriting input/recognition and ink annotation; platform-specific |
| `ReportViewer` | **OBSERVED:** separately distributed Microsoft WinForms control; see [Microsoft SSRS guide](https://learn.microsoft.com/en-us/sql/reporting-services/application-integration/using-the-winforms-reportviewer-control) | report rendering, pagination, parameters and print/export shell |
| `WebView`, `WebView2` | **OBSERVED:** separately shipped Windows Community Toolkit/Edge controls; see [`WebView2` API](https://learn.microsoft.com/en-us/dotnet/api/microsoft.web.webview2.winforms.webview2) | browser-runtime hosts, not core Forms behavior; `WebView` is obsolete |

`AxHost` admits generated wrappers for arbitrary installed ActiveX controls.
That makes the ActiveX child-control set intentionally unbounded. It should be a
host-compatibility capability, not an enumerated GUI.Forms widget list.

### 8.1 Control-adjacent services, accessibility and interoperation

These public families complete the programming model around the designer
objects. They are not additional controls.

| Family | Principal public types | Contract represented |
|---|---|---|
| Application/lifetime | `Application`, `ApplicationContext`, `FormCollection`, `WindowsFormsSynchronizationContext`, `IMessageFilter` | message loop, application exit, thread contexts, open-form tracking and message filtering |
| Window/native interop | `IWin32Window`, `NativeWindow`, `CreateParams`, `IWindowTarget`, `AxHost` | native handle ownership, window procedure interception, class/style creation and ActiveX hosting |
| Display/shell | `Screen`, `SystemInformation`, `PowerStatus`, `Cursor`, `Cursors`, `Clipboard`, `SendKeys` | monitor/work-area discovery, system metrics, cursor/clipboard services and synthesized input |
| Rendering/theme | `ControlPaint`, `TextRenderer`, `VisualStyleState`, `VisualStyleRenderer`, `VisualStyleElement`, `ToolStripRenderer`, `ToolStripProfessionalRenderer`, `ToolStripSystemRenderer`, `ProfessionalColorTable` | text metrics, classic/system painting, theme-part rendering and ToolStrip renderer substitution |
| Accessibility | `AccessibleObject`, `Control.ControlAccessibleObject`, composite controls' public accessible-object subclasses, `QueryAccessibilityHelpEventArgs`, per-control `CreateAccessibilityInstance` contracts | accessible tree identity, role/name/value/state, hit testing, navigation, selection/focus and change notification |
| Input/language | `InputLanguage`, `InputLanguageCollection`, `Keys`, `KeyEventArgs`, `KeyPressEventArgs`, `MouseEventArgs`, `PreviewKeyDownEventArgs` | keyboard layout/IME context, key preprocessing, pointer state and routed event payloads |
| Binding | `Binding`, `BindingContext`, `BindingManagerBase`, `CurrencyManager`, `PropertyManager`, `ControlBindingsCollection` | control-property binding, currency, format/parse, update modes and error/completion notification |
| Layout | `LayoutEngine`, `LayoutSettings`, `FlowLayoutSettings`, `TableLayoutSettings`, `ColumnStyle`, `RowStyle` | owner-selected layout strategy and serializable per-child/per-row/per-column constraints |
| Task dialogs | `TaskDialog`, `TaskDialogPage`, `TaskDialogButton`, `TaskDialogCommandLinkButton`, `TaskDialogRadioButton`, `TaskDialogProgressBar`, `TaskDialogVerificationCheckBox`, `TaskDialogExpander`, `TaskDialogFootnote`, `TaskDialogIcon` | modal task-dialog page model, navigation and hosted task-dialog elements; none derives from Forms `Control` |
| Printing pipeline | `PrintControllerWithStatusDialog`, `PrintPreviewControl`, `PrintPreviewDialog` plus `System.Drawing.Printing.PrintDocument`, `PrintController`, `PrintPageEventArgs`, `PageSettings`, `PrinterSettings` | application-rendered pages, printer setup, controller lifecycle, status shell and preview |
| Immediate dialog services | `MessageBox`, `TaskDialog` | owned modal service calls; no reusable child-control instance |

Accessibility is a cross-cutting contract, not one optional “accessibility
control.” Every visual and hosted item needs a stable accessible subtree and
state/event mapping. Similarly, `AxHost`, `ElementHost`, `WebBrowser`, and
`WebView2` are host bridges whose behavior depends on external runtimes; API
shape alone cannot make their contents native to GUI.Forms.

## 9. Behavioral contracts that cut across controls

Class names alone are insufficient. A compatibility test matrix needs these API
families because applications depend on their interactions and event order.

| Contract | Required observations/tests |
|---|---|
| Component lifetime | `IContainer`/`Site`, deterministic `Dispose`, owned native resources, component-tray serialization |
| Control tree | parent transfer, `Controls` ordering, z-order, name lookup, create/destroy propagation |
| Geometry/layout | `Bounds`, `ClientSize`, preferred size, autosize, margin/padding, `Dock`, `Anchor`, minimum/maximum size, DPI scaling |
| Retained state and invalidation | property mutation, typed layout/paint dirtiness, invalid rectangle propagation, `Refresh`/`Update`, double buffering |
| Focus/keyboard | tab order, validation before focus transition, mnemonics, dialog keys, command keys, key preview, IME composition |
| Pointer/drag | capture, click/double-click order, wheel, drag threshold, drag enter/over/leave/drop effects |
| Text/culture | font metrics, compatible text rendering, RTL, localization, culture-aware parse/format, surrogate/combining text |
| Data binding | property descriptors, simple/complex binding, currency, format/parse, list-change notification, validation/data errors |
| Accessibility | accessible object tree, role/name/value/state, focus and selection events, patterns for composite virtual children |
| Threading | UI-thread ownership, `InvokeRequired`, synchronous/asynchronous invoke, timer/event dispatch and shutdown behavior |
| Owner draw | measure-before-draw rules, state flags, clipping, renderer/theme interaction, high-DPI images |
| Designer serialization | default values, `ShouldSerialize`/`Reset`, content properties and collections, `BeginInit`/`EndInit`, extender properties |
| Modal ownership | owner disabling/reactivation, nested modal loops, `DialogResult`, cancellation and exception/shutdown behavior |

## 10. Proposed exhaustive-verification method

**CANDIDATE procedure; not yet run on a Windows Visual Studio host:**

1. Pin reference/runtime assemblies for .NET Framework 1.0, 1.1, 2.0, 3.5,
   4.0, 4.5, 4.8.1 and supported modern Windows Desktop releases. Record SHA-256,
   assembly version and source commit where available.
2. For each assembly, use metadata inspection without type initialization to
   enumerate public types assignable to `Control`, `Component`, or `IComponent`.
   Record abstract/concrete, base type, assembly, `[Obsolete]`,
   `[ToolboxItem(false)]`, `Designer`, `DefaultEvent`, `DefaultProperty`, and
   public constructor shape.
3. On a disposable Windows/Visual Studio fixture, separately query the designer
   toolbox service for each target framework. This distinguishes “public type”
   from “default palette item” and captures component-tray categories.
4. Expand composite designers and record the types they can create: ToolStrip
   items, DataGridView columns/cells, ListView columns/groups/items, TreeNodes,
   TabPages, DataGrid styles, legacy menu items, status panels and toolbar
   buttons.
5. Diff each version against the previous version. Preserve additions,
   removals, obsolete transitions, replacement guidance, and public types hidden
   from the toolbox.
6. Add separately pinned Microsoft satellite assemblies: DataVisualization,
   WindowsFormsIntegration, Visual Basic Power Packs, Tablet/Ink, ReportViewer,
   Windows Community Toolkit WebView and WebView2. Never merge third-party
   packages into the inbox count.
7. Compare the result against all four independent catalogues in sections 2–3.
   Any type found by only one source becomes an explicit discrepancy to resolve,
   not an automatic addition or deletion.
8. Generate a machine-readable manifest with one record per type and per
   version. From that manifest, generate this human taxonomy and an API-contract
   test skeleton. CI should fail when a pinned source changes without a reviewed
   manifest diff.

Suggested manifest fields:

```text
full_name, assembly, version_first_seen, version_last_seen, base_type,
kind(control|contained|hosted|component|dialog|service), abstract,
toolbox_visible, toolbox_category, obsolete, owner_type, replacement,
behavior_contract_ids, source_locator, verification_status
```

## 11. Result for GUI.Forms pre-planning

**OBSERVED:** the remembered 66-entry GitHub page is a useful visual/designer
index, but the real compatibility surface is larger and structurally richer.
The pinned current source alone exposes 69 control-derived types, before hosted
items, dialogs, component-tray objects, satellite controls, and historical
versions are counted.

**CANDIDATE inventory rule:** GUI.Forms should maintain separate manifests for
controls, contained controls, hosted visual components, nonvisual components,
dialogs, and services. Support can then be declared per type and per behavioral
contract without pretending that namespace coverage, visual resemblance, or a
successful constructor call establishes compatibility.
