from __future__ import annotations

import re

from .diagnostics import Diagnostic, WebFormsError


NUMBER = re.compile(r"^-?\d+(?:\.\d+)?$")
MEASURE = re.compile(r"^(-?\d+(?:\.\d+)?)(px|%|fr|deg|ms)$")
HEX_COLOR = re.compile(r"^#([0-9a-fA-F]{3,8})$")
FUNCTION = re.compile(r"([a-zA-Z][a-zA-Z-]*)\(([^()]*)\)")


def _split_top_level(value: str, separator: str = ",") -> list[str]:
    result: list[str] = []
    start = 0
    depth = 0
    quote: str | None = None
    for index, char in enumerate(value):
        if quote is not None:
            if char == quote and (index == 0 or value[index - 1] != "\\"):
                quote = None
        elif char in "'\"":
            quote = char
        elif char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
        elif char == separator and depth == 0:
            result.append(value[start:index].strip())
            start = index + 1
    result.append(value[start:].strip())
    return result


def _split_whitespace(value: str) -> list[str]:
    result: list[str] = []
    start: int | None = None
    depth = 0
    quote: str | None = None
    for index, char in enumerate(value):
        if quote is not None:
            if char == quote and (index == 0 or value[index - 1] != "\\"):
                quote = None
        elif char in "'\"":
            quote = char
            if start is None:
                start = index
        elif char == "(":
            depth += 1
            if start is None:
                start = index
        elif char == ")":
            depth -= 1
        elif char.isspace() and depth == 0:
            if start is not None:
                result.append(value[start:index])
                start = None
        elif start is None:
            start = index
    if start is not None:
        result.append(value[start:])
    return result


def _packed_color(value: str) -> int | None:
    lowered = value.lower()
    if lowered == "transparent":
        return 0x00000000
    if lowered == "black":
        return 0x000000FF
    if lowered == "white":
        return 0xFFFFFFFF
    match = HEX_COLOR.fullmatch(value)
    if match is not None:
        digits = match.group(1)
        if len(digits) == 3:
            red, green, blue = (int(char * 2, 16) for char in digits)
            alpha = 255
        elif len(digits) == 4:
            red, green, blue, alpha = (int(char * 2, 16) for char in digits)
        elif len(digits) == 6:
            red = int(digits[0:2], 16)
            green = int(digits[2:4], 16)
            blue = int(digits[4:6], 16)
            alpha = 255
        elif len(digits) == 8:
            red = int(digits[0:2], 16)
            green = int(digits[2:4], 16)
            blue = int(digits[4:6], 16)
            alpha = int(digits[6:8], 16)
        else:
            return None
        return (red << 24) | (green << 16) | (blue << 8) | alpha
    color_function = re.fullmatch(r"rgba?\((.*)\)", lowered)
    if color_function is None:
        return None
    parts = _split_top_level(color_function.group(1))
    if len(parts) not in {3, 4}:
        return None
    try:
        red = int(parts[0])
        green = int(parts[1])
        blue = int(parts[2])
        alpha_value = float(parts[3]) if len(parts) == 4 else 1.0
    except ValueError:
        return None
    if not all(0 <= channel <= 255 for channel in (red, green, blue)) or not 0.0 <= alpha_value <= 1.0:
        return None
    alpha = round(alpha_value * 255.0)
    return (red << 24) | (green << 16) | (blue << 8) | alpha


def _token(kind: str, number: float = 0.0, data: int = 0, text: str = "") -> dict[str, object]:
    return {"kind": kind, "number": number, "data": data, "text": text}


def _scalar_token(value: str) -> dict[str, object] | None:
    if value == "0":
        return _token("number", 0.0)
    if NUMBER.fullmatch(value):
        return _token("number", float(value))
    match = MEASURE.fullmatch(value)
    if match is None:
        return None
    unit_kinds = {"px": "logical_px", "%": "percent", "fr": "fraction", "deg": "angle_deg", "ms": "duration_ms"}
    return _token(unit_kinds[match.group(2)], float(match.group(1)))


