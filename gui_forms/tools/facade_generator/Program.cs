using System.Reflection;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

if (args.Length != 3)
{
    Console.Error.WriteLine("usage: gui-forms-facade-generator <catalogue.json> <windows-ref-dir> <output-dir>");
    return 2;
}

var cataloguePath = Path.GetFullPath(args[0]);
var windowsRef = Path.GetFullPath(args[1]);
var output = Path.GetFullPath(args[2]);
var coreRef = Directory.GetDirectories(Path.GetFullPath(Path.Combine(Path.GetDirectoryName(typeof(object).Assembly.Location)!, "..", "..", "..", "packs", "Microsoft.NETCore.App.Ref")))
    .OrderByDescending(value => value, StringComparer.Ordinal).Select(value => Path.Combine(value, "ref", "net10.0"))
    .First(Directory.Exists);
var resolverPaths = Directory.GetFiles(coreRef, "*.dll").Concat(Directory.GetFiles(windowsRef, "*.dll"))
    .Distinct(StringComparer.Ordinal).OrderBy(value => value, StringComparer.Ordinal).ToArray();
using var context = new MetadataLoadContext(new PathAssemblyResolver(resolverPaths), "System.Runtime");
var assemblies = new Dictionary<string, Assembly>(StringComparer.Ordinal)
{
    ["System.Windows.Forms"] = context.LoadFromAssemblyPath(Path.Combine(windowsRef, "System.Windows.Forms.dll")),
    ["System.Windows.Forms.Primitives"] = context.LoadFromAssemblyPath(Path.Combine(windowsRef, "System.Windows.Forms.Primitives.dll")),
    ["System.Drawing.Common"] = context.LoadFromAssemblyPath(Path.Combine(windowsRef, "System.Drawing.Common.dll")),
    ["System.Drawing.Primitives"] = context.LoadFromAssemblyPath(Path.Combine(coreRef, "System.Drawing.Primitives.dll")),
};

using var document = JsonDocument.Parse(File.ReadAllBytes(cataloguePath));
var rows = document.RootElement.GetProperty("apiRows").EnumerateArray()
    .Where(row => row.GetProperty("disposition").GetString() == "required" &&
                  row.GetProperty("implementationOwner").GetString() is
                      "gui_forms_managed_facade" or "gui_drawing_compat_facade")
    .Select(ApiRow.Read).OrderBy(row => row.Assembly, StringComparer.Ordinal)
    .ThenBy(row => row.Type, StringComparer.Ordinal).ThenBy(row => row.Member, StringComparer.Ordinal)
    .ThenBy(row => row.Signature, StringComparer.Ordinal).ToArray();

var selected = new Dictionary<Type, HashSet<MemberInfo>>();
var resolutions = new List<Resolution>();
var forwards = new Dictionary<string, HashSet<Type>>(StringComparer.Ordinal)
{
    ["System.Windows.Forms"] = [], ["System.Windows.Forms.Primitives"] = [],
    ["System.Drawing.Common"] = [], ["System.Drawing.Primitives"] = []
};

foreach (var row in rows)
{
    var type = assemblies[row.Assembly].GetType(row.Type, throwOnError: false, ignoreCase: false);
    if (type is null)
    {
        resolutions.Add(new(row.Id, row.Assembly, row.Type, row.Member, row.Signature, "unresolved_type", null));
        continue;
    }
    var actualAssembly = type.Assembly.GetName().Name!;
    if (actualAssembly != row.Assembly && forwards.ContainsKey(row.Assembly))
        forwards[row.Assembly].Add(type);
    AddType(type);
    if (row.MemberKind == "type")
    {
        resolutions.Add(new(row.Id, row.Assembly, row.Type, null, null, "resolved_type", actualAssembly));
        continue;
    }
    var member = ResolveMember(type, row);
    if (member is null)
    {
        resolutions.Add(new(row.Id, row.Assembly, row.Type, row.Member, row.Signature, "unresolved_member", actualAssembly));
        continue;
    }
    selected[type].Add(member);
    if (member is MethodInfo method)
    {
        AddSignatureType(method.ReturnType);
        foreach (var parameter in method.GetParameters()) AddSignatureType(parameter.ParameterType);
    }
    else if (member is ConstructorInfo constructor)
        foreach (var parameter in constructor.GetParameters()) AddSignatureType(parameter.ParameterType);
    else if (member is FieldInfo field) AddSignatureType(field.FieldType);
    resolutions.Add(new(row.Id, row.Assembly, row.Type, row.Member, row.Signature, "resolved_member", actualAssembly));
}

// Forms' first-party paint projection uses PNG explicitly even when the
// specimen catalogue only reaches ImageFormat indirectly through Image.Save.
var imageFormatType = assemblies["System.Drawing.Common"].GetType(
    "System.Drawing.Imaging.ImageFormat", throwOnError: true)!;
AddType(imageFormatType);
selected[imageFormatType].Add(imageFormatType.GetProperty("Png")!.GetMethod!);
var imageType = assemblies["System.Drawing.Common"].GetType(
    "System.Drawing.Image", throwOnError: true)!;
AddType(imageType);
selected[imageType].Add(imageType.GetMethods(BindingFlags.Public | BindingFlags.Instance)
    .Single(method => method.Name == "Save" && method.GetParameters() is var parameters &&
                      parameters.Length == 2 &&
                      parameters[0].ParameterType.FullName == "System.IO.Stream" &&
                      parameters[1].ParameterType.FullName == "System.Drawing.Imaging.ImageFormat"));

// .resources stores legacy Bitmap values with the ActivatorStream encoding.
// This constructor is reached reflectively by System.Resources.Extensions and
// therefore cannot appear as a static IL operand in the specimen catalogue.
var bitmapType = assemblies["System.Drawing.Common"].GetType(
    "System.Drawing.Bitmap", throwOnError: true)!;
AddType(bitmapType);
selected[bitmapType].Add(bitmapType.GetConstructors(BindingFlags.Public | BindingFlags.Instance)
    .Single(constructor => constructor.GetParameters() is var parameters &&
                           parameters.Length == 1 &&
                           parameters[0].ParameterType.FullName == "System.IO.Stream"));
var iconType = assemblies["System.Drawing.Common"].GetType(
    "System.Drawing.Icon", throwOnError: true)!;
AddType(iconType);
selected[iconType].Add(iconType.GetConstructors(BindingFlags.Public | BindingFlags.Instance)
    .Single(constructor => constructor.GetParameters() is var parameters &&
                           parameters.Length == 1 &&
                           parameters[0].ParameterType.FullName == "System.IO.Stream"));

void AddSignatureType(Type type)
{
    while (type.HasElementType) type = type.GetElementType()!;
    if (type.IsGenericType)
        foreach (var argument in type.GetGenericArguments()) AddSignatureType(argument);
    if (assemblies.ContainsKey(type.Assembly.GetName().Name!)) AddType(type);
}

void AddType(Type type)
{
    if (type.IsGenericType && !type.IsGenericTypeDefinition) type = type.GetGenericTypeDefinition();
    if (!selected.TryAdd(type, [])) return;
    if (type.DeclaringType is not null) AddType(type.DeclaringType);
    if (type.BaseType is { } baseType && assemblies.ContainsKey(baseType.Assembly.GetName().Name!))
        AddType(baseType);
    if (type.IsEnum)
        foreach (var field in type.GetFields(BindingFlags.Public | BindingFlags.Static)) selected[type].Add(field);
    if (type.BaseType?.FullName == "System.MulticastDelegate" && type.GetMethod("Invoke") is { } invoke)
    {
        selected[type].Add(invoke);
        AddSignatureType(invoke.ReturnType);
        foreach (var parameter in invoke.GetParameters()) AddSignatureType(parameter.ParameterType);
    }
    if (type.Assembly.GetName().Name == "System.Drawing.Common" &&
        type.GetInterfaces().Any(value => value.FullName == "System.IDisposable"))
    {
        var dispose = type.GetMethod("Dispose", BindingFlags.Public | BindingFlags.Instance,
                                     binder: null, Type.EmptyTypes, modifiers: null);
        if (dispose is not null && dispose.DeclaringType == type) selected[type].Add(dispose);
    }
}

Directory.CreateDirectory(output);
foreach (var assemblyName in assemblies.Keys.Where(name => name != "System.Drawing.Primitives"))
{
    var directory = Path.Combine(output, assemblyName);
    Directory.CreateDirectory(directory);
    var owned = selected.Keys.Where(type => type.Assembly.GetName().Name == assemblyName).ToHashSet();
    File.WriteAllText(Path.Combine(directory, "Surface.g.cs"), EmitAssembly(assemblyName, owned, selected), new UTF8Encoding(false));
    File.WriteAllText(Path.Combine(directory, assemblyName + ".csproj"), ProjectFile(assemblyName, windowsRef), new UTF8Encoding(false));
    if (assemblyName == "System.Windows.Forms")
        File.WriteAllText(Path.Combine(directory, "NativeBridge.g.cs"), NativeBridgeSource(), new UTF8Encoding(false));
    if (assemblyName == "System.Drawing.Common")
        File.WriteAllText(Path.Combine(directory, "DrawingBridge.g.cs"), DrawingBridgeSource(), new UTF8Encoding(false));
    if (forwards[assemblyName].Count != 0)
    {
        var source = new StringBuilder("using System.Runtime.CompilerServices;\n");
        foreach (var type in forwards[assemblyName].OrderBy(value => value.FullName, StringComparer.Ordinal))
            source.Append("[assembly: TypeForwardedTo(typeof(").Append(CsType(type)).AppendLine("))]");
        File.WriteAllText(Path.Combine(directory, "Forwards.g.cs"), source.ToString(), new UTF8Encoding(false));
    }
}

var manifest = new
{
    schema = "gui.forms.generated-surface/v1",
    toolVersion = "0.1.0",
    catalogue = Path.GetFileName(cataloguePath),
    requiredRowCount = rows.Length,
    resolvedRowCount = resolutions.Count(value => value.Status.StartsWith("resolved_", StringComparison.Ordinal)),
    unresolvedRowCount = resolutions.Count(value => value.Status.StartsWith("unresolved_", StringComparison.Ordinal)),
    emittedTypeCount = selected.Count,
    rows = resolutions,
};
File.WriteAllText(Path.Combine(output, "surface-manifest-v1.json"),
    JsonSerializer.Serialize(manifest, new JsonSerializerOptions { WriteIndented = true }) + "\n", new UTF8Encoding(false));
Console.WriteLine($"surface: {manifest.resolvedRowCount}/{manifest.requiredRowCount} rows resolved; {manifest.emittedTypeCount} types emitted");
return manifest.unresolvedRowCount == 0 ? 0 : 1;

static MemberInfo? ResolveMember(Type type, ApiRow row)
{
    const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly;
    IEnumerable<MemberInfo> candidates = row.MemberKind == "field"
        ? type.GetFields(flags).Where(field => field.Name == row.Member)
        : row.Member == ".ctor"
            ? type.GetConstructors(flags)
            : type.GetMethods(flags).Where(method => method.Name == row.Member);
    return candidates.FirstOrDefault(member => MemberSignature(member) == row.Signature);
}

static string MemberSignature(MemberInfo member) => member switch
{
    FieldInfo field => MetadataType(field.FieldType),
    MethodBase method => (method is MethodInfo info ? MetadataType(info.ReturnType) : "System.Void") + " " +
        (method.IsGenericMethodDefinition ? "``" + method.GetGenericArguments().Length : "") + "(" +
        string.Join(",", method.GetParameters().Select(parameter => MetadataType(parameter.ParameterType))) + ")",
    _ => throw new NotSupportedException(member.MemberType.ToString()),
};

static string MetadataType(Type type)
{
    if (type.IsByRef) return MetadataType(type.GetElementType()!) + "&";
    if (type.IsPointer) return MetadataType(type.GetElementType()!) + "*";
    if (type.IsArray) return MetadataType(type.GetElementType()!) + "[" + new string(',', type.GetArrayRank() - 1) + "]";
    if (type.IsGenericParameter) return (type.DeclaringMethod is null ? "!" : "!!") + type.GenericParameterPosition;
    if (type.IsGenericType)
        return type.GetGenericTypeDefinition().FullName! + "<" + string.Join(",", type.GetGenericArguments().Select(MetadataType)) + ">";
    return type.FullName ?? type.Name;
}

static string EmitAssembly(string assemblyName, HashSet<Type> owned, Dictionary<Type, HashSet<MemberInfo>> selected)
{
    var result = new StringBuilder("// <auto-generated/>\n#nullable enable\n#pragma warning disable 0067,0108,0109,0114,0169,0649,8600,8601,8602,8603,8618,8625\n");
    foreach (var group in owned.Where(type => type.DeclaringType is null).OrderBy(type => type.Namespace, StringComparer.Ordinal)
                 .ThenBy(type => type.Name, StringComparer.Ordinal).GroupBy(type => type.Namespace ?? ""))
    {
        if (group.Key.Length != 0) result.Append("namespace ").Append(group.Key).AppendLine(" {");
        foreach (var type in group) EmitType(result, type, owned, selected, group.Key.Length == 0 ? 0 : 1);
        if (group.Key.Length != 0) result.AppendLine("}");
    }
    if (assemblyName == "System.Windows.Forms")
    {
        result.AppendLine("namespace System.Windows.Forms {");
        result.AppendLine("    internal static class FacadeStubDiagnostics {");
        result.AppendLine("        private static readonly bool Trace = global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_STUBS\") == \"1\";");
        result.AppendLine("        internal static T Value<T>(string member) { if (Trace) global::System.Console.Error.WriteLine(\"facade-stub=\" + member); return default!; }");
        result.AppendLine("    }");
        result.AppendLine("    public class ApplicationContext : global::System.IDisposable {");
        result.AppendLine("        public ApplicationContext() { }");
        result.AppendLine("        public ApplicationContext(Form mainForm) { MainForm = mainForm ?? throw new global::System.ArgumentNullException(nameof(mainForm)); }");
        result.AppendLine("        public Form? MainForm { get; set; }");
        result.AppendLine("        public event global::System.EventHandler? ThreadExit;");
        result.AppendLine("        public void ExitThread() { MainForm?.Close(); }");
        result.AppendLine("        protected virtual void OnThreadExit(global::System.EventArgs e) { ThreadExit?.Invoke(this, e); }");
        result.AppendLine("        internal void __NotifyThreadExit() { OnThreadExit(global::System.EventArgs.Empty); }");
        result.AppendLine("        public void Dispose() { MainForm = null; global::System.GC.SuppressFinalize(this); }");
        result.AppendLine("    }");
        result.AppendLine("}");
    }
    else if (assemblyName == "System.Windows.Forms.Primitives")
    {
        result.AppendLine("namespace System.Windows.Forms {");
        result.AppendLine("    internal static class FacadeStubDiagnostics {");
        result.AppendLine("        private static readonly bool Trace = global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_STUBS\") == \"1\";");
        result.AppendLine("        internal static T Value<T>(string member) { if (Trace) global::System.Console.Error.WriteLine(\"facade-stub=\" + member); return default!; }");
        result.AppendLine("    }");
        result.AppendLine("}");
    }
    else
    {
        result.AppendLine("namespace System.Drawing {");
        result.AppendLine("    internal static class FacadeStubDiagnostics {");
        result.AppendLine("        private static readonly bool Trace = global::System.Environment.GetEnvironmentVariable(\"GUI_DRAWING_TRACE_STUBS\") == \"1\";");
        result.AppendLine("        internal static T Value<T>(string member) { if (Trace) global::System.Console.Error.WriteLine(\"drawing-facade-stub=\" + member); return default!; }");
        result.AppendLine("    }");
        result.AppendLine("    public sealed class ImageConverter : global::System.ComponentModel.TypeConverter {");
        result.AppendLine("        public override bool CanConvertFrom(global::System.ComponentModel.ITypeDescriptorContext? context, global::System.Type sourceType) => sourceType == typeof(byte[]) || base.CanConvertFrom(context, sourceType);");
        result.AppendLine("        public override object? ConvertFrom(global::System.ComponentModel.ITypeDescriptorContext? context, global::System.Globalization.CultureInfo? culture, object value) { if (value is byte[] bytes) return NativeDrawingBridge.DecodePng(bytes); return base.ConvertFrom(context, culture, value); }");
        result.AppendLine("    }");
        result.AppendLine("}");
    }
    return result.ToString();
}

static void EmitType(StringBuilder output, Type type, HashSet<Type> owned,
                     Dictionary<Type, HashSet<MemberInfo>> selected, int indent)
{
    var pad = new string(' ', indent * 4);
    if (type.IsEnum)
    {
        output.Append(pad).Append("public enum ").Append(CleanName(type)).Append(" : ").Append(CsType(Enum.GetUnderlyingType(type))).AppendLine(" {");
        foreach (var field in selected[type].OfType<FieldInfo>().OrderBy(field => field.MetadataToken))
            output.Append(pad).Append("    ").Append(Escape(field.Name)).Append(" = ").Append(Convert.ToString(field.GetRawConstantValue(), System.Globalization.CultureInfo.InvariantCulture)).AppendLine(",");
        output.Append(pad).AppendLine("}");
        return;
    }
    if (type.BaseType?.FullName is "System.MulticastDelegate")
    {
        var invoke = selected[type].OfType<MethodInfo>().Single(method => method.Name == "Invoke");
        output.Append(pad).Append("public delegate ").Append(CsType(invoke.ReturnType)).Append(' ').Append(CleanName(type))
            .Append('(').Append(Parameters(invoke.GetParameters(), false)).AppendLine(");");
        return;
    }
    var isStatic = type.IsAbstract && type.IsSealed;
    var kind = type.IsInterface ? "interface" : type.IsValueType ? "struct" : "class";
    if (type.FullName is "System.Drawing.Image" or "System.Drawing.Bitmap")
        output.Append(pad).AppendLine("[global::System.ComponentModel.TypeConverter(typeof(global::System.Drawing.ImageConverter))]");
    output.Append(pad).Append("public ");
    if (isStatic) output.Append("static "); else if (type.IsAbstract && !type.IsInterface) output.Append("abstract ");
    output.Append("partial ").Append(kind).Append(' ').Append(CleanName(type));
    var hasBase = !type.IsInterface && !type.IsValueType && !isStatic &&
        type.BaseType is { } baseType && baseType != typeof(object);
    if (hasBase) output.Append(" : ").Append(CsType(type.BaseType!));
    if (!type.IsInterface && !type.IsValueType && !isStatic &&
        type.Assembly.GetName().Name == "System.Drawing.Common" &&
        type.GetInterfaces().Any(value => value.FullName == "System.IDisposable"))
        output.Append(hasBase ? ", " : " : ").Append("global::System.IDisposable");
    if (type.FullName == "System.Drawing.Graphics")
        output.Append(", global::System.Drawing.IDeviceContext");
    if (type.FullName == "System.Windows.Forms.Control")
        output.Append(hasBase ? ", " : " : ").Append("global::System.Windows.Forms.IWin32Window");
    if (type.FullName is "System.Windows.Forms.UpDownBase" or
        "System.Windows.Forms.PictureBox" or
        "System.Windows.Forms.DataGridView" or
        "System.Windows.Forms.BindingSource")
        output.Append(", global::System.ComponentModel.ISupportInitialize");
    output.AppendLine(" {");

    if (type.FullName == "System.Windows.Forms.Control")
    {
        output.Append(pad).AppendLine("    private NativeControlBridge __native = null!;");
        output.Append(pad).AppendLine("    protected ControlCollection __controls = null!;");
        output.Append(pad).AppendLine("    private void __NativeChanged(NativeChange change) { if (change == NativeChange.Text) { if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_TEXT\") == \"1\" && Text.Length != 0) global::System.Console.Error.WriteLine(\"facade-text=type:\" + GetType().FullName + \"|name:\" + Name + \"|text:\" + Text.Replace('\\r', ' ').Replace('\\n', ' ')); OnTextChanged(global::System.EventArgs.Empty); if (__loadRaised) __QueueManagedPaint(); } else if (change == NativeChange.Visible) OnVisibleChanged(global::System.EventArgs.Empty); else if (change == NativeChange.Enabled) OnEnabledChanged(global::System.EventArgs.Empty); }");
        output.Append(pad).AppendLine("    internal virtual bool __NativeEvent(NativeEvent kind) { if (kind == NativeEvent.Clicked) Click?.Invoke(this, global::System.EventArgs.Empty); return false; }");
        output.Append(pad).AppendLine("    private void __NativePointer(NativePointer input) { var button = input.Button switch { 1u => MouseButtons.Left, 2u => MouseButtons.Right, 3u => MouseButtons.Middle, _ => MouseButtons.None }; var e = new MouseEventArgs(button, input.Kind is 6u or 7u ? 1 : 0, (int)global::System.Math.Round(input.X), (int)global::System.Math.Round(input.Y), (int)global::System.Math.Round(input.WheelDelta)); switch (input.Kind) { case 5u: OnMouseMove(e); break; case 6u: Application.__DismissActiveMenuForPointer(this); OnMouseDown(e); break; case 7u: OnMouseUp(e); break; case 8u: OnMouseWheel(e); break; case 9u: OnMouseEnter(global::System.EventArgs.Empty); break; case 10u: OnMouseLeave(global::System.EventArgs.Empty); break; } }");
        output.Append(pad).AppendLine("    internal string __RunNativeWindow() { return __native.RunWindow(global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_AUTOMATION_CLOSE\") == \"1\", global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_FORCE_HEADLESS\") == \"1\", global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_AUTOMATION_ACTIVATE\") == \"1\", false); }");
        output.Append(pad).AppendLine("    internal string __RunNativePopup() { return __native.RunWindow(false, global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_FORCE_HEADLESS\") == \"1\", false, true); }");
        output.Append(pad).AppendLine("    internal void __DoEvents() { NativeControlBridge.DoEvents(global::System.Environment.CurrentManagedThreadId); }");
        output.Append(pad).AppendLine("    internal void __RequestClose() { __native.RequestClose(); }");
        output.Append(pad).AppendLine("    internal bool __ShowPathDialog(uint kind, string title, string initialDirectory, string suggestedName, string defaultExtension, string filter, uint flags, out string selectedPath) { return __native.ShowPathDialog(kind, title, initialDirectory, suggestedName, defaultExtension, filter, flags, out selectedPath); }");
        output.Append(pad).AppendLine("    internal void __ShowToolTip(string text, int x, int y, int duration) { __native.ShowToolTip(text, x, y, duration); }");
        output.Append(pad).AppendLine("    internal void __HideToolTip() { __native.HideToolTip(); }");
        output.Append(pad).AppendLine("    internal uint __NativeCheckState { get { return __native.CheckState; } set { __native.CheckState = value; } }");
        output.Append(pad).AppendLine("    internal double __NativeRangeValue { get { return __native.RangeValue; } set { __native.RangeValue = value; } }");
        output.Append(pad).AppendLine("    private bool __loadRaised;");
        output.Append(pad).AppendLine("    internal static readonly bool __TraceLifecycle = global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_LIFECYCLE\") == \"1\";");
        output.Append(pad).AppendLine("    internal static readonly bool __TraceInteraction = global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_INTERACTION\") == \"1\";");
        output.Append(pad).AppendLine("    internal bool __HasRaisedLoad { get { return __loadRaised; } }");
        output.Append(pad).AppendLine("    internal static Control? __FocusedControl { get { return __focusedControl; } }");
        output.Append(pad).AppendLine("    internal bool __BeginLoad() { if (__loadRaised) return false; __loadRaised = true; return true; }");
        output.Append(pad).AppendLine("    internal void __RaiseChildrenLoad() { foreach (Control child in Controls) child.__RaiseLoad(); }");
        output.Append(pad).AppendLine("    internal void __DumpTree(int depth) { var table = this as TableLayoutPanel; global::System.Console.Error.WriteLine(\"facade-tree=\" + new string(' ', depth * 2) + GetType().FullName + \"|name=\" + Name + \"|bounds=\" + Bounds.X + \",\" + Bounds.Y + \",\" + Bounds.Width + \",\" + Bounds.Height + \"|dock=\" + Dock + \"|visible=\" + Visible + \"|autosize=\" + __autoSize + \"|raster=\" + __native.SupportsRaster + \"|children=\" + Controls.Count + (table is null ? string.Empty : table.__TableTrace())); foreach (Control child in Controls) child.__DumpTree(depth + 1); }");
        output.Append(pad).AppendLine("    internal virtual void __RaiseLoad() { if (!__BeginLoad()) return; __RaiseChildrenLoad(); __RenderManagedPaint(); }");
    }
    if (type.FullName == "System.Windows.Forms.Application")
    {
        output.Append(pad).AppendLine("    public static string LastHostTrace { get; private set; } = string.Empty;");
        output.Append(pad).AppendLine("    public static global::System.Exception? LastCallbackException { get; private set; }");
        output.Append(pad).AppendLine("    public static int CallbackFaultCount { get; private set; }");
        output.Append(pad).AppendLine("    public static event global::System.Threading.ThreadExceptionEventHandler? ThreadException;");
        output.Append(pad).AppendLine("    [global::System.ThreadStatic] private static ApplicationContext? __context;");
        output.Append(pad).AppendLine("    [global::System.ThreadStatic] internal static Form? __CurrentForm;");
        output.Append(pad).AppendLine("    [global::System.ThreadStatic] private static ToolStripDropDown? __activeMenu;");
        output.Append(pad).AppendLine("    private static readonly object __contextsGate = new();");
        output.Append(pad).AppendLine("    private static readonly global::System.Collections.Generic.Dictionary<int, ApplicationContext> __contexts = new();");
        output.Append(pad).AppendLine("    internal static void __RegisterMenu(ToolStripDropDown menu) { var root = menu.__MenuRoot; if (global::System.Object.ReferenceEquals(__activeMenu, root)) return; var previous = __activeMenu; __activeMenu = root; previous?.__CloseDropDown(); }");
        output.Append(pad).AppendLine("    internal static void __MenuClosed(ToolStripDropDown menu) { if (global::System.Object.ReferenceEquals(__activeMenu, menu)) __activeMenu = null; }");
        output.Append(pad).AppendLine("    internal static void __DismissActiveMenuForPointer(Control target) { var menu = __activeMenu; if (menu is not null && !menu.__ContainsMenuTarget(target)) menu.__CloseDropDown(); }");
        output.Append(pad).AppendLine("    internal static bool __DispatchKeyToActiveMenu(Control sender, uint physicalKey, bool down, uint modifiers, bool repeat) { var menu = __activeMenu; if (menu is null || menu.__ContainsMenuTarget(sender)) return false; menu.__KeyboardTarget.__DeliverNativeKey(physicalKey, down, modifiers, repeat); return true; }");
        output.Append(pad).AppendLine("    internal static void __CloseActiveMenu() { var menu = __activeMenu; __activeMenu = null; menu?.__CloseDropDown(); }");
        output.Append(pad).AppendLine("    internal static bool __Post(int ownerThreadId, global::System.Action action) { ApplicationContext? context; lock (__contextsGate) __contexts.TryGetValue(ownerThreadId, out context); var form = context?.MainForm; if (form is null) return false; try { _ = form.BeginInvoke(action); return true; } catch (global::System.Exception error) { __ReportCallbackException(error); return false; } }");
        output.Append(pad).AppendLine("    internal static void __ReportCallbackException(global::System.Exception error) { LastCallbackException = error; ++CallbackFaultCount; if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_CALLBACKS\") == \"1\") global::System.Console.Error.WriteLine(\"facade-callback-fault=\" + error.ToString().Replace('\\r', ' ').Replace('\\n', ' ')); try { ThreadException?.Invoke(null, new global::System.Threading.ThreadExceptionEventArgs(error)); } catch { } }");
        output.Append(pad).AppendLine("    private static void __RunContext(ApplicationContext context) { if (context is null) throw new global::System.ArgumentNullException(nameof(context)); if (__context is not null) throw new global::System.InvalidOperationException(\"A GUI.Forms application context is already running on this thread.\"); var form = context.MainForm ?? throw new global::System.InvalidOperationException(\"ApplicationContext.MainForm is required.\"); var ownerThreadId = global::System.Environment.CurrentManagedThreadId; LastHostTrace = string.Empty; LastCallbackException = null; CallbackFaultCount = 0; __context = context; __CurrentForm = form; lock (__contextsGate) __contexts.Add(ownerThreadId, context); try { _ = form.BeginInvoke((global::System.Action)form.__RaiseLoad); LastHostTrace = form.__RunNativeWindow(); } finally { __CloseActiveMenu(); lock (__contextsGate) __contexts.Remove(ownerThreadId); try { context.__NotifyThreadExit(); } catch (global::System.Exception error) { __ReportCallbackException(error); } __CurrentForm = null; __context = null; } }");
        output.Append(pad).AppendLine("    public static void Run(ApplicationContext context) { __RunContext(context); }");
    }
    if (type.FullName == "System.Windows.Forms.Form")
    {
        output.Append(pad).AppendLine("    internal override bool __NativeEvent(NativeEvent kind) { if (kind == NativeEvent.FormClosing) { var args = new FormClosingEventArgs(); FormClosing?.Invoke(this, args); return args.Cancel; } if (kind == NativeEvent.FormClosed) { FormClosed?.Invoke(this, new FormClosedEventArgs()); return false; } return base.__NativeEvent(kind); }");
        output.Append(pad).AppendLine("    internal override void __RaiseLoad() { if (!__BeginLoad()) return; if (__TraceLifecycle) global::System.Console.Error.WriteLine(\"facade-load=begin|type=\" + GetType().FullName); OnLoad(global::System.EventArgs.Empty); if (__TraceLifecycle) global::System.Console.Error.WriteLine(\"facade-load=end|type=\" + GetType().FullName); __RaiseChildrenLoad(); __RenderManagedPaint(); if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_TREE\") == \"1\") __DumpTree(0); }");
        output.Append(pad).AppendLine("    protected virtual void OnLoad(global::System.EventArgs e) { Load?.Invoke(this, e); }");
    }
    if (type.FullName == "System.Windows.Forms.CheckBox")
    {
        output.Append(pad).AppendLine("    internal override bool __NativeEvent(NativeEvent kind) { if (kind == NativeEvent.Clicked) { if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=checkbox-native-begin|name=\" + Name); CheckState = (CheckState)__NativeCheckState; if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=checkbox-native-end|name=\" + Name + \"|checked=\" + Checked); } return base.__NativeEvent(kind); }");
    }
    if (type.FullName == "System.Windows.Forms.Button")
    {
        output.Append(pad).AppendLine("    internal override bool __NativeEvent(NativeEvent kind) { if (kind == NativeEvent.Clicked && __buttonDialogResult != DialogResult.None && FindForm() is Form form) form.DialogResult = __buttonDialogResult; return base.__NativeEvent(kind); }");
        output.Append(pad).AppendLine("    protected override void OnMouseUp(MouseEventArgs e) { base.OnMouseUp(e); if (e.Button == MouseButtons.Left) __NativeEvent(NativeEvent.Clicked); }");
    }
    if (type.FullName == "System.Windows.Forms.RadioButton")
    {
        output.Append(pad).AppendLine("    internal override bool __NativeEvent(NativeEvent kind) { if (kind == NativeEvent.Clicked) { if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=radio-native-begin|name=\" + Name); if (Parent is not null) foreach (Control peer in Parent.Controls) if (peer is RadioButton radio && !global::System.Object.ReferenceEquals(radio, this)) radio.Checked = false; if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=radio-native-peers|name=\" + Name); Checked = __NativeCheckState != 0u; if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=radio-native-end|name=\" + Name + \"|checked=\" + Checked); } return base.__NativeEvent(kind); }");
    }
    if (type.FullName == "System.Windows.Forms.UserControl")
    {
        output.Append(pad).AppendLine("    internal override void __RaiseLoad() { if (!__BeginLoad()) return; if (__TraceLifecycle) global::System.Console.Error.WriteLine(\"facade-load=begin|type=\" + GetType().FullName); OnLoad(global::System.EventArgs.Empty); if (__TraceLifecycle) global::System.Console.Error.WriteLine(\"facade-load=end|type=\" + GetType().FullName); __RaiseChildrenLoad(); __RenderManagedPaint(); }");
    }
    if (type.FullName == "System.Windows.Forms.Control+ControlCollection")
    {
        output.Append(pad).AppendLine("    private readonly Control __owner;");
        output.Append(pad).AppendLine("    private readonly global::System.Collections.Generic.List<Control> __items = new();");
        output.Append(pad).AppendLine("    internal ControlCollection(Control owner) { __owner = owner; }");
        output.Append(pad).AppendLine("    public override int Count { get { return __items.Count; } }");
        output.Append(pad).AppendLine("    public override global::System.Collections.IEnumerator GetEnumerator() { return __items.GetEnumerator(); }");
        output.Append(pad).AppendLine("    internal void __BringToFront(Control child) { if (__items.Remove(child)) { __items.Add(child); __owner.__native.SetChildIndex(child.__native, 0); } }");
        output.Append(pad).AppendLine("    internal void __SendToBack(Control child) { if (__items.Remove(child)) { __items.Insert(0, child); __owner.__native.SetChildIndex(child.__native, global::System.Math.Max(0, __items.Count - 1)); } }");
    }

    EmitBehaviorMembers(output, type, pad);

    var members = selected[type];
    var methods = members.OfType<MethodInfo>().ToArray();
    var constructors = members.OfType<ConstructorInfo>().ToArray();
    var fields = members.OfType<FieldInfo>().ToArray();
    var accessorMethods = methods.Where(method => method.IsSpecialName).ToHashSet();
    foreach (var field in fields.Where(field => !type.IsEnum).OrderBy(field => field.MetadataToken))
    {
        output.Append(pad).Append("    public ");
        if (field.IsStatic) output.Append("static ");
        if (field.IsInitOnly) output.Append("readonly ");
        output.Append(CsType(field.FieldType)).Append(' ').Append(Escape(field.Name));
        if (field.IsStatic && field.IsInitOnly) output.Append(" = default;"); else output.Append(';');
        output.AppendLine();
    }
    if (!type.IsInterface && !type.IsValueType && !isStatic &&
        !constructors.Any(constructor => constructor.GetParameters().Length == 0))
        output.Append(pad).Append("    public ").Append(CleanName(type).Split('<')[0]).Append("() ")
            .AppendLine(ConstructorBody(type, null));
    foreach (var constructor in constructors.OrderBy(MemberSignature, StringComparer.Ordinal))
    {
        output.Append(pad).Append("    public ").Append(CleanName(type).Split('<')[0]).Append('(')
            .Append(Parameters(constructor.GetParameters(), true)).Append(") ").AppendLine(ConstructorBody(type, constructor));
    }
    foreach (var property in type.GetProperties(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly)
                 .Where(property => (property.GetMethod is not null && accessorMethods.Contains(property.GetMethod)) ||
                                    (property.SetMethod is not null && accessorMethods.Contains(property.SetMethod)))
                 .OrderBy(property => property.Name, StringComparer.Ordinal))
    {
        var access = property.GetMethod ?? property.SetMethod!;
        output.Append(pad).Append("    ").Append(Access(access)).Append(' ');
        if (access.IsStatic) output.Append("static ");
        else if (access.IsVirtual && !access.IsFinal && !type.IsValueType)
            output.Append(CanEmitOverride(type, access, selected) ? "override " : "virtual ");
        output.Append(CsType(property.PropertyType)).Append(' ');
        var index = property.GetIndexParameters();
        if (index.Length == 0) output.Append(Escape(property.Name));
        else output.Append("this[").Append(Parameters(index, false)).Append(']');
        output.Append(' ').AppendLine(PropertyBody(type, property, accessorMethods));
    }
    foreach (var evt in type.GetEvents(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly)
                 .Where(evt => (evt.AddMethod is not null && accessorMethods.Contains(evt.AddMethod)) ||
                               (evt.RemoveMethod is not null && accessorMethods.Contains(evt.RemoveMethod)))
                 .OrderBy(evt => evt.Name, StringComparer.Ordinal))
    {
        var access = evt.AddMethod ?? evt.RemoveMethod!;
        output.Append(pad).Append("    ").Append(Access(access)).Append(' ');
        if (access.IsStatic) output.Append("static ");
        else if (access.IsVirtual && !access.IsFinal)
            output.Append(CanEmitOverride(type, access, selected) ? "override " : "virtual ");
        output.Append("event ").Append(CsType(evt.EventHandlerType!)).Append(' ').Append(Escape(evt.Name)).AppendLine(";");
    }
    if (methods.Any(method => method.Name == "op_Inequality"))
    {
        var self = CsType(type);
        output.Append(pad).Append("    public static bool operator !=(").Append(self).Append(" left, ").Append(self).AppendLine(" right) { return !global::System.Object.ReferenceEquals(left, right); }");
        output.Append(pad).Append("    public static bool operator ==(").Append(self).Append(" left, ").Append(self).AppendLine(" right) { return global::System.Object.ReferenceEquals(left, right); }");
        output.Append(pad).AppendLine("    public override bool Equals(object? value) { return global::System.Object.ReferenceEquals(this, value); }");
        output.Append(pad).AppendLine("    public override int GetHashCode() { return global::System.Runtime.CompilerServices.RuntimeHelpers.GetHashCode(this); }");
    }
    foreach (var method in methods.Where(method => !method.IsSpecialName).OrderBy(MemberSignature, StringComparer.Ordinal))
    {
        output.Append(pad).Append("    ").Append(Access(method)).Append(' ');
        if (method.IsStatic) output.Append("static ");
        else if (type.FullName == "System.Windows.Forms.Control" && method.Name == "Dispose" && method.GetParameters().Length == 1) output.Append("override ");
        else if (method.IsVirtual && !method.IsFinal && !type.IsValueType)
            output.Append(CanEmitOverride(type, method, selected) ? "override " : "virtual ");
        output.Append(CsType(method.ReturnType)).Append(' ').Append(Escape(method.Name));
        if (method.IsGenericMethodDefinition)
            output.Append('<').Append(string.Join(",", method.GetGenericArguments().Select(arg => arg.Name))).Append('>');
        output.Append('(').Append(Parameters(method.GetParameters(), true)).Append(") ");
        output.AppendLine(type.IsInterface ? ";" : MethodBody(type, method));
    }
    foreach (var nested in owned.Where(candidate => candidate.DeclaringType == type).OrderBy(candidate => candidate.Name, StringComparer.Ordinal))
        EmitType(output, nested, owned, selected, indent + 1);
    output.Append(pad).AppendLine("}");
}

