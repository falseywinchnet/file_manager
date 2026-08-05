using System;
using System.Collections;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;

if (args.Length == 1 && args[0] == "timer")
{
    return RunTimerHost();
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
Require(enabled.Checked && radio.Checked, "check state");
Require(checkEvents == 1 && selectionEvents == 1 && valueEvents == 1 && radioEvents == 1, "state events");
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

Console.WriteLine("surface=control,table,check,radio,combo,numeric,toolstrip,args");
Console.WriteLine($"events=check:{checkEvents}|selection:{selectionEvents}|value:{valueEvents}|radio:{radioEvents}|move:{moveEvents}|size:{sizeEvents}");
Console.WriteLine($"tree={form.Controls.Count}/{table.Controls.Count}|selected={mode.SelectedItem}|gain={gain.Value}|focus={gain.Focused}");

var disposalParent = new Panel();
var latePaintChild = new PaintInputProbe { Size = new Size(40, 20) };
disposalParent.Controls.Add(latePaintChild);
disposalParent.Dispose();
latePaintChild.Invalidate();

form.Dispose();
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
