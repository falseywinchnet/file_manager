"""Render compiler facts and separately authored contracts without inferred prose."""
from __future__ import annotations
import html
import json
from pathlib import Path
import re

STYLE = """
:root{color-scheme:light;--ink:#172b3a;--muted:#52616e;--line:#d8e1e8;--link:#075aab;--bg:#f5f8fa}
*{box-sizing:border-box}body{margin:0;color:var(--ink);background:white;font:16px/1.55 system-ui,-apple-system,Segoe UI,sans-serif}
a{color:var(--link);text-decoration:none}a:hover{text-decoration:underline}a:focus-visible,button:focus-visible,input:focus-visible{outline:3px solid #c77400;outline-offset:3px}
header{background:#132f47;color:white;padding:16px 32px;display:flex;justify-content:space-between;gap:20px}header a{color:white;font-weight:650}header span{color:#cfdeea}
.layout{max-width:1500px;margin:auto;display:grid;grid-template-columns:245px minmax(0,1fr);min-height:90vh}aside{padding:28px 22px;background:var(--bg);border-right:1px solid var(--line)}aside nav{position:sticky;top:20px}aside a{display:block;margin:10px 0}aside input{width:100%;border:1px solid #9aafbe;border-radius:3px;padding:10px;font:inherit}aside label{display:block;font-weight:600;margin-bottom:6px}aside small{display:block;color:var(--muted);margin:16px 0}
main{padding:30px 48px 70px;min-width:0;max-width:1120px}.crumb{font-size:14px;color:var(--muted);margin-bottom:24px}h1{font-size:34px;line-height:1.2;font-weight:650;overflow-wrap:anywhere;margin:8px 0 18px}h2{font-size:23px;font-weight:650;margin:32px 0 12px;scroll-margin-top:20px}h3{font-size:18px}.lead{font-size:18px;max-width:78ch}p{max-width:88ch}pre{background:#f4f7fa;border:1px solid var(--line);border-left:3px solid #2773af;padding:18px;overflow:auto;border-radius:3px;font-size:14px;line-height:1.6;white-space:pre-wrap;overflow-wrap:anywhere}code{font-family:ui-monospace,SFMono-Regular,Consolas,monospace;font-size:.91em}p code,td code{background:#eff3f6;padding:1px 4px;overflow-wrap:anywhere}.metadata{color:var(--muted);font-size:14px;overflow-wrap:anywhere}.status{padding:12px 16px;border-left:3px solid #b88a39;background:#fff9ee;font-size:14px}.status.complete{border-color:#378458;background:#f0f8f3}table{border-collapse:collapse;width:100%;margin:14px 0;font-size:14px}th{text-align:left;background:#f4f7fa}td,th{padding:12px;border-bottom:1px solid var(--line);vertical-align:top}td:first-child{min-width:170px}td code{white-space:normal}.symbol{overflow-wrap:anywhere}.dim{color:var(--muted)}ul{padding-left:23px}li{margin:7px 0}.search-results{list-style:none;padding:0}.search-results li{border-bottom:1px solid var(--line);padding:14px 0}.search-results code{display:block;font-size:13px}.search-results p{margin:5px 0}.empty{color:var(--muted)}footer{margin-top:45px;padding-top:15px;border-top:1px solid var(--line);font-size:13px;color:var(--muted)}
@media(max-width:820px){header{padding:14px 20px}header span{display:none}.layout{display:block}aside{padding:16px 20px;border-right:0;border-bottom:1px solid var(--line)}aside nav{position:static}aside nav>a{display:inline-block;margin:8px 18px 0 0}aside small{display:none}main{padding:25px 20px}h1{font-size:28px}table{display:block;overflow:auto}aside input{max-width:400px}}
@media print{aside,header,.crumb{display:none}.layout{display:block}main{max-width:none;padding:0}pre{break-inside:avoid}a{color:inherit}}
"""
SEARCH = """'use strict';
const box=document.getElementById('api-search');
const results=document.getElementById('search-results');
const content=document.getElementById('page-content');
const query=new URLSearchParams(location.search).get('q')||'';
function search(){const q=box.value.toLowerCase().trim();results.replaceChildren();content.hidden=Boolean(q);results.hidden=!q;if(!q)return;const words=q.split(/\\s+/);const matches=window.GUIFormsSearch.filter(s=>words.every(w=>(s.id+' '+s.summary+' '+s.header).toLowerCase().includes(w)));const count=document.createElement('p');count.textContent=matches.length+' matching declarations'+(matches.length>100?' (first 100 shown)':'');results.append(count);const list=document.createElement('ul');list.className='search-results';for(const s of matches.slice(0,100)){const li=document.createElement('li');const a=document.createElement('a');a.href=s.page;a.textContent=s.id.split('|')[0];li.append(a);const code=document.createElement('code');code.textContent=s.declaration;li.append(code);if(s.summary){const p=document.createElement('p');p.textContent=s.summary;li.append(p);}list.append(li);}results.append(list);}
box.value=query;box.addEventListener('input',search);search();
"""


