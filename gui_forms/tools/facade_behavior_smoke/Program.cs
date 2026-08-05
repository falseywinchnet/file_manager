using System;
using System.Collections;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;

if (args.Length == 1 && args[0] == "timer")
{
    return RunTimerHost();
}
if (args.Length == 1 && args[0] == "doevents")
{
    return RunDoEventsHost();
}
if (args.Length == 1 && args[0] == "lifecycle")
{
    return RunLoadLifecycle();
}
if (args.Length == 1 && args[0] == "pointer")
{
    return RunPointerHost();
}
if (args.Length == 1 && args[0] == "button")
{
    return RunRasterButtonHost();
}
if (args.Length == 1 && args[0] == "dialog")
{
    return RunDialogHost();
}
if (args.Length == 1 && args[0] == "menu")
{
    return RunMenuHost();
}
if (args.Length == 1 && args[0] == "menu-live")
{
    return RunOverlayMenuHost();
}
if (args.Length == 1 && args[0] == "menu-hover")
{
    return RunHoverMenuHost();
}
if (args.Length == 1 && args[0] == "popup-repeat")
{
    return RunPopupRepeatHost();
}
if (args.Length == 1 && args[0] == "combo")
{
    return RunComboHost();
}
if (args.Length == 1 && args[0] == "font-thread")
{
    return RunCrossThreadFontHost();
}
if (args.Length == 1 && args[0] == "slider-live")
{
    return RunSliderLiveHost();
}
if (args.Length == 1 && args[0] == "messagebox")
{
    return RunMessageBoxHost();
}
if (args.Length == 1 && args[0] == "dialog-live")
{
    return RunNativeDialogLiveHost();
}
if (args.Length == 1 && args[0] == "folder-live")
{
    return RunNativeFolderDialogLiveHost();
}
if (args.Length == 1 && args[0] == "tooltip-live")
{
    return RunTooltipLiveHost();
}

var form = new Form { Name = "behaviorForm", Text = "M11d behavior", Size = new Size(640, 420) };
using var userControl = new UserControl();
using var pictureBox = new PictureBox();
var table = new TableLayoutPanel
{
    Name = "settingsTable",
    Location = new Point(12, 18),
    Size = new Size(440, 260),
    ColumnCount = 2,
    RowCount = 3,
    Dock = DockStyle.Fill,
    Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
};
table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 40f));
table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 60f));
table.RowStyles.Add(new RowStyle(SizeType.AutoSize));

var enabled = new CheckBox { Name = "enabledCheck", Text = "Enabled", Checked = false };
var mode = new ComboBox { Name = "modeCombo", DropDownStyle = ComboBoxStyle.DropDownList };
var gain = new NumericUpDown { Name = "gainNumeric", Minimum = -20m, Maximum = 80m, Increment = 0.5m, DecimalPlaces = 1 };
var radio = new RadioButton { Name = "radioChoice" };
var stateProbe = new StateChangeProbe { Name = "stateProbe" };

var checkEvents = 0;
var selectionEvents = 0;
var valueEvents = 0;
var radioEvents = 0;
enabled.CheckedChanged += (_, _) => ++checkEvents;
mode.SelectedIndexChanged += (_, _) => ++selectionEvents;
gain.ValueChanged += (_, _) => ++valueEvents;
radio.CheckedChanged += (_, _) => ++radioEvents;

mode.Items.AddRange(["WFM", "NFM", "AM"]);
mode.SelectedIndex = 1;
gain.Value = 12.5m;
enabled.Checked = true;
radio.Checked = true;
stateProbe.Text = "ready";
stateProbe.Enabled = false;
stateProbe.Visible = false;
stateProbe.Visible = true;
stateProbe.ForeColor = Color.Navy;
stateProbe.BackColor = Color.White;
stateProbe.Font = new Font("Lucida Grande", 11f);

table.Controls.Add(enabled, 0, 0);
table.Controls.Add(mode, 1, 0);
table.Controls.Add(gain, 1, 1);
table.Controls.Add(radio, 0, 1);
table.SetColumnSpan(radio, 2);
form.Controls.Add(table);

var moveEvents = 0;
var sizeEvents = 0;
table.Move += (_, _) => ++moveEvents;
table.SizeChanged += (_, _) => ++sizeEvents;
table.Location = new Point(20, 24);
table.Size = new Size(460, 280);

var menu = new ToolStrip();
var renderer = new ToolStripProfessionalRenderer();
var startItem = new ToolStripMenuItem("Start") { Tag = "receiver", Size = new Size(80, 24) };
var modeItem = new ToolStripMenuItem("Mode");
var fmItem = new ToolStripMenuItem("FM");
menu.Items.Add(startItem);
modeItem.DropDownItems.Add(fmItem);

