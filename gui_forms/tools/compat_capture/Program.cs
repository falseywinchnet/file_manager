using System.Reflection.Metadata;
using System.Reflection.PortableExecutable;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace GuiForms.CompatCapture;

internal static class Program
{
    private static readonly string[] DefaultTrackedAssemblyPrefixes =
    [
        "Accessibility",
        "System.ComponentModel*",
        "System.Drawing*",
        "System.Windows.Forms",
        "System.Windows.Forms.Primitives",
    ];

    public static int Main(string[] args)
    {
        try
        {
            var options = CaptureOptions.Parse(args);
            if (options.ShowHelp)
            {
                Console.Out.WriteLine(CaptureOptions.HelpText);
                return 0;
            }
            Run(options);
            return 0;
        }
        catch (CaptureUsageException exception)
        {
            Console.Error.WriteLine("compat-capture: " + exception.Message);
            Console.Error.WriteLine("Run with --help for usage.");
            return 2;
        }
        catch (Exception exception) when (
            exception is IOException or UnauthorizedAccessException or BadImageFormatException or
            InvalidDataException or JsonException or OverflowException)
        {
            Console.Error.WriteLine($"compat-capture: {exception.GetType().Name}: {exception.Message}");
            return 1;
        }
    }

    private static void Run(CaptureOptions options)
    {
        var inputPath = Path.GetFullPath(options.Input!);
        if (!File.Exists(inputPath))
        {
            throw new CaptureUsageException($"Input does not exist: {options.Input}");
        }

        var image = File.ReadAllBytes(inputPath);
        var inputHash = BundleReader.Sha256(image);
        if (options.ExpectedSha256 is not null &&
            !inputHash.Equals(options.ExpectedSha256, StringComparison.OrdinalIgnoreCase))
        {
            throw new InvalidDataException(
                $"Input SHA-256 {inputHash} does not match expected {options.ExpectedSha256.ToLowerInvariant()}.");
        }

        var bundle = BundleReader.TryRead(image);
        var inputs = new List<InputFileRecord>();
        var managedInputs = new List<ManagedInput>();
        try
        {
            BundleRecord? bundleRecord = null;
            if (bundle is not null)
            {
                var fileRecords = new List<BundleFileRecord>(bundle.Entries.Count);
                foreach (var entry in bundle.Entries)
                {
                    var bytes = BundleReader.ReadEntry(image, entry);
                    var hash = BundleReader.Sha256(bytes);
                    fileRecords.Add(new BundleFileRecord
                    {
                        RelativePath = entry.RelativePath,
                        Kind = entry.FileType.ToString().ToLowerInvariant(),
                        Size = entry.Size,
                        StoredSize = entry.StoredSize,
                        Sha256 = hash,
                    });
                    var managed = entry.FileType == BundleFileType.Assembly && HasManagedMetadata(bytes);
                    inputs.Add(new InputFileRecord
                    {
                        RelativePath = entry.RelativePath,
                        SourceKind = "bundle",
                        Size = bytes.LongLength,
                        Sha256 = hash,
                        ManagedMetadata = managed,
                    });
                    if (managed)
                    {
                        managedInputs.Add(new ManagedInput(entry.RelativePath, "bundle", bytes));
                    }
                }
                bundleRecord = new BundleRecord
                {
                    MajorVersion = bundle.MajorVersion,
                    MinorVersion = bundle.MinorVersion,
                    BundleId = bundle.BundleId,
                    FileCount = bundle.Entries.Count,
                    Files = fileRecords.OrderBy(value => value.RelativePath, StringComparer.Ordinal).ToArray(),
                };
            }
            else if (HasManagedMetadata(image))
            {
                var relativePath = Path.GetFileName(inputPath);
                inputs.Add(new InputFileRecord
                {
                    RelativePath = relativePath,
                    SourceKind = "primary",
                    Size = image.LongLength,
                    Sha256 = inputHash,
                    ManagedMetadata = true,
                });
                managedInputs.Add(new ManagedInput(relativePath, "primary", image));
            }
            else
            {
                throw new BadImageFormatException("Input is neither a .NET single-file bundle nor a managed PE file.");
            }

            foreach (var includeDirectory in options.IncludeDirectories)
            {
                AddDirectoryInputs(includeDirectory, inputs, managedInputs);
            }

            IEnumerable<string> selectedPrefixes = options.TrackedAssemblyPrefixes.Count == 0
                ? DefaultTrackedAssemblyPrefixes
                : options.TrackedAssemblyPrefixes;
            var trackedPrefixes = selectedPrefixes
                .Distinct(StringComparer.Ordinal)
                .OrderBy(value => value, StringComparer.Ordinal)
                .ToArray();
            var scan = new MetadataScanner(trackedPrefixes).Scan(managedInputs);
            var errorCount = scan.Diagnostics.Count(value => value.Severity == "error");
            var warningCount = scan.Diagnostics.Count(value => value.Severity == "warning");
            var manifest = new CaptureManifest
            {
                Specimen = new SpecimenRecord
                {
                    Label = options.Label!,
                    FileName = Path.GetFileName(inputPath),
                    Sha256 = inputHash,
                    Size = image.LongLength,
                    ContainerKind = bundle is null ? "managed_pe" : "dotnet_single_file_bundle",
                    Bundle = bundleRecord,
                    Inputs = inputs
                        .OrderBy(value => value.SourceKind, StringComparer.Ordinal)
                        .ThenBy(value => value.RelativePath, StringComparer.Ordinal)
                        .ToArray(),
                },
                Policy = new ScanPolicyRecord
                {
                    EvidenceClass = "OBSERVED static metadata and IL operands",
                    DefaultDisposition = "unclassified",
                    ExecuteTargetCode = false,
                    IncludeMethodBodyBytes = false,
                    IncludeResources = false,
                    RedactApplicationTypeNames = true,
                    TrackedAssemblyPatterns = trackedPrefixes,
                    DispositionCatalog = DispositionCatalog,
                },
                Assemblies = scan.Assemblies,
                ApiUses = scan.ApiUses,
                CustomControls = scan.CustomControls,
                NativeImports = scan.NativeImports,
                Summary = new SummaryRecord
                {
                    ManagedAssemblyCount = scan.Assemblies.Count,
                    TrackedApiCount = scan.ApiUses.Count,
                    IlObservedApiCount = scan.ApiUses.Count(value => value.IlOccurrenceCount > 0),
                    CustomControlCount = scan.CustomControls.Count,
                    NativeImportCount = scan.NativeImports.Sum(value => value.Count),
                    ErrorCount = errorCount,
                    WarningCount = warningCount,
                },
                Diagnostics = scan.Diagnostics,
            };

            var json = JsonSerializer.Serialize(manifest, JsonOptions) + "\n";
            if (options.Output == "-")
            {
                Console.Out.Write(json);
            }
            else
            {
                var outputPath = Path.GetFullPath(options.Output!);
                var directory = Path.GetDirectoryName(outputPath);
                if (!string.IsNullOrEmpty(directory))
                {
                    Directory.CreateDirectory(directory);
                }
                File.WriteAllText(outputPath, json, new System.Text.UTF8Encoding(false));
            }
        }
        finally
        {
            foreach (var managedInput in managedInputs)
            {
                managedInput.Dispose();
            }
        }
    }

