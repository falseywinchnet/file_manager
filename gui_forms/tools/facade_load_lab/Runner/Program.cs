using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace GuiForms.CompatCapture;

internal static class Program
{
    private static int firstChanceCount;
    private const string ExpectedSpecimenSha256 =
        "7e184a595dc88ca1a77ba6841cd6ed9b2f514b9b64728611165e3e4ef24e5c8a";

    internal static readonly string[] DefaultRuntimeSupport =
    [
        "System.Formats.Nrbf",
        "Microsoft.Win32.SystemEvents",
        "System.Private.Windows.Core",
        "System.Resources.Extensions",
    ];

    internal static Assembly? DrawingFacade { get; private set; }

    public static int Main(string[] args)
    {
        if (args.Length != 9)
        {
            Console.Error.WriteLine("usage: facade-load-runner <bundle> <extract-dir> <forms> <primitives> <drawing> <core-runtime-dir> <desktop-runtime-dir> <native-dir> <specimen-dir>");
            return 2;
        }
        try
        {
            if (Environment.GetEnvironmentVariable("GUI_FORMS_TRACE_FIRST_CHANCE") == "1")
                AppDomain.CurrentDomain.FirstChanceException += (_, eventArgs) =>
                {
                    var stack = eventArgs.Exception.StackTrace ?? string.Empty;
                    var source = eventArgs.Exception.TargetSite?.DeclaringType?.Assembly.GetName().Name ?? string.Empty;
                    var traceAll = Environment.GetEnvironmentVariable("GUI_FORMS_TRACE_FIRST_CHANCE_ALL") == "1";
                    if (!traceAll && !stack.Contains("retired compatibility specimen.", StringComparison.Ordinal) &&
                        !stack.Contains("System.Drawing", StringComparison.Ordinal) &&
                        !source.Contains("retired compatibility specimen", StringComparison.Ordinal) &&
                        !source.Contains("System.Drawing", StringComparison.Ordinal)) return;
                    if (Interlocked.Increment(ref firstChanceCount) > (traceAll ? 2000 : 200)) return;
                    Console.Error.WriteLine($"first-chance={eventArgs.Exception.GetType().FullName}|source={source}|{Sanitize(eventArgs.Exception.Message)}|{Sanitize(stack)}");
                };
            return Run(args);
        }
        catch (Exception error)
        {
            ReportException(error);
            return 1;
        }
    }

