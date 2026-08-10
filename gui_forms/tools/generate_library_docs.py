#!/usr/bin/env python3
"""Generate the GUI.Forms iframe reference and AI-readable Markdown mirror.

The parser is deliberately conservative: it inventories declarations from the
checked-in public/private headers and definitions from implementation sources.
Human-authored explanations in docs/library/manual.json override the generated
fallbacks. Generated prose never upgrades a candidate or unmeasured claim.
"""

from __future__ import annotations

import html
import json
import re
import shutil
from dataclasses import dataclass, field
from pathlib import Path


GUI_FORMS = Path(__file__).resolve().parents[1]
OUTPUT = GUI_FORMS / "docs" / "library"
MANUAL_PATH = OUTPUT / "manual.json"


@dataclass
class Method:
    name: str
    signature: str
    access: str = "public"
    explanation: str = ""


@dataclass
class TypeRecord:
    name: str
    kind: str
    header: str
    line: int
    bases: list[str] = field(default_factory=list)
    methods: list[Method] = field(default_factory=list)
    definition_files: list[str] = field(default_factory=list)
    visual: bool = False
    slug: str = ""


def sanitized(text: str) -> str:
    result = list(text)
    index = 0
    while index < len(text):
        if text.startswith("//", index):
            end = text.find("\n", index)
            end = len(text) if end < 0 else end
            for offset in range(index, end):
                result[offset] = " "
            index = end
        elif text.startswith("/*", index):
            end = text.find("*/", index + 2)
            end = len(text) - 2 if end < 0 else end
            for offset in range(index, end + 2):
                if result[offset] != "\n":
                    result[offset] = " "
            index = end + 2
        elif (text[index] == "'" and index > 0 and index + 1 < len(text) and
              text[index - 1].isalnum() and text[index + 1].isalnum()):
            # C++ digit separators are not character-literal delimiters.
            index += 1
        elif text[index] in {'"', "'"}:
            quote = text[index]
            result[index] = " "
            index += 1
            while index < len(text):
                if text[index] == "\\":
                    result[index] = " "
                    if index + 1 < len(text):
                        result[index + 1] = " "
                    index += 2
                    continue
                if text[index] == quote:
                    result[index] = " "
                    index += 1
                    break
                if result[index] != "\n":
                    result[index] = " "
                index += 1
        else:
            index += 1
    return "".join(result)


def matching_brace(text: str, opening: int) -> int:
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return index
    return -1


def normalize_signature(value: str) -> str:
    value = re.sub(r"\s+", " ", value).strip()
    return value[:-1].rstrip() if value.endswith("{") else value


def method_name(signature: str, owner: str) -> str | None:
    function_pointer = re.search(
        r"\(\s*\*\s*([A-Za-z_]\w*)\s*\)", signature)
    if function_pointer:
        return function_pointer.group(1)
    prefix = signature.split("(", 1)[0].strip()
    if not prefix or prefix.startswith(("if ", "for ", "while ", "switch ")):
        return None
    operator = re.search(r"(operator\s*[^\s(]+)\s*$", prefix)
    if operator:
        return operator.group(1).replace(" ", "")
    match = re.search(r"(~?[A-Za-z_]\w*)\s*$", prefix)
    if not match:
        return None
    name = match.group(1)
    if name in {"return", "requires", "sizeof"}:
        return None
    return name


def declared_methods(body: str, kind: str, owner: str) -> list[Method]:
    clean = sanitized(body)
    access = "public" if kind == "struct" else "private"
    depth = 0
    buffer: list[str] = []
    methods: list[Method] = []
    for original_line, clean_line in zip(body.splitlines(), clean.splitlines()):
        stripped = clean_line.strip()
        if depth == 0 and re.fullmatch(r"(public|protected|private)\s*:", stripped):
            access = stripped.split(":", 1)[0]
            buffer.clear()
            continue
        if depth == 0 and stripped:
            buffer.append(original_line.strip())
            candidate = normalize_signature(" ".join(buffer))
            first_brace = clean_line.find("{")
            terminal = ";" in clean_line or first_brace >= 0
            if terminal:
                candidate = candidate.split("{", 1)[0].strip()
                if ("(" in candidate and
                        not candidate.startswith("std::function<") and
                        not re.match(r"^(class|struct|enum|using|typedef)\b", candidate)):
                    name = method_name(candidate, owner)
                    if name:
                        methods.append(Method(
                            name, candidate.rstrip("; "), access))
                buffer.clear()
        depth += clean_line.count("{") - clean_line.count("}")
        if depth < 0:
            depth = 0
        if depth > 0 and buffer:
            buffer.clear()

    unique: dict[str, Method] = {}
    for method in methods:
        unique.setdefault(method.signature, method)
    return list(unique.values())


