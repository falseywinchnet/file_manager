#!/usr/bin/env python3
"""Generate the Engine's source-adjacent HTML atlas and Markdown mirror.

The inventory is deliberately standard-library-only and deterministic. It
extracts Go packages, internal imports, top-level declarations, methods, source
locations, and test entry points. Hand-authored status and boundary language in
docs/LIBRARY_MANUAL.json remains authoritative over generated fallback prose.
"""

from __future__ import annotations

import argparse
import html
import json
import re
import shutil
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path


ENGINE = Path(__file__).resolve().parents[1]
MANUAL_PATH = ENGINE / "docs" / "LIBRARY_MANUAL.json"
DEFAULT_OUTPUT = ENGINE / "docs" / "library"
MODULE_PATTERN = re.compile(r"^module\s+(\S+)", re.MULTILINE)
IMPORT_PATTERN = re.compile(r'"([^"]+)"')
FUNC_PATTERN = re.compile(
    r"^func\s+(?:\((?P<receiver>[^)]*)\)\s+)?(?P<name>[A-Za-z_]\w*)\s*\("
)
TYPE_PATTERN = re.compile(r"^type\s+(?P<name>[A-Za-z_]\w*)\b(?P<tail>.*)")
VALUE_PATTERN = re.compile(r"^(?:const|var)\s+(?P<name>[A-Za-z_]\w*)\b(?P<tail>.*)")
BLOCK_VALUE_PATTERN = re.compile(r"^(?P<name>[A-Za-z_]\w*)\b")


@dataclass
class Symbol:
    name: str
    kind: str
    signature: str
    source: str
    line: int
    doc: str = ""
    receiver: str = ""
    exported: bool = False
    test: bool = False


@dataclass
class Package:
    import_path: str
    relative: str
    name: str
    doc: str = ""
    files: list[str] = field(default_factory=list)
    test_files: list[str] = field(default_factory=list)
    imports: set[str] = field(default_factory=set)
    symbols: list[Symbol] = field(default_factory=list)
    lines: int = 0
    test_lines: int = 0


def load_manual() -> dict:
    manual = json.loads(MANUAL_PATH.read_text(encoding="utf-8"))
    if manual.get("schema") != 1:
        raise ValueError("docs/LIBRARY_MANUAL.json must use schema 1")
    return manual


def module_name() -> str:
    match = MODULE_PATTERN.search((ENGINE / "go.mod").read_text(encoding="utf-8"))
    if not match:
        raise ValueError("go.mod has no module declaration")
    return match.group(1)


def leading_doc(lines: list[str], index: int) -> str:
    comments: list[str] = []
    cursor = index - 1
    while cursor >= 0 and lines[cursor].lstrip().startswith("//"):
        comments.append(lines[cursor].lstrip()[2:].strip())
        cursor -= 1
    comments.reverse()
    return " ".join(part for part in comments if part)


def package_doc(lines: list[str], package_line: int) -> str:
    doc = leading_doc(lines, package_line)
    return doc if doc.startswith("Package ") else ""


def compact_signature(lines: list[str], start: int) -> tuple[str, int]:
    parts: list[str] = []
    cursor = start
    while cursor < len(lines):
        text = lines[cursor].strip()
        parts.append(text)
        if "{" in text:
            break
        cursor += 1
    signature = re.sub(r"\s+", " ", " ".join(parts)).strip()
    if "{" in signature:
        signature = signature.split("{", 1)[0].rstrip()
    return signature, cursor


def receiver_name(raw: str) -> str:
    if not raw:
        return ""
    value = raw.split()[-1].lstrip("*")
    return value.split("[")[0]


def type_kind(tail: str) -> str:
    tail = tail.strip()
    if tail.startswith("struct"):
        return "struct"
    if tail.startswith("interface"):
        return "interface"
    return "type"