static void EmitBehaviorMembers(StringBuilder output, Type type, string pad)
{
    void Add(string value) => output.Append(pad).Append("    ").AppendLine(value);
    switch (type.FullName)
    {
        case "System.Drawing.Image":
            Add("internal NativeDrawingBridge.Handle __bitmap;");
            Add("internal int __width;");
            Add("internal int __height;");
            Add("internal global::System.Drawing.Imaging.PixelFormat __pixelFormat = global::System.Drawing.Imaging.PixelFormat.Format32bppPArgb;");
            Add("internal bool __imageDisposed;");
            Add("internal NativeDrawingBridge.Handle __BitmapHandle { get { return __bitmap; } }");
            break;
        case "System.Drawing.Icon":
            Add("internal byte[] __iconData = global::System.Array.Empty<byte>();");
            Add("internal bool __iconDisposed;");
            break;
        case "System.Drawing.Brush":
            Add("internal NativeDrawingBridge.Handle __handle;");
            Add("internal global::System.Drawing.Color __color;");
            Add("internal bool __brushDisposed;");
            break;
        case "System.Drawing.Pen":
            Add("internal NativeDrawingBridge.Handle __handle;");
            Add("internal bool __penDisposed;");
            break;
        case "System.Drawing.Font":
            Add("internal NativeDrawingBridge.Handle __handle;");
            Add("internal string __family = \"Lucida Grande\";");
            Add("internal float __size = 12f;");
            Add("internal global::System.Drawing.FontStyle __style;");
            Add("internal global::System.Drawing.GraphicsUnit __unit = global::System.Drawing.GraphicsUnit.Point;");
            Add("internal bool __fontDisposed;");
            break;
        case "System.Drawing.FontFamily":
            Add("internal string __name = \"Lucida Grande\";");
            break;
        case "System.Drawing.StringFormat":
            Add("internal NativeDrawingBridge.Handle __handle;");
            Add("internal global::System.Drawing.StringAlignment __alignment;");
            Add("internal global::System.Drawing.StringAlignment __lineAlignment;");
            Add("internal global::System.Drawing.StringTrimming __trimming;");
            Add("internal global::System.Drawing.StringFormatFlags __flags;");
            Add("internal bool __formatDisposed;");
            break;
        case "System.Drawing.Graphics":
            Add("internal NativeDrawingBridge.Handle __recorder;");
            Add("internal void __EnsureRecorder() { if (__recorder.IsNull) __recorder = NativeDrawingBridge.RecorderCreate(); }");
            Add("internal global::System.Drawing.Image? __target;");
            Add("internal nint __nativeSurface;");
            Add("internal uint __nativeSurfaceKind;");
            Add("internal nint __leasedHdc;");
            Add("internal ulong __hdcLeaseToken;");
            Add("internal bool __graphicsDisposed;");
            Add("internal global::System.Drawing.Drawing2D.SmoothingMode __smoothing;");
            Add("internal global::System.Drawing.Drawing2D.InterpolationMode __interpolation;");
            Add("internal global::System.Drawing.Drawing2D.PixelOffsetMode __pixelOffset;");
            Add("internal global::System.Drawing.Drawing2D.CompositingMode __compositing;");
            Add("internal global::System.Drawing.Drawing2D.CompositingQuality __compositingQuality;");
            Add("internal global::System.Drawing.Drawing2D.Matrix __transform = new global::System.Drawing.Drawing2D.Matrix();");
            break;
        case "System.Drawing.Drawing2D.GraphicsState":
            Add("internal ulong __token;");
            break;
        case "System.Drawing.Drawing2D.GraphicsPath":
            Add("internal global::System.Drawing.NativeDrawingBridge.Handle __handle;");
            Add("internal bool __pathDisposed;");
            break;
        case "System.Drawing.Drawing2D.Matrix":
            Add("internal float __m11 = 1f, __m12, __m21, __m22 = 1f, __dx, __dy;");
            break;
        case "System.Drawing.Imaging.ImageAttributes":
            Add("internal global::System.Drawing.NativeDrawingBridge.Handle __handle;");
            Add("internal bool __attributesDisposed;");
            break;
        case "System.Drawing.Region":
            Add("internal NativeDrawingBridge.Handle __handle;");
            Add("internal bool __regionDisposed;");
            break;
        case "System.Drawing.Drawing2D.ColorBlend":
            Add("internal global::System.Drawing.Color[] __colors = global::System.Array.Empty<global::System.Drawing.Color>();");
            Add("internal float[] __positions = global::System.Array.Empty<float>();");
            break;
        case "System.Drawing.Imaging.ColorMap":
            Add("internal global::System.Drawing.Color __oldColor;");
            Add("internal global::System.Drawing.Color __newColor;");
            break;
        case "System.Drawing.Imaging.ColorMatrix":
            Add("internal float[] __values = new float[] { 1,0,0,0,0, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,1,0, 0,0,0,0,1 };");
            break;
        case "System.Drawing.Imaging.BitmapData":
            Add("internal global::System.Drawing.NativeDrawingBridge.Handle __bitmap;");
            Add("internal ulong __token;");
            Add("internal nint __scan0;");
            Add("internal int __stride;");
            break;
        case "System.Drawing.Imaging.ImageFormat":
            Add("internal static readonly global::System.Drawing.Imaging.ImageFormat __png = new global::System.Drawing.Imaging.ImageFormat();");
            break;
        case "System.Windows.Forms.BaseCollection":
            Add("protected virtual global::System.Collections.IList __Collection { get { return global::System.Array.Empty<object>(); } }");
            break;
        case "System.Windows.Forms.Clipboard":
            Add("private static string __clipboardText = string.Empty;");
            break;
        case "System.Windows.Forms.CommonDialog":
            Add("internal virtual DialogResult __RunDialog(IWin32Window? owner) { if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_DIALOGS\") == \"1\") global::System.Console.Error.WriteLine(\"facade-dialog=cancel|type=\" + GetType().FullName + \"|reason=no-host-provider\"); return DialogResult.Cancel; }");
            break;
        case "System.Windows.Forms.FileDialog":
            Add("private string __defaultExt = string.Empty;");
            Add("protected string __fileName = string.Empty;");
            Add("private string __filter = string.Empty;");
            Add("private string __initialDirectory = string.Empty;");
            Add("private bool __restoreDirectory;");
            Add("internal override DialogResult __RunDialog(IWin32Window? owner) { var selected = global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_DIALOG_FILE\"); if (!global::System.String.IsNullOrWhiteSpace(selected)) { __fileName = selected; if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_DIALOGS\") == \"1\") global::System.Console.Error.WriteLine(\"facade-dialog=file|result=OK|path=\" + selected); return DialogResult.OK; } var host = owner as Control ?? Application.__CurrentForm; if (host is null) return base.__RunDialog(owner); var kind = this is SaveFileDialog ? 2u : 1u; var flags = this is SaveFileDialog ? 2u : 0u; if (host.__ShowPathDialog(kind, this is SaveFileDialog ? \"Save file\" : \"Open file\", __initialDirectory, __fileName, __defaultExt, __filter, flags, out selected)) { __fileName = selected; if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_DIALOGS\") == \"1\") global::System.Console.Error.WriteLine(\"facade-dialog=file|result=OK|path=\" + selected + \"|provider=native\"); return DialogResult.OK; } if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_DIALOGS\") == \"1\") global::System.Console.Error.WriteLine(\"facade-dialog=file|result=Cancel|provider=native\"); return DialogResult.Cancel; }");
            break;
        case "System.Windows.Forms.FolderBrowserDialog":
            Add("private string __selectedPath = string.Empty;");
            Add("internal override DialogResult __RunDialog(IWin32Window? owner) { var selected = global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_DIALOG_FOLDER\"); if (!global::System.String.IsNullOrWhiteSpace(selected)) { __selectedPath = selected; if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_DIALOGS\") == \"1\") global::System.Console.Error.WriteLine(\"facade-dialog=folder|result=OK|path=\" + selected); return DialogResult.OK; } var host = owner as Control ?? Application.__CurrentForm; if (host is null) return base.__RunDialog(owner); if (host.__ShowPathDialog(3u, \"Select folder\", __selectedPath, string.Empty, string.Empty, string.Empty, 0u, out selected)) { __selectedPath = selected; if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_DIALOGS\") == \"1\") global::System.Console.Error.WriteLine(\"facade-dialog=folder|result=OK|path=\" + selected + \"|provider=native\"); return DialogResult.OK; } if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_DIALOGS\") == \"1\") global::System.Console.Error.WriteLine(\"facade-dialog=folder|result=Cancel|provider=native\"); return DialogResult.Cancel; }");
            break;
        case "System.Windows.Forms.ColorDialog":
            Add("private bool __anyColor;");
            Add("private global::System.Drawing.Color __dialogColor = global::System.Drawing.Color.Black;");
            Add("private bool __fullOpen;");
            break;
        case "System.Windows.Forms.ToolTip":
            Add("private readonly global::System.Collections.Generic.Dictionary<Control, string> __toolTips = new();");
            Add("private bool __toolTipActive = true;");
            Add("private int __autoPopDelay = 5000;");
            Add("private int __initialDelay = 500;");
            Add("private int __reshowDelay = 100;");
            Add("private bool __showAlways;");
            Add("private IWin32Window? __shownWindow;");
            Add("private string __shownText = string.Empty;");
            Add("private readonly global::System.Collections.Generic.Dictionary<Control, (global::System.EventHandler Enter, global::System.EventHandler Leave)> __toolTipHandlers = new();");
            Add("private global::System.Threading.Timer? __toolTipTimer;");
            Add("private long __toolTipGeneration;");
            Add("private void __CancelPending(Control? control, bool hide) { global::System.Threading.Interlocked.Increment(ref __toolTipGeneration); var timer = global::System.Threading.Interlocked.Exchange(ref __toolTipTimer, null); timer?.Dispose(); if (hide && control is not null) { try { control.__HideToolTip(); } catch (global::System.InvalidOperationException) { } } }");
            Add("private void __Schedule(Control control, string text) { __CancelPending(control, true); if (!__toolTipActive || text.Length == 0 || (!__showAlways && !control.Enabled)) return; var generation = global::System.Threading.Interlocked.Read(ref __toolTipGeneration); __toolTipTimer = new global::System.Threading.Timer(_ => { try { _ = control.BeginInvoke((global::System.Action)(() => { if (generation != global::System.Threading.Interlocked.Read(ref __toolTipGeneration) || !__toolTipActive || control.IsDisposed) return; control.__ShowToolTip(text, 12, control.Height + 4, __autoPopDelay); __shownWindow = control; __shownText = text; })); } catch (global::System.InvalidOperationException) { } }, null, __initialDelay, global::System.Threading.Timeout.Infinite); }");
            break;
        case "System.Windows.Forms.MessageBox":
            Add("private static DialogResult __Show(string text, string caption, MessageBoxButtons buttons, MessageBoxIcon icon, MessageBoxDefaultButton defaultButton) { using var dialog = new Form { Text = caption ?? string.Empty, ClientSize = new global::System.Drawing.Size(420, 150), FormBorderStyle = FormBorderStyle.FixedDialog, MaximizeBox = false, MinimizeBox = false, ShowInTaskbar = false, StartPosition = FormStartPosition.CenterScreen }; var message = new Label { Text = text ?? string.Empty, Bounds = new global::System.Drawing.Rectangle(18, 18, 384, 72) }; dialog.Controls.Add(message); var choices = buttons switch { MessageBoxButtons.OKCancel => new[] { (\"OK\", DialogResult.OK), (\"Cancel\", DialogResult.Cancel) }, MessageBoxButtons.YesNo => new[] { (\"Yes\", DialogResult.Yes), (\"No\", DialogResult.No) }, MessageBoxButtons.YesNoCancel => new[] { (\"Yes\", DialogResult.Yes), (\"No\", DialogResult.No), (\"Cancel\", DialogResult.Cancel) }, MessageBoxButtons.RetryCancel => new[] { (\"Retry\", DialogResult.Retry), (\"Cancel\", DialogResult.Cancel) }, MessageBoxButtons.AbortRetryIgnore => new[] { (\"Abort\", DialogResult.Abort), (\"Retry\", DialogResult.Retry), (\"Ignore\", DialogResult.Ignore) }, MessageBoxButtons.CancelTryContinue => new[] { (\"Cancel\", DialogResult.Cancel), (\"Try Again\", DialogResult.TryAgain), (\"Continue\", DialogResult.Continue) }, _ => new[] { (\"OK\", DialogResult.OK) } }; var totalWidth = choices.Length * 88; var left = global::System.Math.Max(12, 420 - totalWidth - 12); for (var index = 0; index < choices.Length; ++index) { var choice = choices[index]; var button = new Button { Text = choice.Item1, Bounds = new global::System.Drawing.Rectangle(left + index * 88, 106, 80, 26), DialogResult = choice.Item2 }; dialog.Controls.Add(button); } var result = dialog.ShowDialog(); return result == DialogResult.None ? DialogResult.Cancel : result; }");
            break;
        case "System.Windows.Forms.DrawItemEventArgs":
            Add("internal global::System.Drawing.Rectangle __drawBounds;");
            Add("internal global::System.Drawing.Graphics? __drawGraphics;");
            Add("internal int __drawIndex = -1;");
            Add("internal DrawItemState __drawState;");
            break;
        case "System.Windows.Forms.ConvertEventArgs":
            Add("private object? __convertedValue;");
            break;
        case "System.Windows.Forms.CreateParams":
            Add("private int __extendedStyle;");
            break;
        case "System.Windows.Forms.Cursor":
            Add("private static Cursor? __current;");
            Add("private static global::System.Drawing.Point __position;");
            break;
        case "System.Windows.Forms.DateTimePicker":
            Add("private global::System.Drawing.Color __calendarForeColor;");
            Add("private global::System.Drawing.Color __calendarMonthBackground;");
            Add("private string __customFormat = string.Empty;");
            Add("private DateTimePickerFormat __dateTimeFormat;");
            Add("private global::System.DateTime __minimumDate = global::System.DateTime.MinValue;");
            Add("private global::System.DateTime __maximumDate = global::System.DateTime.MaxValue;");
            Add("private global::System.DateTime __dateTimeValue = global::System.DateTime.Now;");
            break;
        case "System.Windows.Forms.Screen":
            Add("private global::System.Drawing.Rectangle __screenBounds = new global::System.Drawing.Rectangle(0, 0, 1920, 1080);");
            break;
        case "System.Windows.Forms.FormClosingEventArgs":
            Add("private CloseReason __closeReason = CloseReason.UserClosing;");
            break;
        case "System.Windows.Forms.PreviewKeyDownEventArgs":
            Add("private Keys __previewKeyCode;");
            break;
        case "System.Windows.Forms.ToolStripItemRenderEventArgs":
            Add("internal global::System.Drawing.Graphics? __renderGraphics;");
            Add("internal ToolStripItem? __renderItem;");
            Add("internal ToolStrip? __renderToolStrip;");
            break;
        case "System.Windows.Forms.ToolStripItemImageRenderEventArgs":
            Add("internal global::System.Drawing.Image? __renderImage;");
            Add("internal global::System.Drawing.Rectangle __renderImageRectangle;");
            break;
        case "System.Windows.Forms.ToolStripItemTextRenderEventArgs":
            Add("internal string __renderText = string.Empty;");
            Add("internal global::System.Drawing.Font? __renderTextFont;");
            Add("internal TextFormatFlags __renderTextFormat;");
            Add("internal global::System.Drawing.Rectangle __renderTextRectangle;");
            break;
        case "System.Windows.Forms.ToolStripRenderEventArgs":
            Add("internal global::System.Drawing.Graphics? __stripGraphics;");
            Add("internal ToolStrip? __strip;");
            break;
        case "System.Windows.Forms.ToolStripArrowRenderEventArgs":
            Add("private global::System.Drawing.Color __arrowColor;");
            Add("internal ToolStripItem? __arrowItem;");
            break;
        case "System.Windows.Forms.ToolStripGripRenderEventArgs":
            Add("internal global::System.Drawing.Rectangle __gripBounds;");
            break;
        case "System.Windows.Forms.ToolStripSeparatorRenderEventArgs":
            Add("internal bool __verticalSeparator;");
            break;
        case "System.Windows.Forms.Control":
            Add("private static Control? __focusedControl;");
            Add("private static bool __checkForIllegalCrossThreadCalls;");
            Add("private Control? __parent;");
            Add("private int __tabIndex;");
            Add("private AnchorStyles __anchor = AnchorStyles.Top | AnchorStyles.Left;");
            Add("private DockStyle __dock;");
            Add("private Padding __margin = new Padding(3);");
            Add("private Padding __padding;");
            Add("private bool __autoSize;");
            Add("private bool __tabStop = true;");
            Add("private bool __allowDrop;");
            Add("private bool __doubleBuffered;");
            Add("private int __layoutSuspendDepth;");
            Add("private bool __performingLayout;");
            Add("private long __styles;");
            Add("private object? __tag;");
            Add("private global::System.Drawing.Color __backColor;");
            Add("private global::System.Drawing.Color __foreColor;");
            Add("private global::System.Drawing.Font? __font;");
            Add("private global::System.Drawing.Image? __backgroundImage;");
            Add("private ImageLayout __backgroundImageLayout;");
            Add("private Cursor? __cursor;");
            Add("private global::System.Drawing.Region? __region;");
            Add("private ContextMenuStrip? __contextMenuStrip;");
            Add("private RightToLeft __rightToLeft;");
            Add("private global::System.Drawing.Size __minimumSize;");
            Add("private global::System.Drawing.Size __maximumSize;");
            Add("internal bool __InheritsColors { get { return __foreColor.IsEmpty || __backColor.IsEmpty; } }");
            Add("internal global::System.Drawing.Image? __ProjectionBackgroundImage { get { return __backgroundImage; } }");
            Add("internal ImageLayout __ProjectionBackgroundImageLayout { get { return __backgroundImageLayout; } }");
            Add("private void __ApplyEffectiveColors() { __native.SetColors(ForeColor, BackColor); foreach (Control child in Controls) if (child.__InheritsColors) child.__ApplyEffectiveColors(); }");
            Add("private bool __renderingManagedPaint;");
            Add("private int __paintQueued;");
            Add("private static readonly global::System.Collections.Concurrent.ConcurrentDictionary<string, byte> __paintTrace = new();");
            Add("private static int __paintCaptureSequence;");
            Add("internal virtual void __NativeKeyInput(uint physicalKey, bool down, uint modifiers, bool repeat) { if (Application.__DispatchKeyToActiveMenu(this, physicalKey, down, modifiers, repeat)) return; __DeliverNativeKey(physicalKey, down, modifiers, repeat); }");
            Add("internal void __DeliverNativeKey(uint physicalKey, bool down, uint modifiers, bool repeat) { var key = physicalKey switch { >= 0x04u and <= 0x1du => (Keys)((int)Keys.A + (int)(physicalKey - 0x04u)), >= 0x1eu and <= 0x26u => (Keys)((int)Keys.D1 + (int)(physicalKey - 0x1eu)), 0x27u => Keys.D0, 0x28u => Keys.Enter, 0x29u => Keys.Escape, 0x2au => Keys.Back, 0x2bu => Keys.Tab, 0x2cu => Keys.Space, 0x4au => Keys.Home, 0x4bu => Keys.PageUp, 0x4du => Keys.End, 0x4eu => Keys.PageDown, 0x4fu => Keys.Right, 0x50u => Keys.Left, 0x51u => Keys.Down, 0x52u => Keys.Up, _ => Keys.None }; if ((modifiers & 1u) != 0) key |= Keys.Shift; if ((modifiers & 2u) != 0) key |= Keys.Control; if ((modifiers & 4u) != 0) key |= Keys.Alt; var e = new KeyEventArgs(key); if (down) OnKeyDown(e); else OnKeyUp(e); }");
            Add("internal virtual void __NativeTextInput(string text, bool composing, int replacementStart, int replacementLength) { }");
            Add("internal void __RenderManagedPaint() { if (!global::System.OperatingSystem.IsWindows()) return; __RenderManagedPaintWindows(); }");
            Add("private void __RenderManagedPaintNow() { global::System.Threading.Interlocked.Exchange(ref __paintQueued, 0); if (!__native.IsDisposed) __RenderManagedPaint(); }");
            Add("private void __QueueManagedPaint() { if (__native.IsDisposed) return; var form = FindForm(); if (form is not null && !form.Visible) { global::System.Threading.Interlocked.Exchange(ref __paintQueued, 0); return; } if (!__loadRaised || form is null) { __RenderManagedPaint(); return; } if (global::System.Threading.Interlocked.Exchange(ref __paintQueued, 1) != 0) return; try { _ = BeginInvoke((global::System.Action)__RenderManagedPaintNow); } catch { global::System.Threading.Interlocked.Exchange(ref __paintQueued, 0); throw; } }");
            Add("private void __RenderManagedPaintWindows() { if (__native.IsDisposed || !__native.SupportsRaster || __renderingManagedPaint || Width <= 0 || Height <= 0 || Width > 4096 || Height > 4096) return; __renderingManagedPaint = true; try { using var bitmap = new global::System.Drawing.Bitmap(Width, Height, global::System.Drawing.Imaging.PixelFormat.Format32bppPArgb); using (var graphics = global::System.Drawing.Graphics.FromImage(bitmap)) { graphics.Clear(BackColor); OnPaint(new PaintEventArgs(graphics, ClientRectangle)); } using var stream = new global::System.IO.MemoryStream(); bitmap.Save(stream, global::System.Drawing.Imaging.ImageFormat.Png); var png = stream.ToArray(); var captureDirectory = global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_CAPTURE_RASTER_DIR\"); if (this is ContextMenuStrip && !global::System.String.IsNullOrWhiteSpace(captureDirectory)) { global::System.IO.Directory.CreateDirectory(captureDirectory); var sequence = global::System.Threading.Interlocked.Increment(ref __paintCaptureSequence); var capturePath = global::System.IO.Path.Combine(captureDirectory, \"context-menu-\" + sequence.ToString(\"D4\", global::System.Globalization.CultureInfo.InvariantCulture) + \".png\"); global::System.IO.File.WriteAllBytes(capturePath, png); global::System.Console.Error.WriteLine(\"facade-paint-capture=\" + capturePath); } if (!__native.IsDisposed) __native.SetRaster(png); var typeName = GetType().FullName ?? GetType().Name; if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_PAINT\") == \"1\" && __paintTrace.TryAdd(typeName, 0)) global::System.Console.Error.WriteLine(\"facade-paint=type:\" + typeName + \"|size:\" + Width + \"x\" + Height + \"|png:\" + png.Length); } catch (global::System.Exception error) { Application.__ReportCallbackException(error); } finally { __renderingManagedPaint = false; } }");
            Add("private void __ApplyDockLayout() { var remaining = new global::System.Drawing.Rectangle(__padding.Left, __padding.Top, global::System.Math.Max(0, ClientSize.Width - __padding.Left - __padding.Right), global::System.Math.Max(0, ClientSize.Height - __padding.Top - __padding.Bottom)); for (var index = __controls.Count - 1; index >= 0; --index) { var child = __controls[index]; if (!child.Visible) continue; var bounds = child.Bounds; switch (child.Dock) { case DockStyle.Top: bounds = new global::System.Drawing.Rectangle(remaining.X, remaining.Y, remaining.Width, bounds.Height); remaining.Y += bounds.Height; remaining.Height = global::System.Math.Max(0, remaining.Height - bounds.Height); break; case DockStyle.Bottom: bounds = new global::System.Drawing.Rectangle(remaining.X, remaining.Bottom - bounds.Height, remaining.Width, bounds.Height); remaining.Height = global::System.Math.Max(0, remaining.Height - bounds.Height); break; case DockStyle.Left: bounds = new global::System.Drawing.Rectangle(remaining.X, remaining.Y, bounds.Width, remaining.Height); remaining.X += bounds.Width; remaining.Width = global::System.Math.Max(0, remaining.Width - bounds.Width); break; case DockStyle.Right: bounds = new global::System.Drawing.Rectangle(remaining.Right - bounds.Width, remaining.Y, bounds.Width, remaining.Height); remaining.Width = global::System.Math.Max(0, remaining.Width - bounds.Width); break; case DockStyle.Fill: bounds = remaining; break; default: continue; } child.Bounds = bounds; } }");
            Add("internal void __SetParent(Control? value) { if (global::System.Object.ReferenceEquals(__parent, value)) return; if (__parent is not null) __parent.Controls.Remove(this); if (value is not null) value.Controls.Add(this); }");
            Add("internal global::System.Drawing.Point __ScreenOffset() { var point = Location; for (var current = __parent; current is not null; current = current.__parent) point.Offset(current.Location); return point; }");
            Add("internal bool __ContainsDescendant(Control candidate) { foreach (Control child in Controls) if (global::System.Object.ReferenceEquals(child, candidate) || child.__ContainsDescendant(candidate)) return true; return false; }");
            Add("internal void __SetStyle(ControlStyles style, bool enabled) { var bits = (long)style; if (enabled) __styles |= bits; else __styles &= ~bits; }");
            Add("internal Padding __LayoutMargin { get { return __margin; } }");
            Add("internal bool __TabStop { get { return __tabStop; } }");
            Add("internal bool __LayoutAutoSize { get { return __autoSize; } }");
            Add("internal global::System.Drawing.Size __PreferredLayoutSize() { var width = global::System.Math.Max(Width, __minimumSize.Width); var height = global::System.Math.Max(Height, __minimumSize.Height); if (this is ButtonBase) { width = global::System.Math.Max(width, 75); height = global::System.Math.Max(height, 23); } if (__autoSize || width == 0 || height == 0) { var childWidth = 0; var childHeight = 0; foreach (Control child in Controls) { if (!child.Visible) continue; var preferred = child.__PreferredLayoutSize(); var margin = child.__LayoutMargin; childWidth = global::System.Math.Max(childWidth, child.Left + preferred.Width + margin.Right); childHeight = global::System.Math.Max(childHeight, child.Top + preferred.Height + margin.Bottom); } if (__autoSize || width == 0) width = global::System.Math.Max(width, childWidth + __padding.Right); if (__autoSize || height == 0) height = global::System.Math.Max(height, childHeight + __padding.Bottom); } return new global::System.Drawing.Size(width, height); }");
            Add("internal void __RefreshAutoSizeFromChildren() { if (!__autoSize) return; var preferred = __PreferredLayoutSize(); var next = Size; if ((Dock is DockStyle.Top or DockStyle.Bottom || next.Height == 0) && preferred.Height > 0) next.Height = preferred.Height; if ((Dock is DockStyle.Left or DockStyle.Right || next.Width == 0) && preferred.Width > 0) next.Width = preferred.Width; if (next != Size) Size = next; __parent?.PerformLayout(); }");
            Add("public event global::System.EventHandler? BackColorChanged;");
            Add("public event global::System.EventHandler? EnabledChanged;");
            Add("public event global::System.EventHandler? FontChanged;");
            Add("public event global::System.EventHandler? ForeColorChanged;");
            Add("public event global::System.EventHandler? GotFocus;");
            Add("protected virtual void OnBackColorChanged(global::System.EventArgs e) { BackColorChanged?.Invoke(this, e); }");
            Add("protected virtual void OnTextChanged(global::System.EventArgs e) { TextChanged?.Invoke(this, e); }");
            Add("protected virtual void OnVisibleChanged(global::System.EventArgs e) { VisibleChanged?.Invoke(this, e); }");
            break;
        case "System.Windows.Forms.ContainerControl":
            Add("private global::System.Drawing.SizeF __autoScaleDimensions;");
            Add("private AutoScaleMode __autoScaleMode;");
            break;
        case "System.Windows.Forms.TableLayoutStyle":
            Add("protected SizeType __sizeType;");
            Add("protected float __size;");
            Add("internal SizeType __LayoutSizeType { get { return __sizeType; } }");
            Add("internal float __LayoutSize { get { return __size; } }");
            break;
        case "System.Windows.Forms.TableLayoutStyleCollection":
            Add("protected readonly global::System.Collections.Generic.List<TableLayoutStyle> __styles = new();");
            Add("internal global::System.Collections.Generic.IReadOnlyList<TableLayoutStyle> __LayoutStyles { get { return __styles; } }");
            break;
        case "System.Windows.Forms.TableLayoutPanel":
            Add("private readonly TableLayoutControlCollection __tableControls;");
            Add("private readonly TableLayoutColumnStyleCollection __columnStyles = new();");
            Add("private readonly TableLayoutRowStyleCollection __rowStyles = new();");
            Add("private readonly global::System.Collections.Generic.Dictionary<Control, int> __columnSpans = new();");
            Add("private readonly global::System.Collections.Generic.Dictionary<Control, int> __rowSpans = new();");
            Add("private int __columnCount;");
            Add("private int __rowCount;");
            Add("private readonly global::System.Collections.Generic.Dictionary<Control, (int Column, int Row)> __cells = new();");
            Add("internal void __SetCell(Control control, int column, int row) { if (column < -1) throw new global::System.ArgumentOutOfRangeException(nameof(column)); if (row < -1) throw new global::System.ArgumentOutOfRangeException(nameof(row)); __cells[control] = (column, row); }");
            Add("internal string __TableTrace() { var columns = global::System.String.Join(',', global::System.Linq.Enumerable.Select(__columnStyles.__LayoutStyles, style => style.__LayoutSizeType + \":\" + style.__LayoutSize)); var rows = global::System.String.Join(',', global::System.Linq.Enumerable.Select(__rowStyles.__LayoutStyles, style => style.__LayoutSizeType + \":\" + style.__LayoutSize)); var cells = global::System.String.Join(',', global::System.Linq.Enumerable.Select(__cells, pair => pair.Key.Name + \":\" + pair.Value.Column + \":\" + pair.Value.Row)); return \"|table-columns=\" + columns + \"|table-rows=\" + rows + \"|table-cells=\" + cells; }");
            Add("private static int[] __TableAxis(int total, int count, global::System.Collections.Generic.IReadOnlyList<TableLayoutStyle> styles, bool columns, TableLayoutPanel owner) { count = global::System.Math.Max(1, count); total = global::System.Math.Max(0, total); var sizes = new int[count]; var remaining = total; var percent = 0f; var percentCells = 0; for (var i = 0; i < count; ++i) { var style = i < styles.Count ? styles[i] : null; if (style?.__LayoutSizeType == SizeType.Absolute) { sizes[i] = global::System.Math.Min(remaining, global::System.Math.Max(0, (int)global::System.Math.Round(style.__LayoutSize))); remaining -= sizes[i]; } else if (style?.__LayoutSizeType == SizeType.Percent) { percent += global::System.Math.Max(0f, style.__LayoutSize); ++percentCells; } } for (var i = 0; i < count && remaining > 0; ++i) { var style = i < styles.Count ? styles[i] : null; if (style?.__LayoutSizeType == SizeType.Absolute || style?.__LayoutSizeType == SizeType.Percent) continue; var desired = 0; foreach (Control child in owner.Controls) if (owner.__cells.TryGetValue(child, out var cell) && (columns ? cell.Column : cell.Row) == i && (columns ? (!owner.__columnSpans.TryGetValue(child, out var columnSpan) || columnSpan == 1) : (!owner.__rowSpans.TryGetValue(child, out var rowSpan) || rowSpan == 1))) { var margin = child.__LayoutMargin; var preferred = child.__PreferredLayoutSize(); desired = global::System.Math.Max(desired, columns ? preferred.Width + margin.Left + margin.Right : preferred.Height + margin.Top + margin.Bottom); } sizes[i] = global::System.Math.Min(remaining, global::System.Math.Max(0, desired)); remaining -= sizes[i]; } var percentRemaining = remaining; var lastPercent = -1; for (var i = 0; i < count; ++i) { var style = i < styles.Count ? styles[i] : null; if (style?.__LayoutSizeType != SizeType.Percent) continue; lastPercent = i; var share = percent <= 0f ? (percentCells == 0 ? 0 : percentRemaining / percentCells) : (int)global::System.Math.Floor(percentRemaining * global::System.Math.Max(0f, style.__LayoutSize) / percent); share = global::System.Math.Min(remaining, global::System.Math.Max(0, share)); sizes[i] = share; remaining -= share; } if (remaining > 0) sizes[lastPercent >= 0 ? lastPercent : count - 1] += remaining; return sizes; }");
            Add("private void __ApplyTableLayout() { var columns = global::System.Math.Max(1, __columnCount); var rows = global::System.Math.Max(1, __rowCount); var widths = __TableAxis(ClientSize.Width - Padding.Left - Padding.Right, columns, __columnStyles.__LayoutStyles, true, this); var heights = __TableAxis(ClientSize.Height - Padding.Top - Padding.Bottom, rows, __rowStyles.__LayoutStyles, false, this); var next = 0; foreach (Control child in Controls) { if (!child.Visible) continue; var cell = __cells.TryGetValue(child, out var assigned) ? assigned : (Column: -1, Row: -1); var column = cell.Column < 0 ? next % columns : global::System.Math.Min(cell.Column, columns - 1); var row = cell.Row < 0 ? next / columns : global::System.Math.Min(cell.Row, rows - 1); ++next; row = global::System.Math.Min(row, rows - 1); var columnSpan = global::System.Math.Min(__columnSpans.TryGetValue(child, out var cs) ? cs : 1, columns - column); var rowSpan = global::System.Math.Min(__rowSpans.TryGetValue(child, out var rs) ? rs : 1, rows - row); var x = Padding.Left; for (var i = 0; i < column; ++i) x += widths[i]; var y = Padding.Top; for (var i = 0; i < row; ++i) y += heights[i]; var width = 0; for (var i = column; i < column + columnSpan; ++i) width += widths[i]; var height = 0; for (var i = row; i < row + rowSpan; ++i) height += heights[i]; var margin = child.__LayoutMargin; var cellBounds = new global::System.Drawing.Rectangle(x + margin.Left, y + margin.Top, global::System.Math.Max(0, width - margin.Left - margin.Right), global::System.Math.Max(0, height - margin.Top - margin.Bottom)); var bounds = child.Bounds; var preferred = child.__PreferredLayoutSize(); if (child.__LayoutAutoSize || bounds.Width == 0) bounds.Width = preferred.Width; if (child.__LayoutAutoSize || bounds.Height == 0) bounds.Height = preferred.Height; if (child.Dock is DockStyle.Fill or DockStyle.Top or DockStyle.Bottom || (child.Anchor & (AnchorStyles.Left | AnchorStyles.Right)) == (AnchorStyles.Left | AnchorStyles.Right)) bounds.Width = cellBounds.Width; if (child.Dock is DockStyle.Fill or DockStyle.Left or DockStyle.Right || (child.Anchor & (AnchorStyles.Top | AnchorStyles.Bottom)) == (AnchorStyles.Top | AnchorStyles.Bottom)) bounds.Height = cellBounds.Height; bounds.Width = global::System.Math.Min(bounds.Width, cellBounds.Width); bounds.Height = global::System.Math.Min(bounds.Height, cellBounds.Height); bounds.X = child.Dock == DockStyle.Right || ((child.Anchor & AnchorStyles.Right) != 0 && (child.Anchor & AnchorStyles.Left) == 0) ? cellBounds.Right - bounds.Width : child.Dock is DockStyle.Fill or DockStyle.Top or DockStyle.Bottom or DockStyle.Left || (child.Anchor & AnchorStyles.Left) != 0 ? cellBounds.Left : cellBounds.Left + (cellBounds.Width - bounds.Width) / 2; bounds.Y = child.Dock == DockStyle.Bottom || ((child.Anchor & AnchorStyles.Bottom) != 0 && (child.Anchor & AnchorStyles.Top) == 0) ? cellBounds.Bottom - bounds.Height : child.Dock is DockStyle.Fill or DockStyle.Top or DockStyle.Left or DockStyle.Right || (child.Anchor & AnchorStyles.Top) != 0 ? cellBounds.Top : cellBounds.Top + (cellBounds.Height - bounds.Height) / 2; child.Bounds = bounds; } }");
            Add("protected override void OnLayout(LayoutEventArgs levent) { __ApplyTableLayout(); }");
            break;
        case "System.Windows.Forms.TableLayoutControlCollection":
            Add("private readonly TableLayoutPanel? __tableOwner;");
            Add("internal TableLayoutControlCollection(Control owner) : base(owner) { __tableOwner = owner as TableLayoutPanel; }");
            break;
        case "System.Windows.Forms.FlowLayoutPanel":
            Add("private bool __wrapContents = true;");
            Add("private void __ApplyFlowLayout() { var x = Padding.Left; var y = Padding.Top; var lineHeight = 0; var right = global::System.Math.Max(Padding.Left, ClientSize.Width - Padding.Right); foreach (Control child in Controls) { if (!child.Visible) continue; var margin = child.__LayoutMargin; var width = child.Width; var height = child.Height; if (__wrapContents && x > Padding.Left && x + margin.Left + width + margin.Right > right) { x = Padding.Left; y += lineHeight; lineHeight = 0; } child.Bounds = new global::System.Drawing.Rectangle(x + margin.Left, y + margin.Top, width, height); x += margin.Left + width + margin.Right; lineHeight = global::System.Math.Max(lineHeight, margin.Top + height + margin.Bottom); } }");
            Add("protected override void OnLayout(LayoutEventArgs levent) { __ApplyFlowLayout(); }");
            break;
        case "System.Windows.Forms.CheckBox":
            Add("private CheckState __checkState;");
            Add("private Appearance __checkAppearance;");
            Add("private global::System.Drawing.ContentAlignment __checkAlignment = global::System.Drawing.ContentAlignment.MiddleLeft;");
            break;
        case "System.Windows.Forms.ButtonBase":
            Add("private readonly FlatButtonAppearance __flatAppearance = new();");
            Add("private FlatStyle __buttonFlatStyle = FlatStyle.Standard;");
            Add("private global::System.Drawing.ContentAlignment __buttonTextAlign = global::System.Drawing.ContentAlignment.MiddleCenter;");
            Add("private bool __useVisualStyleBackColor = true;");
            Add("private void __PaintButtonSurface(global::System.Drawing.Graphics graphics) { using var background = new global::System.Drawing.SolidBrush(BackColor); graphics.FillRectangle(background, ClientRectangle); using var top = new global::System.Drawing.Pen(global::System.Drawing.Color.FromArgb(110, global::System.Drawing.Color.White)); using var edge = new global::System.Drawing.Pen(global::System.Drawing.Color.FromArgb(150, global::System.Drawing.Color.Black)); if (Width > 1 && Height > 1) { graphics.DrawLine(top, 0, 0, Width - 1, 0); graphics.DrawLine(top, 0, 0, 0, Height - 1); graphics.DrawLine(edge, 0, Height - 1, Width - 1, Height - 1); graphics.DrawLine(edge, Width - 1, 0, Width - 1, Height - 1); } var image = __ProjectionBackgroundImage; if (image is not null) { var destination = __ProjectionBackgroundImageLayout switch { ImageLayout.Stretch => ClientRectangle, ImageLayout.Zoom => __ZoomImage(image), ImageLayout.Center => new global::System.Drawing.Rectangle((Width - image.Width) / 2, (Height - image.Height) / 2, image.Width, image.Height), _ => new global::System.Drawing.Rectangle(2, 2, global::System.Math.Min(image.Width, global::System.Math.Max(0, Width - 4)), global::System.Math.Min(image.Height, global::System.Math.Max(0, Height - 4))) }; if (destination.Width > 0 && destination.Height > 0) graphics.DrawImage(image, destination); } else if (!global::System.String.IsNullOrEmpty(Text)) { using var ink = new global::System.Drawing.SolidBrush(ForeColor); using var font = new global::System.Drawing.Font(\"Portsmouth Rapids\", 10f, global::System.Drawing.FontStyle.Regular, global::System.Drawing.GraphicsUnit.Pixel, 1); var measured = graphics.MeasureString(Text, font); graphics.DrawString(Text, font, ink, global::System.Math.Max(3f, (Width - measured.Width) / 2f), global::System.Math.Max(2f, (Height - measured.Height) / 2f)); } }");
            Add("private global::System.Drawing.Rectangle __ZoomImage(global::System.Drawing.Image image) { if (image.Width <= 0 || image.Height <= 0 || Width <= 4 || Height <= 4) return global::System.Drawing.Rectangle.Empty; var scale = global::System.Math.Min((Width - 6d) / image.Width, (Height - 6d) / image.Height); var width = global::System.Math.Max(1, (int)global::System.Math.Round(image.Width * scale)); var height = global::System.Math.Max(1, (int)global::System.Math.Round(image.Height * scale)); return new global::System.Drawing.Rectangle((Width - width) / 2, (Height - height) / 2, width, height); }");
            break;
        case "System.Windows.Forms.Button":
            Add("private DialogResult __buttonDialogResult;");
            break;
        case "System.Windows.Forms.RadioButton":
            Add("private bool __checked;");
            Add("private bool __radioTabStop;");
            Add("private void __RaiseCheckedChanged() { var handlers = CheckedChanged; if (handlers is null) return; foreach (global::System.EventHandler handler in handlers.GetInvocationList()) { if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=radio-handler-begin|name=\" + Name + \"|owner=\" + (handler.Method.DeclaringType?.FullName ?? \"<unknown>\") + \"|method=\" + handler.Method.Name); handler.Invoke(this, global::System.EventArgs.Empty); if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=radio-handler-end|name=\" + Name + \"|method=\" + handler.Method.Name); } }");
            break;
        case "System.Windows.Forms.ScrollProperties":
            Add("private bool __scrollEnabled = true;");
            break;
        case "System.Windows.Forms.ScrollableControl":
            Add("private readonly HScrollProperties __horizontalScroll = new();");
            Add("private readonly VScrollProperties __verticalScroll = new();");
            Add("private readonly DockPaddingEdges __dockPadding = new();");
            Add("private bool __autoScroll;");
            Add("private bool __hScroll;");
            Add("private bool __vScroll;");
            break;
        case "System.Windows.Forms.ScrollableControl+DockPaddingEdges":
            Add("private int __left;");
            Add("private int __top;");
            Add("private int __right;");
            Add("private int __bottom;");
            break;
        case "System.Windows.Forms.Cursors":
            Add("private static readonly Cursor __cross = new();");
            Add("private static readonly Cursor __default = new();");
            Add("private static readonly Cursor __hSplit = new();");
            Add("private static readonly Cursor __hand = new();");
            Add("private static readonly Cursor __no = new();");
            Add("private static readonly Cursor __sizeWE = new();");
            Add("private static readonly Cursor __vSplit = new();");
            Add("private static readonly Cursor __wait = new();");
            break;
        case "System.Windows.Forms.ListControl":
            Add("private int __selectedIndex = -1;");
            Add("private string __displayMember = string.Empty;");
            Add("private bool __formattingEnabled;");
            Add("protected virtual int __SelectionItemCount => global::System.Int32.MaxValue;");
            Add("internal void __SetSelectedIndexSilently(int value) { __selectedIndex = value; }");
            Add("protected virtual void __OnSelectedIndexChanged() { SelectedValueChanged?.Invoke(this, global::System.EventArgs.Empty); }");
            break;
        case "System.Windows.Forms.ListBox":
            Add("private readonly ObjectCollection __listItems;");
            Add("private DrawMode __listDrawMode;");
            Add("protected override int __SelectionItemCount => __listItems.Count;");
            Add("protected override void OnMouseUp(global::System.Windows.Forms.MouseEventArgs e) { base.OnMouseUp(e); if (e.Button != global::System.Windows.Forms.MouseButtons.Left || __listItems.Count == 0) return; var row = global::System.Math.Clamp(e.Y / 22, 0, __listItems.Count - 1); SelectedIndex = row; }");
            break;
        case "System.Windows.Forms.ListBox+ObjectCollection":
            Add("private readonly global::System.Collections.Generic.List<object> __items = new();");
            Add("private readonly ListBox? __owner;");
            Add("internal ObjectCollection(ListBox owner) { __owner = owner; }");
            break;
        case "System.Windows.Forms.Label":
            Add("private bool __autoEllipsis;");
            Add("private global::System.Drawing.ContentAlignment __labelTextAlign = global::System.Drawing.ContentAlignment.MiddleLeft;");
            break;
        case "System.Windows.Forms.LinkLabel":
            Add("private readonly LinkCollection __links = new();");
            break;
        case "System.Windows.Forms.LinkLabel+Link":
            Add("private int __linkStart;");
            Add("private int __linkLength;");
            Add("private object? __linkData;");
            break;
        case "System.Windows.Forms.LinkLabel+LinkCollection":
            Add("private readonly global::System.Collections.Generic.List<Link> __linkItems = new();");
            break;
        case "System.Windows.Forms.LinkLabelLinkClickedEventArgs":
            Add("internal LinkLabel.Link? __clickedLink;");
            break;
        case "System.Windows.Forms.ProgressBar":
            Add("private int __progressMinimum;");
            Add("private int __progressMaximum = 100;");
            Add("private int __progressValue;");
            break;
        case "System.Windows.Forms.SplitContainer":
            Add("private readonly SplitterPanel __panel1 = new();");
            Add("private readonly SplitterPanel __panel2 = new();");
            Add("private int __splitterDistance = -1;");
            Add("protected override void OnLayout(LayoutEventArgs levent) { var distance = __splitterDistance < 0 ? global::System.Math.Max(0, ClientSize.Width / 2 - 2) : global::System.Math.Clamp(__splitterDistance, 0, global::System.Math.Max(0, ClientSize.Width - 4)); __panel1.Bounds = new global::System.Drawing.Rectangle(0, 0, distance, ClientSize.Height); __panel2.Bounds = new global::System.Drawing.Rectangle(global::System.Math.Min(ClientSize.Width, distance + 4), 0, global::System.Math.Max(0, ClientSize.Width - distance - 4), ClientSize.Height); base.OnLayout(levent); }");
            break;
        case "System.Windows.Forms.Splitter":
            Add("private int __splitPosition;");
            Add("private bool __splitterTabStop;");
            break;
        case "System.Windows.Forms.ComboBox":
            Add("private readonly ObjectCollection __comboItems;");
            Add("private ComboBoxStyle __dropDownStyle;");
            Add("private FlatStyle __comboFlatStyle;");
            Add("private object? __selectedItem;");
            Add("private AutoCompleteMode __autoCompleteMode;");
            Add("private AutoCompleteSource __autoCompleteSource;");
            Add("private DrawMode __comboDrawMode;");
            Add("private int __dropDownHeight = 106;");
            Add("private int __dropDownWidth;");
            Add("private bool __integralHeight = true;");
            Add("private ContextMenuStrip? __comboDropDown;");
            Add("private bool __comboDropDownOpen;");
            Add("private int __comboCaret = -1;");
            Add("private int __comboAnchor = -1;");
            Add("protected override int __SelectionItemCount => __comboItems.Count;");
            Add("private void __NormalizeComboSelection() { if (__comboCaret < 0) __comboCaret = Text.Length; if (__comboAnchor < 0) __comboAnchor = __comboCaret; __comboCaret = global::System.Math.Clamp(__comboCaret, 0, Text.Length); __comboAnchor = global::System.Math.Clamp(__comboAnchor, 0, Text.Length); }");
            Add("private int __ComboSelectionStart { get { __NormalizeComboSelection(); return global::System.Math.Min(__comboCaret, __comboAnchor); } }");
            Add("private int __ComboSelectionLength { get { __NormalizeComboSelection(); return global::System.Math.Abs(__comboCaret - __comboAnchor); } }");
            Add("private void __SetComboSelection(int start, int length) { if (start < 0) throw new global::System.ArgumentOutOfRangeException(nameof(start)); if (length < 0) throw new global::System.ArgumentOutOfRangeException(nameof(length)); start = global::System.Math.Min(start, Text.Length); length = global::System.Math.Min(length, Text.Length - start); __comboAnchor = start; __comboCaret = start + length; Invalidate(); }");
            Add("private void __MoveComboCaret(int target, bool extend) { __NormalizeComboSelection(); target = global::System.Math.Clamp(target, 0, Text.Length); if (!extend) __comboAnchor = target; __comboCaret = target; Invalidate(); }");
            Add("private void __ReplaceComboSelection(string value, int replacementStart = -1, int replacementLength = 0) { __NormalizeComboSelection(); var start = replacementStart >= 0 ? global::System.Math.Clamp(replacementStart, 0, Text.Length) : __ComboSelectionStart; var length = replacementStart >= 0 ? global::System.Math.Clamp(replacementLength, 0, Text.Length - start) : __ComboSelectionLength; __SetSelectedIndexSilently(-1); Text = Text.Remove(start, length).Insert(start, value); __comboCaret = __comboAnchor = start + value.Length; Invalidate(); }");
            Add("internal void __ResetItemsSelection() { __SetSelectedIndexSilently(-1); Text = string.Empty; __comboCaret = __comboAnchor = 0; }");
            Add("internal override void __NativeTextInput(string text, bool composing, int replacementStart, int replacementLength) { if (__dropDownStyle != ComboBoxStyle.DropDownList && !global::System.String.IsNullOrEmpty(text)) __ReplaceComboSelection(text, replacementStart, replacementLength); }");
            Add("internal override void __NativeKeyInput(uint physicalKey, bool down, uint modifiers, bool repeat) { base.__NativeKeyInput(physicalKey, down, modifiers, repeat); if (!down || __dropDownStyle == ComboBoxStyle.DropDownList) return; __NormalizeComboSelection(); var shift = (modifiers & 1u) != 0; var control = (modifiers & 2u) != 0; if (control && physicalKey == 0x04u) { __comboAnchor = 0; __comboCaret = Text.Length; Invalidate(); return; } if (physicalKey == 0x2au) { if (__ComboSelectionLength > 0) __ReplaceComboSelection(string.Empty); else if (__comboCaret > 0) { __comboAnchor = __comboCaret - 1; __ReplaceComboSelection(string.Empty); } } else if (physicalKey == 0x4cu) { if (__ComboSelectionLength > 0) __ReplaceComboSelection(string.Empty); else if (__comboCaret < Text.Length) { __comboAnchor = __comboCaret + 1; __ReplaceComboSelection(string.Empty); } } else if (physicalKey == 0x4au) __MoveComboCaret(0, shift); else if (physicalKey == 0x4du) __MoveComboCaret(Text.Length, shift); else if (physicalKey == 0x50u) { var target = !shift && __ComboSelectionLength > 0 ? __ComboSelectionStart : control ? 0 : global::System.Math.Max(0, __comboCaret - 1); __MoveComboCaret(target, shift); } else if (physicalKey == 0x4fu) { var target = !shift && __ComboSelectionLength > 0 ? __ComboSelectionStart + __ComboSelectionLength : control ? Text.Length : global::System.Math.Min(Text.Length, __comboCaret + 1); __MoveComboCaret(target, shift); } }");
            Add("protected override void __OnSelectedIndexChanged() { Text = SelectedIndex >= 0 && SelectedIndex < __comboItems.Count ? global::System.Convert.ToString(__comboItems[SelectedIndex], global::System.Globalization.CultureInfo.CurrentCulture) ?? string.Empty : string.Empty; __comboCaret = __comboAnchor = Text.Length; base.__OnSelectedIndexChanged(); SelectedIndexChanged?.Invoke(this, global::System.EventArgs.Empty); }");
            Add("private void __CloseItems() { if (!__comboDropDownOpen) return; __comboDropDownOpen = false; __comboDropDown?.__CloseDropDown(); DropDownClosed?.Invoke(this, global::System.EventArgs.Empty); }");
            Add("private void __ShowItems() { if (__dropDownStyle == ComboBoxStyle.Simple || __comboItems.Count == 0 || __comboDropDownOpen) return; if (__comboDropDown is not null) { __comboDropDown.__CloseDropDown(); __comboDropDown.Dispose(); } var menu = new ContextMenuStrip { Name = Name + \".DropDown\" }; __comboDropDown = menu; for (var index = 0; index < __comboItems.Count; ++index) { var selection = index; var text = global::System.Convert.ToString(__comboItems[index], global::System.Globalization.CultureInfo.CurrentCulture) ?? string.Empty; var item = new ToolStripMenuItem(text) { Name = \"item.\" + index.ToString(global::System.Globalization.CultureInfo.InvariantCulture), Checked = index == SelectedIndex }; item.Click += (_, _) => { SelectedIndex = selection; __CloseItems(); }; menu.Items.Add(item); } DropDown?.Invoke(this, global::System.EventArgs.Empty); __comboDropDownOpen = true; menu.Show(this, new global::System.Drawing.Point(0, Height)); }");
            Add("protected override void OnMouseUp(global::System.Windows.Forms.MouseEventArgs e) { base.OnMouseUp(e); if (e.Button != global::System.Windows.Forms.MouseButtons.Left) return; if (__dropDownStyle == ComboBoxStyle.DropDownList || e.X >= global::System.Math.Max(0, Width - 22)) { __ShowItems(); return; } var target = global::System.Math.Clamp((int)global::System.Math.Round((e.X - 5) / 7.0), 0, Text.Length); __MoveComboCaret(target, false); }");
            break;
        case "System.Windows.Forms.ComboBox+ObjectCollection":
            Add("private readonly global::System.Collections.Generic.List<object> __items = new();");
            Add("private readonly ComboBox? __owner;");
            Add("internal ObjectCollection(ComboBox owner) { __owner = owner; }");
            break;
        case "System.Windows.Forms.TextBoxBase":
            Add("private bool __textReadOnly;");
            Add("private bool __textMultiline;");
            Add("private BorderStyle __textBorderStyle = BorderStyle.Fixed3D;");
            Add("private int __textCaret = -1;");
            Add("private int __textAnchor = -1;");
            Add("private void __NormalizeTextSelection() { if (__textCaret < 0) __textCaret = Text.Length; if (__textAnchor < 0) __textAnchor = __textCaret; __textCaret = global::System.Math.Clamp(__textCaret, 0, Text.Length); __textAnchor = global::System.Math.Clamp(__textAnchor, 0, Text.Length); }");
            Add("private int __TextSelectionStart { get { __NormalizeTextSelection(); return global::System.Math.Min(__textCaret, __textAnchor); } }");
            Add("private int __TextSelectionLength { get { __NormalizeTextSelection(); return global::System.Math.Abs(__textCaret - __textAnchor); } }");
            Add("private void __SetTextSelection(int start, int length) { if (start < 0) throw new global::System.ArgumentOutOfRangeException(nameof(start)); if (length < 0) throw new global::System.ArgumentOutOfRangeException(nameof(length)); start = global::System.Math.Min(start, Text.Length); length = global::System.Math.Min(length, Text.Length - start); __textAnchor = start; __textCaret = start + length; Invalidate(); }");
            Add("private void __MoveTextCaret(int target, bool extend) { __NormalizeTextSelection(); target = global::System.Math.Clamp(target, 0, Text.Length); if (!extend) __textAnchor = target; __textCaret = target; Invalidate(); }");
            Add("private void __ReplaceTextSelection(string value, int replacementStart = -1, int replacementLength = 0) { __NormalizeTextSelection(); var start = replacementStart >= 0 ? global::System.Math.Clamp(replacementStart, 0, Text.Length) : __TextSelectionStart; var length = replacementStart >= 0 ? global::System.Math.Clamp(replacementLength, 0, Text.Length - start) : __TextSelectionLength; Text = Text.Remove(start, length).Insert(start, value); __textCaret = __textAnchor = start + value.Length; Invalidate(); }");
            Add("internal override void __NativeTextInput(string text, bool composing, int replacementStart, int replacementLength) { if (!__textReadOnly && !global::System.String.IsNullOrEmpty(text)) __ReplaceTextSelection(text, replacementStart, replacementLength); }");
            Add("internal override void __NativeKeyInput(uint physicalKey, bool down, uint modifiers, bool repeat) { base.__NativeKeyInput(physicalKey, down, modifiers, repeat); if (!down || __textReadOnly) return; __NormalizeTextSelection(); var shift = (modifiers & 1u) != 0; var control = (modifiers & 2u) != 0; if (control && physicalKey == 0x04u) { __textAnchor = 0; __textCaret = Text.Length; Invalidate(); return; } if (physicalKey == 0x2au) { if (__TextSelectionLength > 0) __ReplaceTextSelection(string.Empty); else if (__textCaret > 0) { __textAnchor = __textCaret - 1; __ReplaceTextSelection(string.Empty); } } else if (physicalKey == 0x4cu) { if (__TextSelectionLength > 0) __ReplaceTextSelection(string.Empty); else if (__textCaret < Text.Length) { __textAnchor = __textCaret + 1; __ReplaceTextSelection(string.Empty); } } else if (physicalKey == 0x4au) __MoveTextCaret(0, shift); else if (physicalKey == 0x4du) __MoveTextCaret(Text.Length, shift); else if (physicalKey == 0x50u) { var target = !shift && __TextSelectionLength > 0 ? __TextSelectionStart : control ? 0 : global::System.Math.Max(0, __textCaret - 1); __MoveTextCaret(target, shift); } else if (physicalKey == 0x4fu) { var target = !shift && __TextSelectionLength > 0 ? __TextSelectionStart + __TextSelectionLength : control ? Text.Length : global::System.Math.Min(Text.Length, __textCaret + 1); __MoveTextCaret(target, shift); } }");
            Add("protected override void OnMouseUp(global::System.Windows.Forms.MouseEventArgs e) { base.OnMouseUp(e); if (e.Button != global::System.Windows.Forms.MouseButtons.Left) return; var target = global::System.Math.Clamp((int)global::System.Math.Round((e.X - 5) / 7.0), 0, Text.Length); __MoveTextCaret(target, false); }");
            break;
        case "System.Windows.Forms.NumericUpDown":
            Add("private decimal __minimum;");
            Add("private decimal __maximum = 100m;");
            Add("private decimal __increment = 1m;");
            Add("private decimal __value;");
            Add("private int __decimalPlaces;");
            Add("private bool __thousandsSeparator;");
            Add("private void __UpdateNumericText() { var format = (__thousandsSeparator ? \"N\" : \"F\") + __decimalPlaces.ToString(global::System.Globalization.CultureInfo.InvariantCulture); Text = __value.ToString(format, global::System.Globalization.CultureInfo.CurrentCulture); }");
            Add("private void __Spin(bool increase) { var candidate = __value + (increase ? __increment : -__increment); Value = candidate < __minimum ? __minimum : candidate > __maximum ? __maximum : candidate; }");
            Add("protected override void OnMouseUp(global::System.Windows.Forms.MouseEventArgs e) { base.OnMouseUp(e); if (e.Button == global::System.Windows.Forms.MouseButtons.Left && e.X >= global::System.Math.Max(0, Width - 18)) __Spin(e.Y < Height / 2); }");
            Add("protected override void OnMouseWheel(global::System.Windows.Forms.MouseEventArgs e) { base.OnMouseWheel(e); if (e.Delta != 0) __Spin(e.Delta > 0); }");
            break;
        case "System.Windows.Forms.UpDownBase":
            Add("private int __initializationDepth;");
            Add("private BorderStyle __upDownBorderStyle = BorderStyle.Fixed3D;");
            Add("private HorizontalAlignment __upDownTextAlign;");
            Add("public void BeginInit() { ++__initializationDepth; }");
            Add("public void EndInit() { if (__initializationDepth == 0) return; if (--__initializationDepth == 0) PerformLayout(); }");
            break;
        case "System.Windows.Forms.PictureBox":
            Add("private int __initializationDepth;");
            Add("private BorderStyle __pictureBorderStyle;");
            Add("private global::System.Drawing.Image? __pictureImage;");
            Add("private global::System.Drawing.Image? __errorImage;");
            Add("private global::System.Drawing.Image? __initialImage;");
            Add("private PictureBoxSizeMode __pictureSizeMode;");
            Add("public void BeginInit() { ++__initializationDepth; }");
            Add("public void EndInit() { if (__initializationDepth == 0) return; if (--__initializationDepth == 0) PerformLayout(); }");
            break;
        case "System.Windows.Forms.Panel":
            Add("private AutoSizeMode __panelAutoSizeMode;");
            Add("private BorderStyle __panelBorderStyle;");
            break;
        case "System.Windows.Forms.FlatButtonAppearance":
            Add("private int __flatBorderSize = 1;");
            break;
        case "System.Windows.Forms.GroupBox":
            Add("private FlatStyle __groupFlatStyle = FlatStyle.Standard;");
            Add("private bool __groupTabStop;");
            break;
        case "System.Windows.Forms.KeyPressEventArgs":
            Add("private bool __keyPressHandled;");
            break;
        case "System.Windows.Forms.HandledMouseEventArgs":
            Add("private bool __mouseHandled;");
            break;
        case "System.Windows.Forms.StatusStrip":
            Add("private bool __statusTabStop;");
            break;
        case "System.Windows.Forms.TextBox":
            Add("private HorizontalAlignment __textBoxAlignment;");
            break;
        case "System.Windows.Forms.UserControl":
            Add("private AutoSizeMode __userControlAutoSizeMode;");
            break;
        case "System.Windows.Forms.DataGridView":
            Add("private int __initializationDepth;");
            Add("private readonly DataGridViewColumnCollection __gridColumns;");
            Add("private readonly DataGridViewRowCollection __gridRows;");
            Add("private readonly DataGridViewSelectedCellCollection __selectedCells = new();");
            Add("private readonly DataGridViewSelectedRowCollection __selectedRows = new();");
            Add("private readonly DataGridViewCellStyle __alternatingRowsDefaultCellStyle = new();");
            Add("private readonly DataGridViewCellStyle __columnHeadersDefaultCellStyle = new();");
            Add("private readonly DataGridViewCellStyle __defaultCellStyle = new();");
            Add("private readonly DataGridViewRow __rowTemplate = new();");
            Add("private readonly global::System.Collections.Generic.Dictionary<(int Column, int Row), DataGridViewCell> __cells = new();");
            Add("private object? __gridDataSource;");
            Add("private bool __allowUserToAddRows = true, __allowUserToDeleteRows = true, __allowUserToResizeColumns = true, __allowUserToResizeRows = true, __autoGenerateColumns = true;");
            Add("private global::System.Drawing.Color __gridBackgroundColor, __gridColor;");
            Add("private BorderStyle __gridBorderStyle;");
            Add("private DataGridViewHeaderBorderStyle __columnHeaderBorderStyle, __rowHeaderBorderStyle;");
            Add("private int __columnHeadersHeight = 23, __firstDisplayedRow;");
            Add("private DataGridViewColumnHeadersHeightSizeMode __columnHeadersHeightSizeMode;");
            Add("private bool __enableHeadersVisualStyles = true, __multiSelect = true, __gridReadOnly, __rowHeadersVisible = true;");
            Add("private ScrollBars __gridScrollBars = ScrollBars.Both;");
            Add("private DataGridViewSelectionMode __gridSelectionMode;");
            Add("private bool __showCellErrors = true, __showCellToolTips = true, __showEditingIcon = true, __showRowErrors = true;");
            Add("internal int __DataRowCount { get { if (__gridDataSource is BindingSource source) return source.List.Count; if (__gridDataSource is global::System.Collections.IList list) return list.Count; return 0; } }");
            Add("private DataGridViewCell __Cell(int columnIndex, int rowIndex) { if (columnIndex < 0 || rowIndex < 0) throw new global::System.ArgumentOutOfRangeException(); var key = (columnIndex, rowIndex); if (!__cells.TryGetValue(key, out var cell)) { cell = new DataGridViewTextBoxCell { __dataGridView = this, __rowIndex = rowIndex }; __cells.Add(key, cell); } return cell; }");
            Add("public void BeginInit() { ++__initializationDepth; }");
            Add("public void EndInit() { if (__initializationDepth == 0) return; if (--__initializationDepth == 0) PerformLayout(); }");
            break;
        case "System.Windows.Forms.DataGridViewElement":
            Add("internal DataGridView? __dataGridView;");
            break;
        case "System.Windows.Forms.DataGridViewBand":
            Add("private DataGridViewCellStyle __bandDefaultCellStyle = new();");
            Add("private bool __bandReadOnly;");
            Add("private bool __bandSelected;");
            Add("private DataGridViewTriState __bandResizable = DataGridViewTriState.NotSet;");
            break;
        case "System.Windows.Forms.DataGridViewCell":
            Add("private readonly DataGridViewCellStyle __cellStyle = new();");
            Add("internal object? __cellValue;");
            Add("internal int __rowIndex = -1;");
            break;
        case "System.Windows.Forms.DataGridViewCellStyle":
            Add("private DataGridViewContentAlignment __alignment;");
            Add("private global::System.Drawing.Color __styleBackColor;");
            Add("private global::System.Drawing.Color __styleForeColor;");
            Add("private global::System.Drawing.Color __selectionBackColor;");
            Add("private global::System.Drawing.Font? __styleFont;");
            Add("private string __format = string.Empty;");
            break;
        case "System.Windows.Forms.DataGridViewColumn":
            Add("private DataGridViewCell? __cellTemplate;");
            Add("private string __dataPropertyName = string.Empty;");
            Add("private string __headerText = string.Empty;");
            Add("private string __columnName = string.Empty;");
            Add("private int __columnWidth = 100;");
            Add("private DataGridViewAutoSizeColumnMode __columnAutoSizeMode;");
            Add("private float __fillWeight = 100f;");
            break;
        case "System.Windows.Forms.DataGridViewCellFormattingEventArgs":
            Add("internal int __formattingColumnIndex = -1;");
            Add("private bool __formattingApplied;");
            break;
        case "System.Windows.Forms.DataGridViewRow":
            Add("private int __rowHeight = 22;");
            break;
        case "System.Windows.Forms.DataGridViewColumnCollection":
            Add("private readonly DataGridView? __owner;");
            Add("private readonly global::System.Collections.Generic.List<DataGridViewColumn> __items = new();");
            Add("internal DataGridViewColumnCollection(DataGridView owner) { __owner = owner; }");
            Add("protected override global::System.Collections.IList __Collection { get { return __items; } }");
            Add("public override int Count { get { return __items.Count; } }");
            break;
        case "System.Windows.Forms.DataGridViewRowCollection":
            Add("private readonly DataGridView? __owner;");
            Add("private readonly global::System.Collections.Generic.List<DataGridViewRow> __rows = new();");
            Add("internal DataGridViewRowCollection(DataGridView owner) { __owner = owner; }");
            Add("private void __EnsureRows() { var count = __owner?.__DataRowCount ?? 0; while (__rows.Count < count) { var row = new DataGridViewRow(); row.__dataGridView = __owner; __rows.Add(row); } if (__rows.Count > count) __rows.RemoveRange(count, __rows.Count - count); }");
            break;
        case "System.Windows.Forms.DataGridViewSelectedCellCollection":
        case "System.Windows.Forms.DataGridViewSelectedRowCollection":
            Add("protected override global::System.Collections.IList __Collection { get { return global::System.Array.Empty<object>(); } }");
            break;
        case "System.Windows.Forms.BindingSource":
            Add("private int __initializationDepth;");
            Add("private object? __bindingDataSource;");
            Add("private global::System.Collections.IList __bindingList = new global::System.Collections.ArrayList();");
            Add("private int __position = -1;");
            Add("private void __SetDataSource(object? value) { __bindingDataSource = value; __bindingList = value switch { BindingSource source => source.List, global::System.Collections.IList list => list, global::System.Collections.IEnumerable sequence when value is not string => new global::System.Collections.ArrayList(global::System.Linq.Enumerable.ToArray(global::System.Linq.Enumerable.Cast<object>(sequence))), _ => new global::System.Collections.ArrayList() }; __position = __bindingList.Count == 0 ? -1 : global::System.Math.Clamp(__position < 0 ? 0 : __position, 0, __bindingList.Count - 1); }");
            Add("public void BeginInit() { ++__initializationDepth; }");
            Add("public void EndInit() { if (__initializationDepth != 0) --__initializationDepth; }");
            break;
        case "System.Windows.Forms.Timer":
            Add("private readonly object __timerGate = new();");
            Add("private readonly int __ownerThreadId = global::System.Environment.CurrentManagedThreadId;");
            Add("private global::System.Threading.Timer? __timer;");
            Add("private int __interval = 100;");
            Add("private volatile bool __timerEnabled;");
            Add("private int __tickPending;");
            Add("private void __RaiseTick() { var handlers = Tick; if (handlers is null) return; if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_TIMERS\") != \"1\") { handlers.Invoke(this, global::System.EventArgs.Empty); return; } foreach (global::System.EventHandler handler in handlers.GetInvocationList()) { var method = handler.Method; var owner = method.DeclaringType?.FullName ?? \"<unknown>\"; var started = global::System.Diagnostics.Stopwatch.GetTimestamp(); global::System.Console.Error.WriteLine(\"facade-timer=begin|owner:\" + owner + \"|method:\" + method.Name + \"|interval:\" + __interval); try { handler.Invoke(this, global::System.EventArgs.Empty); } finally { global::System.Console.Error.WriteLine(\"facade-timer=end|owner:\" + owner + \"|method:\" + method.Name + \"|elapsed-ms:\" + global::System.Diagnostics.Stopwatch.GetElapsedTime(started).TotalMilliseconds.ToString(\"F3\", global::System.Globalization.CultureInfo.InvariantCulture)); } } }");
            Add("private void __Schedule() { lock (__timerGate) { __timer?.Dispose(); global::System.Threading.Interlocked.Exchange(ref __tickPending, 0); __timer = __timerEnabled ? new global::System.Threading.Timer(_ => { if (!__timerEnabled || global::System.Threading.Interlocked.Exchange(ref __tickPending, 1) != 0) return; if (!Application.__Post(__ownerThreadId, () => { global::System.Threading.Interlocked.Exchange(ref __tickPending, 0); if (__timerEnabled) __RaiseTick(); })) global::System.Threading.Interlocked.Exchange(ref __tickPending, 0); }, null, __interval, __interval) : null; } }");
            Add("protected override void Dispose(bool disposing) { if (disposing) { __timerEnabled = false; global::System.Threading.Interlocked.Exchange(ref __tickPending, 0); lock (__timerGate) { __timer?.Dispose(); __timer = null; } } base.Dispose(disposing); }");
            break;
        case "System.Windows.Forms.ToolStrip":
            Add("private readonly ToolStripItemCollection __toolItems;");
            Add("private Padding __gripMargin;");
            Add("private ToolStripRenderMode __renderMode;");
            Add("private ToolStripRenderer? __renderer;");
            Add("private bool __toolTraceDone;");
            Add("private ToolStripItem? __hotItem;");
            Add("private readonly global::System.Collections.Generic.List<Label> __dropDownLabels = new();");
            Add("private readonly int __menuOwnerThreadId = global::System.Environment.CurrentManagedThreadId;");
            Add("private readonly object __menuHoverGate = new();");
            Add("private global::System.Threading.Timer? __menuHoverTimer;");
            Add("private int __menuHoverGeneration;");
            Add("internal void __RefreshItems() { if (__HasRaisedLoad) __RenderManagedPaint(); }");
            Add("internal void __TraceToolItems() { if (__toolTraceDone || global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_TOOLSTRIP\") != \"1\") return; __toolTraceDone = true; var rows = new global::System.Collections.Generic.List<string>(); foreach (ToolStripItem item in __toolItems) { var image = item.__ProjectionImage; rows.Add(item.GetType().Name + \":text=\" + item.__ProjectionText + \",tip=\" + item.__ProjectionToolTip + \",size=\" + item.__ProjectionSize.Width + \"x\" + item.__ProjectionSize.Height + \",image=\" + (image is null ? \"none\" : image.Width + \"x\" + image.Height)); } global::System.Console.Error.WriteLine(\"facade-toolstrip=name:\" + Name + \"|size:\" + Width + \"x\" + Height + \"|items:\" + global::System.String.Join(\";\", rows)); }");
            Add("private static int __ItemWidth(ToolStripItem item) { var size = item.__ProjectionSize; if (size.Width > 0) return size.Width; if (item.__ProjectionImage is not null) return 34; return global::System.Math.Max(24, item.__ProjectionText.Length * 7 + 14); }");
            Add("private int __DropDownRowHeight(ToolStripItem item) { return item is ToolStripSeparator ? 9 : global::System.Math.Max(24, item.__ProjectionSize.Height); }");
            Add("private int __DropDownItemTop(ToolStripItem sought) { var y = 4; foreach (ToolStripItem item in __toolItems) { if (!item.__ProjectionVisible) continue; if (global::System.Object.ReferenceEquals(item, sought)) return y; y += __DropDownRowHeight(item); } return 4; }");
            Add("private global::System.Collections.Generic.List<ToolStripItem> __SelectableItems() { var items = new global::System.Collections.Generic.List<ToolStripItem>(); foreach (ToolStripItem item in __toolItems) if (item.__ProjectionVisible && item.__ProjectionEnabled && item is not ToolStripSeparator) items.Add(item); return items; }");
            Add("private void __CancelHoverAction() { global::System.Threading.Interlocked.Increment(ref __menuHoverGeneration); lock (__menuHoverGate) { __menuHoverTimer?.Dispose(); __menuHoverTimer = null; } }");
            Add("private void __SetHotItem(ToolStripItem? item, bool scheduleHover) { if (global::System.Object.ReferenceEquals(item, __hotItem)) return; __hotItem = item; __CancelHoverAction(); __RenderManagedPaint(); if (!scheduleHover || this is not ToolStripDropDown dropDown || !Visible) return; var generation = __menuHoverGeneration; lock (__menuHoverGate) __menuHoverTimer = new global::System.Threading.Timer(_ => { _ = Application.__Post(__menuOwnerThreadId, () => { if (generation != __menuHoverGeneration || !Visible || !global::System.Object.ReferenceEquals(item, __hotItem)) return; if (item is ToolStripDropDownItem branch && branch.DropDownItems.Count > 0) __OpenBranch(branch, false); else dropDown.__CloseChildDropDown(); }); }, null, 400, global::System.Threading.Timeout.Infinite); }");
            Add("private void __MoveHot(int delta, bool edge) { var items = __SelectableItems(); if (items.Count == 0) return; var index = __hotItem is null ? -1 : items.IndexOf(__hotItem); index = edge ? (delta < 0 ? 0 : items.Count - 1) : index < 0 ? (delta < 0 ? items.Count - 1 : 0) : (index + delta + items.Count) % items.Count; __SetHotItem(items[index], false); }");
            Add("private void __OpenBranch(ToolStripDropDownItem branch, bool focusChild) { if (branch.DropDownItems.Count == 0) return; var location = PointToScreen(new global::System.Drawing.Point(global::System.Math.Max(0, Width - 2), __DropDownItemTop(branch))); if (this is ToolStripDropDown parentDropDown) parentDropDown.__ShowChildDropDown(branch.DropDown, location, focusChild); else { branch.DropDown.Show(location); if (focusChild) branch.DropDown.__FocusFirstItem(); } }");
            Add("private void __ActivateHotItem() { var item = __hotItem; if (item is null || !item.__ProjectionEnabled) return; if (item is ToolStripDropDownItem branch && branch.DropDownItems.Count > 0) { __OpenBranch(branch, true); return; } item.__PerformClick(); if (this is ToolStripDropDown dropDown && Visible) dropDown.__CloseMenuChain(); }");
            Add("internal void __FocusFirstItem() { var items = __SelectableItems(); if (items.Count != 0) __SetHotItem(items[0], false); Focus(); }");
            Add("internal void __PrepareDropDown(global::System.Drawing.Point screenLocation) { var width = 136; var height = 4; foreach (ToolStripItem item in __toolItems) if (item.__ProjectionVisible) { width = global::System.Math.Max(width, __ItemWidth(item) + 54); height += __DropDownRowHeight(item); } width = global::System.Math.Min(420, width); Bounds = new global::System.Drawing.Rectangle(screenLocation.X, screenLocation.Y, width, global::System.Math.Max(20, height + 4)); BackColor = global::System.Drawing.Color.FromArgb(247, 249, 252); ForeColor = global::System.Drawing.Color.FromArgb(31, 37, 44); foreach (var label in __dropDownLabels) { Controls.Remove(label); label.Dispose(); } __dropDownLabels.Clear(); }");
            Add("private void __PaintDropDownItems(global::System.Drawing.Graphics graphics) { using var background = new global::System.Drawing.SolidBrush(global::System.Drawing.Color.FromArgb(247, 249, 252)); using var rail = new global::System.Drawing.SolidBrush(global::System.Drawing.Color.FromArgb(230, 235, 241)); using var hot = new global::System.Drawing.SolidBrush(global::System.Drawing.Color.FromArgb(218, 231, 247)); using var ink = new global::System.Drawing.SolidBrush(global::System.Drawing.Color.FromArgb(31, 37, 44)); using var disabledInk = new global::System.Drawing.SolidBrush(global::System.Drawing.Color.FromArgb(145, 151, 158)); using var font = new global::System.Drawing.Font(\"Portsmouth Rapids\", 12f, global::System.Drawing.FontStyle.Regular, global::System.Drawing.GraphicsUnit.Pixel, 1); using var border = new global::System.Drawing.Pen(global::System.Drawing.Color.FromArgb(126, 139, 153)); using var separator = new global::System.Drawing.Pen(global::System.Drawing.Color.FromArgb(188, 197, 207)); graphics.FillRectangle(background, ClientRectangle); graphics.FillRectangle(rail, 1, 1, 31, global::System.Math.Max(0, Height - 2)); var y = 4; foreach (ToolStripItem item in __toolItems) { if (!item.__ProjectionVisible) continue; var rowHeight = __DropDownRowHeight(item); if (item is ToolStripSeparator) { graphics.DrawLine(separator, 34, y + rowHeight / 2, global::System.Math.Max(35, Width - 5), y + rowHeight / 2); y += rowHeight; continue; } var row = new global::System.Drawing.Rectangle(3, y, global::System.Math.Max(0, Width - 6), rowHeight); if (global::System.Object.ReferenceEquals(item, __hotItem) && item.__ProjectionEnabled) { graphics.FillRectangle(hot, row); graphics.DrawRectangle(border, row.X, row.Y, global::System.Math.Max(0, row.Width - 1), global::System.Math.Max(0, row.Height - 1)); } if (item is ToolStripMenuItem menu && menu.Checked) { graphics.DrawLine(border, 10, y + rowHeight / 2, 14, y + rowHeight / 2 + 4); graphics.DrawLine(border, 14, y + rowHeight / 2 + 4, 22, y + rowHeight / 2 - 5); } var image = item.__ProjectionImage; if (image is not null) graphics.DrawImage(image, new global::System.Drawing.Rectangle(7, y + global::System.Math.Max(2, (rowHeight - 20) / 2), 20, 20)); var text = item.__ProjectionText; if (!global::System.String.IsNullOrEmpty(text)) { var measured = graphics.MeasureString(text, font); graphics.DrawString(text, font, item.__ProjectionEnabled ? ink : disabledInk, 39f, y + global::System.Math.Max(2f, (rowHeight - measured.Height) / 2f)); } if (item is ToolStripDropDownItem child && child.DropDownItems.Count > 0) { var arrowX = Width - 14; var mid = y + rowHeight / 2; graphics.DrawLine(border, arrowX, mid - 4, arrowX + 4, mid); graphics.DrawLine(border, arrowX + 4, mid, arrowX, mid + 4); } y += rowHeight; } graphics.DrawRectangle(border, 0, 0, global::System.Math.Max(0, Width - 1), global::System.Math.Max(0, Height - 1)); }");
            Add("private void __PaintItems(global::System.Drawing.Graphics graphics) { if (this is ToolStripDropDown) { __PaintDropDownItems(graphics); return; } graphics.SmoothingMode = global::System.Drawing.Drawing2D.SmoothingMode.AntiAlias; using var background = new global::System.Drawing.SolidBrush(BackColor); graphics.FillRectangle(background, ClientRectangle); using var border = new global::System.Drawing.Pen(global::System.Drawing.Color.FromArgb(110, ForeColor)); using var face = new global::System.Drawing.SolidBrush(global::System.Drawing.Color.FromArgb(24, ForeColor)); using var ink = new global::System.Drawing.SolidBrush(ForeColor); using var font = new global::System.Drawing.Font(\"Portsmouth Rapids\", 10f, global::System.Drawing.FontStyle.Regular, global::System.Drawing.GraphicsUnit.Pixel, 1); var x = global::System.Math.Max(2, __gripMargin.Left); foreach (ToolStripItem item in __toolItems) { if (!item.__ProjectionVisible) continue; var margin = item.__ProjectionMargin; x += margin.Left; var width = __ItemWidth(item); var height = item.__ProjectionSize.Height > 0 ? global::System.Math.Min(Height - 2, item.__ProjectionSize.Height) : global::System.Math.Max(1, Height - 6); var top = global::System.Math.Max(1, (Height - height) / 2); if (item is ToolStripSeparator) { var separatorX = x + width / 2; graphics.DrawLine(border, separatorX, top + 4, separatorX, top + height - 4); } else { var bounds = new global::System.Drawing.Rectangle(x, top, width, height); graphics.FillRectangle(face, bounds); graphics.DrawRectangle(border, bounds.X, bounds.Y, global::System.Math.Max(0, bounds.Width - 1), global::System.Math.Max(0, bounds.Height - 1)); var image = item.__ProjectionImage; var text = item.__ProjectionText; if (image is not null) { var side = global::System.Math.Min(24, global::System.Math.Min(bounds.Width - 6, bounds.Height - 6)); if (side > 0) graphics.DrawImage(image, new global::System.Drawing.Rectangle(bounds.X + (bounds.Width - side) / 2, bounds.Y + (bounds.Height - side) / 2, side, side)); } else if (!global::System.String.IsNullOrEmpty(text)) { var measured = graphics.MeasureString(text, font); graphics.DrawString(text, font, ink, bounds.X + global::System.Math.Max(4f, (bounds.Width - measured.Width) / 2f), bounds.Y + global::System.Math.Max(2f, (bounds.Height - measured.Height) / 2f)); } } x += width + margin.Right; } if (!__toolTraceDone && global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_TOOLSTRIP\") == \"1\") { __toolTraceDone = true; var rows = new global::System.Collections.Generic.List<string>(); foreach (ToolStripItem item in __toolItems) { var image = item.__ProjectionImage; rows.Add(item.GetType().Name + \":text=\" + item.__ProjectionText + \",tip=\" + item.__ProjectionToolTip + \",size=\" + item.__ProjectionSize.Width + \"x\" + item.__ProjectionSize.Height + \",image=\" + (image is null ? \"none\" : image.Width + \"x\" + image.Height)); } global::System.Console.Error.WriteLine(\"facade-toolstrip=name:\" + Name + \"|size:\" + Width + \"x\" + Height + \"|items:\" + global::System.String.Join(\";\", rows)); } }");
            Add("private ToolStripItem? __ItemAt(int pointX, int pointY) { if (this is ToolStripDropDown) { var y = 4; foreach (ToolStripItem item in __toolItems) { if (!item.__ProjectionVisible) continue; var rowHeight = __DropDownRowHeight(item); if (item is not ToolStripSeparator && new global::System.Drawing.Rectangle(3, y, global::System.Math.Max(0, Width - 6), rowHeight).Contains(pointX, pointY)) return item; y += rowHeight; } return null; } var x = global::System.Math.Max(2, __gripMargin.Left); foreach (ToolStripItem item in __toolItems) { if (!item.__ProjectionVisible) continue; var margin = item.__ProjectionMargin; x += margin.Left; var width = __ItemWidth(item); var height = item.__ProjectionSize.Height > 0 ? global::System.Math.Min(Height - 2, item.__ProjectionSize.Height) : global::System.Math.Max(1, Height - 6); var top = global::System.Math.Max(1, (Height - height) / 2); if (item is not ToolStripSeparator && new global::System.Drawing.Rectangle(x, top, width, height).Contains(pointX, pointY)) return item; x += width + margin.Right; } return null; }");
            Add("protected override void OnPaint(PaintEventArgs e) { __PaintItems(e.Graphics); base.OnPaint(e); }");
            Add("protected override void OnMouseMove(MouseEventArgs e) { base.OnMouseMove(e); __SetHotItem(__ItemAt(e.X, e.Y), true); }");
            Add("protected override void OnMouseLeave(global::System.EventArgs e) { base.OnMouseLeave(e); __CancelHoverAction(); if (this is not ToolStripDropDown dropDown || !dropDown.__HasVisibleChild) __SetHotItem(null, false); }");
            Add("protected override void OnMouseUp(MouseEventArgs e) { base.OnMouseUp(e); if (e.Button != MouseButtons.Left) return; var item = __ItemAt(e.X, e.Y); if (item is null || !item.__ProjectionEnabled) return; __SetHotItem(item, false); __ActivateHotItem(); }");
            Add("protected override void OnKeyDown(KeyEventArgs e) { var handled = false; if (this is ToolStripDropDown dropDown && Visible) { switch (e.KeyCode) { case Keys.Down: __MoveHot(1, false); handled = true; break; case Keys.Up: __MoveHot(-1, false); handled = true; break; case Keys.Home: __MoveHot(-1, true); handled = true; break; case Keys.End: __MoveHot(1, true); handled = true; break; case Keys.Right: if (__hotItem is ToolStripDropDownItem branch && branch.DropDownItems.Count > 0) __OpenBranch(branch, true); handled = true; break; case Keys.Left: handled = dropDown.__CloseKeyboardLevel(); break; case Keys.Enter: case Keys.Space: __ActivateHotItem(); handled = true; break; case Keys.Escape: dropDown.__EscapeMenuLevel(); handled = true; break; } } if (handled) e.Handled = true; base.OnKeyDown(e); }");
            Add("protected override void Dispose(bool disposing) { if (disposing) { __CancelHoverAction(); if (this is ToolStripDropDown dropDown) dropDown.__CloseDropDown(); } base.Dispose(disposing); }");
            break;
        case "System.Windows.Forms.ToolStripItem":
            Add("private string __itemText = string.Empty;");
            Add("private string __itemName = string.Empty;");
            Add("private string __toolTipText = string.Empty;");
            Add("private object? __itemTag;");
            Add("private global::System.Drawing.Size __itemSize;");
            Add("private Padding __itemMargin;");
            Add("private global::System.Drawing.Image? __itemImage;");
            Add("private bool __itemVisible = true;");
            Add("private bool __itemEnabled = true;");
            Add("internal ToolStrip? __owner;");
            Add("internal string __ProjectionText { get { return __itemText; } }");
            Add("internal string __ProjectionName { get { return __itemName; } }");
            Add("internal global::System.Drawing.Image? __ProjectionImage { get { return __itemImage; } }");
            Add("internal global::System.Drawing.Size __ProjectionSize { get { return __itemSize; } }");
            Add("internal Padding __ProjectionMargin { get { return __itemMargin; } }");
            Add("internal bool __ProjectionVisible { get { return __itemVisible; } }");
            Add("internal bool __ProjectionEnabled { get { return __itemEnabled; } }");
            Add("internal string __ProjectionToolTip { get { return __toolTipText; } }");
            Add("private void __RefreshOwner() { __owner?.__RefreshItems(); }");
            Add("internal void __PerformClick() { if (__itemEnabled && __itemVisible) Click?.Invoke(this, global::System.EventArgs.Empty); }");
            break;
        case "System.Windows.Forms.ToolStripDropDownItem":
            Add("private readonly ToolStripDropDown __dropDown;");
            break;
        case "System.Windows.Forms.ToolStripDropDown":
            Add("private ToolStripItem? __ownerItem;");
            Add("private ToolStripDropDown? __parentDropDown;");
            Add("private ToolStripDropDown? __childDropDown;");
            Add("private bool __nativePopupActive;");
            Add("internal ToolStripDropDown __MenuRoot { get { var root = this; while (root.__parentDropDown is not null) root = root.__parentDropDown; return root; } }");
            Add("internal ToolStripDropDown __KeyboardTarget { get { var target = this; while (target.__childDropDown is not null && target.__childDropDown.Visible) target = target.__childDropDown; return target; } }");
            Add("internal bool __HasVisibleChild { get { return __childDropDown is not null && __childDropDown.Visible; } }");
            Add("internal bool __ContainsMenuTarget(Control target) { for (Control? candidate = target; candidate is not null; candidate = candidate.Parent) for (ToolStripDropDown? menu = this; menu is not null; menu = menu.__childDropDown) if (global::System.Object.ReferenceEquals(candidate, menu)) return true; return false; }");
            Add("private Form? __OverlayForm(Control? ownerControl) { if (Name.Length == 0) Name = ownerControl is not null && ownerControl.Name.Length != 0 ? ownerControl.Name + \".DropDown\" : __ownerItem?.Owner?.Name + \".DropDown\" ?? \"dropDown\"; __TraceToolItems(); return ownerControl?.FindForm() ?? __ownerItem?.Owner?.FindForm() ?? Application.__CurrentForm; }");
            Add("internal void __ShowChildDropDown(ToolStripDropDown child, global::System.Drawing.Point screenLocation, bool focusChild = false) { if (global::System.Object.ReferenceEquals(__childDropDown, child) && child.Visible) { if (focusChild) child.__FocusFirstItem(); return; } __childDropDown?.__CloseDropDown(); __childDropDown = child; child.__parentDropDown = this; child.__ShowDropDown(screenLocation, this); if (focusChild) child.__FocusFirstItem(); }");
            Add("internal void __CloseChildDropDown() { var child = __childDropDown; __childDropDown = null; child?.__CloseDropDown(); }");
            Add("internal bool __CloseKeyboardLevel() { var parent = __parentDropDown; if (parent is null) return false; __CloseDropDown(); parent.Focus(); return true; }");
            Add("internal void __EscapeMenuLevel() { var parent = __parentDropDown; if (parent is null) { __CloseDropDown(); return; } __CloseDropDown(); parent.Focus(); }");
            Add("internal void __CloseDropDown() { var wasRoot = __parentDropDown is null; var child = __childDropDown; __childDropDown = null; child?.__CloseDropDown(); if (__parentDropDown is not null && global::System.Object.ReferenceEquals(__parentDropDown.__childDropDown, this)) __parentDropDown.__childDropDown = null; __parentDropDown = null; Visible = false; var parent = Parent; if (parent is not null) parent.Controls.Remove(this); if (wasRoot) Application.__MenuClosed(this); if (__nativePopupActive) __RequestClose(); }");
            Add("internal void __CloseMenuChain() { var root = this; while (root.__parentDropDown is not null) root = root.__parentDropDown; root.__CloseDropDown(); }");
            Add("internal void __ShowDropDown(global::System.Drawing.Point screenLocation, Control? ownerControl = null) { if (Items.Count == 0) return; var form = __OverlayForm(ownerControl); if (__parentDropDown is null) Application.__RegisterMenu(this); if (form is not null) { var clientLocation = form.PointToClient(screenLocation); __PrepareDropDown(clientLocation); var x = clientLocation.X; var y = clientLocation.Y; if (ownerControl is ToolStripDropDown parentDropDown && x + Width > form.Width) x = parentDropDown.Left - Width; x = global::System.Math.Clamp(x, 0, global::System.Math.Max(0, form.Width - Width)); y = global::System.Math.Clamp(y, 0, global::System.Math.Max(0, form.Height - Height)); Bounds = new global::System.Drawing.Rectangle(x, y, Width, Height); if (!global::System.Object.ReferenceEquals(Parent, form)) { Parent?.Controls.Remove(this); form.Controls.Add(this); } __RaiseLoad(); Visible = true; BringToFront(); __RenderManagedPaint(); return; } var work = Screen.GetWorkingArea(screenLocation); __PrepareDropDown(screenLocation); var screenX = global::System.Math.Clamp(screenLocation.X, work.Left, global::System.Math.Max(work.Left, work.Right - Width)); var screenY = global::System.Math.Clamp(screenLocation.Y, work.Top, global::System.Math.Max(work.Top, work.Bottom - Height)); Bounds = new global::System.Drawing.Rectangle(screenX, screenY, Width, Height); __RaiseLoad(); __RenderManagedPaint(); Visible = true; __nativePopupActive = true; try { _ = __RunNativePopup(); } finally { __nativePopupActive = false; Visible = false; if (__parentDropDown is null) Application.__MenuClosed(this); } }");
            break;
        case "System.Windows.Forms.ToolStripProfessionalRenderer":
            Add("private ProfessionalColorTable __colorTable = new();");
            Add("private bool __roundedEdges = true;");
            break;
        case "System.Windows.Forms.ToolStripButton":
            Add("private bool __toolButtonChecked;");
            break;
        case "System.Windows.Forms.ToolStripManager":
            Add("private static ToolStripManagerRenderMode __managerRenderMode = ToolStripManagerRenderMode.Professional;");
            Add("private static ToolStripRenderer __managerRenderer = new ToolStripProfessionalRenderer();");
            break;
        case "System.Windows.Forms.ToolStripItemCollection":
            Add("private readonly global::System.Collections.Generic.List<ToolStripItem> __items = new();");
            Add("private readonly ToolStrip? __owner;");
            Add("internal ToolStripItemCollection(ToolStrip owner) { __owner = owner; }");
            Add("public override int Count { get { return __items.Count; } }");
            Add("public override global::System.Collections.IEnumerator GetEnumerator() { return __items.GetEnumerator(); }");
            break;
        case "System.Windows.Forms.ToolStripMenuItem":
            Add("private bool __menuChecked;");
            break;
        case "System.Windows.Forms.Padding":
            Add("private int __left;");
            Add("private int __top;");
            Add("private int __right;");
            Add("private int __bottom;");
            break;
        case "System.Windows.Forms.PaintEventArgs":
            Add("private global::System.Drawing.Graphics? __graphics;");
            Add("private global::System.Drawing.Rectangle __clipRectangle;");
            Add("internal PaintEventArgs(global::System.Drawing.Graphics graphics, global::System.Drawing.Rectangle clipRectangle) { __graphics = graphics; __clipRectangle = clipRectangle; }");
            break;
        case "System.Windows.Forms.MouseEventArgs":
            Add("private MouseButtons __button;");
            Add("private int __clicks;");
            Add("private int __x;");
            Add("private int __y;");
            Add("private int __delta;");
            break;
        case "System.Windows.Forms.KeyEventArgs":
            Add("private Keys __keyData;");
            Add("private bool __handled;");
            break;
        case "System.Windows.Forms.Form":
            Add("private DialogResult __dialogResult;");
            Add("private bool __controlBox = true;");
            Add("private bool __topLevel = true;");
            Add("private bool __rightToLeftLayout;");
            Add("private FormWindowState __windowState;");
            Add("private Form? __mdiParent;");
            Add("private MenuStrip? __mainMenuStrip;");
            Add("private global::System.Drawing.Icon? __formIcon;");
            Add("private IButtonControl? __acceptButton;");
            Add("private IButtonControl? __cancelButton;");
            Add("private readonly CreateParams __createParams = new();");
            Add("private FormBorderStyle __formBorderStyle = FormBorderStyle.Sizable;");
            Add("private bool __keyPreview;");
            Add("private bool __maximizeBox = true;");
            Add("private bool __minimizeBox = true;");
            Add("private double __opacity = 1d;");
            Add("private Form? __ownerForm;");
            Add("private bool __showIcon = true;");
            Add("private bool __showInTaskbar = true;");
            Add("private SizeGripStyle __sizeGripStyle;");
            Add("private FormStartPosition __startPosition;");
            Add("private bool __topMost;");
            break;
    }
}