var padding = new Padding(1, 2, 3, 4);
var mouse = new MouseEventArgs(MouseButtons.Left, 2, 17, 23, 120);
var key = new KeyEventArgs(Keys.Control | Keys.A);
var layoutProbe = new LayoutProbe { Size = new Size(220, 90) };
var dockedChild = new Panel { Dock = DockStyle.Fill };
layoutProbe.Controls.Add(dockedChild);
var binding = new BindingSource { DataSource = new ArrayList { "notch-a", "notch-b" } };
var grid = new DataGridView { DataSource = binding };
var firstColumn = new DataGridViewTextBoxColumn();
var secondColumn = new DataGridViewTextBoxColumn();
grid.Columns.AddRange([firstColumn, secondColumn]);
var tableProbe = new TableLayoutPanel
{
    Size = new Size(200, 80), ColumnCount = 2, RowCount = 1, Padding = new Padding(0)
};
tableProbe.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 80f));
tableProbe.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100f));
tableProbe.RowStyles.Add(new RowStyle(SizeType.Percent, 100f));
var tableLeft = new Button { Dock = DockStyle.Fill, Margin = new Padding(0) };
var tableRight = new Button { Dock = DockStyle.Fill, Margin = new Padding(0) };
tableProbe.Controls.Add(tableLeft, 0, 0);
tableProbe.Controls.Add(tableRight, 1, 0);
var tableTop = new Panel { Dock = DockStyle.Top, Height = 12, Margin = new Padding(0) };
tableProbe.SetColumnSpan(tableTop, 2);
tableProbe.Controls.Add(tableTop, 0, 0);
var boundedTable = new TableLayoutPanel
{
    Size = new Size(120, 40), ColumnCount = 3, RowCount = 1, Padding = new Padding(0)
};
boundedTable.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
boundedTable.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 40f));
boundedTable.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 60f));
boundedTable.RowStyles.Add(new RowStyle(SizeType.Percent, 100f));
var oversizedAuto = new Label { Size = new Size(300, 40), Margin = new Padding(0) };
var boundedMiddle = new Button { Dock = DockStyle.Fill, Margin = new Padding(0) };
var boundedRight = new Button { Dock = DockStyle.Fill, Margin = new Padding(0) };
boundedTable.Controls.Add(oversizedAuto, 0, 0);
boundedTable.Controls.Add(boundedMiddle, 1, 0);
boundedTable.Controls.Add(boundedRight, 2, 0);
var flowProbe = new FlowLayoutPanel { Size = new Size(170, 80), Padding = new Padding(0), WrapContents = true };
var flowFirst = new Button { Size = new Size(80, 20), Margin = new Padding(0) };
var flowSecond = new Button { Size = new Size(80, 20), Margin = new Padding(0) };
var flowThird = new Button { Size = new Size(80, 20), Margin = new Padding(0) };
flowProbe.Controls.AddRange([flowFirst, flowSecond, flowThird]);
var listProbe = new ListBox { Size = new Size(160, 88) };
listProbe.Items.Add("alpha");
listProbe.Items.Add("gamma");
listProbe.Items.Insert(1, "beta");
listProbe.SelectedIndex = 2;
listProbe.Items.RemoveAt(2);
var splitProbe = new SplitContainer { Size = new Size(204, 80) };
splitProbe.PerformLayout();
var splitterProbe = new Splitter { SplitPosition = 37 };
var progressProbe = new ProgressBar();
progressProbe.Value = 42;
var progressRejected = false;
try { progressProbe.Value = 101; }
catch (ArgumentOutOfRangeException) { progressRejected = true; }
var projectedProgress = (double)(typeof(Control).GetProperty("__NativeRangeValue",
    global::System.Reflection.BindingFlags.Instance | global::System.Reflection.BindingFlags.NonPublic)!
    .GetValue(progressProbe) ?? -1d);
var tooltipProbe = new ToolTip();
tooltipProbe.SetToolTip(enabled, "Enable receiver");
var dialogFileVariable = Environment.GetEnvironmentVariable("GUI_FORMS_DIALOG_FILE");
var dialogFolderVariable = Environment.GetEnvironmentVariable("GUI_FORMS_DIALOG_FOLDER");
Environment.SetEnvironmentVariable("GUI_FORMS_DIALOG_FILE", "/tmp/gui-forms/radio.ini");
Environment.SetEnvironmentVariable("GUI_FORMS_DIALOG_FOLDER", "/tmp/gui-forms");
using var openDialogProbe = new OpenFileDialog { FileName = "old.ini", InitialDirectory = "/tmp" };
using var folderDialogProbe = new FolderBrowserDialog { SelectedPath = "/" };
var openDialogResult = openDialogProbe.ShowDialog();
var folderDialogResult = folderDialogProbe.ShowDialog();
Environment.SetEnvironmentVariable("GUI_FORMS_DIALOG_FILE", dialogFileVariable);
Environment.SetEnvironmentVariable("GUI_FORMS_DIALOG_FOLDER", dialogFolderVariable);
using var colorDialogProbe = new ColorDialog { Color = Color.CadetBlue };
var dateProbe = new DateTimePicker { CustomFormat = "yyyy-MM-dd", Value = new DateTime(2026, 8, 5) };
Clipboard.SetText("GUI.Forms retained clipboard");

Require(form.Size == new Size(640, 420), "form size");
Require(userControl.Size == new Size(150, 150), "user-control default size");
((System.ComponentModel.ISupportInitialize)gain).BeginInit();
((System.ComponentModel.ISupportInitialize)gain).EndInit();
((System.ComponentModel.ISupportInitialize)pictureBox).BeginInit();
((System.ComponentModel.ISupportInitialize)pictureBox).EndInit();
Require(table.Parent == form && enabled.Parent == table, "parenting");
Require(table.Controls.Count == 4 && table.Controls[1] == mode, "table collection");
Require(mode.Items.Count == 3 && Equals(mode.SelectedItem, "NFM"), "combo state");
Require(gain.Value == 12.5m && gain.Minimum == -20m && gain.Maximum == 80m, "numeric state");
Require(mode.Text == "NFM" && gain.Text == "12.5", "field text projection");
Require(gain.Controls.Count == 2 && gain.Controls[0] is Button && gain.Controls[1] is TextBox,
    "numeric composite children");
using (var spin = new NumericProbe { Minimum = 0m, Maximum = 10m, Increment = 2m, Value = 5m, Size = new Size(120, 24) })
{
    spin.SpinAt(115, 4);
    Require(spin.Value == 7m && spin.Text == "7", "numeric upper spin");
    spin.SpinAt(115, 20);
    spin.WheelBy(120);
    Require(spin.Value == 7m, "numeric lower spin and wheel");
}
using (var textFont = new Font("Lucida Grande", 12f))
using (var textBitmap = new Bitmap(140, 30))
using (var textGraphics = Graphics.FromImage(textBitmap))
{
    var measured = TextRenderer.MeasureText("Radio", textFont);
    Require(measured.Width > 8 && measured.Height >= textFont.Height, "text renderer measurement");
    Require(textGraphics.MeasureString(null!, textFont, 120, new StringFormat()) == SizeF.Empty,
        "drawing null text matches System.Drawing empty-span semantics");
    using var nullTextBrush = new SolidBrush(Color.Black);
    textGraphics.DrawString(null!, null!, nullTextBrush, PointF.Empty);
    textGraphics.DrawString(string.Empty, null!, nullTextBrush, PointF.Empty);
    textGraphics.Clear(Color.White);
    TextRenderer.DrawText(textGraphics, "Radio", textFont, new Rectangle(0, 0, 140, 30),
        Color.Black, TextFormatFlags.Left | TextFormatFlags.VerticalCenter | TextFormatFlags.SingleLine);
    var inkFound = false;
    for (var y = 0; y < textBitmap.Height && !inkFound; ++y)
        for (var x = 0; x < textBitmap.Width; ++x)
            if (textBitmap.GetPixel(x, y).ToArgb() != Color.White.ToArgb()) { inkFound = true; break; }
    Require(inkFound, "text renderer drawing");
}
Require(enabled.Checked && radio.Checked, "check state");
Require(checkEvents == 1 && selectionEvents == 1 && valueEvents == 1 && radioEvents == 1, "state events");
Require(stateProbe.TextChanges == 1 && stateProbe.EnabledChanges == 1 &&
    stateProbe.VisibleChanges == 2 && stateProbe.ForeColorChanges == 1 &&
    stateProbe.BackColorChanges == 1 && stateProbe.FontChanges == 1,
    "virtual state-change routing");