def _space_tokens(value: str) -> list[dict[str, object]]:
    tokens: list[dict[str, object]] = []
    for part in _split_whitespace(value):
        scalar = _scalar_token(part)
        if scalar is not None:
            tokens.append(scalar)
            continue
        color = _packed_color(part)
        if color is not None:
            tokens.append(_token("color_rgba", data=color))
            continue
        tokens.append(_token("keyword", text=part.lower()))
    return tokens


def _gradient_layer(value: str) -> list[dict[str, object]] | None:
    match = re.fullmatch(
        r"(linear-gradient|radial-gradient|repeating-linear-gradient|repeating-radial-gradient)\((.*)\)",
        value,
        re.IGNORECASE,
    )
    if match is None:
        return None
    function_name = match.group(1).lower()
    parts = _split_top_level(match.group(2))
    tokens = [_token("keyword", text=function_name)]
    if function_name in {"linear-gradient", "repeating-linear-gradient"}:
        angle = _scalar_token(parts[0]) if parts else None
        if angle is not None and angle["kind"] == "angle_deg":
            tokens.append(angle)
            parts = parts[1:]
    else:
        # Bounded standard radial prelude: `ellipse RX RY at CX CY`.
        # Percent radii/centres map exactly to GUI.Forms' normalized ellipse.
        prelude = _split_whitespace(parts[0]) if parts else []
        if prelude and prelude[0].lower() == "ellipse":
            if len(prelude) != 6 or prelude[3].lower() != "at":
                return None
            geometry = [_scalar_token(item) for item in (prelude[1], prelude[2], prelude[4], prelude[5])]
            if any(item is None or item["kind"] != "percent" for item in geometry):
                return None
            tokens.extend(
                [_token("keyword", text="ellipse"), geometry[0], geometry[1],
                 _token("keyword", text="at"), geometry[2], geometry[3]]
            )
            parts = parts[1:]
    for part in parts:
        stop = _split_whitespace(part)
        if len(stop) not in {1, 2}:
            return None
        color = _packed_color(stop[0])
        if color is None:
            return None
        tokens.append(_token("color_rgba", data=color))
        if len(stop) == 2:
            position = _scalar_token(stop[1])
            if position is None or position["kind"] not in {"percent", "logical_px"}:
                return None
            tokens.append(position)
    if sum(1 for token in tokens if token["kind"] == "color_rgba") < 2:
        return None
    return tokens


def _gradient(value: str) -> dict[str, object] | None:
    layers = _split_top_level(value)
    if not layers or len(layers) > 7:
        return None
    tokens: list[dict[str, object]] = []
    for index, layer in enumerate(layers):
        parsed = _gradient_layer(layer)
        if parsed is None:
            return None
        if index:
            tokens.append(_token("separator"))
        tokens.extend(parsed)
    return {"kind": "gradient", "tokens": tokens}


def _transform(value: str) -> dict[str, object] | None:
    tokens: list[dict[str, object]] = []
    position = 0
    for match in FUNCTION.finditer(value):
        if value[position:match.start()].strip():
            return None
        tokens.append(_token("keyword", text=match.group(1).lower()))
        arguments = _split_top_level(match.group(2))
        for argument in arguments:
            scalar = _scalar_token(argument)
            if scalar is None:
                return None
            tokens.append(scalar)
        position = match.end()
    if value[position:].strip() or not tokens:
        return None
    return {"kind": "transform", "tokens": tokens}