static string ConstructorBody(Type type, ConstructorInfo? constructor)
{
    var count = constructor?.GetParameters().Length ?? 0;
    return type.FullName switch
    {
        "System.Drawing.Bitmap" when count is 2 or 3 => "{ if (width <= 0) throw new global::System.ArgumentOutOfRangeException(nameof(width)); if (height <= 0) throw new global::System.ArgumentOutOfRangeException(nameof(height)); __width = width; __height = height; __pixelFormat = " + (count == 3 ? "format" : "global::System.Drawing.Imaging.PixelFormat.Format32bppPArgb") + "; __bitmap = NativeDrawingBridge.BitmapCreate(width, height); }",
        "System.Drawing.Bitmap" when count == 1 && constructor!.GetParameters()[0].ParameterType.FullName == "System.IO.Stream" => "{ if (stream is null) throw new global::System.ArgumentNullException(nameof(stream)); using var copy = new global::System.IO.MemoryStream(); stream.CopyTo(copy); var decoded = NativeDrawingBridge.DecodePng(copy.ToArray()); __width = decoded.__width; __height = decoded.__height; __pixelFormat = decoded.__pixelFormat; __bitmap = decoded.__bitmap; decoded.__bitmap = default; decoded.Dispose(); }",
        "System.Drawing.Bitmap" when count == 1 => "{ if (original is null) throw new global::System.ArgumentNullException(nameof(original)); __width = original.Width; __height = original.Height; __pixelFormat = original.PixelFormat; __bitmap = NativeDrawingBridge.BitmapClone(original.__BitmapHandle, 0, 0, __width, __height); }",
        "System.Drawing.Icon" when count == 1 => "{ if (stream is null) throw new global::System.ArgumentNullException(nameof(stream)); __iconData = NativeDrawingBridge.ReadBounded(stream, \"ICO\"); }",
        "System.Drawing.SolidBrush" when count == 1 => "{ __color = color; __handle = NativeDrawingBridge.SolidBrushCreate(color); }",
        "System.Drawing.Pen" when count is 1 or 2 && constructor!.GetParameters()[0].ParameterType.FullName == "System.Drawing.Color" => "{ __handle = NativeDrawingBridge.PenCreate(color, " + (count == 2 ? "width" : "1f") + "); }",
        "System.Drawing.Pen" when count is 1 or 2 => "{ if (brush is null) throw new global::System.ArgumentNullException(nameof(brush)); __handle = NativeDrawingBridge.PenCreate(brush.__color, " + (count == 2 ? "width" : "1f") + "); }",
        "System.Drawing.Font" when count == 2 && constructor!.GetParameters()[0].ParameterType.FullName == "System.String" => "{ __family = familyName ?? throw new global::System.ArgumentNullException(nameof(familyName)); __size = emSize; __handle = NativeDrawingBridge.FontCreate(__family, __size, __style, __unit, 1); }",
        "System.Drawing.Font" when count == 2 => "{ if (prototype is null) throw new global::System.ArgumentNullException(nameof(prototype)); __family = prototype.__family; __size = prototype.__size; __style = newStyle; __unit = prototype.__unit; __handle = NativeDrawingBridge.FontCreate(__family, __size, __style, __unit, 1); }",
        "System.Drawing.Font" when count == 5 => "{ __family = familyName ?? throw new global::System.ArgumentNullException(nameof(familyName)); __size = emSize; __style = style; __unit = unit; __handle = NativeDrawingBridge.FontCreate(__family, __size, __style, __unit, gdiCharSet); }",
        "System.Drawing.FontFamily" when count == 1 => "{ __name = name ?? throw new global::System.ArgumentNullException(nameof(name)); }",
        "System.Drawing.StringFormat" when count == 0 => "{ __handle = NativeDrawingBridge.StringFormatCreate(0); }",
        "System.Drawing.StringFormat" when count == 1 && constructor!.GetParameters()[0].ParameterType.FullName == "System.Drawing.StringFormatFlags" => "{ __flags = options; __handle = NativeDrawingBridge.StringFormatCreate((uint)options); }",
        "System.Drawing.StringFormat" when count == 1 => "{ if (format is null) throw new global::System.ArgumentNullException(nameof(format)); __alignment = format.__alignment; __lineAlignment = format.__lineAlignment; __trimming = format.__trimming; __flags = format.__flags; __handle = NativeDrawingBridge.StringFormatCreate((uint)__flags); NativeDrawingBridge.StringFormatSet(__handle, __alignment, __lineAlignment, __trimming, __flags); }",
        // Bind an empty recorder to the thread that first uses Graphics. retired compatibility specimen
        // constructs some Graphics instances before handing them to its paint
        // thread; post-first-use operations remain owner-thread enforced.
        "System.Drawing.Graphics" => "{ }",
        "System.Drawing.Drawing2D.GraphicsPath" => "{ __handle = global::System.Drawing.NativeDrawingBridge.GraphicsPathCreate(); }",
        "System.Drawing.Drawing2D.Matrix" when count == 2 => "{ if (rect.Width == 0 || rect.Height == 0) throw new global::System.ArgumentException(\"Matrix source rectangle is empty.\", nameof(rect)); if (plgpts is null || plgpts.Length != 3) throw new global::System.ArgumentException(\"Matrix parallelogram requires three points.\", nameof(plgpts)); __m11 = (plgpts[1].X - plgpts[0].X) / (float)rect.Width; __m12 = (plgpts[1].Y - plgpts[0].Y) / (float)rect.Width; __m21 = (plgpts[2].X - plgpts[0].X) / (float)rect.Height; __m22 = (plgpts[2].Y - plgpts[0].Y) / (float)rect.Height; __dx = plgpts[0].X - rect.X * __m11 - rect.Y * __m21; __dy = plgpts[0].Y - rect.X * __m12 - rect.Y * __m22; }",
        "System.Drawing.Drawing2D.HatchBrush" when count == 3 => "{ __color = foreColor; __handle = global::System.Drawing.NativeDrawingBridge.HatchBrushCreate((uint)hatchstyle, foreColor, backColor); }",
        "System.Drawing.Drawing2D.LinearGradientBrush" when count == 4 => "{ var bounds = new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height); var angle = linearGradientMode switch { global::System.Drawing.Drawing2D.LinearGradientMode.Vertical => 90f, global::System.Drawing.Drawing2D.LinearGradientMode.ForwardDiagonal => 45f, global::System.Drawing.Drawing2D.LinearGradientMode.BackwardDiagonal => 135f, _ => 0f }; __color = color1; __handle = global::System.Drawing.NativeDrawingBridge.LinearGradientBrushCreate(bounds, color1, color2, angle, global::System.Drawing.Drawing2D.WrapMode.Tile); }",
        "System.Drawing.Drawing2D.PathGradientBrush" when count == 1 => "{ if (path is null) throw new global::System.ArgumentNullException(nameof(path)); __handle = global::System.Drawing.NativeDrawingBridge.PathGradientBrushCreate(global::System.Drawing.NativeDrawingBridge.GraphicsPathPoints(path.__handle)); }",
        "System.Drawing.Drawing2D.ColorBlend" when count == 1 => "{ if (count < 0) throw new global::System.ArgumentOutOfRangeException(nameof(count)); __colors = new global::System.Drawing.Color[count]; __positions = new float[count]; }",
        "System.Drawing.Imaging.ImageAttributes" => "{ __handle = global::System.Drawing.NativeDrawingBridge.ImageAttributesCreate(); }",
        "System.Drawing.Region" when count == 1 && constructor!.GetParameters()[0].ParameterType.FullName == "System.Drawing.Rectangle" => "{ __handle = NativeDrawingBridge.RegionCreate(new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height)); }",
        "System.Drawing.Region" when count == 1 => "{ if (path is null) throw new global::System.ArgumentNullException(nameof(path)); __handle = NativeDrawingBridge.RegionCreate(path.__handle); }",
        "System.Windows.Forms.Control" => "{ __native = NativeControlBridge.Create(GetType()); __native.Changed += __NativeChanged; __native.NativeEventRaised += __NativeEvent; __native.PointerRaised += __NativePointer; __native.KeyRaised += input => __NativeKeyInput(input.PhysicalKey, input.Kind == 11u, input.Modifiers, input.Repeat); __native.TextRaised += __NativeTextInput; __controls = new ControlCollection(this); __ApplyEffectiveColors(); }",
        "System.Windows.Forms.UserControl" => "{ Size = new global::System.Drawing.Size(150, 150); }",
        "System.Windows.Forms.Button" => "{ Size = new global::System.Drawing.Size(75, 23); }",
        "System.Windows.Forms.ListBox" => "{ __listItems = new ObjectCollection(this); }",
        "System.Windows.Forms.SplitContainer" => "{ Controls.Add(__panel1); Controls.Add(__panel2); }",
        "System.Windows.Forms.TableLayoutPanel" => "{ __tableControls = new TableLayoutControlCollection(this); __controls = __tableControls; }",
        "System.Windows.Forms.UpDownBase" => "{ Controls.Add(new Button { Name = \"upDownButtons\", Visible = false }); Controls.Add(new TextBox { Name = \"upDownEdit\", Visible = false }); }",
        "System.Windows.Forms.NumericUpDown" => "{ __UpdateNumericText(); }",
        "System.Windows.Forms.ComboBox" => "{ __comboItems = new ObjectCollection(this); }",
        "System.Windows.Forms.DataGridView" => "{ __gridColumns = new DataGridViewColumnCollection(this); __gridRows = new DataGridViewRowCollection(this); }",
        "System.Windows.Forms.DataGridViewColumn" when count == 1 => "{ __cellTemplate = cellTemplate ?? throw new global::System.ArgumentNullException(nameof(cellTemplate)); }",
        "System.Windows.Forms.BindingSource" when count == 1 => "{ container?.Add(this); }",
        "System.Windows.Forms.ToolStrip" => "{ __toolItems = new ToolStripItemCollection(this); }",
        "System.Windows.Forms.ToolStripDropDownItem" => "{ __dropDown = new ToolStripDropDown { OwnerItem = this }; }",
        "System.Windows.Forms.ToolStripProfessionalRenderer" when count == 1 => "{ __colorTable = professionalColorTable ?? throw new global::System.ArgumentNullException(nameof(professionalColorTable)); }",
        "System.Windows.Forms.RowStyle" when count == 1 => "{ __sizeType = sizeType; }",
        "System.Windows.Forms.RowStyle" when count == 2 => "{ __sizeType = sizeType; __size = height; }",
        "System.Windows.Forms.ColumnStyle" when count == 1 => "{ __sizeType = sizeType; }",
        "System.Windows.Forms.ColumnStyle" when count == 2 => "{ __sizeType = sizeType; __size = width; }",
        "System.Windows.Forms.Padding" when count == 1 => "{ __left = __top = __right = __bottom = all; }",
        "System.Windows.Forms.Padding" when count == 4 => "{ __left = left; __top = top; __right = right; __bottom = bottom; }",
        "System.Windows.Forms.MouseEventArgs" when count == 5 => "{ __button = button; __clicks = clicks; __x = x; __y = y; __delta = delta; }",
        "System.Windows.Forms.KeyEventArgs" when count == 1 => "{ __keyData = keyData; }",
        "System.Windows.Forms.Timer" when count == 1 => "{ container?.Add(this); }",
        "System.Windows.Forms.ToolTip" when count == 1 => "{ cont?.Add(this); }",
        "System.Windows.Forms.ToolStripMenuItem" when count == 1 => "{ Text = text ?? string.Empty; }",
        _ when type.IsValueType => "{ this = default; }",
        _ => "{ }",
    };
}