def base_names(raw: str) -> list[str]:
    if not raw:
        return []
    bases = []
    for part in raw.split(","):
        cleaned = re.sub(r"\b(public|protected|private|virtual)\b", "", part)
        cleaned = cleaned.strip().split("<", 1)[0].split("::")[-1]
        if re.fullmatch(r"[A-Za-z_]\w*", cleaned):
            bases.append(cleaned)
    return bases


def declarations() -> tuple[list[TypeRecord], list[dict]]:
    headers = sorted((GUI_FORMS / "include" / "gui_forms").rglob("*.h*"))
    headers += sorted((GUI_FORMS / "src").rglob("*.hpp"))
    types: list[TypeRecord] = []
    enums: list[dict] = []
    # ``enum class`` declarations are catalogued separately as state/value
    # vocabularies.  Excluding their ``class`` token here keeps the type and
    # vocabulary branches disjoint instead of publishing duplicate pages.
    type_pattern = re.compile(
        r"(?<!enum )\b(class|struct)\s+([A-Za-z_]\w*)\s*(?:final\s*)?(?::\s*([^\{]+))?\s*\{")
    enum_pattern = re.compile(
        r"\benum\s+class\s+([A-Za-z_]\w*)\s*(?::\s*[^\{]+)?\s*\{")
    for path in headers:
        text = path.read_text(encoding="utf-8")
        clean = sanitized(text)
        relative = path.relative_to(GUI_FORMS).as_posix()
        for match in type_pattern.finditer(clean):
            # A qualified friend declaration such as
            # ``friend class detail::PopupAttachment`` is not a nested class
            # declaration.  The conservative declaration regex begins again
            # at ``class detail`` and would otherwise publish a bogus
            # ``detail`` type whose body is the next unrelated brace block.
            line_prefix = clean[clean.rfind("\n", 0, match.start()) + 1:match.start()]
            if re.search(r"\bfriend\s*$", line_prefix):
                continue
            end = matching_brace(clean, match.end() - 1)
            if end < 0:
                continue
            name = match.group(2)
            body = text[match.end():end]
            types.append(TypeRecord(
                name=name,
                kind=match.group(1),
                header=relative,
                line=text.count("\n", 0, match.start()) + 1,
                bases=base_names(match.group(3) or ""),
                methods=declared_methods(body, match.group(1), name),
            ))
        for match in enum_pattern.finditer(clean):
            end = matching_brace(clean, match.end() - 1)
            if end < 0:
                continue
            body = clean[match.end():end]
            values = []
            for part in body.split(","):
                value = re.sub(r"=.*", "", part).strip()
                if re.fullmatch(r"[A-Za-z_]\w*", value):
                    values.append(value)
            enums.append({
                "name": match.group(1),
                "header": relative,
                "line": text.count("\n", 0, match.start()) + 1,
                "values": values,
            })
    return types, enums


def definition_map(types: list[TypeRecord]) -> None:
    sources = sorted((GUI_FORMS / "src").rglob("*.cpp"))
    sources += sorted((GUI_FORMS / "src").rglob("*.mm"))
    source_text = {
        path.relative_to(GUI_FORMS).as_posix(): path.read_text(
            encoding="utf-8", errors="replace")
        for path in sources
    }
    for record in types:
        # Require a definition body (and allow a constructor initializer list)
        # so qualified base calls such as ``Panel::on_paint(...)`` do not make
        # an otherwise isolated type look distributed across derived files.
        needle = re.compile(
            r"(?m)^(?!\s*(?:return|if|for|while|switch|case)\b)"
            r"(?![^;\n{}]*[?=][^;\n{}]*\b" + re.escape(record.name) + r"::)"
            r"[^;\n{}]*\b" + re.escape(record.name) +
            r"::(?:~?" + re.escape(record.name) +
            r"|[A-Za-z_]\w*)\s*\([^;{}]*\)"
            r"(?:\s*(?:const|noexcept|&|&&))*"
            r"(?:\s*:\s*[^;{}]+)?\s*\{")
        record.definition_files = [
            path for path, text in source_text.items() if needle.search(text)
        ]