Require(moveEvents == 1 && sizeEvents == 1 && table.ClientRectangle.Size == table.Size, "geometry events");
Require(table.FindForm() == form && gain.PointToScreen(Point.Empty) ==
    new Point(table.Left + gain.Left, table.Top + gain.Top), "coordinate ancestry");
Require(menu.Items[0] == startItem && startItem.Owner == menu && startItem.Text == "Start" && Equals(startItem.Tag, "receiver"), "toolstrip state");
Require(ReferenceEquals(enabled.FlatAppearance, enabled.FlatAppearance), "flat appearance ownership");
Require(ReferenceEquals(renderer.ColorTable, renderer.ColorTable), "renderer color-table ownership");
Require(modeItem.DropDown.OwnerItem == modeItem && modeItem.DropDownItems.Count == 1 &&
    modeItem.DropDownItems[0] == fmItem, "dropdown item ownership");
Require(padding.Left == 1 && padding.Top == 2 && padding.Right == 3 && padding.Bottom == 4 && padding.All == -1, "padding state");
Require(mouse.Button == MouseButtons.Left && mouse.X == 17 && mouse.Y == 23 && mouse.Delta == 120, "mouse args");
Require(key.KeyCode == Keys.A && key.Modifiers == Keys.Control, "key args");
Require(gain.Focus(), "focus acceptance");
Require(gain.Focused, "focused state");
Require(form.ContainsFocus, "ancestor focus state");
Require(ReferenceEquals(form.ActiveControl, gain), "container active-control projection");
var dockPadding = new ScrollableControl.DockPaddingEdges { All = 4, Left = 7 };
Require(dockPadding.Left == 7 && dockPadding.Top == 4 && dockPadding.Right == 4 &&
    dockPadding.Bottom == 4, "dock padding edge state");
Require(Cursors.Default is not null && Cursors.Hand is not null && Cursors.HSplit is not null &&
    Cursors.VSplit is not null && ReferenceEquals(Cursors.Default, Cursors.Default),
    "cursor singleton identity");
Require(layoutProbe.Layouts > 0 && dockedChild.Bounds == layoutProbe.ClientRectangle,
    "virtual layout and fill docking");
Require(grid.Columns.Count == 2 && ReferenceEquals(grid.Columns[0], firstColumn) &&
    grid.Rows.Count == 2 && ReferenceEquals(grid.DefaultCellStyle, grid.DefaultCellStyle),
    "grid collections and binding source");
Require(binding.List.Count == 2 && Equals(binding.Current, "notch-a"), "binding source state");
Require(tableLeft.Bounds == new Rectangle(0, 0, 80, 80) &&
    tableRight.Bounds == new Rectangle(80, 0, 120, 80), "table layout cells");
Require(tableTop.Bounds == new Rectangle(0, 0, 200, 12), "table top docking");
Require(oversizedAuto.Right <= boundedTable.ClientSize.Width &&
    boundedMiddle.Right <= boundedTable.ClientSize.Width &&
    boundedRight.Right <= boundedTable.ClientSize.Width,
    "table layout containment");
Require(flowFirst.Location == Point.Empty && flowSecond.Location == new Point(80, 0) &&
    flowThird.Location == new Point(0, 20), "flow layout wrapping");
Require(listProbe.Items.Count == 2 && Equals(listProbe.Items[0], "alpha") &&
    Equals(listProbe.Items[1], "beta") && listProbe.SelectedIndex == 1,
    "listbox collection and selection normalization");
Require(splitProbe.Panel1.Parent == splitProbe && splitProbe.Panel2.Parent == splitProbe &&
    splitProbe.Panel1.Right <= splitProbe.Panel2.Left && splitProbe.Panel2.Right <= splitProbe.Width,
    "split container panel ownership and containment");
Require(splitterProbe.SplitPosition == 37, "splitter position state");
Require(progressProbe.Minimum == 0 && progressProbe.Maximum == 100 && progressRejected,
    "progress range validation");
Require(projectedProgress == 42d, "progress native range projection");
Require(!progressProbe.Capture, "unattached capture query");
Require(tooltipProbe.GetToolTip(enabled) == "Enable receiver", "tooltip association state");
tooltipProbe.SetToolTip(enabled, string.Empty);
Require(tooltipProbe.GetToolTip(enabled) == string.Empty, "tooltip association removal");
Require(openDialogResult == DialogResult.OK && openDialogProbe.FileName == "/tmp/gui-forms/radio.ini" &&
    openDialogProbe.SafeFileName == "radio.ini", "file dialog provider state");
Require(folderDialogResult == DialogResult.OK && folderDialogProbe.SelectedPath == "/tmp/gui-forms",
    "folder dialog provider state");
Require(colorDialogProbe.Color == Color.CadetBlue, "color dialog state");
Require(dateProbe.Value == new DateTime(2026, 8, 5) && dateProbe.CustomFormat == "yyyy-MM-dd",
    "date-time state");
Require(Clipboard.GetText() == "GUI.Forms retained clipboard", "clipboard state");
Require(ControlPaint.Dark(Color.White).R < 255 && ControlPaint.Light(Color.Black).R > 0,
    "control-paint color transforms");
Require(Application.ExecutablePath.Length != 0 && Screen.AllScreens.Length == 1 &&
    Screen.GetWorkingArea(Point.Empty).Height > 0 && SystemInformation.DragSize.Width > 0,
    "environment compatibility state");

