from __future__ import annotations

from dataclasses import dataclass
import json
import math
from pathlib import Path
from typing import Any

from .capabilities import assess_capabilities
from .diagnostics import Diagnostic, WebFormsError
from .keylines import parse_keylines
from .profile import IR_SCHEMA
from .stage2 import _symbol, validate_generated_cpp


SURFACE_PROPERTIES = frozenset(
    {
        "background",
        "background-color",
        "background-image",
        "background-position",
        "background-repeat",
        "background-size",
        "border",
        "border-top",
        "border-right",
        "border-bottom",
        "border-left",
        "border-color",
        "border-style",
        "border-width",
        "border-radius",
        "box-shadow",
        "filter",
        "opacity",
        "outline",
        "outline-color",
        "outline-offset",
        "outline-style",
        "outline-width",
        "visibility",
    }
)


@dataclass(frozen=True)
class ColorValue:
    red: int
    green: int
    blue: int
    alpha: int


@dataclass(frozen=True)
class GradientStopValue:
    offset: float
    color: ColorValue


@dataclass(frozen=True)
class GradientValue:
    kind: str
    angle_degrees: float
    stops: tuple[GradientStopValue, ...]
    center_x: float = 0.5
    center_y: float = 0.5
    radius_x: float = 0.5
    radius_y: float = 0.5
    repeat_period: float = 0.0


@dataclass(frozen=True)
class BorderValue:
    width: float
    color: ColorValue


@dataclass(frozen=True)
class BorderEdgesValue:
    top: BorderValue | None
    right: BorderValue | None
    bottom: BorderValue | None
    left: BorderValue | None


@dataclass(frozen=True)
class ShadowValue:
    offset_x: float
    offset_y: float
    blur: float
    spread: float
    color: ColorValue
    inset: bool


@dataclass(frozen=True)
class MaterialValue:
    color: ColorValue
    gradients: tuple[GradientValue, ...]
    border: BorderValue | None
    border_edges: BorderEdgesValue | None
    shadows: tuple[ShadowValue, ...]
    corner_radius: float


@dataclass(frozen=True)
class RecipeValue:
    material: MaterialValue
    text: ColorValue
    focus_ring: ColorValue
    focus_width: float
    focus_offset: float
    authored_focus_outline: bool
    visual_offset_x: float
    visual_offset_y: float


def _diagnostic(code: str, message: str) -> WebFormsError:
    return WebFormsError([Diagnostic(code, message, "<ir>")])


def _color(token: dict[str, Any]) -> ColorValue:
    if token.get("kind") != "color_rgba":
        raise _diagnostic("WFGM010", "expected a packed color token")
    packed = int(token.get("data", 0))
    return ColorValue(
        (packed >> 24) & 0xFF,
        (packed >> 16) & 0xFF,
        (packed >> 8) & 0xFF,
        packed & 0xFF,
    )


def _length(token: dict[str, Any], purpose: str) -> float:
    if token.get("kind") not in {"number", "logical_px"}:
        raise _diagnostic("WFGM011", f"{purpose} requires a logical-pixel token")
    value = float(token.get("number", 0.0))
    if not math.isfinite(value):
        raise _diagnostic("WFGM012", f"{purpose} is not finite")
    return value


def _single_color(item: dict[str, Any], purpose: str) -> ColorValue:
    tokens = item["typed"]["tokens"]
    if item["typed"]["kind"] != "color" or len(tokens) != 1:
        raise _diagnostic("WFGM013", f"{purpose} requires one color token")
    return _color(tokens[0])


def _gradient_stops(
    tokens: list[dict[str, Any]], *, repeating: bool
) -> tuple[tuple[GradientStopValue, ...], float]:
    raw: list[tuple[ColorValue, dict[str, Any] | None]] = []
    position = 0
    while position < len(tokens):
        if tokens[position].get("kind") != "color_rgba":
            raise _diagnostic("WFGM016", "gradient requires color stops")
        color = _color(tokens[position])
        position += 1
        stop_position = None
        if position < len(tokens) and tokens[position].get("kind") in {
            "percent", "logical_px"
        }:
            stop_position = tokens[position]
            position += 1
        raw.append((color, stop_position))
    if len(raw) < 2 or len(raw) > 32:
        raise _diagnostic("WFGM017", "gradient stop count is outside the retained limit")
    authored = [item[1] for item in raw]
    if any(item is None for item in authored) and any(item is not None for item in authored):
        raise _diagnostic(
            "WFGM018", "bounded gradient stops must either all have positions or all omit them"
        )
    if all(item is None for item in authored):
        denominator = len(raw) - 1
        return tuple(
            GradientStopValue(index / denominator, color)
            for index, (color, _) in enumerate(raw)
        ), 0.0
    required_kind = "logical_px" if repeating else "percent"
    if any(item is None or item.get("kind") != required_kind for item in authored):
        suffix = "logical-pixel" if repeating else "percent"
        raise _diagnostic("WFGM019", f"gradient requires {suffix} stop positions")
    values = [float(item.get("number", 0.0)) for item in authored if item is not None]
    if any(right < left for left, right in zip(values, values[1:])):
        raise _diagnostic("WFGM019", "gradient stop positions must be nondecreasing")
    if repeating:
        period = values[-1]
        if values[0] != 0.0 or period <= 0.0 or period > 256.0:
            raise _diagnostic(
                "WFGM019", "repeating gradient must span 0px through a period at most 256px"
            )
        return tuple(
            GradientStopValue(value / period, raw[index][0])
            for index, value in enumerate(values)
        ), period
    if values[0] < 0.0 or values[-1] > 100.0:
        raise _diagnostic("WFGM019", "gradient percent stops must remain within 0 through 100")
    return tuple(
        GradientStopValue(value / 100.0, raw[index][0])
        for index, value in enumerate(values)
    ), 0.0


