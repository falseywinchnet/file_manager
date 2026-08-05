# GUI.Forms managed facade loader laboratory

Status: **HYPOTHESIS test**, not the supported managed facade.

This opt-in .NET 10 laboratory tests one narrow question: can a consumer built
against Microsoft's strong-named `System.Windows.Forms` reference assembly be
loaded in a private `AssemblyLoadContext` while resolving that request to an
unsigned experimental assembly with the same simple name and version?

The synthetic facade contains only `Control`, its collection, `ButtonBase`,
`Button`, `ScrollableControl`, `ContainerControl`, `Form`, and `Application`.
It does not call the native GUI.Forms ABI and proves no visual or behavioral
compatibility. The consumer creates a form and button, exercises inheritance,
collection mutation, event subscription, activation, and deterministic
disposal through the replacement assembly.

The test runner supplies the installed Windows Desktop reference assembly by
path, builds all three assemblies on the host, and runs the loader with both
the host `dotnet` and x64 Windows `dotnet.exe` under Wine. No target retired compatibility specimen code is
loaded or executed.

**OBSERVED negative:** the first consumer build referenced only
`System.Windows.Forms.dll` and failed because the .NET 10 public Forms types
carry inherited COM/Windows interface contracts from
`System.Windows.Forms.Primitives` and `System.Private.Windows.Core`. The runner
now supplies that compile-time reference closure. CA1416 is disabled only for
this deliberately Windows-referenced synthetic consumer compiled on macOS.

**OBSERVED negative:** byte comparison across separate intermediate directories
initially failed because the portable-PDB identity changed the PE debug
directory. Release laboratory projects now omit debug symbols; independently
built facade, consumer, and loader DLLs must compare byte-for-byte.

Run:

```text
tests/run_compat_loader_lab_tests.sh
```