def escape(value):
    return html.escape(str(value), quote=True)


def prose(value):
    return re.sub(r'`([^`]+)`', r'<code>\1</code>', escape(value))


def paragraphs(values):
    if isinstance(values, str): values = [values]
    return ''.join('<p>' + prose(value) + '</p>' for value in values)


def all_symbols(data):
    for entry in data['types']:
        yield entry
        yield from entry['members']
    yield from data['functions']
    yield from data['aliases']


def load_contracts(directory, data, root):
    contracts = {}
    for path in sorted(directory.glob('*.json')):
        contents = json.loads(path.read_text(encoding="utf-8"))
        if contents.get('schema') != 1: raise ValueError('Unknown contract schema: ' + str(path))
        for identifier, contract in contents['symbols'].items():
            if identifier in contracts: raise ValueError('Duplicate contract: ' + identifier)
            contract = dict(contract, contract_source=str(path.relative_to(root)))
            contracts[identifier] = contract
    symbols = {s['id']: s for s in all_symbols(data)}
    required = {'summary', 'remarks', 'ownership', 'threading', 'availability'}
    for identifier, contract in contracts.items():
        if identifier not in symbols: raise ValueError('Stale contract: ' + identifier)
        symbol = symbols[identifier]
        if contract.get('reviewed'):
            missing = required - contract.keys()
            if symbol['kind'] in {'FunctionDecl', 'FunctionTemplateDecl', 'CXXMethodDecl', 'CXXConstructorDecl', 'CXXDestructorDecl'}:
                missing |= {'returns', 'errors', 'parameters'} - contract.keys()
            if missing: raise ValueError('Incomplete reviewed contract ' + identifier + ': ' + ', '.join(sorted(missing)))
            params = {p['name'] or '#' + str(i + 1) for i, p in enumerate(symbol.get('parameters', []))}
            if set(contract.get('parameters', {})) != params:
                raise ValueError('Parameter contract differs from declaration: ' + identifier)
            for key in required:
                if not contract[key]: raise ValueError('Empty contract section ' + key + ': ' + identifier)
        for related in contract.get('see_also', []):
            if related not in symbols: raise ValueError('Unknown related symbol: ' + related)
        for example in contract.get('examples', []):
            path = (root / example['source']).resolve()
            if not path.is_relative_to(root.resolve()) or not path.is_file():
                raise ValueError('Missing example: ' + str(path))
    return contracts


