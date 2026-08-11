from __future__ import annotations

from dataclasses import dataclass
import json
import math
from pathlib import Path
from typing import Any

from .diagnostics import Diagnostic, WebFormsError
from .profile import IR_SCHEMA
from .stage2 import _symbol, validate_generated_cpp


@dataclass(frozen=True)
class TypographyRecipe:
    role: str
    size: float
    weight: int
    italic: bool
    letter_spacing: float
    line_spacing: float
    label_alignment: str
    button_alignment: str
    text_transform: str
    wrapping: str
    foreground: tuple[int, int, int, int]


def _diagnostic(code: str, message: str) -> WebFormsError:
    return WebFormsError([Diagnostic(code, message, "<ir>")])


def _properties(record: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {item["name"]: item for item in record["properties"]}


def _one_token(item: dict[str, Any], purpose: str) -> dict[str, Any]:
    tokens = item["typed"]["tokens"]
    if len(tokens) != 1:
        raise _diagnostic("WFGT010", f"{purpose} requires one canonical token")
    return tokens[0]


def _number(value: float) -> str:
    rendered = repr(float(value))
    return rendered if "." in rendered else rendered + ".0"


def _font_role(item: dict[str, Any]) -> str:
    names = tuple(token.get("text", "") for token in item["typed"]["tokens"])
    mappings = {
        ("Carlito",): "gui_forms::FontRole::content",
        ("Carlito", "sans-serif"): "gui_forms::FontRole::content",
        ("Cousine",): "gui_forms::FontRole::monospace",
        ("Cousine", "monospace"): "gui_forms::FontRole::monospace",
        ("Portsmouth Rapids",): "gui_forms::FontRole::control",
        ("Portsmouth Rapids", "sans-serif"): "gui_forms::FontRole::control",
    }
    role = mappings.get(names)
    if role is None:
        raise _diagnostic(
            "WFGT011", "font-family must resolve through the bundled GUI.Forms font pack"
        )
    return role


def _logical_number(
    item: dict[str, Any], purpose: str, minimum: float, maximum: float,
) -> float:
    token = _one_token(item, purpose)
    if token.get("kind") not in {"number", "logical_px"}:
        raise _diagnostic("WFGT012", f"{purpose} requires a number or logical pixels")
    value = float(token.get("number", 0.0))
    if not math.isfinite(value) or value < minimum or value > maximum:
        raise _diagnostic(
            "WFGT013", f"{purpose} must be between {minimum} and {maximum}"
        )
    return value


def _keyword(
    properties: dict[str, dict[str, Any]], name: str, default: str,
) -> str:
    item = properties.get(name)
    if item is None:
        return default
    token = _one_token(item, name)
    if token.get("kind") != "keyword":
        raise _diagnostic("WFGT014", f"{name} requires one keyword")
    return str(token.get("text", ""))


def _foreground(record: dict[str, Any]) -> tuple[int, int, int, int]:
    item = _properties(record).get("color")
    if item is None:
        raise _diagnostic("WFGT015", "native text projection requires a computed color")
    token = _one_token(item, "color")
    if token.get("kind") != "color_rgba":
        raise _diagnostic("WFGT016", "native text color requires packed RGBA")
    packed = int(token.get("data", 0))
    return (
        (packed >> 24) & 0xFF,
        (packed >> 16) & 0xFF,
        (packed >> 8) & 0xFF,
        packed & 0xFF,
    )


def _typography_recipe(
    typography: dict[str, Any], material: dict[str, Any], tag: str,
) -> TypographyRecipe:
    properties = _properties(typography)
    family = properties.get("font-family")
    size_item = properties.get("font-size")
    if family is None or size_item is None:
        raise _diagnostic(
            "WFGT017", "native typography requires an explicit or inherited family and size"
        )
    role = _font_role(family)
    size = _logical_number(size_item, "font-size", 1.0, 256.0)
    weight = 400
    if "font-weight" in properties:
        value = _logical_number(properties["font-weight"], "font-weight", 1.0, 1000.0)
        if value != int(value):
            raise _diagnostic("WFGT018", "font-weight must be an integer")
        weight = int(value)
    style = _keyword(properties, "font-style", "normal")
    if style not in {"normal", "italic"}:
        raise _diagnostic("WFGT019", "native typography supports normal and italic")
    letter_spacing = 0.0
    if "letter-spacing" in properties:
        letter_spacing = _logical_number(
            properties["letter-spacing"], "letter-spacing", -size * 0.25, size
        )
    line_spacing = 1.25
    if "line-height" in properties:
        token = _one_token(properties["line-height"], "line-height")
        if token.get("kind") == "number":
            line_spacing = float(token.get("number", 0.0))
        elif token.get("kind") == "logical_px":
            line_spacing = float(token.get("number", 0.0)) / size
        else:
            raise _diagnostic("WFGT020", "line-height requires a number or logical pixels")
        if not math.isfinite(line_spacing) or line_spacing < 0.75 or line_spacing > 3.0:
            raise _diagnostic("WFGT021", "native line-height ratio must be from 0.75 through 3")
    alignment = _keyword(properties, "text-align", "left")
    alignments = {
        "left": ("gui_forms::HorizontalAlignment::near", "gui_forms::ContentAlignment::middle_left"),
        "start": ("gui_forms::HorizontalAlignment::near", "gui_forms::ContentAlignment::middle_left"),
        "center": ("gui_forms::HorizontalAlignment::center", "gui_forms::ContentAlignment::middle_center"),
        "right": ("gui_forms::HorizontalAlignment::far", "gui_forms::ContentAlignment::middle_right"),
        "end": ("gui_forms::HorizontalAlignment::far", "gui_forms::ContentAlignment::middle_right"),
    }
    native_alignment = alignments.get(alignment)
    if native_alignment is None:
        raise _diagnostic("WFGT022", "justified text needs a retained justification solver")
    transform = _keyword(properties, "text-transform", "none")
    transforms = {
        "none": "gui_forms::TextCaseTransform::none",
        "uppercase": "gui_forms::TextCaseTransform::uppercase_ascii",
        "lowercase": "gui_forms::TextCaseTransform::lowercase_ascii",
    }
    native_transform = transforms.get(transform)
    if native_transform is None:
        raise _diagnostic("WFGT023", "capitalize needs a bounded Unicode word transform")
    if tag == "button" and transform != "none":
        raise _diagnostic("WFGT024", "button text transforms need a retained button transform")
    decoration = _keyword(properties, "text-decoration", "none")
    if decoration != "none":
        raise _diagnostic("WFGT025", "text decoration lowering is not yet retained")
    white_space = _keyword(properties, "white-space", "normal")
    wrapping = {
        "normal": "gui_forms::TextWrapping::word",
        "nowrap": "gui_forms::TextWrapping::no_wrap",
    }.get(white_space)
    if wrapping is None:
        raise _diagnostic("WFGT026", "preformatted whitespace needs a retained text mode")
    if "word-break" in properties and _keyword(properties, "word-break", "normal") != "normal":
        raise _diagnostic("WFGT027", "non-normal word-break needs a retained breaker")
    if "text-overflow" in properties:
        raise _diagnostic("WFGT028", "text-overflow needs retained clipping and ellipsis")
    return TypographyRecipe(
        role, size, weight, style == "italic", letter_spacing, line_spacing,
        native_alignment[0], native_alignment[1], native_transform, wrapping,
        _foreground(material),
    )


def _emit_color(value: tuple[int, int, int, int]) -> str:
    return (
        "gui_forms::Color::rgba("
        f"{value[0]}U, {value[1]}U, {value[2]}U, {value[3]}U)"
    )


def generate_gui_typography(
    ir: dict[str, Any], manifest: dict[str, Any], output_dir: Path,
    unit_name: str | None = None,
) -> tuple[Path, Path, Path]:
    if ir.get("schema") != IR_SCHEMA:
        raise _diagnostic("WFGT001", f"native typography Stage 2 requires {IR_SCHEMA!r}")
    capability = manifest.get("features", {}).get("typography.retained", {})
    if capability.get("status") != "supported":
        raise _diagnostic("WFGT002", "target manifest does not support retained typography")
    font_capability = manifest.get("features", {}).get("typography.bundled-font-pack", {})
    if font_capability.get("status") != "supported":
        raise _diagnostic("WFGT003", "target manifest does not support the bundled font pack")

    nodes = ir["document"]["nodes"]
    style_by_node = {item["node"]: item for item in ir["styles"]["nodes"]}
    typography_pool = ir["styles"]["pools"]["typography"]
    material_pool = ir["styles"]["pools"]["material"]
    recipes: dict[int, TypographyRecipe] = {}
    records: list[dict[str, Any]] = []
    for node in nodes:
        if not node["runtime"]:
            continue
        style = style_by_node[node["index"]]
        try:
            recipe = _typography_recipe(
                typography_pool[style["typography"]],
                material_pool[style["material"]], node["tag"],
            )
        except WebFormsError as error:
            records.append(
                {
                    "node": node["index"], "id": node["id"],
                    "status": "unavailable",
                    "diagnostics": [item.render() for item in error.diagnostics],
                }
            )
        else:
            recipes[node["index"]] = recipe
            records.append(
                {
                    "node": node["index"], "id": node["id"],
                    "status": "bounded-native-typography-projection",
                    "diagnostics": [],
                }
            )

    root_id = next((node["id"] for node in nodes if node["tag"] == "body"), "form")
    unit = _symbol(unit_name or root_id)
    namespace = f"web_forms_generated_{unit}"
    output_dir.mkdir(parents=True, exist_ok=True)
    header_path = output_dir / f"{unit}.gui_typography.wf.hpp"
    source_path = output_dir / f"{unit}.gui_typography.wf.cpp"
    report_path = output_dir / f"{unit}.gui_typography.json"
    header_lines = [
        "#pragma once", "", "#include <cstddef>",
        '#include "gui_forms/controls/button_base/button_base.hpp"',
        '#include "gui_forms/controls/label/label.hpp"', "",
        f"namespace {namespace} {{", "",
        "bool has_native_typography(std::size_t node_index) noexcept;",
        "gui_forms::FontSpec make_native_font(std::size_t node_index);",
        "void apply_native_label_typography(std::size_t node_index, gui_forms::Label& label);",
        "void apply_native_button_typography(std::size_t node_index, gui_forms::ButtonBase& button);",
        "", f"}}  // namespace {namespace}", "",
    ]
    source_lines = [
        f'#include "{header_path.name}"', "", "#include <stdexcept>", "",
        f"namespace {namespace} {{", "",
        "bool has_native_typography(std::size_t node_index) noexcept {",
        "    switch (node_index) {",
    ]
    for node_index in recipes:
        source_lines.append(f"    case {node_index}U:")
    if recipes:
        source_lines.append("        return true;")
    source_lines.extend(
        ["    default:", "        return false;", "    }", "}", "",
         "gui_forms::FontSpec make_native_font(std::size_t node_index) {",
         "    switch (node_index) {"]
    )
    for node_index, recipe in recipes.items():
        source_lines.extend(
            [
                f"    case {node_index}U:",
                "        return {"
                f"{recipe.role}, {_number(recipe.size)}, {recipe.weight}U, "
                + ("true, " if recipe.italic else "false, ")
                + f"{_number(recipe.letter_spacing)}}};",
            ]
        )
    source_lines.extend(
        ["    default:",
         "        throw std::invalid_argument(\"node has no bounded native typography\");",
         "    }", "}", "",
         "void apply_native_label_typography(",
         "    std::size_t node_index, gui_forms::Label& label) {",
         "    switch (node_index) {"]
    )
    for node_index, recipe in recipes.items():
        source_lines.extend(
            [
                f"    case {node_index}U:",
                "        label.set_font(make_native_font(node_index));",
                f"        label.set_line_spacing({_number(recipe.line_spacing)});",
                f"        label.set_alignment({recipe.label_alignment});",
                f"        label.set_text_case_transform({recipe.text_transform});",
                f"        label.set_text_wrapping({recipe.wrapping});",
                f"        label.set_foreground({_emit_color(recipe.foreground)});",
                "        return;",
            ]
        )
    source_lines.extend(
        ["    default:",
         "        throw std::invalid_argument(\"node has no bounded native label typography\");",
         "    }", "}", "",
         "void apply_native_button_typography(",
         "    std::size_t node_index, gui_forms::ButtonBase& button) {",
         "    switch (node_index) {"]
    )
    for node_index, recipe in recipes.items():
        source_lines.extend(
            [
                f"    case {node_index}U:",
                "        button.set_font(make_native_font(node_index));",
                f"        button.set_text_line_spacing({_number(recipe.line_spacing)});",
                f"        button.set_text_alignment({recipe.button_alignment});",
                "        return;",
            ]
        )
    source_lines.extend(
        ["    default:",
         "        throw std::invalid_argument(\"node has no bounded native button typography\");",
         "    }", "}", "", f"}}  // namespace {namespace}", ""]
    )
    header_text = "\n".join(header_lines)
    source_text = "\n".join(source_lines)
    validate_generated_cpp(header_text, header_path)
    validate_generated_cpp(source_text, source_path)
    header_path.write_text(header_text, encoding="utf-8")
    source_path.write_text(source_text, encoding="utf-8")
    report = {
        "schema": "web.forms.gui-typography-projection/0.1-experimental",
        "target": manifest.get("target", "unknown"),
        "source_digest": ir["source"]["digest"],
        "status": "bounded-native-typography-projection",
        "exact_typography_count": len(recipes),
        "unavailable_typography_count": len(records) - len(recipes),
        "nodes": records,
    }
    report_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return header_path, source_path, report_path
