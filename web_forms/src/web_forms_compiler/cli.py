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
from .fidelity import (
    FidelityTolerance,
    capture_browser_snapshot,
    compare_fidelity_snapshots,
    normalize_native_snapshot,
    read_fidelity_snapshot,
    write_json,
)


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

    browser_fidelity = subparsers.add_parser(
        "capture-browser-fidelity",
        help="Capture stable-ID geometry, typography, material, and state from Chromium",
    )
    browser_fidelity.add_argument("html", type=Path)
    browser_fidelity.add_argument("--style", action="append", type=Path, default=[])
    browser_fidelity.add_argument("--root-id")
    browser_fidelity.add_argument("--state", default="reference")
    browser_fidelity.add_argument("--viewport", default="1080x720")
    browser_fidelity.add_argument("--device-scale", type=float, default=1.0)
    browser_fidelity.add_argument("--text-scale", type=float, default=1.0)
    browser_fidelity.add_argument("--inactive", action="store_true")
    browser_fidelity.add_argument(
        "--interaction",
        choices=("reference", "hover", "pressed", "focused", "disabled"),
        default="reference",
    )
    browser_fidelity.add_argument("--interaction-target", default="")
    browser_fidelity.add_argument("--reduced-motion", action="store_true")
    browser_fidelity.add_argument("--chrome", type=Path)
    browser_fidelity.add_argument("--output", required=True, type=Path)

    native_fidelity = subparsers.add_parser(
        "normalize-native-fidelity",
        help="Normalize a GUI.Forms visual-inspection snapshot into the fidelity schema",
    )
    native_fidelity.add_argument("snapshot", type=Path)
    native_fidelity.add_argument("--projection", type=Path)
    native_fidelity.add_argument("--root-id")
    native_fidelity.add_argument("--source-digest", default="")
    native_fidelity.add_argument("--state", default="reference")
    native_fidelity.add_argument("--output", required=True, type=Path)

    compare_fidelity = subparsers.add_parser(
        "compare-fidelity",
        help="Compare browser and GUI.Forms fidelity snapshots by stable ID",
    )
    compare_fidelity.add_argument("browser", type=Path)
    compare_fidelity.add_argument("native", type=Path)
    compare_fidelity.add_argument("--geometry-tolerance", type=float, default=0.51)
    compare_fidelity.add_argument("--clip-tolerance", type=float, default=0.51)
    compare_fidelity.add_argument("--baseline-tolerance", type=float, default=1.0)
    compare_fidelity.add_argument("--raster-color-tolerance", type=int, default=120)
    compare_fidelity.add_argument("--output", required=True, type=Path)
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
        if args.command == "capture-browser-fidelity":
            viewport_match = __import__("re").fullmatch(r"([1-9][0-9]*)x([1-9][0-9]*)", args.viewport)
            if viewport_match is None:
                from .diagnostics import fail
                fail("WFV017", "viewport must use WIDTHxHEIGHT", str(args.html))
            snapshot = capture_browser_snapshot(
                args.html,
                styles=args.style or None,
                root_id=args.root_id,
                state=args.state,
                viewport=(int(viewport_match.group(1)), int(viewport_match.group(2))),
                device_scale=args.device_scale,
                text_scale=args.text_scale,
                window_active=not args.inactive,
                interaction=args.interaction,
                interaction_target=args.interaction_target,
                reduced_motion=args.reduced_motion,
                chrome=args.chrome,
            )
            write_json(snapshot, args.output)
            print(args.output)
            return 0
        if args.command == "normalize-native-fidelity":
            native = json.loads(args.snapshot.read_text(encoding="utf-8"))
            projection = (
                json.loads(args.projection.read_text(encoding="utf-8"))
                if args.projection is not None
                else None
            )
            snapshot = normalize_native_snapshot(
                native,
                projection=projection,
                root_id=args.root_id,
                source_digest=args.source_digest,
                state=args.state,
            )
            write_json(snapshot, args.output)
            print(args.output)
            return 0
        if args.command == "compare-fidelity":
            report = compare_fidelity_snapshots(
                read_fidelity_snapshot(args.browser),
                read_fidelity_snapshot(args.native),
                FidelityTolerance(
                    geometry=args.geometry_tolerance,
                    clip=args.clip_tolerance,
                    baseline=args.baseline_tolerance,
                    raster_color_distance=args.raster_color_tolerance,
                ),
            )
            write_json(report, args.output)
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
