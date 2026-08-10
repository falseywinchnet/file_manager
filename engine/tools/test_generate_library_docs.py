import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("generate_library_docs.py")
SPEC = importlib.util.spec_from_file_location("engine_library_docs", SCRIPT)
DOCS = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
sys.modules[SPEC.name] = DOCS
SPEC.loader.exec_module(DOCS)


class LibraryDocumentationTests(unittest.TestCase):
    def test_every_package_has_reviewed_manual_metadata(self):
        packages = DOCS.discover_packages()
        manual = DOCS.load_manual()
        self.assertEqual(
            {package.import_path for package in packages},
            set(manual["packages"]),
        )

    def test_service_facade_is_split_into_navigable_declarations(self):
        service = next(
            package for package in DOCS.discover_packages()
            if package.import_path == "filemanager/engine/internal/service"
        )
        source_by_symbol = {
            (symbol.receiver, symbol.name): symbol.source
            for symbol in service.symbols if not symbol.test
        }
        self.assertEqual(source_by_symbol[("Service", "Query")], "internal/service/query.go")
        self.assertEqual(source_by_symbol[("Service", "Reconcile")], "internal/service/reconciliation.go")
        self.assertEqual(source_by_symbol[("Service", "PlanRoots")], "internal/service/roots.go")
        self.assertEqual(source_by_symbol[("Service", "Status")], "internal/service/status.go")
        self.assertEqual(source_by_symbol[("Service", "Configuration")], "internal/service/configuration.go")

    def test_public_contract_and_live_query_are_inventoried(self):
        packages = {package.import_path: package for package in DOCS.discover_packages()}
        api_names = {symbol.name for symbol in packages["filemanager/engine/api"].symbols if not symbol.test}
        live_names = {symbol.name for symbol in packages["filemanager/engine/internal/live"].symbols if not symbol.test}
        self.assertIn("Engine", api_names)
        self.assertIn("LiveQueryResponse", api_names)
        self.assertIn("Manager", live_names)
        self.assertIn("NewManager", live_names)

    def test_generated_manifest_navigates_to_every_page(self):
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "library"
            package_count, _ = DOCS.generate(output)
            manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
            self.assertEqual(len(manifest), package_count + 3)
            for entry in manifest:
                self.assertTrue((output / entry["page"]).is_file(), entry["page"])
                self.assertTrue(entry["search"])
            index = (output / "index.html").read_text(encoding="utf-8")
            self.assertIn("manifest.js", index)
            self.assertIn("assets/library.js", index)


if __name__ == "__main__":
    unittest.main()
