from __future__ import annotations

import copy
from pathlib import Path
import unittest


WEB_FORMS_ROOT = Path(__file__).resolve().parents[1]
import sys
sys.path.insert(0, str(WEB_FORMS_ROOT / "src"))

from web_forms_compiler.diagnostics import WebFormsError
from web_forms_compiler.fidelity import (
    FidelityTolerance,
    SNAPSHOT_SCHEMA,
    capture_browser_snapshot,
    compare_fidelity_snapshots,
    normalize_native_snapshot,
    validate_fidelity_snapshot,
)


class FidelityTest(unittest.TestCase):
    def test_real_chromium_capture_has_exact_viewport_loaded_fonts_and_stable_ids(self) -> None:
        source = WEB_FORMS_ROOT / "boards/controls/button/button.wf.html"
        snapshot = capture_browser_snapshot(source, viewport=(800, 600))
        self.assertEqual(SNAPSHOT_SCHEMA, snapshot["schema"])
        self.assertEqual([800, 600], snapshot["environment"]["viewport"])
        self.assertEqual("loaded", snapshot["environment"]["font_status"])
        self.assertEqual("button-study", snapshot["root_id"])
        self.assertEqual(7, len(snapshot["nodes"]))
        primary = next(
            item for item in snapshot["nodes"]
            if item["id"] == "button-study.stage.archive"
        )
        self.assertEqual("button", primary["kind"])
        self.assertEqual("Archive selection", primary["typography"]["text"])
        self.assertGreater(primary["typography"]["baseline"], primary["bounds"][1])
        self.assertIn("solid", primary["material"]["fill_kinds"])
        self.assertEqual(8, len(primary["raster_probes"]))

    def test_browser_capture_exercises_scaled_interaction_state(self) -> None:
        source = WEB_FORMS_ROOT / "boards/controls/button/button.wf.html"
        target = "button-study.stage.archive"
        snapshot = capture_browser_snapshot(
            source,
            viewport=(800, 600),
            text_scale=1.5,
            interaction="pressed",
            interaction_target=target,
            state="pressed-150",
        )
        primary = next(item for item in snapshot["nodes"] if item["id"] == target)
        self.assertEqual(1.5, snapshot["environment"]["text_scale"])
        self.assertEqual("pressed-150", snapshot["state"])
        self.assertTrue(primary["state"]["hovered"])
        self.assertTrue(primary["state"]["pressed"])
        self.assertEqual("pressed", primary["state"]["visual_status"])
        self.assertGreater(primary["typography"]["font_size"], 13.0)
        self.assertEqual(8, len(primary["raster_probes"]))

    def test_native_visual_inspection_normalizes_without_renderer_types(self) -> None:
        material = {
            "corner_radius": 4,
            "fills": [{"kind": "solid", "color": [10, 20, 30, 255]}],
            "shadows": [],
            "border": {"width": 1, "color": [1, 2, 3, 255]},
            "border_edges": {},
            "keylines": [],
        }
        state = {
            "effectively_visible": True,
            "effectively_enabled": True,
            "focused": False,
            "hovered": False,
            "pressed": False,
            "window_active": True,
            "layout_collapsed": False,
            "visual_status": "normal",
        }
        native = {
            "client_size": [200, 100],
            "device_scale": 2,
            "text_scale": 1.25,
            "window_active": True,
            "controls": [
                {
                    "stable_id": "study",
                    "parent_stable_id": "",
                    "layout": {
                        "absolute_bounds": [20, 10, 160, 80],
                        "effective_clip": [20, 10, 160, 80],
                    },
                    "state": state,
                    "authored_material": material,
                    "display_operations": [],
                },
                {
                    "stable_id": "study.private-panel",
                    "parent_stable_id": "study",
                    "layout": {
                        "absolute_bounds": [20, 10, 160, 80],
                        "effective_clip": [20, 10, 160, 80],
                    },
                    "state": state,
                    "authored_material": None,
                    "display_operations": [],
                },
                {
                    "stable_id": "study.label",
                    "parent_stable_id": "study.private-panel",
                    "layout": {
                        "absolute_bounds": [30, 25, 90, 20],
                        "effective_clip": [30, 25, 90, 20],
                    },
                    "state": state,
                    "authored_material": None,
                    "display_operations": [
                        {
                            "operation": "draw_text",
                            "text": "Exact",
                            "first": [2, 14],
                            "font": {
                                "size": 12,
                                "weight": 700,
                                "italic": False,
                                "letter_spacing": 0.25,
                            },
                            "resolved_text": {
                                "status": "exact",
                                "primary_family": "Carlito",
                                "ascent": 10,
                                "descent": 2,
                                "line_gap": 0,
                                "runs": [],
                            },
                        }
                    ],
                },
                {
                    "stable_id": "study.private-grip",
                    "parent_stable_id": "study",
                    "layout": {
                        "absolute_bounds": [50, 10, 3, 80],
                        "effective_clip": [50, 10, 3, 80],
                    },
                    "state": state,
                    "authored_material": None,
                    "display_operations": [],
                },
            ],
        }
        projection = {
            "root_id": "study",
            "source_digest": "abc",
            "nodes": [
                {"id": "study", "kind": "panel"},
                {"id": "study.label", "kind": "label"},
            ],
        }
        snapshot = normalize_native_snapshot(native, projection=projection)
        root, label = snapshot["nodes"]
        self.assertEqual(2, len(snapshot["nodes"]))
        self.assertEqual([0, 0, 160, 80], root["bounds"])
        self.assertEqual([10, 15, 90, 20], label["bounds"])
        self.assertEqual("study", label["parent_id"])
        self.assertEqual(29, label["typography"]["baseline"])
        self.assertEqual("Carlito", label["typography"]["family"])
        self.assertEqual(["solid"], root["material"]["fill_kinds"])

    def test_comparator_keeps_difference_dimensions_separate(self) -> None:
        source = WEB_FORMS_ROOT / "boards/controls/button/button.wf.html"
        browser = capture_browser_snapshot(source, viewport=(800, 600))
        native = copy.deepcopy(browser)
        native["producer"] = "test-native"
        exact = compare_fidelity_snapshots(browser, native)
        self.assertEqual("match", exact["status"])

        primary = next(
            item for item in native["nodes"]
            if item["id"] == "button-study.stage.archive"
        )
        primary["bounds"][0] += 3
        primary["typography"]["baseline"] += 2
        primary["material"]["fill_kinds"] = []
        report = compare_fidelity_snapshots(
            browser, native, FidelityTolerance(geometry=0.5, baseline=1.0)
        )
        self.assertEqual("different", report["status"])
        self.assertEqual(1, report["measurements"]["category_node_counts"]["geometry"])
        self.assertEqual(1, report["measurements"]["category_node_counts"]["typography"])
        self.assertEqual(1, report["measurements"]["category_node_counts"]["material"])
        self.assertEqual(0, report["measurements"]["category_node_counts"]["structure"])

    def test_snapshot_validation_fails_closed_on_duplicate_identity(self) -> None:
        invalid = {
            "schema": SNAPSHOT_SCHEMA,
            "producer": "test",
            "root_id": "root",
            "state": "reference",
            "environment": {},
            "nodes": [
                {"id": "root", "bounds": [0, 0, 1, 1], "clip": [0, 0, 1, 1]},
                {"id": "root", "bounds": [0, 0, 1, 1], "clip": [0, 0, 1, 1]},
            ],
        }
        with self.assertRaises(WebFormsError) as captured:
            validate_fidelity_snapshot(invalid, "fixture")
        self.assertEqual("WFV013", captured.exception.diagnostics[0].code)


if __name__ == "__main__":
    unittest.main()
