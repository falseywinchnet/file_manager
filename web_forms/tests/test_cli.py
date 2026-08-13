from __future__ import annotations

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class CliTest(unittest.TestCase):
    def test_quiet_stage_one_is_silent_on_accept(self) -> None:
        source = ROOT / "boards/controls/button/button.wf.html"
        result = subprocess.run(
            [sys.executable, str(ROOT / "tools/webforms.py"), "check", str(source), "--quiet"],
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(0, result.returncode, result.stderr)
        self.assertEqual("", result.stdout)

    def test_build_writes_ir_and_cpp(self) -> None:
        source = ROOT / "boards/widgets/breadcrumb/breadcrumb.wf.html"
        with tempfile.TemporaryDirectory() as temporary_name:
            output = Path(temporary_name)
            result = subprocess.run(
                [
                    sys.executable,
                    str(ROOT / "tools/webforms.py"),
                    "build",
                    str(source),
                    "--emit-ir",
                    str(output / "breadcrumb.wfir.json"),
                    "--output-dir",
                    str(output / "generated"),
                    "--unit",
                    "breadcrumb",
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertTrue((output / "breadcrumb.wfir.json").is_file())
            self.assertTrue((output / "generated/breadcrumb.wf.cpp").is_file())

    def test_profile_is_machine_readable(self) -> None:
        result = subprocess.run(
            [sys.executable, str(ROOT / "tools/webforms.py"), "profile"],
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(0, result.returncode, result.stderr)
        self.assertIn('"source_bytes": 262144', result.stdout)

    def test_gui_material_projection_requires_and_uses_manifest(self) -> None:
        source = ROOT / "boards/controls/button/button.wf.html"
        manifest = ROOT / "capabilities/gui_forms_observed_002.json"
        with tempfile.TemporaryDirectory() as temporary_name:
            output = Path(temporary_name)
            ir = output / "button.wfir.json"
            check = subprocess.run(
                [
                    sys.executable,
                    str(ROOT / "tools/webforms.py"),
                    "check",
                    str(source),
                    "--emit-ir",
                    str(ir),
                    "--quiet",
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(0, check.returncode, check.stdout + check.stderr)
            result = subprocess.run(
                [
                    sys.executable,
                    str(ROOT / "tools/webforms.py"),
                    "generate-gui-materials",
                    str(ir),
                    "--manifest",
                    str(manifest),
                    "--output-dir",
                    str(output / "native"),
                    "--unit",
                    "button",
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertTrue(
                (output / "native/button.gui_materials.wf.cpp").is_file()
            )
            layout_result = subprocess.run(
                [
                    sys.executable,
                    str(ROOT / "tools/webforms.py"),
                    "generate-gui-layouts",
                    str(ir),
                    "--manifest",
                    str(manifest),
                    "--output-dir",
                    str(output / "native"),
                    "--unit",
                    "button",
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(
                0, layout_result.returncode,
                layout_result.stdout + layout_result.stderr,
            )
            self.assertTrue(
                (output / "native/button.gui_layouts.wf.cpp").is_file()
            )
            typography_result = subprocess.run(
                [
                    sys.executable,
                    str(ROOT / "tools/webforms.py"),
                    "generate-gui-typography",
                    str(ir),
                    "--manifest",
                    str(manifest),
                    "--output-dir",
                    str(output / "native"),
                    "--unit",
                    "button",
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(
                0, typography_result.returncode,
                typography_result.stdout + typography_result.stderr,
            )
            self.assertTrue(
                (output / "native/button.gui_typography.wf.cpp").is_file()
            )
            decoration_result = subprocess.run(
                [
                    sys.executable,
                    str(ROOT / "tools/webforms.py"),
                    "generate-gui-decorations",
                    str(ir),
                    "--manifest",
                    str(manifest),
                    "--output-dir",
                    str(output / "native"),
                    "--unit",
                    "button",
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(
                0, decoration_result.returncode,
                decoration_result.stdout + decoration_result.stderr,
            )
            self.assertTrue(
                (output / "native/button.gui_decorations.wf.cpp").is_file()
            )
            tree_result = subprocess.run(
                [
                    sys.executable,
                    str(ROOT / "tools/webforms.py"),
                    "generate-gui-tree",
                    str(ir),
                    "--manifest",
                    str(manifest),
                    "--output-dir",
                    str(output / "native"),
                    "--unit",
                    "button",
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(
                0, tree_result.returncode,
                tree_result.stdout + tree_result.stderr,
            )
            self.assertTrue((output / "native/button.gui_tree.wf.cpp").is_file())
            self.assertTrue((output / "native/button.gui_tree.json").is_file())


if __name__ == "__main__":
    unittest.main()
