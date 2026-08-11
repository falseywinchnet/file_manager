#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import subprocess
import sys


WEB_FORMS_ROOT = Path(__file__).resolve().parents[1]
REPOSITORY_ROOT = WEB_FORMS_ROOT.parent
sys.path.insert(0, str(WEB_FORMS_ROOT / "src"))

from web_forms_compiler.capabilities import read_capabilities
from web_forms_compiler.gui_tree_stage2 import generate_gui_tree
from web_forms_compiler.stage1 import compile_source


def main() -> int:
    gui_forms_build = Path(sys.argv[1]) if len(sys.argv) > 1 else (
        REPOSITORY_ROOT / "gui_forms/build"
    )
    output = WEB_FORMS_ROOT / ".build/native_tree_probe"
    output.mkdir(parents=True, exist_ok=True)
    unit = "standard_shell_sapphire"
    generated = generate_gui_tree(
        compile_source(
            WEB_FORMS_ROOT / "boards/apps/standard_shell/standard_shell.wf.html"
        ),
        read_capabilities(
            WEB_FORMS_ROOT / "capabilities/gui_forms_observed_001.json"
        ),
        output,
        unit,
    )
    sources = [
        output / f"{unit}.gui_materials.wf.cpp",
        output / f"{unit}.gui_layouts.wf.cpp",
        output / f"{unit}.gui_typography.wf.cpp",
        output / f"{unit}.gui_decorations.wf.cpp",
        generated[1],
        WEB_FORMS_ROOT / "tests/native_tree_probe.cpp",
    ]
    executable = output / "native_tree_probe"
    command = [
        "/usr/bin/c++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
        "-I", str(REPOSITORY_ROOT / "gui_forms/include"),
        "-I", str(output),
    ]
    command.extend(str(source) for source in sources)
    command.extend(
        [
            str(gui_forms_build / "libgui_forms_controls.a"),
            str(gui_forms_build / "libgui_forms_core.a"),
            str(gui_forms_build / "libgui_drawing_core.a"),
            "-o", str(executable),
        ]
    )
    result = subprocess.run(command, check=False)
    if result.returncode != 0:
        return result.returncode
    return subprocess.run([str(executable)], check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