Console.WriteLine("surface=control,table,check,radio,combo,numeric,toolstrip,args");
Console.WriteLine($"events=check:{checkEvents}|selection:{selectionEvents}|value:{valueEvents}|radio:{radioEvents}|move:{moveEvents}|size:{sizeEvents}");
Console.WriteLine($"tree={form.Controls.Count}/{table.Controls.Count}|selected={mode.SelectedItem}|gain={gain.Value}|focus={gain.Focused}");

var disposalParent = new Panel();
var latePaintChild = new PaintInputProbe { Size = new Size(40, 20) };
disposalParent.Controls.Add(latePaintChild);
disposalParent.Dispose();
latePaintChild.Invalidate();

form.Dispose();
listProbe.Dispose();
splitProbe.Dispose();
splitterProbe.Dispose();
progressProbe.Dispose();
tooltipProbe.Dispose();
dateProbe.Dispose();
Console.WriteLine("behavior=pass");
return 0;

static int RunTimerHost()
{
    var form = new Form { Name = "timerForm", Text = "M11d timer", Size = new Size(360, 180) };
    using var timer = new System.Windows.Forms.Timer { Interval = 25 };
    var ticks = 0;
    timer.Tick += (_, _) =>
    {
        ++ticks;
        timer.Stop();
        form.Close();
    };
    timer.Start();
    Application.Run(form);
    Require(ticks == 1 && !timer.Enabled, "timer lifecycle");
    Console.WriteLine($"timer=ticks:{ticks}|enabled:{timer.Enabled}|host:{(Application.LastHostTrace.Contains("win32-dib", StringComparison.Ordinal) ? "win32-dib" : "other")}");
    form.Dispose();
    return 0;
}

static int RunDoEventsHost()
{
    var form = new Form { Name = "doEventsForm", Text = "Nested dispatch", Size = new Size(360, 180) };
    var nested = 0;
    var loadReturned = false;
    form.Load += (_, _) =>
    {
        form.BeginInvoke((Action)(() => ++nested));
        Application.DoEvents();
        Require(nested == 0, "BeginInvoke remains posted until the load callback unwinds");
        loadReturned = true;
        form.BeginInvoke((Action)form.Close);
    };
    Application.Run(form);
    Require(loadReturned && nested == 1, "posted dispatch runs exactly once after callback unwind");
    Console.WriteLine($"doevents=nested:{nested}|load-returned:{loadReturned}");
    form.Dispose();
    return 0;
}

static int RunMenuHost()
{
    var form = new Form { Name = "menuForm", Size = new Size(700, 220) };
    var menu = new MenuProbe { Name = "contextMenu" };
    using var icon = new Bitmap(32, 32);
    using (var iconGraphics = Graphics.FromImage(icon)) iconGraphics.Clear(Color.SteelBlue);
    var connect = new ToolStripMenuItem("Connect") { Image = icon, Size = new Size(112, 22) };
    var clicked = 0;
    connect.Click += (_, _) => ++clicked;
    var source = new ToolStripMenuItem("Source") { Image = icon, Size = new Size(112, 22) };
    var nestedClicked = 0;
    var server = new ToolStripMenuItem("AIRSPY Server Network");
    server.Click += (_, _) => ++nestedClicked;
    source.DropDownItems.Add(server);
    menu.Items.Add(connect);
    menu.Items.Add(new ToolStripSeparator());
    menu.Items.Add(source);
    menu.Show(form, new Point(24, 36));
    Require(menu.Bounds.X == 24 && menu.Bounds.Y == 36, "menu screen position");
    Require(menu.Width >= 136 && menu.Height >= 61, "menu retained vertical extent");
    Require(source.DropDown.OwnerItem == source && source.DropDownItems.Count == 1,
        "nested menu ownership");
    using var raster = menu.RenderToBitmap();
    var glyphPixels = 0;
    var nonBackgroundPixels = 0;
    for (var y = 7; y < Math.Min(26, raster.Height - 2); ++y)
    for (var x = 39; x < Math.Min(raster.Width - 18, 112); ++x)
    {
        var pixel = raster.GetPixel(x, y);
        if (pixel.R != 247 || pixel.G != 249 || pixel.B != 252) ++nonBackgroundPixels;
        if (pixel.A > 0 && pixel.R < 100 && pixel.G < 100 && pixel.B < 100) ++glyphPixels;
    }
    var capturePath = Environment.GetEnvironmentVariable("GUI_FORMS_CAPTURE_MENU");
    if (!string.IsNullOrEmpty(capturePath)) raster.Save(capturePath);
    Console.WriteLine($"menu-raster=glyph-pixels:{glyphPixels}|non-background:{nonBackgroundPixels}|capture:{capturePath ?? "none"}");
    Require(glyphPixels >= 3 && nonBackgroundPixels >= 40, "menu text glyph raster");
    menu.ReleaseAt(50, 40);
    Require(menu.Visible && source.DropDown.Visible && source.DropDown.Parent == form,
        "nested menu retained with parent open");
    Require(source.DropDown.Left >= menu.Right - 3,
        "nested menu placed beside parent");
    typeof(ToolStrip).GetMethod("OnMouseUp",
        global::System.Reflection.BindingFlags.Instance | global::System.Reflection.BindingFlags.NonPublic)!
        .Invoke(source.DropDown, [new MouseEventArgs(MouseButtons.Left, 1, 50, 12, 0)]);
    Require(nestedClicked == 1 && !source.DropDown.Visible && source.DropDown.Parent is null &&
        !menu.Visible && menu.Parent is null, "nested leaf activation closes menu chain");
    menu.Show(form, new Point(24, 36));
    menu.ReleaseAt(50, 12);
    Require(clicked == 1 && !menu.Visible && menu.Parent is null,
        "root leaf activation and dismissal");

    menu.Show(form, new Point(form.Width - 8, 36));
    Require(menu.Right == form.Width, "root menu clamps to retained form edge");
    menu.Key(Keys.End);
    menu.Key(Keys.Right);
    Require(source.DropDown.Visible && source.DropDown.Right <= menu.Left,
        "keyboard branch opens with leftward edge reversal");
    PressKey(source.DropDown, Keys.Left);
    Require(menu.Visible && !source.DropDown.Visible,
        "left closes one keyboard menu level");
    menu.Key(Keys.End);
    menu.Key(Keys.Right);
    PressKey(source.DropDown, Keys.Enter);
    Require(nestedClicked == 2 && !menu.Visible,
        "keyboard nested activation closes complete chain");

    menu.Show(form, new Point(24, 36));
    menu.Key(Keys.Home);
    menu.Key(Keys.Enter);
    Require(clicked == 2 && !menu.Visible, "keyboard root activation");
    menu.Show(form, new Point(24, 36));
    menu.Key(Keys.Escape);
    Require(!menu.Visible && menu.Parent is null, "escape dismisses root menu");

    var baselineChildren = form.Controls.Count;
    for (var cycle = 0; cycle < 24; ++cycle)
    {
        menu.Show(form, new Point(24, 36));
        Require(form.Controls.Count == baselineChildren + 1, "one retained popup per cycle");
        menu.Key(Keys.Escape);
        Require(form.Controls.Count == baselineChildren && menu.Parent is null,
            "popup cycle detaches without retained children");
    }
    Console.WriteLine($"menu=bounds:{menu.Bounds.X},{menu.Bounds.Y},{menu.Width},{menu.Height}|items:{menu.Items.Count}|nested:{source.DropDownItems.Count}|nested-clicked:{nestedClicked}|clicked:{clicked}|glyph-pixels:{glyphPixels}|keyboard:pass|edge:left|cycles:24");
    menu.Dispose();
    form.Dispose();
    return 0;
}

