using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace CaptureFixture;

internal static class ExecutionTripwire
{
    [ModuleInitializer]
    internal static void Initialize()
    {
        var path = Environment.GetEnvironmentVariable("GUI_FORMS_CAPTURE_SENTINEL");
        if (!string.IsNullOrEmpty(path))
        {
            File.WriteAllText(path, "target code executed");
        }
    }
}

public class FixtureWindow : Form
{
    private readonly Button button = new();
    private readonly System.Windows.Forms.Timer timer = new();
    private readonly BindingBox<FixtureWindow> bindingBox;

    public FixtureWindow()
    {
        SuspendLayout();
        button.Text = "Capture";
        button.Click += HandleClick;
        timer.Interval = 25;
        timer.Tick += HandleTick;
        bindingBox = new BindingBox<FixtureWindow>(this);
        ResumeLayout(true);
    }

    public void Exercise()
    {
        timer.Start();
        button.PerformClick();
        _ = bindingBox.Value;
        _ = ShowDialog();
        _ = SendMessage(0, 0, 0, 0);
    }

    private void HandleClick(object? sender, EventArgs eventArgs) { }
    private void HandleTick(object? sender, EventArgs eventArgs) { }

    [DllImport("user32.dll", EntryPoint = "SendMessageW")]
    private static extern nint SendMessage(nint window, uint message, nuint wParam, nint lParam);
}

public sealed class DerivedFixtureWindow : FixtureWindow;
