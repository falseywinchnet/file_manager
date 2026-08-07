#!/usr/bin/env python3
"""Generate the admitted GUI.Forms/GUI.Drawing compatibility oversight ledger.

This intentionally consumes reference API lists rather than implementation
source.  The result is a planning inventory: a row marked partial is not a
claim that its WinForms behavior is complete.
"""

from __future__ import annotations

import argparse
import csv
import json
from collections import Counter
from pathlib import Path
import re
import xml.etree.ElementTree as ET


UPSTREAM_COMMIT = "1457ed5beef24a7d7e3688a4725c7487da09791e"

FORM_EXCLUSIONS = {
    "activex": ("System.Windows.Forms.AxHost", "System.Windows.Forms.ActiveX"),
    "browser": ("System.Windows.Forms.WebBrowser", "System.Windows.Forms.Html"),
    "printing": (
        "System.Windows.Forms.PrintControllerWithStatusDialog",
        "System.Windows.Forms.PrintDialog",
        "System.Windows.Forms.PrintPreviewControl",
        "System.Windows.Forms.PrintPreviewDialog",
        "System.Windows.Forms.PageSetupDialog",
    ),
    "mdi": ("System.Windows.Forms.Mdi", "System.Windows.Forms.MDI"),
    "design_time": ("System.Windows.Forms.Design",),
    "classic_datagrid": (
        "System.Windows.Forms.DataGrid",
        "System.Windows.Forms.DataGridBoolColumn",
        "System.Windows.Forms.DataGridCell",
        "System.Windows.Forms.DataGridColumnStyle",
        "System.Windows.Forms.DataGridLineStyle",
        "System.Windows.Forms.DataGridParentRowsLabelStyle",
        "System.Windows.Forms.DataGridPreferredColumnWidthTypeConverter",
        "System.Windows.Forms.DataGridTableStyle",
        "System.Windows.Forms.DataGridTextBox",
        "System.Windows.Forms.DataGridTextBoxColumn",
    ),
    "obsolete_menu_toolbar": (
        "System.Windows.Forms.ContextMenu",
        "System.Windows.Forms.MainMenu",
        "System.Windows.Forms.MenuItem",
        "System.Windows.Forms.StatusBar",
        "System.Windows.Forms.ToolBar",
    ),
}

FORM_PARTIAL_NATIVE = (
    "System.Windows.Forms.Application",
    "System.Windows.Forms.ApplicationContext",
    "System.Windows.Forms.Form",
    "System.Windows.Forms.Control",
    "System.Windows.Forms.ContainerControl",
    "System.Windows.Forms.UserControl",
    "System.Windows.Forms.Panel",
    "System.Windows.Forms.GroupBox",
    "System.Windows.Forms.FlowLayoutPanel",
    "System.Windows.Forms.TableLayoutPanel",
    "System.Windows.Forms.Label",
    "System.Windows.Forms.Button",
    "System.Windows.Forms.ButtonBase",
    "System.Windows.Forms.CheckBox",
    "System.Windows.Forms.RadioButton",
    "System.Windows.Forms.LinkLabel",
    "System.Windows.Forms.PictureBox",
    "System.Windows.Forms.ImageList",
    "System.Windows.Forms.TextBox",
    "System.Windows.Forms.TextBoxBase",
    "System.Windows.Forms.ComboBox",
    "System.Windows.Forms.ListBox",
    "System.Windows.Forms.CheckedListBox",
    "System.Windows.Forms.TreeView",
    "System.Windows.Forms.NumericUpDown",
    "System.Windows.Forms.TrackBar",
    "System.Windows.Forms.ProgressBar",
    "System.Windows.Forms.ScrollBar",
    "System.Windows.Forms.HScrollBar",
    "System.Windows.Forms.VScrollBar",
    "System.Windows.Forms.DateTimePicker",
    "System.Windows.Forms.SplitContainer",
    "System.Windows.Forms.SplitterPanel",
    "System.Windows.Forms.Splitter",
    "System.Windows.Forms.TabControl",
    "System.Windows.Forms.TabPage",
    "System.Windows.Forms.ToolTip",
    "System.Windows.Forms.ErrorProvider",
    "System.Windows.Forms.HelpProvider",
    "System.Windows.Forms.HelpEventArgs",
    "System.Windows.Forms.HelpEventHandler",
    "System.Windows.Forms.HelpNavigator",
    "System.Windows.Forms.ErrorBlinkStyle",
    "System.Windows.Forms.ErrorIconAlignment",
    "System.Windows.Forms.Binding",
    "System.Windows.Forms.BindingSource",
    "System.Windows.Forms.BindingManagerBase",
    "System.Windows.Forms.CurrencyManager",
    "System.Windows.Forms.BindingContext",
    "System.Windows.Forms.ControlBindingsCollection",
    "System.Windows.Forms.BindingCompleteEventArgs",
    "System.Windows.Forms.BindingCompleteContext",
    "System.Windows.Forms.BindingCompleteState",
    "System.Windows.Forms.DataSourceUpdateMode",
    "System.Windows.Forms.ControlUpdateMode",
    "System.Windows.Forms.AutoValidate",
    "System.Windows.Forms.ValidationConstraints",
    "System.Windows.Forms.DialogResult",
    "System.Windows.Forms.IButtonControl",
    "System.Windows.Forms.Timer",
    "System.Windows.Forms.ContextMenuStrip",
    "System.Windows.Forms.MenuStrip",
    "System.Windows.Forms.ToolStripMenuItem",
)