static string PropertyBody(Type type, PropertyInfo property, HashSet<MethodInfo> accessors)
{
    var get = property.GetMethod is not null && accessors.Contains(property.GetMethod);
    var set = property.SetMethod is not null && accessors.Contains(property.SetMethod);
    string Stub() => StubProperty(type, property, type.IsInterface, get, set);
    if (type.FullName == "System.Windows.Forms.Application" && property.Name == "ExecutablePath")
        return "{ get { return global::System.Environment.ProcessPath ?? global::System.AppContext.BaseDirectory; } }";
    if (type.FullName == "System.Windows.Forms.Cursor")
        return property.Name switch
        {
            "Current" => "{ set { __current = value; } }",
            "Position" => "{ get { return __position; } set { __position = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DateTimePicker")
        return property.Name switch
        {
            "CalendarForeColor" => "{ set { __calendarForeColor = value; } }",
            "CalendarMonthBackground" => "{ set { __calendarMonthBackground = value; } }",
            "CustomFormat" => "{ get { return __customFormat; } set { __customFormat = value ?? string.Empty; } }",
            "Format" => "{ set { __dateTimeFormat = value; } }",
            "MaxDate" => "{ set { if (value < __minimumDate) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __maximumDate = value; if (__dateTimeValue > value) __dateTimeValue = value; } }",
            "MinDate" => "{ set { if (value > __maximumDate) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __minimumDate = value; if (__dateTimeValue < value) __dateTimeValue = value; } }",
            "Value" => "{ get { return __dateTimeValue; } set { if (value < __minimumDate || value > __maximumDate) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __dateTimeValue = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.Screen")
        return property.Name switch
        {
            "AllScreens" => "{ get { return new[] { new Screen() }; } }",
            "Bounds" => "{ get { return __screenBounds; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.SystemInformation")
        return property.Name switch
        {
            "DragSize" => "{ get { return new global::System.Drawing.Size(4, 4); } }",
            "HorizontalScrollBarArrowWidth" => "{ get { return 17; } }",
            "MouseHoverTime" => "{ get { return 400; } }",
            "VirtualScreen" => "{ get { return new global::System.Drawing.Rectangle(0, 0, 1920, 1080); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.Layout.ArrangedElementCollection" && property.Name == "Count")
        return "{ get { return 0; } }";
    if (type.FullName == "System.Windows.Forms.Label")
        return property.Name switch
        {
            "AutoEllipsis" => "{ set { __autoEllipsis = value; Invalidate(); } }",
            "TextAlign" => "{ set { __labelTextAlign = value; Invalidate(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.LinkLabel")
        return property.Name switch
        {
            "Links" => "{ get { return __links; } }",
            "Text" => "{ get { return base.Text; } set { base.Text = value ?? string.Empty; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.LinkLabel+Link")
        return property.Name switch
        {
            "Start" => "{ set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __linkStart = value; } }",
            "Length" => "{ set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __linkLength = value; } }",
            "LinkData" => "{ get { return __linkData!; } set { __linkData = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.LinkLabelLinkClickedEventArgs" && property.Name == "Link")
        return "{ get { return __clickedLink!; } }";
    if (type.FullName == "System.Windows.Forms.ColorDialog")
        return property.Name switch
        {
            "AnyColor" => "{ set { __anyColor = value; } }",
            "Color" => "{ get { return __dialogColor; } set { __dialogColor = value; } }",
            "FullOpen" => "{ set { __fullOpen = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.FileDialog")
        return property.Name switch
        {
            "DefaultExt" => "{ set { __defaultExt = value ?? string.Empty; } }",
            "FileName" => "{ get { return __fileName; } set { __fileName = value ?? string.Empty; } }",
            "Filter" => "{ set { __filter = value ?? string.Empty; } }",
            "InitialDirectory" => "{ get { return __initialDirectory; } set { __initialDirectory = value ?? string.Empty; } }",
            "RestoreDirectory" => "{ set { __restoreDirectory = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.FolderBrowserDialog" && property.Name == "SelectedPath")
        return "{ get { return __selectedPath; } set { __selectedPath = value ?? string.Empty; } }";
    if (type.FullName == "System.Windows.Forms.OpenFileDialog" && property.Name == "SafeFileName")
        return "{ get { return global::System.IO.Path.GetFileName(__fileName); } }";
    if (type.FullName == "System.Windows.Forms.ToolTip")
        return property.Name switch
        {
            "Active" => "{ set { __toolTipActive = value; if (!value) { if (__shownWindow is Control control) __CancelPending(control, true); __shownWindow = null; __shownText = string.Empty; } } }",
            "AutoPopDelay" => "{ set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __autoPopDelay = value; } }",
            "AutomaticDelay" => "{ set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __initialDelay = value; __reshowDelay = value / 5; __autoPopDelay = checked(value * 10); } }",
            "InitialDelay" => "{ set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __initialDelay = value; } }",
            "ReshowDelay" => "{ set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __reshowDelay = value; } }",
            "ShowAlways" => "{ set { __showAlways = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DrawItemEventArgs")
        return property.Name switch
        {
            "Bounds" => "{ get { return __drawBounds; } }",
            "Graphics" => "{ get { return __drawGraphics!; } }",
            "Index" => "{ get { return __drawIndex; } }",
            "State" => "{ get { return __drawState; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ConvertEventArgs" && property.Name == "Value")
        return "{ get { return __convertedValue!; } set { __convertedValue = value; } }";
    if (type.FullName == "System.Windows.Forms.CreateParams" && property.Name == "ExStyle")
        return "{ get { return __extendedStyle; } set { __extendedStyle = value; } }";
    if (type.FullName == "System.Windows.Forms.FormClosingEventArgs" && property.Name == "CloseReason")
        return "{ get { return __closeReason; } }";
    if (type.FullName == "System.Windows.Forms.PreviewKeyDownEventArgs" && property.Name == "KeyCode")
        return "{ get { return __previewKeyCode; } }";
    if (type.FullName == "System.Windows.Forms.ToolStripItemRenderEventArgs")
        return property.Name switch
        {
            "Graphics" => "{ get { return __renderGraphics!; } }",
            "Item" => "{ get { return __renderItem!; } }",
            "ToolStrip" => "{ get { return __renderToolStrip!; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripItemImageRenderEventArgs")
        return property.Name switch
        {
            "Image" => "{ get { return __renderImage!; } }",
            "ImageRectangle" => "{ get { return __renderImageRectangle; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripItemTextRenderEventArgs")
        return property.Name switch
        {
            "Text" => "{ get { return __renderText; } }",
            "TextFont" => "{ get { return __renderTextFont!; } }",
            "TextFormat" => "{ get { return __renderTextFormat; } }",
            "TextRectangle" => "{ get { return __renderTextRectangle; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripRenderEventArgs")
        return property.Name switch
        {
            "Graphics" => "{ get { return __stripGraphics!; } }",
            "ToolStrip" => "{ get { return __strip!; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripArrowRenderEventArgs")
        return property.Name switch
        {
            "ArrowColor" => "{ set { __arrowColor = value; } }",
            "Item" => "{ get { return __arrowItem!; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripGripRenderEventArgs" && property.Name == "GripBounds")
        return "{ get { return __gripBounds; } }";
    if (type.FullName == "System.Windows.Forms.ToolStripSeparatorRenderEventArgs" && property.Name == "Vertical")
        return "{ get { return __verticalSeparator; } }";
    if (type.FullName == "System.Drawing.Image")
        return property.Name switch
        {
            "Width" => "{ get { return __width; } }",
            "Height" => "{ get { return __height; } }",
            "PixelFormat" => "{ get { return __pixelFormat; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Drawing.SolidBrush" && property.Name == "Color")
        return "{ get { return __color; } }";
    if (type.FullName == "System.Drawing.Font" && property.Name == "Height")
        return "{ get { return global::System.Math.Max(1, (int)global::System.Math.Ceiling(__size * 1.2f)); } }";
    if (type.FullName == "System.Drawing.Pen")
        return property.Name switch
        {
            "Width" => "{ set { NativeDrawingBridge.PenSetWidth(__handle, value); } }",
            "DashStyle" => "{ set { NativeDrawingBridge.PenSetDashStyle(__handle, (uint)value); } }",
            "DashPattern" => "{ set { NativeDrawingBridge.PenSetDashPattern(__handle, value); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Drawing.Imaging.ImageFormat" && property.Name == "Png")
        return "{ get { return __png; } }";
    if (type.FullName == "System.Drawing.SystemFonts")
        return "{ get { return new global::System.Drawing.Font(\"Lucida Grande\", 12f); } }";
    if (type.FullName == "System.Drawing.Brushes")
        return "{ get { return new global::System.Drawing.SolidBrush(global::System.Drawing.Color.FromName(\"" + property.Name + "\")); } }";
    if (type.FullName == "System.Drawing.Pens")
        return "{ get { return new global::System.Drawing.Pen(global::System.Drawing.Color.FromName(\"" + property.Name + "\")); } }";
    if (type.FullName == "System.Drawing.StringFormat")
        return property.Name switch
        {
            "Alignment" => "{ set { __alignment = value; NativeDrawingBridge.StringFormatSet(__handle, __alignment, __lineAlignment, __trimming, __flags); } }",
            "LineAlignment" => "{ set { __lineAlignment = value; NativeDrawingBridge.StringFormatSet(__handle, __alignment, __lineAlignment, __trimming, __flags); } }",
            "Trimming" => "{ set { __trimming = value; NativeDrawingBridge.StringFormatSet(__handle, __alignment, __lineAlignment, __trimming, __flags); } }",
            "FormatFlags" => "{ get { return __flags; } set { __flags = value; NativeDrawingBridge.StringFormatSet(__handle, __alignment, __lineAlignment, __trimming, __flags); } }",
            "GenericDefault" or "GenericTypographic" => "{ get { return new global::System.Drawing.StringFormat(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Drawing.Graphics")
        return property.Name switch
        {
            "ClipBounds" => "{ get { return __target is null ? global::System.Drawing.RectangleF.Empty : new global::System.Drawing.RectangleF(0, 0, __target.Width, __target.Height); } }",
            "SmoothingMode" => "{ get { return __smoothing; } set { __smoothing = value; __EnsureRecorder(); NativeDrawingBridge.RecorderQuality(this); } }",
            "InterpolationMode" => "{ get { return __interpolation; } set { __interpolation = value; __EnsureRecorder(); NativeDrawingBridge.RecorderQuality(this); } }",
            "PixelOffsetMode" => "{ set { __pixelOffset = value; __EnsureRecorder(); NativeDrawingBridge.RecorderQuality(this); } }",
            "CompositingMode" => "{ set { __compositing = value; __EnsureRecorder(); NativeDrawingBridge.RecorderQuality(this); } }",
            "CompositingQuality" => "{ set { __compositingQuality = value; __EnsureRecorder(); NativeDrawingBridge.RecorderQuality(this); } }",
            "Transform" => "{ get { return __transform; } set { __transform = value ?? throw new global::System.ArgumentNullException(nameof(value)); __EnsureRecorder(); NativeDrawingBridge.RecorderSetTransform(__recorder, __transform); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Drawing.Drawing2D.GraphicsPath" && property.Name == "PathPoints")
        return "{ get { return global::System.Drawing.NativeDrawingBridge.GraphicsPathPoints(__handle); } }";
    if (type.FullName == "System.Drawing.Drawing2D.ColorBlend")
        return property.Name switch
        {
            "Colors" => "{ get { return __colors; } set { __colors = value ?? throw new global::System.ArgumentNullException(nameof(value)); } }",
            "Positions" => "{ get { return __positions; } set { __positions = value ?? throw new global::System.ArgumentNullException(nameof(value)); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Drawing.Drawing2D.LinearGradientBrush")
        return property.Name switch
        {
            "InterpolationColors" => "{ set { if (value is null) throw new global::System.ArgumentNullException(nameof(value)); global::System.Drawing.NativeDrawingBridge.LinearGradientSetInterpolation(__handle, value.__colors, value.__positions); } }",
            "WrapMode" => "{ set { global::System.Drawing.NativeDrawingBridge.LinearGradientSetWrap(__handle, value); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Drawing.Drawing2D.PathGradientBrush")
        return property.Name switch
        {
            "CenterColor" => "{ set { global::System.Drawing.NativeDrawingBridge.PathGradientSetCenterColor(__handle, value); } }",
            "SurroundColors" => "{ set { global::System.Drawing.NativeDrawingBridge.PathGradientSetSurroundColors(__handle, value); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Drawing.Imaging.ColorMap")
        return property.Name switch
        {
            "OldColor" => "{ set { __oldColor = value; } }",
            "NewColor" => "{ set { __newColor = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Drawing.Imaging.ColorMatrix" && property.Name == "Matrix33")
        return "{ set { __values[18] = value; } }";
    if (type.FullName == "System.Drawing.Imaging.BitmapData")
        return property.Name switch
        {
            "Scan0" => "{ get { return __scan0; } }",
            "Stride" => "{ get { return __stride; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.BaseCollection" && property.Name == "Count")
        return "{ get { return __Collection.Count; } }";
    if (type.FullName == "System.Windows.Forms.BindingSource")
        return property.Name switch
        {
            "Current" => "{ get { return __position >= 0 && __position < __bindingList.Count ? __bindingList[__position]! : null!; } }",
            "DataSource" => "{ set { __SetDataSource(value); } }",
            "List" => "{ get { return __bindingList; } }",
            "Position" => "{ set { __position = __bindingList.Count == 0 ? -1 : global::System.Math.Clamp(value, -1, __bindingList.Count - 1); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.Control")
    {
        return property.Name switch
        {
            "AllowDrop" => "{ set { __allowDrop = value; } }",
            "Anchor" => "{ get { return __anchor; } set { __anchor = value; } }",
            "AutoSize" => "{ set { if (__autoSize == value) return; __autoSize = value; if (value) __RefreshAutoSizeFromChildren(); else __parent?.PerformLayout(); } }",
            "BackColor" => "{ get { return !__backColor.IsEmpty ? __backColor : __parent is not null ? __parent.BackColor : global::System.Drawing.Color.FromArgb(229, 234, 239); } set { if (__backColor == value) return; __backColor = value; __ApplyEffectiveColors(); OnBackColorChanged(global::System.EventArgs.Empty); if (__loadRaised) __QueueManagedPaint(); } }",
            "BackgroundImage" => "{ set { __backgroundImage = value; if (__loadRaised) __RenderManagedPaint(); } }",
            "BackgroundImageLayout" => "{ set { __backgroundImageLayout = value; if (__loadRaised) __RenderManagedPaint(); } }",
            "Bottom" => "{ get { return Bounds.Bottom; } }",
            "Name" => "{ get { return __native.Name; } set { __native.Name = value ?? string.Empty; } }",
            "Text" => "{ get { return __native.Text; } set { __native.Text = value ?? string.Empty; } }",
            "Visible" => "{ get { return __native.Visible; } set { if (value) __RaiseLoad(); __native.Visible = value; } }",
            "Enabled" => "{ get { return __native.Enabled; } set { __native.Enabled = value; } }",
            "Bounds" => "{ get { return __native.Bounds; } set { var normalized = new global::System.Drawing.Rectangle(value.X, value.Y, global::System.Math.Max(0, value.Width), global::System.Math.Max(0, value.Height)); var previous = __native.Bounds; if (previous == normalized) return; __native.Bounds = normalized; if (previous.Location != normalized.Location) Move?.Invoke(this, global::System.EventArgs.Empty); if (previous.Size != normalized.Size) { OnSizeChanged(global::System.EventArgs.Empty); OnResize(global::System.EventArgs.Empty); ClientSizeChanged?.Invoke(this, global::System.EventArgs.Empty); if (__parent is not null || __loadRaised) PerformLayout(); __parent?.__RefreshAutoSizeFromChildren(); if (__loadRaised) __RenderManagedPaint(); } } }",
            "Capture" => "{ get { return __native.Capture; } set { __native.Capture = value; } }",
            "CheckForIllegalCrossThreadCalls" => "{ set { __checkForIllegalCrossThreadCalls = value; } }",
            "ClientRectangle" => "{ get { return new global::System.Drawing.Rectangle(0, 0, Width, Height); } }",
            "ClientSize" => "{ get { return Size; } set { Size = value; } }",
            "ContainsFocus" => "{ get { return global::System.Object.ReferenceEquals(__focusedControl, this) || (__focusedControl is not null && __ContainsDescendant(__focusedControl)); } }",
            "ContextMenuStrip" => "{ get { return __contextMenuStrip!; } set { __contextMenuStrip = value; } }",
            "Controls" => "{ get { return __controls; } }",
            "Cursor" => "{ get { return __cursor!; } set { __cursor = value; } }",
            "DefaultFont" => "{ get { return global::System.Drawing.SystemFonts.DefaultFont; } }",
            "DisplayRectangle" => "{ get { return ClientRectangle; } }",
            "Disposing" => "{ get { return __native.IsDisposed; } }",
            "Dock" => "{ get { return __dock; } set { if (__dock == value) return; __dock = value; __parent?.PerformLayout(); } }",
            "DoubleBuffered" => "{ set { __doubleBuffered = value; } }",
            "Focused" => "{ get { return global::System.Object.ReferenceEquals(__focusedControl, this); } }",
            "Font" => "{ get { return __font ?? global::System.Drawing.SystemFonts.DefaultFont; } set { if (global::System.Object.ReferenceEquals(__font, value)) return; __font = value; OnFontChanged(global::System.EventArgs.Empty); if (__loadRaised) __QueueManagedPaint(); } }",
            "ForeColor" => "{ get { return !__foreColor.IsEmpty ? __foreColor : __parent is not null ? __parent.ForeColor : global::System.Drawing.Color.FromArgb(27, 39, 51); } set { if (__foreColor == value) return; __foreColor = value; __ApplyEffectiveColors(); OnForeColorChanged(global::System.EventArgs.Empty); if (__loadRaised) __QueueManagedPaint(); } }",
            "Handle" => "{ get { return __native.WindowHandle; } }",
            "Height" => "{ get { return Bounds.Height; } set { var bounds = Bounds; bounds.Height = value; Bounds = bounds; } }",
            "InvokeRequired" => "{ get { return __native.InvokeRequired; } }",
            "IsDisposed" => "{ get { return __native.IsDisposed; } }",
            "IsHandleCreated" => "{ get { return __native.HasWindowHandle; } }",
            "Left" => "{ get { return Bounds.Left; } set { var bounds = Bounds; bounds.X = value; Bounds = bounds; } }",
            "Location" => "{ get { return Bounds.Location; } set { var bounds = Bounds; bounds.Location = value; Bounds = bounds; } }",
            "Margin" => "{ set { __margin = value; } }",
            "MaximumSize" => "{ set { __maximumSize = value; } }",
            "MinimumSize" => "{ set { __minimumSize = value; } }",
            "ModifierKeys" => "{ get { return Keys.None; } }",
            "MousePosition" => "{ get { return global::System.Drawing.Point.Empty; } }",
            "Padding" => "{ get { return __padding; } set { __padding = value; PerformLayout(); } }",
            "Parent" => "{ get { return __parent!; } set { __SetParent(value); } }",
            "Region" => "{ get { return __region!; } set { __region = value; } }",
            "Right" => "{ get { return Bounds.Right; } }",
            "RightToLeft" => "{ get { return __rightToLeft; } set { __rightToLeft = value; } }",
            "Size" => "{ get { return Bounds.Size; } set { var bounds = Bounds; bounds.Size = value; Bounds = bounds; } }",
            "TabIndex" => "{ set { __tabIndex = value; } }",
            "TabStop" => "{ set { __tabStop = value; } }",
            "Tag" => "{ get { return __tag!; } set { __tag = value; } }",
            "Top" => "{ get { return Bounds.Top; } set { var bounds = Bounds; bounds.Y = value; Bounds = bounds; } }",
            "Width" => "{ get { return Bounds.Width; } set { var bounds = Bounds; bounds.Width = value; Bounds = bounds; } }",
            _ => Stub(),
        };
    }
    if (type.FullName == "System.Windows.Forms.TextBoxBase")
        return property.Name switch
        {
            "BorderStyle" => "{ get { return __textBorderStyle; } set { __textBorderStyle = value; } }",
            "Multiline" => "{ get { return __textMultiline; } set { __textMultiline = value; } }",
            "ReadOnly" => "{ get { return __textReadOnly; } set { __textReadOnly = value; } }",
            "SelectedText" => "{ get { return Text.Substring(__TextSelectionStart, __TextSelectionLength); } set { if (__textReadOnly) return; __ReplaceTextSelection(value ?? string.Empty); } }",
            "SelectionLength" => "{ get { return __TextSelectionLength; } set { __SetTextSelection(__TextSelectionStart, value); } }",
            "SelectionStart" => "{ get { return __TextSelectionStart; } set { __SetTextSelection(value, 0); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.Control+ControlCollection" && property.Name == "Item")
        return "{ get { return __items[index]; } }";
    if (type.FullName == "System.Windows.Forms.ScrollProperties" && property.Name == "Enabled")
        return "{ set { __scrollEnabled = value; } }";
    if (type.FullName == "System.Windows.Forms.ScrollableControl")
        return property.Name switch
        {
            "AutoScroll" => "{ set { __autoScroll = value; } }",
            "DockPadding" => "{ get { return __dockPadding; } }",
            "HScroll" => "{ set { __hScroll = value; } }",
            "HorizontalScroll" => "{ get { return __horizontalScroll; } }",
            "VScroll" => "{ set { __vScroll = value; } }",
            "VerticalScroll" => "{ get { return __verticalScroll; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ScrollableControl+DockPaddingEdges")
        return property.Name switch
        {
            "All" => "{ set { __left = __top = __right = __bottom = value; } }",
            "Bottom" => "{ get { return __bottom; } set { __bottom = value; } }",
            "Left" => "{ get { return __left; } set { __left = value; } }",
            "Right" => "{ get { return __right; } set { __right = value; } }",
            "Top" => "{ get { return __top; } set { __top = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.Cursors")
        return property.Name switch
        {
            "Cross" => "{ get { return __cross; } }",
            "Default" => "{ get { return __default; } }",
            "HSplit" => "{ get { return __hSplit; } }",
            "Hand" => "{ get { return __hand; } }",
            "No" => "{ get { return __no; } }",
            "SizeWE" => "{ get { return __sizeWE; } }",
            "VSplit" => "{ get { return __vSplit; } }",
            "WaitCursor" => "{ get { return __wait; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ContainerControl")
        return property.Name switch
        {
            "ActiveControl" => "{ get { var focused = Control.__FocusedControl; return focused is not null && (global::System.Object.ReferenceEquals(focused, this) || __ContainsDescendant(focused)) ? focused : null!; } }",
            "AutoScaleDimensions" => "{ set { __autoScaleDimensions = value; } }",
            "AutoScaleMode" => "{ set { __autoScaleMode = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DataGridView")
        return property.Name switch
        {
            "AllowUserToAddRows" => "{ set { __allowUserToAddRows = value; } }",
            "AllowUserToDeleteRows" => "{ set { __allowUserToDeleteRows = value; } }",
            "AllowUserToResizeColumns" => "{ set { __allowUserToResizeColumns = value; } }",
            "AllowUserToResizeRows" => "{ set { __allowUserToResizeRows = value; } }",
            "AlternatingRowsDefaultCellStyle" => "{ get { return __alternatingRowsDefaultCellStyle; } }",
            "AutoGenerateColumns" => "{ set { __autoGenerateColumns = value; } }",
            "BackgroundColor" => "{ set { __gridBackgroundColor = value; BackColor = value; } }",
            "BorderStyle" => "{ set { __gridBorderStyle = value; } }",
            "ColumnHeadersBorderStyle" => "{ set { __columnHeaderBorderStyle = value; } }",
            "ColumnHeadersDefaultCellStyle" => "{ get { return __columnHeadersDefaultCellStyle; } }",
            "ColumnHeadersHeight" => "{ set { if (value < 4) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __columnHeadersHeight = value; } }",
            "ColumnHeadersHeightSizeMode" => "{ set { __columnHeadersHeightSizeMode = value; } }",
            "Columns" => "{ get { return __gridColumns; } }",
            "DataSource" => "{ set { __gridDataSource = value; } }",
            "DefaultCellStyle" => "{ get { return __defaultCellStyle; } }",
            "EditingControl" => "{ get { return null!; } }",
            "EnableHeadersVisualStyles" => "{ set { __enableHeadersVisualStyles = value; } }",
            "FirstDisplayedScrollingRowIndex" => "{ set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __firstDisplayedRow = value; } }",
            "GridColor" => "{ set { __gridColor = value; } }",
            "Item" => "{ get { return __Cell(columnIndex, rowIndex); } }",
            "MultiSelect" => "{ set { __multiSelect = value; } }",
            "ReadOnly" => "{ set { __gridReadOnly = value; } }",
            "RowHeadersBorderStyle" => "{ set { __rowHeaderBorderStyle = value; } }",
            "RowHeadersVisible" => "{ set { __rowHeadersVisible = value; } }",
            "RowTemplate" => "{ get { return __rowTemplate; } }",
            "Rows" => "{ get { return __gridRows; } }",
            "ScrollBars" => "{ set { __gridScrollBars = value; } }",
            "SelectedCells" => "{ get { return __selectedCells; } }",
            "SelectedRows" => "{ get { return __selectedRows; } }",
            "SelectionMode" => "{ set { __gridSelectionMode = value; } }",
            "ShowCellErrors" => "{ set { __showCellErrors = value; } }",
            "ShowCellToolTips" => "{ set { __showCellToolTips = value; } }",
            "ShowEditingIcon" => "{ set { __showEditingIcon = value; } }",
            "ShowRowErrors" => "{ set { __showRowErrors = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DataGridViewElement" && property.Name == "DataGridView")
        return "{ get { return __dataGridView!; } }";
    if (type.FullName == "System.Windows.Forms.DataGridViewBand")
        return property.Name switch
        {
            "DefaultCellStyle" => "{ get { return __bandDefaultCellStyle; } set { __bandDefaultCellStyle = value ?? throw new global::System.ArgumentNullException(nameof(value)); } }",
            "Displayed" => "{ get { return true; } }",
            "ReadOnly" => "{ set { __bandReadOnly = value; } }",
            "Resizable" => "{ set { __bandResizable = value; } }",
            "Selected" => "{ set { __bandSelected = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DataGridViewCell")
        return property.Name switch
        {
            "RowIndex" => "{ get { return __rowIndex; } }",
            "Style" => "{ get { return __cellStyle; } }",
            "Value" => "{ get { return __cellValue!; } }",
            "DefaultNewRowValue" => "{ get { return null!; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DataGridViewCellStyle")
        return property.Name switch
        {
            "Alignment" => "{ set { __alignment = value; } }",
            "BackColor" => "{ get { return __styleBackColor; } set { __styleBackColor = value; } }",
            "Font" => "{ get { return __styleFont!; } }",
            "ForeColor" => "{ get { return __styleForeColor; } set { __styleForeColor = value; } }",
            "Format" => "{ set { __format = value ?? string.Empty; } }",
            "SelectionBackColor" => "{ set { __selectionBackColor = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DataGridViewCellFormattingEventArgs")
        return property.Name switch
        {
            "ColumnIndex" => "{ get { return __formattingColumnIndex; } }",
            "FormattingApplied" => "{ set { __formattingApplied = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DataGridViewColumn")
        return property.Name switch
        {
            "CellTemplate" => "{ get { return __cellTemplate!; } set { __cellTemplate = value; } }",
            "AutoSizeMode" => "{ set { __columnAutoSizeMode = value; } }",
            "DataPropertyName" => "{ get { return __dataPropertyName; } set { __dataPropertyName = value ?? string.Empty; } }",
            "HeaderText" => "{ set { __headerText = value ?? string.Empty; } }",
            "FillWeight" => "{ set { if (value <= 0f) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __fillWeight = value; } }",
            "Name" => "{ set { __columnName = value ?? string.Empty; } }",
            "Width" => "{ set { __columnWidth = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DataGridViewRow" && property.Name == "Height")
        return "{ set { if (value < 2) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __rowHeight = value; } }";
    if (type.FullName == "System.Windows.Forms.DataGridViewColumnCollection" && property.Name == "Item")
        return "{ get { return __items[index]; } }";
    if (type.FullName == "System.Windows.Forms.DataGridViewRowCollection")
        return property.Name switch
        {
            "Count" => "{ get { __EnsureRows(); return __rows.Count; } }",
            "Item" => "{ get { __EnsureRows(); return __rows[index]; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.DataGridViewSelectedCellCollection" && property.Name == "Item")
        return "{ get { throw new global::System.ArgumentOutOfRangeException(nameof(index)); } }";
    if (type.FullName == "System.Windows.Forms.TableLayoutPanel")
        return property.Name switch
        {
            "ColumnCount" => "{ get { return __columnCount; } set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __columnCount = value; PerformLayout(); } }",
            "ColumnStyles" => "{ get { return __columnStyles; } }",
            "Controls" => "{ get { return __tableControls; } }",
            "RowCount" => "{ get { return __rowCount; } set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __rowCount = value; PerformLayout(); } }",
            "RowStyles" => "{ get { return __rowStyles; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.FlowLayoutPanel" && property.Name == "WrapContents")
        return "{ set { __wrapContents = value; PerformLayout(); } }";
    if (type.FullName == "System.Windows.Forms.TableLayoutColumnStyleCollection" && property.Name == "Item")
        return "{ set { if (value is null) throw new global::System.ArgumentNullException(nameof(value)); __styles[index] = value; } }";
    if (type.FullName == "System.Windows.Forms.CheckBox")
        return property.Name switch
        {
            "Checked" => "{ get { return __checkState != CheckState.Unchecked; } set { CheckState = value ? CheckState.Checked : CheckState.Unchecked; } }",
            "CheckState" => "{ set { if (__checkState == value) return; var oldChecked = __checkState != CheckState.Unchecked; __checkState = value; __NativeCheckState = (uint)value; CheckStateChanged?.Invoke(this, global::System.EventArgs.Empty); if (oldChecked != (__checkState != CheckState.Unchecked)) CheckedChanged?.Invoke(this, global::System.EventArgs.Empty); } }",
            "Appearance" => "{ set { __checkAppearance = value; Invalidate(); } }",
            "CheckAlign" => "{ set { __checkAlignment = value; Invalidate(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ButtonBase")
        return property.Name switch
        {
            "FlatAppearance" => "{ get { return __flatAppearance; } }",
            "FlatStyle" => "{ set { __buttonFlatStyle = value; Invalidate(); } }",
            "TextAlign" => "{ set { __buttonTextAlign = value; Invalidate(); } }",
            "UseVisualStyleBackColor" => "{ set { __useVisualStyleBackColor = value; Invalidate(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.Button" && property.Name == "DialogResult")
        return "{ set { __buttonDialogResult = value; } }";
    if (type.FullName == "System.Windows.Forms.RadioButton")
        return property.Name switch
        {
            "Checked" => "{ get { return __checked; } set { if (__checked == value) return; __checked = value; if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=radio-native-write-begin|name=\" + Name + \"|checked=\" + value); __NativeCheckState = value ? 1u : 0u; if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=radio-native-write-end|name=\" + Name); __RaiseCheckedChanged(); } }",
            "TabStop" => "{ set { __radioTabStop = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ListControl")
        return property.Name switch
        {
            "DisplayMember" => "{ set { __displayMember = value ?? string.Empty; } }",
            "FormattingEnabled" => "{ set { __formattingEnabled = value; } }",
            "SelectedIndex" => "{ get { if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=list-selection-get|name=\" + Name + \"|value=\" + __selectedIndex + \"|count=\" + __SelectionItemCount); return __selectedIndex; } set { if (value < -1 || value >= __SelectionItemCount) throw new global::System.ArgumentOutOfRangeException(nameof(value)); if (__selectedIndex == value) return; var previous = __selectedIndex; __selectedIndex = value; if (__TraceInteraction) global::System.Console.Error.WriteLine(\"facade-interaction=list-selection-set|name=\" + Name + \"|previous=\" + previous + \"|value=\" + value + \"|count=\" + __SelectionItemCount); __OnSelectedIndexChanged(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ListBox")
        return property.Name switch
        {
            "Items" => "{ get { return __listItems; } }",
            "DrawMode" => "{ set { __listDrawMode = value; Invalidate(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ListBox+ObjectCollection")
        return property.Name switch
        {
            "Count" => "{ get { return __items.Count; } }",
            "Item" => "{ get { return __items[index]; } set { __items[index] = value ?? throw new global::System.ArgumentNullException(nameof(value)); __owner?.Invalidate(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ProgressBar")
        return property.Name switch
        {
            "Maximum" => "{ get { return __progressMaximum; } }",
            "Minimum" => "{ get { return __progressMinimum; } }",
            "Value" => "{ set { if (value < __progressMinimum || value > __progressMaximum) throw new global::System.ArgumentOutOfRangeException(nameof(value)); if (__progressValue == value) return; __NativeRangeValue = value; __progressValue = value; Invalidate(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.SplitContainer")
        return property.Name switch
        {
            "Panel1" => "{ get { return __panel1; } }",
            "Panel2" => "{ get { return __panel2; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.Splitter")
        return property.Name switch
        {
            "SplitPosition" => "{ get { return __splitPosition; } set { __splitPosition = global::System.Math.Max(0, value); } }",
            "TabStop" => "{ set { __splitterTabStop = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ComboBox")
        return property.Name switch
        {
            "DropDownStyle" => "{ get { return __dropDownStyle; } set { __dropDownStyle = value; } }",
            "FlatStyle" => "{ get { return __comboFlatStyle; } set { __comboFlatStyle = value; } }",
            "AutoCompleteMode" => "{ set { __autoCompleteMode = value; } }",
            "AutoCompleteSource" => "{ set { __autoCompleteSource = value; } }",
            "DrawMode" => "{ set { __comboDrawMode = value; Invalidate(); } }",
            "DropDownHeight" => "{ set { if (value < 1) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __dropDownHeight = value; } }",
            "DropDownWidth" => "{ set { if (value < 1) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __dropDownWidth = value; } }",
            "IntegralHeight" => "{ set { __integralHeight = value; } }",
            "Items" => "{ get { return __comboItems; } }",
            "SelectedText" => "{ get { return Text.Substring(__ComboSelectionStart, __ComboSelectionLength); } set { if (__dropDownStyle == ComboBoxStyle.DropDownList) return; __ReplaceComboSelection(value ?? string.Empty); } }",
            "SelectionLength" => "{ get { return __ComboSelectionLength; } set { __SetComboSelection(__ComboSelectionStart, value); } }",
            "SelectionStart" => "{ get { return __ComboSelectionStart; } set { __SetComboSelection(value, 0); } }",
            "SelectedItem" => "{ get { return SelectedIndex >= 0 && SelectedIndex < __comboItems.Count ? __comboItems[SelectedIndex] : null!; } set { __selectedItem = value; SelectedIndex = value is null ? -1 : __comboItems.IndexOf(value); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ComboBox+ObjectCollection")
        return property.Name switch
        {
            "Count" => "{ get { return __items.Count; } }",
            "Item" => "{ get { return __items[index]; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.NumericUpDown")
        return property.Name switch
        {
            "DecimalPlaces" => "{ set { if (value < 0 || value > 99) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __decimalPlaces = value; __UpdateNumericText(); } }",
            "Increment" => "{ get { return __increment; } set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __increment = value; } }",
            "Maximum" => "{ get { return __maximum; } set { __maximum = value; if (__minimum > value) __minimum = value; if (__value > value) Value = value; } }",
            "Minimum" => "{ get { return __minimum; } set { __minimum = value; if (__maximum < value) __maximum = value; if (__value < value) Value = value; } }",
            "ThousandsSeparator" => "{ set { __thousandsSeparator = value; __UpdateNumericText(); } }",
            "Value" => "{ get { return __value; } set { if (value < __minimum || value > __maximum) throw new global::System.ArgumentOutOfRangeException(nameof(value)); if (__value == value) { __UpdateNumericText(); return; } __value = value; __UpdateNumericText(); ValueChanged?.Invoke(this, global::System.EventArgs.Empty); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.Timer")
        return property.Name switch
        {
            "Enabled" => "{ get { return __timerEnabled; } set { if (__timerEnabled == value) return; __timerEnabled = value; __Schedule(); } }",
            "Interval" => "{ get { return __interval; } set { if (value < 1) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __interval = value; if (__timerEnabled) __Schedule(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStrip")
        return property.Name switch
        {
            "GripMargin" => "{ get { return __gripMargin; } set { __gripMargin = value; } }",
            "Items" => "{ get { return __toolItems; } }",
            "Orientation" => "{ get { return Orientation.Horizontal; } }",
            "RenderMode" => "{ get { return __renderMode; } set { __renderMode = value; } }",
            "Renderer" => "{ get { return __renderer!; } set { __renderer = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripItem")
        return property.Name switch
        {
            "ContentRectangle" => "{ get { return new global::System.Drawing.Rectangle(0, 0, __itemSize.Width, __itemSize.Height); } }",
            "Enabled" => "{ get { return __itemEnabled; } }",
            "Height" => "{ get { return __itemSize.Height; } }",
            "Image" => "{ set { __itemImage = value; __RefreshOwner(); } }",
            "Margin" => "{ set { __itemMargin = value; __RefreshOwner(); } }",
            "Name" => "{ set { __itemName = value ?? string.Empty; } }",
            "Owner" => "{ get { return __owner!; } }",
            "Pressed" => "{ get { return false; } }",
            "Selected" => "{ get { return false; } }",
            "Size" => "{ set { __itemSize = value; __RefreshOwner(); } }",
            "Tag" => "{ get { return __itemTag!; } set { __itemTag = value; } }",
            "Text" => "{ get { return __itemText; } set { __itemText = value ?? string.Empty; __RefreshOwner(); } }",
            "ToolTipText" => "{ get { return __toolTipText; } set { __toolTipText = value ?? string.Empty; } }",
            "Visible" => "{ set { __itemVisible = value; __RefreshOwner(); } }",
            "Width" => "{ get { return __itemSize.Width; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripItemCollection" && property.Name == "Item")
        return "{ get { return __items[index]; } }";
    if (type.FullName == "System.Windows.Forms.ToolStripMenuItem" && property.Name == "Checked")
        return "{ get { return __menuChecked; } set { __menuChecked = value; } }";
    if (type.FullName == "System.Windows.Forms.ToolStripDropDownItem")
        return property.Name switch
        {
            "DropDown" => "{ get { return __dropDown; } }",
            "DropDownItems" => "{ get { return __dropDown.Items; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripDropDown" && property.Name == "OwnerItem")
        return "{ get { return __ownerItem!; } set { __ownerItem = value; } }";
    if (type.FullName == "System.Windows.Forms.ToolStripProfessionalRenderer" && property.Name == "ColorTable")
        return "{ get { return __colorTable; } }";
    if (type.FullName == "System.Windows.Forms.Padding")
        return property.Name switch
        {
            "All" => "{ get { return __left == __top && __left == __right && __left == __bottom ? __left : -1; } }",
            "Bottom" => "{ get { return __bottom; } }",
            "Left" => "{ get { return __left; } }",
            "Right" => "{ get { return __right; } }",
            "Top" => "{ get { return __top; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.PaintEventArgs")
        return property.Name switch
        {
            "ClipRectangle" => "{ get { return __clipRectangle; } }",
            "Graphics" => "{ get { return __graphics ?? throw new global::System.InvalidOperationException(\"PaintEventArgs does not have a graphics context.\"); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.MouseEventArgs")
        return property.Name switch
        {
            "Button" => "{ get { return __button; } }",
            "Delta" => "{ get { return __delta; } }",
            "Location" => "{ get { return new global::System.Drawing.Point(__x, __y); } }",
            "X" => "{ get { return __x; } }",
            "Y" => "{ get { return __y; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.KeyEventArgs")
        return property.Name switch
        {
            "Handled" => "{ set { __handled = value; } }",
            "KeyCode" => "{ get { return __keyData & Keys.KeyCode; } }",
            "Modifiers" => "{ get { return __keyData & Keys.Modifiers; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.FlatButtonAppearance" && property.Name == "BorderSize")
        return "{ set { if (value < 0) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __flatBorderSize = value; } }";
    if (type.FullName == "System.Windows.Forms.GroupBox")
        return property.Name switch
        {
            "FlatStyle" => "{ set { __groupFlatStyle = value; Invalidate(); } }",
            "TabStop" => "{ set { __groupTabStop = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.KeyPressEventArgs" && property.Name == "Handled")
        return "{ set { __keyPressHandled = value; } }";
    if (type.FullName == "System.Windows.Forms.HandledMouseEventArgs" && property.Name == "Handled")
        return "{ set { __mouseHandled = value; } }";
    if (type.FullName == "System.Windows.Forms.Panel")
        return property.Name switch
        {
            "AutoSizeMode" => "{ set { __panelAutoSizeMode = value; } }",
            "BorderStyle" => "{ set { __panelBorderStyle = value; Invalidate(); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.StatusStrip")
        return property.Name switch
        {
            "TabStop" => "{ set { __statusTabStop = value; } }",
            "SizeGripBounds" => "{ get { return new global::System.Drawing.Rectangle(global::System.Math.Max(0, Width - 16), global::System.Math.Max(0, Height - 16), global::System.Math.Min(16, Width), global::System.Math.Min(16, Height)); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.TextBox" && property.Name == "TextAlign")
        return "{ set { __textBoxAlignment = value; } }";
    if (type.FullName == "System.Windows.Forms.UserControl" && property.Name == "AutoSizeMode")
        return "{ set { __userControlAutoSizeMode = value; } }";
    if (type.FullName == "System.Windows.Forms.PictureBox")
        return property.Name switch
        {
            "BorderStyle" => "{ set { __pictureBorderStyle = value; Invalidate(); } }",
            "ErrorImage" => "{ set { __errorImage = value; Invalidate(); } }",
            "Image" => "{ set { __pictureImage = value; Invalidate(); } }",
            "InitialImage" => "{ set { __initialImage = value; Invalidate(); } }",
            "SizeMode" => "{ set { __pictureSizeMode = value; Invalidate(); } }",
            "TabIndex" => "{ set { base.TabIndex = value; } }",
            "TabStop" => "{ set { base.TabStop = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.UpDownBase")
        return property.Name switch
        {
            "BorderStyle" => "{ get { return __upDownBorderStyle; } set { __upDownBorderStyle = value; } }",
            "CreateParams" => "{ get { return new CreateParams(); } }",
            "TextAlign" => "{ set { __upDownTextAlign = value; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripButton" && property.Name == "Checked")
        return "{ get { return __toolButtonChecked; } }";
    if (type.FullName == "System.Windows.Forms.ToolStripManager")
        return property.Name switch
        {
            "RenderMode" => "{ get { return __managerRenderMode; } set { __managerRenderMode = value; } }",
            "Renderer" => "{ get { return __managerRenderer; } set { __managerRenderer = value ?? throw new global::System.ArgumentNullException(nameof(value)); __managerRenderMode = ToolStripManagerRenderMode.Custom; } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripProfessionalRenderer" && property.Name == "RoundedEdges")
        return "{ set { __roundedEdges = value; } }";
    if (type.FullName == "System.Windows.Forms.ProfessionalColorTable")
        return property.Name switch
        {
            "ButtonCheckedGradientBegin" => "{ get { return global::System.Drawing.Color.FromArgb(224, 236, 249); } }",
            "ButtonCheckedGradientMiddle" => "{ get { return global::System.Drawing.Color.FromArgb(191, 219, 248); } }",
            "ButtonCheckedGradientEnd" => "{ get { return global::System.Drawing.Color.FromArgb(152, 194, 233); } }",
            "ButtonPressedBorder" => "{ get { return global::System.Drawing.Color.FromArgb(61, 123, 173); } }",
            "ButtonPressedGradientBegin" => "{ get { return global::System.Drawing.Color.FromArgb(208, 228, 247); } }",
            "ButtonPressedGradientMiddle" => "{ get { return global::System.Drawing.Color.FromArgb(170, 204, 235); } }",
            "ButtonPressedGradientEnd" => "{ get { return global::System.Drawing.Color.FromArgb(126, 175, 216); } }",
            "ButtonSelectedBorder" => "{ get { return global::System.Drawing.Color.FromArgb(91, 143, 185); } }",
            "ButtonSelectedGradientBegin" => "{ get { return global::System.Drawing.Color.FromArgb(238, 246, 254); } }",
            "ButtonSelectedGradientMiddle" => "{ get { return global::System.Drawing.Color.FromArgb(213, 232, 250); } }",
            "ButtonSelectedGradientEnd" => "{ get { return global::System.Drawing.Color.FromArgb(175, 210, 241); } }",
            "MenuItemBorder" => "{ get { return global::System.Drawing.Color.FromArgb(112, 153, 187); } }",
            "MenuItemSelected" => "{ get { return global::System.Drawing.Color.FromArgb(217, 235, 252); } }",
            "MenuItemSelectedGradientBegin" => "{ get { return global::System.Drawing.Color.FromArgb(236, 246, 255); } }",
            "MenuItemSelectedGradientEnd" => "{ get { return global::System.Drawing.Color.FromArgb(190, 220, 246); } }",
            _ => Stub(),
        };
    if (type.FullName == "System.Windows.Forms.Form")
        return property.Name switch
        {
            "ClientSize" => "{ set { base.ClientSize = value; } }",
            "AcceptButton" => "{ set { __acceptButton = value; } }",
            "ActiveMdiChild" => "{ get { return null!; } }",
            "CancelButton" => "{ set { __cancelButton = value; } }",
            "ControlBox" => "{ get { return __controlBox; } set { __controlBox = value; } }",
            "CreateParams" => "{ get { return __createParams; } }",
            "DialogResult" => "{ set { __dialogResult = value; if (value != DialogResult.None && Visible) __RequestClose(); } }",
            "FormBorderStyle" => "{ set { __formBorderStyle = value; } }",
            "Icon" => "{ get { return __formIcon!; } set { __formIcon = value; } }",
            "KeyPreview" => "{ set { __keyPreview = value; } }",
            "Location" => "{ get { return base.Location; } set { base.Location = value; } }",
            "MainMenuStrip" => "{ get { return __mainMenuStrip!; } }",
            "MaximizeBox" => "{ set { __maximizeBox = value; } }",
            "MinimizeBox" => "{ set { __minimizeBox = value; } }",
            "Opacity" => "{ set { if (value < 0d || value > 1d) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __opacity = value; } }",
            "Owner" => "{ set { __ownerForm = value; } }",
            "Margin" => "{ set { base.Margin = value; } }",
            "MdiParent" => "{ get { return __mdiParent!; } set { __mdiParent = value; } }",
            "RightToLeftLayout" => "{ get { return __rightToLeftLayout; } set { __rightToLeftLayout = value; } }",
            "ShowIcon" => "{ set { __showIcon = value; } }",
            "ShowInTaskbar" => "{ set { __showInTaskbar = value; } }",
            "Size" => "{ get { return base.Size; } set { base.Size = value; } }",
            "SizeGripStyle" => "{ set { __sizeGripStyle = value; } }",
            "StartPosition" => "{ set { __startPosition = value; } }",
            "TopLevel" => "{ get { return __topLevel; } set { __topLevel = value; } }",
            "TopMost" => "{ set { __topMost = value; } }",
            "WindowState" => "{ get { return __windowState; } set { __windowState = value; } }",
            _ => Stub(),
        };
    return Stub();
}

