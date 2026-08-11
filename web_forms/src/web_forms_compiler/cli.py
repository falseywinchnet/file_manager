from __future__ import annotations

import argparse
from pathlib import Path
import sys

from .diagnostics import WebFormsError
from .capabilities import assess_capabilities, read_capabilities
import json
from .stage1 import compile_source, read_ir, write_ir
from .stage2 import generate_cpp
from .gui_material_stage2 import generate_gui_materials
from .gui_layout_stage2 import generate_gui_layouts
from .gui_typography_stage2 import generate_gui_typography
from .gui_decoration_stage2 import generate_gui_decorations
from .gui_tree_stage2 import generate_gui_tree
from .profile import profile_manifest


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="webforms")
    subparsers = parser.add_subparsers(dest="command", required=True)

    check = subparsers.add_parser("check", help="Stage 1: validate HTML/CSS and optionally emit IR")
    check.add_argument("html", type=Path)
    check.add_argument("--style", action="append", type=Path, default=[])
    check.add_argument("--emit-ir", type=Path)
    check.add_argument("--quiet", action="store_true")

    generate = subparsers.add_parser("generate", help="Stage 2: generate C++ from accepted IR")
    generate.add_argument("ir", type=Path)
    generate.add_argument("--output-dir", required=True, type=Path)
    generate.add_argument("--unit")

    build = subparsers.add_parser("build", help="Run Stage 1 then Stage 2")
    build.add_argument("html", type=Path)
    build.add_argument("--style", action="append", type=Path, default=[])
    build.add_argument("--emit-ir", required=True, type=Path)
    build.add_argument("--output-dir", required=True, type=Path)
    build.add_argument("--unit")

    report = subparsers.add_parser("report", help="Assess accepted IR against a GUI.Forms capability manifest")
    report.add_argument("ir", type=Path)
    report.add_argument("--manifest", required=True, type=Path)
    report.add_argument("--output", type=Path)

    materials = subparsers.add_parser(
        "generate-gui-materials",
        help="Experimental Stage 2: project exact surface materials into GUI.Forms",
    )
    materials.add_argument("ir", type=Path)
    materials.add_argument("--manifest", required=True, type=Path)
    materials.add_argument("--output-dir", required=True, type=Path)
    materials.add_argument("--unit")

    layouts = subparsers.add_parser(
        "generate-gui-layouts",
        help="Experimental Stage 2: project relational flex/grid and bounded box layout into GUI.Forms",
    )
    layouts.add_argument("ir", type=Path)
    layouts.add_argument("--manifest", required=True, type=Path)
    layouts.add_argument("--output-dir", required=True, type=Path)
    layouts.add_argument("--unit")

    typography = subparsers.add_parser(
        "generate-gui-typography",
        help="Experimental Stage 2: project retained typography into GUI.Forms",
    )
    typography.add_argument("ir", type=Path)
    typography.add_argument("--manifest", required=True, type=Path)
    typography.add_argument("--output-dir", required=True, type=Path)
    typography.add_argument("--unit")

    decorations = subparsers.add_parser(
        "generate-gui-decorations",
        help="Experimental Stage 2: project owned pseudo-decoration into GUI.Forms",
    )
    decorations.add_argument("ir", type=Path)
    decorations.add_argument("--manifest", required=True, type=Path)
    decorations.add_argument("--output-dir", required=True, type=Path)
    decorations.add_argument("--unit")

    tree = subparsers.add_parser(
        "generate-gui-tree",
        help="Experimental Stage 2: construct a complete typed retained GUI.Forms tree",
    )
    tree.add_argument("ir", type=Path)
    tree.add_argument("--manifest", required=True, type=Path)
    tree.add_argument("--output-dir", required=True, type=Path)
    tree.add_argument("--unit")

    profile = subparsers.add_parser("profile", help="Emit the machine-readable Stage 1 source profile")
    profile.add_argument("--output", type=Path)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        if args.command == "check":
            ir = compile_source(args.html, args.style or None)
            if args.emit_ir is not None:
                write_ir(ir, args.emit_ir)
            if not args.quiet:
                measurements = ir["measurements"]
                print(
                    f"accepted {args.html}: {measurements['runtime_nodes']} runtime nodes, "
                    f"{measurements['style_rules']} rules, {measurements['state_variants']} variants, "
                    f"{measurements['decorations']} decorations"
                )
            return 0
        if args.command == "generate":
            paths = generate_cpp(read_ir(args.ir), args.output_dir, args.unit)
            for path in paths:
                print(path)
            return 0
        if args.command == "report":
            assessment = assess_capabilities(read_ir(args.ir), read_capabilities(args.manifest))
            rendered = json.dumps(assessment, indent=2, sort_keys=True) + "\n"
            if args.output is None:
                print(rendered, end="")
            else:
                args.output.parent.mkdir(parents=True, exist_ok=True)
                args.output.write_text(rendered, encoding="utf-8")
                print(args.output)
            return 0
        if args.command == "generate-gui-materials":
            paths = generate_gui_materials(
                read_ir(args.ir),
                read_capabilities(args.manifest),
                args.output_dir,
                args.unit,
            )
            for path in paths:
                print(path)
            return 0
        if args.command == "generate-gui-layouts":
            paths = generate_gui_layouts(
                read_ir(args.ir),
                read_capabilities(args.manifest),
                args.output_dir,
                args.unit,
            )
            for path in paths:
                print(path)
            return 0
        if args.command == "generate-gui-typography":
            paths = generate_gui_typography(
                read_ir(args.ir),
                read_capabilities(args.manifest),
                args.output_dir,
                args.unit,
            )
            for path in paths:
                print(path)
            return 0
        if args.command == "generate-gui-decorations":
            paths = generate_gui_decorations(
                read_ir(args.ir),
                read_capabilities(args.manifest),
                args.output_dir,
                args.unit,
            )
            for path in paths:
                print(path)
            return 0
        if args.command == "generate-gui-tree":
            paths = generate_gui_tree(
                read_ir(args.ir),
                read_capabilities(args.manifest),
                args.output_dir,
                args.unit,
            )
            for path in paths:
                print(path)
            return 0
        if args.command == "profile":
            rendered = json.dumps(profile_manifest(), indent=2, sort_keys=True) + "\n"
            if args.output is None:
                print(rendered, end="")
            else:
                args.output.parent.mkdir(parents=True, exist_ok=True)
                args.output.write_text(rendered, encoding="utf-8")
                print(args.output)
            return 0
        ir = compile_source(args.html, args.style or None)
        write_ir(ir, args.emit_ir)
        paths = generate_cpp(ir, args.output_dir, args.unit)
        print(args.emit_ir)
        for path in paths:
            print(path)
        return 0
    except WebFormsError as error:
        for diagnostic in error.diagnostics:
            print(diagnostic.render(), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
