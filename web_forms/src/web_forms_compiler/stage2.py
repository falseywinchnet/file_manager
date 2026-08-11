from __future__ import annotations

import json
from pathlib import Path
import re
from typing import Any

from .diagnostics import Diagnostic, WebFormsError
from .profile import IR_SCHEMA
from .value_types import fnv1a_32


FORBIDDEN_GENERATED_PATTERNS = (
    (re.compile(r"\bauto\b"), "implicit auto typing"),
    (re.compile(r"\bstd::vector\b"), "std::vector"),
    (re.compile(r"\[\s*[^\]]*\]\s*\("), "lambda syntax"),
    (re.compile(r"\bstd::function\b"), "std::function closure machinery"),
)


def generated_code_only(text: str) -> str:
    without_strings = re.sub(r'(?:u8)?"(?:\\.|[^"\\])*"', '""', text)
    without_line_comments = re.sub(r"//.*", "", without_strings)
    return re.sub(r"/\*.*?\*/", "", without_line_comments, flags=re.DOTALL)


def validate_generated_cpp(text: str, path: Path) -> None:
    code = generated_code_only(text)
    for pattern, description in FORBIDDEN_GENERATED_PATTERNS:
        if pattern.search(code):
            raise WebFormsError([Diagnostic("WFG002", f"generator emitted forbidden {description}", str(path))])


def validate_property_ids(ir: dict[str, Any]) -> None:
    by_id: dict[int, str] = {}
    for records in ir["styles"]["pools"].values():
        for record in records:
            for item in record["properties"]:
                name = item["name"]
                property_id = fnv1a_32(name)
                previous = by_id.get(property_id)
                if previous is not None and previous != name:
                    raise WebFormsError(
                        [
                            Diagnostic(
                                "WFG003",
                                f"property ID collision between {previous!r} and {name!r}",
                                "<ir>",
                            )
                        ]
                    )
                by_id[property_id] = name


def _cpp_string(value: str) -> str:
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    escaped = escaped.replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t")
    return f'u8"{escaped}"'


def _symbol(value: str) -> str:
    normalized = re.sub(r"[^a-zA-Z0-9_]", "_", value)
    if not normalized or normalized[0].isdigit():
        normalized = "wf_" + normalized
    return normalized


def _surface(value: str) -> str:
    mapping = {
        "reveal-parent": "SurfaceMode::reveal_parent",
        "own-surface": "SurfaceMode::own_surface",
        "baked-into-parent": "SurfaceMode::baked_into_parent",
    }
    return mapping[value]


def _style_exposure(value: str) -> str:
    return "StyleExposure::exposed" if value == "exposed" else "StyleExposure::baked"


def _emit_style_domain(lines: list[str], domain: str, records: list[dict[str, Any]]) -> None:
    for index, record in enumerate(records):
        properties = record["properties"]
        if not properties:
            continue
        for property_index, item in enumerate(properties):
            typed = item["typed"]
            tokens = typed["tokens"]
            if not tokens:
                continue
            token_name = f"{domain}_style_{index}_property_{property_index}_tokens"
            lines.append(f"static const Property::ValueToken {token_name}[] = {{")
            for token in tokens:
                number = repr(float(token["number"]))
                data = f"0x{int(token['data']):08X}U"
                lines.append(
                    "    {"
                    f"Property::ValueToken::Kind::{_symbol(str(token['kind']))}, "
                    f"{number}, {data}, {_cpp_string(str(token['text']))}"
                    "},"
                )
            lines.append("};")
            lines.append("")
        lines.append(f"static const Property {domain}_properties_{index}[] = {{")
        for property_index, item in enumerate(properties):
            typed = item["typed"]
            count = len(typed["tokens"])
            pointer = "nullptr" if count == 0 else f"{domain}_style_{index}_property_{property_index}_tokens"
            lines.append(
                "    {"
                f"0x{fnv1a_32(item['name']):08X}U, {_cpp_string(item['name'])}, "
                f"Property::ValueKind::{_symbol(typed['kind'])}, {pointer}, {count}U"
                "},"
            )
        lines.append("};")
        lines.append("")
    lines.append(f"static const StyleRecord {domain}_styles[] = {{")
    for index, record in enumerate(records):
        count = len(record["properties"])
        pointer = "nullptr" if count == 0 else f"{domain}_properties_{index}"
        lines.append(f"    {{{pointer}, {count}U}},")
    lines.append("};")
    lines.append("")