def mark_visual(types: list[TypeRecord]) -> None:
    visual = {"Control"}
    changed = True
    while changed:
        changed = False
        for record in types:
            if record.name not in visual and any(base in visual for base in record.bases):
                visual.add(record.name)
                changed = True
    for record in types:
        # The ergonomic C++ ABI wrapper is also named Control, but it is a
        # handle owner rather than a retained visual type. The retained root
        # is the declaration that actually derives from Component.
        record.visual = ("Component" in record.bases
                         if record.name == "Control"
                         else record.name in visual)


def slugify(record: TypeRecord, used: set[str]) -> str:
    name = re.sub(r"(?<!^)(?=[A-Z])", "_", record.name).lower()
    candidate = name
    suffix = 2
    while candidate in used:
        candidate = f"{name}_{suffix}"
        suffix += 1
    used.add(candidate)
    return candidate


def default_explanation(owner: str, method: Method) -> str:
    name = method.name
    property_name = name.removeprefix("set_").removeprefix("clear_").replace("_", " ")
    if name in {owner, f"~{owner}"}:
        return f"Constructs or tears down the retained {owner} object according to its ownership contract."
    if name.startswith("set_"):
        return (f"Synchronously updates the retained {property_name} property. "
                "Validation, typed invalidation, and notifications are defined by the implementation.")
    if name.startswith("reset_"):
        return f"Returns {property_name.replace('reset ', '')} to its inherited or default policy."
    if name.startswith("clear_"):
        return f"Removes the explicit {property_name} value and restores fallback behavior."
    if name.startswith("add_"):
        return (f"Adds {property_name.replace('add ', '')} to {owner}'s retained ownership "
                "model after validating identity and lifetime constraints.")
    if name.startswith("remove_"):
        return (f"Removes the exact {property_name.replace('remove ', '')} entry and publishes "
                "the resulting retained-state change when one exists.")
    if name.startswith("subscribe"):
        return "Connects a revocable callback in deterministic registration order."
    if name in {"emit", "publish"}:
        return "Publishes a stable callback snapshot so mutation during delivery affects only later emissions."
    if name in {"start", "resume"}:
        return f"Transitions {owner} into its active state while preserving accumulated state."
    if name == "pause":
        return f"Suspends {owner}'s active progression without discarding its current position."
    if name == "stop":
        return f"Returns {owner} to its stopped baseline and clears active progression."
    if name.startswith("resolve"):
        return "Resolves the requested retained resource against exact identity, scale, and fallback policy."
    if name in {"operator==", "operator<=>"}:
        return "Compares the complete value identity used by deterministic retained-state decisions."
    if name in {"operatorbool", "operator bool"}:
        return "Reports whether the record contains a usable resolved value."
    if name == "measure":
        return "Computes desired size from the available constraint without arranging children."
    if name == "arrange":
        return "Commits final geometry and arranges retained child roles within it."
    if name == "on_paint":
        return "Records renderer-neutral paint operations for the damaged local region."
    if name == "semantic_descriptor":
        return "Projects the current retained state into the framework semantic/accessibility graph."
    if name == "on_pointer":
        return "Consumes normalized routed pointer input and updates retained interaction state."
    if name == "on_key":
        return "Consumes normalized keyboard input for this control's interaction contract."
    if name == "on_focus_changed":
        return "Updates focus-dependent retained state and invalidates affected presentation/semantics."
    if name == "on_activate":
        return "Runs the control's single authoritative activation path."
    if name == "initialize_control_tree":
        return "Idempotently attaches lazily constructed internal controls before layout or use."
    if name.startswith(("is_", "has_", "uses_", "can_")) or ") const" in method.signature:
        return f"Reports the current {name.replace('_', ' ')} value without mutation."
    if owner.endswith("api_v0") or owner.endswith("service_v0"):
        return (f"ABI table entry for {name.replace('_', ' ')}. It applies the documented "
                "result-code, bounded-buffer, ownership, and thread-affinity laws.")
    return (f"Executes {owner}'s {name.replace('_', ' ')} operation against retained state; "
            "the signature records its exact inputs, result, constness, and failure surface.")