DRAWING_PARTIAL_NATIVE = (
    "System.Drawing.Bitmap",
    "System.Drawing.Brush",
    "System.Drawing.Color",
    "System.Drawing.Font",
    "System.Drawing.Graphics",
    "System.Drawing.Icon",
    "System.Drawing.Image",
    "System.Drawing.Pen",
    "System.Drawing.Point",
    "System.Drawing.Rectangle",
    "System.Drawing.Region",
    "System.Drawing.Size",
    "System.Drawing.SolidBrush",
    "System.Drawing.StringFormat",
    "System.Drawing.TextureBrush",
    "System.Drawing.Drawing2D.ColorBlend",
    "System.Drawing.Drawing2D.GraphicsPath",
    "System.Drawing.Drawing2D.HatchBrush",
    "System.Drawing.Drawing2D.LinearGradientBrush",
    "System.Drawing.Drawing2D.Matrix",
    "System.Drawing.Drawing2D.PathGradientBrush",
    "System.Drawing.Imaging.BitmapData",
    "System.Drawing.Imaging.ColorMap",
    "System.Drawing.Imaging.ColorMatrix",
    "System.Drawing.Imaging.ImageAttributes",
)

DRAWING_EXCLUSIONS = {
    "printing": ("System.Drawing.Printing",),
    "design_time": ("System.Drawing.Design",),
    "metafile": (
        "System.Drawing.Imaging.Metafile",
        "System.Drawing.Imaging.Emf",
        "System.Drawing.Imaging.Wmf",
    ),
}


def clean_api_line(line: str) -> tuple[str, str]:
    marker = ""
    line = line.strip()
    while line.startswith("["):
        end = line.find("]")
        if end < 0:
            break
        marker += line[: end + 1]
        line = line[end + 1 :]
    if line.startswith("~"):
        marker += "~"
        line = line[1:]
    for prefix in ("abstract ", "override ", "virtual ", "static readonly ", "static "):
        if line.startswith(prefix):
            marker += prefix.strip() + ";"
            line = line[len(prefix) :]
            break
    return line.strip(), marker


def owner_for(line: str, types: list[str]) -> str:
    candidates = [value for value in types if line == value or line.startswith(value + ".")]
    return max(candidates, key=len) if candidates else line.split("(", 1)[0].rsplit(".", 1)[0]


def form_family(owner: str) -> str:
    names = owner.split(".")
    leaf = names[-1]
    if any(word in owner for word in ("Accessible", "Automation")):
        return "accessibility_semantics"
    if any(word in leaf for word in ("Layout", "Panel", "Splitter", "Scroll")):
        return "layout_scrolling"
    if any(word in leaf for word in ("Text", "Mask", "RichText")):
        return "text_editing"
    if any(word in leaf for word in ("List", "Tree", "Grid", "Binding", "Currency")):
        return "collections_binding_virtualization"
    if any(word in leaf for word in ("Menu", "Strip", "ToolTip", "Help", "ErrorProvider")):
        return "commands_popups_guidance"
    if any(word in leaf for word in ("Dialog", "ColorDialog", "FileDialog", "FolderBrowser")):
        return "dialogs_host_services"
    if any(word in leaf for word in ("Mouse", "Key", "Drag", "Clipboard", "Cursor", "DataObject")):
        return "input_transfer"
    if any(word in leaf for word in ("Renderer", "VisualStyle", "Paint", "Image", "Icon")):
        return "style_resources_owner_draw"
    if leaf in {"Application", "ApplicationContext", "Form", "Control", "ContainerControl", "UserControl"}:
        return "application_window_control_kernel"
    return "controls_components_misc"