    private static int Run(string[] args)
    {
        var bundlePath = Path.GetFullPath(args[0]);
        var extractDirectory = Path.GetFullPath(args[1]);
        var image = File.ReadAllBytes(bundlePath);
        var hash = BundleReader.Sha256(image);
        if (!hash.Equals(ExpectedSpecimenSha256, StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException($"Specimen hash mismatch: {hash}.");
        var bundle = BundleReader.TryRead(image) ??
            throw new BadImageFormatException("The specimen is not a supported .NET bundle.");
        Directory.CreateDirectory(extractDirectory);
        foreach (var bundleEntry in bundle.Entries)
        {
            var path = Path.GetFullPath(Path.Combine(extractDirectory,
                bundleEntry.RelativePath.Replace('/', Path.DirectorySeparatorChar)));
            if (!path.StartsWith(extractDirectory + Path.DirectorySeparatorChar,
                                 StringComparison.OrdinalIgnoreCase))
                throw new InvalidDataException("Bundle entry escaped the extraction root.");
            Directory.CreateDirectory(Path.GetDirectoryName(path)!);
            File.WriteAllBytes(path, BundleReader.ReadEntry(image, bundleEntry));
        }
        Console.WriteLine($"bundle=version:{bundle.MajorVersion}.{bundle.MinorVersion}|entries:{bundle.Entries.Count}|hash:matched");

        // ResourceManager resolves reader and image types embedded in retired compatibility specimen's
        // resources from System.Private.CoreLib. Those reflective loads therefore
        // start in the default context rather than the specimen's collectible
        // context. Share the bounded drawing/resource support closure so the facade
        // and the resource decoder also see one Bitmap identity.
        PreloadDrawingFacade(Path.GetFullPath(args[4]), Path.GetFullPath(args[7]));
        PreloadDefaultRuntimeSupport(Path.GetFullPath(args[6]));

        var probe = Environment.GetEnvironmentVariable("GUI_FORMS_LOAD_PROBE_METHOD");
        var context = new FacadeLoadContext(
            extractDirectory, Path.GetFullPath(args[2]), Path.GetFullPath(args[3]),
            Path.GetFullPath(args[4]), Path.GetFullPath(args[5]),
            Path.GetFullPath(args[6]), Path.GetFullPath(args[7]),
            Path.GetFullPath(args[8]),
            isCollectible: !string.IsNullOrWhiteSpace(probe));
        var entryPath = Path.Combine(extractDirectory, "retired compatibility specimen.dll");
        var assembly = context.LoadFromAssemblyPath(entryPath);
        Console.WriteLine($"entry={assembly.GetName().Name}|forms={context.FormsIdentity}");
        if (!string.IsNullOrWhiteSpace(probe))
        {
            IlSiteProbe.Write(context, extractDirectory, probe);
            context.Unload();
            return 0;
        }
        var runWorkingDirectory = Environment.GetEnvironmentVariable(
            "GUI_FORMS_RUN_WORKING_DIRECTORY");
        var previousWorkingDirectory = Environment.CurrentDirectory;
        if (!string.IsNullOrWhiteSpace(runWorkingDirectory))
        {
            runWorkingDirectory = Path.GetFullPath(runWorkingDirectory);
            Directory.CreateDirectory(runWorkingDirectory);
            Environment.CurrentDirectory = runWorkingDirectory;
            Console.WriteLine($"working-directory={runWorkingDirectory}|source=GUI_FORMS_RUN_WORKING_DIRECTORY");
        }
        try
        {
            var entryPoint = assembly.EntryPoint ??
                throw new MissingMethodException("Extracted retired compatibility specimen assembly has no entry point.");
            object?[]? parameters = entryPoint.GetParameters().Length == 0
                ? null : [Array.Empty<string>()];
            var result = entryPoint.Invoke(null, parameters);
            if (result is Task task) task.GetAwaiter().GetResult();
            Console.WriteLine("entry=returned");
        }
        finally
        {
            Environment.CurrentDirectory = previousWorkingDirectory;
        }
        if (context.IsCollectible) context.Unload();
        return 0;
    }

    private static void PreloadDefaultRuntimeSupport(string desktopRuntimeDirectory)
    {
        foreach (var name in DefaultRuntimeSupport)
        {
            var path = Path.Combine(desktopRuntimeDirectory, name + ".dll");
            if (!File.Exists(path))
            {
                Console.WriteLine($"preload={name}|source=desktop-runtime-default|status=absent");
                continue;
            }
            var assembly = AssemblyLoadContext.Default.LoadFromAssemblyPath(path);
            Console.WriteLine($"preload={assembly.GetName().Name}|source=desktop-runtime-default");
        }
    }

    private static void PreloadDrawingFacade(string drawingPath, string nativeDirectory)
    {
        var assembly = AssemblyLoadContext.Default.LoadFromAssemblyPath(drawingPath);
        NativeLibrary.SetDllImportResolver(assembly, (libraryName, _, _) =>
        {
            if (!libraryName.Equals("gui_drawing_abi0", StringComparison.OrdinalIgnoreCase) &&
                !libraryName.Equals("gui_drawing_raster0", StringComparison.OrdinalIgnoreCase))
                return 0;
            var fileName = Path.HasExtension(libraryName) ? libraryName : libraryName + ".dll";
            var path = Path.Combine(nativeDirectory, fileName);
            if (!File.Exists(path)) return 0;
            Console.WriteLine($"native-default={fileName}");
            return NativeLibrary.Load(path);
        });
        DrawingFacade = assembly;
        var bitmap = assembly.GetType("System.Drawing.Bitmap", throwOnError: true)!;
        var token = assembly.GetName().GetPublicKeyToken();
        var tokenText = token is null || token.Length == 0 ? "null" :
            Convert.ToHexString(token).ToLowerInvariant();
        Console.WriteLine($"preload={assembly.GetName().Name}|source=drawing-facade-default|token:{tokenText}|bitmap:{bitmap.Assembly.GetName().Name}");
    }

    private static void ReportException(Exception error)
    {
        var current = error;
        for (var depth = 0; current is not null && depth < 12; ++depth)
        {
            Console.Error.WriteLine($"failure[{depth}]={current.GetType().FullName}|{Sanitize(current.Message)}");
            if (!string.IsNullOrWhiteSpace(current.StackTrace))
                Console.Error.WriteLine($"failure-stack[{depth}]={Sanitize(current.StackTrace)}");
            current = current.InnerException!;
        }
    }

    private static string Sanitize(string value) => value.Replace('\r', ' ').Replace('\n', ' ');
}

internal sealed class FacadeLoadContext : AssemblyLoadContext
{
    private readonly string extractDirectory;
    private readonly string formsPath;
    private readonly string primitivesPath;
    private readonly string drawingPath;
    private readonly string coreRuntimeDirectory;
    private readonly string desktopRuntimeDirectory;
    private readonly string nativeDirectory;
    private readonly string specimenDirectory;

    public FacadeLoadContext(string extractDirectory, string formsPath,
                             string primitivesPath, string drawingPath,
                             string coreRuntimeDirectory,
                             string desktopRuntimeDirectory, string nativeDirectory,
                             string specimenDirectory, bool isCollectible)
        : base("GUI.Forms retired compatibility specimen facade load", isCollectible)
    {
        this.extractDirectory = extractDirectory;
        this.formsPath = formsPath;
        this.primitivesPath = primitivesPath;
        this.drawingPath = drawingPath;
        this.coreRuntimeDirectory = coreRuntimeDirectory;
        this.desktopRuntimeDirectory = desktopRuntimeDirectory;
        this.nativeDirectory = nativeDirectory;
        this.specimenDirectory = specimenDirectory;
    }

    public string FormsIdentity { get; private set; } = "not-loaded";

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        var name = assemblyName.Name ?? string.Empty;
        if (name.Equals("System.Drawing.Common", StringComparison.OrdinalIgnoreCase))
        {
            Console.WriteLine("resolve=System.Drawing.Common|source=drawing-facade-default-explicit");
            return Program.DrawingFacade ??
                throw new InvalidOperationException("Drawing facade was not preloaded.");
        }
        if (Program.DefaultRuntimeSupport.Contains(name, StringComparer.OrdinalIgnoreCase))
        {
            Console.WriteLine($"resolve={name}|source=default-shared");
            return null;
        }
        var facadePath = name switch
        {
            "System.Windows.Forms" => formsPath,
            "System.Windows.Forms.Primitives" => primitivesPath,
            "System.Drawing.Common" => drawingPath,
            _ => null,
        };
        var path = facadePath ?? Candidate(extractDirectory, name);
        if (path is null && Candidate(coreRuntimeDirectory, name) is not null)
        {
            // Core.App is part of the runner's trusted platform set. Returning null
            // shares its types through the default context and prevents lookalike
            // identities such as two distinct System.Drawing.Color structs.
            Console.WriteLine($"resolve={name}|source=default-core");
            return null;
        }
        path ??= Candidate(desktopRuntimeDirectory, name);
        if (path is null) return null;
        var loaded = LoadFromAssemblyPath(path);
        if (name == "System.Windows.Forms")
        {
            var token = loaded.GetName().GetPublicKeyToken();
            FormsIdentity = token is null || token.Length == 0 ? "facade-token-null" :
                "unexpected-token-" + Convert.ToHexString(token).ToLowerInvariant();
        }
        Console.WriteLine($"resolve={name}|source={Source(path)}");
        return loaded;
    }

    protected override nint LoadUnmanagedDll(string unmanagedDllName)
    {
        foreach (var directory in new[] { nativeDirectory, specimenDirectory })
        {
            var path = Path.Combine(directory, unmanagedDllName);
            if (!Path.HasExtension(path)) path += ".dll";
            if (!File.Exists(path)) continue;
            Console.WriteLine($"native={Path.GetFileName(path)}");
            return LoadUnmanagedDllFromPath(path);
        }
        return 0;
    }

    private static string? Candidate(string directory, string name)
    {
        var path = Path.Combine(directory, name + ".dll");
        return File.Exists(path) ? path : null;
    }

    private string Source(string path)
    {
        if (path.Equals(formsPath, StringComparison.OrdinalIgnoreCase) ||
            path.Equals(primitivesPath, StringComparison.OrdinalIgnoreCase) ||
            path.Equals(drawingPath, StringComparison.OrdinalIgnoreCase)) return "facade";
        if (path.StartsWith(extractDirectory, StringComparison.OrdinalIgnoreCase)) return "bundle";
        if (path.StartsWith(coreRuntimeDirectory, StringComparison.OrdinalIgnoreCase)) return "core-runtime";
        return "desktop-runtime";
    }
}
