"""Regression tests for compiler facts used by the published API reference."""
from __future__ import annotations
from typing import ClassVar
import re
import json
import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT: Path = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import api_reference
import api_reference_render
from api_reference_model import Inventory, Symbol, Contract, Coverage


class ApiReferenceTests(unittest.TestCase):
    scratch: ClassVar[tempfile.TemporaryDirectory[str]]
    root: ClassVar[Path]
    header: ClassVar[Path]
    data: ClassVar[Inventory]
    types: ClassVar[dict[str, Symbol]]
    @classmethod
    def setUpClass(cls) -> None:
        cls.scratch = tempfile.TemporaryDirectory(prefix='gui-forms-api-tests-')
        cls.addClassCleanup(cls.scratch.cleanup)
        cls.root = Path(cls.scratch.name)
        cls.header = cls.root / 'include/gui_forms/example.hpp'
        cls.header.parent.mkdir(parents=True)
        cls.header.write_text('''#pragma once
// UTF-8 before byte offsets: café, λ, 日本語.
namespace gui_forms {
namespace detail { struct Hidden {}; }
template <typename Value> class Example {
public:
    explicit Example(int count = 3) : count_(count) {}
    [[nodiscard]] int value() const { return count_; }
    void value(int next);
    template <typename Argument> void append(Argument item) { const int converted{static_cast<int>(item)}; count_ += converted; }
    struct Nested { int field{}; };
protected:
    virtual void extend() = 0;
private:
    int count_{};
    struct Private {};
    friend bool operator==(const Example& left, const Example& right) { const bool equal{left.count_ == right.count_}; return equal; }
};
[[nodiscard]] int process(int input = 7);
[[nodiscard]] int process(double input);
template <typename T> T copy(T value) { return value; }
using Count = unsigned;
inline constexpr int limit = 10;
}
namespace gui_drawing { struct Example { int visible{}; }; }
''', encoding="utf-8")
        result: subprocess.CompletedProcess[str] = subprocess.run(['clang++', '-std=c++20', '-x', 'c++', '-fsyntax-only', '-Wno-pragma-once-outside-header', '-Xclang', '-ast-dump=json', '-Xclang', '-ast-dump-filter=gui_', str(cls.header)], capture_output=True, text=True, check=True, encoding="utf-8")
        old: Path = api_reference.ROOT
        api_reference.ROOT = cls.root
        try:
            cls.data = api_reference.inventory(result.stdout)
        finally:
            api_reference.ROOT = old
        cls.types = {}
        record: Symbol
        for record in cls.data['types']:
            cls.types[record['id']] = record

    def named_function(self, name: str) -> Symbol:
        function: Symbol
        for function in self.data['functions']:
            if function['name'] == name:
                return function
        raise AssertionError('Fixture function missing: ' + name)

    def test_templates_byte_offsets_and_access(self) -> None:
        example: Symbol = self.types['gui_forms::Example']
        self.assertEqual(example['declaration'], 'template <typename Value> class Example')
        members: dict[str, Symbol] = {}
        member: Symbol
        for member in example['members']:
            members[member['name']] = member
        self.assertEqual(members['Example']['declaration'], 'explicit Example(int count = 3)')
        self.assertEqual(members['append']['declaration'], 'template <typename Argument> void append(Argument item)')
        self.assertEqual(members['extend']['access'], 'protected')
        self.assertNotIn('count_', members)
        self.assertNotIn('gui_forms::Example::Private', self.types)
        self.assertNotIn('gui_forms::detail::Hidden', self.types)
        nested: Symbol = self.types['gui_forms::Example::Nested']
        self.assertEqual(nested['namespace'], 'gui_forms')
        self.assertEqual(nested['enclosing_type'], 'gui_forms::Example')
        self.assertIn('gui_drawing::Example', self.types)

    def test_overloads_attributes_defaults_and_hidden_friend(self) -> None:
        functions: list[Symbol] = self.data['functions']
        process: list[Symbol] = []
        function: Symbol
        for function in functions:
            if function['name'] == 'process':
                process.append(function)
        self.assertEqual(len(process), 2)
        self.assertNotEqual(process[0]['page'], process[1]['page'])
        default_found: bool = False
        for function in process:
            self.assertTrue(function['declaration'].startswith('[[nodiscard]] int process'))
            parameter: dict[str, str]
            for parameter in function['parameters']:
                if parameter['declaration'] == 'int input = 7':
                    default_found = True
        self.assertTrue(default_found)
        friend: Symbol = self.named_function('operator==')
        self.assertEqual(friend['namespace'], 'gui_forms')
        self.assertIn('gui_forms::Example', friend['id'])
        self.assertNotIn('return', friend['declaration'])
        copy: Symbol = self.named_function('copy')
        self.assertEqual(copy['declaration'], 'template <typename T> T copy(T value)')
        self.assertEqual(len(self.data['aliases']), 2)

    def test_contracts_reject_stale_identifiers_and_wrong_parameters(self) -> None:
        directory: Path = self.root / 'contracts'
        directory.mkdir(exist_ok=True)
        path: Path = directory / 'test.json'
        path.write_text(json.dumps({'schema': 1, 'symbols': {'missing': {'summary': 'bad'}}}), encoding="utf-8")
        with self.assertRaisesRegex(ValueError, 'Stale contract'):
            api_reference_render.load_contracts(directory, self.data, self.root)
        symbol: Symbol = self.named_function('process')
        contract: Contract = dict(reviewed=True, summary='Processes input.', remarks='Example.', ownership='Value.', threading='Any.', availability='Core.', returns='Integer.', errors='None.', parameters={'wrong': 'Input.'})
        path.write_text(json.dumps({'schema': 1, 'symbols': {symbol['id']: contract}}), encoding="utf-8")
        with self.assertRaisesRegex(ValueError, 'Parameter contract differs'):
            api_reference_render.load_contracts(directory, self.data, self.root)
        path.unlink()

    def reference_contracts(self) -> dict[str, Contract]:
        symbol: Symbol = self.named_function('process')
        contract: Contract = {
            'reviewed': True, 'summary': 'Processes `input` <exactly>.',
            'remarks': ['First paragraph.', 'Second `paragraph`.'],
            'ownership': 'Value.', 'threading': 'Any.', 'availability': 'Core.',
            'returns': 'Integer.', 'errors': ['None.'],
            'parameters': {'input': 'Input <value>.'},
            'examples': [{'source': 'include/gui_forms/example.hpp', 'caption': 'UTF-8 example.'}],
            'see_also': ['gui_forms::Example'], 'contract_source': 'contracts/fixture.json'}
        result: dict[str, Contract] = {symbol['id']: contract}
        return result

    def test_reference_golden_utf8_text(self) -> None:
        golden_path: Path = ROOT / 'tests/api_reference_fixture_hashes.json'
        golden_text: str = golden_path.read_text(encoding='utf-8')
        expected: dict[str, str] = json.loads(golden_text)
        output: Path = self.root / 'golden-site'
        contracts: dict[str, Contract] = self.reference_contracts()
        api_reference_render.render(self.data, contracts, output, self.root)
        encoded: str = json.dumps(self.data, indent=2) + '\n'
        inventory_bytes: bytes = encoded.encode('utf-8')
        actual: str = hashlib.sha256(inventory_bytes).hexdigest()
        self.assertEqual(actual, expected['inventory.json'])
        name: str
        for name in expected:
            if name == 'inventory.json':
                continue
            artifact_path: Path = output / name
            # Text outputs use host newlines; compare canonical UTF-8 across OSes.
            artifact_text: str = artifact_path.read_text(encoding='utf-8')
            artifact_bytes: bytes = artifact_text.encode('utf-8')
            actual = hashlib.sha256(artifact_bytes).hexdigest()
            self.assertEqual(actual, expected[name], name)
        # search.js is the sole intentional output change under owner direction.
        artifacts: list[Path] = list(output.iterdir())
        self.assertEqual(len(artifacts), len(expected))

    def test_rendered_reference_links_and_escaping(self) -> None:
        output: Path = self.root / 'site'
        coverage: Coverage = api_reference_render.render(self.data, {}, output, self.root)
        self.assertEqual(coverage['reviewed_contracts'], 0)
        path: Path
        href: str
        for path in output.glob('*.html'):
            source: str = path.read_text(encoding="utf-8")
            for href in re.findall(r'href="([^"]+)"', source):
                if not href.startswith('#'):
                    self.assertTrue((output / href).exists(), (path.name, href))
            self.assertNotIn('template <typename', source)
        self.assertTrue((output / 'search-data.js').is_file())
        index_path: Path = output / 'index.html'
        index_text: str = index_path.read_text(encoding='utf-8')
        self.assertIn('GUI.Forms · C++ API reference', index_text)
        self.assertIn('Type, member, or header…', index_text)


if __name__ == '__main__':
    unittest.main()
