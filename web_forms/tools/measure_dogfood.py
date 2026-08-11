#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))

from web_forms_compiler.stage1 import compile_source
from web_forms_compiler.stage2 import generate_cpp
from web_forms_compiler.capabilities import read_capabilities
from web_forms_compiler.gui_material_stage2 import generate_gui_materials
from web_forms_compiler.gui_layout_stage2 import generate_gui_layouts
from web_forms_compiler.gui_typography_stage2 import generate_gui_typography
from web_forms_compiler.gui_decoration_stage2 import generate_gui_decorations
from web_forms_compiler.gui_tree_stage2 import generate_gui_tree


def percentile(values: list[float], fraction: float) -> float:
    ordered = sorted(values)
    index = min(len(ordered) - 1, int((len(ordered) - 1) * fraction + 0.5))
    return ordered[index]


def timing_summary(values: list[float]) -> dict[str, float]:
    milliseconds = [value * 1000.0 for value in values]
    return {
        "p50_ms": round(statistics.median(milliseconds), 3),
        "p95_ms": round(percentile(milliseconds, 0.95), 3),
        "p99_ms": round(percentile(milliseconds, 0.99), 3),
        "worst_ms": round(max(milliseconds), 3),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--iterations", type=int, default=20)
    args = parser.parse_args()
    if args.iterations < 2 or args.iterations > 200:
        parser.error("iterations must be in [2, 200]")

    specimens = [
        ("button", ROOT / "boards/controls/button/button.wf.html", None),
        ("breadcrumb", ROOT / "boards/widgets/breadcrumb/breadcrumb.wf.html", None),
        ("standard-shell-sapphire", ROOT / "boards/apps/standard_shell/standard_shell.wf.html", None),
        (
            "standard-shell-parchment",
            ROOT / "boards/apps/standard_shell/standard_shell.wf.html",
            [ROOT / "boards/apps/standard_shell/parchment.wf.css"],
        ),
    ]
    compiler = shutil.which("c++")
    manifest = read_capabilities(ROOT / "capabilities/gui_forms_observed_001.json")
    print(f"environment python={platform.python_version()} system={platform.system()} machine={platform.machine()} iterations={args.iterations}")
    for name, source, styles in specimens:
        stage_one_times: list[float] = []
        stage_two_times: list[float] = []
        material_stage_two_times: list[float] = []
        layout_stage_two_times: list[float] = []
        typography_stage_two_times: list[float] = []
        decoration_stage_two_times: list[float] = []
        tree_stage_two_times: list[float] = []
        ir = None
        with tempfile.TemporaryDirectory() as temporary_name:
            temporary = Path(temporary_name)
            for iteration in range(args.iterations):
                started = time.perf_counter()
                candidate = compile_source(source, styles)
                stage_one_times.append(time.perf_counter() - started)
                if ir is None:
                    ir = candidate
                elif candidate != ir:
                    raise RuntimeError(f"{name}: nondeterministic Stage 1 result at iteration {iteration}")
            assert ir is not None
            generated_text = None
            generated_paths = None
            for iteration in range(args.iterations):
                destination = temporary / f"generation-{iteration}"
                started = time.perf_counter()
                paths = generate_cpp(ir, destination, name)
                stage_two_times.append(time.perf_counter() - started)
                text = paths[0].read_bytes() + paths[1].read_bytes() + paths[2].read_bytes()
                if generated_text is None:
                    generated_text = text
                    generated_paths = paths
                elif text != generated_text:
                    raise RuntimeError(f"{name}: nondeterministic Stage 2 result at iteration {iteration}")
            compile_ms = None
            if compiler is not None and generated_paths is not None:
                started = time.perf_counter()
                result = subprocess.run(
                    [
                        compiler,
                        "-std=c++17",
                        "-Wall",
                        "-Wextra",
                        "-Werror",
                        "-I",
                        str(ROOT / "runtime/include"),
                        "-I",
                        str(generated_paths[1].parent),
                        "-c",
                        str(generated_paths[1]),
                        "-o",
                        str(temporary / f"{name}.o"),
                    ],
                    text=True,
                    capture_output=True,
                    check=False,
                )
                compile_ms = round((time.perf_counter() - started) * 1000.0, 3)
                if result.returncode != 0:
                    raise RuntimeError(result.stdout + result.stderr)
            native_generated_text = None
            for iteration in range(args.iterations):
                destination = temporary / f"native-generation-{iteration}"
                started = time.perf_counter()
                native_paths = generate_gui_materials(
                    ir, manifest, destination, name
                )
                material_stage_two_times.append(time.perf_counter() - started)
                text = (
                    native_paths[0].read_bytes()
                    + native_paths[1].read_bytes()
                    + native_paths[2].read_bytes()
                )
                if native_generated_text is None:
                    native_generated_text = text
                elif text != native_generated_text:
                    raise RuntimeError(
                        f"{name}: nondeterministic native material Stage 2 "
                        f"result at iteration {iteration}"
                    )
            layout_generated_text = None
            for iteration in range(args.iterations):
                destination = temporary / f"layout-generation-{iteration}"
                started = time.perf_counter()
                layout_paths = generate_gui_layouts(
                    ir, manifest, destination, name
                )
                layout_stage_two_times.append(time.perf_counter() - started)
                text = (
                    layout_paths[0].read_bytes()
                    + layout_paths[1].read_bytes()
                    + layout_paths[2].read_bytes()
                )
                if layout_generated_text is None:
                    layout_generated_text = text
                elif text != layout_generated_text:
                    raise RuntimeError(
                        f"{name}: nondeterministic native layout Stage 2 "
                        f"result at iteration {iteration}"
                    )
            typography_generated_text = None
            for iteration in range(args.iterations):
                destination = temporary / f"typography-generation-{iteration}"
                started = time.perf_counter()
                typography_paths = generate_gui_typography(
                    ir, manifest, destination, name
                )
                typography_stage_two_times.append(time.perf_counter() - started)
                text = (
                    typography_paths[0].read_bytes()
                    + typography_paths[1].read_bytes()
                    + typography_paths[2].read_bytes()
                )
                if typography_generated_text is None:
                    typography_generated_text = text
                elif text != typography_generated_text:
                    raise RuntimeError(
                        f"{name}: nondeterministic native typography Stage 2 "
                        f"result at iteration {iteration}"
                    )
            decoration_generated_text = None
            for iteration in range(args.iterations):
                destination = temporary / f"decoration-generation-{iteration}"
                started = time.perf_counter()
                decoration_paths = generate_gui_decorations(
                    ir, manifest, destination, name
                )
                decoration_stage_two_times.append(time.perf_counter() - started)
                text = (
                    decoration_paths[0].read_bytes()
                    + decoration_paths[1].read_bytes()
                    + decoration_paths[2].read_bytes()
                )
                if decoration_generated_text is None:
                    decoration_generated_text = text
                elif text != decoration_generated_text:
                    raise RuntimeError(
                        f"{name}: nondeterministic native decoration Stage 2 "
                        f"result at iteration {iteration}"
                    )
            tree_generated_text = None
            for iteration in range(args.iterations):
                destination = temporary / f"tree-generation-{iteration}"
                started = time.perf_counter()
                tree_paths = generate_gui_tree(
                    ir, manifest, destination, name
                )
                tree_stage_two_times.append(time.perf_counter() - started)
                text = b"".join(
                    path.read_bytes() for path in sorted(destination.iterdir())
                    if path.is_file()
                )
                if tree_generated_text is None:
                    tree_generated_text = text
                elif text != tree_generated_text:
                    raise RuntimeError(
                        f"{name}: nondeterministic complete native tree Stage 2 "
                        f"result at iteration {iteration}"
                    )
            print(
                f"{name} measurements={ir['measurements']} "
                f"stage1={timing_summary(stage_one_times)} stage2={timing_summary(stage_two_times)} "
                f"generated_bytes={len(generated_text or b'')} cpp17_compile_ms={compile_ms} "
                f"native_material_stage2={timing_summary(material_stage_two_times)} "
                f"native_material_bytes={len(native_generated_text or b'')} "
                f"native_layout_stage2={timing_summary(layout_stage_two_times)} "
                f"native_layout_bytes={len(layout_generated_text or b'')} "
                f"native_typography_stage2={timing_summary(typography_stage_two_times)} "
                f"native_typography_bytes={len(typography_generated_text or b'')} "
                f"native_decoration_stage2={timing_summary(decoration_stage_two_times)} "
                f"native_decoration_bytes={len(decoration_generated_text or b'')} "
                f"native_tree_stage2={timing_summary(tree_stage_two_times)} "
                f"native_tree_bundle_bytes={len(tree_generated_text or b'')}"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