static string StubProperty(Type type, PropertyInfo property, bool isInterface, bool get, bool set) => "{ " +
    (get ? isInterface ? "get; " : $"get {{ return global::{(type.Assembly.GetName().Name == "System.Drawing.Common" ? "System.Drawing" : "System.Windows.Forms")}.FacadeStubDiagnostics.Value<{CsType(property.PropertyType)}>(\"{type.FullName}.{property.Name}\"); }} " : "") +
    (set ? isInterface ? "set; " : "set { } " : "") + "}";

static string DrawingImageBody(MethodInfo method)
{
    var parameters = method.GetParameters();
    if (parameters.Length == 2)
        return "{ __EnsureRecorder(); NativeDrawingBridge.DrawImage(__recorder, image, new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height), new global::System.Drawing.RectangleF(0, 0, image.Width, image.Height), null); }";
    if (parameters.Length == 3)
        return "{ __EnsureRecorder(); NativeDrawingBridge.DrawImage(__recorder, image, new global::System.Drawing.RectangleF(x, y, image.Width, image.Height), new global::System.Drawing.RectangleF(0, 0, image.Width, image.Height), null); }";
    if (parameters.Length == 5)
        return "{ __EnsureRecorder(); NativeDrawingBridge.DrawImage(__recorder, image, new global::System.Drawing.RectangleF(x, y, width, height), new global::System.Drawing.RectangleF(0, 0, image.Width, image.Height), null); }";
    if (parameters.Length == 4 && parameters[1].ParameterType.FullName == "System.Drawing.Rectangle")
        return "{ __EnsureRecorder(); NativeDrawingBridge.DrawImage(__recorder, image, new global::System.Drawing.RectangleF(destRect.X, destRect.Y, destRect.Width, destRect.Height), new global::System.Drawing.RectangleF(srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height), null); }";
    var attributeName = parameters.Last().ParameterType.FullName == "System.Drawing.Imaging.ImageAttributes" ?
        parameters.Last().Name : "null";
    return "{ __EnsureRecorder(); NativeDrawingBridge.DrawImage(__recorder, image, new global::System.Drawing.RectangleF(destRect.X, destRect.Y, destRect.Width, destRect.Height), new global::System.Drawing.RectangleF(srcX, srcY, srcWidth, srcHeight), " + attributeName + "); }";
}

