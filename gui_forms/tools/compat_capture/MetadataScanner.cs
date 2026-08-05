using System.Buffers.Binary;
using System.Collections.Immutable;
using System.Reflection;
using System.Reflection.Emit;
using System.Reflection.Metadata;
using System.Reflection.Metadata.Ecma335;
using System.Reflection.PortableExecutable;
using System.Security.Cryptography;

namespace GuiForms.CompatCapture;

internal sealed class ManagedInput : IDisposable
{
    private readonly MemoryStream stream;

    public ManagedInput(string relativePath, string sourceKind, byte[] bytes)
    {
        RelativePath = relativePath;
        SourceKind = sourceKind;
        Bytes = bytes;
        stream = new MemoryStream(bytes, writable: false);
        PeReader = new PEReader(stream, PEStreamOptions.LeaveOpen);
        if (!PeReader.HasMetadata)
        {
            throw new BadImageFormatException($"{relativePath} has no CLI metadata.");
        }
        Reader = PeReader.GetMetadataReader();
        Assembly = Reader.GetAssemblyDefinition();
        AssemblyName = Reader.GetString(Assembly.Name);
    }

    public string RelativePath { get; }
    public string SourceKind { get; }
    public byte[] Bytes { get; }
    public PEReader PeReader { get; }
    public MetadataReader Reader { get; }
    public AssemblyDefinition Assembly { get; }
    public string AssemblyName { get; }

    public void Dispose()
    {
        PeReader.Dispose();
        stream.Dispose();
    }
}

internal sealed class MetadataScanResult
{
    public required IReadOnlyList<AssemblyRecord> Assemblies { get; init; }
    public required IReadOnlyList<ApiUseRecord> ApiUses { get; init; }
    public required IReadOnlyList<CustomControlRecord> CustomControls { get; init; }
    public required IReadOnlyList<NativeImportRecord> NativeImports { get; init; }
    public required IReadOnlyList<DiagnosticRecord> Diagnostics { get; init; }
}

internal sealed class MetadataScanner
{
    private readonly string[] trackedPrefixes;
    private readonly List<DiagnosticRecord> diagnostics = [];
    private readonly Dictionary<ApiKey, ApiUseAccumulator> apiUses = [];
    private readonly Dictionary<NativeImportKey, int> nativeImports = [];
    private readonly Dictionary<TypeKey, TypeNode> definedTypes = [];

    public MetadataScanner(IEnumerable<string> trackedPrefixes)
    {
        this.trackedPrefixes = trackedPrefixes
            .Where(prefix => !string.IsNullOrWhiteSpace(prefix))
            .Select(prefix => prefix.Trim())
            .Distinct(StringComparer.Ordinal)
            .OrderBy(prefix => prefix, StringComparer.Ordinal)
            .ToArray();
    }

    public MetadataScanResult Scan(IReadOnlyList<ManagedInput> inputs)
    {
        foreach (var input in inputs)
        {
            IndexDefinedTypes(input);
        }

        var assemblies = new List<AssemblyRecord>(inputs.Count);
        foreach (var input in inputs.OrderBy(value => value.RelativePath, StringComparer.Ordinal))
        {
            assemblies.Add(ScanAssembly(input));
        }

        return new MetadataScanResult
        {
            Assemblies = assemblies,
            ApiUses = apiUses.Values
                .Select(value => new ApiUseRecord
                {
                    TargetAssembly = value.TargetAssembly,
                    Type = value.Type,
                    Member = value.Member,
                    Signature = value.Signature,
                    MemberKind = value.MemberKind,
                    Operations = value.Operations.OrderBy(item => item, StringComparer.Ordinal).ToArray(),
                    IlOccurrenceCount = value.IlOccurrenceCount,
                    MetadataReference = value.MetadataReference,
                    SourceAssemblies = value.SourceAssemblies.OrderBy(item => item, StringComparer.Ordinal).ToArray(),
                    Evidence = value.IlOccurrenceCount > 0 ? "OBSERVED static IL operand" : "OBSERVED metadata reference",
                    Disposition = "unclassified",
                })
                .OrderBy(value => value.TargetAssembly, StringComparer.Ordinal)
                .ThenBy(value => value.Type, StringComparer.Ordinal)
                .ThenBy(value => value.Member ?? string.Empty, StringComparer.Ordinal)
                .ThenBy(value => value.Signature ?? string.Empty, StringComparer.Ordinal)
                .ToArray(),
            CustomControls = FindCustomControls(),
            NativeImports = nativeImports
                .Select(pair => new NativeImportRecord
                {
                    SourceAssembly = pair.Key.SourceAssembly,
                    Module = pair.Key.Module,
                    EntryPoint = pair.Key.EntryPoint,
                    Attributes = pair.Key.Attributes,
                    Count = pair.Value,
                    Evidence = "OBSERVED ImplMap metadata",
                    Disposition = "unclassified",
                })
                .OrderBy(value => value.SourceAssembly, StringComparer.Ordinal)
                .ThenBy(value => value.Module, StringComparer.OrdinalIgnoreCase)
                .ThenBy(value => value.EntryPoint, StringComparer.Ordinal)
                .ToArray(),
            Diagnostics = diagnostics
                .OrderBy(value => value.Severity, StringComparer.Ordinal)
                .ThenBy(value => value.Code, StringComparer.Ordinal)
                .ThenBy(value => value.Subject, StringComparer.Ordinal)
                .ToArray(),
        };
    }

