"""Render compiler facts and separately authored contracts without inferred prose."""
from __future__ import annotations
import html
import json
from pathlib import Path
import re
from collections.abc import Iterator
from typing import cast
from api_reference_model import Inventory, Symbol, Contract, ContractFile, Coverage, Parameter, Example

STYLE: str = """
:root{color-scheme:light;--ink:#172b3a;--muted:#52616e;--line:#d8e1e8;--link:#075aab;--bg:#f5f8fa}
*{box-sizing:border-box}body{margin:0;color:var(--ink);background:white;font:16px/1.55 system-ui,-apple-system,Segoe UI,sans-serif}
a{color:var(--link);text-decoration:none}a:hover{text-decoration:underline}a:focus-visible,button:focus-visible,input:focus-visible{outline:3px solid #c77400;outline-offset:3px}
header{background:#132f47;color:white;padding:16px 32px;display:flex;justify-content:space-between;gap:20px}header a{color:white;font-weight:650}header span{color:#cfdeea}
.layout{max-width:1500px;margin:auto;display:grid;grid-template-columns:245px minmax(0,1fr);min-height:90vh}aside{padding:28px 22px;background:var(--bg);border-right:1px solid var(--line)}aside nav{position:sticky;top:20px}aside a{display:block;margin:10px 0}aside input{width:100%;border:1px solid #9aafbe;border-radius:3px;padding:10px;font:inherit}aside label{display:block;font-weight:600;margin-bottom:6px}aside small{display:block;color:var(--muted);margin:16px 0}
main{padding:30px 48px 70px;min-width:0;max-width:1120px}.crumb{font-size:14px;color:var(--muted);margin-bottom:24px}h1{font-size:34px;line-height:1.2;font-weight:650;overflow-wrap:anywhere;margin:8px 0 18px}h2{font-size:23px;font-weight:650;margin:32px 0 12px;scroll-margin-top:20px}h3{font-size:18px}.lead{font-size:18px;max-width:78ch}p{max-width:88ch}pre{background:#f4f7fa;border:1px solid var(--line);border-left:3px solid #2773af;padding:18px;overflow:auto;border-radius:3px;font-size:14px;line-height:1.6;white-space:pre-wrap;overflow-wrap:anywhere}code{font-family:ui-monospace,SFMono-Regular,Consolas,monospace;font-size:.91em}p code,td code{background:#eff3f6;padding:1px 4px;overflow-wrap:anywhere}.metadata{color:var(--muted);font-size:14px;overflow-wrap:anywhere}.status{padding:12px 16px;border-left:3px solid #b88a39;background:#fff9ee;font-size:14px}.status.complete{border-color:#378458;background:#f0f8f3}table{border-collapse:collapse;width:100%;margin:14px 0;font-size:14px}th{text-align:left;background:#f4f7fa}td,th{padding:12px;border-bottom:1px solid var(--line);vertical-align:top}td:first-child{min-width:170px}td code{white-space:normal}.symbol{overflow-wrap:anywhere}.dim{color:var(--muted)}ul{padding-left:23px}li{margin:7px 0}.search-results{list-style:none;padding:0}.search-results li{border-bottom:1px solid var(--line);padding:14px 0}.search-results code{display:block;font-size:13px}.search-results p{margin:5px 0}.empty{color:var(--muted)}footer{margin-top:45px;padding-top:15px;border-top:1px solid var(--line);font-size:13px;color:var(--muted)}
@media(max-width:820px){header{padding:14px 20px}header span{display:none}.layout{display:block}aside{padding:16px 20px;border-right:0;border-bottom:1px solid var(--line)}aside nav{position:static}aside nav>a{display:inline-block;margin:8px 18px 0 0}aside small{display:none}main{padding:25px 20px}h1{font-size:28px}table{display:block;overflow:auto}aside input{max-width:400px}}
@media print{aside,header,.crumb{display:none}.layout{display:block}main{max-width:none;padding:0}pre{break-inside:avoid}a{color:inherit}}
"""



def escape(value: object) -> str:
    text: str = str(value)
    result: str = html.escape(text, quote=True)
    return result


def prose(value: object) -> str:
    escaped: str = escape(value)
    result: str = re.sub(r'`([^`]+)`', r'<code>\1</code>', escaped)
    return result