def drawing_family(owner: str) -> str:
    if owner.startswith("System.Drawing.Drawing2D"):
        if any(word in owner for word in ("Brush", "Blend")):
            return "paint_gradients_textures"
        return "paths_transforms_regions_strokes"
    if owner.startswith("System.Drawing.Imaging"):
        return "images_pixels_color_adjustment"
    if owner.startswith("System.Drawing.Text"):
        return "fonts_text_glyphs"
    if any(word in owner for word in ("Font", "StringFormat", "CharacterRange")):
        return "fonts_text_glyphs"
    if any(word in owner for word in ("Image", "Bitmap", "Icon")):
        return "images_pixels_color_adjustment"
    if any(word in owner for word in ("Brush", "Pen")):
        return "paint_gradients_textures"
    if owner.endswith(("Point", "PointF", "Size", "SizeF", "Rectangle", "RectangleF", "Color")):
        return "values_geometry_color"
    if owner.endswith("Graphics"):
        return "graphics_state_commands_targets"
    return "drawing_misc"


def prefix_match(owner: str, prefixes: tuple[str, ...]) -> bool:
    return any(owner == prefix or owner.startswith(prefix + ".") for prefix in prefixes)


def classify_forms(owner: str, member: str) -> tuple[str, str]:
    for reason, prefixes in FORM_EXCLUSIONS.items():
        if prefix_match(owner, prefixes):
            return "excluded", reason
    if owner.startswith("System.Windows.Forms.ComponentModel.Com2Interop"):
        return "excluded", "com_interop"
    if owner in {"System.Windows.Forms.NativeWindow", "System.Windows.Forms.IWin32Window", "System.Windows.Forms.CreateParams", "System.Windows.Forms.Message"}:
        return "platform_extension_review", "portable core does not expose raw HWND/message policy"
    if owner == "System.Windows.Forms.Control" and member.startswith((
        "DoubleBuffered.",
        "GetStyle(",
        "SetStyle(",
        "OnPaintBackground(",
        "InvokePaintBackground(",
    )):
        return "measured_partial", "M11h-P1 generated facade reflects styles and physically proves coherent epoch-safe managed owner paint; overload and ordering parity remain"
    if owner == "System.Windows.Forms.Control" and (
        member.startswith("Invalidate(") or
        member == "Invalidated" or
        member.startswith("NotifyInvalidate(") or
        member.startswith("OnInvalidated(")
    ):
        return "measured_partial", "M11h-P1 physical Wine gate proves clipped rectangle/region invalidation, synchronous notification, child translation, damage merge, and failure restoration; nonrectangular fidelity and complete ordering remain"
    if owner in {
        "System.Windows.Forms.InvalidateEventArgs",
        "System.Windows.Forms.InvalidateEventHandler",
    }:
        return "measured_partial", "M11h-P1 generated nominal surface and physical Wine gate prove invalid rectangle construction, protected notification, and public event delivery"
    if owner in {
        "System.Windows.Forms.AutoSizeMode",
        "System.Windows.Forms.BoundsSpecified",
        "System.Windows.Forms.GetChildAtPointSkip",
    }:
        return "measured_partial", "M12-P9 publishes the exact enum values and rejects undefined flag bits at retained and generated mutation boundaries"
    if owner == "System.Windows.Forms.Control" and member.startswith((
        "AutoSize.",
        "AutoSizeChanged",
        "Bounds.",
        "Bottom.get",
        "BringToFront(",
        "ClientRectangle.get",
        "ClientSize.",
        "Contains(",
        "ContainsFocus.get",
        "GetAutoSizeMode(",
        "GetChildAtPoint(",
        "GetNextControl(",
        "GetPreferredSize(",
        "Height.",
        "Left.",
        "Location.",
        "LocationChanged",
        "Margin.",
        "MaximumSize.",
        "MinimumSize.",
        "PointToClient(",
        "PointToScreen(",
        "PreferredSize.get",
        "RectangleToClient(",
        "RectangleToScreen(",
        "Right.get",
        "SendToBack(",
        "SetAutoSizeMode(",
        "SetBounds(",
        "SetBoundsCore(",
        "Size.",
        "SizeChanged",
        "TabIndex.",
        "TabStop.",
        "Top.",
        "Width.",
    )):
        return "measured_partial", "M12-P9 retained and generated gates prove constrained requested/client geometry, masked bounds, preferred/min/max sizing, GrowOnly/GrowAndShrink AutoSize, ancestry transforms, descendant containment, filtered direct-child lookup, nested TabIndex traversal, and coherent z order; portable host screen origin and complete protected event-order parity remain"
    if owner in {
        "System.Windows.Forms.Control.ControlCollection",
        "System.Windows.Forms.Control+ControlCollection",
    } and member.startswith((
        "Contains(",
        "GetChildIndex(",
        "IndexOf(",
        "SetChildIndex(",
    )):
        return "measured_partial", "M12-P9 native and generated gates prove direct membership plus topmost-first index/query/mutation semantics synchronized with layout, paint, and hit testing"
    if owner in {
        "System.Windows.Forms.ErrorBlinkStyle",
        "System.Windows.Forms.ErrorIconAlignment",
    }:
        return "measured_partial", "M12-P4 renderer-free provider gate proves retained enum policy, all published alignment positions, RTL mirroring, and bounded/continuous/disabled blink behavior"
    if owner == "System.Windows.Forms.ErrorProvider" and member.startswith((
        "BlinkRate.",
        "BlinkStyle.",
        "CanExtend(",
        "Clear(",
        "ContainerControl.get",
        "GetError(",
        "GetIconAlignment(",
        "GetIconPadding(",
        "HasErrors.get",
        "Icon.",
        "RightToLeft.",
        "RightToLeftChanged",
        "SetError(",
        "SetIconAlignment(",
        "SetIconPadding(",
        "Tag.",
    )):
        return "measured_partial", "M12-P4 retained ErrorProvider proves per-control metadata, overlay geometry, ImageId substitution, semantic invalid state, target/provider cleanup, occlusion/reduced-motion scheduling, and deterministic event behavior; data binding remains open"
    if owner == "System.Windows.Forms.HelpProvider" and member.startswith((
        "CanExtend(",
        "GetHelpKeyword(",
        "GetHelpNavigator(",
        "GetHelpString(",
        "GetShowHelp(",
        "HelpNamespace.",
        "ResetShowHelp(",
        "SetHelpKeyword(",
        "SetHelpNavigator(",
        "SetHelpString(",
        "SetShowHelp(",
        "Tag.",
    )):
        return "measured_partial", "M12-P4 renderer-free HelpProvider proves retained extender metadata, automatic/explicit ShowHelp state, semantic projection, and control-first/provider-second F1 routing without implicit external navigation"
    if owner in {
        "System.Windows.Forms.HelpEventArgs",
        "System.Windows.Forms.HelpEventHandler",
    }:
        return "measured_partial", "M12-P4 HelpRequestEvent carries target, position, namespace, string, keyword, navigator, keyboard origin, and mutable handled state through tokenized deterministic delivery"
    if owner == "System.Windows.Forms.Control" and member == "HelpRequested":
        return "measured_partial", "M12-P4 public tokenized Control help event is emitted before provider policy and can terminate routing by setting handled"
    if owner == "System.Windows.Forms.Control" and member.startswith("IsMnemonic("):
        return "measured_partial", "M12-P7 renderer-free mnemonic parser proves single-marker recognition, escaped ampersands, Unicode scalar retention, ASCII case folding, and display-text elision; locale-sensitive Unicode case folding and underline cue policy remain"
    if owner == "System.Windows.Forms.Control" and member.startswith("ProcessMnemonic("):
        return "measured_partial", "M12-P8 retained Control routing precomputes a stable eligible candidate set before callbacks, preventing disposal/reparent mutation from changing the arbitration walk; protected managed override identity remains"
    if owner == "System.Windows.Forms.DialogResult":
        return "measured_partial", "M12-P8 publishes and validates every exact WinForms DialogResult numeric value in the retained native core and generated facade; independent native modal-loop closure remains host projection work"
    if owner == "System.Windows.Forms.IButtonControl":
        return "measured_partial", "M12-P8 retained Button/Window behavior and the generated interface prove DialogResult get/set, default notification, validated PerformClick, and Click-before-result ordering; arbitrary third-party IButtonControl native adaptation remains"
    if owner == "System.Windows.Forms.ButtonBase" and member.startswith("UseMnemonic."):
        return "measured_partial", "M12-P7 retained ButtonBase proves opt-in marker parsing across measure, paint, semantics, and command routing"
    if owner == "System.Windows.Forms.Button" and member.startswith((
        "DialogResult.",
        "PerformClick(",
        "NotifyDefault(",
        "ProcessMnemonic(",
    )):
        return "measured_partial", "M12-P8 retained command gate proves programmatic click, default-cue transfer, mnemonic activation, availability rejection, validation cancellation, exact DialogResult validation, and Click-before-result propagation; independent native modal closure remains separate"
    if owner in {
        "System.Windows.Forms.CheckBox",
        "System.Windows.Forms.RadioButton",
        "System.Windows.Forms.GroupBox",
        "System.Windows.Forms.Label",
    } and member.startswith("ProcessMnemonic("):
        return "measured_partial", "M12-P8 retained mnemonic routing proves choice activation, Label/GroupBox next-control focus, and duplicate collision cycling over a precomputed stable candidate set; locale case folding and underline cue policy remain"
    if owner == "System.Windows.Forms.Label" and member.startswith("UseMnemonic."):
        return "measured_partial", "M12-P7 retained Label proves marker-aware measure, paint, semantics, and next-control focus"
    if owner == "System.Windows.Forms.ContainerControl" and member.startswith((
        "ProcessDialogChar(",
        "ProcessMnemonic(",
    )):
        return "measured_partial", "M12-P8 renderer-free retained subtree routing proves stable candidate snapshots, duplicate cycling, callback-mutation safety, effective visibility/enabled state, active-focus-scope containment, and counters; protected managed override identity remains"
    if owner == "System.Windows.Forms.Form" and member.startswith((
        "AcceptButton.",
        "CancelButton.",
        "DialogResult.",
        "ProcessDialogChar(",
        "ProcessDialogKey(",
        "ProcessMnemonic(",
    )):
        return "measured_partial", "M12-P8 Window/Form command kernel proves default/cancel assignment, cue transfer, Enter/Escape routing, exact retained DialogResult state, Click-before-result ordering, validation, disposal cleanup, active-scope containment, and counters; independent native modal closure and full managed protected-call projection remain"
    if owner in {
        "System.Windows.Forms.ToolStrip",
        "System.Windows.Forms.ToolStripDropDown",
        "System.Windows.Forms.ToolStripMenuItem",
        "System.Windows.Forms.ToolStripItem",
    } and member.startswith("ProcessMnemonic("):
        return "measured_partial", "M12-P8 retained MenuStrip and popup rows prove marker-free presentation, top-level duplicate cycling, contained submenu mnemonic activation, and shared command execution; protected generated ToolStrip override identity and remaining item subclasses stay open"
    if owner == "System.Windows.Forms.HelpNavigator" and member != "TopicId = -2147483641":
        return "measured_partial", "M12-P4 retains six help navigation intents; TopicId and raw managed enum-value projection remain open"
    if owner in {
        "System.Windows.Forms.AutoValidate",
        "System.Windows.Forms.ValidationConstraints",
    }:
        return "measured_partial", "M12-P6 renderer-free validation gate proves the published enum values, inheritance, filtering, and prevent/allow focus branches"
    if owner == "System.Windows.Forms.Control" and member.startswith((
        "CausesValidation.",
        "CausesValidationChanged",
        "Validated",
        "Validating",
        "OnCausesValidationChanged(",
        "OnValidated(",
        "OnValidating(",
    )):
        return "measured_partial", "M12-P6 retained focus transaction proves CausesValidation, cancellable Validating, successful Validated, pre-focus-loss ordering, reentrancy rejection, and disposal-safe endpoint rechecks; managed delegate identity remains open"
    if owner == "System.Windows.Forms.ContainerControl" and member.startswith((
        "AutoValidate.",
        "AutoValidateChanged",
        "OnAutoValidateChanged(",
        "Validate(",
        "ValidateChildren(",
    )):
        return "measured_partial", "M12-P6 retained ContainerControl proves inherited AutoValidate, prevent/allow/disable policy, ancestor focus validation, explicit validation, and deterministic constrained child traversal; nested managed container edge parity remains"
    if owner in {
        "System.Windows.Forms.Form",
        "System.Windows.Forms.UserControl",
    } and member.startswith((
        "AutoValidate.",
        "AutoValidateChanged",
        "Validate(",
        "ValidateChildren(",
    )):
        return "measured_partial", "M12-P6 Form/UserControl inherit the measured retained ContainerControl validation transaction; independent native-form oracle and generated facade override identity remain"
    if owner == "System.Windows.Forms.ErrorProvider" and member.startswith((
        "BindToDataAndErrors(",
        "DataMember.",
        "DataSource.",
        "UpdateBinding(",
    )):
        return "measured_partial", "M12-P6 binding-aware ErrorProvider proves same-window BindingSource attachment, current-record field and aggregate errors, BindingComplete parse failures, currency refresh, aggregation, and synchronous source cleanup; arbitrary managed IDataErrorInfo and nested object traversal remain"
    if owner in {
        "System.Windows.Forms.BindingCompleteContext",
        "System.Windows.Forms.BindingCompleteState",
        "System.Windows.Forms.DataSourceUpdateMode",
        "System.Windows.Forms.ControlUpdateMode",
    }:
        return "measured_partial", "M12-P5 renderer-free and Wine gates prove the published enum values and update-mode branches"
    if owner == "System.Windows.Forms.Binding" and member.startswith((
        "BindableComponent.get",
        "Binding(",
        "BindingComplete",
        "Control.get",
        "ControlUpdateMode.",
        "DataSource.get",
        "DataSourceNullValue.",
        "DataSourceUpdateMode.",
        "Format",
        "FormatString.",
        "FormattingEnabled.",
        "IsBinding.get",
        "NullValue.",
        "Parse",
        "PropertyName.get",
        "ReadValue(",
        "WriteValue(",
        "OnBindingComplete(",
        "OnFormat(",
        "OnParse(",
    )):
        return "measured_partial", "M12-P5 retained Binding proves explicit descriptors, two-way modes, invariant format/parse, null substitution, post-commit completion, errors, reentrancy suppression, and endpoint cleanup; managed reflection, arbitrary IFormatProvider, and exact façade overloads remain"
    if owner == "System.Windows.Forms.BindingSource" and member.startswith((
        "Add(",
        "AllowEdit.get",
        "AllowNew.",
        "AllowRemove.get",
        "BindingComplete",
        "CancelEdit(",
        "Clear(",
        "Count.get",
        "CurrencyManager.get",
        "Current.get",
        "CurrentChanged",
        "CurrentItemChanged",
        "DataError",
        "DataMember.",
        "DataMemberChanged",
        "DataSourceChanged",
        "EndEdit(",
        "Find(",
        "Insert(",
        "IsBindingSuspended.get",
        "List.get",
        "ListChanged",
        "MoveFirst(",
        "MoveLast(",
        "MoveNext(",
        "MovePrevious(",
        "Position.",
        "PositionChanged",
        "RaiseListChangedEvents.",
        "RemoveAt(",
        "RemoveCurrent(",
        "ResetBindings(",
        "ResetCurrentItem(",
        "ResetItem(",
        "ResumeBinding(",
        "SuspendBinding(",
        "OnBindingComplete(",
        "OnCurrentChanged(",
        "OnCurrentItemChanged(",
        "OnDataError(",
        "OnDataMemberChanged(",
        "OnDataSourceChanged(",
        "OnListChanged(",
        "OnPositionChanged(",
    )):
        return "measured_partial", "M12-P5 retained record source proves currency, stable identity, add/insert/remove/edit, property/list/reset notification, suspension coalescing, completion/error propagation, deterministic ordering, and disposal; raw IList/object facades, nested DataMember, sort/filter, and descriptor reflection remain"
    if owner == "System.Windows.Forms.BindingManagerBase" and member.startswith((
        "BindingComplete",
        "CancelCurrentEdit(",
        "Count.get",
        "Current.get",
        "CurrentChanged",
        "CurrentItemChanged",
        "DataError",
        "EndCurrentEdit(",
        "IsBindingSuspended.get",
        "Position.",
        "PositionChanged",
        "PullData(",
        "PushData(",
        "ResumeBinding(",
        "SuspendBinding(",
    )):
        return "measured_partial", "M12-P5 CurrencyManager proves current/count/position, edit, suspension, manager-wide push/pull, and propagated completion over one retained BindingSource; arbitrary derived managers and descriptor collections remain"
    if owner == "System.Windows.Forms.CurrencyManager" and member.startswith((
        "CancelCurrentEdit(",
        "Count.get",
        "Current.get",
        "EndCurrentEdit(",
        "List.get",
        "ListChanged",
        "Position.",
        "Refresh(",
        "ResumeBinding(",
        "SuspendBinding(",
    )):
        return "measured_partial", "M12-P5 proves retained list currency, deterministic movement, edit transactions, reset refresh, and suspension; managed property descriptors remain"
    if owner == "System.Windows.Forms.BindingContext" and member.startswith((
        "Add(",
        "Clear(",
        "CollectionChanged",
        "Contains(",
        "Remove(",
        "this[",
        "AddCore(",
        "ClearCore(",
        "OnCollectionChanged(",
        "RemoveCore(",
    )):
        return "measured_partial", "M12-P5 same-window BindingContext proves manager identity, membership, ordered collection changes, and eager source-disposal removal; nested member paths and static context reassignment remain"
    if owner == "System.Windows.Forms.ControlBindingsCollection" and member.startswith((
        "Add(",
        "AddCore(",
        "BindableComponent.get",
        "Clear(",
        "ClearCore(",
        "Control.get",
        "ControlBindingsCollection(",
        "DefaultDataSourceUpdateMode.",
        "Remove(",
        "RemoveCore(",
        "this[",
    )):
        return "measured_partial", "M12-P5 per-Control ownership proves unique canonical properties, explicit/default update modes, add/find/remove/clear, and synchronous disposal; managed object/reflection overload and RemoveAt parity remain"
    if prefix_match(owner, FORM_PARTIAL_NATIVE):
        return "partial_native", "retained type/facade exists; this exact member remains unproven unless separately cited"
    return "missing", "admitted nominal base or unresolved package placement"