    private AssemblyRecord ScanAssembly(ManagedInput input)
    {
        var reader = input.Reader;
        var provider = new TypeNameProvider(input, redactPrivateTypeNames: true);
        var methodBodyCount = 0;

        foreach (var typeReferenceHandle in reader.TypeReferences)
        {
            var identity = provider.GetTypeFromReference(reader, typeReferenceHandle, 0);
            if (IsTracked(identity.Assembly))
            {
                ObserveApi(input.AssemblyName, identity, null, null, "type", "metadata_reference", false);
            }
        }

        foreach (var memberHandle in reader.MemberReferences)
        {
            try
            {
                var resolved = ResolveMemberReference(input, memberHandle, provider);
                if (resolved is not null && IsTracked(resolved.Value.Owner.Assembly))
                {
                    ObserveApi(input.AssemblyName, resolved.Value.Owner, resolved.Value.Name,
                        resolved.Value.Signature, resolved.Value.Kind, "metadata_reference", false);
                }
            }
            catch (BadImageFormatException exception)
            {
                AddDiagnostic("warning", "CAPTURE_MEMBER_SIGNATURE", input.RelativePath, exception.Message);
            }
        }

        foreach (var methodHandle in reader.MethodDefinitions)
        {
            var method = reader.GetMethodDefinition(methodHandle);
            if ((method.Attributes & MethodAttributes.PinvokeImpl) != 0)
            {
                var import = method.GetImport();
                var importModule = reader.GetModuleReference(import.Module);
                var key = new NativeImportKey(
                    input.AssemblyName,
                    reader.GetString(importModule.Name),
                    reader.GetString(import.Name),
                    import.Attributes.ToString());
                nativeImports[key] = nativeImports.GetValueOrDefault(key) + 1;
            }

            if (method.RelativeVirtualAddress == 0)
            {
                continue;
            }
            ++methodBodyCount;
            try
            {
                var body = input.PeReader.GetMethodBody(method.RelativeVirtualAddress);
                var il = body.GetILBytes();
                if (il is null)
                {
                    AddDiagnostic("warning", "CAPTURE_IL_BODY", input.RelativePath,
                        "Method body did not expose an IL byte stream.");
                }
                else
                {
                    ScanIl(input, il, provider);
                }
            }
            catch (Exception exception) when (exception is BadImageFormatException or InvalidOperationException)
            {
                AddDiagnostic("warning", "CAPTURE_IL_BODY", input.RelativePath, exception.Message);
            }
        }

        var assembly = input.Assembly;
        var publicKey = reader.GetBlobBytes(assembly.PublicKey);
        var culture = assembly.Culture.IsNil ? "neutral" : reader.GetString(assembly.Culture);
        var moduleDefinition = reader.GetModuleDefinition();
        var mvid = moduleDefinition.Mvid.IsNil ? Guid.Empty : reader.GetGuid(moduleDefinition.Mvid);
        var references = reader.AssemblyReferences
            .Select(handle => FormatAssemblyReference(reader, reader.GetAssemblyReference(handle)))
            .OrderBy(value => value, StringComparer.Ordinal)
            .ToArray();

        return new AssemblyRecord
        {
            RelativePath = input.RelativePath,
            SourceKind = input.SourceKind,
            Sha256 = BundleReader.Sha256(input.Bytes),
            Name = input.AssemblyName,
            Version = assembly.Version.ToString(),
            Culture = culture,
            PublicKeyToken = GetPublicKeyToken(publicKey, assembly.Flags),
            Mvid = mvid.ToString("D"),
            Machine = input.PeReader.PEHeaders.CoffHeader.Machine.ToString(),
            TargetFramework = ReadTargetFramework(input, provider),
            DefinedTypeCount = reader.TypeDefinitions.Count,
            MethodBodyCount = methodBodyCount,
            References = references,
        };
    }