def esc(value: str) -> str:
    return html.escape(value, quote=True)


def source_status(record: TypeRecord) -> str:
    if record.header.startswith("src/controls/gallery/"):
        # GalleryControl and GalleryContext are source-private demoboard
        # composition adapters, not reusable library types. Their declarations
        # remain isolated for atlas navigation while one composition unit owns
        # the generated specimen tree and its private helpers.
        return "demonstration composition"
    if len(record.definition_files) != 1:
        return "header-only or distributed"
    source = Path(record.definition_files[0])
    expected = re.sub(r"(?<!^)(?=[A-Z])", "_", record.name).lower()
    # Acronym spelling is intentionally allowed (CoreGraphics/coregraphics,
    # HarfBuzz/harfbuzz).  Removing separators still requires the source stem
    # to name the complete type, so family files such as ``host_types.cpp`` do
    # not acquire a false isolated status.
    source_identity = re.sub(r"[^a-z0-9]", "", source.stem.lower())
    type_identity = re.sub(r"[^a-z0-9]", "", record.name.lower())
    return ("isolated per type" if source_identity == type_identity
            else "grouped migration pending")


def manual_review(record: TypeRecord, manual: dict) -> dict:
    matches = []
    for review_key, review in manual.get("types", {}).items():
        if review.get("name", review_key) != record.name:
            continue
        declaration_header = review.get("declaration_header")
        if declaration_header and declaration_header != record.header:
            continue
        matches.append(review)
    if len(matches) > 1:
        raise RuntimeError(
            f"{record.name} at {record.header} has {len(matches)} manual reviews")
    return matches[0] if matches else {}


def write_page(record: TypeRecord, manual: dict) -> dict:
    override = manual_review(record, manual)
    if "visual" in override:
        record.visual = bool(override["visual"])
    summary = override.get(
        "summary",
        f"{record.name} is a {'visual retained control' if record.visual else record.kind} declared in {record.header}.")
    status = override.get(
        "status", "OBSERVED source inventory; declaration-derived narrative")
    capture = override.get("capture")
    method_overrides = override.get("methods", {})
    for method in record.methods:
        method.explanation = method_overrides.get(
            method.name, default_explanation(record.name, method))

    methods_html = "".join(
        f'<section class="method"><h3>{esc(method.name)} '
        f'<span class="access">{esc(method.access)}</span></h3>'
        f'<code>{esc(method.signature)}</code><p>{esc(method.explanation)}</p></section>'
        for method in record.methods
    ) or '<p class="empty">No public methods were discovered in this declaration.</p>'
    capture_html = (
        f'<figure><img src="../{esc(capture)}" alt="{esc(record.name)} control capture">'
        f'<figcaption>{esc(override.get("capture_note", "Verified control capture."))}</figcaption></figure>'
        if capture else (
        '<p class="pending">Capture pending: the type is inventoried but has not yet passed the Screen Sharing crop gate.</p>'
        if record.visual else
        '<p>Not applicable: this is a nonvisual contract, value, service, or state owner.</p>')
    )
    bases = " → ".join(record.bases + [record.name]) if record.bases else record.name
    definitions = ", ".join(record.definition_files) or "inline/header-only"
    page = f'''<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width">
<title>{esc(record.name)} · GUI.Forms</title><link rel="icon" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'%3E%3Crect width='64' height='64' rx='14' fill='%23131822'/%3E%3Cpath d='M17 18h30v8H25v12h18v8H25v10h-8z' fill='%236ee7c8'/%3E%3C/svg%3E"><link rel="stylesheet" href="../assets/page.css"></head>
<body><article><header><p class="eyebrow">{esc(record.kind)} · {"visual" if record.visual else "library"}</p>
<h1>{esc(record.name)}</h1><p class="summary">{esc(summary)}</p>
<div class="badges"><span>{esc(status)}</span><span>{esc(source_status(record))}</span></div></header>
<section><h2>Visual evidence</h2>{capture_html}</section>
<section><h2>Ownership and hierarchy</h2><dl><dt>Hierarchy</dt><dd>{esc(bases)}</dd>
<dt>Declaration</dt><dd>{esc(record.header)}:{record.line}</dd><dt>Definition</dt><dd>{esc(definitions)}</dd></dl></section>
<section><h2>Declared methods</h2>{methods_html}</section>
<footer><a href="../markdown/{esc(record.slug)}.md">AI-readable Markdown source</a></footer>
</article></body></html>'''
    (OUTPUT / "pages" / f"{record.slug}.html").write_text(page, encoding="utf-8")

    method_md = "\n\n".join(
        f"### `{method.name}` ({method.access})\n\n```cpp\n{method.signature}\n```\n\n{method.explanation}"
        for method in record.methods
    ) or "No public methods were discovered in this declaration."
    markdown = f"""# {record.name}

- Status: **{status}**
- Kind: **{record.kind}{' / visual retained control' if record.visual else ''}**
- Hierarchy: `{bases}`
- Declaration: `{record.header}:{record.line}`
- Definition: `{definitions}`

{summary}

## Visual evidence

{f'![{record.name}](../{capture})' if capture else ('Capture pending; this visual type has not yet passed the Screen Sharing crop gate.' if record.visual else 'Not applicable: this is a nonvisual contract, value, service, or state owner.')}

## Declared methods

{method_md}
"""
    (OUTPUT / "markdown" / f"{record.slug}.md").write_text(markdown, encoding="utf-8")
    return {
        "name": record.name,
        "kind": record.kind,
        "bases": record.bases,
        "header": record.header,
        "visual": record.visual,
        "page": f"pages/{record.slug}.html",
        "status": status,
        "sourceStatus": source_status(record),
        "search": " ".join([record.name, record.header, summary] + record.bases).lower(),
    }


