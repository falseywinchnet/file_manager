using System.Reflection;
using System.Text.Json;

if (args.Length is not (5 or 6))
{
    Console.Error.WriteLine("usage: facade-verifier <catalogue> <windows-ref-dir> <forms.dll> <primitives.dll> <drawing.dll> [call-report-dir]");
    return 2;
}

var coreRoot = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(typeof(object).Assembly.Location)!, "..", "..", "..", "packs", "Microsoft.NETCore.App.Ref"));
var coreRef = Directory.GetDirectories(coreRoot).OrderByDescending(value => value, StringComparer.Ordinal)
    .Select(value => Path.Combine(value, "ref", "net10.0")).First(Directory.Exists);
var paths = Directory.GetFiles(coreRef, "*.dll")
    .Concat(Directory.GetFiles(Path.GetFullPath(args[1]), "*.dll").Where(path => Path.GetFileName(path) is not ("System.Windows.Forms.dll" or "System.Windows.Forms.Primitives.dll" or "System.Drawing.Common.dll")))
    .Concat([Path.GetFullPath(args[2]), Path.GetFullPath(args[3]), Path.GetFullPath(args[4])]).Distinct(StringComparer.Ordinal).ToArray();
using var context = new MetadataLoadContext(new PathAssemblyResolver(paths), "System.Runtime");
var assemblies = new Dictionary<string, Assembly>(StringComparer.Ordinal)
{
    ["System.Windows.Forms"] = context.LoadFromAssemblyPath(Path.GetFullPath(args[2])),
    ["System.Windows.Forms.Primitives"] = context.LoadFromAssemblyPath(Path.GetFullPath(args[3])),
    ["System.Drawing.Common"] = context.LoadFromAssemblyPath(Path.GetFullPath(args[4])),
    ["System.Drawing.Primitives"] = context.LoadFromAssemblyPath(Path.Combine(coreRef, "System.Drawing.Primitives.dll")),
};
using var document = JsonDocument.Parse(File.ReadAllBytes(args[0]));
var missing = new List<string>();
var instrumentableKeys = new HashSet<string>(StringComparer.Ordinal);
var checkedRows = 0;
foreach (var row in document.RootElement.GetProperty("apiRows").EnumerateArray())
{
    if (row.GetProperty("disposition").GetString() != "required" ||
        row.GetProperty("implementationOwner").GetString() is not
            ("gui_forms_managed_facade" or "gui_drawing_compat_facade")) continue;
    ++checkedRows;
    var assemblyName = row.GetProperty("targetAssembly").GetString()!;
    var typeName = row.GetProperty("type").GetString()!;
    var type = assemblies[assemblyName].GetType(typeName, false, false);
    if (type is null) { missing.Add($"{assemblyName}:{typeName}"); continue; }
    if (row.GetProperty("memberKind").GetString() == "type") continue;
    var memberName = row.GetProperty("member").GetString()!;
    var signature = row.GetProperty("signature").GetString()!;
    const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly;
    IEnumerable<MemberInfo> candidates = row.GetProperty("memberKind").GetString() == "field"
        ? type.GetFields(flags).Where(field => field.Name == memberName)
        : memberName == ".ctor" ? type.GetConstructors(flags) : type.GetMethods(flags).Where(method => method.Name == memberName);
    var member = candidates.FirstOrDefault(member => Signature(member) == signature);
    if (member is null)
    {
        missing.Add($"{assemblyName}:{typeName}.{memberName} {signature}");
        continue;
    }
    if (!type.IsInterface && type.Assembly.GetName().Name != "System.Drawing.Primitives" &&
        member is MethodBase callable &&
        (callable is ConstructorInfo || callable is MethodInfo method &&
         (!method.IsSpecialName || method.Name.StartsWith("get_", StringComparison.Ordinal) ||
          method.Name.StartsWith("set_", StringComparison.Ordinal))))
        instrumentableKeys.Add(CallKey(type, callable));
}

