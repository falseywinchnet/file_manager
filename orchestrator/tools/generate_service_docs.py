#!/usr/bin/env python3
"""Generate the Orchestrator service atlas and Markdown mirror.

Human-reviewed system meaning lives in docs/library/manual.json. The generator
adds a conservative Rust source inventory and never promotes a capability,
contract, or platform from implementation shape alone.
"""

from __future__ import annotations

import argparse
import html
import json
import re
import sys
from pathlib import Path


ORCHESTRATOR = Path(__file__).resolve().parents[1]
OUTPUT = ORCHESTRATOR / "docs" / "library"
MANUAL = OUTPUT / "manual.json"
GENERATED_DIRECTORIES = ("pages", "markdown")


def load_manual() -> dict:
    value = json.loads(MANUAL.read_text(encoding="utf-8"))
    if value.get("schema") != 1:
        raise ValueError("manual.json must use schema 1")
    return value


def source_inventory(path: Path) -> list[dict]:
    text = path.read_text(encoding="utf-8")
    declarations: list[dict] = []
    pattern = re.compile(
        r"^\s*(pub(?:\((?:crate|super|self)\))?\s+)?"
        r"(?:const\s+|unsafe\s+|async\s+)*"
        r"(struct|enum|trait|type|fn|const|static)\s+([A-Za-z_]\w*)"
    )
    lines = text.splitlines()
    for index, line in enumerate(lines):
        match = pattern.match(line)
        if not match:
            continue
        signature = line.strip()
        cursor = index + 1
        while not any(token in signature for token in ("{", ";")) and cursor < len(lines):
            signature += " " + lines[cursor].strip()
            cursor += 1
        signature = re.sub(r"\s+", " ", signature).split("{", 1)[0].rstrip()
        visibility = "private"
        if match.group(1):
            visibility = match.group(1).strip().replace(" ", "")
        declarations.append(
            {
                "kind": match.group(2),
                "name": match.group(3),
                "line": index + 1,
                "visibility": visibility,
                "signature": signature,
            }
        )
    return declarations


def source_href(source: str, line: int | None = None) -> str:
    suffix = f"#L{line}" if line else ""
    return f"../../../{source}{suffix}"


def paragraph_html(values: list[str]) -> str:
    return "".join(f"<p>{html.escape(value)}</p>" for value in values)


def list_html(values: list[str]) -> str:
    if not values:
        return ""
    return "<ul>" + "".join(f"<li>{html.escape(value)}</li>" for value in values) + "</ul>"


def page_shell(title: str, eyebrow: str, status: str, body: str) -> str:
    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width">