    private void ScanIl(ManagedInput input, byte[] il, TypeNameProvider provider)
    {
        var span = il.AsSpan();
        var offset = 0;
        while (offset < span.Length)
        {
            var instructionOffset = offset;
            ushort value = span[offset++];
            if (value == 0xfe)
            {
                if (offset >= span.Length)
                {
                    throw new BadImageFormatException("Truncated two-byte IL opcode.");
                }
                value = (ushort)(0xfe00 | span[offset++]);
            }
            if (!IlOpcodeTable.OperandTypes.TryGetValue(value, out var operandType))
            {
                throw new BadImageFormatException($"Unknown IL opcode 0x{value:x4} at {instructionOffset}.");
            }

            switch (operandType)
            {
                case OperandType.InlineField:
                case OperandType.InlineMethod:
                case OperandType.InlineTok:
                case OperandType.InlineType:
                    {
                        RequireBytes(span, offset, sizeof(int));
                        var token = BinaryPrimitives.ReadInt32LittleEndian(span.Slice(offset, sizeof(int)));
                        offset += sizeof(int);
                        ObserveToken(input, token, OpcodeOperation(value), provider);
                        break;
                    }
                case OperandType.InlineBrTarget:
                case OperandType.InlineI:
                case OperandType.InlineSig:
                case OperandType.InlineString:
                case OperandType.ShortInlineR:
                    RequireBytes(span, offset, 4);
                    offset += 4;
                    break;
                case OperandType.InlineI8:
                case OperandType.InlineR:
                    RequireBytes(span, offset, 8);
                    offset += 8;
                    break;
                case OperandType.InlineSwitch:
                    {
                        RequireBytes(span, offset, 4);
                        var count = BinaryPrimitives.ReadInt32LittleEndian(span.Slice(offset, 4));
                        if (count < 0 || count > (span.Length - offset - 4) / 4)
                        {
                            throw new BadImageFormatException("Invalid IL switch operand.");
                        }
                        offset += 4 + count * 4;
                        break;
                    }
                case OperandType.InlineVar:
                    RequireBytes(span, offset, 2);
                    offset += 2;
                    break;
                case OperandType.ShortInlineBrTarget:
                case OperandType.ShortInlineI:
                case OperandType.ShortInlineVar:
                    RequireBytes(span, offset, 1);
                    offset += 1;
                    break;
                case OperandType.InlineNone:
                    break;
                default:
                    throw new BadImageFormatException($"Unsupported IL operand kind {operandType}.");
            }
        }
    }

    private void ObserveToken(ManagedInput input, int token, string operation, TypeNameProvider provider)
    {
        EntityHandle handle;
        try
        {
            handle = MetadataTokens.EntityHandle(token);
        }
        catch (ArgumentException exception)
        {
            AddDiagnostic("warning", "CAPTURE_IL_TOKEN", input.RelativePath, exception.Message);
            return;
        }

        ResolvedMember? member = null;
        TypeIdentity? type = null;
        try
        {
            switch (handle.Kind)
            {
                case HandleKind.MemberReference:
                    member = ResolveMemberReference(input, (MemberReferenceHandle)handle, provider);
                    break;
                case HandleKind.MethodSpecification:
                    {
                        var specification = input.Reader.GetMethodSpecification((MethodSpecificationHandle)handle);
                        if (specification.Method.Kind == HandleKind.MemberReference)
                        {
                            member = ResolveMemberReference(input, (MemberReferenceHandle)specification.Method, provider);
                        }
                        break;
                    }
                case HandleKind.TypeReference:
                    type = provider.GetTypeFromReference(input.Reader, (TypeReferenceHandle)handle, 0);
                    break;
                case HandleKind.TypeSpecification:
                    type = provider.GetTypeFromSpecification(input.Reader, null,
                        (TypeSpecificationHandle)handle, 0);
                    break;
            }
        }
        catch (BadImageFormatException exception)
        {
            AddDiagnostic("warning", "CAPTURE_IL_RESOLVE", input.RelativePath, exception.Message);
            return;
        }

        if (member is not null && IsTracked(member.Value.Owner.Assembly))
        {
            ObserveApi(input.AssemblyName, member.Value.Owner, member.Value.Name,
                member.Value.Signature, member.Value.Kind, operation, true);
        }
        else if (type is not null && IsTracked(type.Value.Assembly))
        {
            ObserveApi(input.AssemblyName, type.Value, null, null, "type", operation, true);
        }
    }

