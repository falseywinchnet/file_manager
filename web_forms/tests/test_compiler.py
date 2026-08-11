from __future__ import annotations

import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

WEB_FORMS_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(WEB_FORMS_ROOT / "src"))

from web_forms_compiler.diagnostics import WebFormsError
from web_forms_compiler.capabilities import assess_capabilities, read_capabilities
from web_forms_compiler.gui_material_stage2 import generate_gui_materials
from web_forms_compiler.gui_layout_stage2 import generate_gui_layouts
from web_forms_compiler.gui_typography_stage2 import generate_gui_typography
from web_forms_compiler.gui_decoration_stage2 import generate_gui_decorations
from web_forms_compiler.gui_tree_stage2 import generate_gui_tree
from web_forms_compiler.stage1 import compile_source, read_ir, write_ir
from web_forms_compiler.stage2 import FORBIDDEN_GENERATED_PATTERNS, generate_cpp, generated_code_only


class CompilerTest(unittest.TestCase):
    def setUp(self) -> None:
        self.button = WEB_FORMS_ROOT / "boards/controls/button/button.wf.html"
        self.breadcrumb = WEB_FORMS_ROOT / "boards/widgets/breadcrumb/breadcrumb.wf.html"
        self.shell_root = WEB_FORMS_ROOT / "boards/apps/standard_shell"
        self.shell = self.shell_root / "standard_shell.wf.html"

    def _temporary_source(self, body: str, css: str) -> tuple[tempfile.TemporaryDirectory[str], Path]:
        temporary = tempfile.TemporaryDirectory()
        root = Path(temporary.name)
        html = root / "test.wf.html"
        stylesheet = root / "test.wf.css"
        html.write_text(
            "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">"
            "<title>Test</title><link rel=\"stylesheet\" href=\"test.wf.css\"></head>"
            f"<body id=\"test\" data-wf-surface=\"own-surface\">{body}</body></html>",
            encoding="utf-8",
        )
        stylesheet.write_text(css, encoding="utf-8")
        return temporary, html

    def assert_rejected(self, body: str, css: str, code: str) -> None:
        temporary, source = self._temporary_source(body, css)
        with temporary:
            with self.assertRaises(WebFormsError) as captured:
                compile_source(source)
        self.assertIn(code, {item.code for item in captured.exception.diagnostics})

    def test_dogfood_corpus_is_accepted(self) -> None:
        button = compile_source(self.button)
        breadcrumb = compile_source(self.breadcrumb)
        shell = compile_source(self.shell)
        self.assertEqual(7, button["measurements"]["runtime_nodes"])
        self.assertEqual(3, breadcrumb["measurements"]["decorations"])
        self.assertGreaterEqual(shell["measurements"]["runtime_nodes"], 35)
        self.assertIn("box-shadow", shell["requirements"]["properties"])
        self.assertIn("surface.nested-ambient-context", shell["requirements"]["features"])
        self.assertIn("effect.inset-shadow", shell["requirements"]["features"])
        self.assertIn("paint.side-border", shell["requirements"]["features"])
        self.assertIn("paint.css-angle-gradient", shell["requirements"]["features"])
        material_properties = [
            item
            for record in shell["styles"]["pools"]["material"]
            for item in record["properties"]
        ]
        gradient = next(item for item in material_properties if "linear-gradient(" in item["value"])
        shadow = next(item for item in material_properties if item["name"] == "box-shadow")
        self.assertEqual("gradient", gradient["typed"]["kind"])
        self.assertIn("color_rgba", {token["kind"] for token in shadow["typed"]["tokens"]})

    def test_content_and_geometry_survive_style_swap(self) -> None:
        sapphire = compile_source(self.shell)
        parchment = compile_source(self.shell, [self.shell_root / "parchment.wf.css"])
        self.assertEqual(sapphire["document"]["nodes"], parchment["document"]["nodes"])
        self.assertEqual(sapphire["styles"]["pools"]["geometry"], parchment["styles"]["pools"]["geometry"])
        self.assertEqual(sapphire["styles"]["pools"]["typography"], parchment["styles"]["pools"]["typography"])
        self.assertNotEqual(sapphire["styles"]["pools"]["material"], parchment["styles"]["pools"]["material"])

    def test_stage_one_is_deterministic(self) -> None:
        self.assertEqual(compile_source(self.breadcrumb), compile_source(self.breadcrumb))

    def test_rejects_script(self) -> None:
        self.assert_rejected("<script>bad()</script>", "body { color: #111111; }", "WFH027")

    def test_unsupported_subtree_does_not_corrupt_following_html(self) -> None:
        temporary, source = self._temporary_source(
            '<svg><path></path></svg><button id="test.go">Go</button>',
            "body { color: #111111; }",
        )
        with temporary:
            with self.assertRaises(WebFormsError) as captured:
                compile_source(source)
        codes = [item.code for item in captured.exception.diagnostics]
        self.assertEqual(["WFH010"], codes)

    def test_rejects_event_attribute(self) -> None:
        self.assert_rejected(
            '<button id="test.go" onclick="bad()">Go</button>',
            "body { color: #111111; }",
            "WFH015",
        )

    def test_rejects_addressable_control_without_id(self) -> None:
        self.assert_rejected("<button>Go</button>", "body { color: #111111; }", "WFH026")

    def test_rejects_identity_outside_parent_tree(self) -> None:
        self.assert_rejected(
            '<section id="test.group"><button id="test.other.go">Go</button></section>',
            "body { color: #111111; }",
            "WFH037",
        )

    def test_rejects_unknown_property_and_unit(self) -> None:
        self.assert_rejected("<p>Text</p>", "body { sparkle: yes; }", "WFC022")
        self.assert_rejected("<p>Text</p>", "body { width: 10vh; }", "WFC023")

    def test_only_bundled_local_font_faces_are_admitted(self) -> None:
        temporary, source = self._temporary_source(
            "<p>Text</p>",
            '@font-face { font-family: "Carlito"; '
            'src: url("../../../../gui_forms/assets/fonts/Carlito-Regular.ttf") '
            'format("truetype"); font-weight: 400; font-style: normal; } '
            'body { color: #111111; font-family: "Carlito", sans-serif; '
            'font-size: 12px; line-height: 1.25; }',
        )
        with temporary:
            ir = compile_source(source)
        self.assertIn("font-family", ir["requirements"]["properties"])
        self.assert_rejected(
            "<p>Text</p>",
            '@font-face { font-family: "Remote"; '
            'src: url("https://example.test/remote.ttf") format("truetype"); '
            'font-weight: 400; font-style: normal; } body { color: #111111; }',
            "WFC009",
        )

    def test_rejects_background_without_surface_owner(self) -> None:
        self.assert_rejected("<section class=\"paint\">Text</section>", ".paint { background: #ffffff; }", "WFC035")

    def test_rejects_gradient_without_solid_fallback(self) -> None:
        self.assert_rejected(
            '<section class="paint" data-wf-surface="own-surface">Text</section>',
            ".paint { background-image: linear-gradient(180deg, #ffffff, #eeeeee); }",
            "WFC037",
        )

    def test_background_shorthand_is_bounded_and_resets_image(self) -> None:
        temporary, source = self._temporary_source(
            '<section id="test.surface" class="paint" '
            'data-wf-surface="own-surface">Text</section>',
            ".paint { background-color: #112233; "
            "background-image: linear-gradient(180deg, #ffffff, #112233); } "
            ".paint:hover { background: #445566; }",
        )
        with temporary:
            ir = compile_source(source)
        node_index = next(
            node["index"] for node in ir["document"]["nodes"]
            if node["id"] == "test.surface"
        )
        variant = next(
            item for item in ir["styles"]["variants"]
            if item["node"] == node_index
        )
        properties = {
            item["name"]: item
            for item in ir["styles"]["pools"]["material"][variant["material"]]["properties"]
        }
        self.assertNotIn("background", properties)
        self.assertEqual("#445566", properties["background-color"]["value"])
        self.assertEqual("none", properties["background-image"]["value"])
        self.assertEqual("keyword", properties["background-image"]["typed"]["kind"])

        self.assert_rejected(
            '<section class="paint" data-wf-surface="own-surface">Text</section>',
            ".paint { background: linear-gradient(180deg, #ffffff, #112233); }",
            "WFC023",
        )

    def test_stage_two_reads_ir_and_is_deterministic(self) -> None:
        ir = compile_source(self.breadcrumb)
        with tempfile.TemporaryDirectory() as temporary_name:
            root = Path(temporary_name)
            ir_path = root / "breadcrumb.wfir.json"
            write_ir(ir, ir_path)
            loaded = read_ir(ir_path)
            first = root / "first"
            second = root / "second"
            first_paths = generate_cpp(loaded, first, "breadcrumb")
            second_paths = generate_cpp(loaded, second, "breadcrumb")
            self.assertEqual(first_paths[0].read_text(), second_paths[0].read_text())
            self.assertEqual(first_paths[1].read_text(), second_paths[1].read_text())
            report = json.loads(first_paths[2].read_text())
            self.assertIn("descriptor-generated", report["status"])
            generated = generated_code_only(first_paths[0].read_text() + first_paths[1].read_text())
            for pattern, description in FORBIDDEN_GENERATED_PATTERNS:
                self.assertIsNone(pattern.search(generated), description)
            self.assertNotIn("linear-gradient(", first_paths[1].read_text())
            self.assertNotIn("18px", first_paths[1].read_text())

    def test_stage_two_rejects_property_id_collision(self) -> None:
        ir = compile_source(self.button)
        with tempfile.TemporaryDirectory() as temporary_name:
            with patch(
                "web_forms_compiler.stage2.fnv1a_32", return_value=7
            ):
                with self.assertRaises(WebFormsError) as captured:
                    generate_cpp(ir, Path(temporary_name), "collision")
        self.assertEqual("WFG003", captured.exception.diagnostics[0].code)

    def test_generated_cpp_compiles_as_cpp17(self) -> None:
        compiler = shutil.which("c++")
        if compiler is None:
            self.skipTest("no C++ compiler available")
        ir = compile_source(self.shell)
        with tempfile.TemporaryDirectory() as temporary_name:
            root = Path(temporary_name)
            _, source, _ = generate_cpp(ir, root, "standard_shell")
            result = subprocess.run(
                [
                    compiler,
                    "-std=c++17",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(WEB_FORMS_ROOT / "runtime/include"),
                    "-I",
                    str(root),
                    "-c",
                    str(source),
                    "-o",
                    str(root / "standard_shell.o"),
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_exact_button_materials_project_into_gui_forms(self) -> None:
        compiler = shutil.which("c++")
        if compiler is None:
            self.skipTest("no C++ compiler available")
        ir = compile_source(self.button)
        manifest = read_capabilities(
            WEB_FORMS_ROOT / "capabilities/gui_forms_observed_001.json"
        )
        gui_forms_root = WEB_FORMS_ROOT.parent / "gui_forms"
        with tempfile.TemporaryDirectory() as temporary_name:
            root = Path(temporary_name)
            header, source, report_path = generate_gui_materials(
                ir, manifest, root, "button"
            )
            report = json.loads(report_path.read_text())
            by_style = {item["style"]: item for item in report["styles"]}
            self.assertEqual("exact-surface-projection", by_style[1]["status"])
            self.assertEqual("exact-surface-projection", by_style[2]["status"])
            self.assertEqual("exact-surface-projection", by_style[6]["status"])
            self.assertEqual(1, report["native_button_state_recipe_count"])
            self.assertEqual(
                "bounded-native-state-projection", report["buttons"][0]["status"]
            )
            self.assertIn(
                "keyboard/pointer/semantic modality are retained",
                " ".join(report["buttons"][0]["diagnostics"]),
            )
            generated = generated_code_only(header.read_text() + source.read_text())
            for pattern, description in FORBIDDEN_GENERATED_PATTERNS:
                self.assertIsNone(pattern.search(generated), description)

            probe = root / "probe.cpp"
            probe.write_text(
                '#include "button.gui_materials.wf.hpp"\n'
                "int main() {\n"
                "    if (!web_forms_generated_button::has_native_surface_material(6U)) return 1;\n"
                "    const gui_forms::SurfaceMaterial material = "
                "web_forms_generated_button::make_native_surface_material(6U);\n"
                "    if (material.fills.size() != 2U || material.shadows.size() != 1U) return 2;\n"
                "    if (material.fills[1].stops.size() != 2U || "
                "material.fills[1].stops[0].color != "
                "gui_forms::Color::rgba(255U, 253U, 245U, 255U)) return 3;\n"
                "    if (!material.border || material.border->color != "
                "gui_forms::Color::rgba(121U, 153U, 165U, 255U)) return 4;\n"
                "    if (material.corner_radius != 9.0 || "
                "material.shadows[0].offset.y != 2.0) return 5;\n"
                "    if (!web_forms_generated_button::has_native_button_state_recipes(10U)) "
                "return 6;\n"
                "    const gui_forms::ControlStateRecipes recipes = "
                "web_forms_generated_button::make_native_button_state_recipes(10U);\n"
                "    const gui_forms::ControlVisualRecipe& hot = "
                "recipes.resolve(gui_forms::ControlSurfaceState::hot);\n"
                "    const gui_forms::ControlVisualRecipe& pressed = "
                "recipes.resolve(gui_forms::ControlSurfaceState::pressed);\n"
                "    if (!hot.material.border || hot.material.border->color != "
                "gui_forms::Color::rgba(40U, 125U, 155U, 255U)) return 7;\n"
                "    if (pressed.visual_offset != gui_forms::Point{0.0, 1.0} || "
                "pressed.material.shadows[0].offset.y != 1.0) return 8;\n"
                "    if (!hot.authored_focus_outline || hot.focus_width != 2.0 || "
                "hot.focus_offset != 3.0) return 9;\n"
                "    return 0;\n"
                "}\n",
                encoding="utf-8",
            )
            executable = root / "probe"
            result = subprocess.run(
                [
                    compiler,
                    "-std=c++20",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(gui_forms_root / "include"),
                    "-I",
                    str(root),
                    str(source),
                    str(
                        gui_forms_root
                        / "src/core/surface_material/material_fill_layer/material_fill_layer.cpp"
                    ),
                    str(
                        gui_forms_root
                        / "src/core/surface_material/types/surface_material_types.cpp"
                    ),
                    str(gui_forms_root / "src/core/types/painter/painter.cpp"),
                    str(gui_forms_root / "src/core/theme/theme/theme.cpp"),
                    str(probe),
                    "-o",
                    str(executable),
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            run = subprocess.run(
                [str(executable)], text=True, capture_output=True, check=False
            )
            self.assertEqual(0, run.returncode, run.stdout + run.stderr)

    def test_shell_material_projection_includes_retained_inset_shadows(self) -> None:
        compiler = shutil.which("c++")
        if compiler is None:
            self.skipTest("no C++ compiler available")
        ir = compile_source(self.shell)
        manifest = read_capabilities(
            WEB_FORMS_ROOT / "capabilities/gui_forms_observed_001.json"
        )
        gui_forms_root = WEB_FORMS_ROOT.parent / "gui_forms"
        with tempfile.TemporaryDirectory() as temporary_name:
            root = Path(temporary_name)
            _, source, report_path = generate_gui_materials(
                ir, manifest, root, "standard_shell"
            )
            report = json.loads(report_path.read_text())
            self.assertEqual(16, report["exact_style_count"])
            self.assertEqual(0, report["unavailable_style_count"])
            self.assertIn("true}", source.read_text())
            self.assertIn(
                "MaterialBorderEdges::from_parts", source.read_text()
            )
            result = subprocess.run(
                [
                    compiler,
                    "-std=c++20",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(gui_forms_root / "include"),
                    "-I",
                    str(root),
                    "-c",
                    str(source),
                    "-o",
                    str(root / "standard_shell_materials.o"),
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_relational_flex_layout_projects_into_gui_forms(self) -> None:
        compiler = shutil.which("c++")
        if compiler is None:
            self.skipTest("no C++ compiler available")
        manifest = read_capabilities(
            WEB_FORMS_ROOT / "capabilities/gui_forms_observed_001.json"
        )
        gui_forms_root = WEB_FORMS_ROOT.parent / "gui_forms"
        with tempfile.TemporaryDirectory() as temporary_name:
            root = Path(temporary_name)
            header, source, report_path = generate_gui_layouts(
                compile_source(self.button), manifest, root, "button"
            )
            report = json.loads(report_path.read_text())
            self.assertEqual(2, report["exact_flow_count"])
            self.assertEqual(0, report["unavailable_flow_count"])
            self.assertEqual(2, len(report["nested_owned_surfaces"]))
            generated = generated_code_only(header.read_text() + source.read_text())
            for pattern, description in FORBIDDEN_GENERATED_PATTERNS:
                self.assertIsNone(pattern.search(generated), description)
            emitted = source.read_text()
            self.assertIn("FlowMainAlignment::center", emitted)
            self.assertIn("FlowCrossAlignment::center", emitted)
            self.assertIn("panel.set_padding({48.0, 48.0, 48.0, 48.0})", emitted)
            result = subprocess.run(
                [
                    compiler,
                    "-std=c++20",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(gui_forms_root / "include"),
                    "-I",
                    str(root),
                    "-c",
                    str(source),
                    "-o",
                    str(root / "button_layouts.o"),
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

            _, shell_source, shell_report_path = generate_gui_layouts(
                compile_source(self.shell), manifest, root, "standard_shell"
            )
            shell_report = json.loads(shell_report_path.read_text())
            self.assertEqual(11, shell_report["exact_flow_count"])
            self.assertEqual(0, shell_report["unavailable_flow_count"])
            self.assertEqual(2, shell_report["exact_grid_count"])
            self.assertEqual(0, shell_report["unavailable_grid_count"])
            self.assertEqual(4, shell_report["grid_cell_count"])
            self.assertEqual(28, shell_report["exact_box_count"])
            self.assertEqual(0, shell_report["unavailable_box_count"])
            self.assertTrue(
                all(
                    not item["unprojected_box_geometry"]
                    for item in shell_report["boxes"]
                    if item["status"] == "bounded-native-box-projection"
                )
            )
            self.assertEqual(4, len(shell_report["flex_grows"]))
            emitted_shell = shell_source.read_text()
            self.assertIn("native_flex_grow", emitted_shell)
            self.assertIn(
                "panel.set_column_style(0U, {gui_forms::TableSizeMode::absolute, 210.0})",
                emitted_shell,
            )
            self.assertIn(
                "panel.set_track_spacing({18.0, 18.0})", emitted_shell
            )
            self.assertIn(
                "panel.set_cell_position(child, {1U, 0U})", emitted_shell
            )
            self.assertIn("minimum.width = 1080.0", emitted_shell)
            self.assertIn("bounds.width = 1012.0", emitted_shell)
            self.assertIn("native_box_has_horizontal_auto_margin", emitted_shell)
            shell_compile = subprocess.run(
                [
                    compiler,
                    "-std=c++20",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(gui_forms_root / "include"),
                    "-I",
                    str(root),
                    "-c",
                    str(shell_source),
                    "-o",
                    str(root / "standard_shell_layouts.o"),
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(
                0, shell_compile.returncode,
                shell_compile.stdout + shell_compile.stderr,
            )

            temporary, wrapped_source = self._temporary_source(
                '<section id="test.wrap" class="wrap" '
                'data-wf-control="panel"></section>',
                ".wrap { display: flex; flex-wrap: wrap; }",
            )
            with temporary:
                _, _, wrapped_report_path = generate_gui_layouts(
                    compile_source(wrapped_source), manifest, root, "wrapped"
                )
                wrapped_report = json.loads(wrapped_report_path.read_text())
            self.assertEqual(0, wrapped_report["exact_flow_count"])
            self.assertEqual(1, wrapped_report["unavailable_flow_count"])
            self.assertIn(
                "align-content:flex-start",
                " ".join(wrapped_report["flows"][0]["diagnostics"]),
            )

    def test_current_gui_forms_manifest_is_ready_for_dogfood_native_lowering(self) -> None:
        ir = compile_source(self.breadcrumb)
        manifest = read_capabilities(WEB_FORMS_ROOT / "capabilities/gui_forms_observed_001.json")
        assessment = assess_capabilities(ir, manifest)
        self.assertTrue(assessment["ready_for_native_lowering"])
        by_name = {item["feature"]: item for item in assessment["features"]}
        self.assertEqual("supported", by_name["surface.nested-ambient-context"]["classification"])
        self.assertEqual("supported", by_name["decoration.after"]["classification"])
        self.assertEqual("supported", by_name["effect.inset-shadow"]["classification"])

    def test_retained_typography_projects_shared_font_and_text_geometry(self) -> None:
        compiler = shutil.which("c++")
        if compiler is None:
            self.skipTest("no C++ compiler available")
        manifest = read_capabilities(
            WEB_FORMS_ROOT / "capabilities/gui_forms_observed_001.json"
        )
        gui_forms_root = WEB_FORMS_ROOT.parent / "gui_forms"
        with tempfile.TemporaryDirectory() as temporary_name:
            root = Path(temporary_name)
            header, source, report_path = generate_gui_typography(
                compile_source(self.shell), manifest, root, "standard_shell"
            )
            report = json.loads(report_path.read_text())
            self.assertEqual(38, report["exact_typography_count"])
            self.assertEqual(0, report["unavailable_typography_count"])
            emitted = source.read_text()
            self.assertIn("gui_forms::FontRole::content", emitted)
            self.assertIn("28.0, 400U, false, 0.0", emitted)
            self.assertIn("label.set_line_spacing(1.25)", emitted)
            self.assertIn("button.set_text_alignment", emitted)
            generated = generated_code_only(header.read_text() + emitted)
            for pattern, description in FORBIDDEN_GENERATED_PATTERNS:
                self.assertIsNone(pattern.search(generated), description)
            result = subprocess.run(
                [
                    compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror",
                    "-I", str(gui_forms_root / "include"), "-I", str(root),
                    "-c", str(source), "-o", str(root / "typography.o"),
                ],
                text=True, capture_output=True, check=False,
            )
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_breadcrumb_chevrons_project_as_owner_relative_decorations(self) -> None:
        compiler = shutil.which("c++")
        if compiler is None:
            self.skipTest("no C++ compiler available")
        manifest = read_capabilities(
            WEB_FORMS_ROOT / "capabilities/gui_forms_observed_001.json"
        )
        gui_forms_root = WEB_FORMS_ROOT.parent / "gui_forms"
        with tempfile.TemporaryDirectory() as temporary_name:
            root = Path(temporary_name)
            header, source, report_path = generate_gui_decorations(
                compile_source(self.breadcrumb), manifest, root, "breadcrumb"
            )
            report = json.loads(report_path.read_text())
            self.assertEqual(3, report["exact_decoration_count"])
            self.assertEqual(0, report["unavailable_decoration_count"])
            self.assertEqual(3, report["owners"])
            emitted = source.read_text()
            self.assertIn("decoration_0.right = 10.0", emitted)
            self.assertIn("decoration_0.rotation_degrees = 45.0", emitted)
            self.assertIn("MaterialBorderEdges::from_parts", emitted)
            generated = generated_code_only(header.read_text() + emitted)
            for pattern, description in FORBIDDEN_GENERATED_PATTERNS:
                self.assertIsNone(pattern.search(generated), description)
            result = subprocess.run(
                [
                    compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror",
                    "-I", str(gui_forms_root / "include"), "-I", str(root),
                    "-c", str(source), "-o", str(root / "decorations.o"),
                ],
                text=True, capture_output=True, check=False,
            )
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

            temporary, decorated_source = self._temporary_source(
                '<button id="test.decorated">Decorated</button>',
                'button::before { content: ""; position: absolute; width: 6px; '
                'height: 6px; left: 2px; top: 2px; transform: rotate(45deg); '
                'border-top: 1px solid #112233; border-left: 1px solid #112233; } '
                'button::after { content: ""; position: absolute; width: 6px; '
                'height: 6px; right: 2px; bottom: 2px; transform: rotate(45deg); '
                'border-right: 1px solid #445566; '
                'border-bottom: 1px solid #445566; }',
            )
            with temporary:
                decorated_ir = compile_source(decorated_source)
                assessment = assess_capabilities(decorated_ir, manifest)
                self.assertTrue(assessment["ready_for_native_lowering"])
                decorated_header, decorated_cpp, decorated_report_path = (
                    generate_gui_decorations(
                        decorated_ir, manifest, root, "before_after"
                    )
                )
            decorated_report = json.loads(decorated_report_path.read_text())
            self.assertEqual(2, decorated_report["exact_decoration_count"])
            decorated_emitted = (
                decorated_header.read_text() + decorated_cpp.read_text()
            )
            self.assertIn("OwnerDecorationLayer::before_content", decorated_emitted)
            self.assertIn("OwnerDecorationLayer::after_content", decorated_emitted)
            decorated_compile = subprocess.run(
                [
                    compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror",
                    "-I", str(gui_forms_root / "include"), "-I", str(root),
                    "-c", str(decorated_cpp),
                    "-o", str(root / "before_after.o"),
                ],
                text=True, capture_output=True, check=False,
            )
            self.assertEqual(
                0, decorated_compile.returncode,
                decorated_compile.stdout + decorated_compile.stderr,
            )

    def test_complete_dogfood_trees_are_typed_compile_valid_and_fail_closed(self) -> None:
        compiler = shutil.which("c++")
        if compiler is None:
            self.skipTest("no C++ compiler available")
        manifest = read_capabilities(
            WEB_FORMS_ROOT / "capabilities/gui_forms_observed_001.json"
        )
        gui_forms_root = WEB_FORMS_ROOT.parent / "gui_forms"
        with tempfile.TemporaryDirectory() as temporary_name:
            root = Path(temporary_name)
            header, source, report_path = generate_gui_tree(
                compile_source(self.button), manifest, root, "button"
            )
            report = json.loads(report_path.read_text())
            self.assertEqual(
                "complete-bounded-native-tree-projection", report["status"]
            )
            self.assertEqual(7, report["node_count"])
            self.assertEqual(7, report["typed_member_count"])
            self.assertEqual("button-study", report["root_id"])
            self.assertEqual(
                "button_study_stage_archive", report["commands"][0]["member"]
            )
            emitted = header.read_text() + source.read_text()
            self.assertIn(
                "std::shared_ptr<gui_forms::Button> button_study_stage_archive",
                emitted,
            )
            self.assertIn(
                "button_study_stage->add_child(form.button_study_stage_archive)",
                emitted,
            )
            self.assertIn("make_native_button_state_recipes(10U)", emitted)
            generated = generated_code_only(emitted)
            for pattern, description in FORBIDDEN_GENERATED_PATTERNS:
                self.assertIsNone(pattern.search(generated), description)
            result = subprocess.run(
                [
                    compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror",
                    "-I", str(gui_forms_root / "include"), "-I", str(root),
                    "-c", str(source), "-o", str(root / "button_tree.o"),
                ],
                text=True, capture_output=True, check=False,
            )
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

            breadcrumb_header, breadcrumb_source, breadcrumb_report_path = (
                generate_gui_tree(
                    compile_source(self.breadcrumb), manifest, root, "breadcrumb"
                )
            )
            breadcrumb_report = json.loads(breadcrumb_report_path.read_text())
            self.assertEqual(14, breadcrumb_report["typed_member_count"])
            self.assertEqual(6, breadcrumb_report["button_count"])
            self.assertIn(
                "apply_native_owner_decorations(11U",
                breadcrumb_source.read_text(),
            )
            self.assertIn(
                "breadcrumb_study_stage_recent_three",
                breadcrumb_header.read_text(),
            )
            breadcrumb_compile = subprocess.run(
                [
                    compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror",
                    "-I", str(gui_forms_root / "include"), "-I", str(root),
                    "-c", str(breadcrumb_source),
                    "-o", str(root / "breadcrumb_tree.o"),
                ],
                text=True, capture_output=True, check=False,
            )
            self.assertEqual(
                0, breadcrumb_compile.returncode,
                breadcrumb_compile.stdout + breadcrumb_compile.stderr,
            )

            shell_header, shell_source, shell_report_path = generate_gui_tree(
                compile_source(self.shell), manifest, root,
                "standard_shell_sapphire",
            )
            shell_report = json.loads(shell_report_path.read_text())
            self.assertEqual(38, shell_report["typed_member_count"])
            self.assertEqual(10, shell_report["button_count"])
            self.assertEqual(2, shell_report["grid_count"])
            self.assertIn(
                "std::shared_ptr<gui_forms::TableLayoutPanel> "
                "standard_app_shell_workspace",
                shell_header.read_text(),
            )
            self.assertIn(
                "apply_native_grid_layout(17U",
                shell_source.read_text(),
            )
            shell_compile = subprocess.run(
                [
                    compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror",
                    "-I", str(gui_forms_root / "include"), "-I", str(root),
                    "-c", str(shell_source),
                    "-o", str(root / "standard_shell_tree.o"),
                ],
                text=True, capture_output=True, check=False,
            )
            self.assertEqual(
                0, shell_compile.returncode,
                shell_compile.stdout + shell_compile.stderr,
            )

            parchment_source_ir = compile_source(
                self.shell, [self.shell_root / "parchment.wf.css"]
            )
            _, parchment_source, parchment_report_path = generate_gui_tree(
                parchment_source_ir, manifest, root,
                "standard_shell_parchment",
            )
            parchment_report = json.loads(parchment_report_path.read_text())
            self.assertEqual(38, parchment_report["typed_member_count"])
            self.assertEqual(2, parchment_report["grid_count"])
            parchment_compile = subprocess.run(
                [
                    compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror",
                    "-I", str(gui_forms_root / "include"), "-I", str(root),
                    "-c", str(parchment_source),
                    "-o", str(root / "standard_shell_parchment_tree.o"),
                ],
                text=True, capture_output=True, check=False,
            )
            self.assertEqual(
                0, parchment_compile.returncode,
                parchment_compile.stdout + parchment_compile.stderr,
            )

            temporary, refused_source = self._temporary_source(
                '<section id="test.wrap" class="wrap"></section>',
                '.wrap { display: flex; flex-wrap: wrap; }',
            )
            with temporary:
                with self.assertRaises(WebFormsError) as captured:
                    generate_gui_tree(
                        compile_source(refused_source), manifest, root,
                        "refused_tree",
                    )
            self.assertEqual("WFGT007", captured.exception.diagnostics[0].code)
            self.assertFalse((root / "refused_tree.gui_tree.wf.cpp").exists())


if __name__ == "__main__":
    unittest.main()
