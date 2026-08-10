# House-language enforcement specification

Status: **DECIDED design for the future implementation sibling; checker not yet
implemented**.

The semantic authority is a narrow standalone Clang LibTooling program. Cheap
text scans and tightened compiler warnings supplement it; neither substitutes
for source-aware AST classification.

## Tool boundary

- Proposed implementation home: `gui_forms/tools/house_policy_check/`.
- Development/build dependency only; it does not link into GUI.Forms products.
- Consume the exact build's `compile_commands.json`.
- Use a current Clang/LLVM toolchain in C++20 mode.
- Scan normative first-party production `include/` and `src/`.
- Exclude third-party, experiments, build products, and generated output unless
  a first-party generated production file is explicitly routed through its
  generator.
- Emit both ordinary diagnostics and stable JSON records containing path, line,
  column, enclosing symbol, construct kind, replacement category, and approved
  exception identifier.

## Required semantic checks

| Construct | AST/source treatment |
|---|---|
| `auto` and `decltype(auto)` | source-spelled deduced type locations; ignore compiler-synthesized closure internals while still reporting every source lambda |
| trailing return | inspect function-declaration token ranges and return-type source locations |
| pointer-member arrow | `MemberExpr` and dependent member expressions whose source spelling uses arrow; do not confuse trailing-return arrows |
| lambdas | every source `LambdaExpr`, including captureless and generic |
| structured bindings | every source `DecompositionDecl` |
| coroutines | coroutine body/await/yield/return source nodes |
| `std::any` | type locations and constructions, with only the named Tag exception below |
| defaulted comparisons | source-defaulted comparison/equality operators; explicit implementations remain |
| requires-expression | report the current detection use until it is replaced by the selected explicit trait; reject new uses without approval |
| `decltype(expression)` | permit only under a named private trait/detection declaration; reject ordinary declaration, return, and public-signature uses |
| ranges/views | report `std::ranges`/view types and operations for concrete architect review |
| designated initialization | classify target type; permit approved plain option/configuration records, reject stateful result alternatives after named factories exist |
| C++20-added features | one-time classified inventory; `consteval`, selected `if constexpr`/pack folding, `span`, and clear bit predicates follow their decided rules |

The `std::any` exception is symbol-bound, not directory-wide. It covers only:

- the existing Tag accessors/backing members of Control, ImageList,
  ErrorProvider, and HelpProvider;
- their disposal/reset paths;
- the production compatibility/dogfooding path necessary to implement Tag.

Supporting tests and facade shims are repaired after production and may use the
same exception only to prove that exact compatibility surface. New Tag-bearing
types are not implicitly exempt.

## Diagnostic modes

1. **Inventory:** scan the entire normative source and write all findings
   without failing the build.
2. **Ratchet:** a touched production file may not add findings and must close
   the batch's declared findings.
3. **Closure:** every unapproved normative production finding is fatal.

Do not encode sorting, `std::function`, allocator, ownership, or performance
judgment as token bans. Those remain classified audit/measurement decisions.

## Textual controls

Use `rg`-based checks as fast controls for obvious spellings such as `auto`,
lambda introducers, `std::any`, coroutine keywords, `std::ranges`, and
defaulted comparison tokens. Text results are orientation only: macros,
comments, Objective-C++ syntax, trailing-return arrows, and pointer arrows make
regex an unreliable authority.

## Warning ratchet

The current main CMake file selects C++20 but does not declare a general warning
profile. The sibling may tighten warnings incrementally, target by target.

Candidate Clang/GCC inventory set:

```text
-Wall -Wextra -Wpedantic
-Wshadow -Wconversion -Wsign-conversion
-Wold-style-cast -Wcast-align
-Wnon-virtual-dtor -Woverloaded-virtual
-Wimplicit-fallthrough -Wextra-semi -Wundef
```

Rules:

- inventory warning counts before making any warning fatal;
- keep C, C++, Objective-C++, MinGW, and generated/third-party warning surfaces
  separate;
- enable only warnings whose fixes preserve behavior and improve the selected
  house discipline;
- record every suppression at the narrowest target/source boundary with its
  reason;
- do not use warning cleanup to smuggle arithmetic, ABI, conversion, or
  platform behavior changes into a syntax batch;
- `-Werror` is a final per-target ratchet only after that target is clean and
  stable across the required toolchains.

Warnings may discover defects outside the rewrite. Record and route those
separately instead of silently broadening a batch.

## Checker verification

The checker itself requires fixtures proving positive and negative cases for
every construct, macro/source-location behavior, Objective-C++ handling,
generated/excluded paths, Tag exceptions, trait-only `decltype`, and explicit
comparison implementations. Test diagnostics and JSON records for stability.

Run the checker under the actual native and MinGW compilation databases before
closure. A result produced from an incomplete configuration is not a whole-tree
proof.