    private ResolvedMember? ResolveMemberReference(
        ManagedInput input, MemberReferenceHandle handle, TypeNameProvider provider)
    {
        var member = input.Reader.GetMemberReference(handle);
        var owner = provider.ResolveTypeHandle(member.Parent);
        if (owner is null)
        {
            return null;
        }
        var name = input.Reader.GetString(member.Name);
        if (member.GetKind() == MemberReferenceKind.Method)
        {
            var signature = member.DecodeMethodSignature(provider, null);
            var generic = signature.GenericParameterCount == 0
                ? string.Empty
                : $"``{signature.GenericParameterCount}";
            var parameters = string.Join(",", signature.ParameterTypes.Select(type => type.FullName));
            return new ResolvedMember(owner.Value, name,
                $"{signature.ReturnType.FullName} {generic}({parameters})", "method");
        }
        var fieldType = member.DecodeFieldSignature(provider, null);
        return new ResolvedMember(owner.Value, name, fieldType.FullName, "field");
    }

    private void ObserveApi(string sourceAssembly, TypeIdentity owner, string? member,
        string? signature, string kind, string operation, bool ilOccurrence)
    {
        var key = new ApiKey(owner.Assembly, owner.FullName, member, signature, kind);
        if (!apiUses.TryGetValue(key, out var use))
        {
            use = new ApiUseAccumulator
            {
                TargetAssembly = owner.Assembly,
                Type = owner.FullName,
                Member = member,
                Signature = signature,
                MemberKind = kind,
            };
            apiUses.Add(key, use);
        }
        use.SourceAssemblies.Add(sourceAssembly);
        use.Operations.Add(operation);
        if (ilOccurrence)
        {
            ++use.IlOccurrenceCount;
        }
        else
        {
            use.MetadataReference = true;
        }
    }

    private void IndexDefinedTypes(ManagedInput input)
    {
        var provider = new TypeNameProvider(input, redactPrivateTypeNames: false);
        foreach (var handle in input.Reader.TypeDefinitions)
        {
            var type = input.Reader.GetTypeDefinition(handle);
            var identity = provider.GetTypeFromDefinition(input.Reader, handle, 0);
            var baseType = type.BaseType.IsNil ? null : provider.ResolveTypeHandle(type.BaseType);
            definedTypes[new TypeKey(input.AssemblyName, identity.FullName)] =
                new TypeNode(input.AssemblyName, identity.FullName, baseType);
        }
    }

    private IReadOnlyList<CustomControlRecord> FindCustomControls()
    {
        var controls = new List<CustomControlRecord>();
        foreach (var node in definedTypes.Values)
        {
            var depth = 0;
            var current = node.BaseType;
            var visited = new HashSet<TypeKey>();
            while (current is not null && depth < 256)
            {
                ++depth;
                if (IsFormsAssembly(current.Value.Assembly))
                {
                    controls.Add(new CustomControlRecord
                    {
                        PrivateTypeId = PrivateTypeId(node.Assembly, node.FullName),
                        SourceAssembly = node.Assembly,
                        FormsBaseAssembly = current.Value.Assembly,
                        FormsBaseType = current.Value.FullName,
                        InheritanceDepth = depth,
                        Evidence = "OBSERVED metadata inheritance chain",
                        Disposition = "unclassified",
                    });
                    break;
                }

                var key = new TypeKey(current.Value.Assembly, current.Value.FullName);
                if (!visited.Add(key) || !definedTypes.TryGetValue(key, out var parent))
                {
                    break;
                }
                current = parent.BaseType;
            }
        }
        return controls
            .OrderBy(value => value.SourceAssembly, StringComparer.Ordinal)
            .ThenBy(value => value.PrivateTypeId, StringComparer.Ordinal)
            .ToArray();
    }

