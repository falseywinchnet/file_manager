# GUI.Forms house-policy checker

This development-only Clang LibTooling program is the semantic authority for
the explicit C++ house subset. It consumes the exact build compilation database
and emits a stable, deduplicated JSON ledger. It is not linked into any
GUI.Forms product.

Configure the main build with `CMAKE_EXPORT_COMPILE_COMMANDS=ON`, then build the
checker against a current LLVM installation:

```sh
/opt/homebrew/bin/cmake -S gui_forms/tools/house_policy_check \
  -B gui_forms/.build/house-policy-check \
  -DLLVM_DIR=/opt/homebrew/opt/llvm/lib/cmake/llvm \
  -DClang_DIR=/opt/homebrew/opt/llvm/lib/cmake/clang
/opt/homebrew/bin/cmake --build gui_forms/.build/house-policy-check --parallel
```

Run inventory mode over production translation units listed in the compilation
database. `include/` findings are discovered through those translation units
and deduplicated by source location and construct kind. The wrapper performs
that exact source selection without relying on whitespace-splitting filenames.
On macOS it also supplies Homebrew Clang's resource directory and the active
Xcode SDK explicitly; the recorded Apple Clang invocation does not convey those
implicit driver paths to a differently installed LibTooling build.

```sh
gui_forms/tools/house_policy_check/run_inventory.sh \
  "$PWD/gui_forms/build" \
  "$PWD/gui_forms_rewrite/HOUSE_POLICY_AUDIT.json"
```

Inventory mode records admitted constructs and violations without failing.
Closure mode fails if any finding still has `violation` disposition. The
symbol-bound `O-011-tag` exception is encoded only for the four decided Tag
owners and the named showcase and File Manager demoboard Tag dogfood functions.

An optional third wrapper argument writes a verified pointer-arrow rewrite
plan. The plan contains relative paths, captured file sizes, base-expression
offsets, and operator offsets from the AST. Apply it only to the same local
snapshot; the applier validates sizes and every `->` token before atomically
rewriting `base->member` as `(*base).member`. Multiple nested arrows are handled
as simultaneous insertions and non-overlapping token replacements.