def classify_drawing(owner: str, member: str) -> tuple[str, str]:
    for reason, prefixes in DRAWING_EXCLUSIONS.items():
        if prefix_match(owner, prefixes):
            return "excluded", reason
    if "CopyFromScreen" in member:
        return "excluded", "arbitrary desktop capture"
    if "Metafile" in member:
        return "excluded", "metafile playback/recording"
    if any(word in member for word in ("GetHdc", "ReleaseHdc", "FromHdc", "FromHwnd", "GetHbitmap", "FromHbitmap", "FromHicon")):
        return "platform_extension_partial", "opaque native-surface lease only"
    if owner == "System.Drawing.Region" and member == "GetBounds":
        return "measured_partial", "M11h-P1 GUI.Drawing ABI region-bounds projection is physically consumed by Wine managed invalidation; graphics-transform and infinite-region parity remain"
    if prefix_match(owner, DRAWING_PARTIAL_NATIVE):
        return "partial_native", "GUI.Drawing type exists; exact overload/behavior remains ledger-open"
    if owner.startswith("System.Drawing.Imaging.ImageCodecInfo") or owner.startswith("System.Drawing.Imaging.PropertyItem"):
        return "package_review", "PNG-only core; other codecs/metadata require isolated admission"
    return "missing", "admitted GUI.Drawing oversight or unresolved package placement"