    private string? ReadTargetFramework(ManagedInput input, TypeNameProvider provider)
    {
        foreach (var handle in input.Assembly.GetCustomAttributes())
        {
            var attribute = input.Reader.GetCustomAttribute(handle);
            if (attribute.Constructor.Kind != HandleKind.MemberReference)
            {
                continue;
            }
            var member = input.Reader.GetMemberReference((MemberReferenceHandle)attribute.Constructor);
            var owner = provider.ResolveTypeHandle(member.Parent);
            if (owner?.FullName != "System.Runtime.Versioning.TargetFrameworkAttribute")
            {
                continue;
            }
            try
            {
                var value = input.Reader.GetBlobReader(attribute.Value);
                if (value.ReadUInt16() != 1)
                {
                    return null;
                }
                return value.ReadSerializedString();
            }
            catch (BadImageFormatException exception)
            {
                AddDiagnostic("warning", "CAPTURE_TARGET_FRAMEWORK", input.RelativePath, exception.Message);
                return null;
            }
        }
        return null;
    }

    private bool IsTracked(string assembly) => trackedPrefixes.Any(pattern =>
        pattern.EndsWith('*')
            ? assembly.StartsWith(pattern[..^1], StringComparison.Ordinal)
            : assembly.Equals(pattern, StringComparison.Ordinal));

    private static bool IsFormsAssembly(string assembly) =>
        assembly.Equals("System.Windows.Forms", StringComparison.Ordinal) ||
        assembly.Equals("System.Windows.Forms.Primitives", StringComparison.Ordinal);

    private static string PrivateTypeId(string assembly, string type)
    {
        return TypeNameRedaction.PrivateTypeId(assembly, type);
    }

    private static string GetPublicKeyToken(byte[] publicKeyOrToken, AssemblyFlags flags)
    {
        if (publicKeyOrToken.Length == 0)
        {
            return "null";
        }
        if ((flags & AssemblyFlags.PublicKey) == 0)
        {
            return Convert.ToHexStringLower(publicKeyOrToken);
        }
        var hash = SHA1.HashData(publicKeyOrToken);
        var token = hash.AsSpan(hash.Length - 8, 8).ToArray();
        Array.Reverse(token);
        return Convert.ToHexStringLower(token);
    }

    private static string FormatAssemblyReference(MetadataReader reader, AssemblyReference reference)
    {
        var name = reader.GetString(reference.Name);
        var culture = reference.Culture.IsNil ? "neutral" : reader.GetString(reference.Culture);
        var token = reader.GetBlobBytes(reference.PublicKeyOrToken);
        return $"{name}, Version={reference.Version}, Culture={culture}, PublicKeyToken=" +
               (token.Length == 0 ? "null" : Convert.ToHexStringLower(token));
    }

    private static string OpcodeOperation(ushort value) => value switch
    {
        0x27 => "jmp",
        0x28 => "call",
        0x6f => "callvirt",
        0x73 => "newobj",
        0x7b => "ldfld",
        0x7c => "ldflda",
        0x7d => "stfld",
        0x7e => "ldsfld",
        0x7f => "ldsflda",
        0x80 => "stsfld",
        0xd0 => "ldtoken",
        0xfe06 => "ldftn",
        0xfe07 => "ldvirtftn",
        _ => "type_operand",
    };

    private void AddDiagnostic(string severity, string code, string subject, string message) =>
        diagnostics.Add(new DiagnosticRecord
        {
            Severity = severity,
            Code = code,
            Subject = subject,
            Message = message,
        });

    private static void RequireBytes(ReadOnlySpan<byte> bytes, int offset, int count)
    {
        if (offset < 0 || count < 0 || offset > bytes.Length - count)
        {
            throw new BadImageFormatException("Truncated IL operand.");
        }
    }