def paragraphs(values: str | list[str]) -> str:
    items: list[str] = []
    if isinstance(values, str):
        items = [values]
    else:
        items = values
    parts: list[str] = []
    value: str
    for value in items:
        parts.append('<p>' + prose(value) + '</p>')
    result: str = ''.join(parts)
    return result


def all_symbols(data: Inventory) -> Iterator[Symbol]:
    entry: Symbol
    for entry in data['types']:
        yield entry
        yield from entry['members']
    yield from data['functions']
    yield from data['aliases']


def symbol_index(data: Inventory) -> dict[str, Symbol]:
    symbols: dict[str, Symbol] = {}
    symbol: Symbol
    for symbol in all_symbols(data):
        symbols[symbol['id']] = symbol
    return symbols


def load_contracts(directory: Path, data: Inventory, root: Path) -> dict[str, Contract]:
    contracts: dict[str, Contract] = {}
    path: Path
    identifier: str
    for path in sorted(directory.glob('*.json')):
        text: str = path.read_text(encoding='utf-8')
        contents: ContractFile = cast(ContractFile, json.loads(text))
        if contents.get('schema') != 1:
            raise ValueError('Unknown contract schema: ' + str(path))
        for identifier in contents['symbols']:
            if identifier in contracts:
                raise ValueError('Duplicate contract: ' + identifier)
            contract: Contract = contents['symbols'][identifier].copy()
            relative: Path = path.relative_to(root)
            contract['contract_source'] = str(relative)
            contracts[identifier] = contract
    symbols: dict[str, Symbol] = symbol_index(data)
    required: set[str] = {'summary', 'remarks', 'ownership', 'threading', 'availability'}
    for identifier in contracts:
        contract = contracts[identifier]
        if identifier not in symbols:
            raise ValueError('Stale contract: ' + identifier)
        symbol: Symbol = symbols[identifier]
        if contract.get('reviewed'):
            missing: set[str] = required - contract.keys()
            if symbol['kind'] in {'FunctionDecl', 'FunctionTemplateDecl', 'CXXMethodDecl', 'CXXConstructorDecl', 'CXXDestructorDecl'}:
                missing |= {'returns', 'errors', 'parameters'} - contract.keys()
            if missing:
                missing_names: str = ', '.join(sorted(missing))
                raise ValueError('Incomplete reviewed contract ' + identifier + ': ' + missing_names)
            parameters: list[Parameter] = symbol.get('parameters', [])
            parameter_names: set[str] = set()
            index: int = 0
            for index in range(len(parameters)):
                name: str = parameters[index]['name']
                if not name:
                    name = '#' + str(index + 1)
                parameter_names.add(name)
            if set(contract.get('parameters', {})) != parameter_names:
                raise ValueError('Parameter contract differs from declaration: ' + identifier)
            key: str
            for key in required:
                if not contract[key]:
                    raise ValueError('Empty contract section ' + key + ': ' + identifier)
        related: str
        for related in contract.get('see_also', []):
            if related not in symbols:
                raise ValueError('Unknown related symbol: ' + related)
        example: Example
        for example in contract.get('examples', []):
            candidate: Path = root / example['source']
            path = candidate.resolve()
            resolved_root: Path = root.resolve()
            if not path.is_relative_to(resolved_root) or not path.is_file():
                raise ValueError('Missing example: ' + str(path))
    return contracts