def _gradient_layer(tokens: list[dict[str, Any]]) -> GradientValue:
    if not tokens or tokens[0].get("kind") != "keyword":
        raise _diagnostic("WFGM014", "background-image is not a supported gradient")
    function = tokens[0].get("text")
    if function == "repeating-radial-gradient":
        raise _diagnostic("WFGM015", "repeating radial gradients are not in the bounded native projection")
    if function not in {"linear-gradient", "radial-gradient", "repeating-linear-gradient"}:
        raise _diagnostic("WFGM015", "background-image gradient kind is not supported")
    position = 1
    angle = 180.0
    center_x = center_y = radius_x = radius_y = 0.5
    if function in {"linear-gradient", "repeating-linear-gradient"}:
        if position < len(tokens) and tokens[position].get("kind") == "angle_deg":
            angle = float(tokens[position].get("number", 0.0)) % 360.0
            position += 1
    elif position < len(tokens) and tokens[position].get("text") == "ellipse":
        if position + 5 >= len(tokens) or tokens[position + 3].get("text") != "at":
            raise _diagnostic("WFGM015", "radial ellipse geometry is malformed")
        values = tokens[position + 1 : position + 6]
        if any(values[index].get("kind") != "percent" for index in (0, 1, 3, 4)):
            raise _diagnostic("WFGM015", "radial ellipse geometry requires percentages")
        radius_x = float(values[0].get("number", 0.0)) / 100.0
        radius_y = float(values[1].get("number", 0.0)) / 100.0
        center_x = float(values[3].get("number", 0.0)) / 100.0
        center_y = float(values[4].get("number", 0.0)) / 100.0
        if radius_x <= 0.0 or radius_y <= 0.0 or radius_x > 8.0 or radius_y > 8.0:
            raise _diagnostic("WFGM015", "radial radii must be positive and at most 800 percent")
        position += 6
    repeating = function == "repeating-linear-gradient"
    stops, repeat_period = _gradient_stops(tokens[position:], repeating=repeating)
    kind = "repeating_linear" if repeating else (
        "radial" if function == "radial-gradient" else "linear"
    )
    return GradientValue(
        kind, angle, stops, center_x, center_y, radius_x, radius_y, repeat_period
    )


def _gradients(item: dict[str, Any]) -> tuple[GradientValue, ...]:
    typed = item["typed"]
    tokens = typed["tokens"]
    if typed["kind"] == "keyword" and len(tokens) == 1 and tokens[0].get("text") == "none":
        return tuple()
    if typed["kind"] != "gradient" or not tokens:
        raise _diagnostic("WFGM014", "background-image is not a supported gradient list")
    groups: list[list[dict[str, Any]]] = [[]]
    for token in tokens:
        if token.get("kind") == "separator":
            groups.append([])
        else:
            groups[-1].append(token)
    if any(not group for group in groups) or len(groups) > 7:
        raise _diagnostic("WFGM014", "gradient layer count is outside the retained limit")
    return tuple(_gradient_layer(group) for group in groups)


def _parse_border_item(item: dict[str, Any], purpose: str) -> BorderValue | None:
    tokens = item["typed"]["tokens"]
    if len(tokens) != 3:
        raise _diagnostic(
            "WFGM022",
            f"the first native {purpose} projection requires width style color",
        )
    width = _length(tokens[0], f"{purpose} width")
    if tokens[1].get("kind") != "keyword" or tokens[1].get("text") != "solid":
        raise _diagnostic(
            "WFGM023",
            f"the first native {purpose} projection requires solid style",
        )
    if width == 0.0:
        return None
    if width < 0.0:
        raise _diagnostic("WFGM024", f"{purpose} width cannot be negative")
    return BorderValue(width, _color(tokens[2]))