    private readonly record struct ApiKey(
        string Assembly, string Type, string? Member, string? Signature, string Kind);
    private readonly record struct NativeImportKey(
        string SourceAssembly, string Module, string EntryPoint, string Attributes);
    private readonly record struct TypeKey(string Assembly, string FullName);
    private readonly record struct TypeNode(string Assembly, string FullName, TypeIdentity? BaseType);
    private readonly record struct ResolvedMember(
        TypeIdentity Owner, string Name, string Signature, string Kind);
}

internal sealed class TypeNameProvider : ISignatureTypeProvider<TypeIdentity, object?>
{
    private readonly ManagedInput input;
    private readonly bool redactPrivateTypeNames;

    public TypeNameProvider(ManagedInput input, bool redactPrivateTypeNames)
    {
        this.input = input;
        this.redactPrivateTypeNames = redactPrivateTypeNames;
    }

    public TypeIdentity GetArrayType(TypeIdentity elementType, ArrayShape shape) =>
        elementType with { FullName = elementType.FullName + "[" + new string(',', Math.Max(0, shape.Rank - 1)) + "]" };

    public TypeIdentity GetByReferenceType(TypeIdentity elementType) =>
        elementType with { FullName = elementType.FullName + "&" };

    public TypeIdentity GetFunctionPointerType(MethodSignature<TypeIdentity> signature) =>
        TypeIdentity.Unknown("methodptr(" + string.Join(",", signature.ParameterTypes.Select(type => type.FullName)) + ")");

    public TypeIdentity GetGenericInstantiation(TypeIdentity genericType, ImmutableArray<TypeIdentity> typeArguments) =>
        genericType with
        {
            FullName = genericType.FullName + "<" +
                       string.Join(",", typeArguments.Select(type => type.FullName)) + ">",
        };

    public TypeIdentity GetGenericMethodParameter(object? genericContext, int index) =>
        TypeIdentity.Unknown("!!" + index);

    public TypeIdentity GetGenericTypeParameter(object? genericContext, int index) =>
        TypeIdentity.Unknown("!" + index);

    public TypeIdentity GetModifiedType(TypeIdentity modifier, TypeIdentity unmodifiedType, bool isRequired) =>
        unmodifiedType;

    public TypeIdentity GetPinnedType(TypeIdentity elementType) =>
        elementType with { FullName = elementType.FullName + " pinned" };

    public TypeIdentity GetPointerType(TypeIdentity elementType) =>
        elementType with { FullName = elementType.FullName + "*" };

    public TypeIdentity GetPrimitiveType(PrimitiveTypeCode typeCode) =>
        new("System.Private.CoreLib", typeCode switch
        {
            PrimitiveTypeCode.Boolean => "System.Boolean",
            PrimitiveTypeCode.Byte => "System.Byte",
            PrimitiveTypeCode.SByte => "System.SByte",
            PrimitiveTypeCode.Char => "System.Char",
            PrimitiveTypeCode.Int16 => "System.Int16",
            PrimitiveTypeCode.UInt16 => "System.UInt16",
            PrimitiveTypeCode.Int32 => "System.Int32",
            PrimitiveTypeCode.UInt32 => "System.UInt32",
            PrimitiveTypeCode.Int64 => "System.Int64",
            PrimitiveTypeCode.UInt64 => "System.UInt64",
            PrimitiveTypeCode.Single => "System.Single",
            PrimitiveTypeCode.Double => "System.Double",
            PrimitiveTypeCode.IntPtr => "System.IntPtr",
            PrimitiveTypeCode.UIntPtr => "System.UIntPtr",
            PrimitiveTypeCode.Object => "System.Object",
            PrimitiveTypeCode.String => "System.String",
            PrimitiveTypeCode.TypedReference => "System.TypedReference",
            PrimitiveTypeCode.Void => "System.Void",
            _ => typeCode.ToString(),
        });

    public TypeIdentity GetSZArrayType(TypeIdentity elementType) =>
        elementType with { FullName = elementType.FullName + "[]" };

    public TypeIdentity GetTypeFromDefinition(
        MetadataReader reader, TypeDefinitionHandle handle, byte rawTypeKind)
    {
        return MaybeRedact(GetTypeFromDefinitionCore(reader, handle));
    }