static string MethodBody(Type type, MethodInfo method)
{
    if (type.FullName == "System.Windows.Forms.CommonDialog" && method.Name == "ShowDialog")
        return method.GetParameters().Length == 0
            ? "{ return __RunDialog(null); }"
            : "{ return __RunDialog(owner); }";
    if (type.FullName == "System.Windows.Forms.ToolTip")
        return method.Name switch
        {
            "GetToolTip" => "{ if (control is null) throw new global::System.ArgumentNullException(nameof(control)); return __toolTips.TryGetValue(control, out var value) ? value : string.Empty; }",
            "SetToolTip" => "{ if (control is null) throw new global::System.ArgumentNullException(nameof(control)); caption ??= string.Empty; if (caption.Length == 0) { __toolTips.Remove(control); if (__toolTipHandlers.Remove(control, out var handlers)) { control.MouseEnter -= handlers.Enter; control.MouseLeave -= handlers.Leave; } __CancelPending(control, true); return; } __toolTips[control] = caption; if (!__toolTipHandlers.ContainsKey(control)) { global::System.EventHandler enter = (_, _) => { if (__toolTips.TryGetValue(control, out var current)) __Schedule(control, current); }; global::System.EventHandler leave = (_, _) => { __CancelPending(control, true); if (global::System.Object.ReferenceEquals(__shownWindow, control)) { __shownWindow = null; __shownText = string.Empty; } }; __toolTipHandlers[control] = (enter, leave); control.MouseEnter += enter; control.MouseLeave += leave; } }",
            "Show" => "{ if (window is null) throw new global::System.ArgumentNullException(nameof(window)); if (duration < 0) throw new global::System.ArgumentOutOfRangeException(nameof(duration)); if (!__toolTipActive) return; __shownWindow = window; __shownText = text ?? string.Empty; if (window is Control control) { __CancelPending(control, true); if (__shownText.Length != 0) { __toolTips[control] = __shownText; control.__ShowToolTip(__shownText, x, y, duration); } } if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_TOOLTIPS\") == \"1\") global::System.Console.Error.WriteLine(\"facade-tooltip=show|text=\" + __shownText.Replace('\\r', ' ').Replace('\\n', ' ') + \"|x=\" + x + \"|y=\" + y + \"|duration=\" + duration + \"|provider=native\"); }",
            "Hide" => "{ if (win is null) throw new global::System.ArgumentNullException(nameof(win)); if (win is Control control) __CancelPending(control, true); if (global::System.Object.ReferenceEquals(__shownWindow, win)) { __shownWindow = null; __shownText = string.Empty; } }",
            _ => BodyFor(method.ReturnType, false),
        };
    if (type.FullName == "System.Windows.Forms.MessageBox" && method.Name == "Show")
    {
        var parameters = method.GetParameters();
        if (parameters.Length == 1)
            return "{ return __Show(text, string.Empty, MessageBoxButtons.OK, MessageBoxIcon.None, MessageBoxDefaultButton.Button1); }";
        if (parameters.Length == 2)
            return "{ return __Show(text, caption, MessageBoxButtons.OK, MessageBoxIcon.None, MessageBoxDefaultButton.Button1); }";
        if (parameters[0].ParameterType.FullName == "System.Windows.Forms.IWin32Window")
            return "{ return __Show(text, caption, buttons, icon, MessageBoxDefaultButton.Button1); }";
        if (parameters.Length == 4)
            return "{ return __Show(text, caption, buttons, icon, MessageBoxDefaultButton.Button1); }";
        return "{ return __Show(text, caption, buttons, icon, defaultButton); }";
    }
    if (type.FullName == "System.Windows.Forms.Clipboard")
    {
        if (method.Name == "GetText")
            return "{ return __clipboardText; }";
        if (method.Name == "SetText")
            return "{ if (text is null) throw new global::System.ArgumentNullException(nameof(text)); __clipboardText = text; }";
    }
    if (type.FullName == "System.Windows.Forms.LinkLabel+LinkCollection" && method.Name == "Add")
        return "{ if (value is null) throw new global::System.ArgumentNullException(nameof(value)); __linkItems.Add(value); return __linkItems.Count - 1; }";
    if (type.FullName == "System.Windows.Forms.ContainerControl")
    {
        if (method.Name == "ProcessCmdKey") return "{ return false; }";
        if (method.Name == "SelectNextControl")
            return "{ if (Controls.Count == 0) return false; var start = ctl is null ? (forward ? -1 : 0) : Controls.IndexOf(ctl); for (var offset = 1; offset <= Controls.Count; ++offset) { var index = forward ? start + offset : start - offset; if (wrap) index = (index % Controls.Count + Controls.Count) % Controls.Count; else if (index < 0 || index >= Controls.Count) return false; var candidate = Controls[index]; if ((!tabStopOnly || candidate.__TabStop) && candidate.Enabled && candidate.Visible && candidate.Focus()) return true; } return false; }";
    }
    if (type.FullName == "System.Windows.Forms.Control")
    {
        if (method.Name == "FromChildHandle") return "{ return null!; }";
        if (method.Name == "SelectNextControl")
            return "{ if (Controls.Count == 0) return false; var start = ctl is null ? (forward ? -1 : 0) : Controls.IndexOf(ctl); for (var offset = 1; offset <= Controls.Count; ++offset) { var index = forward ? start + offset : start - offset; if (wrap) index = (index % Controls.Count + Controls.Count) % Controls.Count; else if (index < 0 || index >= Controls.Count) return false; var candidate = Controls[index]; if ((!tabStopOnly || candidate.__TabStop) && candidate.Enabled && candidate.Visible && candidate.Focus()) return true; } return false; }";
    }
    if (type.FullName == "System.Windows.Forms.Layout.ArrangedElementCollection")
    {
        if (method.Name == "GetEnumerator") return "{ return global::System.Array.Empty<object>().GetEnumerator(); }";
    }
    if (type.FullName == "System.Windows.Forms.Screen" && method.Name == "GetWorkingArea")
        return "{ return new global::System.Drawing.Rectangle(0, 0, 1920, 1040); }";
    if (type.FullName == "System.Windows.Forms.ControlPaint")
    {
        var amount = method.GetParameters().Length == 2
            ? "global::System.Math.Clamp(" + method.GetParameters()[1].Name + ", 0f, 1f)"
            : "0.25f";
        if (method.Name == "Dark")
            return "{ var amount = " + amount + "; return global::System.Drawing.Color.FromArgb(baseColor.A, (int)(baseColor.R * (1f - amount)), (int)(baseColor.G * (1f - amount)), (int)(baseColor.B * (1f - amount))); }";
        if (method.Name == "Light" || method.Name == "LightLight")
        {
            var lightAmount = method.Name == "LightLight" ? "0.5f" : amount;
            return "{ var amount = " + lightAmount + "; return global::System.Drawing.Color.FromArgb(baseColor.A, baseColor.R + (int)((255 - baseColor.R) * amount), baseColor.G + (int)((255 - baseColor.G) * amount), baseColor.B + (int)((255 - baseColor.B) * amount)); }";
        }
    }
    if (type.FullName == "System.Windows.Forms.TextRenderer")
    {
        if (method.Name == "MeasureText" && method.GetParameters().Length == 2)
            return "{ if (text is null) throw new global::System.ArgumentNullException(nameof(text)); if (font is null) throw new global::System.ArgumentNullException(nameof(font)); var height = global::System.Math.Max(1, font.Height); var padding = 8; return new global::System.Drawing.Size(global::System.Math.Max(1, (int)global::System.Math.Ceiling(text.Length * height * 0.46f) + padding), height + 4); }";
        if (method.Name == "MeasureText")
            return "{ var measured = MeasureText(text, font); var width = proposedSize.Width > 0 ? global::System.Math.Min(measured.Width, proposedSize.Width) : measured.Width; var height = proposedSize.Height > 0 ? global::System.Math.Min(measured.Height, proposedSize.Height) : measured.Height; if ((flags & global::System.Windows.Forms.TextFormatFlags.WordBreak) != 0 && proposedSize.Width > 0 && measured.Width > proposedSize.Width) { var rows = global::System.Math.Max(1, (int)global::System.Math.Ceiling((double)measured.Width / proposedSize.Width)); height = proposedSize.Height > 0 ? global::System.Math.Min(proposedSize.Height, measured.Height * rows) : measured.Height * rows; } return new global::System.Drawing.Size(global::System.Math.Max(1, width), global::System.Math.Max(1, height)); }";
        if (method.Name == "DrawText")
            return "{ if (dc is null) throw new global::System.ArgumentNullException(nameof(dc)); if (text is null) throw new global::System.ArgumentNullException(nameof(text)); if (font is null) throw new global::System.ArgumentNullException(nameof(font)); if (dc is not global::System.Drawing.Graphics graphics || bounds.Width <= 0 || bounds.Height <= 0 || text.Length == 0) return; if (global::System.Environment.GetEnvironmentVariable(\"GUI_FORMS_TRACE_TEXT\") == \"1\") global::System.Console.Error.WriteLine(\"facade-text-draw=text:\" + text.Replace('\\r', ' ').Replace('\\n', ' ') + \"|bounds:\" + bounds.X + \",\" + bounds.Y + \",\" + bounds.Width + \",\" + bounds.Height + \"|font-height:\" + font.Height); var measured = graphics.MeasureString(text, font); var x = (float)bounds.X; var y = (float)bounds.Y; if ((flags & global::System.Windows.Forms.TextFormatFlags.HorizontalCenter) != 0) x += global::System.Math.Max(0f, (bounds.Width - measured.Width) / 2f); else if ((flags & global::System.Windows.Forms.TextFormatFlags.Right) != 0) x += global::System.Math.Max(0f, bounds.Width - measured.Width); if ((flags & global::System.Windows.Forms.TextFormatFlags.VerticalCenter) != 0) y += global::System.Math.Max(0f, (bounds.Height - measured.Height) / 2f); else if ((flags & global::System.Windows.Forms.TextFormatFlags.Bottom) != 0) y += global::System.Math.Max(0f, bounds.Height - measured.Height); using var brush = new global::System.Drawing.SolidBrush(foreColor); graphics.DrawString(text, font, brush, new global::System.Drawing.PointF(x, y)); }";
    }
    if (type.FullName == "System.Windows.Forms.ToolStripDropDown" && method.Name == "Show")
    {
        var parameters = method.GetParameters();
        if (parameters.Length == 1)
            return "{ __ShowDropDown(screenLocation); }";
        if (parameters.Length == 2 && parameters[0].ParameterType.FullName == "System.Drawing.Point")
            return "{ __ShowDropDown(position); }";
        if (parameters.Length == 2)
            return "{ if (control is null) throw new global::System.ArgumentNullException(nameof(control)); __ShowDropDown(control.PointToScreen(position), control); }";
        return "{ if (control is null) throw new global::System.ArgumentNullException(nameof(control)); __ShowDropDown(control.PointToScreen(new global::System.Drawing.Point(x, y)), control); }";
    }
    if (type.FullName == "System.Drawing.Icon")
    {
        if (method.Name == "ToBitmap") return "{ if (__iconDisposed) throw new global::System.ObjectDisposedException(nameof(Icon)); return NativeDrawingBridge.DecodeIcon(__iconData); }";
        if (method.Name == "Dispose") return "{ if (__iconDisposed) return; __iconDisposed = true; __iconData = global::System.Array.Empty<byte>(); global::System.GC.SuppressFinalize(this); }";
    }
    if (type.FullName == "System.Drawing.Brush" && method.Name == "Dispose")
        return "{ if (__brushDisposed) return; __brushDisposed = true; NativeDrawingBridge.Release(ref __handle); global::System.GC.SuppressFinalize(this); }";
    if (type.FullName == "System.Drawing.Pen" && method.Name == "Dispose")
        return "{ if (__penDisposed) return; __penDisposed = true; NativeDrawingBridge.Release(ref __handle); global::System.GC.SuppressFinalize(this); }";
    if (type.FullName == "System.Drawing.Font" && method.Name == "Dispose")
        return "{ if (__fontDisposed) return; __fontDisposed = true; NativeDrawingBridge.Release(ref __handle); global::System.GC.SuppressFinalize(this); }";
    if (type.FullName == "System.Drawing.StringFormat" && method.Name == "Dispose")
        return "{ if (__formatDisposed) return; __formatDisposed = true; NativeDrawingBridge.Release(ref __handle); global::System.GC.SuppressFinalize(this); }";
    if (type.FullName == "System.Drawing.Image")
    {
        if (method.Name == "Dispose") return "{ if (__imageDisposed) return; __imageDisposed = true; NativeDrawingBridge.Release(ref __bitmap); global::System.GC.SuppressFinalize(this); }";
        if (method.Name == "Save" && method.GetParameters().Length == 2) return "{ if (stream is null) throw new global::System.ArgumentNullException(nameof(stream)); var png = NativeDrawingBridge.EncodePng(__bitmap); stream.Write(png, 0, png.Length); }";
        if (method.Name == "Save") return "{ if (filename is null) throw new global::System.ArgumentNullException(nameof(filename)); global::System.IO.File.WriteAllBytes(filename, NativeDrawingBridge.EncodePng(__bitmap)); }";
        if (method.Name == "Clone") return "{ return NativeDrawingBridge.CloneBitmap(this); }";
        if (method.Name == "GetThumbnailImage") return "{ return NativeDrawingBridge.Thumbnail(this, thumbWidth, thumbHeight); }";
        if (method.Name == "FromStream") return "{ if (stream is null) throw new global::System.ArgumentNullException(nameof(stream)); using var copy = new global::System.IO.MemoryStream(); stream.CopyTo(copy); return NativeDrawingBridge.DecodePng(copy.ToArray()); }";
        if (method.Name == "FromFile") return "{ if (filename is null) throw new global::System.ArgumentNullException(nameof(filename)); return NativeDrawingBridge.DecodePng(global::System.IO.File.ReadAllBytes(filename)); }";
        if (method.Name == "FromHbitmap") return "{ return NativeDrawingBridge.FromHbitmap(hbitmap); }";
    }
    if (type.FullName == "System.Drawing.Bitmap")
    {
        if (method.Name == "GetPixel") return "{ return NativeDrawingBridge.BitmapGetPixel(__bitmap, x, y); }";
        if (method.Name == "MakeTransparent") return "{ NativeDrawingBridge.BitmapMakeTransparent(__bitmap, transparentColor); }";
        if (method.Name == "GetHbitmap") return "{ return NativeDrawingBridge.GetHbitmap(this, background); }";
        if (method.Name == "LockBits") return "{ return NativeDrawingBridge.BitmapLock(__bitmap, flags); }";
        if (method.Name == "UnlockBits") return "{ if (bitmapdata is null) throw new global::System.ArgumentNullException(nameof(bitmapdata)); NativeDrawingBridge.BitmapUnlock(__bitmap, bitmapdata); }";
    }
    if (type.FullName == "System.Drawing.Graphics")
    {
        var count = method.GetParameters().Length;
        if (method.Name == "FromImage") return "{ if (image is null) throw new global::System.ArgumentNullException(nameof(image)); if (image.__BitmapHandle.IsNull) throw new global::System.ArgumentException(\"Image has no native bitmap.\", nameof(image)); return new global::System.Drawing.Graphics { __target = image }; }";
        if (method.Name is "FromHdc" or "FromHdcInternal") return "{ return NativeDrawingBridge.GraphicsFromNativeSurface(" + method.GetParameters()[0].Name + ", 0); }";
        if (method.Name == "FromHwnd") return "{ return NativeDrawingBridge.GraphicsFromNativeSurface(" + method.GetParameters()[0].Name + ", 1); }";
        if (method.Name == "Dispose") return "{ if (__graphicsDisposed) return; __graphicsDisposed = true; try { if (__hdcLeaseToken != 0) NativeDrawingBridge.ReleaseHdc(this, __leasedHdc); if (__target is not null && !__recorder.IsNull) { NativeDrawingBridge.Execute(__recorder, __target.__BitmapHandle); if (__nativeSurface != 0) NativeDrawingBridge.PresentNativeSurface(this); } } finally { NativeDrawingBridge.Release(ref __recorder); if (__nativeSurface != 0) __target?.Dispose(); __target = null; __nativeSurface = 0; global::System.GC.SuppressFinalize(this); } }";
        if (method.Name == "Clear") return "{ __EnsureRecorder(); NativeDrawingBridge.RecorderClear(__recorder, color); }";
        if (method.Name == "FillRectangle")
        {
            var first = method.GetParameters()[1].ParameterType.FullName;
            if (first == "System.Drawing.Rectangle") return "{ __EnsureRecorder(); NativeDrawingBridge.FillRectangle(__recorder, brush.__handle, new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height)); }";
            if (first == "System.Drawing.RectangleF") return "{ __EnsureRecorder(); NativeDrawingBridge.FillRectangle(__recorder, brush.__handle, rect); }";
            return "{ __EnsureRecorder(); NativeDrawingBridge.FillRectangle(__recorder, brush.__handle, new global::System.Drawing.RectangleF(x, y, width, height)); }";
        }
        if (method.Name == "FillRectangles") return "{ if (rects is null) throw new global::System.ArgumentNullException(nameof(rects)); __EnsureRecorder(); foreach (var rect in rects) NativeDrawingBridge.FillRectangle(__recorder, brush.__handle, new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height)); }";
        if (method.Name == "DrawRectangle")
        {
            if (count == 2) return "{ __EnsureRecorder(); NativeDrawingBridge.DrawRectangle(__recorder, pen.__handle, new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height)); }";
            return "{ __EnsureRecorder(); NativeDrawingBridge.DrawRectangle(__recorder, pen.__handle, new global::System.Drawing.RectangleF(x, y, width, height)); }";
        }
        if (method.Name == "DrawLine")
        {
            if (count == 3) return "{ __EnsureRecorder(); NativeDrawingBridge.DrawLine(__recorder, pen.__handle, new global::System.Drawing.PointF(pt1.X, pt1.Y), new global::System.Drawing.PointF(pt2.X, pt2.Y)); }";
            return "{ __EnsureRecorder(); NativeDrawingBridge.DrawLine(__recorder, pen.__handle, new global::System.Drawing.PointF(x1, y1), new global::System.Drawing.PointF(x2, y2)); }";
        }
        if (method.Name == "DrawLines") return "{ if (points is null) throw new global::System.ArgumentNullException(nameof(points)); __EnsureRecorder(); for (var i = 1; i < points.Length; ++i) NativeDrawingBridge.DrawLine(__recorder, pen.__handle, new global::System.Drawing.PointF(points[i - 1].X, points[i - 1].Y), new global::System.Drawing.PointF(points[i].X, points[i].Y)); }";
        if (method.Name == "DrawEllipse") return "{ __EnsureRecorder(); NativeDrawingBridge.DrawEllipse(__recorder, pen.__handle, new global::System.Drawing.RectangleF(x, y, width, height)); }";
        if (method.Name == "FillEllipse") return "{ __EnsureRecorder(); NativeDrawingBridge.FillEllipse(__recorder, brush.__handle, new global::System.Drawing.RectangleF(x, y, width, height)); }";
        if (method.Name == "MeasureString") return "{ if (global::System.String.IsNullOrEmpty(text)) return global::System.Drawing.SizeF.Empty; if (font is null) throw new global::System.ArgumentNullException(nameof(font)); var measured = new global::System.Drawing.SizeF(text.Length * font.__size * 0.55f, font.Height); return " + (count == 4 ? "new global::System.Drawing.SizeF(global::System.Math.Min(width, measured.Width), measured.Height)" : "measured") + "; }";
        if (method.Name == "DrawString")
        {
            const string validate = "if (brush is null) throw new global::System.ArgumentNullException(nameof(brush)); if (global::System.String.IsNullOrEmpty(s)) return; if (font is null) throw new global::System.ArgumentNullException(nameof(font)); ";
            if (count == 4 && method.GetParameters()[3].ParameterType.FullName == "System.Drawing.PointF") return "{ " + validate + "__EnsureRecorder(); NativeDrawingBridge.DrawString(__recorder, s, font, brush, point, null); }";
            if (count == 5 && method.GetParameters()[3].ParameterType.FullName == "System.Drawing.PointF") return "{ " + validate + "__EnsureRecorder(); NativeDrawingBridge.DrawString(__recorder, s, font, brush, point, format); }";
            if (count >= 4 && method.GetParameters()[3].ParameterType.FullName == "System.Drawing.RectangleF") return "{ " + validate + "__EnsureRecorder(); NativeDrawingBridge.DrawString(__recorder, s, font, brush, layoutRectangle.Location, " + (count == 5 ? "format" : "null") + "); }";
            return "{ " + validate + "__EnsureRecorder(); NativeDrawingBridge.DrawString(__recorder, s, font, brush, new global::System.Drawing.PointF(x, y), null); }";
        }
        if (method.Name == "DrawImage" || method.Name == "DrawImageUnscaled") return DrawingImageBody(method);
        if (method.Name == "DrawPath") return "{ if (path is null) throw new global::System.ArgumentNullException(nameof(path)); __EnsureRecorder(); NativeDrawingBridge.DrawPath(__recorder, pen.__handle, path.__handle); }";
        if (method.Name == "FillPath") return "{ if (path is null) throw new global::System.ArgumentNullException(nameof(path)); __EnsureRecorder(); NativeDrawingBridge.FillPath(__recorder, brush.__handle, path.__handle); }";
        if (method.Name == "FillPolygon") return "{ if (points is null) throw new global::System.ArgumentNullException(nameof(points)); __EnsureRecorder(); var converted = new global::System.Drawing.PointF[points.Length]; for (var i = 0; i < points.Length; ++i) converted[i] = new global::System.Drawing.PointF(points[i].X, points[i].Y); NativeDrawingBridge.FillPolygon(__recorder, brush.__handle, converted); }";
        if (method.Name == "Save") return "{ __EnsureRecorder(); return new global::System.Drawing.Drawing2D.GraphicsState { __token = NativeDrawingBridge.RecorderSave(__recorder) }; }";
        if (method.Name == "Restore") return "{ if (gstate is null) throw new global::System.ArgumentNullException(nameof(gstate)); __EnsureRecorder(); NativeDrawingBridge.RecorderRestore(__recorder, gstate.__token); }";
        if (method.Name == "TranslateTransform") return "{ __transform.Translate(dx, dy); __EnsureRecorder(); NativeDrawingBridge.RecorderTranslate(__recorder, dx, dy); }";
        if (method.Name == "SetClip") return "{ __EnsureRecorder(); NativeDrawingBridge.RecorderSetClip(__recorder, new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height)); }";
        if (method.Name == "Flush") return "{ }";
        if (method.Name == "ReleaseHdc") return "{ NativeDrawingBridge.ReleaseHdc(this, hdc); }";
        if (method.Name == "GetHdc") return "{ return NativeDrawingBridge.GetHdc(this); }";
        if (method.Name == "IsVisible") return "{ __EnsureRecorder(); return NativeDrawingBridge.RecorderIsVisible(__recorder, point); }";
        if (method.Name == "DrawIcon") return "{ if (icon is null) throw new global::System.ArgumentNullException(nameof(icon)); using var image = icon.ToBitmap(); DrawImage(image, targetRect); }";
    }
    if (type.FullName == "System.Drawing.Drawing2D.GraphicsPath")
    {
        if (method.Name == "Dispose") return "{ if (__pathDisposed) return; __pathDisposed = true; global::System.Drawing.NativeDrawingBridge.Release(ref __handle); global::System.GC.SuppressFinalize(this); }";
        if (method.Name == "Reset") return "{ global::System.Drawing.NativeDrawingBridge.GraphicsPathReset(__handle); }";
        if (method.Name == "StartFigure") return "{ global::System.Drawing.NativeDrawingBridge.GraphicsPathStart(__handle); }";
        if (method.Name == "CloseFigure") return "{ global::System.Drawing.NativeDrawingBridge.GraphicsPathClose(__handle); }";
        if (method.Name == "AddLine")
        {
            if (method.GetParameters().Length == 2) return "{ global::System.Drawing.NativeDrawingBridge.GraphicsPathAddLine(__handle, pt1, pt2); }";
            return "{ global::System.Drawing.NativeDrawingBridge.GraphicsPathAddLine(__handle, new global::System.Drawing.PointF(x1, y1), new global::System.Drawing.PointF(x2, y2)); }";
        }
        if (method.Name == "AddRectangle") return "{ global::System.Drawing.NativeDrawingBridge.GraphicsPathAddRectangle(__handle, new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height)); }";
        if (method.Name == "AddEllipse") return "{ global::System.Drawing.NativeDrawingBridge.GraphicsPathAddEllipse(__handle, rect); }";
        if (method.Name == "AddArc")
        {
            if (method.GetParameters().Length == 3) return "{ global::System.Drawing.NativeDrawingBridge.GraphicsPathAddArc(__handle, new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height), startAngle, sweepAngle); }";
            return "{ global::System.Drawing.NativeDrawingBridge.GraphicsPathAddArc(__handle, new global::System.Drawing.RectangleF(x, y, width, height), startAngle, sweepAngle); }";
        }
        if (method.Name == "AddPath") return "{ if (addingPath is null) throw new global::System.ArgumentNullException(nameof(addingPath)); global::System.Drawing.NativeDrawingBridge.GraphicsPathAddPath(__handle, addingPath.__handle, connect); }";
        if (method.Name == "GetBounds") return "{ return global::System.Drawing.NativeDrawingBridge.GraphicsPathBounds(__handle); }";
        if (method.Name == "IsVisible") return "{ return global::System.Drawing.NativeDrawingBridge.GraphicsPathIsVisible(__handle, new global::System.Drawing.PointF(point.X, point.Y)); }";
        if (method.Name == "Clone") return "{ return global::System.Drawing.NativeDrawingBridge.GraphicsPathClone(__handle); }";
        if (method.Name == "Transform") return "{ if (matrix is null) throw new global::System.ArgumentNullException(nameof(matrix)); global::System.Drawing.NativeDrawingBridge.GraphicsPathTransform(__handle, matrix); }";
        if (method.Name == "AddString") return "{ if (s is null) throw new global::System.ArgumentNullException(nameof(s)); var width = global::System.Math.Max(0f, s.Length * emSize * 0.55f); global::System.Drawing.NativeDrawingBridge.GraphicsPathAddRectangle(__handle, new global::System.Drawing.RectangleF(origin.X, origin.Y - emSize, width, emSize * 1.2f)); }";
    }
    if (type.FullName == "System.Drawing.Drawing2D.Matrix")
    {
        if (method.Name == "Dispose") return "{ }";
        if (method.Name == "Translate") return "{ __dx += offsetX; __dy += offsetY; }";
        if (method.Name == "RotateAt") return "{ var radians = angle * (float)(global::System.Math.PI / 180.0); var cosine = (float)global::System.Math.Cos(radians); var sine = (float)global::System.Math.Sin(radians); __m11 = cosine; __m12 = sine; __m21 = -sine; __m22 = cosine; __dx = point.X - point.X * cosine + point.Y * sine; __dy = point.Y - point.X * sine - point.Y * cosine; }";
    }
    if (type.FullName == "System.Drawing.Region")
    {
        if (method.Name == "Dispose") return "{ if (__regionDisposed) return; __regionDisposed = true; NativeDrawingBridge.Release(ref __handle); global::System.GC.SuppressFinalize(this); }";
        if (method.Name == "Exclude") return "{ NativeDrawingBridge.RegionExclude(__handle, new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height)); }";
        if (method.Name == "Union" && method.GetParameters()[0].ParameterType.FullName == "System.Drawing.Rectangle") return "{ NativeDrawingBridge.RegionUnion(__handle, new global::System.Drawing.RectangleF(rect.X, rect.Y, rect.Width, rect.Height)); }";
        if (method.Name == "Union") return "{ if (path is null) throw new global::System.ArgumentNullException(nameof(path)); NativeDrawingBridge.RegionUnion(__handle, path.__handle); }";
    }
    if (type.FullName == "System.Drawing.Imaging.ImageAttributes")
    {
        if (method.Name == "Dispose") return "{ if (__attributesDisposed) return; __attributesDisposed = true; global::System.Drawing.NativeDrawingBridge.Release(ref __handle); global::System.GC.SuppressFinalize(this); }";
        if (method.Name == "SetColorMatrix") return "{ if (newColorMatrix is null) throw new global::System.ArgumentNullException(nameof(newColorMatrix)); global::System.Drawing.NativeDrawingBridge.ImageAttributesSetColorMatrix(__handle, newColorMatrix.__values); }";
        if (method.Name == "SetRemapTable") return "{ global::System.Drawing.NativeDrawingBridge.ImageAttributesSetRemap(__handle, map); }";
        if (method.Name == "Clone") return "{ return global::System.Drawing.NativeDrawingBridge.ImageAttributesClone(__handle); }";
    }
    if (type.FullName == "System.Windows.Forms.ButtonBase" && method.Name == "OnPaint")
        return "{ __PaintButtonSurface(pevent.Graphics); base.OnPaint(pevent); }";
    if (type.FullName == "System.Windows.Forms.BaseCollection" && method.Name == "GetEnumerator")
        return "{ return __Collection.GetEnumerator(); }";
    if (type.FullName == "System.Windows.Forms.BindingSource")
        return method.Name switch
        {
            "GetEnumerator" => "{ return __bindingList.GetEnumerator(); }",
            "Add" => "{ var result = __bindingList.Add(value); if (__position < 0) __position = 0; return result; }",
            "IndexOf" => "{ return __bindingList.IndexOf(value); }",
            "Clear" => "{ __bindingList.Clear(); __position = -1; }",
            "RemoveCurrent" => "{ if (__position < 0 || __position >= __bindingList.Count) return; __bindingList.RemoveAt(__position); if (__bindingList.Count == 0) __position = -1; else if (__position >= __bindingList.Count) __position = __bindingList.Count - 1; }",
            "ResetBindings" => "{ if (__bindingList.Count == 0) __position = -1; else if (__position < 0 || __position >= __bindingList.Count) __position = 0; }",
            _ => BodyFor(method.ReturnType, false),
        };
    if (type.FullName == "System.Windows.Forms.Application" && method.Name == "Run" &&
        method.GetParameters().Length == 1)
        return "{ if (mainForm is null) throw new global::System.ArgumentNullException(nameof(mainForm)); __RunContext(new ApplicationContext(mainForm)); }";
    if (type.FullName == "System.Windows.Forms.Application" && method.Name == "ExitThread")
        return "{ __context?.ExitThread(); }";
    if (type.FullName == "System.Windows.Forms.Application" && method.Name == "DoEvents")
        return "{ __CurrentForm?.__DoEvents(); }";
    if (type.FullName == "System.Windows.Forms.Control")
    {
        if (method.Name == "Dispose" && method.GetParameters().Length == 1) return "{ if (disposing) { var children = new global::System.Collections.Generic.List<Control>(); foreach (Control child in Controls) children.Add(child); foreach (var child in children) child.Dispose(); if (global::System.Object.ReferenceEquals(__focusedControl, this)) { __focusedControl = null; OnLostFocus(global::System.EventArgs.Empty); Leave?.Invoke(this, global::System.EventArgs.Empty); } __parent?.Controls.Remove(this); __native.Dispose(); } base.Dispose(disposing); }";
        if (method.Name == "Show" && method.GetParameters().Length == 0) return "{ Visible = true; }";
        if (method.Name == "Hide" && method.GetParameters().Length == 0) return "{ Visible = false; }";
        if (method.Name == "BeginInvoke") return "{ return __native.BeginInvoke(method); }";
        if (method.Name == "Invoke" && method.GetParameters().Length == 1 && method.ReturnType.FullName == "System.Void") return "{ __native.Invoke(method); }";
        if (method.Name == "Invoke" && method.GetParameters().Length == 1) return "{ return __native.Invoke(method); }";
        if (method.Name == "BringToFront") return "{ __parent?.Controls.__BringToFront(this); }";
        if (method.Name == "SendToBack") return "{ __parent?.Controls.__SendToBack(this); }";
        if (method.Name == "Focus") return "{ if (!Enabled || !Visible || __native.IsDisposed) return false; var previous = __focusedControl; if (global::System.Object.ReferenceEquals(previous, this)) return true; __focusedControl = this; if (previous is not null) { previous.OnLostFocus(global::System.EventArgs.Empty); previous.Leave?.Invoke(previous, global::System.EventArgs.Empty); } Enter?.Invoke(this, global::System.EventArgs.Empty); OnGotFocus(global::System.EventArgs.Empty); return true; }";
        if (method.Name == "FindForm") return "{ for (Control? current = this; current is not null; current = current.Parent) if (current is Form form) return form; return null!; }";
        if (method.Name == "SuspendLayout") return "{ ++__layoutSuspendDepth; }";
        if (method.Name == "ResumeLayout" && method.GetParameters().Length == 0) return "{ if (__layoutSuspendDepth > 0) --__layoutSuspendDepth; if (__layoutSuspendDepth == 0) PerformLayout(); }";
        if (method.Name == "ResumeLayout" && method.GetParameters().Length == 1) return "{ if (__layoutSuspendDepth > 0) --__layoutSuspendDepth; if (__layoutSuspendDepth == 0 && performLayout) PerformLayout(); }";
        if (method.Name == "PerformLayout") return "{ if (__layoutSuspendDepth != 0 || __performingLayout) return; __performingLayout = true; try { OnLayout(new LayoutEventArgs()); } finally { __performingLayout = false; } }";
        if (method.Name == "Invalidate") return "{ __QueueManagedPaint(); }";
        if (method.Name == "Update") return "{ if (__native.IsDisposed) return; var form = FindForm(); if (form is not null && !form.Visible) return; if (InvokeRequired) Invoke((global::System.Action)__RenderManagedPaintNow); else __RenderManagedPaintNow(); }";
        if (method.Name == "UpdateStyles") return "{ }";
        if (method.Name == "Refresh") return "{ Invalidate(); Update(); }";
        if (method.Name == "SetStyle") return "{ __SetStyle(flag, value); }";
        if (method.Name == "ResetBackColor") return "{ if (__backColor.IsEmpty) return; __backColor = global::System.Drawing.Color.Empty; __ApplyEffectiveColors(); OnBackColorChanged(global::System.EventArgs.Empty); if (__loadRaised) __QueueManagedPaint(); }";
        if (method.Name == "PointToScreen") return "{ var offset = __ScreenOffset(); return new global::System.Drawing.Point(p.X + offset.X, p.Y + offset.Y); }";
        if (method.Name == "PointToClient") return "{ var offset = __ScreenOffset(); return new global::System.Drawing.Point(p.X - offset.X, p.Y - offset.Y); }";
        if (method.Name == "RectangleToScreen") return "{ var offset = __ScreenOffset(); return new global::System.Drawing.Rectangle(r.X + offset.X, r.Y + offset.Y, r.Width, r.Height); }";
        if (method.Name == "RectangleToClient") return "{ var offset = __ScreenOffset(); return new global::System.Drawing.Rectangle(r.X - offset.X, r.Y - offset.Y, r.Width, r.Height); }";
        if (method.Name == "OnKeyDown") return "{ KeyDown?.Invoke(this, e); }";
        if (method.Name == "OnEnabledChanged") return "{ EnabledChanged?.Invoke(this, e); }";
        if (method.Name == "OnFontChanged") return "{ FontChanged?.Invoke(this, e); }";
        if (method.Name == "OnForeColorChanged") return "{ ForeColorChanged?.Invoke(this, e); }";
        if (method.Name == "OnGotFocus") return "{ GotFocus?.Invoke(this, e); }";
        if (method.Name == "OnLostFocus") return "{ LostFocus?.Invoke(this, e); }";
        if (method.Name == "OnMouseDown") return "{ MouseDown?.Invoke(this, e); }";
        if (method.Name == "OnMouseEnter") return "{ MouseEnter?.Invoke(this, e); }";
        if (method.Name == "OnMouseHover") return "{ MouseHover?.Invoke(this, e); }";
        if (method.Name == "OnMouseLeave") return "{ MouseLeave?.Invoke(this, e); }";
        if (method.Name == "OnMouseMove") return "{ MouseMove?.Invoke(this, e); }";
        if (method.Name == "OnMouseUp") return "{ MouseUp?.Invoke(this, e); }";
        if (method.Name == "OnMouseWheel") return "{ MouseWheel?.Invoke(this, e); }";
        if (method.Name == "OnPaint") return "{ Paint?.Invoke(this, e); }";
        if (method.Name == "OnParentChanged") return "{ ParentChanged?.Invoke(this, e); }";
        if (method.Name == "OnResize") return "{ Resize?.Invoke(this, e); }";
        if (method.Name == "OnSizeChanged") return "{ SizeChanged?.Invoke(this, e); }";
        if (method.Name == "OnLayout") return "{ __ApplyDockLayout(); Layout?.Invoke(this, levent); }";
        if (method.Name == "ProcessDialogKey") return "{ return false; }";
    }
    if (type.FullName == "System.Windows.Forms.ComboBox" && method.Name == "Select")
        return "{ __SetComboSelection(start, length); }";
    if (type.FullName == "System.Windows.Forms.TextBoxBase")
        return method.Name switch
        {
            "AppendText" => "{ if (text is null) return; __SetTextSelection(Text.Length, 0); __ReplaceTextSelection(text); }",
            "Clear" => "{ if (__textReadOnly) return; Text = string.Empty; __textCaret = __textAnchor = 0; Invalidate(); }",
            "DeselectAll" => "{ __NormalizeTextSelection(); __textAnchor = __textCaret; Invalidate(); }",
            "Select" => "{ __SetTextSelection(start, length); }",
            "SelectAll" => "{ __SetTextSelection(0, Text.Length); }",
            _ => BodyFor(method.ReturnType, false),
        };
    if (method.Name == "Dispose" && method.GetParameters().Length == 1 &&
        method.GetParameters()[0].ParameterType.FullName == "System.Boolean")
        return "{ base.Dispose(disposing); }";
    if (type.FullName == "System.Windows.Forms.Form")
    {
        if (method.Name == "Close") return "{ __RequestClose(); }";
        if (method.Name == "ShowDialog") return "{ if (__TraceLifecycle) global::System.Console.Error.WriteLine(\"facade-dialog=show|type=\" + GetType().FullName + \"|name=\" + Name + \"|text=\" + Text + \"|size=\" + Width + \"x\" + Height); Application.__CloseActiveMenu(); var previousActiveForm = Application.__CurrentForm; Application.__CurrentForm = this; try { __RaiseLoad(); Visible = true; _ = __RunNativeWindow(); } finally { Application.__CloseActiveMenu(); Visible = false; Application.__CurrentForm = previousActiveForm; if (__TraceLifecycle) global::System.Console.Error.WriteLine(\"facade-dialog=closed|type=\" + GetType().FullName + \"|result=\" + __dialogResult); } return __dialogResult; }";
        if (method.Name == "SetBoundsCore") return "{ Bounds = new global::System.Drawing.Rectangle(x, y, width, height); }";
        if (method.Name == "OnFormClosing") return "{ FormClosing?.Invoke(this, e); }";
        if (method.Name == "OnActivated") return "{ }";
        if (method.Name == "Activate") return "{ Focus(); }";
    }
    if (type.FullName == "System.Windows.Forms.Control+ControlCollection")
    {
        return method.Name switch
        {
            "Add" => "{ if (value is null) throw new global::System.ArgumentNullException(nameof(value)); if (global::System.Object.ReferenceEquals(value, __owner)) throw new global::System.ArgumentException(\"A control cannot parent itself.\", nameof(value)); if (global::System.Object.ReferenceEquals(value.__parent, __owner)) return; value.__parent?.Controls.Remove(value); __owner.__native.AddChild(value.__native); __items.Add(value); value.__parent = __owner; value.__ApplyEffectiveColors(); value.OnParentChanged(global::System.EventArgs.Empty); if (__owner is Panel && __owner is not TableLayoutPanel && __owner.Height == 0 && value.Visible && value.Dock is DockStyle.Top or DockStyle.Bottom) { var preferred = value.__PreferredLayoutSize(); if (preferred.Height > 0) __owner.Height = preferred.Height + __owner.Padding.Top + __owner.Padding.Bottom; } __owner.PerformLayout(); if (__owner.__HasRaisedLoad) value.__RaiseLoad(); }",
            "AddRange" => "{ foreach (var child in controls) Add(child); }",
            "Remove" => "{ if (__items.Remove(value)) { __owner.__native.RemoveChild(value.__native); value.__parent = null; value.OnParentChanged(global::System.EventArgs.Empty); } }",
            "Clear" => "{ foreach (var child in __items.ToArray()) Remove(child); }",
            "Contains" => "{ return __items.Contains(control); }",
            "IndexOf" => "{ return __items.IndexOf(control); }",
            "SetChildIndex" => "{ if (!__items.Remove(child)) throw new global::System.ArgumentException(\"Control is not a child.\", nameof(child)); var bounded = global::System.Math.Clamp(newIndex, 0, __items.Count); __items.Insert(bounded, child); __owner.__native.SetChildIndex(child.__native, bounded); }",
            _ => BodyFor(method.ReturnType, false),
        };
    }
    if (type.FullName == "System.Windows.Forms.TableLayoutPanel")
        return method.Name switch
        {
            "SetColumnSpan" => "{ if (value < 1) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __columnSpans[control] = value; PerformLayout(); }",
            "SetRowSpan" => "{ if (value < 1) throw new global::System.ArgumentOutOfRangeException(nameof(value)); __rowSpans[control] = value; PerformLayout(); }",
            _ => BodyFor(method.ReturnType, false),
        };
    if (type.FullName == "System.Windows.Forms.TableLayoutControlCollection" && method.Name == "Add")
        return "{ __tableOwner?.__SetCell(control, column, row); base.Add(control); }";
    if (type.FullName == "System.Windows.Forms.DataGridView")
        return method.Name switch
        {
            "DisplayedRowCount" => "{ return Rows.Count; }",
            "ClearSelection" => "{ SelectionChanged?.Invoke(this, global::System.EventArgs.Empty); }",
            "NotifyCurrentCellDirty" => "{ }",
            _ => BodyFor(method.ReturnType, false),
        };
    if (type.FullName == "System.Windows.Forms.DataGridViewColumnCollection" && method.Name == "AddRange")
        return "{ if (dataGridViewColumns is null) throw new global::System.ArgumentNullException(nameof(dataGridViewColumns)); foreach (var column in dataGridViewColumns) { if (column is null) throw new global::System.ArgumentNullException(nameof(dataGridViewColumns)); column.__dataGridView = __owner; __items.Add(column); } }";
    if (type.FullName == "System.Windows.Forms.TableLayoutColumnStyleCollection" && method.Name == "Add")
        return "{ if (columnStyle is null) throw new global::System.ArgumentNullException(nameof(columnStyle)); __styles.Add(columnStyle); return __styles.Count - 1; }";
    if (type.FullName == "System.Windows.Forms.TableLayoutRowStyleCollection" && method.Name == "Add")
        return "{ if (rowStyle is null) throw new global::System.ArgumentNullException(nameof(rowStyle)); __styles.Add(rowStyle); return __styles.Count - 1; }";
    if (type.FullName == "System.Windows.Forms.ComboBox+ObjectCollection")
        return method.Name switch
        {
            "Add" => "{ if (item is null) throw new global::System.ArgumentNullException(nameof(item)); __items.Add(item); __owner?.Invalidate(); return __items.Count - 1; }",
            "AddRange" => "{ if (items is null) throw new global::System.ArgumentNullException(nameof(items)); foreach (var item in items) Add(item); }",
            "Clear" => "{ __items.Clear(); __owner?.__ResetItemsSelection(); }",
            "GetEnumerator" => "{ return __items.GetEnumerator(); }",
            "IndexOf" => "{ return __items.IndexOf(value); }",
            "Insert" => "{ if (item is null) throw new global::System.ArgumentNullException(nameof(item)); __items.Insert(index, item); if (__owner is not null && __owner.SelectedIndex >= index) __owner.__SetSelectedIndexSilently(__owner.SelectedIndex + 1); __owner?.Invalidate(); }",
            "RemoveAt" => "{ var selected = __owner?.SelectedIndex ?? -1; __items.RemoveAt(index); if (__owner is not null) { if (selected == index) __owner.SelectedIndex = -1; else if (selected > index) __owner.__SetSelectedIndexSilently(selected - 1); __owner.Invalidate(); } }",
            _ => BodyFor(method.ReturnType, false),
        };
    if (type.FullName == "System.Windows.Forms.ListBox+ObjectCollection")
        return method.Name switch
        {
            "Add" => "{ if (item is null) throw new global::System.ArgumentNullException(nameof(item)); __items.Add(item); __owner?.Invalidate(); return __items.Count - 1; }",
            "Insert" => "{ if (item is null) throw new global::System.ArgumentNullException(nameof(item)); __items.Insert(index, item); if (__owner is not null && __owner.SelectedIndex >= index) __owner.SelectedIndex += 1; __owner?.Invalidate(); }",
            "RemoveAt" => "{ __items.RemoveAt(index); if (__owner is not null && __owner.SelectedIndex >= __items.Count) __owner.SelectedIndex = __items.Count - 1; __owner?.Invalidate(); }",
            _ => BodyFor(method.ReturnType, false),
        };
    if (type.FullName == "System.Windows.Forms.Timer")
        return method.Name switch
        {
            "Start" => "{ Enabled = true; }",
            "Stop" => "{ Enabled = false; }",
            _ => BodyFor(method.ReturnType, false),
        };
    if (type.FullName == "System.Windows.Forms.ToolStripItemCollection")
        return method.Name switch
        {
            "Add" when method.GetParameters().Length == 1 => "{ if (value is null) throw new global::System.ArgumentNullException(nameof(value)); value.__owner = __owner; __items.Add(value); __owner?.__RefreshItems(); return __items.Count - 1; }",
            "Add" when method.GetParameters().Length == 2 => "{ var item = new ToolStripMenuItem(text) { Image = image }; Add(item); return item; }",
            "AddRange" => "{ foreach (var item in toolStripItems) Add(item); }",
            "Clear" => "{ foreach (var item in __items) item.__owner = null; __items.Clear(); __owner?.__RefreshItems(); }",
            _ => BodyFor(method.ReturnType, false),
        };
    if (method.ReturnType.FullName == "System.Void" && method.GetParameters().Length == 1 &&
        OverridesBase(method) && method.Name is "OnLayout" or "OnResize" or "OnSizeChanged" or
            "OnParentChanged" or "OnPaint" or "OnTextChanged" or "OnVisibleChanged")
        return $"{{ base.{method.Name}({Escape(method.GetParameters()[0].Name ?? "e")}); }}";
    return BodyFor(method.ReturnType, type.IsValueType);
}

