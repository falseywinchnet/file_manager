from __future__ import annotations

from hashlib import sha256
import json
from pathlib import Path
from typing import Any

from .css_source import compute_styles, parse_css
from .diagnostics import Diagnostic, WebFormsError
from .html_source import HtmlDocument, parse_html
from .profile import IR_SCHEMA, PROFILE


def _read_styles(document: HtmlDocument, explicit_styles: list[Path] | None) -> list[tuple[str, str]]:
    sources: list[tuple[str, str]] = []
    requested: list[Path] = []
    if explicit_styles:
        requested.extend(explicit_styles)
    else:
        for link in document.style_links:
            candidate = (document.source_path.parent / link).resolve()
            if candidate.parent != document.source_path.parent.resolve():
                raise WebFormsError([Diagnostic("WFS001", "stylesheet must remain beside its HTML specimen", str(candidate))])
            requested.append(candidate)
    for path in requested:
        try:
            data = path.read_bytes()
        except OSError as error:
            raise WebFormsError([Diagnostic("WFS002", f"cannot read stylesheet: {error}", str(path))]) from error
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError as error:
            raise WebFormsError([Diagnostic("WFS003", f"stylesheet is not strict UTF-8: {error}", str(path))]) from error
        sources.append((path.name, text))
    for index, text in enumerate(document.inline_styles):
        sources.append((f"{document.source_path.name}#style-{index + 1}", text))
    if not sources:
        raise WebFormsError([Diagnostic("WFS004", "document has no attached stylesheet", str(document.source_path))])
    return sources


def _normalized_text(chunks: list[str]) -> str:
    return " ".join("".join(chunks).split())


def _runtime_node(document: HtmlDocument, index: int) -> bool:
    current: int | None = index
    while current is not None:
        node = document.nodes[current]
        if node.tag == "body":
            return True
        current = node.parent
    return False


def _feature_policy(feature: str) -> str:
    if feature.startswith(("identity.", "layout.")) or feature == "surface.nested-ambient-context":
        return "structural-required"
    if feature.startswith(("geometry.", "transform.", "decoration.")):
        return "style-geometry-explicit-fallback"
    if feature.startswith(("paint.", "effect.", "surface.authored")):
        return "style-material-explicit-fallback"
    if feature.startswith("state."):
        return "state-default-cue-fallback"
    return "required"


def compile_source(html_path: Path, style_paths: list[Path] | None = None) -> dict[str, Any]:
    html_path = html_path.resolve()
    document = parse_html(html_path)
    style_sources = _read_styles(document, style_paths)
    rules = []
    order = 0
    for path, text in style_sources:
        parsed = parse_css(text, path, order)
        rules.extend(parsed)
        order += len(parsed)
    styles = compute_styles(document, rules)

    digest = sha256()
    html_bytes = html_path.read_bytes()
    digest.update(b"HTML\0")
    digest.update(html_bytes)
    style_records = []
    for path, text in style_sources:
        encoded = text.encode("utf-8")
        digest.update(b"CSS\0")
        digest.update(path.encode("utf-8"))
        digest.update(b"\0")
        digest.update(encoded)
        style_records.append({"path": path, "sha256": sha256(encoded).hexdigest(), "bytes": len(encoded)})

    nodes = []
    control_kinds: set[str] = set()
    elements: set[str] = set()
    for node in document.nodes:
        attributes = dict(sorted(node.attributes.items()))
        control_kind = attributes.get("data-wf-control", node.tag)
        if _runtime_node(document, node.index):
            control_kinds.add(control_kind)
            elements.add(node.tag)
        nodes.append(
            {
                "index": node.index,
                "parent": node.parent,
                "runtime": _runtime_node(document, node.index),
                "tag": node.tag,
                "id": attributes.get("id", ""),
                "classes": attributes.get("class", "").split(),
                "control": control_kind,
                "text": _normalized_text(node.text),
                "attributes": attributes,
                "source": {"path": html_path.name, "line": node.line, "column": node.column},
            }
        )

    properties = {
        item["name"]
        for domain in styles["pools"].values()
        for record in domain
        for item in record["properties"]
    }
    states = {state for variant in styles["variants"] for state in variant["states"]}
    states.update(state for decoration in styles["decorations"] for state in decoration["states"])
    pseudo_elements = {item["pseudo"] for item in styles["decorations"] if item["pseudo"]}
    features: set[str] = {"surface.nested-ambient-context", "identity.typed-hierarchical"}
    for domain_name, records in styles["pools"].items():
        for record in records:
            for item in record["properties"]:
                name = item["name"]
                value = item["value"].lower()
                if name == "display" and value in {"flex", "grid"}:
                    features.add(f"layout.{value}")
                if "linear-gradient(" in value:
                    features.add("paint.linear-gradient")
                    angle_tokens = [
                        token for token in item["typed"]["tokens"]
                        if token["kind"] == "angle_deg"
                    ]
                    if angle_tokens and angle_tokens[0]["number"] % 90.0 != 0.0:
                        features.add("paint.css-angle-gradient")
                if "radial-gradient(" in value:
                    features.add("paint.radial-gradient")
                if name == "box-shadow":
                    shadow_keywords = {
                        token["text"] for token in item["typed"]["tokens"]
                        if token["kind"] == "keyword"
                    }
                    if "inset" in shadow_keywords:
                        features.add("effect.inset-shadow")
                    else:
                        features.add("effect.bounded-shadow")
                if name in {"border-top", "border-right", "border-bottom", "border-left"}:
                    features.add("paint.side-border")
                if name == "border":
                    features.add("paint.uniform-border")
                if name == "border-radius":
                    features.add("geometry.rounded-corners")
                if name == "transform" and "rotate(" in value:
                    features.add("transform.rotate")
                if name == "transform" and "translate" in value:
                    features.add("transform.translate")
                if domain_name == "material" and name.startswith("background"):
                    features.add("surface.authored-material")
    for state in states:
        features.add(f"state.{state}")
    for pseudo in pseudo_elements:
        features.add(f"decoration.{pseudo}")
    if any(item["exposure"] == "exposed" for item in styles["nodes"]):
        features.add("style.exposed-properties")

    return {
        "schema": IR_SCHEMA,
        "profile": PROFILE,
        "source": {
            "html": html_path.name,
            "html_sha256": sha256(html_bytes).hexdigest(),
            "styles": style_records,
            "digest": digest.hexdigest(),
        },
        "document": {"title": document.title, "nodes": nodes},
        "styles": styles,
        "requirements": {
            "elements": sorted(elements),
            "control_kinds": sorted(control_kinds),
            "properties": sorted(properties),
            "states": sorted(states),
            "pseudo_elements": sorted(pseudo_elements),
            "features": sorted(features),
            "feature_policies": {feature: _feature_policy(feature) for feature in sorted(features)},
        },
        "measurements": {
            "static_nodes": len(document.nodes),
            "runtime_nodes": sum(1 for node in nodes if node["runtime"]),
            "style_rules": len(rules),
            "style_records": sum(len(records) for records in styles["pools"].values()),
            "state_variants": len(styles["variants"]),
            "decorations": len(styles["decorations"]),
        },
    }


def write_ir(ir: dict[str, Any], output: Path) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(ir, indent=2, sort_keys=True, ensure_ascii=False) + "\n", encoding="utf-8")


def read_ir(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
        raise WebFormsError([Diagnostic("WFS010", f"cannot read IR: {error}", str(path))]) from error
    if not isinstance(value, dict) or value.get("schema") != IR_SCHEMA:
        raise WebFormsError([Diagnostic("WFS011", f"expected IR schema {IR_SCHEMA!r}", str(path))])
    return value
