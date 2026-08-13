#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import sys


WEB_FORMS_ROOT = Path(__file__).resolve().parents[1]
REPOSITORY_ROOT = WEB_FORMS_ROOT.parent
sys.path.insert(0, str(WEB_FORMS_ROOT / "src"))

from web_forms_compiler.capabilities import read_capabilities
from web_forms_compiler.gui_tree_stage2 import generate_gui_tree
from web_forms_compiler.stage1 import compile_source
from web_forms_compiler.stage2 import _symbol


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("--style", action="append", type=Path, default=[])
    parser.add_argument("--unit", required=True)
    parser.add_argument("--gui-forms-build", type=Path, default=REPOSITORY_ROOT / "gui_forms/build")
    args = parser.parse_args()

    output = WEB_FORMS_ROOT / ".build/native_fidelity_probe" / _symbol(args.unit)
    output.mkdir(parents=True, exist_ok=True)
    unit = _symbol(args.unit)
    generated = generate_gui_tree(
        compile_source(args.source, args.style or None),
        read_capabilities(WEB_FORMS_ROOT / "capabilities/gui_forms_observed_002.json"),
        output,
        unit,
    )
    sources = [
        output / f"{unit}.gui_materials.wf.cpp",
        output / f"{unit}.gui_layouts.wf.cpp",
        output / f"{unit}.gui_typography.wf.cpp",
        output / f"{unit}.gui_decorations.wf.cpp",
        generated[1],
        WEB_FORMS_ROOT / "tests/native_fidelity_probe.cpp",
    ]
    executable = output / "native_fidelity_probe"
    build = args.gui_forms_build
    command = [
        "/usr/bin/c++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
        f'-DWEB_FORMS_GENERATED_HEADER="{unit}.gui_tree.wf.hpp"',
        f"-DWEB_FORMS_GENERATED_NAMESPACE=web_forms_generated_{unit}",
        "-I", str(REPOSITORY_ROOT / "gui_forms/include"),
        "-I", str(REPOSITORY_ROOT / "gui_forms/src"),
        "-I", str(output),
    ]
    command.extend(str(source) for source in sources)
    command.extend(
        [
            str(build / "libgui_forms_controls.a"),
            str(build / "libgui_drawing_core.a"),
            str(build / "libgui_forms_skia.a"),
            str(build / "skia-cpu-release/libskia.a"),
            str(build / "skia-cpu-release/libskcms.a"),
            str(build / "skia-cpu-release/libpng.a"),
            str(build / "skia-cpu-release/libzlib.a"),
            str(build / "libgui_forms_text_engine.a"),
            str(build / "libgui_forms_core.a"),
            str(build / "third_party/harfbuzz/libharfbuzz.a"),
            str(build / "third_party/freetype/libfreetype.a"),
            "-framework", "CoreFoundation",
            "-framework", "CoreGraphics",
            "-framework", "CoreText",
            "-o", str(executable),
        ]
    )
    result = subprocess.run(command, check=False)
    if result.returncode == 0:
        print(executable)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