    private TypeIdentity GetTypeFromDefinitionCore(
        MetadataReader reader, TypeDefinitionHandle handle)
    {
        var definition = reader.GetTypeDefinition(handle);
        var name = reader.GetString(definition.Name);
        var declaring = definition.GetDeclaringType();
        if (!declaring.IsNil)
        {
            var parent = GetTypeFromDefinitionCore(reader, declaring);
            return new TypeIdentity(input.AssemblyName, parent.FullName + "+" + name);
        }
        var ns = reader.GetString(definition.Namespace);
        return new TypeIdentity(input.AssemblyName, Qualify(ns, name));
    }

    public TypeIdentity GetTypeFromReference(
        MetadataReader reader, TypeReferenceHandle handle, byte rawTypeKind)
    {
        return MaybeRedact(GetTypeFromReferenceCore(reader, handle, rawTypeKind));
    }

    private TypeIdentity GetTypeFromReferenceCore(
        MetadataReader reader, TypeReferenceHandle handle, byte rawTypeKind)
    {
        var reference = reader.GetTypeReference(handle);
        var name = reader.GetString(reference.Name);
        var ns = reader.GetString(reference.Namespace);
        return reference.ResolutionScope.Kind switch
        {
            HandleKind.AssemblyReference => new TypeIdentity(
                reader.GetString(reader.GetAssemblyReference(
                    (AssemblyReferenceHandle)reference.ResolutionScope).Name), Qualify(ns, name)),
            HandleKind.TypeReference => Nest(
                GetTypeFromReferenceCore(reader, (TypeReferenceHandle)reference.ResolutionScope, rawTypeKind), name),
            HandleKind.ModuleDefinition or HandleKind.ModuleReference =>
                new TypeIdentity(input.AssemblyName, Qualify(ns, name)),
            _ => TypeIdentity.Unknown(Qualify(ns, name)),
        };
    }

    public TypeIdentity GetTypeFromSpecification(
        MetadataReader reader, object? genericContext, TypeSpecificationHandle handle, byte rawTypeKind) =>
        reader.GetTypeSpecification(handle).DecodeSignature(this, genericContext);

    public TypeIdentity? ResolveTypeHandle(EntityHandle handle) => handle.Kind switch
    {
        HandleKind.TypeDefinition => GetTypeFromDefinition(input.Reader, (TypeDefinitionHandle)handle, 0),
        HandleKind.TypeReference => GetTypeFromReference(input.Reader, (TypeReferenceHandle)handle, 0),
        HandleKind.TypeSpecification => GetTypeFromSpecification(input.Reader, null,
            (TypeSpecificationHandle)handle, 0),
        _ => null,
    };

    private static TypeIdentity Nest(TypeIdentity parent, string name) =>
        parent with { FullName = parent.FullName + "+" + name };

    private TypeIdentity MaybeRedact(TypeIdentity identity)
    {
        if (!redactPrivateTypeNames || TypeNameRedaction.IsFrameworkAssembly(identity.Assembly))
        {
            return identity;
        }
        return identity with
        {
            FullName = "<private:" + TypeNameRedaction.PrivateTypeId(identity.Assembly, identity.FullName) + ">",
        };
    }

    private static string Qualify(string ns, string name) =>
        string.IsNullOrEmpty(ns) ? name : ns + "." + name;
}

internal static class TypeNameRedaction
{
    public static string PrivateTypeId(string assembly, string type)
    {
        var bytes = System.Text.Encoding.UTF8.GetBytes($"{assembly}\0{type}");
        return "type-" + Convert.ToHexStringLower(SHA256.HashData(bytes).AsSpan(0, 12));
    }

    public static bool IsFrameworkAssembly(string assembly) =>
        assembly is "Accessibility" or "mscorlib" or "netstandard" or "WindowsBase" ||
        assembly.StartsWith("System.", StringComparison.Ordinal) ||
        assembly.StartsWith("Microsoft.", StringComparison.Ordinal);
}

internal static class IlOpcodeTable
{
    public static IReadOnlyDictionary<ushort, OperandType> OperandTypes { get; } = Build();

    private static IReadOnlyDictionary<ushort, OperandType> Build()
    {
        var result = new Dictionary<ushort, OperandType>();
        foreach (var field in typeof(OpCodes).GetFields(BindingFlags.Public | BindingFlags.Static))
        {
            if (field.GetValue(null) is OpCode opcode)
            {
                result[unchecked((ushort)opcode.Value)] = opcode.OperandType;
            }
        }
        return result;
    }
}
