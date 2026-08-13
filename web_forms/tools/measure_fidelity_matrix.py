#!/usr/bin/env python3
from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import subprocess
import sys
from typing import Any


WEB_FORMS_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(WEB_FORMS_ROOT / "src"))

from web_forms_compiler.fidelity import (
    capture_browser_snapshot,
    compare_fidelity_snapshots,
    normalize_native_snapshot,
    write_json,
)
from web_forms_compiler.stage1 import compile_source


@dataclass(frozen=True)
class Profile:
    name: str
    viewport: tuple[int, int]
    text_scale: float = 1.0
    active: bool = True
    interaction: str = "reference"


PROFILES = (
    Profile("ordinary", (1450, 850)),
    Profile("narrow-overflow", (720, 720)),
    Profile("tiny-collapse", (150, 150)),
    Profile("text-125", (1450, 850), 1.25),
    Profile("text-150", (1450, 850), 1.5),
    Profile("text-200", (1450, 850), 2.0),
    Profile("inactive", (1450, 850), active=False),
    Profile("hover", (1450, 850), interaction="hover"),
    Profile("pressed", (1450, 850), interaction="pressed"),
    Profile("focused", (1450, 850), interaction="focused"),
    Profile("disabled", (1450, 850), interaction="disabled"),
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("--native-probe", type=Path, required=True)
    parser.add_argument("--projection", type=Path, required=True)
    parser.add_argument("--font-directory", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument(
        "--interaction-target",
        default="file-manager-app.shell.commands.arrange-group.actions.details",
    )
    parser.add_argument("--chrome", type=Path)
    args = parser.parse_args()

    source = args.source.resolve()
    projection = json.loads(args.projection.read_text(encoding="utf-8"))
    source_ir = compile_source(source)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    records: list[dict[str, Any]] = []
    structural_failure = False
    for profile in PROFILES:
        target = args.interaction_target if profile.interaction != "reference" else ""
        browser = capture_browser_snapshot(
            source,
            state=profile.name,
            viewport=profile.viewport,
            text_scale=profile.text_scale,
            window_active=profile.active,
            interaction=profile.interaction,
            interaction_target=target,
            chrome=args.chrome,
        )
        native_command = [
            str(args.native_probe),
            str(profile.viewport[0]),
            str(profile.viewport[1]),
            str(args.font_directory),
            str(profile.text_scale),
            "active" if profile.active else "inactive",
            profile.interaction,
            target or "-",
        ]
        native_result = subprocess.run(
            native_command, text=True, capture_output=True, check=False
        )
        if native_result.returncode != 0:
            raise RuntimeError(
                f"native profile {profile.name} failed: {native_result.stderr}"
            )
        native_inspection = json.loads(native_result.stdout)
        native = normalize_native_snapshot(
            native_inspection,
            projection=projection,
            source_digest=source_ir["source"]["digest"],
            state=profile.name,
        )
        report = compare_fidelity_snapshots(browser, native)
        write_json(browser, args.output_dir / f"{profile.name}.browser.json")
        write_json(native, args.output_dir / f"{profile.name}.native.json")
        write_json(report, args.output_dir / f"{profile.name}.report.json")
        counts = report["measurements"]["category_node_counts"]
        invariant_failure = (
            not report["source_digest_match"] or
            not report["state_match"] or
            counts["structure"] != 0
        )
        structural_failure = structural_failure or invariant_failure
        records.append(
            {
                "name": profile.name,
                "viewport": list(profile.viewport),
                "text_scale": profile.text_scale,
                "window_active": profile.active,
                "interaction": profile.interaction,
                "source_digest_match": report["source_digest_match"],
                "state_match": report["state_match"],
                "status": report["status"],
                "category_node_counts": counts,
                "maximum_absolute_bounds_delta": report["measurements"][
                    "maximum_absolute_bounds_delta"
                ],
                "maximum_absolute_clip_delta": report["measurements"][
                    "maximum_absolute_clip_delta"
                ],
                "maximum_absolute_baseline_delta": report["measurements"][
                    "maximum_absolute_baseline_delta"
                ],
                "maximum_raster_color_distance": report["measurements"][
                    "maximum_raster_color_distance"
                ],
                "report": f"{profile.name}.report.json",
            }
        )
    aggregate = {
        "schema": "web.forms.fidelity-matrix/0.1-experimental",
        "source": str(source),
        "source_digest": source_ir["source"]["digest"],
        "profile_count": len(records),
        "structural_invariants": "failed" if structural_failure else "passed",
        "requirements_exercised": {
            "ordinary": True,
            "narrow": True,
            "150x150": True,
            "text_scales": [1.0, 1.25, 1.5, 2.0],
            "active_inactive": True,
            "interactions": ["hover", "pressed", "focused", "disabled"],
            "collapsed_and_overflow": True,
            "semantic_material_operations": True,
            "raster_probes_per_visible_node": 8,
        },
        "profiles": records,
    }
    write_json(aggregate, args.output_dir / "matrix.json")
    print(args.output_dir / "matrix.json")
    return 1 if structural_failure else 0


if __name__ == "__main__":
    raise SystemExit(main())