def _border(
    properties: dict[str, dict[str, Any]],
) -> tuple[BorderValue | None, BorderEdgesValue | None]:
    side_names = {"border-top", "border-right", "border-bottom", "border-left"}
    present_sides = side_names.intersection(properties)
    border_item = properties.get("border")
    if present_sides and border_item is not None:
        raise _diagnostic(
            "WFGM020",
            "mixed border and side-border shorthands need canonical cascade expansion",
        )
    if present_sides and "border-color" in properties:
        raise _diagnostic(
            "WFGM025",
            "mixed border-color and side borders need canonical cascade expansion",
        )
    if present_sides:
        edges = BorderEdgesValue(
            _parse_border_item(properties["border-top"], "top border")
            if "border-top" in properties else None,
            _parse_border_item(properties["border-right"], "right border")
            if "border-right" in properties else None,
            _parse_border_item(properties["border-bottom"], "bottom border")
            if "border-bottom" in properties else None,
            _parse_border_item(properties["border-left"], "left border")
            if "border-left" in properties else None,
        )
        return None, edges
    if border_item is None:
        if any(name in properties for name in ("border-width", "border-style")):
            raise _diagnostic("WFGM021", "split border declarations need an explicit base border")
        return None, None
    border = _parse_border_item(border_item, "border")
    color_item = properties.get("border-color")
    if color_item is not None and border is not None:
        border = BorderValue(
            border.width, _single_color(color_item, "border-color")
        )
    return border, None


def _shadow_parts(tokens: list[dict[str, Any]]) -> ShadowValue:
    inset = any(
        token.get("kind") == "keyword" and token.get("text") == "inset"
        for token in tokens
    )
    colors = [token for token in tokens if token.get("kind") == "color_rgba"]
    lengths = [token for token in tokens if token.get("kind") in {"number", "logical_px"}]
    others = [
        token for token in tokens
        if token.get("kind") not in {"color_rgba", "number", "logical_px"}
        and not (token.get("kind") == "keyword" and token.get("text") == "inset")
    ]
    if others or len(colors) != 1 or len(lengths) not in {2, 3, 4}:
        raise _diagnostic(
            "WFGM031",
            "shadow projection requires x y, optional blur/spread, and one color",
        )
    values = [_length(token, "shadow length") for token in lengths]
    blur = values[2] if len(values) >= 3 else 0.0
    spread = values[3] if len(values) == 4 else 0.0
    if blur < 0.0:
        raise _diagnostic("WFGM032", "shadow blur cannot be negative")
    return ShadowValue(
        values[0], values[1], blur, spread, _color(colors[0]), inset
    )


def _shadows(item: dict[str, Any] | None) -> tuple[ShadowValue, ...]:
    if item is None:
        return tuple()
    groups: list[list[dict[str, Any]]] = [[]]
    for token in item["typed"]["tokens"]:
        if token.get("kind") == "separator":
            groups.append([])
        else:
            groups[-1].append(token)
    if any(not group for group in groups) or len(groups) > 4:
        raise _diagnostic("WFGM033", "shadow list is outside the retained limit")
    return tuple(_shadow_parts(group) for group in groups)


def _corner_radius(item: dict[str, Any] | None) -> float:
    if item is None:
        return 0.0
    tokens = item["typed"]["tokens"]
    if len(tokens) != 1:
        raise _diagnostic("WFGM040", "nonuniform corner radii need a four-corner primitive")
    radius = _length(tokens[0], "corner radius")
    if radius < 0.0:
        raise _diagnostic("WFGM041", "corner radius cannot be negative")
    return radius


def _lower_material(record: dict[str, Any]) -> MaterialValue:
    properties = {item["name"]: item for item in record["properties"]}
    if "background" in properties:
        raise _diagnostic("WFGM050", "IR contains an uncanonicalized background shorthand")
    background = properties.get("background-color")
    if background is None:
        raise _diagnostic("WFGM051", "surface material has no background-color")
    unsupported = {
        name for name in properties
        if name in SURFACE_PROPERTIES and name in {
            "background-position", "background-repeat", "background-size", "filter",
            "opacity", "outline", "outline-color", "outline-offset",
            "outline-style", "outline-width", "visibility",
        }
    }
    if unsupported:
        raise _diagnostic(
            "WFGM052",
            "unsupported surface properties: " + ", ".join(sorted(unsupported)),
        )
    gradient_item = properties.get("background-image")
    gradients = tuple() if gradient_item is None else _gradients(gradient_item)
    border, border_edges = _border(properties)
    corner_radius = _corner_radius(properties.get("border-radius"))
    if border_edges is not None and corner_radius != 0.0:
        raise _diagnostic(
            "WFGM053",
            "side borders with rounded joins need a proven corner primitive",
        )
    return MaterialValue(
        _single_color(background, "background-color"),
        gradients,
        border,
        border_edges,
        _shadows(properties.get("box-shadow")),
        corner_radius,
    )