static string BodyFor(Type? returnType, bool structConstructor) => structConstructor ? "{ this = default; }" :
    returnType is null || returnType.FullName == "System.Void" ? "{ }" : "{ return default!; }";

static bool OverridesBase(MethodInfo method) => method.IsVirtual &&
    (method.Attributes & MethodAttributes.NewSlot) == 0;

static bool CanEmitOverride(Type type, MethodInfo method,
                            Dictionary<Type, HashSet<MemberInfo>> selected)
{
    if (!OverridesBase(method)) return false;
    const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic |
        BindingFlags.Instance | BindingFlags.DeclaredOnly;
    for (var current = type.BaseType; current is not null; current = current.BaseType)
    {
        var candidate = current.GetMethods(flags).FirstOrDefault(value =>
            value.Name == method.Name && MemberSignature(value) == MemberSignature(method));
        if (candidate is null) continue;
        var assembly = current.Assembly.GetName().Name;
        if (current.FullName == "System.Windows.Forms.Control" &&
            method.Name is "OnTextChanged" or "OnVisibleChanged")
            return true;
        return assembly is not ("System.Windows.Forms" or "System.Windows.Forms.Primitives") ||
            selected.TryGetValue(current, out var members) && members.Contains(candidate);
    }
    return false;
}

static string Access(MethodBase method) => method.IsPublic ? "public" : method.IsFamily || method.IsFamilyOrAssembly ? "protected" : "public";

static string Parameters(ParameterInfo[] parameters, bool includeDefault) => string.Join(", ", parameters.Select((parameter, index) =>
{
    var type = parameter.ParameterType;
    var prefix = "";
    if (type.IsByRef) { prefix = parameter.IsOut ? "out " : parameter.IsIn ? "in " : "ref "; type = type.GetElementType()!; }
    var value = prefix + CsType(type) + " " + Escape(string.IsNullOrEmpty(parameter.Name) ? "p" + index : parameter.Name!);
    if (includeDefault && parameter.HasDefaultValue) value += " = default";
    return value;
}));

static string CsType(Type type)
{
    if (type.IsByRef) return CsType(type.GetElementType()!);
    if (type.IsPointer) return "nint";
    if (type.IsArray) return CsType(type.GetElementType()!) + "[" + new string(',', type.GetArrayRank() - 1) + "]";
    if (type.IsGenericParameter) return type.Name;
    if (type.IsGenericType)
    {
        var name = type.GetGenericTypeDefinition().FullName!.Split('`')[0].Replace('+', '.');
        return "global::" + name + "<" + string.Join(", ", type.GetGenericArguments().Select(CsType)) + ">";
    }
    return type.FullName switch
    {
        "System.Void" => "void", "System.Boolean" => "bool", "System.Byte" => "byte", "System.SByte" => "sbyte",
        "System.Int16" => "short", "System.UInt16" => "ushort", "System.Int32" => "int", "System.UInt32" => "uint",
        "System.Int64" => "long", "System.UInt64" => "ulong", "System.Single" => "float", "System.Double" => "double",
        "System.Decimal" => "decimal", "System.Char" => "char", "System.String" => "string", "System.Object" => "object",
        "System.IntPtr" => "nint", "System.UIntPtr" => "nuint", _ => "global::" + (type.FullName ?? type.Name).Replace('+', '.')
    };
}

static string CleanName(Type type)
{
    var name = type.Name;
    var tick = name.IndexOf('`');
    if (tick < 0) return Escape(name);
    var args = type.GetGenericArguments().Where(arg => arg.DeclaringType == type).Select(arg => arg.Name);
    return Escape(name[..tick]) + "<" + string.Join(",", args) + ">";
}

static string Escape(string name) => Regex.IsMatch(name, "^(event|object|string|ref|out|in|base|this|params|internal|public|private|protected|new|default|operator|checked|unchecked|fixed|lock|is|as)$") ? "@" + name : name;

static string ProjectFile(string assemblyName, string windowsRef)
{
    var projectReferences = assemblyName switch
    {
        "System.Windows.Forms" =>
            "    <ProjectReference Include=\"../System.Windows.Forms.Primitives/System.Windows.Forms.Primitives.csproj\" />\n" +
            "    <ProjectReference Include=\"../System.Drawing.Common/System.Drawing.Common.csproj\" />\n",
        "System.Windows.Forms.Primitives" =>
            "    <ProjectReference Include=\"../System.Drawing.Common/System.Drawing.Common.csproj\" />\n",
        _ => "",
    };
    var windowsReferences = assemblyName == "System.Drawing.Common" ? "" : $"""
    <Reference Include="System.Private.Windows.Core"><HintPath>{Path.Combine(windowsRef, "System.Private.Windows.Core.dll")}</HintPath></Reference>
    <Reference Include="System.Private.Windows.GdiPlus"><HintPath>{Path.Combine(windowsRef, "System.Private.Windows.GdiPlus.dll")}</HintPath></Reference>
    <Reference Include="Accessibility"><HintPath>{Path.Combine(windowsRef, "Accessibility.dll")}</HintPath></Reference>
""";
    return $"""
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net10.0</TargetFramework>
    <AssemblyName>{assemblyName}</AssemblyName>
    <RootNamespace>{(assemblyName == "System.Drawing.Common" ? "System.Drawing" : "System.Windows.Forms")}</RootNamespace>
    <Nullable>enable</Nullable>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
    <DebugType>none</DebugType>
    <DebugSymbols>false</DebugSymbols>
    <Deterministic>true</Deterministic>
    <GenerateAssemblyInfo>false</GenerateAssemblyInfo>
  </PropertyGroup>
  <ItemGroup>
{projectReferences}{windowsReferences}
  </ItemGroup>
</Project>
""";
}

static string DrawingBridgeSource() => """
// <auto-generated/>
#nullable enable
using System.Runtime.InteropServices;
using System.Text;

namespace System.Drawing;

internal static unsafe class NativeDrawingBridge
{
    [StructLayout(LayoutKind.Sequential)] internal record struct Handle(uint Slot, uint Generation)
    {
        internal readonly bool IsNull => Slot == 0;
    }
    [StructLayout(LayoutKind.Sequential)] private struct NativeColor { internal uint Argb, IsEmpty; }
    [StructLayout(LayoutKind.Sequential)] private struct Point { internal double X, Y; }
    [StructLayout(LayoutKind.Sequential)] private struct Rect { internal double X, Y, Width, Height; }
    [StructLayout(LayoutKind.Sequential)] private struct RectI { internal int X, Y, Width, Height; }
    [StructLayout(LayoutKind.Sequential)] private struct MatrixValue { internal double M11, M12, M21, M22, Dx, Dy; }
    [StructLayout(LayoutKind.Sequential)] private struct BitmapLockView
    {
        internal void* Data;
        internal void* WritableData;
        internal ulong RowBytes;
        internal uint Width, Height, PixelFormat;
        internal ulong Token;
    }
    [StructLayout(LayoutKind.Sequential)] private struct ColorRemap
    {
        internal NativeColor OldColor, NewColor;
    }
    [StructLayout(LayoutKind.Sequential)] private struct StringView { internal byte* Data; internal ulong Size; }
    [StructLayout(LayoutKind.Sequential)] private struct Api
    {
        internal uint StructSize, AbiVersion;
        internal fixed ulong Entries[96];
    }

    [DllImport("gui_drawing_abi0", EntryPoint = "gd_get_api_v0", CallingConvention = CallingConvention.Cdecl)]
    private static extern int GetApi(uint requestedVersion, ref Api api);
    [DllImport("gui_drawing_raster0", EntryPoint = "gdr_initialize", CallingConvention = CallingConvention.Cdecl)]
    private static extern int InitializeRaster();

    private static Api api = Load();

    private static Api Load()
    {
        var value = new Api { StructSize = (uint)sizeof(Api) };
        Check(GetApi(1, ref value));
        if (value.AbiVersion != 1 || value.StructSize < sizeof(Api))
            throw new InvalidOperationException("GUI.Drawing ABI 0.1 table is incomplete.");
        return value;
    }

    private static int rasterInitialized;
    private static readonly object rasterLock = new();
    private static void EnsureRaster()
    {
        if (global::System.Threading.Volatile.Read(ref rasterInitialized) != 0) return;
        lock (rasterLock)
        {
            if (rasterInitialized != 0) return;
            Check(InitializeRaster());
            global::System.Threading.Volatile.Write(ref rasterInitialized, 1);
        }
    }

    private static nint Entry(int index)
    {
        fixed (ulong* entries = api.Entries) return (nint)entries[index];
    }

    private static void Check(int result)
    {
        if (result != 0) throw new InvalidOperationException("GUI.Drawing native operation failed with result " + result + ".");
    }

    private static NativeColor Native(global::System.Drawing.Color color) =>
        new() { Argb = unchecked((uint)color.ToArgb()), IsEmpty = color.IsEmpty ? 1u : 0u };
    private static Rect Native(global::System.Drawing.RectangleF value) =>
        new() { X = value.X, Y = value.Y, Width = value.Width, Height = value.Height };

    internal static void Release(ref Handle handle)
    {
        if (handle.IsNull) return;
        Check(((delegate* unmanaged[Cdecl]<Handle, int>)Entry(2))(handle));
        handle = default;
    }

    internal static Handle BitmapCreate(int width, int height)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<uint, uint, uint, Handle*, int>)Entry(49))(
            checked((uint)width), checked((uint)height), 0, &handle));
        return handle;
    }

    internal static Handle BitmapClone(Handle source, int x, int y, int width, int height)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<Handle, RectI, Handle*, int>)Entry(55))(
            source, new RectI { X = x, Y = y, Width = width, Height = height }, &handle));
        return handle;
    }

    private static void Dimensions(Handle bitmap, out int width, out int height)
    {
        uint w, h, format;
        ulong generation;
        Check(((delegate* unmanaged[Cdecl]<Handle, uint*, uint*, uint*, ulong*, int>)Entry(50))(
            bitmap, &w, &h, &format, &generation));
        width = checked((int)w);
        height = checked((int)h);
    }

    internal static global::System.Drawing.Color BitmapGetPixel(Handle bitmap, int x, int y)
    {
        NativeColor color;
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, uint, NativeColor*, int>)Entry(51))(
            bitmap, checked((uint)x), checked((uint)y), &color));
        return global::System.Drawing.Color.FromArgb(unchecked((int)color.Argb));
    }

    internal static void BitmapMakeTransparent(Handle bitmap, global::System.Drawing.Color color) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, NativeColor, int>)Entry(56))(bitmap, Native(color)));

    internal static global::System.Drawing.Bitmap CloneBitmap(global::System.Drawing.Image image)
    {
        var result = new global::System.Drawing.Bitmap();
        result.__width = image.Width;
        result.__height = image.Height;
        result.__pixelFormat = image.PixelFormat;
        result.__bitmap = BitmapClone(image.__BitmapHandle, 0, 0, image.Width, image.Height);
        return result;
    }

    internal static global::System.Drawing.Bitmap Thumbnail(global::System.Drawing.Image image, int width, int height)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, uint, Handle*, int>)Entry(57))(
            image.__BitmapHandle, checked((uint)width), checked((uint)height), &handle));
        return WrapBitmap(handle, width, height);
    }

    internal static Handle SolidBrushCreate(global::System.Drawing.Color color)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<NativeColor, Handle*, int>)Entry(6))(Native(color), &handle));
        return handle;
    }

    internal static Handle PenCreate(global::System.Drawing.Color color, float width)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<NativeColor, double, Handle*, int>)Entry(7))(
            Native(color), width, &handle));
        return handle;
    }
    internal static void PenSetWidth(Handle pen, float width) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, double, int>)Entry(8))(pen, width));
    internal static void PenSetDashStyle(Handle pen, uint style) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, int>)Entry(9))(pen, style));
    internal static void PenSetDashPattern(Handle pen, float[] pattern)
    {
        if (pattern is null) throw new ArgumentNullException(nameof(pattern));
        var values = new double[pattern.Length];
        for (var index = 0; index < pattern.Length; ++index) values[index] = pattern[index];
        fixed (double* pointer = values)
            Check(((delegate* unmanaged[Cdecl]<Handle, double*, ulong, int>)Entry(10))(
                pen, pointer, (ulong)values.Length));
    }

    internal static Handle FontCreate(string family, float size,
                                      global::System.Drawing.FontStyle style,
                                      global::System.Drawing.GraphicsUnit unit,
                                      byte charset)
    {
        var bytes = Encoding.UTF8.GetBytes(family);
        fixed (byte* pointer = bytes)
        {
            Handle handle;
            Check(((delegate* unmanaged[Cdecl]<StringView, double, uint, uint, uint, Handle*, int>)Entry(11))(
                new StringView { Data = pointer, Size = (ulong)bytes.Length }, size,
                (uint)style, (uint)unit, charset, &handle));
            return handle;
        }
    }

    internal static Handle StringFormatCreate(uint flags)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<uint, Handle*, int>)Entry(12))(flags, &handle));
        return handle;
    }
    internal static void StringFormatSet(Handle format,
        global::System.Drawing.StringAlignment alignment,
        global::System.Drawing.StringAlignment lineAlignment,
        global::System.Drawing.StringTrimming trimming,
        global::System.Drawing.StringFormatFlags flags) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, uint, uint, uint, int>)Entry(13))(
            format, (uint)alignment, (uint)lineAlignment, (uint)trimming, (uint)flags));

    internal static Handle RecorderCreate()
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<Handle*, int>)Entry(14))(&handle));
        return handle;
    }
    internal static ulong RecorderSave(Handle recorder)
    {
        ulong token;
        Check(((delegate* unmanaged[Cdecl]<Handle, ulong*, int>)Entry(15))(recorder, &token));
        return token;
    }
    internal static void RecorderRestore(Handle recorder, ulong token) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, ulong, int>)Entry(16))(recorder, token));
    internal static void RecorderTranslate(Handle recorder, float x, float y) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, double, double, int>)Entry(17))(recorder, x, y));
    internal static void RecorderSetTransform(Handle recorder,
        global::System.Drawing.Drawing2D.Matrix matrix) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, MatrixValue, int>)Entry(18))(
            recorder, new MatrixValue { M11 = matrix.__m11, M12 = matrix.__m12,
                M21 = matrix.__m21, M22 = matrix.__m22,
                Dx = matrix.__dx, Dy = matrix.__dy }));
    internal static void RecorderSetClip(Handle recorder, global::System.Drawing.RectangleF rect) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Rect, int>)Entry(19))(recorder, Native(rect)));
    internal static bool RecorderIsVisible(Handle recorder, global::System.Drawing.PointF point)
    {
        uint visible;
        Check(((delegate* unmanaged[Cdecl]<Handle, Point, uint*, int>)Entry(22))(
            recorder, new Point { X = point.X, Y = point.Y }, &visible));
        return visible != 0;
    }
    internal static void RecorderClear(Handle recorder, global::System.Drawing.Color color) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, NativeColor, int>)Entry(23))(recorder, Native(color)));
    internal static void FillRectangle(Handle recorder, Handle brush, global::System.Drawing.RectangleF rect) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, Rect, int>)Entry(24))(recorder, brush, Native(rect)));
    internal static void DrawRectangle(Handle recorder, Handle pen, global::System.Drawing.RectangleF rect) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, Rect, int>)Entry(25))(recorder, pen, Native(rect)));
    internal static void DrawLine(Handle recorder, Handle pen,
                                  global::System.Drawing.PointF first,
                                  global::System.Drawing.PointF second) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, Point, Point, int>)Entry(26))(
            recorder, pen, new Point { X = first.X, Y = first.Y },
            new Point { X = second.X, Y = second.Y }));
    internal static void DrawString(Handle recorder, string text,
                                    global::System.Drawing.Font font,
                                    global::System.Drawing.Brush brush,
                                    global::System.Drawing.PointF origin,
                                    global::System.Drawing.StringFormat? format)
    {
        if (string.IsNullOrEmpty(text)) return;
        var temporary = format is null;
        format ??= new global::System.Drawing.StringFormat();
        try
        {
            var bytes = Encoding.UTF8.GetBytes(text);
            fixed (byte* pointer = bytes)
                Check(((delegate* unmanaged[Cdecl]<Handle, StringView, Handle, Handle, Point, Handle, int>)Entry(27))(
                    recorder, new StringView { Data = pointer, Size = (ulong)bytes.Length },
                    font.__handle, brush.__handle,
                    new Point { X = origin.X, Y = origin.Y }, format.__handle));
        }
        finally { if (temporary) format.Dispose(); }
    }
    internal static void RecorderQuality(global::System.Drawing.Graphics graphics)
    {
        graphics.__EnsureRecorder();
        static uint Interpolation(global::System.Drawing.Drawing2D.InterpolationMode value) => value switch
        {
            global::System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor => 3,
            global::System.Drawing.Drawing2D.InterpolationMode.Bilinear => 4,
            global::System.Drawing.Drawing2D.InterpolationMode.Bicubic => 5,
            global::System.Drawing.Drawing2D.InterpolationMode.HighQualityBilinear => 6,
            global::System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic => 7,
            _ => (uint)global::System.Math.Clamp((int)value, 0, 2),
        };
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, uint, uint, uint, uint, int>)Entry(21))(
            graphics.__recorder,
            (uint)global::System.Math.Clamp((int)graphics.__smoothing, 0, 4),
            Interpolation(graphics.__interpolation),
            (uint)global::System.Math.Clamp((int)graphics.__pixelOffset, 0, 4),
            (uint)global::System.Math.Clamp((int)graphics.__compositing, 0, 1),
            (uint)global::System.Math.Clamp((int)graphics.__compositingQuality, 0, 4)));
    }
    internal static void DrawEllipse(Handle recorder, Handle pen, global::System.Drawing.RectangleF rect) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, Rect, int>)Entry(43))(recorder, pen, Native(rect)));
    internal static void FillEllipse(Handle recorder, Handle brush, global::System.Drawing.RectangleF rect) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, Rect, int>)Entry(44))(recorder, brush, Native(rect)));

    internal static void DrawImage(Handle recorder, global::System.Drawing.Image image,
                                   global::System.Drawing.RectangleF destination,
                                   global::System.Drawing.RectangleF source,
                                   global::System.Drawing.Imaging.ImageAttributes? attributes)
    {
        var temporary = attributes is null;
        attributes ??= new global::System.Drawing.Imaging.ImageAttributes();
        try
        {
            Check(((delegate* unmanaged[Cdecl]<Handle, Handle, Rect, Rect, Handle, int>)Entry(59))(
                recorder, image.__BitmapHandle, Native(destination), Native(source), attributes.__handle));
        }
        finally { if (temporary) attributes.Dispose(); }
    }
    internal static global::System.Drawing.Imaging.BitmapData BitmapLock(
        Handle bitmap, global::System.Drawing.Imaging.ImageLockMode mode)
    {
        var nativeMode = mode switch
        {
            global::System.Drawing.Imaging.ImageLockMode.ReadOnly => 0u,
            global::System.Drawing.Imaging.ImageLockMode.WriteOnly => 1u,
            global::System.Drawing.Imaging.ImageLockMode.ReadWrite => 2u,
            _ => throw new ArgumentOutOfRangeException(nameof(mode)),
        };
        BitmapLockView view;
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, BitmapLockView*, int>)Entry(53))(
            bitmap, nativeMode, &view));
        return new global::System.Drawing.Imaging.BitmapData
        {
            __bitmap = bitmap, __token = view.Token,
            __scan0 = (nint)(view.WritableData != null ? view.WritableData : view.Data),
            __stride = checked((int)view.RowBytes),
        };
    }
    internal static void BitmapUnlock(Handle bitmap,
                                      global::System.Drawing.Imaging.BitmapData data)
    {
        if (data.__bitmap != bitmap || data.__token == 0)
            throw new ArgumentException("BitmapData does not belong to this bitmap.", nameof(data));
        Check(((delegate* unmanaged[Cdecl]<Handle, ulong, int>)Entry(54))(bitmap, data.__token));
        data.__token = 0;
        data.__scan0 = 0;
    }

    internal static Handle GraphicsPathCreate()
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<uint, Handle*, int>)Entry(31))(0, &handle));
        return handle;
    }
    internal static void GraphicsPathReset(Handle path) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, int>)Entry(32))(path));
    internal static void GraphicsPathStart(Handle path) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, int>)Entry(33))(path));
    internal static void GraphicsPathClose(Handle path) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, int>)Entry(34))(path));
    internal static void GraphicsPathAddLine(Handle path,
        global::System.Drawing.PointF first, global::System.Drawing.PointF second) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Point, Point, int>)Entry(35))(
            path, new Point { X = first.X, Y = first.Y },
            new Point { X = second.X, Y = second.Y }));
    internal static void GraphicsPathAddRectangle(Handle path,
        global::System.Drawing.RectangleF rect) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Rect, int>)Entry(36))(path, Native(rect)));
    internal static void GraphicsPathAddEllipse(Handle path,
        global::System.Drawing.RectangleF rect) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Rect, int>)Entry(37))(path, Native(rect)));
    internal static global::System.Drawing.RectangleF GraphicsPathBounds(Handle path)
    {
        Rect bounds;
        Check(((delegate* unmanaged[Cdecl]<Handle, Rect*, int>)Entry(38))(path, &bounds));
        return new((float)bounds.X, (float)bounds.Y, (float)bounds.Width, (float)bounds.Height);
    }
    internal static void GraphicsPathAddArc(Handle path,
        global::System.Drawing.RectangleF rect, float start, float sweep) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Rect, double, double, int>)Entry(60))(
            path, Native(rect), start, sweep));
    internal static void GraphicsPathAddPath(Handle path, Handle added, bool connect) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, uint, int>)Entry(61))(
            path, added, connect ? 1u : 0u));
    internal static void GraphicsPathTransform(Handle path,
        global::System.Drawing.Drawing2D.Matrix matrix) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, MatrixValue, int>)Entry(62))(
            path, new MatrixValue { M11 = matrix.__m11, M12 = matrix.__m12,
                M21 = matrix.__m21, M22 = matrix.__m22,
                Dx = matrix.__dx, Dy = matrix.__dy }));
    internal static bool GraphicsPathIsVisible(Handle path,
        global::System.Drawing.PointF point)
    {
        uint visible;
        Check(((delegate* unmanaged[Cdecl]<Handle, Point, uint*, int>)Entry(63))(
            path, new Point { X = point.X, Y = point.Y }, &visible));
        return visible != 0;
    }
    internal static global::System.Drawing.PointF[] GraphicsPathPoints(Handle path)
    {
        ulong required;
        var first = ((delegate* unmanaged[Cdecl]<Handle, Point*, ulong, ulong*, int>)Entry(64))(
            path, null, 0, &required);
        if (required == 0 && first == 0) return [];
        if (first != 6) Check(first);
        var native = new Point[checked((int)required)];
        fixed (Point* pointer = native)
            Check(((delegate* unmanaged[Cdecl]<Handle, Point*, ulong, ulong*, int>)Entry(64))(
                path, pointer, (ulong)native.Length, &required));
        var result = new global::System.Drawing.PointF[native.Length];
        for (var index = 0; index < result.Length; ++index)
            result[index] = new((float)native[index].X, (float)native[index].Y);
        return result;
    }
    internal static global::System.Drawing.Drawing2D.GraphicsPath GraphicsPathClone(Handle path)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle*, int>)Entry(65))(path, &handle));
        var result = new global::System.Drawing.Drawing2D.GraphicsPath();
        var replaced = result.__handle;
        Release(ref replaced);
        result.__handle = handle;
        return result;
    }

    internal static Handle HatchBrushCreate(uint style, global::System.Drawing.Color foreground,
                                            global::System.Drawing.Color background)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<uint, NativeColor, NativeColor, Handle*, int>)Entry(68))(
            style, Native(foreground), Native(background), &handle));
        return handle;
    }
    internal static Handle LinearGradientBrushCreate(global::System.Drawing.RectangleF bounds,
        global::System.Drawing.Color first, global::System.Drawing.Color second,
        float angle, global::System.Drawing.Drawing2D.WrapMode wrap)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<Rect, NativeColor, NativeColor, double, uint, Handle*, int>)Entry(69))(
            Native(bounds), Native(first), Native(second), angle, (uint)wrap, &handle));
        return handle;
    }
    internal static void LinearGradientSetInterpolation(Handle brush,
        global::System.Drawing.Color[] colors, float[] positions)
    {
        if (colors is null || positions is null || colors.Length != positions.Length)
            throw new ArgumentException("Gradient colors and positions must have equal lengths.");
        var nativeColors = new NativeColor[colors.Length];
        var nativePositions = new double[positions.Length];
        for (var index = 0; index < colors.Length; ++index)
        { nativeColors[index] = Native(colors[index]); nativePositions[index] = positions[index]; }
        fixed (NativeColor* colorPointer = nativeColors)
        fixed (double* positionPointer = nativePositions)
            Check(((delegate* unmanaged[Cdecl]<Handle, NativeColor*, double*, ulong, int>)Entry(71))(
                brush, colorPointer, positionPointer, (ulong)colors.Length));
    }
    internal static void LinearGradientSetWrap(Handle brush,
        global::System.Drawing.Drawing2D.WrapMode wrap) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, int>)Entry(72))(brush, (uint)wrap));
    internal static Handle PathGradientBrushCreate(global::System.Drawing.PointF[] points)
    {
        var native = new Point[points.Length];
        for (var index = 0; index < points.Length; ++index)
            native[index] = new Point { X = points[index].X, Y = points[index].Y };
        fixed (Point* pointer = native)
        {
            Handle handle;
            Check(((delegate* unmanaged[Cdecl]<Point*, ulong, uint, Handle*, int>)Entry(73))(
                pointer, (ulong)native.Length, 4, &handle));
            return handle;
        }
    }
    internal static void PathGradientSetCenterColor(Handle brush,
        global::System.Drawing.Color color) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, NativeColor, int>)Entry(74))(brush, Native(color)));
    internal static void PathGradientSetSurroundColors(Handle brush,
        global::System.Drawing.Color[] colors)
    {
        if (colors is null) throw new ArgumentNullException(nameof(colors));
        var native = new NativeColor[colors.Length];
        for (var index = 0; index < colors.Length; ++index) native[index] = Native(colors[index]);
        fixed (NativeColor* pointer = native)
            Check(((delegate* unmanaged[Cdecl]<Handle, NativeColor*, ulong, int>)Entry(76))(
                brush, pointer, (ulong)native.Length));
    }

    internal static Handle RegionCreate(global::System.Drawing.RectangleF rectangle)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<Rect, Handle*, int>)Entry(78))(Native(rectangle), &handle));
        return handle;
    }
    internal static Handle RegionCreate(Handle path)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle*, int>)Entry(79))(path, &handle));
        return handle;
    }
    internal static void RegionUnion(Handle region, global::System.Drawing.RectangleF rectangle) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Rect, int>)Entry(80))(region, Native(rectangle)));
    internal static void RegionUnion(Handle region, Handle path) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, int>)Entry(81))(region, path));
    internal static void RegionExclude(Handle region, global::System.Drawing.RectangleF rectangle) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Rect, int>)Entry(82))(region, Native(rectangle)));

    internal static Handle ImageAttributesCreate()
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<Handle*, int>)Entry(40))(&handle));
        return handle;
    }
    internal static void ImageAttributesSetColorMatrix(Handle attributes, float[] values)
    {
        if (values is null || values.Length != 25) throw new ArgumentException("Color matrix requires 25 values.");
        var native = new double[25];
        for (var index = 0; index < native.Length; ++index) native[index] = values[index];
        fixed (double* pointer = native)
            Check(((delegate* unmanaged[Cdecl]<Handle, double*, ulong, int>)Entry(41))(
                attributes, pointer, 25));
    }
    internal static void ImageAttributesSetRemap(Handle attributes,
        global::System.Drawing.Imaging.ColorMap[] maps)
    {
        if (maps is null) throw new ArgumentNullException(nameof(maps));
        var native = new ColorRemap[maps.Length];
        for (var index = 0; index < maps.Length; ++index)
            native[index] = new ColorRemap { OldColor = Native(maps[index].__oldColor),
                                             NewColor = Native(maps[index].__newColor) };
        fixed (ColorRemap* pointer = native)
            Check(((delegate* unmanaged[Cdecl]<Handle, ColorRemap*, ulong, int>)Entry(66))(
                attributes, pointer, (ulong)native.Length));
    }
    internal static global::System.Drawing.Imaging.ImageAttributes ImageAttributesClone(Handle attributes)
    {
        Handle handle;
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle*, int>)Entry(89))(attributes, &handle));
        var result = new global::System.Drawing.Imaging.ImageAttributes();
        Release(ref result.__handle);
        result.__handle = handle;
        return result;
    }

    internal static void FillPolygon(Handle recorder, Handle brush,
                                     global::System.Drawing.PointF[] points)
    {
        var native = new Point[points.Length];
        for (var index = 0; index < points.Length; ++index)
            native[index] = new Point { X = points[index].X, Y = points[index].Y };
        fixed (Point* pointer = native)
            Check(((delegate* unmanaged[Cdecl]<Handle, Handle, Point*, ulong, uint, int>)Entry(45))(
                recorder, brush, pointer, (ulong)native.Length, 0));
    }
    internal static void DrawPath(Handle recorder, Handle pen, Handle path) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, Handle, int>)Entry(46))(
            recorder, pen, path));
    internal static void FillPath(Handle recorder, Handle brush, Handle path) =>
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, Handle, int>)Entry(47))(
            recorder, brush, path));

    internal static void Execute(Handle recorder, Handle bitmap)
    {
        ulong recorded;
        Check(((delegate* unmanaged[Cdecl]<Handle, ulong*, int>)Entry(29))(
            recorder, &recorded));
        if (recorded == 0) return;
        EnsureRaster();
        ulong count;
        Check(((delegate* unmanaged[Cdecl]<Handle, Handle, ulong*, int>)Entry(86))(
            recorder, bitmap, &count));
    }

    internal static byte[] EncodePng(Handle bitmap)
    {
        EnsureRaster();
        ulong required;
        var first = ((delegate* unmanaged[Cdecl]<Handle, void*, ulong, ulong*, int>)Entry(87))(
            bitmap, null, 0, &required);
        if (first != 6) Check(first);
        var bytes = new byte[checked((int)required)];
        fixed (byte* pointer = bytes)
            Check(((delegate* unmanaged[Cdecl]<Handle, void*, ulong, ulong*, int>)Entry(87))(
                bitmap, pointer, (ulong)bytes.Length, &required));
        return bytes;
    }

    internal static global::System.Drawing.Bitmap DecodePng(byte[] bytes)
    {
        EnsureRaster();
        if (bytes is null || bytes.Length == 0) throw new ArgumentException("PNG data is empty.", nameof(bytes));
        fixed (byte* pointer = bytes)
        {
            Handle handle;
            Check(((delegate* unmanaged[Cdecl]<void*, ulong, Handle*, int>)Entry(88))(
                pointer, (ulong)bytes.Length, &handle));
            Dimensions(handle, out var width, out var height);
            var bitmap = WrapBitmap(handle, width, height);
            // PNG exposes straight-alpha ARGB semantics even though the owned
            // raster store normalizes its internal bytes to premultiplied BGRA.
            bitmap.__pixelFormat = global::System.Drawing.Imaging.PixelFormat.Format32bppArgb;
            return bitmap;
        }
    }
    internal static byte[] ReadBounded(global::System.IO.Stream stream, string format)
    {
        const int maximumBytes = 64 * 1024 * 1024;
        using var copy = new global::System.IO.MemoryStream();
        var buffer = new byte[81920];
        while (true)
        {
            var read = stream.Read(buffer, 0, buffer.Length);
            if (read == 0) break;
            if (copy.Length + read > maximumBytes)
                throw new ArgumentException(format + " data exceeds the 64 MiB compatibility limit.", nameof(stream));
            copy.Write(buffer, 0, read);
        }
        return copy.ToArray();
    }
    internal static global::System.Drawing.Bitmap DecodeIcon(byte[] bytes)
    {
        if (bytes is null || bytes.Length < 22 || bytes[0] != 0 || bytes[1] != 0 ||
            bytes[2] != 1 || bytes[3] != 0)
            throw new ArgumentException("ICO directory is invalid.", nameof(bytes));
        static ushort U16(byte[] data, int offset) =>
            (ushort)(data[offset] | data[offset + 1] << 8);
        static uint U32(byte[] data, int offset) =>
            (uint)(data[offset] | data[offset + 1] << 8 | data[offset + 2] << 16 |
                   data[offset + 3] << 24);
        var count = U16(bytes, 4);
        if (count == 0 || 6 + count * 16 > bytes.Length)
            throw new ArgumentException("ICO directory is truncated.", nameof(bytes));
        var selectedOffset = -1;
        var selectedSize = 0;
        var selectedScore = -1L;
        for (var index = 0; index < count; ++index)
        {
            var entry = 6 + index * 16;
            var size = checked((int)U32(bytes, entry + 8));
            var offset = checked((int)U32(bytes, entry + 12));
            if (size < 8 || offset < 0 || offset > bytes.Length - size) continue;
            if (bytes[offset] != 0x89 || bytes[offset + 1] != 0x50 ||
                bytes[offset + 2] != 0x4e || bytes[offset + 3] != 0x47 ||
                bytes[offset + 4] != 0x0d || bytes[offset + 5] != 0x0a ||
                bytes[offset + 6] != 0x1a || bytes[offset + 7] != 0x0a) continue;
            var width = bytes[entry] == 0 ? 256 : bytes[entry];
            var height = bytes[entry + 1] == 0 ? 256 : bytes[entry + 1];
            var score = (long)width * height * 65536 + U16(bytes, entry + 6);
            if (score <= selectedScore) continue;
            selectedScore = score;
            selectedOffset = offset;
            selectedSize = size;
        }
        if (selectedOffset < 0)
            throw new NotSupportedException("GUI.Drawing currently admits PNG-backed ICO frames; this ICO is DIB-only.");
        return DecodePng(bytes.AsSpan(selectedOffset, selectedSize).ToArray());
    }
    private static global::System.Drawing.Bitmap WrapBitmap(Handle handle, int width, int height)
    {
        var bitmap = new global::System.Drawing.Bitmap();
        bitmap.__bitmap = handle;
        bitmap.__width = width;
        bitmap.__height = height;
        bitmap.__pixelFormat = global::System.Drawing.Imaging.PixelFormat.Format32bppPArgb;
        return bitmap;
    }

    internal static global::System.Drawing.Graphics GraphicsFromNativeSurface(nint surface,
                                                                               uint kind)
    {
        if (surface == 0) throw new ArgumentException("Native surface must be nonzero.", nameof(surface));
        Handle bitmapHandle;
        Rect bounds;
        var result = ((delegate* unmanaged[Cdecl]<nuint, uint, Handle*, Rect*, int>)Entry(92))(
            (nuint)surface, kind, &bitmapHandle, &bounds);
        if (result == 7) throw new PlatformNotSupportedException(
            "Native HDC/HWND drawing is available only through the Windows adapter.");
        Check(result);
        Dimensions(bitmapHandle, out var width, out var height);
        var graphics = new global::System.Drawing.Graphics
        {
            __target = WrapBitmap(bitmapHandle, width, height),
            __nativeSurface = surface,
            __nativeSurfaceKind = kind,
        };
        graphics.__EnsureRecorder();
        if (bounds.X != 0 || bounds.Y != 0)
            RecorderTranslate(graphics.__recorder, (float)-bounds.X, (float)-bounds.Y);
        return graphics;
    }
    internal static void PresentNativeSurface(global::System.Drawing.Graphics graphics)
    {
        if (graphics.__target is null || graphics.__nativeSurface == 0) return;
        var result = ((delegate* unmanaged[Cdecl]<nuint, uint, Handle, int>)Entry(93))(
            (nuint)graphics.__nativeSurface, graphics.__nativeSurfaceKind,
            graphics.__target.__BitmapHandle);
        if (result == 7) throw new PlatformNotSupportedException(
            "Native surface presentation is available only through the Windows adapter.");
        Check(result);
    }
    internal static nint GetHdc(global::System.Drawing.Graphics graphics) =>
        AcquireHdc(graphics);
    private static nint AcquireHdc(global::System.Drawing.Graphics graphics)
    {
        if (graphics.__target is null)
            throw new InvalidOperationException("Graphics has no bitmap-backed target.");
        graphics.__EnsureRecorder();
        if (graphics.__hdcLeaseToken != 0)
            throw new InvalidOperationException("An HDC lease is already active.");
        Execute(graphics.__recorder, graphics.__target.__BitmapHandle);
        var priorRecorder = graphics.__recorder;
        Release(ref priorRecorder);
        graphics.__recorder = RecorderCreate();
        nuint device;
        ulong token;
        var result = ((delegate* unmanaged[Cdecl]<Handle, nuint*, ulong*, int>)Entry(94))(
            graphics.__target.__BitmapHandle, &device, &token);
        if (result == 7) throw new PlatformNotSupportedException(
            "Bitmap-backed HDC leases are available only through the Windows adapter.");
        Check(result);
        graphics.__leasedHdc = (nint)device;
        graphics.__hdcLeaseToken = token;
        return graphics.__leasedHdc;
    }
    internal static void ReleaseHdc(global::System.Drawing.Graphics graphics, nint device)
    {
        if (graphics.__target is null || graphics.__hdcLeaseToken == 0 ||
            device == 0 || device != graphics.__leasedHdc)
            throw new ArgumentException("HDC does not belong to this Graphics lease.", nameof(device));
        var result = ((delegate* unmanaged[Cdecl]<Handle, ulong, int>)Entry(95))(
            graphics.__target.__BitmapHandle, graphics.__hdcLeaseToken);
        if (result == 7) throw new PlatformNotSupportedException(
            "Bitmap-backed HDC leases are available only through the Windows adapter.");
        Check(result);
        graphics.__leasedHdc = 0;
        graphics.__hdcLeaseToken = 0;
    }
    internal static nint GetHbitmap(global::System.Drawing.Bitmap bitmap,
                                    global::System.Drawing.Color background)
    {
        nuint native;
        var result = ((delegate* unmanaged[Cdecl]<Handle, NativeColor, nuint*, int>)Entry(90))(
            bitmap.__BitmapHandle, Native(background), &native);
        if (result == 7) throw new PlatformNotSupportedException(
            "Native HBITMAP export is available only through the Windows adapter.");
        Check(result);
        return (nint)native;
    }
    internal static global::System.Drawing.Bitmap FromHbitmap(nint hbitmap)
    {
        if (hbitmap == 0) throw new ArgumentException("HBITMAP must be nonzero.", nameof(hbitmap));
        Handle handle;
        var result = ((delegate* unmanaged[Cdecl]<nuint, Handle*, int>)Entry(91))(
            (nuint)hbitmap, &handle);
        if (result == 7) throw new PlatformNotSupportedException(
            "Native HBITMAP import is available only through the Windows adapter.");
        Check(result);
        Dimensions(handle, out var width, out var height);
        return WrapBitmap(handle, width, height);
    }
}
""";

