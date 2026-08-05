using System.Windows.Forms;

namespace GuiForms.LoaderConsumer;

public static class Probe
{
    public static string Run()
    {
        using var form = new Form { Name = "probe-form", Text = "GUI.Forms facade probe" };
        using var button = new Button { Name = "probe-button", Text = "Activate" };
        var clicks = 0;
        button.Click += (_, _) => ++clicks;
        form.Controls.Add(button);
        button.PerformClick();
        Application.Run(form);
        return $"clicks={clicks};controls={form.Controls.Count};text={button.Text}";
    }
}