class ReferencePages:
    """Own rendered strings for one call; borrow immutable symbol/contract maps."""
    def __init__(self, symbols: dict[str, Symbol], contracts: dict[str, Contract]) -> None:
        self.symbols: dict[str, Symbol] = symbols
        self.contracts: dict[str, Contract] = contracts
        self.pages: dict[str, str] = {}

    def link(self, identifier: str, label: str | None = None) -> str:
        symbol: Symbol | None = self.symbols.get(identifier)
        if symbol is None:
            missing: str = '<code>' + escape(label or identifier) + '</code>'
            return missing
        result: str = '<a href="' + escape(symbol['page']) + '">' + escape(label or identifier.split('|')[0]) + '</a>'
        return result
    def page(self, filename: str, title: str, body: str, crumb: str = '', sections: tuple[tuple[str, str], ...] | list[tuple[str, str]] = ()) -> None:
        links: list[str] = []
        section: tuple[str, str]
        for section in sections:
            links.append('<a href="#' + section[0] + '">' + section[1] + '</a>')
        toc: str = ''.join(links)
        self.pages[filename] = ('<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">'
            '<title>' + escape(title) + ' · GUI.Forms</title><link rel="stylesheet" href="reference.css"></head><body>'
            '<header><a href="index.html">GUI.Forms · C++ API reference</a><span>0.1 · Native, retained, CPU rendered</span></header>'
            '<div class="layout"><aside><nav aria-label="Reference navigation"><label for="api-search">Search the API</label>'
            '<input id="api-search" type="search" placeholder="Type, member, or header…" autocomplete="off">'
            '<a href="index.html">Namespaces &amp; types</a><a href="coverage.html">Contract coverage</a><small>On this page</small>' + toc +
            '</nav></aside><main><section id="search-results" aria-live="polite" hidden></section><article id="page-content">'
            '<div class="crumb"><a href="index.html">Reference</a>' + crumb + '</div><h1>' + escape(title) + '</h1>' + body +
            '<footer>Declarations are extracted from the public C++20 headers. Authored contracts are tracked separately. '
            'Protected members are extension points. The C ABI and managed experiments have separate references.</footer>'
            '</article></main></div><script src="search-data.js"></script><script src="search.js"></script></body></html>')
    def table(self, entries: list[Symbol]) -> str:
        if not entries:
            return '<p class="empty">No declarations in this category.</p>'
        rows: list[str] = []
        symbol: Symbol
        for symbol in entries:
            contract: Contract = self.contracts.get(symbol['id'], {})
            summary: str = contract.get('summary') or symbol.get('comment') or 'Declaration available; behavioral contract pending.'
            rows.append('<tr><td class="symbol">' + self.link(symbol['id'], symbol['name']) + '</td><td><code>' + escape(symbol['declaration']) + '</code><p>' + prose(summary) + '</p></td></tr>')
        result: str = '<table><thead><tr><th>API</th><th>Declaration and description</th></tr></thead><tbody>' + ''.join(rows) + '</tbody></table>'
        return result


