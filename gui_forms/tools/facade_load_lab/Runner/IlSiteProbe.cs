using System.Buffers.Binary;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.Loader;

namespace GuiForms.CompatCapture;

internal static class IlSiteProbe
{
    private static readonly IReadOnlyDictionary<ushort, OpCode> Opcodes = BuildOpcodes();

    internal static void Write(AssemblyLoadContext context, string extractDirectory,
                               string specification)
    {
        var parts = specification.Split('|')
            .Select(global::System.Uri.UnescapeDataString).ToArray();
        if ((parts.Length != 3 && parts.Length != 4) || parts.Any(string.IsNullOrWhiteSpace))
            throw new ArgumentException(
                "GUI_FORMS_LOAD_PROBE_METHOD must be assembly|type|method[|parameter-count].");
        var path = Path.Combine(extractDirectory, parts[0] + ".dll");
        var assembly = context.LoadFromAssemblyPath(path);
        var type = assembly.GetType(parts[1], throwOnError: true)!;
        Console.WriteLine($"il-type={type.FullName}|base:{type.BaseType?.FullName}|interfaces:{string.Join(',', type.GetInterfaces().Select(value => value.FullName).OrderBy(value => value, StringComparer.Ordinal))}");
        const BindingFlags flags = BindingFlags.Instance | BindingFlags.Static |
                                   BindingFlags.Public | BindingFlags.NonPublic |
                                   BindingFlags.DeclaredOnly;
        var selector = parts.Length == 4 ? parts[3] : null;
        var parameterCount = int.TryParse(selector, out var parsedCount) ? parsedCount : (int?)null;
        var candidates = parts[2] == ".ctor"
            ? type.GetConstructors(flags).Where(candidate => !candidate.IsStatic).Cast<MethodBase>()
            : type.GetMethods(flags).Where(candidate => candidate.Name == parts[2]).Cast<MethodBase>();
        if (parameterCount.HasValue)
            candidates = candidates.Where(candidate => candidate.GetParameters().Length == parameterCount.Value);
        else if (selector is not null)
        {
            var names = selector.Split(',', StringSplitOptions.TrimEntries);
            candidates = candidates.Where(candidate => candidate.GetParameters().Select(parameter =>
                    parameter.ParameterType.FullName ?? parameter.ParameterType.Name)
                .SequenceEqual(names, StringComparer.Ordinal));
        }
        var matches = candidates.OrderBy(candidate => candidate.MetadataToken).ToArray();
        if (matches.Length != 1)
        {
            Console.WriteLine($"il-candidates={string.Join(';', matches.Select(FormatMethod))}");
            throw new InvalidOperationException($"Selected method matched {matches.Length} candidates.");
        }
        var method = matches[0];
        var bytes = method.GetMethodBody()?.GetILAsByteArray() ??
            throw new InvalidOperationException("Selected method has no managed IL body.");
        Console.WriteLine($"il-probe={parts[0]}|{parts[1]}|{parts[2]}|bytes:{bytes.Length}");
        var offset = 0;
        while (offset < bytes.Length)
        {
            var start = offset;
            ushort value = bytes[offset++];
            if (value == 0xfe) value = (ushort)(0xfe00 | bytes[offset++]);
            if (!Opcodes.TryGetValue(value, out var opcode))
                throw new BadImageFormatException($"Unknown IL opcode 0x{value:x4} at {start}.");
            var operand = ReadOperand(method, bytes, ref offset, opcode.OperandType);
            Console.WriteLine($"il_{start:x4}={opcode.Name}{(operand.Length == 0 ? string.Empty : "|" + operand)}");
        }
    }