    private static void AddDirectoryInputs(string directoryArgument,
        ICollection<InputFileRecord> inputs, ICollection<ManagedInput> managedInputs)
    {
        var directory = Path.GetFullPath(directoryArgument);
        if (!Directory.Exists(directory))
        {
            throw new CaptureUsageException($"Include directory does not exist: {directoryArgument}");
        }

        foreach (var path in Directory.EnumerateFiles(directory, "*", SearchOption.AllDirectories)
                     .Where(path => path.EndsWith(".dll", StringComparison.OrdinalIgnoreCase) ||
                                    path.EndsWith(".exe", StringComparison.OrdinalIgnoreCase))
                     .OrderBy(path => path, StringComparer.Ordinal))
        {
            var bytes = File.ReadAllBytes(path);
            var relative = Path.GetRelativePath(directory, path).Replace('\\', '/');
            var managed = HasManagedMetadata(bytes);
            inputs.Add(new InputFileRecord
            {
                RelativePath = relative,
                SourceKind = "external",
                Size = bytes.LongLength,
                Sha256 = BundleReader.Sha256(bytes),
                ManagedMetadata = managed,
            });
            if (managed)
            {
                managedInputs.Add(new ManagedInput(relative, "external", bytes));
            }
        }
    }

    private static bool HasManagedMetadata(byte[] bytes)
    {
        try
        {
            using var stream = new MemoryStream(bytes, writable: false);
            using var reader = new PEReader(stream);
            return reader.HasMetadata;
        }
        catch (BadImageFormatException)
        {
            return false;
        }
    }