<title>{html.escape(title)} · Orchestrator service atlas</title>
<link rel="stylesheet" href="../assets/page.css"></head><body><article>
<p class="eyebrow">{html.escape(eyebrow)}</p><h1>{html.escape(title)}</h1>
<p class="status">{html.escape(status)}</p>{body}
<footer><a href="../index.html" target="_top">Return to atlas</a></footer>
</article></body></html>
"""


def guide_page(guide: dict) -> tuple[str, str, dict]:
    sections = []
    markdown = [
        f"# {guide['title']}",
        "",
        f"Status: **{guide['status']}**.",
        "",
        guide["summary"],
        "",
    ]
    for section in guide.get("sections", []):
        sections.append(f"<h2>{html.escape(section['heading'])}</h2>")
        sections.append(paragraph_html(section.get("paragraphs", [])))
        sections.append(list_html(section.get("bullets", [])))
        markdown.extend([f"## {section['heading']}", ""])
        for paragraph in section.get("paragraphs", []):
            markdown.extend([paragraph, ""])
        for bullet in section.get("bullets", []):
            markdown.append(f"- {bullet}")
        if section.get("bullets"):
            markdown.append("")
    if guide.get("sources"):
        sections.append("<h2>Authority and evidence locators</h2><ul>")
        for source in guide["sources"]:
            if not (ORCHESTRATOR / source).is_file():
                raise FileNotFoundError(f"guide source does not exist: {source}")
            sections.append(
                f'<li><a href="{source_href(source)}">{html.escape(source)}</a></li>'
            )
        sections.append("</ul>")
        markdown.extend(["## Authority and evidence locators", ""])
        markdown.extend(f"- [{source}]({source_href(source)})" for source in guide["sources"])
        markdown.append("")
    body = f'<p class="summary">{html.escape(guide["summary"])}</p>' + "".join(sections)
    entry = {
        "kind": "guide",
        "category": guide["category"],
        "title": guide["title"],
        "page": f"pages/{guide['slug']}.html",
        "status": guide["status"],
        "search": " ".join(
            [guide["title"], guide["summary"]]
            + [value for section in guide.get("sections", []) for value in section.get("bullets", [])]
        ).lower(),
    }
    return page_shell(guide["title"], guide["category"], guide["status"], body), "\n".join(markdown), entry


def module_slug(source: str) -> str:
    return "module_" + re.sub(r"[^a-z0-9]+", "_", source.lower()).strip("_")


def module_page(source: str, record: dict) -> tuple[str, str, dict]:
    path = ORCHESTRATOR / source
    declarations = source_inventory(path)
    slug = module_slug(source)
    body = [f'<p class="summary">{html.escape(record["summary"])}</p>']
    body.append("<dl>")
    body.append(f"<dt>Source</dt><dd><a href=\"{source_href(source)}\">{html.escape(source)}</a></dd>")
    body.append(f"<dt>Layer</dt><dd>{html.escape(record['category'])}</dd>")
    body.append("</dl>")
    body.append("<h2>Responsibilities</h2>")
    body.append(list_html(record.get("responsibilities", [])))
    body.append("<h2>Boundary</h2>")
    body.append(list_html(record.get("boundaries", [])))
    if record.get("contracts"):
        body.append("<h2>Contract projections</h2>")
        body.append(list_html(record["contracts"]))
    body.append("<h2>Source inventory</h2>")
    if declarations:
        body.append('<div class="symbols">')
        for declaration in declarations:
            body.append(
                '<section class="symbol">'
                f'<h3><a href="{source_href(source, declaration["line"])}">'
                f'{html.escape(declaration["name"])}</a>'
                f'<span>{html.escape(declaration["kind"])} · {html.escape(declaration["visibility"])}</span></h3>'
                f'<code>{html.escape(declaration["signature"])}</code></section>'
            )
        body.append("</div>")
    else:
        body.append('<p class="pending">No conservative top-level Rust declarations were inventoried.</p>')

    markdown = [
        f"# {source}",
        "",
        f"Status: **{record['status']}**.",
        "",
        record["summary"],
        "",
        f"Source: [{source}]({source_href(source)})",
        "",
        "## Responsibilities",
        "",
    ]
    markdown.extend(f"- {value}" for value in record.get("responsibilities", []))
    markdown.extend(["", "## Boundary", ""])
    markdown.extend(f"- {value}" for value in record.get("boundaries", []))
    if record.get("contracts"):
        markdown.extend(["", "## Contract projections", ""])
        markdown.extend(f"- {value}" for value in record["contracts"])
    markdown.extend(["", "## Source inventory", ""])
    for declaration in declarations:
        markdown.extend(
            [
                f"### [{declaration['name']}]({source_href(source, declaration['line'])})",
                "",
                f"`{declaration['kind']}` · `{declaration['visibility']}`",
                "",
                "```rust",
                declaration["signature"],
                "```",
                "",
            ]
        )
    entry = {
        "kind": "module",
        "category": record["category"],
        "title": source,
        "page": f"pages/{slug}.html",
        "status": record["status"],
        "search": " ".join(
            [source, record["summary"]]
            + record.get("responsibilities", [])
            + [declaration["name"] for declaration in declarations]
        ).lower(),
    }
    return page_shell(source, record["category"], record["status"], "".join(body)), "\n".join(markdown), entry


def generated_files(manual: dict) -> dict[str, str]:
    files: dict[str, str] = {}
    entries: list[dict] = []
    for guide in manual["guides"]:
        page, markdown, entry = guide_page(guide)
        files[f"pages/{guide['slug']}.html"] = page
        files[f"markdown/{guide['slug']}.md"] = markdown
        entries.append(entry)
    for source, record in manual["modules"].items():
        if not (ORCHESTRATOR / source).is_file():
            raise FileNotFoundError(f"documented module does not exist: {source}")
        page, markdown, entry = module_page(source, record)
        slug = module_slug(source)
        files[f"pages/{slug}.html"] = page
        files[f"markdown/{slug}.md"] = markdown
        entries.append(entry)
    manifest = {
        "schema": 1,
        "title": manual["title"],
        "status": manual["status"],
        "entries": entries,
    }
    manifest_json = json.dumps(manifest, indent=2, ensure_ascii=False) + "\n"
    files["manifest.json"] = manifest_json
    files["manifest.js"] = "window.ORCHESTRATOR_DOCS = " + manifest_json.rstrip() + ";\n"
    return files


def check(files: dict[str, str]) -> int:
    expected = set(files)
    actual = {
        path.relative_to(OUTPUT).as_posix()
        for directory in GENERATED_DIRECTORIES
        for path in (OUTPUT / directory).glob("*")
        if path.is_file()
    }
    actual.update(name for name in ("manifest.json", "manifest.js") if (OUTPUT / name).is_file())
    failures = []
    for relative, content in files.items():
        path = OUTPUT / relative
        if not path.is_file() or path.read_text(encoding="utf-8") != content:
            failures.append(relative)
    failures.extend(sorted(actual - expected))
    if failures:
        print("service atlas is stale:", file=sys.stderr)
        for failure in sorted(set(failures)):
            print(f"  {failure}", file=sys.stderr)
        return 1
    print(f"service atlas is current ({len(files)} generated files)")
    return 0


def write(files: dict[str, str]) -> None:
    for directory in GENERATED_DIRECTORIES:
        target = OUTPUT / directory
        target.mkdir(parents=True, exist_ok=True)
        for path in target.glob("*"):
            if path.is_file():
                path.unlink()
    for relative, content in files.items():
        path = OUTPUT / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
    print(f"generated {len(files)} service-atlas files")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="fail when checked-in output is stale")
    arguments = parser.parse_args()
    files = generated_files(load_manual())
    return check(files) if arguments.check else (write(files) or 0)


if __name__ == "__main__":
    raise SystemExit(main())
