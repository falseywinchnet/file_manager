namespace System.Windows.Forms;

public class Control : IDisposable
{
    public string Text { get; set; } = string.Empty;
    public event EventHandler? Click;

    public virtual void Dispose() { }
    public void SuspendLayout() { }
    public void ResumeLayout(bool performLayout) { }

    protected void RaiseClick() => Click?.Invoke(this, EventArgs.Empty);
}

public class Form : Control
{
    public virtual int ShowDialog() => 1;
}

public class Button : Control
{
    public void PerformClick() => RaiseClick();
}

public sealed class Timer : IDisposable
{
    public int Interval { get; set; }
    public event EventHandler? Tick;

    public void Start() => Tick?.Invoke(this, EventArgs.Empty);
    public void Dispose() { }
}

public sealed class BindingBox<T>
{
    public BindingBox(T value) => Value = value;
    public T Value { get; }
}