def render(data, contracts, output, root):
    output.mkdir(parents=True, exist_ok=True)
    symbols = {s['id']: s for s in all_symbols(data)}
    namespaces = sorted({s['namespace'] for s in data['types'] + data['functions'] + data['aliases']})
    from api_reference import slug
    pages = {}
    def link(identifier, label=None):
        symbol = symbols.get(identifier)
        if symbol is None: return '<code>' + escape(label or identifier) + '</code>'
        return '<a href="' + escape(symbol['page']) + '">' + escape(label or identifier.split('|')[0]) + '</a>'
    def page(filename, title, body, crumb='', sections=()):
        toc = ''.join('<a href="#' + section[0] + '">' + section[1] + '</a>' for section in sections)
        pages[filename] = ('<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">'
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
    def table(entries):
        if not entries: return '<p class="empty">No declarations in this category.</p>'
        rows = []
        for symbol in entries:
            contract = contracts.get(symbol['id'], {})
            summary = contract.get('summary') or symbol.get('comment') or 'Declaration available; behavioral contract pending.'
            rows.append('<tr><td class="symbol">' + link(symbol['id'], symbol['name']) + '</td><td><code>' + escape(symbol['declaration']) + '</code><p>' + prose(summary) + '</p></td></tr>')
        return '<table><thead><tr><th>API</th><th>Declaration and description</th></tr></thead><tbody>' + ''.join(rows) + '</tbody></table>'
    reviewed = sum(bool(contracts.get(s['id'], {}).get('reviewed')) for s in symbols.values())
    coverage = {'schema': 1, 'types': len(data['types']), 'declared_members': sum(len(t['members']) for t in data['types']),
                'namespace_functions': len(data['functions']), 'namespace_aliases_and_constants': len(data['aliases']),
                'symbols': len(symbols), 'reviewed_contracts': reviewed, 'pending_contracts': len(symbols) - reviewed,
                'missing': [s['id'] for s in symbols.values() if not contracts.get(s['id'], {}).get('reviewed')]}
    intro = '<p class="lead">Types, methods, functions, events, and values for building native applications with GUI.Forms and GUI.Drawing.</p>'
    intro += '<p>The reference separates exact declarations from reviewed behavioral contracts. ' + str(reviewed) + ' of ' + str(len(symbols)) + ' declarations currently have reviewed contracts. See <a href="coverage.html">coverage</a> for the remaining work.</p>'
    intro += '<h2 id="namespaces">Namespaces</h2><table><thead><tr><th>Namespace</th><th>Declarations</th></tr></thead><tbody>'
    for namespace in namespaces:
        entries = [s for s in data['types'] + data['functions'] + data['aliases'] if s['namespace'] == namespace]
        intro += '<tr><td><a href="namespace-' + slug(namespace) + '.html">' + escape(namespace) + '</a></td><td>' + str(len(entries)) + '</td></tr>'
        body = '<h2 id="types">Types</h2>' + table([s for s in entries if 'members' in s])
        body += '<h2 id="functions">Functions</h2>' + table([s for s in entries if s in data['functions']])
        body += '<h2 id="values">Aliases and constants</h2>' + table([s for s in entries if s in data['aliases']])
        page('namespace-' + slug(namespace) + '.html', namespace + ' namespace', body,
             sections=(('types', 'Types'), ('functions', 'Functions'), ('values', 'Aliases and constants')))
    intro += '</tbody></table>'
    page('index.html', 'Native C++ library reference', intro, sections=(('namespaces', 'Namespaces'),))
    parents = {m['id']: t for t in data['types'] for m in t['members']}
    for symbol in symbols.values():
        contract = contracts.get(symbol['id'], {})
        summary = contract.get('summary') or symbol.get('comment')
        body = '<p class="lead">' + prose(summary) + '</p>' if summary else ''
        body += '<p class="metadata">Header: <code>&lt;' + escape(symbol['header']) + '&gt;</code> · ' + escape(symbol['kind']) + ' · ' + escape(symbol.get('access', 'public')) + '</p>'
        if contract.get('reviewed'):
            body += '<p class="status complete">Reviewed contract. Source: <code>' + escape(contract['contract_source']) + '</code>.</p>'
        else:
            body += '<p class="status">Declaration reference. A complete behavioral contract has not yet been reviewed for this API.</p>'
        sections = [('syntax', 'Syntax')]
        body += '<h2 id="syntax">Syntax</h2><pre><code>' + escape(symbol['declaration']) + (';' if 'members' not in symbol else '') + '</code></pre>'
        if symbol.get('bases'):
            body += '<p>Inherits: ' + ', '.join(link(base) for base in symbol['bases']) + '.</p>'
        if symbol.get('parameters'):
            sections.append(('parameters', 'Parameters'))
            body += '<h2 id="parameters">Parameters</h2><table><thead><tr><th>Name and type</th><th>Contract</th></tr></thead><tbody>'
            for i, parameter in enumerate(symbol['parameters']):
                name = parameter['name'] or '#' + str(i + 1)
                body += '<tr><td><code>' + escape(parameter['declaration']) + '</code></td><td>' + prose(contract.get('parameters', {}).get(name, 'Behavioral contract pending.')) + '</td></tr>'
            body += '</tbody></table>'
        for key, title in [('returns', 'Return value'), ('errors', 'Errors and exceptions'), ('remarks', 'Remarks'), ('ownership', 'Ownership and lifetime'), ('threading', 'Thread safety'), ('availability', 'Availability')]:
            if key not in contract: continue
            sections.append((key, title))
            body += '<h2 id="' + key + '">' + title + '</h2>' + paragraphs(contract[key])
        if 'members' in symbol:
            for access in ['public', 'protected']:
                entries = [m for m in symbol['members'] if m['access'] == access]
                if entries:
                    sections.append((access, access.title() + ' members'))
                    body += '<h2 id="' + access + '">' + access.title() + ' members</h2>' + table(entries)
        if contract.get('examples'):
            sections.append(('examples', 'Examples'))
            body += '<h2 id="examples">Examples</h2>'
            for example in contract['examples']:
                body += paragraphs(example['caption']) + '<p class="metadata">' + escape(example['source']) + '</p><pre><code>' + escape((root / example['source']).read_text(encoding="utf-8")) + '</code></pre>'
        if contract.get('see_also'):
            sections.append(('related', 'See also'))
            body += '<h2 id="related">See also</h2><ul>' + ''.join('<li>' + link(s) + '</li>' for s in contract['see_also']) + '</ul>'
        parent = parents.get(symbol['id'])
        crumb = ' / ' + link(parent['id'], parent['name']) if parent else ''
        page(symbol['page'], symbol['id'].split('|')[0], body, crumb, sections)
    body = '<p class="lead">Compiler coverage and behavioral documentation are measured independently.</p>'
    body += '<table><thead><tr><th>Measure</th><th>Count</th></tr></thead><tbody>'
    for key, value in coverage.items():
        if key not in {'schema', 'missing'}: body += '<tr><td>' + escape(key.replace('_', ' ').capitalize()) + '</td><td>' + str(value) + '</td></tr>'
    body += '</tbody></table><h2 id="pending">Pending contracts</h2><p>This list is work remaining, not a statement that the APIs are unimplemented. Search the API to find individual declarations.</p><ul>'
    body += ''.join('<li>' + link(t['id']) + ': ' + str(sum(m['id'] in coverage['missing'] for m in t['members'])) + ' member contracts pending.</li>' for t in data['types'])
    body += '</ul>'
    page('coverage.html', 'Reference coverage', body, sections=(('pending', 'Pending contracts'),))
    for filename, content in pages.items(): (output / filename).write_text(content + '\n', encoding="utf-8")
    # This directory is entirely generated; only remove pages owned by an earlier manifest.
    manifest = output / 'pages.json'
    if manifest.exists():
        for stale in set(json.loads(manifest.read_text(encoding="utf-8"))) - set(pages):
            if Path(stale).name == stale and stale.endswith('.html'): (output / stale).unlink(missing_ok=True)
    manifest.write_text(json.dumps(sorted(pages), indent=2) + '\n', encoding="utf-8")
    (output / 'reference.css').write_text(STYLE, encoding="utf-8")
    (output / 'search.js').write_text(SEARCH, encoding="utf-8")
    search = [{k: s[k] for k in ('id', 'header', 'page', 'declaration')} | {'summary': contracts.get(s['id'], {}).get('summary', '')} for s in symbols.values()]
    (output / 'search-data.js').write_text('window.GUIFormsSearch=' + json.dumps(search, ensure_ascii=True) + ';\n', encoding="utf-8")
    (output / 'coverage.json').write_text(json.dumps(coverage, indent=2) + '\n', encoding="utf-8")
    return coverage