def read_forms(path: Path) -> list[dict[str, str]]:
    raw = []
    for number, source in enumerate(path.read_text(encoding="utf-8-sig").splitlines(), 1):
        if not source.strip() or source.startswith("#"):
            continue
        api, marker = clean_api_line(source)
        raw.append((number, api, marker))
    types = sorted((api for _, api, _ in raw if " -> " not in api), key=len, reverse=True)
    rows = []
    for number, api, marker in raw:
        kind = "type" if " -> " not in api else "member"
        owner = api if kind == "type" else owner_for(api.split(" -> ", 1)[0], types)
        member = "" if kind == "type" else api[len(owner) + 1 :].split(" -> ", 1)[0]
        status, reason = classify_forms(owner, member)
        rows.append({
            "surface": "System.Windows.Forms",
            "source_line": str(number),
            "kind": kind,
            "owner": owner,
            "member": member,
            "api": api,
            "family": form_family(owner),
            "disposition": "excluded" if status == "excluded" else "admitted",
            "status": status,
            "reason": reason,
            "source_marker": marker,
        })
    return rows


def read_drawing(paths: list[Path]) -> list[dict[str, str]]:
    rows = []
    for path in paths:
        root = ET.parse(path).getroot()
        for number, node in enumerate(root.findall(".//member"), 1):
            api = node.attrib.get("name", "")
            if not api or len(api) < 3 or api[1] != ":":
                continue
            code, body = api[0], api[2:]
            kind = {"T": "type", "M": "method", "P": "property", "F": "field", "E": "event"}.get(code, "member")
            if kind == "type":
                owner, member = body, ""
            else:
                head = body.split("(", 1)[0]
                owner, _, member = head.rpartition(".")
            status, reason = classify_drawing(owner, member)
            rows.append({
                "surface": "System.Drawing",
                "source_line": f"{path.name}:{number}",
                "kind": kind,
                "owner": owner,
                "member": member,
                "api": api,
                "family": drawing_family(owner),
                "disposition": "excluded" if status == "excluded" else "admitted",
                "status": status,
                "reason": reason,
                "source_marker": "",
            })
    return rows