def write_enum_page(enum: dict) -> dict:
    slug = "enum_" + re.sub(r"(?<!^)(?=[A-Z])", "_", enum["name"]).lower()
    values = "\n".join(f"- `{value}`" for value in enum["values"])
    markdown = f"""# {enum['name']}

- Status: **OBSERVED source inventory; declaration-derived narrative**
- Declaration: `{enum['header']}:{enum['line']}`

This closed vocabulary makes the named state explicit at API boundaries; the
declaration below is authoritative for admitted values.

## Declared values

{values or 'No values discovered.'}
"""
    (OUTPUT / "markdown" / f"{slug}.md").write_text(markdown, encoding="utf-8")
    value_html = "".join(f"<li><code>{esc(value)}</code></li>" for value in enum["values"])
    page = f'''<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width">
<title>{esc(enum['name'])} · GUI.Forms</title><link rel="icon" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'%3E%3Crect width='64' height='64' rx='14' fill='%23131822'/%3E%3Cpath d='M17 18h30v8H25v12h18v8H25v10h-8z' fill='%236ee7c8'/%3E%3C/svg%3E"><link rel="stylesheet" href="../assets/page.css"></head>
<body><article><header><p class="eyebrow">enum class · state/value vocabulary</p><h1>{esc(enum['name'])}</h1>
<div class="badges"><span>OBSERVED source inventory</span></div></header><section><h2>Purpose</h2>
<p>This closed vocabulary makes the named state explicit at API boundaries; the declaration below is authoritative for admitted values.</p></section><section><h2>Declared values</h2>
<ul>{value_html}</ul><p>Declaration: <code>{esc(enum['header'])}:{enum['line']}</code></p></section>
<footer><a href="../markdown/{slug}.md">AI-readable Markdown source</a></footer></article></body></html>'''
    (OUTPUT / "pages" / f"{slug}.html").write_text(page, encoding="utf-8")
    return {
        "name": enum["name"], "header": enum["header"],
        "page": f"pages/{slug}.html", "values": enum["values"],
        "search": f"{enum['name']} {enum['header']} {' '.join(enum['values'])}".lower(),
    }