    private static string ReadOperand(MethodBase method, byte[] bytes, ref int offset,
                                      OperandType operandType)
    {
        var span = bytes.AsSpan();
        switch (operandType)
        {
            case OperandType.InlineNone:
                return string.Empty;
            case OperandType.ShortInlineI:
                return unchecked((sbyte)bytes[offset++]).ToString();
            case OperandType.InlineI:
                var intValue = BinaryPrimitives.ReadInt32LittleEndian(span.Slice(offset, 4));
                offset += 4;
                return intValue.ToString();
            case OperandType.InlineI8:
                var longValue = BinaryPrimitives.ReadInt64LittleEndian(span.Slice(offset, 8));
                offset += 8;
                return longValue.ToString();
            case OperandType.ShortInlineR:
                var floatBits = BinaryPrimitives.ReadInt32LittleEndian(span.Slice(offset, 4));
                offset += 4;
                return BitConverter.Int32BitsToSingle(floatBits).ToString("R");
            case OperandType.InlineR:
                var doubleBits = BinaryPrimitives.ReadInt64LittleEndian(span.Slice(offset, 8));
                offset += 8;
                return BitConverter.Int64BitsToDouble(doubleBits).ToString("R");
            case OperandType.ShortInlineVar:
                return bytes[offset++].ToString();
            case OperandType.InlineVar:
                var variable = BinaryPrimitives.ReadUInt16LittleEndian(span.Slice(offset, 2));
                offset += 2;
                return variable.ToString();
            case OperandType.ShortInlineBrTarget:
                var shortDelta = unchecked((sbyte)bytes[offset++]);
                return $"il_{offset + shortDelta:x4}";
            case OperandType.InlineBrTarget:
                var delta = BinaryPrimitives.ReadInt32LittleEndian(span.Slice(offset, 4));
                offset += 4;
                return $"il_{offset + delta:x4}";
            case OperandType.InlineSwitch:
                var count = BinaryPrimitives.ReadInt32LittleEndian(span.Slice(offset, 4));
                offset += 4;
                var baseOffset = offset + checked(count * 4);
                var targets = new string[count];
                for (var index = 0; index < count; ++index)
                {
                    var branch = BinaryPrimitives.ReadInt32LittleEndian(span.Slice(offset, 4));
                    offset += 4;
                    targets[index] = $"il_{baseOffset + branch:x4}";
                }
                return string.Join(',', targets);
            case OperandType.InlineString:
                var stringToken = BinaryPrimitives.ReadInt32LittleEndian(
                    span.Slice(offset, 4));
                offset += 4;
                try
                {
                    return method.Module.ResolveString(stringToken)
                        .Replace("\\", "\\\\", StringComparison.Ordinal)
                        .Replace("\r", "\\r", StringComparison.Ordinal)
                        .Replace("\n", "\\n", StringComparison.Ordinal);
                }
                catch (ArgumentException)
                {
                    return $"string-token:0x{stringToken:x8}";
                }
            case OperandType.InlineSig:
                offset += 4;
                return "<signature>";
            case OperandType.InlineField:
            case OperandType.InlineMethod:
            case OperandType.InlineTok:
            case OperandType.InlineType:
                var token = BinaryPrimitives.ReadInt32LittleEndian(span.Slice(offset, 4));
                offset += 4;
                try
                {
                    var member = method.Module.ResolveMember(token,
                        method.DeclaringType?.GetGenericArguments(),
                        method.IsGenericMethod ? method.GetGenericArguments() : null);
                    return member is null ? $"token:0x{token:x8}" : Format(member);
                }
                catch (Exception error) when (error is ArgumentException or BadImageFormatException)
                {
                    return $"token:0x{token:x8}";
                }
            default:
                throw new NotSupportedException(operandType.ToString());
        }
    }

    private static string Format(MemberInfo member)
    {
        var owner = member.DeclaringType?.FullName;
        return owner is null ? member.Name : owner + "." + member.Name;
    }

    private static string FormatMethod(MethodBase method) =>
        $"{method.Name}({string.Join(',', method.GetParameters().Select(parameter => parameter.ParameterType.FullName))})";

    private static IReadOnlyDictionary<ushort, OpCode> BuildOpcodes()
    {
        var result = new Dictionary<ushort, OpCode>();
        foreach (var field in typeof(OpCodes).GetFields(BindingFlags.Public | BindingFlags.Static))
            if (field.GetValue(null) is OpCode opcode)
                result[unchecked((ushort)opcode.Value)] = opcode;
        return result;
    }
}