def _properties(record: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {item["name"]: item for item in record["properties"]}


def _merge_records(*records: dict[str, Any]) -> dict[str, Any]:
    merged: dict[str, dict[str, Any]] = {}
    for record in records:
        merged.update(_properties(record))
    return {"properties": [merged[name] for name in sorted(merged)]}


def _without_properties(record: dict[str, Any], names: frozenset[str]) -> dict[str, Any]:
    return {
        "properties": [
            item for item in record["properties"] if item["name"] not in names
        ]
    }


def _record_color(record: dict[str, Any], name: str,
                  fallback: ColorValue) -> ColorValue:
    item = _properties(record).get(name)
    return fallback if item is None else _single_color(item, name)


def _focus_outline(record: dict[str, Any] | None) -> tuple[ColorValue, float, float, bool]:
    fallback = ColorValue(38, 114, 185, 255)
    if record is None or not record["properties"]:
        return fallback, 0.0, 0.0, False
    properties = _properties(record)
    outline = properties.get("outline")
    split = {
        name for name in properties
        if name in {"outline-color", "outline-style", "outline-width"}
    }
    if outline is None:
        if split or "outline-offset" in properties:
            raise _diagnostic(
                "WFGM060",
                "focus projection requires the canonical outline shorthand",
            )
        return fallback, 0.0, 0.0, False
    if split:
        raise _diagnostic(
            "WFGM061",
            "mixed outline shorthand and longhands need canonical cascade expansion",
        )
    parsed = _parse_border_item(outline, "focus outline")
    if parsed is None:
        return fallback, 0.0, 0.0, False
    offset_item = properties.get("outline-offset")
    offset = 0.0
    if offset_item is not None:
        tokens = offset_item["typed"]["tokens"]
        if len(tokens) != 1:
            raise _diagnostic("WFGM062", "outline-offset requires one logical length")
        offset = _length(tokens[0], "outline offset")
    if offset < -64.0 or offset > 64.0:
        raise _diagnostic(
            "WFGM063",
            "the native authored focus outline offset must be between -64 and 64px",
        )
    return parsed.color, parsed.width, offset, True


def _visual_offset(record: dict[str, Any]) -> tuple[float, float]:
    properties = _properties(record)
    if not properties:
        return 0.0, 0.0
    if set(properties) != {"transform"}:
        raise _diagnostic(
            "WFGM070",
            "button state geometry supports only a bounded translate transform",
        )
    typed = properties["transform"]["typed"]
    tokens = typed["tokens"]
    if typed["kind"] != "transform" or not tokens or tokens[0].get("kind") != "keyword":
        raise _diagnostic("WFGM071", "button state transform is not canonical")
    function = tokens[0].get("text")
    arguments = tokens[1:]
    if any(token.get("kind") not in {"number", "logical_px"} for token in arguments):
        raise _diagnostic("WFGM072", "button state translation must use logical pixels")
    if function == "translatex" and len(arguments) == 1:
        x, y = _length(arguments[0], "translateX"), 0.0
    elif function == "translatey" and len(arguments) == 1:
        x, y = 0.0, _length(arguments[0], "translateY")
    elif function == "translate" and len(arguments) in {1, 2}:
        x = _length(arguments[0], "translate x")
        y = 0.0 if len(arguments) == 1 else _length(arguments[1], "translate y")
    else:
        raise _diagnostic(
            "WFGM073",
            "button state transform permits one translate, translateX, or translateY",
        )
    if abs(x) > 32.0 or abs(y) > 32.0:
        raise _diagnostic("WFGM074", "button state translation exceeds 32 logical pixels")
    return x, y


def _number(value: float) -> str:
    rendered = repr(float(value))
    return rendered if "." in rendered else rendered + ".0"


def _emit_color(value: ColorValue) -> str:
    return (
        "gui_forms::Color::rgba("
        f"{value.red}U, {value.green}U, {value.blue}U, {value.alpha}U)"
    )


def _emit_material(lines: list[str], symbol: str, material: MaterialValue) -> None:
    for gradient_index, gradient in enumerate(material.gradients):
        lines.append(
            f"static const gui_forms::GradientStop material_{symbol}_gradient_{gradient_index}_stops[] = {{"
        )
        for stop in gradient.stops:
            lines.append(
                f"    {{{_number(stop.offset)}, {_emit_color(stop.color)}}},"
            )
        lines.append("};")
        lines.append("")
    lines.append(f"static const gui_forms::MaterialFillLayer material_{symbol}_fills[] = {{")
    lines.append(
        "    gui_forms::MaterialFillLayer::solid("
        f"{_emit_color(material.color)}),"
    )
    # CSS lists the topmost background first. GUI.Forms replays retained fill
    # layers from back to front, so emit the authored images in reverse order
    # above the solid fallback.
    for gradient_index in reversed(range(len(material.gradients))):
        gradient = material.gradients[gradient_index]
        stops = f"material_{symbol}_gradient_{gradient_index}_stops"
        count = f"sizeof({stops}) / sizeof({stops}[0])"
        if gradient.kind == "linear":
            lines.append("    gui_forms::MaterialFillLayer::linear_css_angle(")
            lines.append(f"        {_number(gradient.angle_degrees)},")
            lines.append(f"        {stops}, {count},")
            lines.append("        gui_forms::GradientSpreadMode::pad),")
        elif gradient.kind == "radial":
            lines.append("    gui_forms::MaterialFillLayer::radial(")
            lines.append(
                f"        {{{_number(gradient.center_x)}, {_number(gradient.center_y)}}},"
            )
            lines.append(
                f"        {{{_number(gradient.radius_x)}, {_number(gradient.radius_y)}}},"
            )
            lines.append(f"        {stops}, {count},")
            lines.append("        gui_forms::MaterialCoordinateSpace::normalized),")
        elif gradient.kind == "repeating_linear":
            angle = math.radians(gradient.angle_degrees)
            end_x = math.sin(angle) * gradient.repeat_period
            end_y = -math.cos(angle) * gradient.repeat_period
            lines.append("    gui_forms::MaterialFillLayer::repeating_linear(")
            lines.append(
                f"        {{0.0, 0.0}}, {{{_number(end_x)}, {_number(end_y)}}},"
            )
            lines.append(f"        {stops}, {count},")
            lines.append("        gui_forms::MaterialCoordinateSpace::logical),")
        else:
            raise AssertionError(f"unknown gradient kind {gradient.kind}")
    lines.append("};")
    lines.append("")
    if material.shadows:
        lines.append(f"static const gui_forms::MaterialShadow material_{symbol}_shadows[] = {{")
        for shadow in material.shadows:
            lines.append(
                "    {{"
                f"{_number(shadow.offset_x)}, {_number(shadow.offset_y)}}}, "
                f"{_number(shadow.blur)}, {_number(shadow.spread)}, "
                f"{_emit_color(shadow.color)}, "
                + ("true}," if shadow.inset else "false},")
            )
        lines.append("};")
        lines.append("")
    if material.border is not None:
        lines.append(
            f"static const gui_forms::MaterialBorder material_{symbol}_border = "
            "{"
            f"{_emit_color(material.border.color)}, {_number(material.border.width)}"
            "};"
        )
        lines.append("")
    if material.border_edges is not None:
        edge_pointers: list[str] = []
        for edge_name in ("top", "right", "bottom", "left"):
            edge = getattr(material.border_edges, edge_name)
            if edge is None:
                edge_pointers.append("nullptr")
                continue
            edge_symbol = f"material_{symbol}_{edge_name}_border"
            lines.append(
                f"static const gui_forms::MaterialBorder {edge_symbol} = "
                "{"
                f"{_emit_color(edge.color)}, {_number(edge.width)}"
                "};"
            )
            edge_pointers.append(f"&{edge_symbol}")
        lines.append(
            f"static const gui_forms::MaterialBorderEdges material_{symbol}_border_edges = "
            "gui_forms::MaterialBorderEdges::from_parts("
            + ", ".join(edge_pointers)
            + ");"
        )
        lines.append("")
    shadow_pointer = f"material_{symbol}_shadows" if material.shadows else "nullptr"
    shadow_count = (
        f"sizeof(material_{symbol}_shadows) / sizeof(material_{symbol}_shadows[0])"
        if material.shadows
        else "0U"
    )
    border_pointer = f"&material_{symbol}_border" if material.border is not None else "nullptr"
    border_edges_pointer = (
        f"&material_{symbol}_border_edges"
        if material.border_edges is not None
        else "nullptr"
    )
    lines.append(f"gui_forms::SurfaceMaterial make_material_{symbol}() {{")
    lines.append("    return gui_forms::SurfaceMaterial::from_parts(")
    lines.append(
        f"        material_{symbol}_fills, "
        f"sizeof(material_{symbol}_fills) / sizeof(material_{symbol}_fills[0]),"
    )
    lines.append(
        f"        {shadow_pointer}, {shadow_count}, {border_pointer}, "
        f"{border_edges_pointer}, "
        f"{_number(material.corner_radius)});"
    )
    lines.append("}")
    lines.append("")


def _emit_recipe(lines: list[str], symbol: str, recipe: RecipeValue) -> None:
    lines.append(f"gui_forms::ControlVisualRecipe make_recipe_{symbol}() {{")
    lines.append("    gui_forms::ControlVisualRecipe recipe;")
    lines.append(f"    recipe.material = make_material_{symbol}();")
    lines.append(f"    recipe.text = {_emit_color(recipe.text)};")
    lines.append(f"    recipe.muted_text = {_emit_color(recipe.text)};")
    lines.append(f"    recipe.glyph = {_emit_color(recipe.text)};")
    lines.append(f"    recipe.focus_ring = {_emit_color(recipe.focus_ring)};")
    lines.append("    recipe.default_ring = recipe.focus_ring;")
    lines.append(f"    recipe.focus_width = {_number(recipe.focus_width)};")
    lines.append(f"    recipe.focus_offset = {_number(recipe.focus_offset)};")
    lines.append(
        "    recipe.authored_focus_outline = "
        + ("true;" if recipe.authored_focus_outline else "false;")
    )
    lines.append("    recipe.default_width = 0.0;")
    lines.append(
        "    recipe.visual_offset = {"
        f"{_number(recipe.visual_offset_x)}, {_number(recipe.visual_offset_y)}"
        "};"
    )
    lines.append("    recipe.pressed_content_offset = {0.0, 0.0};")
    lines.append("    return recipe;")
    lines.append("}")
    lines.append("")


OUTLINE_PROPERTIES = frozenset(
    {"outline", "outline-color", "outline-offset", "outline-style", "outline-width"}
)
RETAINED_RECIPE_STATES = (
    "normal", "hot", "pressed", "pending", "invalid", "disabled", "deactivated"
)


def _button_state_recipes(
    ir: dict[str, Any], node: dict[str, Any], style: dict[str, Any]
) -> tuple[dict[str, RecipeValue], list[str]]:
    pools = ir["styles"]["pools"]
    variants = [
        item for item in ir["styles"]["variants"] if item["node"] == node["index"]
    ]
    by_state: dict[str, dict[str, Any]] = {}
    for variant in variants:
        states = variant["states"]
        if len(states) != 1 or states[0] not in {
            "hover", "active", "focus", "focus-visible", "disabled"
        }:
            raise _diagnostic(
                "WFGM080",
                f"button {node['id']!r} uses a state combination outside the retained recipe map",
            )
        state = states[0]
        if state in by_state:
            raise _diagnostic("WFGM081", f"button {node['id']!r} repeats state {state!r}")
        by_state[state] = variant
        if pools["typography"][variant["typography"]]["properties"]:
            raise _diagnostic(
                "WFGM082",
                f"button {node['id']!r} changes typography in state {state!r}",
            )

    if "focus" in by_state and "focus-visible" in by_state:
        raise _diagnostic(
            "WFGM083",
            f"button {node['id']!r} needs distinct focus and focus-visible runtime contexts",
        )
    empty = {"properties": []}

    def domain(state: str, name: str) -> dict[str, Any]:
        variant = by_state.get(state)
        return empty if variant is None else pools[name][variant[name]]

    for state in ("hover", "focus", "focus-visible", "disabled"):
        if domain(state, "geometry")["properties"]:
            raise _diagnostic(
                "WFGM084",
                f"button {node['id']!r} has unsupported {state!r} geometry",
            )
    visual_x, visual_y = _visual_offset(domain("active", "geometry"))

    focus_state = "focus-visible" if "focus-visible" in by_state else "focus"
    focus_material = domain(focus_state, "material")
    focus_names = set(_properties(focus_material))
    if focus_names - OUTLINE_PROPERTIES:
        raise _diagnostic(
            "WFGM085",
            f"button {node['id']!r} changes its surface on focus; retained recipes need a focus axis",
        )
    focus_ring, focus_width, focus_offset, authored_focus_outline = _focus_outline(
        focus_material if focus_state in by_state else None
    )

    base = pools["material"][style["material"]]
    hover = domain("hover", "material")
    active = domain("active", "material")
    disabled = domain("disabled", "material")
    overlap = set(_properties(hover)).intersection(_properties(active))
    conflicting = {
        name for name in overlap
        if _properties(hover)[name] != _properties(active)[name]
    }
    if conflicting:
        raise _diagnostic(
            "WFGM086",
            "pressed projection lost CSS cascade order for: "
            + ", ".join(sorted(conflicting)),
        )

    normal_record = _without_properties(base, OUTLINE_PROPERTIES)
    hot_record = _without_properties(_merge_records(base, hover), OUTLINE_PROPERTIES)
    pressed_record = _without_properties(
        _merge_records(base, hover, active), OUTLINE_PROPERTIES
    )
    disabled_record = _without_properties(
        _merge_records(base, disabled), OUTLINE_PROPERTIES
    )
    black = ColorValue(0, 0, 0, 255)

    def recipe(record: dict[str, Any], x: float = 0.0, y: float = 0.0) -> RecipeValue:
        return RecipeValue(
            _lower_material(record),
            _record_color(record, "color", black),
            focus_ring,
            focus_width,
            focus_offset,
            authored_focus_outline,
            x,
            y,
        )

    normal = recipe(normal_record)
    result = {
        "normal": normal,
        "hot": recipe(hot_record),
        "pressed": recipe(pressed_record, visual_x, visual_y),
        "pending": normal,
        "invalid": normal,
        "disabled": recipe(disabled_record),
        "deactivated": normal,
    }
    notes: list[str] = []
    if "disabled" not in by_state:
        notes.append("no authored :disabled rule; disabled uses the authored base recipe")
    if "focus-visible" in by_state:
        notes.append(
            "focus-visible cue geometry and GUI.Forms keyboard/pointer/semantic modality are retained"
        )
    if "hover" in by_state and "active" in by_state:
        notes.append(
            "pressed composes nonconflicting hover and active deltas for pointer activation"
        )
    return result, notes


def generate_gui_materials(
    ir: dict[str, Any], manifest: dict[str, Any], output_dir: Path,
    unit_name: str | None = None,
) -> tuple[Path, Path, Path]:
    if ir.get("schema") != IR_SCHEMA:
        raise _diagnostic("WFGM001", f"native material Stage 2 requires {IR_SCHEMA!r}")
    construction = manifest.get("features", {}).get("construction.generated-material-parts", {})
    if construction.get("status") != "supported":
        raise _diagnostic(
            "WFGM002",
            "target manifest does not support generated pointer/count material parts",
        )
    state_construction = manifest.get("features", {}).get(
        "construction.generated-control-state-recipes", {}
    )
    state_recipe_enabled = state_construction.get("status") == "supported"

    root_id = next(
        (node["id"] for node in ir["document"]["nodes"] if node["tag"] == "body"),
        "form",
    )
    unit = _symbol(unit_name or root_id)
    output_dir.mkdir(parents=True, exist_ok=True)
    header_path = output_dir / f"{unit}.gui_materials.wf.hpp"
    source_path = output_dir / f"{unit}.gui_materials.wf.cpp"
    report_path = output_dir / f"{unit}.gui_materials.json"
    namespace = f"web_forms_generated_{unit}"

    referenced: dict[int, list[str]] = {}
    document_nodes = ir["document"]["nodes"]
    for style in ir["styles"]["nodes"]:
        if style["surface"] not in {"own-surface", "baked-into-parent"}:
            continue
        node = document_nodes[style["node"]]
        referenced.setdefault(style["material"], []).append(node["id"] or node["tag"])

    exact: dict[int, MaterialValue] = {}
    records: list[dict[str, Any]] = []
    pools = ir["styles"]["pools"]["material"]
    for index in sorted(referenced):
        record = pools[index]
        try:
            material = _lower_material(record)
        except WebFormsError as error:
            records.append(
                {
                    "style": index,
                    "nodes": referenced[index],
                    "status": "unavailable",
                    "diagnostics": [item.render() for item in error.diagnostics],
                }
            )
        else:
            exact[index] = material
            records.append(
                {
                    "style": index,
                    "nodes": referenced[index],
                    "status": "exact-surface-projection",
                    "diagnostics": [],
                }
            )

    style_by_node = {item["node"]: item for item in ir["styles"]["nodes"]}
    button_recipes: dict[int, dict[str, RecipeValue]] = {}
    button_records: list[dict[str, Any]] = []
    for node in document_nodes:
        if not node["runtime"] or node["control"] not in {
            "button", "dropdown-button"
        }:
            continue
        if not state_recipe_enabled:
            button_records.append(
                {
                    "node": node["index"],
                    "id": node["id"],
                    "status": "unavailable",
                    "diagnostics": [
                        "target manifest does not support generated control-state recipes"
                    ],
                }
            )
            continue
        try:
            recipes, notes = _button_state_recipes(
                ir, node, style_by_node[node["index"]]
            )
        except WebFormsError as error:
            button_records.append(
                {
                    "node": node["index"],
                    "id": node["id"],
                    "status": "unavailable",
                    "diagnostics": [item.render() for item in error.diagnostics],
                }
            )
        else:
            button_recipes[node["index"]] = recipes
            button_records.append(
                {
                    "node": node["index"],
                    "id": node["id"],
                    "status": "bounded-native-state-projection",
                    "states": list(RETAINED_RECIPE_STATES),
                    "diagnostics": notes,
                }
            )
    node_keylines = {
        node["index"]: parse_keylines(
            node["attributes"]["data-wf-keylines"], "<ir>"
        )
        for node in document_nodes
        if "data-wf-keylines" in node.get("attributes", {})
    }
    header_lines = [
        "#pragma once",
        "",
        "#include <cstddef>",
        '#include "gui_forms/surface_material/types/surface_material_types.hpp"',
        '#include "gui_forms/theme/types/theme_types.hpp"',
        "",
        f"namespace {namespace} {{",
        "",
        "bool has_native_surface_material(std::size_t style_index) noexcept;",
        "gui_forms::SurfaceMaterial make_native_surface_material(std::size_t style_index);",
        "void apply_native_material_keylines(std::size_t node_index, gui_forms::SurfaceMaterial& material);",
        "bool has_native_button_state_recipes(std::size_t node_index) noexcept;",
        "gui_forms::ControlStateRecipes make_native_button_state_recipes(std::size_t node_index);",
        "void apply_native_recipe_keylines(std::size_t node_index, gui_forms::ControlStateRecipes& recipes);",
        "",
        f"}}  // namespace {namespace}",
        "",
    ]
    source_lines = [
        f'#include "{header_path.name}"',
        "",
        "#include <stdexcept>",
        "",
        f"namespace {namespace} {{",
        "namespace {",
        "",
    ]
    for index, material in exact.items():
        _emit_material(source_lines, str(index), material)
    for node_index, keylines in node_keylines.items():
        source_lines.append(
            f"static const gui_forms::MaterialKeyline node_{node_index}_keylines[] = {{"
        )
        for keyline in keylines:
            color = ColorValue(*keyline["color"])
            source_lines.append(
                "    {gui_forms::MaterialEdge::"
                f"{keyline['edge']}, {_emit_color(color)}, "
                f"{_number(keyline['width'])}, {_number(keyline['inset'])}}},"
            )
        source_lines.extend(["};", ""])
    for node_index, recipes in button_recipes.items():
        for state in RETAINED_RECIPE_STATES:
            symbol = f"button_{node_index}_{state}"
            _emit_material(source_lines, symbol, recipes[state].material)
            _emit_recipe(source_lines, symbol, recipes[state])
        source_lines.append(
            f"gui_forms::ControlStateRecipes make_button_recipes_{node_index}() {{"
        )
        source_lines.append(
            f"    const gui_forms::ControlVisualRecipe button_{node_index}_recipes[] = {{"
        )
        for state in RETAINED_RECIPE_STATES:
            source_lines.append(
                f"        make_recipe_button_{node_index}_{state}(),"
            )
        source_lines.extend(
            [
                "    };",
                "    return gui_forms::ControlStateRecipes::from_parts(",
                f"        button_{node_index}_recipes,",
                f"        sizeof(button_{node_index}_recipes) / sizeof(button_{node_index}_recipes[0]));",
                "}",
                "",
            ]
        )
    source_lines.extend(
        [
            "}  // namespace",
            "",
            "bool has_native_surface_material(std::size_t style_index) noexcept {",
            "    switch (style_index) {",
        ]
    )
    for index in exact:
        source_lines.append(f"    case {index}U:")
    if exact:
        source_lines.append("        return true;")
    source_lines.extend(
        [
            "    default:",
            "        return false;",
            "    }",
            "}",
            "",
            "gui_forms::SurfaceMaterial make_native_surface_material(",
            "    std::size_t style_index) {",
            "    switch (style_index) {",
        ]
    )
    for index in exact:
        source_lines.extend(
            [
                f"    case {index}U:",
                f"        return make_material_{index}();",
            ]
        )
    source_lines.extend(
        [
            "    default:",
            "        throw std::invalid_argument(\"material style has no exact native projection\");",
            "    }",
            "}",
            "",
            "namespace {",
            "[[maybe_unused]] void apply_keylines(gui_forms::SurfaceMaterial& material,",
            "                    const gui_forms::MaterialKeyline* keylines,",
            "                    std::size_t keyline_count) {",
            "    const gui_forms::MaterialBorder* border =",
            "        material.border ? &*material.border : nullptr;",
            "    const gui_forms::MaterialBorderEdges* edges =",
            "        material.border_edges.empty() ? nullptr : &material.border_edges;",
            "    material = gui_forms::SurfaceMaterial::from_parts(",
            "        material.fills.data(), material.fills.size(),",
            "        material.shadows.data(), material.shadows.size(),",
            "        border, edges, keylines, keyline_count, material.corner_radius);",
            "}",
            "}  // namespace",
            "",
            "void apply_native_material_keylines(",
            "    std::size_t node_index, gui_forms::SurfaceMaterial& material) {",
            "    (void)material;",
            "    switch (node_index) {",
        ]
    )
    for node_index in node_keylines:
        source_lines.extend(
            [
                f"    case {node_index}U:",
                f"        apply_keylines(material, node_{node_index}_keylines,",
                f"                       sizeof(node_{node_index}_keylines) / sizeof(node_{node_index}_keylines[0]));",
                "        return;",
            ]
        )
    source_lines.extend(
        [
            "    default:",
            "        return;",
            "    }",
            "}",
            "",
            "bool has_native_button_state_recipes(std::size_t node_index) noexcept {",
            "    switch (node_index) {",
        ]
    )
    for node_index in button_recipes:
        source_lines.append(f"    case {node_index}U:")
    if button_recipes:
        source_lines.append("        return true;")
    source_lines.extend(
        [
            "    default:",
            "        return false;",
            "    }",
            "}",
            "",
            "gui_forms::ControlStateRecipes make_native_button_state_recipes(",
            "    std::size_t node_index) {",
            "    switch (node_index) {",
        ]
    )
    for node_index in button_recipes:
        source_lines.extend(
            [
                f"    case {node_index}U:",
                f"        return make_button_recipes_{node_index}();",
            ]
        )
    source_lines.extend(
        [
            "    default:",
            "        throw std::invalid_argument(\"button node has no bounded native state projection\");",
            "    }",
            "}",
            "",
            "void apply_native_recipe_keylines(",
            "    std::size_t node_index, gui_forms::ControlStateRecipes& recipes) {",
            "    for (gui_forms::ControlVisualRecipe& recipe : recipes.values) {",
            "        apply_native_material_keylines(node_index, recipe.material);",
            "    }",
            "}",
            "",
            f"}}  // namespace {namespace}",
            "",
        ]
    )

    header_text = "\n".join(header_lines)
    source_text = "\n".join(source_lines)
    validate_generated_cpp(header_text, header_path)
    validate_generated_cpp(source_text, source_path)
    header_path.write_text(header_text, encoding="utf-8")
    source_path.write_text(source_text, encoding="utf-8")
    report = {
        "schema": "web.forms.gui-material-projection/0.1-experimental",
        "target": manifest.get("target", "unknown"),
        "source_digest": ir["source"]["digest"],
        "status": "partial-native-material-projection",
        "exact_style_count": len(exact),
        "unavailable_style_count": len(records) - len(exact),
        "styles": records,
        "native_button_state_recipe_count": len(button_recipes),
        "buttons": button_records,
        "keyline_nodes": [
            {"node": node_index, "count": len(keylines)}
            for node_index, keylines in sorted(node_keylines.items())
        ],
        "capability_assessment": assess_capabilities(ir, manifest),
    }
    report_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return header_path, source_path, report_path
