from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from .diagnostics import Diagnostic, WebFormsError
from .gui_decoration_stage2 import generate_gui_decorations
from .gui_layout_stage2 import generate_gui_layouts
from .gui_material_stage2 import generate_gui_materials
from .gui_typography_stage2 import generate_gui_typography
from .profile import IR_SCHEMA
from .stage2 import _cpp_string, _symbol, validate_generated_cpp


def _diagnostic(code: str, message: str) -> WebFormsError:
    return WebFormsError([Diagnostic(code, message, "<ir>")])


def _properties(record: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {item["name"]: item for item in record["properties"]}


def _runtime_parent(
    node: dict[str, Any], nodes: list[dict[str, Any]], runtime: set[int]
) -> int | None:
    parent = node["parent"]
    while parent is not None and parent not in runtime:
        parent = nodes[parent]["parent"]
    return parent


def _read_report(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def _narrow_cpp_string(value: str) -> str:
    # GUI.Forms public text and StableId APIs own UTF-8 bytes in std::string.
    # An ordinary UTF-8 source literal remains compatible with both C++17 and
    # C++20; a u8 literal changes to char8_t in C++20 and ceases to bind.
    return _cpp_string(value)[2:]


def _unavailable_diagnostics(report: dict[str, Any], collection: str) -> list[str]:
    result: list[str] = []
    for record in report.get(collection, []):
        if record.get("status") == "unavailable":
            result.extend(str(item) for item in record.get("diagnostics", []))
    return result


def _cpp_type(
    node: dict[str, Any], geometry: dict[str, dict[str, Any]], has_children: bool
) -> str:
    if node["control"] == "button":
        return "gui_forms::Button"
    display = geometry.get("display", {}).get("value")
    if has_children and display == "grid":
        return "gui_forms::TableLayoutPanel"
    if has_children:
        return "gui_forms::FlowLayoutPanel"
    return "gui_forms::Label"


def generate_gui_tree(
    ir: dict[str, Any], manifest: dict[str, Any], output_dir: Path,
    unit_name: str | None = None,
) -> tuple[Path, Path, Path]:
    """Emit one fail-closed, typed retained GUI.Forms tree.

    The projection is intentionally a dogfood-stage construction adapter. It
    composes only records already accepted by the four bounded native lowering
    passes; it never substitutes a theme default for a refused CSS record.
    """
    if ir.get("schema") != IR_SCHEMA:
        raise _diagnostic("WFGT001", f"native tree Stage 2 requires {IR_SCHEMA!r}")
    construction = manifest.get("features", {}).get(
        "construction.generated-native-tree", {}
    )
    if construction.get("status") != "supported":
        raise _diagnostic(
            "WFGT002", "target manifest does not admit generated native trees"
        )

    nodes = ir["document"]["nodes"]
    runtime_nodes = [node for node in nodes if node["runtime"]]
    if not runtime_nodes:
        raise _diagnostic("WFGT003", "native tree requires at least one runtime node")
    if any(not node["id"] for node in runtime_nodes):
        raise _diagnostic(
            "WFGT004", "every retained native-tree node requires an explicit stable id"
        )
    runtime_indexes = {node["index"] for node in runtime_nodes}
    roots = [
        node for node in runtime_nodes
        if _runtime_parent(node, nodes, runtime_indexes) is None
    ]
    if len(roots) != 1:
        raise _diagnostic("WFGT005", "native tree requires exactly one runtime root")

    symbols: dict[str, str] = {}
    for node in runtime_nodes:
        symbol = _symbol(node["id"])
        previous = symbols.get(symbol)
        if previous is not None and previous != node["id"]:
            raise _diagnostic(
                "WFGT006",
                f"generated member collision between {previous!r} and {node['id']!r}",
            )
        symbols[symbol] = node["id"]

    root_id = roots[0]["id"]
    unit = _symbol(unit_name or root_id)
    namespace = f"web_forms_generated_{unit}"
    output_dir.mkdir(parents=True, exist_ok=True)

    _, _, material_report_path = generate_gui_materials(
        ir, manifest, output_dir, unit_name
    )
    _, _, layout_report_path = generate_gui_layouts(
        ir, manifest, output_dir, unit_name
    )
    _, _, typography_report_path = generate_gui_typography(
        ir, manifest, output_dir, unit_name
    )
    _, _, decoration_report_path = generate_gui_decorations(
        ir, manifest, output_dir, unit_name
    )
    material_report = _read_report(material_report_path)
    layout_report = _read_report(layout_report_path)
    typography_report = _read_report(typography_report_path)
    decoration_report = _read_report(decoration_report_path)

    refused: list[str] = []
    refused.extend(_unavailable_diagnostics(material_report, "styles"))
    refused.extend(_unavailable_diagnostics(material_report, "buttons"))
    refused.extend(_unavailable_diagnostics(layout_report, "flows"))
    refused.extend(_unavailable_diagnostics(layout_report, "grids"))
    refused.extend(_unavailable_diagnostics(layout_report, "flex_grows"))
    refused.extend(_unavailable_diagnostics(layout_report, "boxes"))
    refused.extend(_unavailable_diagnostics(typography_report, "nodes"))
    refused.extend(_unavailable_diagnostics(decoration_report, "decorations"))
    if refused:
        raise _diagnostic(
            "WFGT007", "native tree is fail-closed: " + " | ".join(refused)
        )

    exact_materials = {
        int(item["style"])
        for item in material_report["styles"]
        if item["status"] == "exact-surface-projection"
    }
    exact_buttons = {
        int(item["node"])
        for item in material_report["buttons"]
        if item["status"] == "bounded-native-state-projection"
    }
    exact_flows = {
        int(item["node"])
        for item in layout_report["flows"]
        if item["status"] == "bounded-native-flow-projection"
    }
    exact_grids = {
        int(item["node"])
        for item in layout_report["grids"]
        if item["status"] == "bounded-native-grid-projection"
    }
    exact_boxes = {
        int(item["node"])
        for item in layout_report["boxes"]
        if item["status"] == "bounded-native-box-projection"
    }
    flex_grows = {
        int(item["node"]): float(item["grow"])
        for item in layout_report["flex_grows"]
        if item["status"] == "bounded-native-flex-grow"
    }
    exact_typography = {
        int(item["node"])
        for item in typography_report["nodes"]
        if item["status"] == "bounded-native-typography-projection"
    }
    decoration_owners = {
        int(item["owner"])
        for item in decoration_report["decorations"]
        if item["status"] == "bounded-native-owner-decoration"
    }

    style_by_node = {item["node"]: item for item in ir["styles"]["nodes"]}
    geometry_pool = ir["styles"]["pools"]["geometry"]
    runtime_children: dict[int, list[dict[str, Any]]] = {
        node["index"]: [] for node in runtime_nodes
    }
    for node in runtime_nodes:
        parent = _runtime_parent(node, nodes, runtime_indexes)
        if parent is not None:
            runtime_children[parent].append(node)

    types: dict[int, str] = {}
    geometry_by_node: dict[int, dict[str, dict[str, Any]]] = {}
    for node in runtime_nodes:
        style = style_by_node[node["index"]]
        geometry = _properties(geometry_pool[style["geometry"]])
        geometry_by_node[node["index"]] = geometry
        types[node["index"]] = _cpp_type(
            node, geometry, bool(runtime_children[node["index"]])
        )
        display = geometry.get("display", {}).get("value")
        if display == "flex" and node["index"] not in exact_flows:
            raise _diagnostic(
                "WFGT008", f"node {node['id']!r} has no exact flex projection"
            )
        if display == "grid" and node["index"] not in exact_grids:
            raise _diagnostic(
                "WFGT009", f"node {node['id']!r} has no exact grid projection"
            )
        if node["control"] == "button" and node["index"] not in exact_buttons:
            raise _diagnostic(
                "WFGT010", f"button {node['id']!r} has no exact state projection"
            )
        if types[node["index"]] in {"gui_forms::Button", "gui_forms::Label"}:
            if node["index"] not in exact_typography:
                raise _diagnostic(
                    "WFGT011", f"node {node['id']!r} has no exact typography"
                )
        if style["surface"] in {"own-surface", "baked-into-parent"}:
            if style["material"] not in exact_materials:
                raise _diagnostic(
                    "WFGT012", f"node {node['id']!r} has no exact surface material"
                )

    header_path = output_dir / f"{unit}.gui_tree.wf.hpp"
    source_path = output_dir / f"{unit}.gui_tree.wf.cpp"
    report_path = output_dir / f"{unit}.gui_tree.json"
    header_lines = [
        "#pragma once", "", "#include <memory>",
        '#include "gui_forms/controls/button_base/button/button.hpp"',
        '#include "gui_forms/controls/label/label.hpp"',
        '#include "gui_forms/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.hpp"',
        '#include "gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp"',
        "", f"namespace {namespace} {{", "", "struct NativeForm final {",
    ]
    for node in runtime_nodes:
        header_lines.append(
            f"    std::shared_ptr<{types[node['index']]}> {_symbol(node['id'])};"
        )
    header_lines.extend(
        [
            "", "    gui_forms::Control::Ptr root_control() const noexcept;",
            "};", "", "NativeForm make_native_form();", "",
            f"}}  // namespace {namespace}", "",
        ]
    )

    source_lines = [
        f'#include "{header_path.name}"',
        f'#include "{unit}.gui_decorations.wf.hpp"',
        f'#include "{unit}.gui_layouts.wf.hpp"',
        f'#include "{unit}.gui_materials.wf.hpp"',
        f'#include "{unit}.gui_typography.wf.hpp"',
        "", "#include <utility>", "", f"namespace {namespace} {{", "",
        "gui_forms::Control::Ptr NativeForm::root_control() const noexcept {",
        f"    return {_symbol(root_id)};", "}", "", "NativeForm make_native_form() {",
        "    NativeForm form;",
    ]
    for node in runtime_nodes:
        node_type = types[node["index"]]
        constructor = f"gui_forms::StableId({_narrow_cpp_string(node['id'])})"
        if node_type in {"gui_forms::Button", "gui_forms::Label"}:
            constructor += f", {_narrow_cpp_string(node['text'])}"
        source_lines.extend(
            [
                f"    form.{_symbol(node['id'])} = "
                f"std::make_shared<{node_type}>({constructor});",
                f"    form.{_symbol(node['id'])}->set_name({_narrow_cpp_string(node['id'])});",
                f"    form.{_symbol(node['id'])}->begin_init();",
            ]
        )
    source_lines.append("")

    for node in runtime_nodes:
        index = node["index"]
        symbol = _symbol(node["id"])
        node_type = types[index]
        style = style_by_node[index]
        geometry = geometry_by_node[index]
        display = geometry.get("display", {}).get("value")
        # CSS initial geometry is part of the language, not a request to keep
        # GUI.Forms' Forms-compatible default control margin.
        source_lines.append(f"    form.{symbol}->set_margin({{}});")
        source_lines.append(f"    form.{symbol}->set_padding({{}});")
        if index in exact_boxes:
            source_lines.append(
                f"    apply_native_box_geometry({index}U, *form.{symbol});"
            )
        if node_type == "gui_forms::FlowLayoutPanel":
            if display == "flex":
                source_lines.append(
                    f"    apply_native_flow_layout({index}U, *form.{symbol});"
                )
            else:
                centered = any(
                    child["index"] in exact_boxes
                    and next(
                        (
                            item.get("horizontal_auto_margin", False)
                            for item in layout_report["boxes"]
                            if int(item["node"]) == child["index"]
                            and item["status"] == "bounded-native-box-projection"
                        ),
                        False,
                    )
                    for child in runtime_children[index]
                )
                source_lines.extend(
                    [
                        f"    form.{symbol}->set_flow_direction("
                        "gui_forms::FlowDirection::top_down);",
                        f"    form.{symbol}->set_wrap_contents(false);",
                        f"    form.{symbol}->set_main_alignment("
                        "gui_forms::FlowMainAlignment::start);",
                        f"    form.{symbol}->set_cross_alignment("
                        + (
                            "gui_forms::FlowCrossAlignment::center);"
                            if centered
                            else "gui_forms::FlowCrossAlignment::stretch);"
                        ),
                        f"    form.{symbol}->set_item_spacing({{0.0, 0.0}});",
                    ]
                )
            if node != roots[0]:
                source_lines.extend(
                    [
                        f"    form.{symbol}->set_auto_size(true);",
                        f"    form.{symbol}->set_auto_size_mode("
                        "gui_forms::AutoSizeMode::grow_only);",
                    ]
                )
        elif node_type == "gui_forms::TableLayoutPanel":
            source_lines.append(
                f"    apply_native_grid_layout({index}U, *form.{symbol});"
            )
            source_lines.extend(
                [
                    f"    form.{symbol}->set_auto_size(true);",
                    f"    form.{symbol}->set_auto_size_mode("
                    "gui_forms::AutoSizeMode::grow_only);",
                ]
            )
        if node_type == "gui_forms::Button":
            source_lines.extend(
                [
                    f"    form.{symbol}->set_visual_recipes("
                    f"make_native_button_state_recipes({index}U));",
                    f"    apply_native_button_typography({index}U, *form.{symbol});",
                ]
            )
        elif node_type == "gui_forms::Label":
            source_lines.append(
                f"    apply_native_label_typography({index}U, *form.{symbol});"
            )
            if style["surface"] in {"own-surface", "baked-into-parent"}:
                source_lines.append(
                    f"    form.{symbol}->set_authored_surface_material("
                    f"make_native_surface_material({style['material']}U));"
                )
        elif style["surface"] in {"own-surface", "baked-into-parent"}:
            source_lines.append(
                f"    form.{symbol}->set_authored_surface_material("
                f"make_native_surface_material({style['material']}U));"
            )
        if index in decoration_owners:
            source_lines.append(
                f"    apply_native_owner_decorations({index}U, *form.{symbol});"
            )
        source_lines.append("")

    for node in runtime_nodes:
        parent_index = _runtime_parent(node, nodes, runtime_indexes)
        if parent_index is None:
            continue
        parent = nodes[parent_index]
        parent_symbol = _symbol(parent["id"])
        child_symbol = _symbol(node["id"])
        source_lines.append(
            f"    form.{parent_symbol}->add_child(form.{child_symbol});"
        )
        if types[parent_index] == "gui_forms::FlowLayoutPanel":
            grow = flex_grows.get(node["index"], 0.0)
            if grow > 0.0:
                source_lines.append(
                    f"    form.{parent_symbol}->set_flex_grow("
                    f"*form.{child_symbol}, {repr(grow)});"
                )
        elif types[parent_index] == "gui_forms::TableLayoutPanel":
            source_lines.append(
                f"    apply_native_grid_cell({parent_index}U, {node['index']}U, "
                f"*form.{parent_symbol}, *form.{child_symbol});"
            )
    source_lines.append("")
    for node in reversed(runtime_nodes):
        source_lines.append(f"    form.{_symbol(node['id'])}->end_init();")
    source_lines.extend(
        ["    return form;", "}", "", f"}}  // namespace {namespace}", ""]
    )

    header_text = "\n".join(header_lines)
    source_text = "\n".join(source_lines)
    validate_generated_cpp(header_text, header_path)
    validate_generated_cpp(source_text, source_path)
    header_path.write_text(header_text, encoding="utf-8")
    source_path.write_text(source_text, encoding="utf-8")
    report = {
        "schema": "web.forms.gui-tree-projection/0.1-experimental",
        "target": manifest.get("target", "unknown"),
        "source_digest": ir["source"]["digest"],
        "status": "complete-bounded-native-tree-projection",
        "root_id": root_id,
        "node_count": len(runtime_nodes),
        "typed_member_count": len(runtime_nodes),
        "button_count": sum(
            1 for node in runtime_nodes if types[node["index"]] == "gui_forms::Button"
        ),
        "label_count": sum(
            1 for node in runtime_nodes if types[node["index"]] == "gui_forms::Label"
        ),
        "flow_count": sum(
            1
            for node in runtime_nodes
            if types[node["index"]] == "gui_forms::FlowLayoutPanel"
        ),
        "grid_count": sum(
            1
            for node in runtime_nodes
            if types[node["index"]] == "gui_forms::TableLayoutPanel"
        ),
        "commands": [
            {
                "id": node["id"],
                "command": node["attributes"].get("data-wf-command", ""),
                "member": _symbol(node["id"]),
            }
            for node in runtime_nodes
            if node["control"] == "button"
        ],
        "implicit_block_flow_nodes": [
            node["id"]
            for node in runtime_nodes
            if types[node["index"]] == "gui_forms::FlowLayoutPanel"
            and geometry_by_node[node["index"]].get("display", {}).get("value")
            not in {"flex", "grid"}
        ],
        "component_reports": {
            "materials": material_report_path.name,
            "layouts": layout_report_path.name,
            "typography": typography_report_path.name,
            "decorations": decoration_report_path.name,
        },
    }
    report_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return header_path, source_path, report_path
