using System.Reflection;
using System.Text.Json;

if (args.Length != 5)
{
    Console.Error.WriteLine("usage: facade-verifier <catalogue> <windows-ref-dir> <forms.dll> <primitives.dll> <drawing.dll>");
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
    if (!candidates.Any(member => Signature(member) == signature)) missing.Add($"{assemblyName}:{typeName}.{memberName} {signature}");
}

Console.WriteLine($"surface-verify: checked={checkedRows} missing={missing.Count}");
foreach (var item in missing.Take(20)) Console.Error.WriteLine(item);
return missing.Count == 0 ? 0 : 1;

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