    private static readonly IReadOnlyList<DispositionRecord> DispositionCatalog =
    [
        new() { Name = "unclassified", Meaning = "Observed by the scanner; no compatibility decision has been made." },
        new() { Name = "required", Meaning = "Accepted into the bounded GUI.Forms compatibility facade." },
        new() { Name = "deferred", Meaning = "Compatible behavior is admitted, but not in the current delivery tier." },
        new() { Name = "excluded", Meaning = "Deliberately outside the GUI.Forms compatibility promise." },
        new() { Name = "application_side_port", Meaning = "Behavior belongs in an SDR-specific adapter or portable custom-control port." },
    ];

    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
        WriteIndented = true,
        NewLine = "\n",
    };
}

internal sealed class CaptureOptions
{
    public string? Input { get; private set; }
    public string? Output { get; private set; }
    public string? Label { get; private set; }
    public string? ExpectedSha256 { get; private set; }
    public List<string> IncludeDirectories { get; } = [];
    public List<string> TrackedAssemblyPrefixes { get; } = [];
    public bool ShowHelp { get; private set; }

    public static string HelpText =>
        """
        GUI.Forms compatibility Capture-0

        Inspects CLI metadata and IL operands without loading or executing target code.

        Usage:
          dotnet run --project tools/compat_capture -- \
            --input <bundle-or-managed-pe> --output <manifest.json|-> --label <stable-label> \
            [--expected-sha256 <hex>] [--include-dir <directory>]... \
            [--assembly-prefix <prefix>]...

        Defaults track Accessibility, System.ComponentModel*, System.Drawing*,
        System.Windows.Forms, and System.Windows.Forms.Primitives. A trailing '*'
        means prefix match; otherwise the assembly name must match exactly. All
        managed inputs are scanned; this filter selects compatibility-facade
        targets, not source assemblies.
        """;

    public static CaptureOptions Parse(string[] args)
    {
        var options = new CaptureOptions();
        for (var index = 0; index < args.Length; ++index)
        {
            var argument = args[index];
            switch (argument)
            {
                case "--help" or "-h":
                    options.ShowHelp = true;
                    break;
                case "--input":
                    options.Input = ReadValue(args, ref index, argument);
                    break;
                case "--output":
                    options.Output = ReadValue(args, ref index, argument);
                    break;
                case "--label":
                    options.Label = ReadValue(args, ref index, argument);
                    break;
                case "--expected-sha256":
                    options.ExpectedSha256 = ReadValue(args, ref index, argument);
                    break;
                case "--include-dir":
                    options.IncludeDirectories.Add(ReadValue(args, ref index, argument));
                    break;
                case "--assembly-prefix":
                    options.TrackedAssemblyPrefixes.Add(ReadValue(args, ref index, argument));
                    break;
                default:
                    throw new CaptureUsageException($"Unknown argument: {argument}");
            }
        }

        if (options.ShowHelp)
        {
            return options;
        }
        if (string.IsNullOrWhiteSpace(options.Input) ||
            string.IsNullOrWhiteSpace(options.Output) ||
            string.IsNullOrWhiteSpace(options.Label))
        {
            throw new CaptureUsageException("--input, --output, and --label are required.");
        }
        if (options.ExpectedSha256 is not null &&
            (options.ExpectedSha256.Length != 64 ||
             options.ExpectedSha256.Any(character => !Uri.IsHexDigit(character))))
        {
            throw new CaptureUsageException("--expected-sha256 must contain exactly 64 hexadecimal characters.");
        }
        return options;
    }

    private static string ReadValue(string[] args, ref int index, string argument)
    {
        if (++index >= args.Length || string.IsNullOrEmpty(args[index]))
        {
            throw new CaptureUsageException($"{argument} requires a value.");
        }
        return args[index];
    }
}

internal sealed class CaptureUsageException(string message) : Exception(message);