static string NativeBridgeSource() => """
// <auto-generated/>
#nullable enable
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Linq;
using System.Text;
using System.Threading;

namespace System.Windows.Forms;

internal enum NativeChange { None, Name, Text, Visible, Enabled, Bounds, Tree }
internal enum NativeEvent : uint { Clicked = 2, FormClosing = 3, FormClosed = 4, RangeValueChanged = 14, RangeScroll = 15 }
internal readonly record struct NativePointer(uint Kind, double X, double Y, double WheelDelta, uint Button);
internal readonly record struct NativeKey(uint Kind, uint PhysicalKey, uint Modifiers, bool Repeat);

internal sealed unsafe class NativeControlBridge : IDisposable
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private unsafe delegate uint DispatchThunk(void* context, uint cancelled);
    private static readonly DispatchThunk dispatchThunk = DispatchCallback;
    private static readonly nint dispatchThunkPointer = Marshal.GetFunctionPointerForDelegate(dispatchThunk);
    [StructLayout(LayoutKind.Sequential)] internal struct Handle { internal uint Slot; internal uint Generation; }
    [StructLayout(LayoutKind.Sequential)] private struct StringView { internal byte* Data; internal ulong Size; }
    [StructLayout(LayoutKind.Sequential)] private struct ErrorView { internal uint Code; internal StringView Message; }
    [StructLayout(LayoutKind.Sequential)] private struct Rect { internal double X, Y, Width, Height; }
    [StructLayout(LayoutKind.Sequential)] private struct Api
    {
        internal uint StructSize, AbiVersion;
        internal nint LastError, ControlCreate, Retain, Release, Dispose, ComponentState, StableId;
        internal nint SetVisible, GetVisible, SetBounds, GetBounds, AddChild, RemoveChild, Subscribe, Disconnect;
        internal nint ControlCreateKind, SetName, GetName, SetText, GetText, SetEnabled, GetEnabled;
        internal nint RunWindow, LastHostTrace;
        internal nint SubscribeV2, BeginInvoke, RequestClose, CallbackFaultCount;
        internal nint SetControlPng, SetChildIndex, SetControlColors, SubscribePointer;
        internal nint SetCheckState, GetCheckState;
        internal nint SubscribeKey, SubscribeText;
        internal nint SetRange, GetRange, SetRangeValue, GetRangeValue;
        internal nint SetPointerCapture, GetPointerCapture;
        internal nint ShowPathDialog, LastDialogPath, ShowTooltip, HideTooltip;
    }

    [DllImport("gui_forms_abi0", EntryPoint = "gf_get_api_v0", CallingConvention = CallingConvention.Cdecl)]
    private static extern int GetApi(uint requestedVersion, ref Api api);
    private static readonly Api api = LoadApi();
    private static readonly bool traceControls = Environment.GetEnvironmentVariable("GUI_FORMS_TRACE_CONTROLS") == "1";
    private static readonly bool traceDelegates = Environment.GetEnvironmentVariable("GUI_FORMS_TRACE_DELEGATES") == "1";
    private static long nextId;
    private static long nextAsyncId;
    [ThreadStatic] private static int nativeCallbackDepth;
    private static readonly global::System.Collections.Concurrent.ConcurrentDictionary<int,
        global::System.Collections.Concurrent.ConcurrentDictionary<long, NativeAsyncResult>> pendingByThread = new();
    private SafeControlHandle handle;
    private readonly string stableId;
    private readonly string managedTypeName;
    private readonly bool supportsRaster;
    private GCHandle callbackRoot;
    private readonly global::System.Collections.Generic.List<Handle> subscriptions = new();
    private readonly int ownerThreadId;
    private NativeChange pendingChange;
    private readonly object windowHandleGate = new();
    private readonly object stateGate = new();
    private nint windowHandle;
    private string cachedName = string.Empty;
    private string cachedText = string.Empty;
    private volatile bool cachedVisible = true;
    private volatile bool cachedEnabled = true;
    private global::System.Drawing.Rectangle cachedBounds;
    internal bool IsDisposed { get; private set; }
    // A retained ABI handle is an identity token, not an HWND. Returning it to
    // Win32 callers causes GetDC/SendMessage to operate on an invalid window.
    internal bool HasWindowHandle => windowHandle != 0;
    internal nint WindowHandle
    {
        get
        {
            if (IsDisposed || !global::System.OperatingSystem.IsWindows()) return 0;
            lock (windowHandleGate)
            {
                if (windowHandle == 0)
                    windowHandle = CreateWindowExW(0, "STATIC", string.Empty, 0x80000000u,
                        0, 0, 1, 1, 0, 0, 0, 0);
                return windowHandle;
            }
        }
    }
    internal event Action<NativeChange>? Changed;
    internal event Func<NativeEvent, bool>? NativeEventRaised;
    internal event Action<NativePointer>? PointerRaised;
    internal event Action<NativeKey>? KeyRaised;
    internal event Action<string, bool, int, int>? TextRaised;

    private NativeControlBridge(SafeControlHandle handle, uint kind, string stableId, string managedTypeName)
    {
        this.handle = handle;
        this.stableId = stableId;
        this.managedTypeName = managedTypeName;
        supportsRaster = kind is 20u or 0x7fffffffu;
        ownerThreadId = Environment.CurrentManagedThreadId;
        callbackRoot = GCHandle.Alloc(this, GCHandleType.Weak);
        Handle token;
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, delegate* unmanaged[Cdecl]<Handle, uint, void*, void>, void*, Handle*, int>)api.Subscribe)(
            handle.Value, 1, &StateCallback, (void*)GCHandle.ToIntPtr(callbackRoot), &token));
        subscriptions.Add(token);
        if (kind is 4u or 5u or 11u or 14u) SubscribeTyped(NativeEvent.Clicked);
        if (kind == 1u) { SubscribeTyped(NativeEvent.FormClosing); SubscribeTyped(NativeEvent.FormClosed); }
        if (kind == 10u) { SubscribeTyped(NativeEvent.RangeScroll); SubscribeTyped(NativeEvent.RangeValueChanged); }
        SubscribePointer();
        if (kind is 6u or 8u or 9u or 16u or 18u) { SubscribeKey(); SubscribeText(); }
        else if (kind == 0x7fffffffu) SubscribeKey();
    }

    internal static NativeControlBridge Create(Type managedType)
    {
        var stableId = $"forms.{managedType.Name}.{Interlocked.Increment(ref nextId)}";
        var paintMethod = managedType.GetMethod("OnPaint", global::System.Reflection.BindingFlags.Instance | global::System.Reflection.BindingFlags.NonPublic);
        // DockPanelSuite's empty auto-hide strip reports the entire dock client
        // even when it owns no tabs. Painting that compatibility overlay would
        // obscure every retained pane beneath it.
        var retainedField = typeof(ComboBox).IsAssignableFrom(managedType) ||
            typeof(NumericUpDown).IsAssignableFrom(managedType) ||
            typeof(TextBoxBase).IsAssignableFrom(managedType);
        var toolStripSurface = typeof(ToolStrip).IsAssignableFrom(managedType);
        var buttonSurface = typeof(Button).IsAssignableFrom(managedType);
        var transparentPaintSurface = typeof(Label).IsAssignableFrom(managedType) &&
            !typeof(LinkLabel).IsAssignableFrom(managedType);
        var customPaint = global::System.OperatingSystem.IsWindows() &&
            (toolStripSurface || buttonSurface || (!retainedField && !typeof(Form).IsAssignableFrom(managedType) &&
            !managedType.Name.Contains("AutoHideStrip", StringComparison.Ordinal) &&
            paintMethod?.DeclaringType?.Assembly != typeof(Control).Assembly));
        var kind = managedType.Name.Contains("AutoHideStrip", StringComparison.Ordinal)
            ? 19u : customPaint && transparentPaintSurface ? 20u : customPaint ? 0x7fffffffu : 0u;
        for (var current = managedType; current is not null && kind == 0u; current = current.BaseType)
        {
            kind = current.Name switch
            {
                "Form" => 1u, "UserControl" => 2u, "Panel" => 3u, "Button" => 4u,
                "CheckBox" => 5u, "ComboBox" => 6u, "Label" => 7u, "ListBox" => 8u,
                "TextBox" or "TextBoxBase" => 9u, "TrackBar" => 10u,
                "RadioButton" => 11u, "GroupBox" => 12u, "ProgressBar" => 13u,
                "LinkLabel" => 14u, "PictureBox" => 15u, "DataGridView" => 16u,
                "ToolStrip" => 17u, "NumericUpDown" => 18u, _ => 0u,
            };
        }
        var bytes = Encoding.UTF8.GetBytes(stableId);
        Handle value;
        fixed (byte* data = bytes)
            Check(((delegate* unmanaged[Cdecl]<uint, StringView, Handle*, int>)api.ControlCreateKind)(kind, new StringView { Data = data, Size = (ulong)bytes.Length }, &value));
        if (traceControls) Console.Error.WriteLine($"facade-control=create|id={stableId}|type={managedType.FullName}|kind={kind}");
        return new NativeControlBridge(new SafeControlHandle(value), kind, stableId, managedType.FullName ?? managedType.Name);
    }

    private void EnsureAlive() { if (IsDisposed) throw new global::System.InvalidOperationException("GUI.Forms control is disposed."); }
    internal string Name { get { EnsureAlive(); lock (stateGate) return cachedName; } set => SetString(api.SetName, value, NativeChange.Name); }
    internal string Text { get { EnsureAlive(); lock (stateGate) return cachedText; } set => SetString(api.SetText, value, NativeChange.Text); }
    internal bool Visible { get { EnsureAlive(); return cachedVisible; } set => SetBool(api.SetVisible, value, NativeChange.Visible); }
    internal bool Enabled { get { EnsureAlive(); return cachedEnabled; } set => SetBool(api.SetEnabled, value, NativeChange.Enabled); }
    internal global::System.Drawing.Rectangle Bounds
    {
        get { EnsureAlive(); lock (stateGate) return cachedBounds; }
        set { pendingChange = NativeChange.Bounds; lock (stateGate) cachedBounds = value; Check(((delegate* unmanaged[Cdecl]<Handle, Rect, int>)api.SetBounds)(handle.Value, new Rect { X = value.X, Y = value.Y, Width = value.Width, Height = value.Height })); }
    }

    internal void AddChild(NativeControlBridge child) { if (traceControls) Console.Error.WriteLine($"facade-control=add|parent={stableId}|parent-type={managedTypeName}|child={child.stableId}|child-type={child.managedTypeName}"); pendingChange = NativeChange.Tree; Check(((delegate* unmanaged[Cdecl]<Handle, Handle, int>)api.AddChild)(handle.Value, child.handle.Value)); }
    internal void RemoveChild(NativeControlBridge child) { if (traceControls) Console.Error.WriteLine($"facade-control=remove|parent={stableId}|child={child.stableId}"); pendingChange = NativeChange.Tree; Check(((delegate* unmanaged[Cdecl]<Handle, Handle, int>)api.RemoveChild)(handle.Value, child.handle.Value)); }
    internal void SetChildIndex(NativeControlBridge child, int index) { if (index < 0) throw new global::System.ArgumentOutOfRangeException(nameof(index)); pendingChange = NativeChange.Tree; Check(((delegate* unmanaged[Cdecl]<Handle, Handle, ulong, int>)api.SetChildIndex)(handle.Value, child.handle.Value, (ulong)index)); }
    internal void SetColors(global::System.Drawing.Color foreground, global::System.Drawing.Color background) { Check(((delegate* unmanaged[Cdecl]<Handle, uint, uint, int>)api.SetControlColors)(handle.Value, unchecked((uint)foreground.ToArgb()), unchecked((uint)background.ToArgb()))); }
    internal uint CheckState
    {
        get { EnsureAlive(); uint value; Check(((delegate* unmanaged[Cdecl]<Handle, uint*, int>)api.GetCheckState)(handle.Value, &value)); return value; }
        set { EnsureAlive(); Check(((delegate* unmanaged[Cdecl]<Handle, uint, int>)api.SetCheckState)(handle.Value, value)); }
    }
    internal void SetRange(double minimum, double maximum) { EnsureAlive(); Check(((delegate* unmanaged[Cdecl]<Handle, double, double, int>)api.SetRange)(handle.Value, minimum, maximum)); }
    internal void GetRange(out double minimum, out double maximum) { EnsureAlive(); double minimumValue; double maximumValue; Check(((delegate* unmanaged[Cdecl]<Handle, double*, double*, int>)api.GetRange)(handle.Value, &minimumValue, &maximumValue)); minimum = minimumValue; maximum = maximumValue; }
    internal double RangeValue
    {
        get { EnsureAlive(); double value; Check(((delegate* unmanaged[Cdecl]<Handle, double*, int>)api.GetRangeValue)(handle.Value, &value)); return value; }
        set { EnsureAlive(); Check(((delegate* unmanaged[Cdecl]<Handle, double, int>)api.SetRangeValue)(handle.Value, value)); }
    }
    internal bool Capture
    {
        get { EnsureAlive(); uint value; Check(((delegate* unmanaged[Cdecl]<Handle, uint*, int>)api.GetPointerCapture)(handle.Value, &value)); return value != 0; }
        set { EnsureAlive(); Check(((delegate* unmanaged[Cdecl]<Handle, uint, int>)api.SetPointerCapture)(handle.Value, value ? 1u : 0u)); }
    }
    internal bool ShowPathDialog(uint kind, string title, string initialDirectory,
                                 string suggestedName, string defaultExtension,
                                 string filter, uint flags, out string selectedPath)
    {
        EnsureAlive();
        var titleBytes = Encoding.UTF8.GetBytes(title ?? string.Empty);
        var directoryBytes = Encoding.UTF8.GetBytes(initialDirectory ?? string.Empty);
        var suggestedBytes = Encoding.UTF8.GetBytes(suggestedName ?? string.Empty);
        var extensionBytes = Encoding.UTF8.GetBytes(defaultExtension ?? string.Empty);
        var filterBytes = Encoding.UTF8.GetBytes(filter ?? string.Empty);
        uint accepted;
        fixed (byte* titleData = titleBytes)
        fixed (byte* directoryData = directoryBytes)
        fixed (byte* suggestedData = suggestedBytes)
        fixed (byte* extensionData = extensionBytes)
        fixed (byte* filterData = filterBytes)
            Check(((delegate* unmanaged[Cdecl]<Handle, uint, StringView, StringView, StringView, StringView, StringView, uint, uint*, int>)api.ShowPathDialog)(
                handle.Value, kind,
                new StringView { Data = titleData, Size = (ulong)titleBytes.Length },
                new StringView { Data = directoryData, Size = (ulong)directoryBytes.Length },
                new StringView { Data = suggestedData, Size = (ulong)suggestedBytes.Length },
                new StringView { Data = extensionData, Size = (ulong)extensionBytes.Length },
                new StringView { Data = filterData, Size = (ulong)filterBytes.Length },
                flags, &accepted));
        selectedPath = accepted == 0 ? string.Empty : GetString(api.LastDialogPath);
        return accepted != 0;
    }
    internal void ShowToolTip(string text, int x, int y, int duration)
    {
        EnsureAlive();
        var bytes = Encoding.UTF8.GetBytes(text ?? string.Empty);
        fixed (byte* data = bytes)
            Check(((delegate* unmanaged[Cdecl]<Handle, StringView, double, double, uint, int>)api.ShowTooltip)(
                handle.Value, new StringView { Data = data, Size = (ulong)bytes.Length },
                x, y, checked((uint)duration)));
    }
    internal void HideToolTip() { EnsureAlive(); Check(((delegate* unmanaged[Cdecl]<Handle, int>)api.HideTooltip)(handle.Value)); }
    internal bool InvokeRequired => Environment.CurrentManagedThreadId != ownerThreadId;
    internal bool SupportsRaster { get { return supportsRaster && api.SetControlPng != 0; } }
    internal void SetRaster(byte[] encodedPng) { if (!SupportsRaster) return; fixed (byte* data = encodedPng) Check(((delegate* unmanaged[Cdecl]<Handle, byte*, ulong, int>)api.SetControlPng)(handle.Value, data, (ulong)encodedPng.Length)); }
    internal string RunWindow(bool autoClose, bool forceHeadless, bool autoActivate, bool popup)
    {
        var flags = (autoClose ? 1u : 0u) | (forceHeadless ? 2u : 0u) | (autoActivate ? 4u : 0u) | (popup ? 8u : 0u);
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, int>)api.RunWindow)(handle.Value, flags));
        return GetString(api.LastHostTrace);
    }
    internal static void DoEvents(int ownerThreadId)
    {
        // GUI.Forms defines BeginInvoke as a strictly posted boundary: a native
        // input/lifecycle callback must unwind before work it posts can run.
        // This prevents DoEvents from turning pointer or load callbacks into an
        // accidental recursive dispatcher.
        if (nativeCallbackDepth != 0) return;
        if (!pendingByThread.TryGetValue(ownerThreadId, out var pending)) return;
        foreach (var item in pending.OrderBy(item => item.Key).ToArray())
        {
            try { item.Value.Execute(); }
            catch (Exception error) { Application.__ReportCallbackException(error); }
        }
        if (pending.IsEmpty) pendingByThread.TryRemove(ownerThreadId, out _);
    }
    private static void Track(NativeAsyncResult pending) =>
        pendingByThread.GetOrAdd(pending.OwnerThreadId, _ => new()).TryAdd(pending.Id, pending);
    private static void Untrack(NativeAsyncResult pending)
    {
        if (!pendingByThread.TryGetValue(pending.OwnerThreadId, out var values)) return;
        values.TryRemove(pending.Id, out _);
        if (values.IsEmpty) pendingByThread.TryRemove(pending.OwnerThreadId, out _);
    }
    internal global::System.IAsyncResult BeginInvoke(global::System.Delegate method)
    {
        if (method is null) throw new global::System.ArgumentNullException(nameof(method));
        var declaringType = method.Method.DeclaringType?.FullName ?? string.Empty;
        if (traceDelegates && (declaringType.StartsWith("retired compatibility specimen.FrontEnds.SpyServer", StringComparison.Ordinal) ||
            declaringType == "retired compatibility specimen.MainForm" && method.Method.Name == "HandleFrontendControllerSampleRateChange"))
        {
            var strings = method.Target?.GetType().GetFields(global::System.Reflection.BindingFlags.Instance | global::System.Reflection.BindingFlags.Public | global::System.Reflection.BindingFlags.NonPublic)
                .Where(field => field.FieldType == typeof(string)).Select(field => field.GetValue(method.Target) as string).Where(value => value is not null) ?? [];
            Console.Error.WriteLine($"facade-delegate=begin-invoke|id={stableId}|method={declaringType}.{method.Method.Name}|thread={Environment.CurrentManagedThreadId}|strings={string.Join(';', strings)}");
        }
        var pending = new NativeAsyncResult(method, ownerThreadId);
        Track(pending);
        var root = GCHandle.Alloc(pending);
        var result = ((delegate* unmanaged[Cdecl]<Handle, delegate* unmanaged[Cdecl]<void*, uint, uint>, void*, int>)api.BeginInvoke)(
            handle.Value, (delegate* unmanaged[Cdecl]<void*, uint, uint>)(void*)dispatchThunkPointer, (void*)GCHandle.ToIntPtr(root));
        if (result != 0) { root.Free(); pending.Cancel(); Check(result); }
        return pending;
    }
    internal object? Invoke(global::System.Delegate method)
    {
        if (method is null) throw new global::System.ArgumentNullException(nameof(method));
        if (!InvokeRequired) return method.DynamicInvoke();
        var pending = (NativeAsyncResult)BeginInvoke(method);
        pending.AsyncWaitHandle.WaitOne();
        return pending.GetResult();
    }
    internal void RequestClose() { Check(((delegate* unmanaged[Cdecl]<Handle, int>)api.RequestClose)(handle.Value)); }
    public void Dispose() { if (IsDisposed) return; ReleaseSubscriptions(true); ReleaseWindowHandle(); handle.DisposeNative(); IsDisposed = true; GC.SuppressFinalize(this); }
    ~NativeControlBridge() { ReleaseSubscriptions(false); }

    private void ReleaseWindowHandle()
    {
        nint value;
        lock (windowHandleGate) { value = windowHandle; windowHandle = 0; }
        if (value == 0 || !global::System.OperatingSystem.IsWindows()) return;
        if (global::System.Environment.CurrentManagedThreadId == ownerThreadId) _ = DestroyWindow(value);
        else _ = PostMessageW(value, 0x0010u, 0, 0);
    }

    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern nint CreateWindowExW(uint exStyle, string className, string windowName,
        uint style, int x, int y, int width, int height, nint parent, nint menu,
        nint instance, nint parameter);
    [DllImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool DestroyWindow(nint window);
    [DllImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool PostMessageW(nint window, uint message, nint wParam, nint lParam);

    private void SetString(nint operation, string value, NativeChange change)
    {
        if (traceControls) Console.Error.WriteLine($"facade-control=set|id={stableId}|type={managedTypeName}|change={change}|value={value.Replace('\r', ' ').Replace('\n', ' ')}");
        var bytes = Encoding.UTF8.GetBytes(value);
        pendingChange = change;
        lock (stateGate) { if (change == NativeChange.Name) cachedName = value; else cachedText = value; }
        fixed (byte* data = bytes)
            Check(((delegate* unmanaged[Cdecl]<Handle, StringView, int>)operation)(handle.Value, new StringView { Data = data, Size = (ulong)bytes.Length }));
    }

    private string GetString(nint operation)
    {
        ulong required = 0;
        var result = ((delegate* unmanaged[Cdecl]<Handle, byte*, ulong, ulong*, int>)operation)(handle.Value, null, 0, &required);
        if (result != 6 && result != 0) Check(result);
        if (required == 0) return string.Empty;
        var bytes = new byte[checked((int)required)];
        fixed (byte* data = bytes) Check(((delegate* unmanaged[Cdecl]<Handle, byte*, ulong, ulong*, int>)operation)(handle.Value, data, required, &required));
        return Encoding.UTF8.GetString(bytes);
    }

    private bool GetBool(nint operation) { uint value; Check(((delegate* unmanaged[Cdecl]<Handle, uint*, int>)operation)(handle.Value, &value)); return value != 0; }
    private void SetBool(nint operation, bool value, NativeChange change) { if (traceControls) Console.Error.WriteLine($"facade-control=set|id={stableId}|type={managedTypeName}|change={change}|value={value}"); pendingChange = change; if (change == NativeChange.Visible) cachedVisible = value; else cachedEnabled = value; Check(((delegate* unmanaged[Cdecl]<Handle, uint, int>)operation)(handle.Value, value ? 1u : 0u)); }

    private void SubscribeTyped(NativeEvent kind)
    {
        Handle token;
        Check(((delegate* unmanaged[Cdecl]<Handle, uint, delegate* unmanaged[Cdecl]<Handle, uint, void*, uint>, void*, Handle*, int>)api.SubscribeV2)(
            handle.Value, (uint)kind, &EventCallback, (void*)GCHandle.ToIntPtr(callbackRoot), &token));
        subscriptions.Add(token);
    }

    private void SubscribePointer()
    {
        Handle token;
        Check(((delegate* unmanaged[Cdecl]<Handle, delegate* unmanaged[Cdecl]<Handle, uint, double, double, double, uint, void*, uint>, void*, Handle*, int>)api.SubscribePointer)(
            handle.Value, &PointerCallback, (void*)GCHandle.ToIntPtr(callbackRoot), &token));
        subscriptions.Add(token);
    }

    private void SubscribeKey()
    {
        Handle token;
        Check(((delegate* unmanaged[Cdecl]<Handle, delegate* unmanaged[Cdecl]<Handle, uint, uint, uint, uint, void*, uint>, void*, Handle*, int>)api.SubscribeKey)(
            handle.Value, &KeyCallback, (void*)GCHandle.ToIntPtr(callbackRoot), &token));
        subscriptions.Add(token);
    }

    private void SubscribeText()
    {
        Handle token;
        Check(((delegate* unmanaged[Cdecl]<Handle, delegate* unmanaged[Cdecl]<Handle, StringView, uint, int, int, void*, uint>, void*, Handle*, int>)api.SubscribeText)(
            handle.Value, &TextCallback, (void*)GCHandle.ToIntPtr(callbackRoot), &token));
        subscriptions.Add(token);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void StateCallback(Handle sender, uint kind, void* context)
    {
        if (kind != 1 || context == null) return;
        if (GCHandle.FromIntPtr((nint)context).Target is not NativeControlBridge bridge) return;
        ++nativeCallbackDepth;
        try { bridge.Changed?.Invoke(bridge.pendingChange); }
        catch (Exception error) { Application.__ReportCallbackException(error); }
        finally { --nativeCallbackDepth; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static uint EventCallback(Handle sender, uint kind, void* context)
    {
        if (context == null || GCHandle.FromIntPtr((nint)context).Target is not NativeControlBridge bridge) return 0;
        ++nativeCallbackDepth;
        try { return bridge.NativeEventRaised?.Invoke((NativeEvent)kind) == true ? 1u : 0u; }
        catch (Exception error) { Application.__ReportCallbackException(error); return 2u; }
        finally { --nativeCallbackDepth; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static uint PointerCallback(Handle sender, uint kind, double x, double y,
                                        double wheelDelta, uint button, void* context)
    {
        if (context == null || GCHandle.FromIntPtr((nint)context).Target is not NativeControlBridge bridge) return 0;
        ++nativeCallbackDepth;
        try { bridge.PointerRaised?.Invoke(new NativePointer(kind, x, y, wheelDelta, button)); return 0; }
        catch (Exception error) { Application.__ReportCallbackException(error); return 2u; }
        finally { --nativeCallbackDepth; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static uint KeyCallback(Handle sender, uint kind, uint physicalKey,
                                    uint modifiers, uint repeat, void* context)
    {
        if (context == null || GCHandle.FromIntPtr((nint)context).Target is not NativeControlBridge bridge) return 0;
        ++nativeCallbackDepth;
        try { bridge.KeyRaised?.Invoke(new NativeKey(kind, physicalKey, modifiers, repeat != 0)); return 0; }
        catch (Exception error) { Application.__ReportCallbackException(error); return 2u; }
        finally { --nativeCallbackDepth; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static uint TextCallback(Handle sender, StringView text, uint composing,
                                     int replacementStart, int replacementLength,
                                     void* context)
    {
        if (context == null || GCHandle.FromIntPtr((nint)context).Target is not NativeControlBridge bridge) return 0;
        ++nativeCallbackDepth;
        try { var value = text.Data == null || text.Size == 0 ? string.Empty : Encoding.UTF8.GetString(text.Data, checked((int)text.Size)); bridge.TextRaised?.Invoke(value, composing != 0, replacementStart, replacementLength); return 0; }
        catch (Exception error) { Application.__ReportCallbackException(error); return 2u; }
        finally { --nativeCallbackDepth; }
    }

    private static uint DispatchCallback(void* context, uint cancelled)
    {
        if (context == null) return 0;
        var root = GCHandle.FromIntPtr((nint)context);
        ++nativeCallbackDepth;
        try
        {
            if (root.Target is NativeAsyncResult pending)
            {
                if (cancelled != 0) pending.Cancel();
                else pending.Execute();
            }
            return 0;
        }
        catch (Exception error)
        {
            Application.__ReportCallbackException(error);
            return 2u;
        }
        finally { --nativeCallbackDepth; root.Free(); }
    }

    private void ReleaseSubscriptions(bool check)
    {
        var disconnected = true;
        foreach (var subscription in subscriptions)
        {
            var result = ((delegate* unmanaged[Cdecl]<Handle, int>)api.Disconnect)(subscription);
            if (check) Check(result);
            if (result != 0) disconnected = false;
        }
        if (disconnected)
        {
            subscriptions.Clear();
            if (callbackRoot.IsAllocated) callbackRoot.Free();
        }
    }

    private static Api LoadApi()
    {
        var value = new Api { StructSize = (uint)sizeof(Api) };
        Check(GetApi(10, ref value));
        if (value.AbiVersion != 10 || value.BeginInvoke == 0 || value.RequestClose == 0 || value.SetControlPng == 0 || value.SetChildIndex == 0 || value.SetControlColors == 0 || value.SubscribePointer == 0 || value.SetCheckState == 0 || value.GetCheckState == 0 || value.SubscribeKey == 0 || value.SubscribeText == 0 || value.SetRange == 0 || value.GetRange == 0 || value.SetRangeValue == 0 || value.GetRangeValue == 0 || value.SetPointerCapture == 0 || value.GetPointerCapture == 0 || value.ShowPathDialog == 0 || value.LastDialogPath == 0 || value.ShowTooltip == 0 || value.HideTooltip == 0) throw new InvalidOperationException("GUI.Forms ABI 0.10 table is incomplete.");
        return value;
    }
    private static void Check(int result)
    {
        if (result == 0) return;
        var detail = string.Empty;
        if (api.LastError != 0)
        {
            ErrorView error;
            var errorResult = ((delegate* unmanaged[Cdecl]<ErrorView*, int>)api.LastError)(&error);
            if (errorResult == 0 && error.Message.Data != null && error.Message.Size != 0)
                detail = Encoding.UTF8.GetString(error.Message.Data, checked((int)error.Message.Size));
        }
        if (Environment.GetEnvironmentVariable("GUI_FORMS_TRACE_ABI_ERRORS") == "1")
            Console.Error.WriteLine($"facade-abi-error=result:{result}|thread:{Environment.CurrentManagedThreadId}|detail:{detail}\n{Environment.StackTrace}");
        throw new InvalidOperationException($"GUI.Forms ABI operation failed with result {result}{(detail.Length == 0 ? "." : $": {detail}")}");
    }

    private sealed class SafeControlHandle
    {
        internal Handle Value { get; private set; }
        private bool released;
        internal SafeControlHandle(Handle value) { Value = value; }
        internal void DisposeNative() { if (released) return; Check(((delegate* unmanaged[Cdecl]<Handle, int>)api.Dispose)(Value)); released = true; GC.SuppressFinalize(this); }
        ~SafeControlHandle() { if (!released) _ = ((delegate* unmanaged[Cdecl]<Handle, int>)api.Release)(Value); }
    }

    private sealed class NativeAsyncResult : global::System.IAsyncResult
    {
        private readonly global::System.Delegate method;
        private readonly global::System.Threading.ManualResetEvent completed = new(false);
        private readonly string traceName;
        private readonly bool traceThis;
        private int executionState;
        private object? result;
        private Exception? error;
        internal NativeAsyncResult(global::System.Delegate method, int ownerThreadId)
        {
            this.method = method;
            OwnerThreadId = ownerThreadId;
            Id = global::System.Threading.Interlocked.Increment(ref nextAsyncId);
            var declaringType = method.Method.DeclaringType?.FullName ?? string.Empty;
            traceName = $"{declaringType}.{method.Method.Name}";
            traceThis = traceDelegates && (declaringType.StartsWith("retired compatibility specimen.FrontEnds.SpyServer", StringComparison.Ordinal) ||
                declaringType == "retired compatibility specimen.MainForm" && method.Method.Name == "HandleFrontendControllerSampleRateChange");
        }
        internal long Id { get; }
        internal int OwnerThreadId { get; }
        public object? AsyncState => null;
        public global::System.Threading.WaitHandle AsyncWaitHandle => completed;
        public bool CompletedSynchronously => false;
        public bool IsCompleted { get; private set; }
        internal void Execute()
        {
            if (global::System.Threading.Interlocked.CompareExchange(ref executionState, 1, 0) != 0) return;
            Untrack(this);
            if (traceThis) Console.Error.WriteLine($"facade-delegate=execute-begin|method={traceName}|thread={Environment.CurrentManagedThreadId}");
            try { result = method.DynamicInvoke(); }
            catch (Exception caught) { error = caught; throw; }
            finally
            {
                IsCompleted = true;
                completed.Set();
                if (traceThis) Console.Error.WriteLine($"facade-delegate=execute-end|method={traceName}|thread={Environment.CurrentManagedThreadId}");
            }
        }
        internal void Cancel() { if (global::System.Threading.Interlocked.CompareExchange(ref executionState, 1, 0) != 0) return; Untrack(this); error = new global::System.OperationCanceledException("GUI.Forms host closed before the queued invocation ran."); IsCompleted = true; completed.Set(); }
        internal object? GetResult() { if (error is not null) throw error; return result; }
    }
}
""";

internal sealed record ApiRow(string Id, string Assembly, string Type, string? Member, string? Signature, string MemberKind)
{
    public static ApiRow Read(JsonElement row) => new(row.GetProperty("id").GetString()!, row.GetProperty("targetAssembly").GetString()!,
        row.GetProperty("type").GetString()!, row.TryGetProperty("member", out var member) ? member.GetString() : null,
        row.TryGetProperty("signature", out var signature) ? signature.GetString() : null, row.GetProperty("memberKind").GetString()!);
}

internal sealed record Resolution(string Id, string RequestedAssembly, string Type, string? Member, string? Signature, string Status, string? ActualAssembly);
