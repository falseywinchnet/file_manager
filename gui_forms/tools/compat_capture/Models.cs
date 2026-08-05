using System.Text.Json.Serialization;

namespace GuiForms.CompatCapture;

internal sealed class CaptureManifest
{
    [JsonPropertyOrder(0)] public string Schema { get; init; } = "gui.forms.compat.capture/v0";
    [JsonPropertyOrder(1)] public string ToolVersion { get; init; } = "0.1.0";
    [JsonPropertyOrder(2)] public required SpecimenRecord Specimen { get; init; }
    [JsonPropertyOrder(3)] public required ScanPolicyRecord Policy { get; init; }
    [JsonPropertyOrder(4)] public required IReadOnlyList<AssemblyRecord> Assemblies { get; init; }
    [JsonPropertyOrder(5)] public required IReadOnlyList<ApiUseRecord> ApiUses { get; init; }
    [JsonPropertyOrder(6)] public required IReadOnlyList<CustomControlRecord> CustomControls { get; init; }
    [JsonPropertyOrder(7)] public required IReadOnlyList<NativeImportRecord> NativeImports { get; init; }
    [JsonPropertyOrder(8)] public required SummaryRecord Summary { get; init; }
    [JsonPropertyOrder(9)] public required IReadOnlyList<DiagnosticRecord> Diagnostics { get; init; }
}

internal sealed class SpecimenRecord
{
    public required string Label { get; init; }
    public required string FileName { get; init; }
    public required string Sha256 { get; init; }
    public required long Size { get; init; }
    public required string ContainerKind { get; init; }
    public BundleRecord? Bundle { get; init; }
    public required IReadOnlyList<InputFileRecord> Inputs { get; init; }
}

internal sealed class BundleRecord
{
    public required int MajorVersion { get; init; }
    public required int MinorVersion { get; init; }
    public required string BundleId { get; init; }
    public required int FileCount { get; init; }
    public required IReadOnlyList<BundleFileRecord> Files { get; init; }
}

internal sealed class BundleFileRecord
{
    public required string RelativePath { get; init; }
    public required string Kind { get; init; }
    public required long Size { get; init; }
    public required long StoredSize { get; init; }
    public required string Sha256 { get; init; }
}

internal sealed class InputFileRecord
{
    public required string RelativePath { get; init; }
    public required string SourceKind { get; init; }
    public required long Size { get; init; }
    public required string Sha256 { get; init; }
    public required bool ManagedMetadata { get; init; }
}

internal sealed class ScanPolicyRecord
{
    public required string EvidenceClass { get; init; }
    public required string DefaultDisposition { get; init; }
    public required bool ExecuteTargetCode { get; init; }
    public required bool IncludeMethodBodyBytes { get; init; }
    public required bool IncludeResources { get; init; }
    public required bool RedactApplicationTypeNames { get; init; }
    public required IReadOnlyList<string> TrackedAssemblyPatterns { get; init; }
    public required IReadOnlyList<DispositionRecord> DispositionCatalog { get; init; }
}

internal sealed class DispositionRecord
{
    public required string Name { get; init; }
    public required string Meaning { get; init; }
}

internal sealed class AssemblyRecord
{
    public required string RelativePath { get; init; }
    public required string SourceKind { get; init; }
    public required string Sha256 { get; init; }
    public required string Name { get; init; }
    public required string Version { get; init; }
    public required string Culture { get; init; }
    public required string PublicKeyToken { get; init; }
    public required string Mvid { get; init; }
    public required string Machine { get; init; }
    public string? TargetFramework { get; init; }
    public required int DefinedTypeCount { get; init; }
    public required int MethodBodyCount { get; init; }
    public required IReadOnlyList<string> References { get; init; }
}

internal sealed class ApiUseRecord
{
    public required string TargetAssembly { get; init; }
    public required string Type { get; init; }
    public string? Member { get; init; }
    public string? Signature { get; init; }
    public required string MemberKind { get; init; }
    public required IReadOnlyList<string> Operations { get; init; }
    public required int IlOccurrenceCount { get; init; }
    public required bool MetadataReference { get; init; }
    public required IReadOnlyList<string> SourceAssemblies { get; init; }
    public required string Evidence { get; init; }
    public required string Disposition { get; init; }
}

internal sealed class CustomControlRecord
{
    public required string PrivateTypeId { get; init; }
    public required string SourceAssembly { get; init; }
    public required string FormsBaseAssembly { get; init; }
    public required string FormsBaseType { get; init; }
    public required int InheritanceDepth { get; init; }
    public required string Evidence { get; init; }
    public required string Disposition { get; init; }
}

internal sealed class NativeImportRecord
{
    public required string SourceAssembly { get; init; }
    public required string Module { get; init; }
    public required string EntryPoint { get; init; }
    public required string Attributes { get; init; }
    public required int Count { get; init; }
    public required string Evidence { get; init; }
    public required string Disposition { get; init; }
}

internal sealed class SummaryRecord
{
    public required int ManagedAssemblyCount { get; init; }
    public required int TrackedApiCount { get; init; }
    public required int IlObservedApiCount { get; init; }
    public required int CustomControlCount { get; init; }
    public required int NativeImportCount { get; init; }
    public required int ErrorCount { get; init; }
    public required int WarningCount { get; init; }
}

internal sealed class DiagnosticRecord
{
    public required string Severity { get; init; }
    public required string Code { get; init; }
    public required string Subject { get; init; }
    public required string Message { get; init; }
}

internal readonly record struct TypeIdentity(string Assembly, string FullName)
{
    public static TypeIdentity Unknown(string name) => new("<unknown>", name);
}

internal sealed class ApiUseAccumulator
{
    public required string TargetAssembly { get; init; }
    public required string Type { get; init; }
    public string? Member { get; init; }
    public string? Signature { get; init; }
    public required string MemberKind { get; init; }
    public HashSet<string> Operations { get; } = new(StringComparer.Ordinal);
    public HashSet<string> SourceAssemblies { get; } = new(StringComparer.Ordinal);
    public int IlOccurrenceCount { get; set; }
    public bool MetadataReference { get; set; }
}
