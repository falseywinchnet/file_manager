using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

public static class StartupHook
{
    private static string? tracePath;
    private static string? formsPath;
    private static string? primitivesPath;

    public static void Initialize()
    {
        tracePath = Environment.GetEnvironmentVariable("GUI_FORMS_FACADE_TRACE");
        formsPath = Environment.GetEnvironmentVariable("GUI_FORMS_FACADE_FORMS");
        primitivesPath = Environment.GetEnvironmentVariable("GUI_FORMS_FACADE_PRIMITIVES");
        Trace("hook=initialize");
        AssemblyLoadContext.Default.Resolving += Resolve;
        AppDomain.CurrentDomain.AssemblyLoad += (_, args) =>
        {
            var name = args.LoadedAssembly.GetName().Name ?? string.Empty;
            if (name.StartsWith("retired compatibility specimen", StringComparison.Ordinal) ||
                name.StartsWith("System.Windows.Forms", StringComparison.Ordinal))
            {
                Trace($"load={name}|token={Token(args.LoadedAssembly.GetName())}");
            }
        };
        try
        {
            LoadFacade(primitivesPath, "System.Windows.Forms.Primitives");
            LoadFacade(formsPath, "System.Windows.Forms");
            Trace("hook=ready");
        }
        catch (Exception error)
        {
            Trace($"hook=fault|type={error.GetType().FullName}|message={Sanitize(error.Message)}");
            throw;
        }
    }

    private static Assembly? Resolve(AssemblyLoadContext context, AssemblyName requested)
    {
        var path = requested.Name switch
        {
            "System.Windows.Forms" => formsPath,
            "System.Windows.Forms.Primitives" => primitivesPath,
            _ => null,
        };
        if (string.IsNullOrEmpty(path)) return null;
        Trace($"resolve={requested.Name}|token={Token(requested)}");
        return LoadFacade(path, requested.Name!);
    }

    private static Assembly LoadFacade(string? path, string expectedName)
    {
        if (string.IsNullOrWhiteSpace(path))
            throw new InvalidOperationException($"Missing facade path for {expectedName}.");
        var fullPath = Path.GetFullPath(path);
        var loaded = Array.Find(AppDomain.CurrentDomain.GetAssemblies(), assembly =>
            string.Equals(assembly.GetName().Name, expectedName, StringComparison.Ordinal));
        if (loaded is not null) return loaded;
        var assembly = AssemblyLoadContext.Default.LoadFromAssemblyPath(fullPath);
        if (expectedName == "System.Windows.Forms")
            NativeLibrary.SetDllImportResolver(assembly, ResolveNative);
        Trace($"preload={assembly.GetName().Name}|token={Token(assembly.GetName())}");
        return assembly;
    }

    private static nint ResolveNative(string libraryName, Assembly assembly,
                                      DllImportSearchPath? searchPath)
    {
        if (libraryName != "gui_forms_abi0") return 0;
        var path = Environment.GetEnvironmentVariable("GUI_FORMS_FACADE_NATIVE");
        if (string.IsNullOrWhiteSpace(path)) return 0;
        Trace("native=gui_forms_abi0");
        return NativeLibrary.Load(Path.GetFullPath(path));
    }

    private static string Token(AssemblyName name)
    {
        var token = name.GetPublicKeyToken();
        return token is null || token.Length == 0 ? "null" : Convert.ToHexString(token).ToLowerInvariant();
    }

    private static void Trace(string line)
    {
        if (string.IsNullOrWhiteSpace(tracePath)) return;
        try { File.AppendAllText(tracePath, line + Environment.NewLine); }
        catch { }
    }

    private static string Sanitize(string message) => message.Replace('\r', ' ').Replace('\n', ' ');
}
