from __future__ import annotations

import base64
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


def _forms_text(value: str) -> str:
    # GUI.Forms follows Forms mnemonic spelling. HTML character data is
    # literal, so double ampersands unless a future explicit access-key
    # contract says otherwise.
    return value.replace("&", "&&")


def _unavailable_diagnostics(report: dict[str, Any], collection: str) -> list[str]:
    result: list[str] = []
    for record in report.get(collection, []):
        if record.get("status") == "unavailable":
            result.extend(str(item) for item in record.get("diagnostics", []))
    return result


def _cpp_type(
    node: dict[str, Any], geometry: dict[str, dict[str, Any]], has_children: bool
) -> str:
    authored = node.get("attributes", {}).get("data-wf-control")
    if authored == "dropdown-button":
        return "gui_forms::DropDownButton"
    if authored == "command-overflow":
        return "gui_forms::CommandOverflowPanel"
    if authored == "responsive-tracks":
        return "gui_forms::ResponsiveTrackPanel"
    if authored == "split-view" and "data-wf-split-orientation" in node.get(
        "attributes", {}
    ):
        return "gui_forms::SplitContainer"
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
    box_report_by_node = {
        int(item["node"]): item
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
        if types[node["index"]] in {
            "gui_forms::Button", "gui_forms::DropDownButton"
        } and node["index"] not in exact_buttons:
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
    drag_region_ids = [
        node["id"] for node in runtime_nodes
        if node.get("attributes", {}).get("data-wf-window-drag-region") == "true"
    ]
    resource_records = ir.get("resources", {}).get("local_png", [])
    if resource_records and manifest.get("features", {}).get(
        "resource.local-png", {}
    ).get("status") != "supported":
        raise _diagnostic(
            "WFGT021", "target manifest does not admit generated local PNG resources"
        )
    resource_lists: dict[str, list[dict[str, Any]]] = {}
    for record in resource_records:
        resource_lists.setdefault(record["list"], []).append(record)
    header_lines = [
        "#pragma once", "", "#include <array>", "#include <memory>",
        "#include <string_view>",
        '#include "gui_forms/controls/button_base/button/button.hpp"',
        '#include "gui_forms/controls/button_base/button/drop_down_button/drop_down_button.hpp"',
        '#include "gui_forms/controls/label/label.hpp"',
        '#include "gui_forms/controls/scrollable_control/container_control/command_overflow_panel/command_overflow_panel.hpp"',
        '#include "gui_forms/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.hpp"',
        '#include "gui_forms/controls/scrollable_control/container_control/responsive_track_panel/responsive_track_panel.hpp"',
        '#include "gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp"',
        '#include "gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp"',
        '#include "gui_forms/image_list.hpp"',
        '#include "gui_forms/window.hpp"',
        "", f"namespace {namespace} {{", "", "struct NativeForm final {",
        f"    inline static constexpr std::array<std::string_view, {len(drag_region_ids)}U> "
        "window_drag_region_ids{{"
        + ", ".join(_narrow_cpp_string(item) for item in drag_region_ids)
        + "}};",
    ]
    for node in runtime_nodes:
        header_lines.append(
            f"    std::shared_ptr<{types[node['index']]}> {_symbol(node['id'])};"
        )
    for list_name in sorted(resource_lists):
        header_lines.append(
            f"    std::shared_ptr<gui_forms::ImageList> resource_{_symbol(list_name)};"
        )
    header_lines.extend(
        [
            "", "    gui_forms::Control::Ptr root_control() const noexcept;",
            "};", "", "NativeForm make_native_form();",
            "void bind_native_resources(NativeForm& form, gui_forms::Window& window);", "",
            f"}}  // namespace {namespace}", "",
        ]
    )

    source_lines = [
        f'#include "{header_path.name}"',
        f'#include "{unit}.gui_decorations.wf.hpp"',
        f'#include "{unit}.gui_layouts.wf.hpp"',
        f'#include "{unit}.gui_materials.wf.hpp"',
        f'#include "{unit}.gui_typography.wf.hpp"',
        '#include "gui_forms/connected_controls.hpp"',
        "", "#include <chrono>", "#include <cstddef>", "#include <cstdint>",
        "#include <stdexcept>", "#include <utility>", "", f"namespace {namespace} {{", "",
    ]
    if resource_records:
        source_lines.extend(["namespace {", ""])
        for record_index, record in enumerate(resource_records):
            encoded = base64.b64decode(record["encoded_base64"], validate=True)
            source_lines.append(
                f"static const std::byte local_png_{record_index}[] = {{"
            )
            for offset in range(0, len(encoded), 12):
                source_lines.append(
                    "    " + ", ".join(
                        f"std::byte{{0x{value:02x}}}"
                        for value in encoded[offset:offset + 12]
                    ) + ","
                )
            source_lines.extend(["};", ""])
        source_lines.extend(["}  // namespace", ""])
    source_lines.extend([
        "gui_forms::Control::Ptr NativeForm::root_control() const noexcept {",
        f"    return {_symbol(root_id)};", "}", "", "NativeForm make_native_form() {",
        "    NativeForm form;",
    ])
    for node in runtime_nodes:
        node_type = types[node["index"]]
        constructor = f"gui_forms::StableId({_narrow_cpp_string(node['id'])})"
        if node_type == "gui_forms::Button":
            constructor += f", {_narrow_cpp_string(_forms_text(node['text']))}"
        elif node_type == "gui_forms::Label":
            constructor += f", {_narrow_cpp_string(node['text'])}"
        elif node_type == "gui_forms::DropDownButton":
            mode = node.get("attributes", {}).get("data-wf-dropdown-mode", "menu")
            constructor += (
                f", {_narrow_cpp_string(_forms_text(node['text']))}, "
                f"gui_forms::DropDownButtonMode::{mode}"
            )
        construction = (
            f"gui_forms::make_control<{node_type}>({constructor})"
            if node_type == "gui_forms::SplitContainer"
            else f"std::make_shared<{node_type}>({constructor})"
        )
        source_lines.extend(
            [
                f"    form.{_symbol(node['id'])} = "
                f"{construction};",
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
        if display == "none" and node.get("attributes", {}).get(
            "data-wf-overflow-actuator"
        ) != "true":
            source_lines.append(f"    form.{symbol}->set_visible(false);")
        # CSS initial geometry is part of the language, not a request to keep
        # GUI.Forms' Forms-compatible default control margin.
        source_lines.append(f"    form.{symbol}->set_margin({{}});")
        source_lines.append(f"    form.{symbol}->set_padding({{}});")
        if node_type == "gui_forms::Label":
            source_lines.append(f"    form.{symbol}->set_use_mnemonic(false);")
        if index in exact_boxes:
            source_lines.append(
                f"    apply_native_box_geometry({index}U, *form.{symbol});"
            )
        if node_type == "gui_forms::CommandOverflowPanel":
            gap = float(node.get("attributes", {}).get("data-wf-overflow-gap", "0"))
            source_lines.append(
                f"    form.{symbol}->set_group_gap({repr(gap)});"
            )
        elif node_type == "gui_forms::ResponsiveTrackPanel":
            attributes = node.get("attributes", {})
            orientation = attributes.get(
                "data-wf-responsive-orientation", "horizontal"
            )
            gap = float(attributes.get("data-wf-responsive-gap", "0"))
            direct_children = runtime_children[index]
            by_track: dict[int, dict[str, str]] = {}
            child_tracks: set[int] = set()
            for child in direct_children:
                child_attributes = child.get("attributes", {})
                raw_track = child_attributes.get("data-wf-track-index")
                if raw_track is None:
                    raise _diagnostic(
                        "WFGT013",
                        f"responsive child {child['id']!r} requires data-wf-track-index",
                    )
                track = int(raw_track)
                child_tracks.add(track)
                if "data-wf-track-mode" in child_attributes:
                    if track in by_track:
                        raise _diagnostic(
                            "WFGT014",
                            f"responsive track {track} has more than one specification",
                        )
                    by_track[track] = child_attributes
            if not by_track or set(by_track) != set(range(max(by_track) + 1)):
                raise _diagnostic(
                    "WFGT015",
                    f"responsive panel {node['id']!r} requires one contiguous specification per track",
                )
            if not child_tracks.issubset(by_track):
                raise _diagnostic(
                    "WFGT016",
                    f"responsive panel {node['id']!r} assigns a child to an unspecified track",
                )
            priorities = [
                int(item["data-wf-track-collapse-priority"])
                for item in by_track.values()
                if "data-wf-track-collapse-priority" in item
            ]
            if len(priorities) != len(set(priorities)):
                raise _diagnostic(
                    "WFGT017",
                    f"responsive panel {node['id']!r} requires unique collapse priorities",
                )
            source_lines.append(
                f"    form.{symbol}->set_orientation("
                f"gui_forms::ResponsiveTrackOrientation::{orientation});"
            )
            source_lines.append(
                f"    form.{symbol}->set_track_gap({repr(gap)});"
            )
            source_lines.append(f"    form.{symbol}->set_track_specs({{")
            for track in range(len(by_track)):
                spec = by_track[track]
                mode = spec["data-wf-track-mode"]
                minimum = float(spec.get("data-wf-track-minimum", "0"))
                preferred = float(spec.get("data-wf-track-preferred", minimum))
                maximum = float(spec.get("data-wf-track-maximum", "0"))
                weight = float(spec.get("data-wf-track-weight", "1"))
                if preferred < minimum or (
                    maximum > 0.0 and maximum < preferred
                ) or (mode == "remaining" and not 0.0 < weight <= 1024.0):
                    raise _diagnostic(
                        "WFGT018",
                        f"responsive track {track} has invalid ordered bounds or weight",
                    )
                priority = spec.get("data-wf-track-collapse-priority")
                priority_cpp = (
                    f"std::uint16_t{{{int(priority)}U}}"
                    if priority is not None else "std::nullopt"
                )
                source_lines.append(
                    "        {gui_forms::ResponsiveTrackSizeMode::"
                    f"{mode}, {repr(minimum)}, {repr(preferred)}, "
                    f"{repr(maximum)}, {repr(weight)}, false, {priority_cpp}}},"
                )
            source_lines.append("    });")
        elif node_type == "gui_forms::SplitContainer":
            attributes = node.get("attributes", {})
            direct_children = runtime_children[index]
            panels = [
                child.get("attributes", {}).get("data-wf-split-panel")
                for child in direct_children
            ]
            if sorted(panels) != ["first", "second"]:
                raise _diagnostic(
                    "WFGT019",
                    f"split panel {node['id']!r} requires exactly one first and one second child",
                )
            orientation = attributes["data-wf-split-orientation"]
            distance = float(attributes.get("data-wf-split-distance", "0"))
            visible = float(
                attributes.get("data-wf-split-visible-thickness", "3")
            )
            hit_before = float(attributes.get("data-wf-split-hit-before", "3"))
            hit_after = float(attributes.get("data-wf-split-hit-after", "3"))
            minimum_hit = float(
                attributes.get(
                    "data-wf-split-minimum-hit-target",
                    str(hit_before + visible + hit_after),
                )
            )
            first_minimum = float(
                attributes.get("data-wf-split-first-minimum", "0")
            )
            second_minimum = float(
                attributes.get("data-wf-split-second-minimum", "0")
            )
            transition = float(attributes.get("data-wf-split-transition-ms", "0"))
            source_lines.extend(
                [
                    f"    form.{symbol}->set_orientation("
                    f"gui_forms::Orientation::{orientation});",
                    f"    form.{symbol}->set_splitter_geometry("
                    f"{{{repr(visible)}, "
                    "gui_forms::SplitSeamThicknessPolicy::logical, "
                    f"{repr(hit_before)}, {repr(hit_after)}, "
                    f"{repr(minimum_hit)}}});",
                    f"    form.{symbol}->set_first_minimum({repr(first_minimum)});",
                    f"    form.{symbol}->set_second_minimum({repr(second_minimum)});",
                ]
            )
            if distance > 0.0:
                source_lines.append(
                    f"    form.{symbol}->set_splitter_distance({repr(distance)});"
                )
            for side in ("first", "second"):
                raw = attributes.get(f"data-wf-split-{side}-maximum")
                if raw is not None and float(raw) > 0.0:
                    source_lines.append(
                        f"    form.{symbol}->set_{side}_maximum({repr(float(raw))});"
                    )
            fixed = attributes.get("data-wf-split-fixed-panel", "none")
            collapse = attributes.get("data-wf-split-collapse-panel", "none")
            threshold = float(
                attributes.get(
                    "data-wf-split-automatic-collapse-threshold", "0"
                )
            )
            source_lines.extend(
                [
                    f"    form.{symbol}->set_fixed_panel("
                    f"gui_forms::SplitFixedPanel::{fixed});",
                    f"    form.{symbol}->set_collapse_panel("
                    f"gui_forms::SplitFixedPanel::{collapse});",
                    f"    form.{symbol}->set_automatic_collapse_threshold("
                    f"{repr(threshold)});",
                    f"    form.{symbol}->set_splitter_transition_duration("
                    "std::chrono::duration_cast<gui_forms::FrameInterval>("
                    "std::chrono::duration<double, std::milli>("
                    f"{repr(transition)})));",
                ]
            )
        elif node_type == "gui_forms::FlowLayoutPanel":
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
                            and not item.get("fill_width", False)
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
        if node_type in {"gui_forms::Button", "gui_forms::DropDownButton"}:
            source_lines.extend(
                [
                    "    {",
                    "        gui_forms::ControlStateRecipes recipes =",
                    f"            make_native_button_state_recipes({index}U);",
                    f"        apply_native_recipe_keylines({index}U, recipes);",
                    f"        form.{symbol}->set_visual_recipes(std::move(recipes));",
                    "    }",
                    f"    apply_native_button_typography({index}U, *form.{symbol});",
                ]
            )
            attributes = node.get("attributes", {})
            if (node_type == "gui_forms::DropDownButton" and
                    "data-wf-dropdown-width" in attributes):
                drop_down_width = float(attributes["data-wf-dropdown-width"])
                if not 1.0 <= drop_down_width <= 64.0:
                    raise _diagnostic(
                        "WFGT024",
                        f"drop-down width for {node['id']!r} must be in [1,64]",
                    )
                source_lines.append(
                    f"    form.{symbol}->set_drop_down_width({repr(drop_down_width)});"
                )
            image_key = attributes.get("data-wf-image-key")
            if image_key is not None:
                relation = attributes.get("data-wf-image-relation", "overlay")
                image_alignment = attributes.get(
                    "data-wf-image-alignment", "middle-center"
                )
                text_alignment = attributes.get(
                    "data-wf-text-alignment", "middle-center"
                )
                gap = float(attributes.get("data-wf-image-gap", "0"))
                padding = [
                    float(attributes.get(f"data-wf-content-padding-{edge}", "0"))
                    for edge in ("left", "top", "right", "bottom")
                ]
                source_lines.extend(
                    [
                        f"    form.{symbol}->set_image_key("
                        f"{_narrow_cpp_string(image_key)});",
                        f"    form.{symbol}->set_text_image_relation("
                        "gui_forms::TextImageRelation::"
                        f"{relation.replace('-', '_')});",
                        f"    form.{symbol}->set_image_gap({repr(gap)});",
                        f"    form.{symbol}->set_image_alignment("
                        "gui_forms::ContentAlignment::"
                        f"{image_alignment.replace('-', '_')});",
                        f"    form.{symbol}->set_text_alignment("
                        "gui_forms::ContentAlignment::"
                        f"{text_alignment.replace('-', '_')});",
                        f"    form.{symbol}->set_content_padding("
                        f"{{{', '.join(repr(value) for value in padding)}}});",
                    ]
                )
        elif node_type == "gui_forms::Label":
            source_lines.append(
                f"    apply_native_label_typography({index}U, *form.{symbol});"
            )
            if style["surface"] in {"own-surface", "baked-into-parent"}:
                source_lines.extend(
                    [
                        "    {",
                        "        gui_forms::SurfaceMaterial material =",
                        f"            make_native_surface_material({style['material']}U);",
                        f"        apply_native_material_keylines({index}U, material);",
                        f"        form.{symbol}->set_authored_surface_material(std::move(material));",
                        "    }",
                    ]
                )
        elif style["surface"] in {"own-surface", "baked-into-parent"}:
            source_lines.extend(
                [
                    "    {",
                    "        gui_forms::SurfaceMaterial material =",
                    f"            make_native_surface_material({style['material']}U);",
                    f"        apply_native_material_keylines({index}U, material);",
                    f"        form.{symbol}->set_authored_surface_material(std::move(material));",
                    "    }",
                ]
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
        parent_type = types[parent_index]
        if parent_type == "gui_forms::SplitContainer":
            panel = node.get("attributes", {}).get("data-wf-split-panel")
            if panel not in {"first", "second"}:
                raise _diagnostic(
                    "WFGT020",
                    f"split child {node['id']!r} requires first/second panel metadata",
                )
            source_lines.extend(
                [
                    f"    form.{child_symbol}->set_dock(gui_forms::DockStyle::fill);",
                    f"    form.{parent_symbol}->{panel}_panel()->add_child("
                    f"form.{child_symbol});",
                ]
            )
        else:
            source_lines.append(
                f"    form.{parent_symbol}->add_child(form.{child_symbol});"
            )
        if parent_type == "gui_forms::FlowLayoutPanel":
            grow = flex_grows.get(node["index"], 0.0)
            parent_geometry = geometry_by_node[parent_index]
            parent_direction = parent_geometry.get(
                "flex-direction", {}
            ).get("value", "column" if parent_geometry.get("display", {}).get("value") != "flex" else "row")
            parent_vertical = parent_direction in {"column", "column-reverse"}
            child_box = box_report_by_node.get(node["index"], {})
            fills_main_axis = (
                child_box.get("fill_height", False)
                if parent_vertical
                else child_box.get("fill_width", False)
            )
            if grow <= 0.0 and fills_main_axis:
                grow = 1.0
            if grow > 0.0:
                source_lines.append(
                    f"    form.{parent_symbol}->set_flex_grow("
                    f"*form.{child_symbol}, {repr(grow)});"
                )
        elif parent_type == "gui_forms::TableLayoutPanel":
            source_lines.append(
                f"    apply_native_grid_cell({parent_index}U, {node['index']}U, "
                f"*form.{parent_symbol}, *form.{child_symbol});"
            )
        elif parent_type == "gui_forms::CommandOverflowPanel":
            attributes = node.get("attributes", {})
            if attributes.get("data-wf-overflow-actuator") == "true":
                source_lines.append(
                    f"    form.{parent_symbol}->set_overflow_actuator(*form.{child_symbol});"
                )
            elif "data-wf-overflow-minimum" in attributes:
                minimum = float(attributes["data-wf-overflow-minimum"])
                preferred = float(attributes.get("data-wf-overflow-preferred", minimum))
                maximum = float(attributes.get("data-wf-overflow-maximum", 0.0))
                priority = int(float(attributes["data-wf-overflow-priority"]))
                source_lines.append(
                    f"    form.{parent_symbol}->set_group_spec(*form.{child_symbol}, "
                    f"{{{repr(minimum)}, {repr(preferred)}, {repr(maximum)}, "
                    f"std::uint16_t{{{priority}U}}}});"
                )
        elif parent_type == "gui_forms::ResponsiveTrackPanel":
            track = int(node.get("attributes", {})["data-wf-track-index"])
            source_lines.append(
                f"    form.{child_symbol}->set_dock(gui_forms::DockStyle::fill);"
            )
            source_lines.append(
                f"    form.{parent_symbol}->set_child_track("
                f"*form.{child_symbol}, {track}U);"
            )
    source_lines.append("")
    connected_group_count = 0
    for node in runtime_nodes:
        axis = node.get("attributes", {}).get("data-wf-connected-axis")
        if axis is None:
            continue
        children = runtime_children[node["index"]]
        if len(children) < 2 or any(
            types[child["index"]] not in {
                "gui_forms::Button", "gui_forms::DropDownButton"
            }
            for child in children
        ):
            raise _diagnostic(
                "WFGT022", f"connected group {node['id']!r} has invalid members"
            )
        group_symbol = f"connected_{_symbol(node['id'])}"
        source_lines.append(
            f"    const std::shared_ptr<gui_forms::ButtonBase> {group_symbol}[] = {{"
        )
        for child in children:
            source_lines.append(f"        form.{_symbol(child['id'])},")
        source_lines.extend(
            [
                "    };",
                f"    gui_forms::connect_button_group({group_symbol}, "
                f"gui_forms::ConnectedControlAxis::{axis});",
                "",
            ]
        )
        connected_group_count += 1
    for node in reversed(runtime_nodes):
        source_lines.append(f"    form.{_symbol(node['id'])}->end_init();")
    source_lines.extend(["    return form;", "}", ""])
    source_lines.extend(
        [
            "void bind_native_resources(NativeForm& form, gui_forms::Window& window) {",
        ]
    )
    if not resource_lists:
        source_lines.extend(["    (void)form;", "    (void)window;"])
    else:
        for list_name, entries in sorted(resource_lists.items()):
            list_symbol = _symbol(list_name)
            logical_size = entries[0]["logical_size"]
            source_lines.extend(
                [
                    f"    if (form.resource_{list_symbol}) {{",
                    f"        throw std::logic_error(\"native image list {list_name} is already bound\");",
                    "    }",
                    f"    std::shared_ptr<gui_forms::ImageList> resource_{list_symbol} =",
                    "        std::make_shared<gui_forms::ImageList>(",
                    f"        window, gui_forms::Size{{{repr(float(logical_size[0]))}, {repr(float(logical_size[1]))}}});",
                ]
            )
            by_key: dict[str, list[tuple[int, dict[str, Any]]]] = {}
            for record_index, record in enumerate(resource_records):
                if record["list"] == list_name:
                    by_key.setdefault(record["key"], []).append((record_index, record))
            for key, variants in sorted(by_key.items()):
                first = next(
                    (item for item in variants if float(item[1]["density"]) == 1.0),
                    None,
                )
                if first is None:
                    raise _diagnostic(
                        "WFGT023", f"local image {list_name}/{key} has no 1x resource"
                    )
                first_index, _ = first
                source_lines.extend(
                    [
                        f"    if (!resource_{list_symbol}->add_png("
                        f"{_narrow_cpp_string(key)}, {{local_png_{first_index}, sizeof(local_png_{first_index})}}, 1.0)) {{",
                        f"        throw std::runtime_error(\"native PNG {list_name}/{key}@1x was rejected\");",
                        "    }",
                    ]
                )
                for record_index, record in variants:
                    density = float(record["density"])
                    if density == 1.0:
                        continue
                    source_lines.extend(
                        [
                            f"    if (!resource_{list_symbol}->set_variant_png("
                            f"{_narrow_cpp_string(key)}, gui_forms::ImageVisualState::normal, "
                            f"{repr(density)}, {{local_png_{record_index}, sizeof(local_png_{record_index})}})) {{",
                            f"        throw std::runtime_error(\"native PNG {list_name}/{key}@{density:g}x was rejected\");",
                            "    }",
                        ]
                    )
            targets = sorted({
                node["id"] for node in runtime_nodes
                if node.get("attributes", {}).get("data-wf-image-list") == list_name
            })
            for target in targets:
                source_lines.append(
                    f"    form.{_symbol(target)}->set_image_list(resource_{list_symbol});"
                )
            source_lines.append(
                f"    form.resource_{list_symbol} = std::move(resource_{list_symbol});"
            )
    source_lines.extend(
        ["}", "", f"}}  // namespace {namespace}", ""]
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
        "nodes": [
            {
                "id": node["id"],
                "parent_id": (
                    nodes[parent_index]["id"]
                    if (parent_index := _runtime_parent(
                        node, nodes, runtime_indexes
                    )) is not None
                    else None
                ),
                "kind": (
                    node.get("attributes", {}).get("data-wf-control", "button")
                    if types[node["index"]] in {
                        "gui_forms::Button", "gui_forms::DropDownButton"
                    }
                    else "label"
                    if types[node["index"]] == "gui_forms::Label"
                    else node.get("attributes", {}).get("data-wf-control", "panel")
                ),
                "topology": (
                    {
                        "connected_axis": node["attributes"]["data-wf-connected-axis"],
                        "connected_members": [
                            child["id"] for child in runtime_children[node["index"]]
                        ],
                    }
                    if "data-wf-connected-axis" in node.get("attributes", {})
                    else None
                ),
                "resource": (
                    {
                        "list": node["attributes"]["data-wf-image-list"],
                        "key": node["attributes"]["data-wf-image-key"],
                        "source_1x": node["attributes"]["data-wf-image-src"],
                        "source_2x": node["attributes"].get("data-wf-image-src-2x"),
                    }
                    if "data-wf-image-src" in node.get("attributes", {})
                    else None
                ),
            }
            for node in runtime_nodes
        ],
        "button_count": sum(
            1 for node in runtime_nodes if types[node["index"]] in {
                "gui_forms::Button", "gui_forms::DropDownButton"
            }
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
        "responsive_track_count": sum(
            1 for node in runtime_nodes
            if types[node["index"]] == "gui_forms::ResponsiveTrackPanel"
        ),
        "split_container_count": sum(
            1 for node in runtime_nodes
            if types[node["index"]] == "gui_forms::SplitContainer"
        ),
        "connected_group_count": connected_group_count,
        "local_png_resource_count": len(resource_records),
        "local_png_list_count": len(resource_lists),
        "commands": [
            {
                "id": node["id"],
                "command": node["attributes"].get("data-wf-command", ""),
                "member": _symbol(node["id"]),
            }
            for node in runtime_nodes
            if types[node["index"]] in {
                "gui_forms::Button", "gui_forms::DropDownButton"
            }
        ],
        "implicit_block_flow_nodes": [
            node["id"]
            for node in runtime_nodes
            if types[node["index"]] == "gui_forms::FlowLayoutPanel"
            and geometry_by_node[node["index"]].get("display", {}).get("value")
            not in {"flex", "grid"}
        ],
        "initially_hidden_nodes": [
            node["id"]
            for node in runtime_nodes
            if geometry_by_node[node["index"]].get("display", {}).get("value")
            == "none"
            and node.get("attributes", {}).get("data-wf-overflow-actuator")
            != "true"
        ],
        "window_drag_region_ids": drag_region_ids,
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