Console.WriteLine($"surface-verify: checked={checkedRows} missing={missing.Count}");
foreach (var item in missing.Take(20)) Console.Error.WriteLine(item);
var telemetryErrors = new List<string>();
if (args.Length == 6)
{
    var reportDirectory = Path.GetFullPath(args[5]);
    var counts = new Dictionary<string, long>(StringComparer.Ordinal);
    if (!Directory.Exists(reportDirectory))
        telemetryErrors.Add($"call report directory does not exist: {reportDirectory}");
    else
    {
        foreach (var path in Directory.GetFiles(reportDirectory, "*.calls.tsv").OrderBy(value => value, StringComparer.Ordinal))
        {
            var lines = File.ReadAllLines(path);
            if (lines.Length < 5 || lines[0] != "schema\tgui.forms.call-coverage/v1" ||
                lines[4] != "key\tcount")
            {
                telemetryErrors.Add($"invalid call report: {path}");
                continue;
            }
            foreach (var line in lines.Skip(5))
            {
                var separator = line.LastIndexOf('\t');
                if (separator <= 0 || !long.TryParse(line.AsSpan(separator + 1),
                    System.Globalization.NumberStyles.None,
                    System.Globalization.CultureInfo.InvariantCulture, out var count) || count < 0)
                {
                    telemetryErrors.Add($"invalid call row: {path}:{line}");
                    continue;
                }
                var key = line[..separator];
                counts.TryGetValue(key, out var prior);
                counts[key] = checked(prior + count);
            }
        }
    }
    var exercised = instrumentableKeys.Count(key => counts.GetValueOrDefault(key) > 0);
    var calls = instrumentableKeys.Sum(key => counts.GetValueOrDefault(key));
    var metricKeys = counts.Count(entry => entry.Value != 0 &&
        entry.Key.StartsWith("metric|", StringComparison.Ordinal));
    var extensions = counts.Where(entry => entry.Value > 0 &&
            !entry.Key.StartsWith("metric|", StringComparison.Ordinal) &&
            !instrumentableKeys.Contains(entry.Key))
        .OrderBy(entry => entry.Key, StringComparer.Ordinal).ToArray();
    Console.WriteLine($"call-coverage: required-rows={checkedRows} instrumentable-keys={instrumentableKeys.Count} exercised-keys={exercised} calls={calls} extension-keys={extensions.Length} metric-keys={metricKeys}");
    foreach (var item in extensions.Take(20))
        Console.WriteLine($"call-coverage-extension: key={item.Key} calls={item.Value}");
}
foreach (var error in telemetryErrors.Take(20)) Console.Error.WriteLine(error);
return missing.Count == 0 && telemetryErrors.Count == 0 ? 0 : 1;

static string CallKey(Type type, MethodBase member) =>
    type.Assembly.GetName().Name + "|" + type.FullName + "|" +
    (member is ConstructorInfo ? ".ctor" : member.Name) + "|" + Signature(member);

static string Signature(MemberInfo member) => member switch
{
    FieldInfo field => TypeName(field.FieldType),
    MethodBase method => (method is MethodInfo info ? TypeName(info.ReturnType) : "System.Void") + " " +
        (method.IsGenericMethodDefinition ? "``" + method.GetGenericArguments().Length : "") + "(" +
        string.Join(",", method.GetParameters().Select(parameter => TypeName(parameter.ParameterType))) + ")",
    _ => throw new NotSupportedException(),
};

static string TypeName(Type type)
{
    if (type.IsByRef) return TypeName(type.GetElementType()!) + "&";
    if (type.IsPointer) return TypeName(type.GetElementType()!) + "*";
    if (type.IsArray) return TypeName(type.GetElementType()!) + "[" + new string(',', type.GetArrayRank() - 1) + "]";
    if (type.IsGenericParameter) return (type.DeclaringMethod is null ? "!" : "!!") + type.GenericParameterPosition;
    if (type.IsGenericType) return type.GetGenericTypeDefinition().FullName! + "<" + string.Join(",", type.GetGenericArguments().Select(TypeName)) + ">";
    return type.FullName ?? type.Name;
}
