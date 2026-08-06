# GUI.Drawing metrics differential

This bounded M11g fixture emits the same JSON schema from the GUI.Drawing
compatibility facade and from the installed .NET 10 Windows `System.Drawing`
implementation. It covers glyph-sensitive widths, point-unit height, constrained
wrapping, every font family observed in the unchanged specimen, compound path
bounds/points/hit tests, affine translation, raster ink bounds, and four
exception/null cases. Reports are evidence; they are not golden files because
the oracle's installed font substitution is an environment input.

The facade project runs with the repository's native GUI.Drawing libraries. The
oracle project is built and run with Windows .NET 10 under Wine.
