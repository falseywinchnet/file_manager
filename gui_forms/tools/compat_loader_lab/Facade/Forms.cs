using System.Collections;
using System.Windows.Forms.Layout;

namespace System.Windows.Forms;

// Synthetic loader-laboratory surface only. This assembly does not enter the
// native build and is not the generated or supported GUI.Forms facade.
public class Control : IDisposable
{
    private readonly ControlCollection controls;

    public Control()
    {
        controls = new ControlCollection(this);
    }

    public string Name { get; set; } = string.Empty;
    public string Text { get; set; } = string.Empty;
    public object? Tag { get; set; }
    public bool Enabled { get; set; } = true;
    public bool Visible { get; set; } = true;
    public ControlCollection Controls => controls;
    public event EventHandler? Click;

    protected virtual void OnClick(EventArgs eventArgs) => Click?.Invoke(this, eventArgs);
    public virtual void Dispose() { }

    public class ControlCollection : ArrangedElementCollection, IList
    {
        private readonly List<Control> items = [];

        public ControlCollection(Control owner) => Owner = owner;
        public Control Owner { get; }
        public override int Count => items.Count;
        public bool IsReadOnly => false;
        public bool IsFixedSize => false;
        public bool IsSynchronized => false;
        public object SyncRoot => this;
        public object? this[int index]
        {
            get => items[index];
            set => items[index] = (Control)(value ?? throw new ArgumentNullException(nameof(value)));
        }

        public virtual void Add(Control value) => items.Add(value);
        int IList.Add(object? value)
        {
            Add((Control)(value ?? throw new ArgumentNullException(nameof(value))));
            return items.Count - 1;
        }
        public void Clear() => items.Clear();
        public bool Contains(object? value) => value is Control control && items.Contains(control);
        public int IndexOf(object? value) => value is Control control ? items.IndexOf(control) : -1;
        public void Insert(int index, object? value) =>
            items.Insert(index, (Control)(value ?? throw new ArgumentNullException(nameof(value))));
        public void Remove(object? value)
        {
            if (value is Control control)
            {
                items.Remove(control);
            }
        }
        public void RemoveAt(int index) => items.RemoveAt(index);
        public void CopyTo(Array array, int index) => ((ICollection)items).CopyTo(array, index);
        public IEnumerator GetEnumerator() => items.GetEnumerator();
    }
}

public class ButtonBase : Control { }

public class Button : ButtonBase
{
    public void PerformClick() => OnClick(EventArgs.Empty);
}

public class ScrollableControl : Control { }
public class ContainerControl : ScrollableControl { }

public class Form : ContainerControl
{
    public void Close() { }
}

public static class Application
{
    public static void Run(Form mainForm) =>
        ArgumentNullException.ThrowIfNull(mainForm);
}
