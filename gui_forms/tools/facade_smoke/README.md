# ABI-backed facade smoke

This small managed program compiles against the generated replacement
`System.Windows.Forms` assemblies. It proves that representative control
construction, UTF-8 name/text, visibility/enabled state, bounds, visual
parenting, managed state events, and deterministic subtree disposal cross ABI
0.2 into the native retained core.
