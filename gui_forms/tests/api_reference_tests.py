"""Regression tests for compiler facts used by the published API reference."""
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import api_reference
import api_reference_render


class ApiReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.scratch = tempfile.TemporaryDirectory(prefix='gui-forms-api-tests-')
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
    template <typename Argument> void append(Argument item) { count_ += int(item); }
    struct Nested { int field{}; };
protected:
    virtual void extend() = 0;
private:
    int count_{};
    struct Private {};
    friend bool operator==(const Example& left, const Example& right) { return left.count_ == right.count_; }
};
[[nodiscard]] int process(int input = 7);
[[nodiscard]] int process(double input);
template <typename T> T copy(T value) { return value; }
using Count = unsigned;
inline constexpr int limit = 10;
}
namespace gui_drawing { struct Example { int visible{}; }; }
''')
        result = subprocess.run(['clang++', '-std=c++20', '-x', 'c++', '-fsyntax-only', '-Wno-pragma-once-outside-header', '-Xclang', '-ast-dump=json', '-Xclang', '-ast-dump-filter=gui_', str(cls.header)], capture_output=True, text=True, check=True)
        old = api_reference.ROOT
        api_reference.ROOT = cls.root
        try: cls.data = api_reference.inventory(result.stdout)
        finally: api_reference.ROOT = old
        cls.types = {t['id']: t for t in cls.data['types']}

    @classmethod
    def tearDownClass(cls): cls.scratch.cleanup()

    def test_templates_byte_offsets_and_access(self):
        example = self.types['gui_forms::Example']
        self.assertEqual(example['declaration'], 'template <typename Value> class Example')
        members = {m['name']: m for m in example['members']}
        self.assertEqual(members['Example']['declaration'], 'explicit Example(int count = 3)')
        self.assertEqual(members['append']['declaration'], 'template <typename Argument> void append(Argument item)')
        self.assertEqual(members['extend']['access'], 'protected')
        self.assertNotIn('count_', members)
        self.assertNotIn('gui_forms::Example::Private', self.types)
        self.assertNotIn('gui_forms::detail::Hidden', self.types)
        nested = self.types['gui_forms::Example::Nested']
        self.assertEqual(nested['namespace'], 'gui_forms')
        self.assertEqual(nested['enclosing_type'], 'gui_forms::Example')
        self.assertIn('gui_drawing::Example', self.types)

    def test_overloads_attributes_defaults_and_hidden_friend(self):
        functions = self.data['functions']
        process = [f for f in functions if f['name'] == 'process']
        self.assertEqual(len(process), 2)
        self.assertNotEqual(process[0]['page'], process[1]['page'])
        self.assertTrue(all(f['declaration'].startswith('[[nodiscard]] int process') for f in process))
        self.assertTrue(any(p['declaration'] == 'int input = 7' for f in process for p in f['parameters']))
        friend = next(f for f in functions if f['name'] == 'operator==')
        self.assertEqual(friend['namespace'], 'gui_forms')
        self.assertIn('gui_forms::Example', friend['id'])
        self.assertNotIn('return', friend['declaration'])
        copy = next(f for f in functions if f['name'] == 'copy')
        self.assertEqual(copy['declaration'], 'template <typename T> T copy(T value)')
        self.assertEqual(len(self.data['aliases']), 2)

    def test_contracts_reject_stale_identifiers_and_wrong_parameters(self):
        directory = self.root / 'contracts'
        directory.mkdir(exist_ok=True)
        path = directory / 'test.json'
        path.write_text(json.dumps({'schema': 1, 'symbols': {'missing': {'summary': 'bad'}}}))
        with self.assertRaisesRegex(ValueError, 'Stale contract'):
            api_reference_render.load_contracts(directory, self.data, self.root)
        symbol = next(f for f in self.data['functions'] if f['name'] == 'process')
        contract = dict(reviewed=True, summary='Processes input.', remarks='Example.', ownership='Value.', threading='Any.', availability='Core.', returns='Integer.', errors='None.', parameters={'wrong': 'Input.'})
        path.write_text(json.dumps({'schema': 1, 'symbols': {symbol['id']: contract}}))
        with self.assertRaisesRegex(ValueError, 'Parameter contract differs'):
            api_reference_render.load_contracts(directory, self.data, self.root)
        path.unlink()

    def test_rendered_reference_links_and_escaping(self):
        output = self.root / 'site'
        coverage = api_reference_render.render(self.data, {}, output, self.root)
        self.assertEqual(coverage['reviewed_contracts'], 0)
        for path in output.glob('*.html'):
            import re
            source = path.read_text()
            for href in re.findall(r'href="([^"]+)"', source):
                if not href.startswith('#'):
                    self.assertTrue((output / href).exists(), (path.name, href))
            self.assertNotIn('template <typename', source)
        self.assertTrue((output / 'search-data.js').is_file())


if __name__ == '__main__': unittest.main()
