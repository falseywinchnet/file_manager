using System.Reflection;
using System.Runtime.Loader;
using System.Text.Json;

namespace GuiForms.LoaderLab;

internal sealed class FacadeLoadContext : AssemblyLoadContext
{
    private readonly string facadePath;

    public FacadeLoadContext(string facadePath) : base("gui-forms-facade-lab", isCollectible: true)
    {
        this.facadePath = facadePath;
    }

    public AssemblyName? RequestedFacade { get; private set; }
    public AssemblyName? LoadedFacade { get; private set; }

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        if (!string.Equals(assemblyName.Name, "System.Windows.Forms", StringComparison.Ordinal))
        {
            return null;
        }
        RequestedFacade = assemblyName;
        var assembly = LoadFromAssemblyPath(facadePath);
        LoadedFacade = assembly.GetName();
        return assembly;
    }
}

internal sealed record Result(
    string Schema,
    string Status,
    string RequestedAssembly,
    string RequestedPublicKeyToken,
    string LoadedAssembly,
    string LoadedPublicKeyToken,
    bool IdentityMismatchAccepted,
    string ProbeResult);

internal static class Program
{
    private static string Token(AssemblyName? name) =>
        name?.GetPublicKeyToken() is { Length: > 0 } token
            ? Convert.ToHexString(token).ToLowerInvariant()
            : "null";

    public static int Main(string[] args)
    {
        if (args.Length != 2)
        {
            Console.Error.WriteLine("usage: gui-forms-loader-lab CONSUMER FACADE");
            return 2;
        }

        var consumerPath = Path.GetFullPath(args[0]);
        var facadePath = Path.GetFullPath(args[1]);
        if (!File.Exists(consumerPath) || !File.Exists(facadePath))
        {
            Console.Error.WriteLine("consumer and facade paths must exist");
            return 2;
        }

        try
        {
            var context = new FacadeLoadContext(facadePath);
            var consumer = context.LoadFromAssemblyPath(consumerPath);
            var probe = consumer.GetType("GuiForms.LoaderConsumer.Probe", throwOnError: true)!;
            var method = probe.GetMethod("Run", BindingFlags.Public | BindingFlags.Static)
                         ?? throw new MissingMethodException(probe.FullName, "Run");
            var probeResult = (string?)method.Invoke(null, null)
                              ?? throw new InvalidOperationException("probe returned null");
            var requested = context.RequestedFacade
                            ?? throw new InvalidOperationException("facade resolution was not requested");
            var loaded = context.LoadedFacade
                         ?? throw new InvalidOperationException("facade was not loaded");
            var result = new Result(
                "gui.forms.compat.loader-lab/v1",
                "pass",
                requested.FullName ?? requested.Name ?? "unknown",
                Token(requested),
                loaded.FullName ?? loaded.Name ?? "unknown",
                Token(loaded),
                !string.Equals(Token(requested), Token(loaded), StringComparison.Ordinal),
                probeResult);
            Console.WriteLine(JsonSerializer.Serialize(result, new JsonSerializerOptions
            {
                PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
            }));
            context.Unload();
            return 0;
        }
        catch (Exception error)
        {
            Console.Error.WriteLine($"loader-lab: {error}");
            return 1;
        }
    }
}