static int RunComboHost()
{
    var form = new Form { Name = "comboForm", Size = new Size(360, 180) };
    var combo = new ComboProbe
    {
        Name = "serverUri",
        Bounds = new Rectangle(20, 20, 240, 24),
        DropDownStyle = ComboBoxStyle.DropDownList,
    };
    combo.Items.AddRange(["sdr://airspy.com:5555", "sdr://localhost:5555"]);
    combo.SelectedIndex = 0;
    var capabilityCombo = new ComboBox();
    capabilityCombo.Items.AddRange(["660 kHz", "330 kHz"]);
    capabilityCombo.SelectedIndex = 1;
    var capabilitySelectionEvents = 0;
    capabilityCombo.SelectedIndexChanged += (_, _) => ++capabilitySelectionEvents;
    capabilityCombo.Items.Clear();
    Require(capabilityCombo.SelectedIndex == -1 && capabilitySelectionEvents == 0,
        "combo item reset suppresses transient selection event");
    capabilityCombo.Items.AddRange(["660 kHz", "330 kHz"]);
    capabilityCombo.SelectedIndex = 0;
    Require(capabilitySelectionEvents == 1,
        "combo explicit post-reset selection event");
    var opened = 0;
    var closed = 0;
    combo.DropDown += (_, _) => ++opened;
    combo.DropDownClosed += (_, _) => ++closed;
    form.Controls.Add(combo);

    var editable = new ComboBox { Text = "sdr://old.example:5555" };
    var comboKeyInput = typeof(ComboBox).GetMethod("__NativeKeyInput",
        global::System.Reflection.BindingFlags.Instance | global::System.Reflection.BindingFlags.NonPublic)!;
    var comboTextInput = typeof(ComboBox).GetMethod("__NativeTextInput",
        global::System.Reflection.BindingFlags.Instance | global::System.Reflection.BindingFlags.NonPublic)!;
    comboTextInput.Invoke(editable, ["x", false, -1, 0]);
    Require(editable.Text == "sdr://old.example:5555x", "editable combo caret defaults to end");
    comboKeyInput.Invoke(editable, [0x04u, true, 2u, false]);
    comboTextInput.Invoke(editable, ["sdr://new.example:5555", false, -1, 0]);
    Require(editable.Text == "sdr://new.example:5555", "editable combo control-a replacement");
    comboKeyInput.Invoke(editable, [0x2au, true, 0u, false]);
    Require(editable.Text == "sdr://new.example:555", "editable combo backspace");

    combo.ReleaseAt(combo.Width - 6, combo.Height / 2);
    ContextMenuStrip? dropDown = null;
    foreach (Control child in form.Controls)
        if (child is ContextMenuStrip candidate) dropDown = candidate;
    Require(dropDown is not null && dropDown.Visible && dropDown.Parent == form,
        "combo retained owned drop-down");
    var activeDropDown = dropDown ?? throw new InvalidOperationException("combo drop-down missing");
    typeof(ToolStrip).GetMethod("OnMouseUp",
        global::System.Reflection.BindingFlags.Instance | global::System.Reflection.BindingFlags.NonPublic)!
        .Invoke(activeDropDown, [new MouseEventArgs(MouseButtons.Left, 1, 50, 34, 0)]);
    Require(combo.SelectedIndex == 1 && combo.Text == "sdr://localhost:5555",
        "combo drop-down selection");
    Require(opened == 1 && closed == 1 && !activeDropDown.Visible && activeDropDown.Parent is null,
        "combo drop-down lifecycle");
    Console.WriteLine($"combo=opened:{opened}|closed:{closed}|selected:{combo.SelectedIndex}|text:{combo.Text}|editable:{editable.Text}|reset-events:{capabilitySelectionEvents}|retained:true");
    capabilityCombo.Dispose();
    editable.Dispose();
    form.Dispose();
    return 0;
}

static int RunCrossThreadFontHost()
{
    Exception? workerFailure = null;
    var worker = new Thread(() =>
    {
        try
        {
            using var bitmap = new Bitmap(8, 8);
            using var graphics = Graphics.FromImage(bitmap);
            graphics.Clear(Color.White);
            using var stream = new System.IO.MemoryStream();
            bitmap.Save(stream, System.Drawing.Imaging.ImageFormat.Png);
        }
        catch (Exception error) { workerFailure = error; }
    });
    worker.Start();
    worker.Join();
    if (workerFailure is not null) throw workerFailure;

    using var raster = new Bitmap(180, 32);
    using (var graphics = Graphics.FromImage(raster))
    using (var font = new Font("Portsmouth Rapids", 12f, FontStyle.Regular, GraphicsUnit.Pixel, 1))
    using (var ink = new SolidBrush(Color.FromArgb(31, 37, 44)))
    {
        graphics.Clear(Color.White);
        graphics.DrawString("AIRSPY Server", font, ink, 3f, 3f);
    }
    var darkPixels = 0;
    for (var y = 0; y < raster.Height; ++y)
    for (var x = 0; x < raster.Width; ++x)
    {
        var pixel = raster.GetPixel(x, y);
        if (pixel.R < 120 && pixel.G < 120 && pixel.B < 120) ++darkPixels;
    }
    Require(darkPixels >= 8, "thread-local raster font registration");
    Console.WriteLine($"font-thread=worker-init:true|ui-glyph-pixels:{darkPixels}");
    return 0;
}