def _emit_strings(lines: list[str], name: str, values: list[str]) -> tuple[str, int]:
    if not values:
        return "nullptr", 0
    lines.append(f"static const char* const {name}[] = {{")
    for value in values:
        lines.append(f"    {_cpp_string(value)},")
    lines.append("};")
    lines.append("")
    return name, len(values)


def generate_cpp(ir: dict[str, Any], output_dir: Path, unit_name: str | None = None) -> tuple[Path, Path, Path]:
    if ir.get("schema") != IR_SCHEMA:
        raise WebFormsError([Diagnostic("WFG001", f"Stage 2 requires {IR_SCHEMA!r}", "<ir>")])
    validate_property_ids(ir)
    root_id = next((node["id"] for node in ir["document"]["nodes"] if node["tag"] == "body"), "form")
    unit = _symbol(unit_name or root_id)
    output_dir.mkdir(parents=True, exist_ok=True)
    header_path = output_dir / f"{unit}.wf.hpp"
    source_path = output_dir / f"{unit}.wf.cpp"
    report_path = output_dir / f"{unit}.capabilities.json"

    namespace = f"web_forms_generated_{unit}"
    runtime_nodes = [node for node in ir["document"]["nodes"] if node["runtime"]]
    runtime_index = {node["index"]: index for index, node in enumerate(runtime_nodes)}

    def runtime_parent(node: dict[str, Any]) -> int | None:
        parent = node["parent"]
        all_nodes = ir["document"]["nodes"]
        while parent is not None:
            if parent in runtime_index:
                return runtime_index[parent]
            parent = all_nodes[parent]["parent"]
        return None

    header_lines = [
        "#pragma once",
        "",
        "#include <cstddef>",
        '#include "web_forms/generated/form_descriptor.hpp"',
        "",
        f"namespace {namespace} {{",
        "",
        "struct Handles final {",
    ]
    for node in runtime_nodes:
        if node["id"]:
            header_lines.append(f"    static constexpr std::size_t {_symbol(node['id'])} = {runtime_index[node['index']]}U;")
    header_lines.extend(
        [
            "};",
            "",
            "const web_forms::generated::FormDescriptor& descriptor() noexcept;",
            "",
            f"}}  // namespace {namespace}",
            "",
        ]
    )

    lines = [
        f'#include "{header_path.name}"',
        "",
        f"namespace {namespace} {{",
        "namespace {",
        "",
        "using web_forms::generated::DecorationRecord;",
        "using web_forms::generated::FormDescriptor;",
        "using web_forms::generated::NodeRecord;",
        "using web_forms::generated::Property;",
        "using web_forms::generated::RequirementSet;",
        "using web_forms::generated::StyleExposure;",
        "using web_forms::generated::StyleRecord;",
        "using web_forms::generated::SurfaceMode;",
        "using web_forms::generated::VariantRecord;",
        "",
    ]
    pools = ir["styles"]["pools"]
    _emit_style_domain(lines, "geometry", pools["geometry"])
    _emit_style_domain(lines, "typography", pools["typography"])
    _emit_style_domain(lines, "material", pools["material"])

    style_by_node = {item["node"]: item for item in ir["styles"]["nodes"]}
    lines.append("static const NodeRecord nodes[] = {")
    for node in runtime_nodes:
        style = style_by_node[node["index"]]
        resolved_parent = runtime_parent(node)
        parent = 0 if resolved_parent is None else resolved_parent
        has_parent = "false" if resolved_parent is None else "true"
        lines.append(
            "    {"
            f"{node['index']}U, {parent}U, {has_parent}, true, "
            f"{_cpp_string(node['tag'])}, {_cpp_string(node['id'])}, "
            f"{_cpp_string(node['control'])}, {_cpp_string(node['text'])}, "
            f"{style['geometry']}U, {style['typography']}U, {style['material']}U, "
            f"{_surface(style['surface'])}, {_style_exposure(style['exposure'])}"
            "},"
        )
    lines.append("};")
    lines.append("")

    variants = [item for item in ir["styles"]["variants"] if item["node"] in runtime_index]
    if variants:
        lines.append("static const VariantRecord variants[] = {")
        for item in variants:
            lines.append(
                f"    {{{runtime_index[item['node']]}U, {_cpp_string('+'.join(item['states']))}, "
                f"{item['geometry']}U, {item['typography']}U, {item['material']}U}},"
            )
        lines.append("};")
        lines.append("")

    decorations = [item for item in ir["styles"]["decorations"] if item["node"] in runtime_index]
    if decorations:
        lines.append("static const DecorationRecord decorations[] = {")
        for item in decorations:
            lines.append(
                f"    {{{runtime_index[item['node']]}U, {_cpp_string(item['pseudo'] or '')}, {_cpp_string('+'.join(item['states']))}, "
                f"{_cpp_string(item['content'])}, {item['geometry']}U, {item['typography']}U, {item['material']}U}},"
            )
        lines.append("};")
        lines.append("")

    requirements = ir["requirements"]
    element_ptr, element_count = _emit_strings(lines, "required_elements", requirements["elements"])
    control_ptr, control_count = _emit_strings(lines, "required_controls", requirements["control_kinds"])
    property_ptr, property_count = _emit_strings(lines, "required_properties", requirements["properties"])
    state_ptr, state_count = _emit_strings(lines, "required_states", requirements["states"])
    feature_ptr, feature_count = _emit_strings(lines, "required_features", requirements.get("features", []))

    lines.extend(
        [
            "static const FormDescriptor form_descriptor = {",
            f"    {_cpp_string(ir['schema'])},",
            f"    {_cpp_string(ir['profile'])},",
            f"    {_cpp_string(ir['source']['digest'])},",
            f"    {_cpp_string(ir['document']['title'])},",
            f"    nodes, {len(runtime_nodes)}U,",
            f"    geometry_styles, {len(pools['geometry'])}U,",
            f"    typography_styles, {len(pools['typography'])}U,",
            f"    material_styles, {len(pools['material'])}U,",
            f"    {'variants' if variants else 'nullptr'}, {len(variants)}U,",
            f"    {'decorations' if decorations else 'nullptr'}, {len(decorations)}U,",
            f"    RequirementSet{{{element_ptr}, {element_count}U}},",
            f"    RequirementSet{{{control_ptr}, {control_count}U}},",
            f"    RequirementSet{{{property_ptr}, {property_count}U}},",
            f"    RequirementSet{{{state_ptr}, {state_count}U}},",
            f"    RequirementSet{{{feature_ptr}, {feature_count}U}}",
            "};",
            "",
            "}  // namespace",
            "",
            "const FormDescriptor& descriptor() noexcept {",
            "    return form_descriptor;",
            "}",
            "",
            f"}}  // namespace {namespace}",
            "",
        ]
    )
    header_text = "\n".join(header_lines)
    source_text = "\n".join(lines)
    validate_generated_cpp(header_text, header_path)
    validate_generated_cpp(source_text, source_path)
    header_path.write_text(header_text, encoding="utf-8")
    source_path.write_text(source_text, encoding="utf-8")

    report = {
        "schema": "web.forms.capability-requirements/0.1-experimental",
        "source_digest": ir["source"]["digest"],
        "requirements": requirements,
        "status": "descriptor-generated; GUI.Forms native lowering unproven",
        "fallback_policy": {
            "identity_layout_behavior_accessibility": "required-no-silent-fallback",
            "material_effects": "explicit-reported-fallback-only",
        },
    }
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return header_path, source_path, report_path
