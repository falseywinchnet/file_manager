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
class Border:
    width: float
    color: tuple[int, int, int, int]


@dataclass(frozen=True)
class DecorationRecipe:
    layer: str
    width: float
    height: float
    left: float | None
    top: float | None
    right: float | None
    bottom: float | None
    rotation: float
    borders: tuple[Border | None, Border | None, Border | None, Border | None]


def _diagnostic(code: str, message: str) -> WebFormsError:
    return WebFormsError([Diagnostic(code, message, "<ir>")])


def _properties(record: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {item["name"]: item for item in record["properties"]}


def _number(value: float) -> str:
    rendered = repr(float(value))
    return rendered if "." in rendered else rendered + ".0"


def _length(properties: dict[str, dict[str, Any]], name: str) -> float | None:
    item = properties.get(name)
    if item is None:
        return None
    tokens = item["typed"]["tokens"]
    if len(tokens) != 1 or tokens[0].get("kind") not in {"number", "logical_px"}:
        raise _diagnostic("WFGD010", f"decoration {name} requires logical pixels")
    value = float(tokens[0].get("number", 0.0))
    if not math.isfinite(value) or abs(value) > 4096.0:
        raise _diagnostic("WFGD011", f"decoration {name} exceeds bounded geometry")
    return value


def _border(item: dict[str, Any], name: str) -> Border:
    tokens = item["typed"]["tokens"]
    if (
        len(tokens) != 3
        or tokens[0].get("kind") not in {"number", "logical_px"}
        or tokens[1].get("kind") != "keyword"
        or tokens[1].get("text") != "solid"
        or tokens[2].get("kind") != "color_rgba"
    ):
        raise _diagnostic("WFGD012", f"{name} requires width solid color")
    width = float(tokens[0].get("number", 0.0))
    if not math.isfinite(width) or width <= 0.0 or width > 64.0:
        raise _diagnostic("WFGD013", f"{name} width is outside the retained bound")
    packed = int(tokens[2].get("data", 0))
    return Border(
        width,
        ((packed >> 24) & 0xFF, (packed >> 16) & 0xFF,
         (packed >> 8) & 0xFF, packed & 0xFF),
    )


def _recipe(
    decoration: dict[str, Any], geometry: dict[str, Any],
    typography: dict[str, Any], material: dict[str, Any],
) -> DecorationRecipe:
    if decoration["states"]:
        raise _diagnostic("WFGD014", "stateful pseudo-decoration needs retained state recipes")
    if decoration["pseudo"] not in {"before", "after"}:
        raise _diagnostic("WFGD015", "native decoration requires ::before or ::after")
    if decoration["content"]:
        raise _diagnostic("WFGD016", "textual pseudo-content needs retained text decoration")
    if typography["properties"]:
        raise _diagnostic("WFGD017", "box pseudo-decoration cannot carry typography")
    geometry_properties = _properties(geometry)
    allowed_geometry = {
        "position", "width", "height", "left", "top", "right", "bottom",
        "transform",
    }
    if set(geometry_properties) - allowed_geometry:
        raise _diagnostic("WFGD018", "pseudo-decoration uses unsupported geometry")
    position = geometry_properties.get("position")
    if position is None or position["value"] != "absolute":
        raise _diagnostic("WFGD019", "box pseudo-decoration requires position:absolute")
    width = _length(geometry_properties, "width")
    height = _length(geometry_properties, "height")
    if width is None or height is None or width <= 0.0 or height <= 0.0:
        raise _diagnostic("WFGD020", "box pseudo-decoration requires positive width and height")
    left = _length(geometry_properties, "left")
    top = _length(geometry_properties, "top")
    right = _length(geometry_properties, "right")
    bottom = _length(geometry_properties, "bottom")
    if (left is None) == (right is None) or (top is None) == (bottom is None):
        raise _diagnostic("WFGD021", "decoration requires exactly one anchor per axis")
    rotation = 0.0
    if "transform" in geometry_properties:
        tokens = geometry_properties["transform"]["typed"]["tokens"]
        if (
            len(tokens) != 2
            or tokens[0].get("kind") != "keyword"
            or tokens[0].get("text") != "rotate"
            or tokens[1].get("kind") != "angle_deg"
        ):
            raise _diagnostic("WFGD022", "decoration transform supports one rotate angle")
        rotation = float(tokens[1].get("number", 0.0))
        if not math.isfinite(rotation) or abs(rotation) > 360.0:
            raise _diagnostic("WFGD023", "decoration rotation is outside the retained bound")
    material_properties = _properties(material)
    allowed_material = {"border-top", "border-right", "border-bottom", "border-left"}
    if set(material_properties) - allowed_material:
        raise _diagnostic("WFGD024", "box pseudo-decoration supports only side borders")
    names = ("border-top", "border-right", "border-bottom", "border-left")
    borders = tuple(
        _border(material_properties[name], name) if name in material_properties else None
        for name in names
    )
    if all(border is None for border in borders):
        raise _diagnostic("WFGD025", "box pseudo-decoration requires a visible border edge")
    layer = (
        "gui_forms::OwnerDecorationLayer::before_content"
        if decoration["pseudo"] == "before"
        else "gui_forms::OwnerDecorationLayer::after_content"
    )
    return DecorationRecipe(
        layer, width, height, left, top, right, bottom, rotation,
        (borders[0], borders[1], borders[2], borders[3]),
    )


def _emit_color(color: tuple[int, int, int, int]) -> str:
    return (
        "gui_forms::Color::rgba("
        f"{color[0]}U, {color[1]}U, {color[2]}U, {color[3]}U)"
    )


def generate_gui_decorations(
    ir: dict[str, Any], manifest: dict[str, Any], output_dir: Path,
    unit_name: str | None = None,
) -> tuple[Path, Path, Path]:
    if ir.get("schema") != IR_SCHEMA:
        raise _diagnostic("WFGD001", f"native decoration Stage 2 requires {IR_SCHEMA!r}")
    feature_manifest = manifest.get("features", {})
    for feature in ("decoration.before", "decoration.after", "transform.rotate"):
        if feature in ir["requirements"]["features"] and (
            feature_manifest.get(feature, {}).get("status") != "supported"
        ):
            raise _diagnostic("WFGD002", f"target manifest does not support {feature}")

    nodes = ir["document"]["nodes"]
    pools = ir["styles"]["pools"]
    grouped: dict[int, list[DecorationRecipe]] = {}
    records: list[dict[str, Any]] = []
    for decoration_index, decoration in enumerate(ir["styles"]["decorations"]):
        owner_index = decoration["node"]
        owner_children = [
            node for node in nodes
            if node["runtime"] and node["parent"] == owner_index
        ]
        try:
            if owner_children:
                raise _diagnostic(
                    "WFGD003", "first owned pseudo-decoration subset requires a leaf owner"
                )
            recipe = _recipe(
                decoration, pools["geometry"][decoration["geometry"]],
                pools["typography"][decoration["typography"]],
                pools["material"][decoration["material"]],
            )
        except WebFormsError as error:
            records.append(
                {
                    "decoration": decoration_index, "owner": owner_index,
                    "status": "unavailable",
                    "diagnostics": [item.render() for item in error.diagnostics],
                }
            )
        else:
            grouped.setdefault(owner_index, []).append(recipe)
            records.append(
                {
                    "decoration": decoration_index, "owner": owner_index,
                    "status": "bounded-native-owner-decoration",
                    "diagnostics": [],
                }
            )
    if any(len(recipes) > 8 for recipes in grouped.values()):
        raise _diagnostic("WFGD004", "one owner may retain at most eight decorations")

    root_id = next((node["id"] for node in nodes if node["tag"] == "body"), "form")
    unit = _symbol(unit_name or root_id)
    namespace = f"web_forms_generated_{unit}"
    output_dir.mkdir(parents=True, exist_ok=True)
    header_path = output_dir / f"{unit}.gui_decorations.wf.hpp"
    source_path = output_dir / f"{unit}.gui_decorations.wf.cpp"
    report_path = output_dir / f"{unit}.gui_decorations.json"
    header_lines = [
        "#pragma once", "", "#include <cstddef>", '#include "gui_forms/control.hpp"',
        "", f"namespace {namespace} {{", "",
        "bool has_native_owner_decorations(std::size_t node_index) noexcept;",
        "void apply_native_owner_decorations(std::size_t node_index, gui_forms::Control& control);",
        "", f"}}  // namespace {namespace}", "",
    ]
    source_lines = [
        f'#include "{header_path.name}"', "", "#include <stdexcept>", "",
        f"namespace {namespace} {{", "",
        "bool has_native_owner_decorations(std::size_t node_index) noexcept {",
        "    switch (node_index) {",
    ]
    for node_index in grouped:
        source_lines.append(f"    case {node_index}U:")
    if grouped:
        source_lines.append("        return true;")
    source_lines.extend(
        ["    default:", "        return false;", "    }", "}", "",
         "void apply_native_owner_decorations(",
         "    std::size_t node_index, gui_forms::Control& control) {",
         "    static_cast<void>(control);", "    switch (node_index) {"]
    )
    for node_index, recipes in grouped.items():
        source_lines.append(f"    case {node_index}U: {{")
        for recipe_index, recipe in enumerate(recipes):
            edge_pointers: list[str] = []
            for edge_index, border in enumerate(recipe.borders):
                if border is None:
                    edge_pointers.append("nullptr")
                    continue
                symbol = f"border_{recipe_index}_{edge_index}"
                source_lines.append(
                    f"        const gui_forms::MaterialBorder {symbol} = "
                    f"{{{_emit_color(border.color)}, {_number(border.width)}}};"
                )
                edge_pointers.append(f"&{symbol}")
            source_lines.extend(
                [
                    f"        gui_forms::OwnerDecorationRecipe decoration_{recipe_index};",
                    f"        decoration_{recipe_index}.layer = {recipe.layer};",
                    f"        decoration_{recipe_index}.size = "
                    f"{{{_number(recipe.width)}, {_number(recipe.height)}}};",
                ]
            )
            for name in ("left", "top", "right", "bottom"):
                value = getattr(recipe, name)
                if value is not None:
                    source_lines.append(
                        f"        decoration_{recipe_index}.{name} = {_number(value)};"
                    )
            source_lines.extend(
                [
                    f"        decoration_{recipe_index}.rotation_degrees = "
                    f"{_number(recipe.rotation)};",
                    f"        decoration_{recipe_index}.border_edges = "
                    "gui_forms::MaterialBorderEdges::from_parts("
                    + ", ".join(edge_pointers) + ");",
                ]
            )
        source_lines.append(
            f"        const gui_forms::OwnerDecorationRecipe decorations[{len(recipes)}U] = {{"
        )
        for recipe_index in range(len(recipes)):
            source_lines.append(f"            decoration_{recipe_index},")
        source_lines.extend(
            [
                "        };",
                "        control.set_owned_decorations("
                "decorations, sizeof(decorations) / sizeof(decorations[0]));",
                "        return;", "    }",
            ]
        )
    source_lines.extend(
        ["    default:",
         "        throw std::invalid_argument(\"node has no bounded native owner decoration\");",
         "    }", "}", "", f"}}  // namespace {namespace}", ""]
    )
    header_text = "\n".join(header_lines)
    source_text = "\n".join(source_lines)
    validate_generated_cpp(header_text, header_path)
    validate_generated_cpp(source_text, source_path)
    header_path.write_text(header_text, encoding="utf-8")
    source_path.write_text(source_text, encoding="utf-8")
    report = {
        "schema": "web.forms.gui-decoration-projection/0.1-experimental",
        "target": manifest.get("target", "unknown"),
        "source_digest": ir["source"]["digest"],
        "status": "bounded-native-owner-decoration-projection",
        "exact_decoration_count": sum(len(recipes) for recipes in grouped.values()),
        "unavailable_decoration_count": len(records) - sum(
            len(recipes) for recipes in grouped.values()
        ),
        "owners": len(grouped),
        "decorations": records,
    }
    report_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return header_path, source_path, report_path