static int RunSliderLiveHost()
{
    var form = new Form
    {
        Name = "sliderLiveForm",
        Text = "GUI.Forms drag gate",
        Size = new Size(420, 170),
    };
    var slider = new DragSliderProbe
    {
        Name = "dragSlider",
        Bounds = new Rectangle(30, 45, 330, 34),
    };
    form.Controls.Add(slider);
    Application.Run(form);
    Require(slider.MouseDowns == 1 && slider.MouseUps == 1,
        "slider receives one drag boundary");
    Require(slider.DragMoves >= 2 && slider.Value >= 90,
        "held-button moves drive slider to the requested end");
    Require(!slider.Capture, "slider releases managed capture");
    Console.WriteLine($"slider-live=value:{slider.Value}|down:{slider.MouseDowns}|moves:{slider.DragMoves}|up:{slider.MouseUps}|capture:{slider.Capture}");
    form.Dispose();
    return 0;
}

static int RunMessageBoxHost()
{
    var result = MessageBox.Show("Retained GUI.Forms message", "GUI.Forms",
        MessageBoxButtons.OKCancel, MessageBoxIcon.Information,
        MessageBoxDefaultButton.Button1);
    Require(result == DialogResult.OK, "message box first-button activation");
    Console.WriteLine($"messagebox=result:{result}|retained:true");
    return 0;
}

static int RunNativeDialogLiveHost()
{
    var form = new Form { Name = "nativeDialogForm", Text = "GUI.Forms dialog gate", Size = new Size(420, 170) };
    var open = new Button { Name = "openPicker", Text = "Open file picker", Bounds = new Rectangle(24, 42, 150, 30) };
    var result = DialogResult.None;
    open.Click += (_, _) =>
    {
        using var picker = new OpenFileDialog
        {
            Filter = "Images (*.png)|*.png|All files (*.*)|*.*",
            InitialDirectory = Environment.CurrentDirectory,
        };
        result = picker.ShowDialog(form);
        form.Close();
    };
    form.Controls.Add(open);
    Application.Run(form);
    Console.WriteLine($"dialog-live=result:{result}|provider:native");
    form.Dispose();
    return 0;
}

static int RunTooltipLiveHost()
{
    var form = new Form { Name = "tooltipForm", Text = "GUI.Forms tooltip gate", Size = new Size(420, 170) };
    var target = new Button { Name = "tooltipTarget", Text = "Hover for details", Bounds = new Rectangle(24, 42, 150, 30) };
    using var tooltip = new ToolTip
    {
        InitialDelay = 100,
        AutoPopDelay = 3000,
        ReshowDelay = 50,
    };
    tooltip.SetToolTip(target, "Rendered by the GUI.Forms non-activating popup adapter.");
    form.Controls.Add(target);
    Application.Run(form);
    form.Dispose();
    return 0;
}

static int RunNativeFolderDialogLiveHost()
{
    var form = new Form { Name = "nativeFolderForm", Text = "GUI.Forms folder gate", Size = new Size(420, 170) };
    var open = new Button { Name = "openFolderPicker", Text = "Select folder", Bounds = new Rectangle(24, 42, 150, 30) };
    var result = DialogResult.None;
    open.Click += (_, _) =>
    {
        using var picker = new FolderBrowserDialog { SelectedPath = Environment.CurrentDirectory };
        result = picker.ShowDialog(form);
        form.Close();
    };
    form.Controls.Add(open);
    Application.Run(form);
    Console.WriteLine($"folder-live=result:{result}|provider:native");
    form.Dispose();
    return 0;
}

static int RunOverlayMenuHost()
{
    var form = new Form { Name = "overlayMenuForm", Text = "Retained menu overlay", Size = new Size(360, 220) };
    var open = new Button { Name = "openMenuButton", Text = "Menu", Bounds = new Rectangle(20, 20, 90, 28) };
    var continueButton = new Button { Name = "continueButton", Text = "Continue", Bounds = new Rectangle(130, 20, 100, 28) };
    var menu = new ContextMenuStrip { Name = "overlayMenu" };
    var source = new ToolStripMenuItem("Source");
    source.DropDownItems.Add(new ToolStripMenuItem("AIRSPY Server Network"));
    var childShows = 0;
    var childHides = 0;
    source.DropDown.VisibleChanged += (_, _) =>
    {
        if (source.DropDown.Visible) ++childShows;
        else ++childHides;
    };
    menu.Items.Add(source);
    menu.Items.Add(new ToolStripMenuItem("Radio"));
    var showReturned = false;
    var continued = 0;
    open.Click += (_, _) => { menu.Show(open, 0, open.Height); showReturned = true; };
    continueButton.Click += (_, _) =>
    {
        Require(childShows == 1 && childHides >= 1 && !menu.Visible && menu.Parent is null &&
            !source.DropDown.Visible && source.DropDown.Parent is null,
            "menu chain dismisses before target click");
        ++continued;
        form.Close();
    };
    form.Controls.Add(open);
    form.Controls.Add(continueButton);
    Application.Run(form);
    Require(showReturned && continued == 1, "modeless retained menu overlay");
    Console.WriteLine($"menu-live=returned:{showReturned}|continued:{continued}|keyboard-child:{childShows}/{childHides}|outside-dismissed:{!menu.Visible && menu.Parent is null}");
    menu.Dispose();
    form.Dispose();
    return 0;
}

