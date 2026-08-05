using System;
using System.Drawing;
using System.Windows.Forms;

var form = new Form
{
    Name = "radioConsole",
    Text = "GUI.Forms Radio Console",
    Bounds = new Rectangle(40, 50, 960, 640),
};
var receiverPanel = new Panel { Name = "receiverPanel", Bounds = new Rectangle(16, 72, 440, 340) };
var statusPanel = new Panel { Name = "statusPanel", Bounds = new Rectangle(472, 72, 472, 340) };
var start = new Button { Name = "startButton", Text = "Start radio", Bounds = new Rectangle(20, 72, 116, 30) };
var stop = new Button { Name = "stopButton", Text = "Stop", Bounds = new Rectangle(146, 72, 86, 30) };
var heading = new Label { Name = "headingLabel", Text = "GUI.Forms Radio Console", Bounds = new Rectangle(20, 18, 360, 28) };
var subtitle = new Label { Name = "subtitleLabel", Text = "Portable retained surface · ABI 0.4", Bounds = new Rectangle(20, 45, 360, 20) };
var receiverTitle = new Label { Name = "receiverTitle", Text = "Receiver control", Bounds = new Rectangle(20, 18, 260, 26) };
var frequency = new Label { Name = "frequencyLabel", Text = "104.3 MHz  ·  WFM", Bounds = new Rectangle(20, 44, 240, 22) };
var noiseReduction = new CheckBox { Name = "noiseReduction", Text = "Noise reduction", Bounds = new Rectangle(20, 122, 180, 28) };
var statusTitle = new Label { Name = "statusTitle", Text = "Signal and session", Bounds = new Rectangle(20, 18, 260, 26) };
var signal = new Label { Name = "signalLabel", Text = "Signal  -42 dBFS", Bounds = new Rectangle(20, 58, 220, 22) };
var sampleRate = new Label { Name = "sampleRateLabel", Text = "Sample rate  2.4 MSPS", Bounds = new Rectangle(20, 88, 240, 22) };
var backend = new Label { Name = "backendLabel", Text = "Backend  Win32 DIB CPU", Bounds = new Rectangle(20, 118, 260, 22) };
var record = new Button { Name = "recordButton", Text = "Record", Bounds = new Rectangle(20, 168, 98, 30) };
var configure = new Button { Name = "configureButton", Text = "Configure…", Bounds = new Rectangle(128, 168, 112, 30) };
var footer = new Label { Name = "footerLabel", Text = "Ready · deterministic retained tree · renderer-neutral controls", Bounds = new Rectangle(20, 430, 600, 22) };
var visibleChanges = 0;
var textChanges = 0;
var nativeClicks = 0;
var queuedDispatches = 0;
var formClosing = 0;
var formClosed = 0;
var threadExceptions = 0;
var threadExit = 0;
start.VisibleChanged += (_, _) => ++visibleChanges;
start.TextChanged += (_, _) => ++textChanges;
start.Click += (_, _) =>
{
    ++nativeClicks;
    start.Text = "Receiver active";
    signal.Text = "Signal  -41 dBFS · live";
    _ = backend.BeginInvoke((Action)(() =>
    {
        ++queuedDispatches;
        backend.Text = "Backend  Win32 DIB CPU · dispatched";
        form.Close();
    }));
};
start.Click += (_, _) => throw new InvalidOperationException("contained demonstration callback fault");
form.FormClosing += (_, _) => ++formClosing;
form.FormClosed += (_, _) => ++formClosed;
Application.ThreadException += (_, _) => ++threadExceptions;

receiverPanel.Controls.Add(receiverTitle);
receiverPanel.Controls.Add(frequency);
receiverPanel.Controls.Add(start);
receiverPanel.Controls.Add(stop);
receiverPanel.Controls.Add(noiseReduction);
statusPanel.Controls.Add(statusTitle);
statusPanel.Controls.Add(signal);
statusPanel.Controls.Add(sampleRate);
statusPanel.Controls.Add(backend);
statusPanel.Controls.Add(record);
statusPanel.Controls.Add(configure);
form.Controls.Add(heading);
form.Controls.Add(subtitle);
form.Controls.Add(receiverPanel);
form.Controls.Add(statusPanel);
form.Controls.Add(footer);
start.Enabled = false;
start.Visible = false;
start.Text = "Start receiver";

Console.WriteLine($"facade={typeof(Form).Assembly.GetName().Name}");
Console.WriteLine($"tree={form.Controls[0].Name}/{receiverPanel.Controls[0].Name}");
Console.WriteLine($"form={form.Name}|{form.Text}|{form.Bounds.X},{form.Bounds.Y},{form.Bounds.Width},{form.Bounds.Height}");
Console.WriteLine($"button={start.Name}|{start.Text}|visible={start.Visible}|enabled={start.Enabled}|events={visibleChanges},{textChanges}");

start.Visible = true;
start.Enabled = true;
using var context = new ApplicationContext(form);
context.ThreadExit += (_, _) => ++threadExit;
Application.Run(context);
var hostKind = Application.LastHostTrace.Contains("win32-dib", StringComparison.Ordinal) ? "win32-dib" :
    Application.LastHostTrace.Contains("headless-reference", StringComparison.Ordinal) ? "headless-reference" :
    Application.LastHostTrace.Contains("appkit", StringComparison.OrdinalIgnoreCase) ? "appkit" : "portable-headless";
Console.WriteLine($"host={hostKind}|trace-bytes={Application.LastHostTrace.Length}");
Console.WriteLine($"callbacks=click:{nativeClicks}|dispatch:{queuedDispatches}|closing:{formClosing}|closed:{formClosed}|faults:{Application.CallbackFaultCount}|thread-exceptions:{threadExceptions}|thread-exit:{threadExit}");
Console.WriteLine($"live={start.Text}|{signal.Text}|{backend.Text}");

if (nativeClicks != 1 || queuedDispatches != 1 || formClosing != 1 ||
    formClosed != 1 || Application.CallbackFaultCount != 1 ||
    threadExceptions != 1 || threadExit != 1 ||
    Application.LastCallbackException is null)
{
    Console.WriteLine("m11c=FAILED");
    return 3;
}

form.Dispose();
try
{
    _ = form.Visible;
    Console.WriteLine("dispose=FAILED");
    return 2;
}
catch (InvalidOperationException)
{
    Console.WriteLine("dispose=stale-native-handle");
}

return 0;