def parse_declarations(lines: list[str], source: str, is_test: bool) -> list[Symbol]:
    symbols: list[Symbol] = []
    index = 0
    while index < len(lines):
        raw = lines[index]
        if raw[:1].isspace():
            index += 1
            continue
        text = raw.strip()
        type_match = TYPE_PATTERN.match(text)
        if type_match:
            signature = text.split("{", 1)[0].rstrip()
            name = type_match.group("name")
            symbols.append(Symbol(
                name=name, kind=type_kind(type_match.group("tail")),
                signature=signature, source=source, line=index + 1,
                doc=leading_doc(lines, index), exported=name[:1].isupper(), test=is_test,
            ))
            index += 1
            continue
        func_match = FUNC_PATTERN.match(text)
        if func_match:
            signature, end = compact_signature(lines, index)
            name = func_match.group("name")
            receiver = receiver_name(func_match.group("receiver") or "")
            kind = "method" if receiver else "function"
            symbols.append(Symbol(
                name=name, kind=kind, signature=signature, source=source,
                line=index + 1, doc=leading_doc(lines, index), receiver=receiver,
                exported=name[:1].isupper(), test=is_test,
            ))
            index = end + 1
            continue
        value_match = VALUE_PATTERN.match(text)
        if value_match:
            name = value_match.group("name")
            kind = "constant" if text.startswith("const ") else "variable"
            symbols.append(Symbol(
                name=name, kind=kind, signature=text, source=source,
                line=index + 1, doc=leading_doc(lines, index),
                exported=name[:1].isupper(), test=is_test,
            ))
            index += 1
            continue
        if text in {"const (", "var ("}:
            kind = "constant" if text.startswith("const") else "variable"
            group_doc = leading_doc(lines, index)
            index += 1
            while index < len(lines) and lines[index].strip() != ")":
                candidate = lines[index].strip()
                if candidate and not candidate.startswith("//"):
                    match = BLOCK_VALUE_PATTERN.match(candidate)
                    if match:
                        name = match.group("name")
                        symbols.append(Symbol(
                            name=name, kind=kind, signature=candidate,
                            source=source, line=index + 1,
                            doc=leading_doc(lines, index) or group_doc,
                            exported=name[:1].isupper(), test=is_test,
                        ))
                index += 1
            index += 1
            continue
        index += 1
    return symbols


def parse_imports(text: str, module: str) -> set[str]:
    imports: set[str] = set()
    for match in re.finditer(r"\bimport\s+(?:\((.*?)\)|([^\n]+))", text, re.DOTALL):
        body = match.group(1) if match.group(1) is not None else match.group(2)
        for imported in IMPORT_PATTERN.findall(body):
            if imported == module or imported.startswith(module + "/"):
                imports.add(imported)
    return imports


def discover_packages() -> list[Package]:
    module = module_name()
    grouped: dict[str, Package] = {}
    for path in sorted(ENGINE.rglob("*.go")):
        if any(part in {"build", ".build", "vendor"} for part in path.parts):
            continue
        relative_file = path.relative_to(ENGINE).as_posix()
        relative_dir = path.parent.relative_to(ENGINE).as_posix()
        if relative_dir == ".":
            relative_dir = ""
        text = path.read_text(encoding="utf-8")
        lines = text.splitlines()
        package_match = re.search(r"^package\s+(\w+)", text, re.MULTILINE)
        if not package_match:
            continue
        package_line = text[:package_match.start()].count("\n")
        import_path = module + ("/" + relative_dir if relative_dir else "")
        package = grouped.setdefault(
            import_path,
            Package(import_path=import_path, relative=relative_dir, name=package_match.group(1)),
        )
        is_test = path.name.endswith("_test.go")
        if is_test:
            package.test_files.append(relative_file)
            package.test_lines += len(lines)
        else:
            package.files.append(relative_file)
            package.lines += len(lines)
            if not package.doc:
                package.doc = package_doc(lines, package_line)
        package.imports.update(parse_imports(text, module))
        package.symbols.extend(parse_declarations(lines, relative_file, is_test))
    return sorted(grouped.values(), key=lambda item: item.import_path)