def write_summary(path: Path, rows: list[dict[str, str]]) -> None:
    surfaces = Counter(row["surface"] for row in rows)
    dispositions = Counter((row["surface"], row["disposition"]) for row in rows)
    statuses = Counter((row["surface"], row["status"]) for row in rows)
    families = Counter((row["surface"], row["family"]) for row in rows)
    lines = [
        "# WinForms and drawing API oversight catalogue",
        "",
        "Status: **OBSERVED upstream inventory; conservative implementation ledger**.",
        "",
        f"LibreWinForms source: `wieslawsoltes/LibreWinForms` branch `librewinforms-progpu-port`, commit `{UPSTREAM_COMMIT}`. Forms rows come from its `src/System.Windows.Forms/PublicAPI.Shipped.txt`. Drawing rows come from the pinned local .NET 10.0.3 reference XML. The generated TSV is the member-level authority; this summary is navigation.",
        "",
        "A `partial_native` row means only that a corresponding retained GUI.Forms/GUI.Drawing type exists. `measured_partial` identifies an exact behavioral center with named evidence while retaining its stated gaps. Neither status claims that every overload, exception, event order, accessibility projection, or raster result is complete. `missing` is the default for admitted calls without direct evidence.",
        "",
        "## Totals",
        "",
        "| Surface | Rows | Admitted | Excluded |",
        "|---|---:|---:|---:|",
    ]
    for surface in sorted(surfaces):
        lines.append(f"| `{surface}` | {surfaces[surface]} | {dispositions[surface, 'admitted']} | {dispositions[surface, 'excluded']} |")
    lines += ["", "## Honest current states", "", "| Surface | State | Rows |", "|---|---|---:|"]
    for (surface, status), count in sorted(statuses.items()):
        lines.append(f"| `{surface}` | `{status}` | {count} |")
    lines += ["", "## Families", "", "| Surface | Family | Rows |", "|---|---|---:|"]
    for (surface, family), count in sorted(families.items()):
        lines.append(f"| `{surface}` | `{family}` | {count} |")
    lines += [
        "",
        "## Policy boundary",
        "",
        "The base excludes ActiveX/browser hosting, printing, MDI, design-time services, classic `DataGrid`, obsolete menu/toolbar/status families, metafile playback, and arbitrary desktop capture. Raw HWND/HDC members are platform-extension work behind opaque leases, never portable public types. PNG is the renderer-core codec; other codec/metadata families remain package review.",
        "",
        "Everything else is an admitted oversight row, not an implementation promise. Promotion to `native` requires a named test or runtime trace for the exact behavioral center; generated identity resolution alone is facade evidence.",
        "",
        "## Regeneration",
        "",
        "See `tools/winforms_catalogue/README.md`. Regeneration is deterministic for the pinned inputs and writes `planning/generated/WINFORMS_API_CATALOGUE.tsv` plus this summary.",
        "",
    ]
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--forms-api", required=True, type=Path)
    parser.add_argument("--drawing-xml", required=True, type=Path, action="append")
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--summary", required=True, type=Path)
    args = parser.parse_args()
    rows = read_forms(args.forms_api) + read_drawing(args.drawing_xml)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.summary.parent.mkdir(parents=True, exist_ok=True)
    fields = [
        "surface",
        "source_line",
        "kind",
        "owner",
        "member",
        "api",
        "family",
        "disposition",
        "status",
        "source_marker",
        "reason",
    ]
    with args.output.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(
            stream,
            fieldnames=fields,
            dialect="excel-tab",
            lineterminator="\n",
        )
        writer.writeheader()
        writer.writerows(rows)
    write_summary(args.summary, rows)
    print(json.dumps({"rows": len(rows), "output": str(args.output), "summary": str(args.summary)}, sort_keys=True))


if __name__ == "__main__":
    main()