def typed_value(name: str, value: str) -> dict[str, object]:
    color = _packed_color(value)
    if color is not None and name in {"color", "background", "background-color", "border-color", "outline-color"}:
        return {"kind": "color", "tokens": [_token("color_rgba", data=color)]}

    if name in {"background", "background-image"}:
        gradient = _gradient(value)
        if gradient is not None:
            return gradient
        if name == "background-image" and value == "none":
            return {"kind": "keyword", "tokens": [_token("keyword", text="none")]}

    if name == "font-family":
        families = [part.strip().strip("'\"") for part in _split_top_level(value)]
        if not families or any(not family for family in families):
            raise WebFormsError([Diagnostic("WFV001", "invalid font-family token list", "<computed-style>")])
        return {"kind": "font_family", "tokens": [_token("string", text=family) for family in families]}

    if name in {"border", "border-top", "border-right", "border-bottom", "border-left", "outline"}:
        tokens = _space_tokens(value)
        if len(tokens) < 2:
            raise WebFormsError([Diagnostic("WFV002", f"invalid typed {name}", "<computed-style>")])
        return {"kind": "border", "tokens": tokens}

    if name == "box-shadow":
        shadows = _split_top_level(value)
        tokens: list[dict[str, object]] = []
        for index, shadow in enumerate(shadows):
            if index:
                tokens.append(_token("separator"))
            tokens.extend(_space_tokens(shadow))
        return {"kind": "shadow_list", "tokens": tokens}

    if name == "transform":
        transformed = _transform(value)
        if transformed is None:
            raise WebFormsError([Diagnostic("WFV003", "invalid typed transform", "<computed-style>")])
        return transformed

    if name in {"margin", "padding", "border-radius", "grid-template-columns", "grid-template-rows"}:
        tokens = _space_tokens(value)
        if any(token["kind"] not in {"number", "logical_px", "percent", "fraction", "keyword"} for token in tokens):
            raise WebFormsError([Diagnostic("WFV004", f"invalid typed {name}", "<computed-style>")])
        kind = "track_list" if name.startswith("grid-template") else "edge_list"
        return {"kind": kind, "tokens": tokens}

    if name in {
        "width", "height", "min-width", "min-height", "max-width", "max-height",
        "top", "right", "bottom", "left", "margin-top", "margin-right",
        "margin-bottom", "margin-left", "padding-top", "padding-right",
        "padding-bottom", "padding-left", "gap", "row-gap", "column-gap",
        "font-size", "letter-spacing", "line-height", "outline-offset",
        "outline-width", "border-width", "flex-basis",
    }:
        scalar = _scalar_token(value)
        if scalar is not None:
            return {"kind": "scalar", "tokens": [scalar]}
        if value in {"auto", "none", "normal"}:
            return {"kind": "keyword", "tokens": [_token("keyword", text=value)]}
        raise WebFormsError([Diagnostic("WFV005", f"invalid typed scalar {name}: {value!r}", "<computed-style>")])

    if name in {"opacity", "flex", "flex-grow", "flex-shrink", "z-index", "grid-column", "grid-row"}:
        scalar = _scalar_token(value)
        if scalar is not None:
            return {"kind": "scalar", "tokens": [scalar]}

    if name in {"background-position", "background-size", "transform-origin"}:
        return {"kind": "position_list", "tokens": _space_tokens(value)}

    if name == "content":
        return {"kind": "string", "tokens": [_token("string", text=value[1:-1])]}

    if name in {
        "display", "position", "box-sizing", "flex-direction", "flex-wrap",
        "align-items", "align-content", "align-self", "justify-content",
        "justify-items", "justify-self", "overflow", "overflow-x", "overflow-y",
        "background-repeat", "border-style", "font-style", "font-weight",
        "text-align", "text-decoration", "text-overflow", "text-transform",
        "white-space", "word-break", "visibility", "cursor",
    }:
        scalar = _scalar_token(value)
        token = scalar if scalar is not None else _token("keyword", text=value.lower())
        return {"kind": "keyword", "tokens": [token]}

    if name == "filter":
        transformed = _transform(value)
        if transformed is not None:
            return {"kind": "filter", "tokens": transformed["tokens"]}

    raise WebFormsError([Diagnostic("WFV099", f"no typed lowering for {name}: {value!r}", "<computed-style>")])


def fnv1a_32(value: str) -> int:
    result = 2166136261
    for byte in value.encode("utf-8"):
        result ^= byte
        result = (result * 16777619) & 0xFFFFFFFF
    return result