def slug(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-")


def source_link(symbol: Symbol) -> str:
    target = "../../../" + symbol.source
    return f'<a href="{html.escape(target)}#L{symbol.line}">{html.escape(symbol.source)}:{symbol.line}</a>'


def page_shell(title: str, status: str, body: str) -> str:
    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width">
<title>{html.escape(title)}</title><link rel="stylesheet" href="../assets/page.css"></head>
<body><header><p class="eyebrow">File Manager Engine service atlas</p><h1>{html.escape(title)}</h1>
<p class="status">{html.escape(status)}</p></header><main>{body}</main></body></html>
"""


def prose(value: str) -> str:
    return html.escape(value or "No package documentation comment is present.")


def package_page(package: Package, manual: dict, page_by_import: dict[str, str]) -> str:
    entry = manual["packages"][package.import_path]
    dependencies = []
    for imported in sorted(package.imports):
        if imported == package.import_path:
            continue
        label = imported.removeprefix(module_name() + "/")
        if imported in page_by_import:
            dependencies.append(f'<a class="chip" href="{page_by_import[imported]}">{html.escape(label)}</a>')
        else:
            dependencies.append(f'<span class="chip">{html.escape(label)}</span>')
    if not dependencies:
        dependencies.append('<span class="muted">No internal package imports.</span>')

    invariants = "".join(f"<li>{html.escape(item)}</li>" for item in entry.get("invariants", []))
    production = [symbol for symbol in package.symbols if not symbol.test]
    tests = [symbol for symbol in package.symbols if symbol.test and symbol.kind == "function"]
    symbol_rows = []
    for symbol in sorted(production, key=lambda item: (item.kind, item.receiver, item.name, item.line)):
        label = f"{symbol.receiver}.{symbol.name}" if symbol.receiver else symbol.name
        symbol_rows.append(
            f'<article class="symbol" id="{slug(label)}"><div class="symbol-head"><h3>{html.escape(label)}</h3>'
            f'<span>{html.escape(symbol.kind)}</span></div><pre><code>{html.escape(symbol.signature)}</code></pre>'
            f'<p>{prose(symbol.doc)}</p><p class="source">{source_link(symbol)}</p></article>'
        )
    if not symbol_rows:
        symbol_rows.append('<p class="muted">This package contains verification code only.</p>')
    test_rows = "".join(
        f'<li><code>{html.escape(item.name)}</code> <span>{source_link(item)}</span></li>'
        for item in sorted(tests, key=lambda symbol: (symbol.source, symbol.line))
    ) or "<li>No top-level Go test or benchmark entry points.</li>"
    file_rows = "".join(
        f"<li><code>{html.escape(path)}</code></li>"
        for path in package.files + package.test_files
    )
    body = f"""
<section class="lede"><p>{html.escape(entry['summary'])}</p><p>{prose(package.doc)}</p></section>
<section><h2>Boundary and ownership</h2><ul>{invariants}</ul></section>
<section><h2>Internal dependencies</h2><div class="chips">{''.join(dependencies)}</div></section>
<section><h2>Inventory</h2><dl class="metrics"><div><dt>Production files</dt><dd>{len(package.files)}</dd></div>
<div><dt>Production lines</dt><dd>{package.lines}</dd></div><div><dt>Test files</dt><dd>{len(package.test_files)}</dd></div>
<div><dt>Test lines</dt><dd>{package.test_lines}</dd></div><div><dt>Declarations</dt><dd>{len(production)}</dd></div></dl></section>
<section><h2>Declarations</h2>{''.join(symbol_rows)}</section>
<section><details><summary>Verification entry points ({len(tests)})</summary><ul class="tests">{test_rows}</ul></details></section>
<section><details><summary>Source files ({len(package.files) + len(package.test_files)})</summary><ul>{file_rows}</ul></details></section>
"""
    label = package.import_path.removeprefix(module_name() + "/")
    return page_shell(label, entry["status"], body)


def overview_page(manual: dict, page_by_import: dict[str, str]) -> str:
    boundaries = "".join(f"<li>{html.escape(item)}</li>" for item in manual["boundaries"])
    layer_cards = []
    for layer in manual["layers"]:
        packages = []
        for imported in layer["packages"]:
            label = imported.removeprefix(module_name() + "/")
            target = page_by_import.get(imported)
            packages.append(f'<a class="chip" href="{target}">{html.escape(label)}</a>' if target else f'<span class="chip">{html.escape(label)}</span>')
        layer_cards.append(
            f'<article class="card"><h3>{html.escape(layer["name"])}</h3><p class="status">{html.escape(layer["status"])}</p>'
            f'<p>{html.escape(layer["responsibility"])}</p><div class="chips">{"".join(packages)}</div></article>'
        )
    flows = []
    for flow in manual["flows"]:
        steps = "".join(f"<li>{html.escape(step)}</li>" for step in flow["steps"])
        flows.append(f'<article class="flow"><h3>{html.escape(flow["name"])}</h3><ol>{steps}</ol></article>')
    body = f"""
<section class="lede"><p>{html.escape(manual['summary'])}</p></section>
<section><h2>Non-negotiable boundaries</h2><ul>{boundaries}</ul></section>
<section><h2>System layers</h2><div class="cards">{''.join(layer_cards)}</div></section>
<section><h2>Runtime flows</h2><div class="flows">{''.join(flows)}</div></section>
"""
    return page_shell("Architecture and runtime map", manual["status"], body)


def protocol_page(manual: dict) -> str:
    rows = "".join(
        f"<tr><td><code>{html.escape(item['method'])}</code></td><td>{html.escape(item['authority'])}</td>"
        f"<td>{html.escape(item['status'])}</td><td>{html.escape(item['purpose'])}</td></tr>"
        for item in manual["protocol"]
    )
    body = f"""
<section class="lede"><p>Canonical semantic methods and their current implementation status. Development aliases remain fixtures; this table does not promote JSONL into the production transport ABI.</p></section>
<section><div class="table-wrap"><table><thead><tr><th>Method</th><th>Authority</th><th>Status</th><th>Purpose</th></tr></thead><tbody>{rows}</tbody></table></div></section>
"""
    return page_shell("Protocol and authority", "ORC-ENG semantic v0 with explicit installed-transport gates", body)


def operations_page(manual: dict) -> str:
    cards = "".join(
        f'<article class="symbol"><h3>{html.escape(item["name"])}</h3><pre><code>{html.escape(item["command"])}</code></pre><p>{html.escape(item["outcome"])}</p></article>'
        for item in manual["operations"]
    )
    body = f"""
<section class="lede"><p>Reproducible development build, validation, documentation, and install commands. Installation copies artifacts only; it deliberately does not register a launchd/systemd/SCM service.</p></section>
<section>{cards}</section>
"""
    return page_shell("Build, validate, and install", "OBSERVED developer artifact workflow; production supervisor remains gated", body)


def markdown_package(package: Package, manual: dict) -> str:
    entry = manual["packages"][package.import_path]
    lines = [
        f"# {package.import_path}", "", f"Status: **{entry['status']}**.", "",
        entry["summary"], "", package.doc or "No package documentation comment is present.", "",
        "## Invariants", "",
    ]
    lines.extend(f"- {item}" for item in entry.get("invariants", []))
    lines.extend(["", "## Internal imports", ""])
    internal = sorted(item for item in package.imports if item != package.import_path)
    lines.extend(f"- `{item}`" for item in internal or ["None."])
    lines.extend(["", "## Declarations", ""])
    for symbol in sorted((item for item in package.symbols if not item.test), key=lambda item: (item.kind, item.receiver, item.name, item.line)):
        label = f"{symbol.receiver}.{symbol.name}" if symbol.receiver else symbol.name
        lines.extend([
            f"### {label}", "", f"Kind: `{symbol.kind}`. Source: `{symbol.source}:{symbol.line}`.", "",
            "```go", symbol.signature, "```", "", symbol.doc or "No declaration documentation comment is present.", "",
        ])
    return "\n".join(lines).rstrip() + "\n"


def library_css() -> str:
    return """:root{color-scheme:dark;--bg:#0d141d;--panel:#121d29;--line:#294055;--ink:#e8f0f7;--muted:#94a9ba;--accent:#6ee7c8;--amber:#f2c879}*{box-sizing:border-box}html,body{height:100%;margin:0}body{display:grid;grid-template-columns:minmax(250px,320px) 1fr;background:var(--bg);color:var(--ink);font:14px/1.5 ui-sans-serif,system-ui,sans-serif}aside{padding:20px;border-right:1px solid var(--line);overflow:auto;background:#101923}h1{font-size:19px;margin:0 0 14px}input{width:100%;padding:10px 12px;border:1px solid var(--line);border-radius:7px;background:#0a1119;color:var(--ink)}.count{color:var(--muted);font-size:12px}.group{margin:18px 0}.group h2{color:var(--amber);font-size:11px;letter-spacing:.1em;text-transform:uppercase}.group a{display:block;padding:7px 9px;margin:2px 0;border-radius:6px;color:var(--ink);text-decoration:none}.group a:hover,.group a.active{background:#1b2b3a;color:var(--accent)}iframe{width:100%;height:100%;border:0;background:var(--bg)}@media(max-width:760px){body{grid-template-columns:1fr;grid-template-rows:280px 1fr}aside{border-right:0;border-bottom:1px solid var(--line)}}"""


def page_css() -> str:
    return """:root{color-scheme:dark;--bg:#0d141d;--panel:#121d29;--line:#294055;--ink:#e8f0f7;--muted:#94a9ba;--accent:#6ee7c8;--amber:#f2c879}*{box-sizing:border-box}body{max-width:1120px;margin:0 auto;padding:34px;background:var(--bg);color:var(--ink);font:15px/1.62 ui-sans-serif,system-ui,sans-serif}h1{font-size:32px;line-height:1.1;margin:.2em 0}h2{margin-top:2em;border-bottom:1px solid var(--line);padding-bottom:.35em}h3{margin:.2em 0}.eyebrow{color:var(--accent);font-size:12px;letter-spacing:.12em;text-transform:uppercase}.status{color:var(--amber)}.lede{font-size:18px;color:#c7d7e4}.card,.flow,.symbol{background:var(--panel);border:1px solid var(--line);border-radius:10px;padding:17px;margin:12px 0}.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:12px}.chips{display:flex;flex-wrap:wrap;gap:7px}.chip{border:1px solid var(--line);border-radius:99px;padding:3px 9px;color:var(--accent);text-decoration:none}.symbol-head{display:flex;justify-content:space-between;gap:16px}.symbol-head span,.muted,.source{color:var(--muted)}pre{overflow:auto;background:#091018;border:1px solid #213648;border-radius:7px;padding:12px}code{font:13px/1.45 ui-monospace,SFMono-Regular,Menlo,monospace;color:#d8f8ef}.source a,a{color:var(--accent)}.metrics{display:flex;flex-wrap:wrap;gap:8px}.metrics div{min-width:140px;background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:10px}.metrics dt{color:var(--muted);font-size:12px}.metrics dd{font-size:22px;margin:0}.table-wrap{overflow:auto}table{width:100%;border-collapse:collapse}th,td{text-align:left;vertical-align:top;padding:10px;border-bottom:1px solid var(--line)}details summary{cursor:pointer;color:var(--amber)}.tests li{margin:.5em 0}@media(max-width:600px){body{padding:20px}.symbol-head{display:block}}"""


def library_js() -> str:
    return """(() => {const items=window.ENGINE_LIBRARY_MANIFEST||[];const nav=document.querySelector('nav');const input=document.querySelector('input');const count=document.querySelector('.count');const frame=document.querySelector('iframe');let active=null;function render(query=''){nav.textContent='';const normalized=query.trim().toLowerCase();const visible=items.filter(item=>!normalized||item.search.includes(normalized));count.textContent=`${visible.length} of ${items.length} entries`;const groups=new Map();for(const item of visible){if(!groups.has(item.category))groups.set(item.category,[]);groups.get(item.category).push(item)}for(const [category,entries] of groups){const section=document.createElement('section');section.className='group';const heading=document.createElement('h2');heading.textContent=category;section.appendChild(heading);for(const item of entries){const link=document.createElement('a');link.href=item.page;link.target='reference';link.textContent=item.name;link.title=item.status;link.addEventListener('click',()=>{if(active)active.classList.remove('active');active=link;link.classList.add('active')});section.appendChild(link)}nav.appendChild(section)}}input.addEventListener('input',()=>render(input.value));render();if(items.length){frame.src=items[0].page;const first=nav.querySelector('a');if(first){first.classList.add('active');active=first}}})();"""


def index_html(title: str) -> str:
    return f"""<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width">
<title>{html.escape(title)}</title><link rel="stylesheet" href="assets/library.css"></head><body>
<aside><h1>{html.escape(title)}</h1><input type="search" placeholder="Find a package, symbol, method, or boundary…" aria-label="Search atlas"><p class="count"></p><nav></nav></aside>
<iframe name="reference" title="Engine service reference"></iframe><script src="manifest.js"></script><script src="assets/library.js"></script></body></html>"""


def readme(manual: dict, package_count: int, declaration_count: int) -> str:
    return f"""# File Manager Engine service atlas

Status: **{manual['status']}**.

Open `index.html` directly in a browser. The checked-in atlas contains {package_count}
Go packages and {declaration_count} production declarations, plus architecture,
protocol, operations, source locations, verification entry points, and an
AI-readable Markdown mirror. It uses no server and performs no network access.

Regenerate and verify from `engine/`:

```sh
python3 tools/generate_library_docs.py
python3 tools/generate_library_docs.py --check
```

`docs/LIBRARY_MANUAL.json` owns reviewed status, boundary, flow, and package
language. The generator owns inventories and generated files. An installed copy
retains source paths as provenance even when those relative links are not
available outside a source checkout.
"""


def write_text(root: Path, relative: str, content: str) -> None:
    path = root / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def generate(output: Path) -> tuple[int, int]:
    manual = load_manual()
    packages = discover_packages()
    documented = set(manual["packages"])
    discovered = {package.import_path for package in packages}
    missing = sorted(discovered - documented)
    stale = sorted(documented - discovered)
    if missing or stale:
        details = []
        if missing:
            details.append("missing manual package entries: " + ", ".join(missing))
        if stale:
            details.append("manual entries without Go packages: " + ", ".join(stale))
        raise ValueError("; ".join(details))

    output.mkdir(parents=True, exist_ok=True)
    page_by_import = {
        package.import_path: f"{slug(package.import_path.removeprefix(module_name() + '/'))}.html"
        for package in packages
    }
    manifest = [
        {"name": "Architecture and runtime map", "category": "Guides", "page": "pages/overview.html", "status": manual["status"], "search": "architecture runtime flow layers boundaries service index search"},
        {"name": "Protocol and authority", "category": "Guides", "page": "pages/protocol.html", "status": "ORC-ENG semantic v0", "search": "protocol methods authority query admin contract jsonl"},
        {"name": "Build, validate, and install", "category": "Guides", "page": "pages/operations.html", "status": "development artifact workflow", "search": "build test race vet install m4 documentation"},
    ]
    declaration_count = 0
    for package in packages:
        page_name = page_by_import[package.import_path]
        write_text(output, "pages/" + page_name, package_page(package, manual, page_by_import))
        write_text(output, "markdown/" + Path(page_name).with_suffix(".md").name, markdown_package(package, manual))
        production = [symbol for symbol in package.symbols if not symbol.test]
        declaration_count += len(production)
        entry = manual["packages"][package.import_path]
        label = package.import_path.removeprefix(module_name() + "/")
        search_terms = " ".join(
            [label, package.name, entry["summary"], entry["status"]]
            + [symbol.name for symbol in package.symbols]
            + [symbol.receiver for symbol in package.symbols if symbol.receiver]
        ).lower()
        manifest.append({
            "name": label, "category": entry["category"], "page": "pages/" + page_name,
            "status": entry["status"], "search": search_terms,
        })

    write_text(output, "pages/overview.html", overview_page(manual, page_by_import))
    write_text(output, "pages/protocol.html", protocol_page(manual))
    write_text(output, "pages/operations.html", operations_page(manual))
    write_text(output, "assets/library.css", library_css())
    write_text(output, "assets/page.css", page_css())
    write_text(output, "assets/library.js", library_js())
    write_text(output, "index.html", index_html(manual["title"]))
    encoded_manifest = json.dumps(manifest, indent=2, sort_keys=True) + "\n"
    write_text(output, "manifest.json", encoded_manifest)
    write_text(output, "manifest.js", "window.ENGINE_LIBRARY_MANIFEST = " + json.dumps(manifest, indent=2, sort_keys=True) + ";\n")
    write_text(output, "README.md", readme(manual, len(packages), declaration_count))
    return len(packages), declaration_count


def tree_snapshot(root: Path) -> dict[str, bytes]:
    if not root.exists():
        return {}
    return {
        path.relative_to(root).as_posix(): path.read_bytes()
        for path in sorted(root.rglob("*")) if path.is_file()
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail when checked-in output differs")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()
    output = args.output.resolve()
    with tempfile.TemporaryDirectory(prefix="fileman-engine-docs-") as temporary:
        candidate = Path(temporary) / "library"
        package_count, declaration_count = generate(candidate)
        if args.check:
            expected = tree_snapshot(candidate)
            actual = tree_snapshot(output)
            if expected != actual:
                missing = sorted(set(expected) - set(actual))
                extra = sorted(set(actual) - set(expected))
                changed = sorted(path for path in set(expected) & set(actual) if expected[path] != actual[path])
                print("Engine service atlas is stale.", file=sys.stderr)
                if missing:
                    print("  missing: " + ", ".join(missing), file=sys.stderr)
                if extra:
                    print("  extra: " + ", ".join(extra), file=sys.stderr)
                if changed:
                    print("  changed: " + ", ".join(changed), file=sys.stderr)
                return 1
        else:
            if output.exists():
                shutil.rmtree(output)
            shutil.copytree(candidate, output)
    print(f"Engine service atlas: {package_count} packages, {declaration_count} production declarations")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