def write_assets() -> None:
    (OUTPUT / "assets" / "library.css").write_text('''
:root{color-scheme:light dark;font:14px/1.45 system-ui,sans-serif}*{box-sizing:border-box}body{margin:0;display:grid;grid-template-columns:minmax(250px,23vw) 1fr;height:100vh;background:#10151d;color:#e8edf4}aside{border-right:1px solid #344052;overflow:auto;padding:14px;background:#151c26}h1{font-size:18px;margin:0 0 12px}input{width:100%;padding:9px;border:1px solid #47566b;border-radius:6px;background:#0e141c;color:inherit}nav h2{font-size:12px;text-transform:uppercase;letter-spacing:.08em;color:#93a5bb;margin:18px 0 6px}nav a{display:block;color:#dce7f4;text-decoration:none;padding:5px 7px;border-radius:4px}nav a:hover,nav a.active{background:#2b3d55}nav .pending{color:#aab6c4}iframe{width:100%;height:100%;border:0;background:#f8fafc}.count{color:#91a3b8;font-size:12px;margin-top:8px}
'''.strip() + "\n", encoding="utf-8")
    (OUTPUT / "assets" / "page.css").write_text('''
:root{font:15px/1.55 system-ui,sans-serif;color:#172033;background:#f8fafc}body{margin:0}article{max-width:980px;margin:auto;padding:38px 44px 70px}h1{font-size:38px;line-height:1.1;margin:.15em 0}h2{margin-top:2em;border-bottom:1px solid #d8e0ea;padding-bottom:.3em}.eyebrow{color:#5d7088;text-transform:uppercase;letter-spacing:.1em;font-size:12px}.summary{font-size:18px;color:#35455b}.badges{display:flex;gap:8px;flex-wrap:wrap}.badges span,.access{background:#e5edf7;border:1px solid #c8d5e5;border-radius:999px;padding:3px 9px;font-size:12px}.access{font-weight:500;margin-left:6px;color:#4c6078}.method{border-left:3px solid #90a9c7;padding:2px 0 8px 16px;margin:20px 0}.method h3{margin-bottom:5px}.method code{display:block;white-space:pre-wrap;background:#edf2f7;border-radius:5px;padding:9px}dl{display:grid;grid-template-columns:120px 1fr;gap:8px}dt{font-weight:700}dd{margin:0}figure{margin:0}img{max-width:100%;border:1px solid #c7d1dd;border-radius:7px;box-shadow:0 8px 24px #23364b24}.pending{padding:14px;border:1px dashed #c08a32;background:#fff8e7;border-radius:6px}footer{margin-top:45px}a{color:#245f9e}@media(max-width:650px){article{padding:25px 20px}dl{grid-template-columns:1fr}h1{font-size:31px}}
'''.strip() + "\n", encoding="utf-8")
    (OUTPUT / "assets" / "library.js").write_text('''
const manifest=window.GUI_FORMS_LIBRARY;const nav=document.querySelector("nav");const frame=document.querySelector("iframe");const input=document.querySelector("input");const count=document.querySelector(".count");
function section(title,items){const heading=document.createElement("h2");heading.textContent=title;nav.append(heading);for(const item of items){const link=document.createElement("a");link.href=item.page;link.target="reference";link.textContent=item.name;link.dataset.search=item.search;link.className=item.status?.includes("pending")?"pending":"";link.onclick=()=>{document.querySelectorAll("nav a").forEach(a=>a.classList.remove("active"));link.classList.add("active")};nav.append(link)}}
section("Visual controls",manifest.types.filter(x=>x.visual));section("Library types",manifest.types.filter(x=>!x.visual));section("State and value vocabularies",manifest.enums);const links=[...nav.querySelectorAll("a")];count.textContent=`${links.length} pages`;input.addEventListener("input",()=>{const q=input.value.trim().toLowerCase();let visible=0;for(const link of links){const show=!q||link.dataset.search.includes(q);link.hidden=!show;if(show)visible++}count.textContent=`${visible} of ${links.length} pages`});if(links[0]){links[0].classList.add("active");frame.src=links[0].href}
'''.strip() + "\n", encoding="utf-8")


def write_index() -> None:
    index = '''<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width">
<title>GUI.Forms library atlas</title><link rel="icon" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'%3E%3Crect width='64' height='64' rx='14' fill='%23131822'/%3E%3Cpath d='M17 18h30v8H25v12h18v8H25v10h-8z' fill='%236ee7c8'/%3E%3C/svg%3E"><link rel="stylesheet" href="assets/library.css"></head><body>
<aside><h1>GUI.Forms library atlas</h1><input type="search" placeholder="Find a control, method family, or state…" aria-label="Search library">
<p class="count"></p><nav></nav></aside><iframe name="reference" title="GUI.Forms reference"></iframe>
<script src="manifest.js"></script><script src="assets/library.js"></script></body></html>'''
    (OUTPUT / "index.html").write_text(index, encoding="utf-8")