static int RunHoverMenuHost()
{
    var form = new Form { Name = "hoverMenuForm", Text = "Retained hover menu", Size = new Size(360, 220) };
    var menu = new MenuProbe { Name = "hoverMenu" };
    menu.Items.Add(new ToolStripMenuItem("Connect"));
    menu.Items.Add(new ToolStripSeparator());
    var source = new ToolStripMenuItem("Source");
    source.DropDownItems.Add(new ToolStripMenuItem("AIRSPY Server Network"));
    menu.Items.Add(source);
    var stage = 0;
    using var timer = new System.Windows.Forms.Timer { Interval = 550 };
    timer.Tick += (_, _) =>
    {
        if (stage == 0)
        {
            Require(source.DropDown.Visible, "hover delay opens branch");
            Require(source.DropDown.Right <= menu.Left, "hover branch reverses at retained edge");
            menu.MoveAt(50, 12);
            stage = 1;
            return;
        }
        Require(!source.DropDown.Visible, "hovering a leaf retires prior branch after delay");
        timer.Stop();
        menu.Key(Keys.Escape);
        form.Close();
        stage = 2;
    };
    form.Load += (_, _) =>
    {
        menu.Show(form, new Point(form.Width - 8, 36));
        menu.MoveAt(50, 40);
        timer.Start();
    };
    Application.Run(form);
    Require(stage == 2 && !menu.Visible && menu.Parent is null,
        "hover menu lifecycle completes");
    Console.WriteLine("menu-hover=open:delayed|close:delayed|edge:left|lifecycle:clean");
    menu.Dispose();
    form.Dispose();
    return 0;
}

static int RunPopupRepeatHost()
{
    const int cycles = 12;
    var loads = 0;
    var clicks = 0;
    for (var cycle = 0; cycle < cycles; ++cycle)
    {
        using var dialog = new Form
        {
            Name = "repeatDialog" + cycle,
            Text = "Popup lifecycle " + cycle,
            Size = new Size(320, 160),
        };
        using var popup = new MenuProbe { Name = "repeatPopup" + cycle };
        var close = new ToolStripMenuItem("Close");
        close.Click += (_, _) => ++clicks;
        popup.Items.Add(close);
        dialog.Load += (_, _) =>
        {
            ++loads;
            popup.Show(dialog, new Point(20, 28));
            popup.ReleaseAt(50, 12);
            Require(!popup.Visible && popup.Parent is null,
                "popup detaches before repeated dialog close");
            dialog.BeginInvoke((Action)dialog.Close);
        };
        _ = dialog.ShowDialog();
        Require(!dialog.Visible && popup.Parent is null,
            "repeated dialog and popup close cleanly");
    }
    Require(loads == cycles && clicks == cycles,
        "repeated popup/dialog callbacks exactly once");
    Console.WriteLine($"popup-repeat=cycles:{cycles}|loads:{loads}|clicks:{clicks}|retained-orphans:0");
    return 0;
}

static void PressKey(Control control, Keys key) =>
    typeof(Control).GetMethod("OnKeyDown",
        global::System.Reflection.BindingFlags.Instance |
        global::System.Reflection.BindingFlags.NonPublic)!
        .Invoke(control, [new KeyEventArgs(key)]);

static int RunLoadLifecycle()
{
    var form = new Form { Name = "loadForm", Size = new Size(360, 180) };
    var initial = new LoadProbe();
    LoadProbe? late = null;
    var formLoads = 0;
    form.Controls.Add(initial);
    form.Load += (_, _) =>
    {
        ++formLoads;
        late = new LoadProbe();
        form.Controls.Add(late);
    };
    Application.Run(form);
    Require(formLoads == 1, "form load once");
    Require(initial.Loads == 1, "initial child load once");
    var attachedLate = late ?? throw new InvalidOperationException("M11d behavior check failed: late child created");
    Require(attachedLate.Loads == 1, "late child load once");
    form.Show();
    Require(formLoads == 1 && initial.Loads == 1 && attachedLate.Loads == 1, "load idempotence");
    Console.WriteLine($"lifecycle=form:{formLoads}|initial:{initial.Loads}|late:{attachedLate.Loads}");
    form.Dispose();
    return 0;
}

static int RunPointerHost()
{
    var form = new Form { Name = "pointerForm", Size = new Size(360, 180) };
    var probe = new PaintInputProbe { Dock = DockStyle.Fill };
    form.Controls.Add(probe);
    Application.Run(form);
    Require(probe.Paints > 0, "owner paint lifecycle");
    Require(probe.MouseDowns == 1 && probe.MouseUps == 1, "owner pointer delivery");
    Require(probe.LastPoint.X >= 0 && probe.LastPoint.Y >= 0, "owner pointer coordinates");
    Console.WriteLine($"pointer=paint:{probe.Paints}|down:{probe.MouseDowns}|up:{probe.MouseUps}|point:{probe.LastPoint.X},{probe.LastPoint.Y}");
    form.Dispose();
    return 0;
}

static int RunRasterButtonHost()
{
    var form = new Form { Name = "buttonForm", Size = new Size(360, 180) };
    var button = new Button
    {
        Name = "imageButton",
        Bounds = new Rectangle(20, 20, 80, 40),
        Text = "Run",
    };
    var clicks = 0;
    button.Click += (_, _) => ++clicks;
    form.Controls.Add(button);
    Application.Run(form);
    Require(clicks == 1, "raster button click delivery");
    Console.WriteLine($"button=click:{clicks}|surface:raster");
    form.Dispose();
    return 0;
}

static int RunDialogHost()
{
    var dialog = new Form { Name = "dialogForm", Text = "M11g dialog", Size = new Size(360, 180) };
    var map = new Control { Name = "dialogMap", Dock = DockStyle.Fill };
    dialog.Controls.Add(map);
    var loads = 0;
    var popupClicked = 0;
    ContextMenuStrip? popup = null;
    dialog.Load += (_, _) =>
    {
        ++loads;
        popup = new ContextMenuStrip { Name = "serverChoices" };
        var choice = new ToolStripMenuItem("Server A");
        choice.Click += (_, _) => ++popupClicked;
        popup.Items.Add(choice);
        popup.Show(new Point(24, 36));
        Require(popup.Parent == dialog && popup.Visible, "unowned popup attaches to active dialog");
        typeof(ToolStrip).GetMethod("OnMouseUp",
            global::System.Reflection.BindingFlags.Instance | global::System.Reflection.BindingFlags.NonPublic)!
            .Invoke(popup, [new MouseEventArgs(MouseButtons.Left, 1, 50, 12, 0)]);
    };
    var result = dialog.ShowDialog();
    Require(loads == 1, "dialog load once");
    Require(!dialog.Visible && result == DialogResult.None, "dialog close result");
    Require(popupClicked == 1 && popup is not null && !popup.Visible && popup.Parent is null,
        "active dialog popup leaf dispatch and dismissal");
    Exception? refreshError = null;
    var refreshThread = new Thread(() =>
    {
        try { map.Refresh(); }
        catch (Exception error) { refreshError = error; }
    });
    refreshThread.Start();
    Require(refreshThread.Join(TimeSpan.FromSeconds(2)), "closed dialog refresh returns");
    Require(refreshError is null, $"closed dialog refresh no-op ({refreshError?.GetType().Name})");
    Console.WriteLine($"dialog=loads:{loads}|result:{result}|visible:{dialog.Visible}|popup-clicked:{popupClicked}|closed-refresh:no-op");
    popup?.Dispose();
    dialog.Dispose();
    return 0;
}