def render(data: Inventory, contracts: dict[str, Contract], output: Path, root: Path) -> Coverage:
    """Borrow inputs; own pages through emission. I/O failure leaves partial output.

    Only files listed by the earlier page manifest are eligible for stale-page
    removal. This generates a local projection; it does not publish a site.
    """
    output.mkdir(parents=True, exist_ok=True)
    symbols: dict[str, Symbol] = symbol_index(data)
    namespace_symbols: list[Symbol] = data['types'] + data['functions'] + data['aliases']
    namespace_names: set[str] = set()
    symbol: Symbol
    for symbol in namespace_symbols:
        namespace_names.add(symbol['namespace'])
    namespaces: list[str] = sorted(namespace_names)
    from api_reference import slug
    writer: ReferencePages = ReferencePages(symbols, contracts)
    reviewed: int = 0
    missing_contracts: list[str] = []
    symbol: Symbol
    for symbol in symbols.values():
        if contracts.get(symbol['id'], {}).get('reviewed'):
            reviewed += 1
        else:
            missing_contracts.append(symbol['id'])
    member_count: int = 0
    record: Symbol
    for record in data['types']:
        member_count += len(record['members'])
    coverage: Coverage = {'schema': 1, 'types': len(data['types']), 'declared_members': member_count,
                'namespace_functions': len(data['functions']), 'namespace_aliases_and_constants': len(data['aliases']),
                'symbols': len(symbols), 'reviewed_contracts': reviewed, 'pending_contracts': len(symbols) - reviewed,
                'missing': missing_contracts}
    intro: str = '<p class="lead">Types, methods, functions, events, and values for building native applications with GUI.Forms and GUI.Drawing.</p>'
    intro += '<p>The reference separates exact declarations from reviewed behavioral contracts. ' + str(reviewed) + ' of ' + str(len(symbols)) + ' declarations currently have reviewed contracts. See <a href="coverage.html">coverage</a> for the remaining work.</p>'
    intro += '<h2 id="namespaces">Namespaces</h2><table><thead><tr><th>Namespace</th><th>Declarations</th></tr></thead><tbody>'
    namespace: str
    for namespace in namespaces:
        entries: list[Symbol] = []
        for symbol in namespace_symbols:
            if symbol['namespace'] == namespace:
                entries.append(symbol)
        intro += '<tr><td><a href="namespace-' + slug(namespace) + '.html">' + escape(namespace) + '</a></td><td>' + str(len(entries)) + '</td></tr>'
        type_entries: list[Symbol] = []
        function_entries: list[Symbol] = []
        alias_entries: list[Symbol] = []
        for symbol in entries:
            if 'members' in symbol:
                type_entries.append(symbol)
            if symbol in data['functions']:
                function_entries.append(symbol)
            if symbol in data['aliases']:
                alias_entries.append(symbol)
        body: str = '<h2 id="types">Types</h2>' + writer.table(type_entries)
        body += '<h2 id="functions">Functions</h2>' + writer.table(function_entries)
        body += '<h2 id="values">Aliases and constants</h2>' + writer.table(alias_entries)
        writer.page('namespace-' + slug(namespace) + '.html', namespace + ' namespace', body,
             sections=(('types', 'Types'), ('functions', 'Functions'), ('values', 'Aliases and constants')))
    intro += '</tbody></table>'
    writer.page('index.html', 'Native C++ library reference', intro, sections=(('namespaces', 'Namespaces'),))
    parents: dict[str, Symbol] = {}
    member: Symbol
    for record in data['types']:
        for member in record['members']:
            parents[member['id']] = record
    for symbol in symbols.values():
        contract: Contract = contracts.get(symbol['id'], {})
        summary: str | None = contract.get('summary') or symbol.get('comment')
        body = '<p class="lead">' + prose(summary) + '</p>' if summary else ''
        body += '<p class="metadata">Header: <code>&lt;' + escape(symbol['header']) + '&gt;</code> · ' + escape(symbol['kind']) + ' · ' + escape(symbol.get('access', 'public')) + '</p>'
        if contract.get('reviewed'):
            body += '<p class="status complete">Reviewed contract. Source: <code>' + escape(contract['contract_source']) + '</code>.</p>'
        else:
            body += '<p class="status">Declaration reference. A complete behavioral contract has not yet been reviewed for this API.</p>'
        sections: list[tuple[str, str]] = [('syntax', 'Syntax')]
        body += '<h2 id="syntax">Syntax</h2><pre><code>' + escape(symbol['declaration']) + (';' if 'members' not in symbol else '') + '</code></pre>'
        if symbol.get('bases'):
            base_links: list[str] = []
            base: str
            for base in symbol['bases']:
                base_links.append(writer.link(base))
            body += '<p>Inherits: ' + ', '.join(base_links) + '.</p>'
        if symbol.get('parameters'):
            sections.append(('parameters', 'Parameters'))
            body += '<h2 id="parameters">Parameters</h2><table><thead><tr><th>Name and type</th><th>Contract</th></tr></thead><tbody>'
            i: int
            parameter: Parameter
            for i, parameter in enumerate(symbol['parameters']):
                name: str = parameter['name'] or '#' + str(i + 1)
                body += '<tr><td><code>' + escape(parameter['declaration']) + '</code></td><td>' + prose(contract.get('parameters', {}).get(name, 'Behavioral contract pending.')) + '</td></tr>'
            body += '</tbody></table>'
        key: str
        title: str
        for key, title in [('returns', 'Return value'), ('errors', 'Errors and exceptions'), ('remarks', 'Remarks'), ('ownership', 'Ownership and lifetime'), ('threading', 'Thread safety'), ('availability', 'Availability')]:
            if key not in contract:
                continue
            sections.append((key, title))
            body += '<h2 id="' + key + '">' + title + '</h2>' + paragraphs(contract[key])
        if 'members' in symbol:
            access: str
            for access in ['public', 'protected']:
                entries = []
                for member in symbol['members']:
                    if member['access'] == access:
                        entries.append(member)
                if entries:
                    sections.append((access, access.title() + ' members'))
                    body += '<h2 id="' + access + '">' + access.title() + ' members</h2>' + writer.table(entries)
        if contract.get('examples'):
            sections.append(('examples', 'Examples'))
            body += '<h2 id="examples">Examples</h2>'
            example: Example
            for example in contract['examples']:
                example_path: Path = root / example['source']
                example_text: str = example_path.read_text(encoding='utf-8')
                body += paragraphs(example['caption']) + '<p class="metadata">' + escape(example['source']) + '</p><pre><code>' + escape(example_text) + '</code></pre>'
        if contract.get('see_also'):
            sections.append(('related', 'See also'))
            related_links: list[str] = []
            related: str
            for related in contract['see_also']:
                related_links.append('<li>' + writer.link(related) + '</li>')
            body += '<h2 id="related">See also</h2><ul>' + ''.join(related_links) + '</ul>'
        parent: Symbol | None = parents.get(symbol['id'])
        crumb: str = ' / ' + writer.link(parent['id'], parent['name']) if parent else ''
        writer.page(symbol['page'], symbol['id'].split('|')[0], body, crumb, sections)
    body = '<p class="lead">Compiler coverage and behavioral documentation are measured independently.</p>'
    body += '<table><thead><tr><th>Measure</th><th>Count</th></tr></thead><tbody>'
    value: int | list[str]
    for key, value in coverage.items():
        if key not in {'schema', 'missing'}:
            body += '<tr><td>' + escape(key.replace('_', ' ').capitalize()) + '</td><td>' + str(value) + '</td></tr>'
    body += '</tbody></table><h2 id="pending">Pending contracts</h2><p>This list is work remaining, not a statement that the APIs are unimplemented. Search the API to find individual declarations.</p><ul>'
    pending_rows: list[str] = []
    for record in data['types']:
        pending: int = 0
        for member in record['members']:
            if member['id'] in coverage['missing']:
                pending += 1
        pending_rows.append('<li>' + writer.link(record['id']) + ': ' + str(pending) + ' member contracts pending.</li>')
    body += ''.join(pending_rows)
    body += '</ul>'
    writer.page('coverage.html', 'Reference coverage', body, sections=(('pending', 'Pending contracts'),))
    filename: str
    for filename in writer.pages:
        content: str = writer.pages[filename]
        page_path: Path = output / filename
        page_path.write_text(content + '\n', encoding='utf-8')
    # This directory is entirely generated; only remove pages owned by an earlier manifest.
    manifest: Path = output / 'pages.json'
    if manifest.exists():
        manifest_text: str = manifest.read_text(encoding='utf-8')
        previous_pages: list[str] = cast(list[str], json.loads(manifest_text))
        stale_pages: set[str] = set(previous_pages) - set(writer.pages)
        stale: str
        for stale in stale_pages:
            if Path(stale).name == stale and stale.endswith('.html'):
                stale_path: Path = output / stale
                stale_path.unlink(missing_ok=True)
    page_names: list[str] = sorted(writer.pages)
    manifest_text = json.dumps(page_names, indent=2) + '\n'
    manifest.write_text(manifest_text, encoding='utf-8')
    stylesheet: Path = output / 'reference.css'
    stylesheet.write_text(STYLE, encoding='utf-8')
    search_source: Path = Path(__file__).resolve().parent / 'api_reference_search.js'
    search_script: str = search_source.read_text(encoding='utf-8-sig')
    search_destination: Path = output / 'search.js'
    search_destination.write_text(search_script, encoding='utf-8')
    search: list[dict[str, str]] = []
    for symbol in symbols.values():
        search_record: dict[str, str] = {}
        key: str
        for key in ('id', 'header', 'page', 'declaration'):
            search_record[key] = symbol[key]
        search_record['summary'] = contracts.get(symbol['id'], {}).get('summary', '')
        search.append(search_record)
    search_data: Path = output / 'search-data.js'
    search_json: str = json.dumps(search, ensure_ascii=True)
    search_data.write_text('window.GUIFormsSearch=' + search_json + ';\n', encoding='utf-8')
    coverage_path: Path = output / 'coverage.json'
    coverage_text: str = json.dumps(coverage, indent=2) + '\n'
    coverage_path.write_text(coverage_text, encoding='utf-8')
    return coverage