def validate_output(payload: dict, manual: dict,
                    types: list[TypeRecord]) -> None:
    records = payload["types"] + payload["enums"]
    pages = [record["page"] for record in records]
    expected = len(records)
    if len(set(pages)) != expected:
        raise RuntimeError("generated documentation contains duplicate page paths")
    if len(list((OUTPUT / "pages").glob("*.html"))) != expected:
        raise RuntimeError("generated HTML page count does not match the manifest")
    if len(list((OUTPUT / "markdown").glob("*.md"))) != expected:
        raise RuntimeError("generated Markdown page count does not match the manifest")
    for record in records:
        if not (OUTPUT / record["page"]).is_file():
            raise RuntimeError(f"missing generated page: {record['page']}")
        if not (GUI_FORMS / record["header"]).is_file():
            raise RuntimeError(f"missing declared source: {record['header']}")
    unreviewed = [
        f"{record.name}@{record.header}"
        for record in types if not manual_review(record, manual)
    ]
    if unreviewed:
        raise RuntimeError(
            "manual type narratives are missing: " + ", ".join(unreviewed))
    pending_migrations = [
        f"{record['name']}@{record['header']}"
        for record in payload["types"]
        if record["sourceStatus"] == "grouped migration pending"
    ]
    if pending_migrations:
        raise RuntimeError(
            "grouped source migrations remain: " + ", ".join(pending_migrations))
    missing_visual_evidence = [
        f"{record.name}@{record.header}"
        for record in types
        if record.visual and not manual_review(record, manual).get("capture")
    ]
    if missing_visual_evidence:
        raise RuntimeError(
            "visual declarations lack verified capture evidence: " +
            ", ".join(missing_visual_evidence))
    for review_key, review in manual.get("types", {}).items():
        name = review.get("name", review_key)
        capture = review.get("capture")
        if capture and not (OUTPUT / capture).is_file():
            raise RuntimeError(f"{name} references missing capture: {capture}")
        candidates = [record for record in types if record.name == name]
        declaration_header = review.get("declaration_header")
        if declaration_header:
            candidates = [record for record in candidates
                          if record.header == declaration_header]
        if len(candidates) != 1:
            raise RuntimeError(
                f"manual review {name} resolves to {len(candidates)} declarations")
        method_scope = review.get("method_scope", "public")
        if method_scope not in {"public", "all"}:
            raise RuntimeError(
                f"{name} review has invalid method_scope: {method_scope}")
        # A summary-only review deliberately accepts the generator's
        # declaration-derived per-method narrative. Once an author supplies a
        # methods object, require it to be complete so partial hand review can
        # never masquerade as a complete one.
        if "methods" not in review:
            continue
        discovered = {
            method.name for method in candidates[0].methods
            if method_scope == "all" or method.access == "public"
        }
        explained = set(review.get("methods", {}))
        missing = sorted(discovered - explained)
        stale = sorted(explained - discovered)
        if missing:
            raise RuntimeError(
                f"{name} review is missing method explanations: {missing}")
        if stale:
            raise RuntimeError(
                f"{name} review has stale method explanations: {stale}")


def main() -> int:
    manual = json.loads(MANUAL_PATH.read_text(encoding="utf-8"))
    types, enums = declarations()
    definition_map(types)
    mark_visual(types)
    used: set[str] = set()
    for record in sorted(types, key=lambda item: (not item.visual, item.name, item.header)):
        record.slug = slugify(record, used)

    for directory in ("pages", "markdown", "assets"):
        target = OUTPUT / directory
        if target.exists():
            shutil.rmtree(target)
        target.mkdir(parents=True)
    (OUTPUT / "captures").mkdir(exist_ok=True)

    type_manifest = [write_page(record, manual) for record in types]
    enum_manifest = [write_enum_page(enum) for enum in enums]
    type_manifest.sort(key=lambda item: (not item["visual"], item["name"], item["header"]))
    enum_manifest.sort(key=lambda item: (item["name"], item["header"]))
    payload = {"schema": 1, "types": type_manifest, "enums": enum_manifest}
    (OUTPUT / "manifest.js").write_text(
        "window.GUI_FORMS_LIBRARY=" + json.dumps(payload, separators=(",", ":")) + ";\n",
        encoding="utf-8")
    (OUTPUT / "manifest.json").write_text(
        json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_assets()
    write_index()
    validate_output(payload, manual, types)
    print(f"GUI.Forms library docs: PASS ({len(types)} types, {len(enums)} enums)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