static void Require(bool condition, string name)
{
    if (!condition) throw new InvalidOperationException($"M11d behavior check failed: {name}");
}

sealed class LoadProbe : UserControl
{
    internal int Loads { get; private set; }
    protected override void OnLoad(EventArgs e)
    {
        ++Loads;
        base.OnLoad(e);
    }
}

sealed class LayoutProbe : Panel
{
    internal int Layouts { get; private set; }
    protected override void OnLayout(LayoutEventArgs e)
    {
        ++Layouts;
        base.OnLayout(e);
    }
}

sealed class StateChangeProbe : Control
{
    internal int TextChanges { get; private set; }
    internal int EnabledChanges { get; private set; }
    internal int VisibleChanges { get; private set; }
    internal int ForeColorChanges { get; private set; }
    internal int BackColorChanges { get; private set; }
    internal int FontChanges { get; private set; }

    protected override void OnTextChanged(EventArgs e) { ++TextChanges; base.OnTextChanged(e); }
    protected override void OnEnabledChanged(EventArgs e) { ++EnabledChanges; base.OnEnabledChanged(e); }
    protected override void OnVisibleChanged(EventArgs e) { ++VisibleChanges; base.OnVisibleChanged(e); }
    protected override void OnForeColorChanged(EventArgs e) { ++ForeColorChanges; base.OnForeColorChanged(e); }
    protected override void OnBackColorChanged(EventArgs e) { ++BackColorChanges; base.OnBackColorChanged(e); }
    protected override void OnFontChanged(EventArgs e) { ++FontChanges; base.OnFontChanged(e); }
}

sealed class MenuProbe : ContextMenuStrip
{
    internal void ReleaseAt(int x, int y) =>
        OnMouseUp(new MouseEventArgs(MouseButtons.Left, 1, x, y, 0));

    internal void MoveAt(int x, int y) =>
        OnMouseMove(new MouseEventArgs(MouseButtons.None, 0, x, y, 0));

    internal void Key(Keys key) => OnKeyDown(new KeyEventArgs(key));

    internal Bitmap RenderToBitmap()
    {
        var bitmap = new Bitmap(Width, Height);
        using var graphics = Graphics.FromImage(bitmap);
        var paintArgs = (PaintEventArgs)global::System.Activator.CreateInstance(
            typeof(PaintEventArgs),
            global::System.Reflection.BindingFlags.Instance |
                global::System.Reflection.BindingFlags.Public |
                global::System.Reflection.BindingFlags.NonPublic,
            null, [graphics, ClientRectangle], null)!;
        OnPaint(paintArgs);
        return bitmap;
    }
}

sealed class ComboProbe : ComboBox
{
    internal void ReleaseAt(int x, int y) =>
        OnMouseUp(new MouseEventArgs(MouseButtons.Left, 1, x, y, 0));
}

sealed class PaintInputProbe : Control
{
    internal int Paints { get; private set; }
    internal int MouseDowns { get; private set; }
    internal int MouseUps { get; private set; }
    internal Point LastPoint { get; private set; }

    protected override void OnPaint(PaintEventArgs e)
    {
        ++Paints;
        base.OnPaint(e);
    }

    protected override void OnMouseDown(MouseEventArgs e)
    {
        ++MouseDowns;
        LastPoint = new Point(e.X, e.Y);
        base.OnMouseDown(e);
    }

    protected override void OnMouseUp(MouseEventArgs e)
    {
        ++MouseUps;
        LastPoint = new Point(e.X, e.Y);
        base.OnMouseUp(e);
    }
}

sealed class NumericProbe : NumericUpDown
{
    internal void SpinAt(int x, int y) =>
        OnMouseUp(new MouseEventArgs(MouseButtons.Left, 1, x, y, 0));
    internal void WheelBy(int delta) =>
        OnMouseWheel(new MouseEventArgs(MouseButtons.None, 0, Width / 2, Height / 2, delta));
}

sealed class DragSliderProbe : Control
{
    internal int Value { get; private set; }
    internal int MouseDowns { get; private set; }
    internal int MouseUps { get; private set; }
    internal int DragMoves { get; private set; }
    private bool dragging;

    private void SetFromX(int x)
    {
        Value = Math.Clamp((int)Math.Round(Math.Clamp(x, 0, Width) * 100d /
            Math.Max(1, Width)), 0, 100);
        Invalidate();
    }

    protected override void OnMouseDown(MouseEventArgs e)
    {
        base.OnMouseDown(e);
        if (e.Button != MouseButtons.Left) return;
        ++MouseDowns;
        dragging = true;
        Capture = true;
        SetFromX(e.X);
    }

    protected override void OnMouseMove(MouseEventArgs e)
    {
        base.OnMouseMove(e);
        if (!dragging || e.Button != MouseButtons.Left) return;
        ++DragMoves;
        SetFromX(e.X);
    }

    protected override void OnMouseUp(MouseEventArgs e)
    {
        base.OnMouseUp(e);
        if (!dragging || e.Button != MouseButtons.Left) return;
        ++MouseUps;
        SetFromX(e.X);
        dragging = false;
        Capture = false;
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);
        using var track = new SolidBrush(Color.FromArgb(111, 126, 141));
        using var thumb = new SolidBrush(Color.FromArgb(64, 125, 190));
        e.Graphics.FillRectangle(track, 4, Height / 2 - 2, Math.Max(0, Width - 8), 4);
        var x = 4 + (int)Math.Round(Math.Max(0, Width - 16) * Value / 100d);
        e.Graphics.FillRectangle(thumb, x, 4, 12, Math.Max(1, Height - 8));
    }
}
