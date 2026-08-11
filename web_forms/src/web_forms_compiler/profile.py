from __future__ import annotations

import re


IR_SCHEMA = "web.forms.ir/0.1-experimental"
PROFILE = "web.forms.profile/0.1-dogfood"

MAX_SOURCE_BYTES = 256 * 1024
MAX_STYLESHEET_BYTES = 256 * 1024
MAX_NODES = 2048
MAX_DEPTH = 48
MAX_ATTRIBUTES_PER_NODE = 32
MAX_ATTRIBUTE_VALUE_BYTES = 1024
MAX_ID_BYTES = 160
MAX_TEXT_BYTES = 512 * 1024
MAX_RULES = 2048
MAX_SELECTORS_PER_RULE = 16
MAX_SELECTOR_SEGMENTS = 8
MAX_DECLARATIONS_PER_RULE = 64
MAX_VALUE_BYTES = 2048
MAX_CUSTOM_PROPERTIES = 256
MAX_VARIANTS = 4096

ID_PATTERN = re.compile(r"^[a-z][a-z0-9_-]*(?:\.[a-z][a-z0-9_-]*)*$")
CLASS_PATTERN = re.compile(r"^[a-z][a-z0-9_-]*$")

ALLOWED_ELEMENTS = frozenset(
    {
        "html", "head", "title", "meta", "link", "style", "body",
        "main", "header", "nav", "section", "article", "aside", "footer",
        "div", "span", "strong", "b", "em", "small", "mark", "br",
        "h1", "h2", "h3", "h4", "h5", "h6", "p", "dl", "dt", "dd",
        "ul", "ol", "li", "button", "input", "label", "textarea", "select",
        "option", "img", "template",
    }
)

VOID_ELEMENTS = frozenset({"meta", "link", "br", "img", "input"})
INTERACTIVE_ELEMENTS = frozenset({"button", "input", "textarea", "select"})

GLOBAL_ATTRIBUTES = frozenset(
    {
        "id", "class", "title", "role", "tabindex", "hidden", "lang", "dir",
        "disabled", "checked", "selected", "readonly", "required", "name",
        "type", "value", "placeholder", "for", "src", "alt", "width", "height",
        "rel", "href", "charset", "content",
    }
)

WF_ATTRIBUTES = frozenset(
    {
        "data-wf-control", "data-wf-style-exposure", "data-wf-command",
        "data-wf-owner", "data-wf-state", "data-wf-surface", "data-wf-field",
        "data-wf-item-template", "data-wf-preview-state", "data-wf-variant",
    }
)

CONTROL_KINDS = frozenset(
    {
        "panel", "stack", "toolbar", "breadcrumb", "tree-view", "list-view",
        "object-view", "status-bar", "split-view", "menu-bar", "text-box",
        "search-box", "button", "label", "image", "virtual-list",
    }
)

STATE_PSEUDOS = frozenset(
    {
        "hover", "active", "focus", "focus-visible", "disabled", "checked",
        "selected", "expanded", "drag-target", "unavailable",
    }
)
STRUCTURAL_PSEUDOS = frozenset({"root", "first-child", "last-child"})
PSEUDO_ELEMENTS = frozenset({"before", "after"})

INHERITED_PROPERTIES = frozenset(
    {
        "color", "font-family", "font-size", "font-style", "font-weight",
        "letter-spacing", "line-height", "text-align", "text-decoration",
        "text-transform", "visibility", "cursor",
    }
)

GEOMETRY_PROPERTIES = frozenset(
    {
        "display", "position", "box-sizing", "width", "height", "min-width",
        "min-height", "max-width", "max-height", "top", "right", "bottom",
        "left", "margin", "margin-top", "margin-right", "margin-bottom",
        "margin-left", "padding", "padding-top", "padding-right",
        "padding-bottom", "padding-left", "gap", "row-gap", "column-gap",
        "flex", "flex-grow", "flex-shrink", "flex-basis", "flex-direction",
        "flex-wrap", "align-items", "align-content", "align-self",
        "justify-content", "justify-items", "justify-self", "grid-template-columns",
        "grid-template-rows", "grid-column", "grid-row", "overflow",
        "overflow-x", "overflow-y", "z-index", "transform", "transform-origin",
    }
)

TYPOGRAPHY_PROPERTIES = frozenset(
    {
        "font-family", "font-size", "font-style", "font-weight",
        "letter-spacing", "line-height", "text-align", "text-decoration",
        "text-overflow", "text-transform", "white-space", "word-break",
    }
)

MATERIAL_PROPERTIES = frozenset(
    {
        "color", "background", "background-color", "background-image", "background-size",
        "background-position", "background-repeat", "border", "border-top",
        "border-right", "border-bottom", "border-left", "border-color",
        "border-style", "border-width", "border-radius", "box-shadow", "opacity",
        "outline", "outline-color", "outline-offset", "outline-style",
        "outline-width", "filter", "visibility", "cursor",
    }
)

DECORATION_PROPERTIES = frozenset({"content"})
ALL_PROPERTIES = GEOMETRY_PROPERTIES | TYPOGRAPHY_PROPERTIES | MATERIAL_PROPERTIES | DECORATION_PROPERTIES

ENUM_VALUES = {
    "display": {"block", "inline", "inline-block", "flex", "grid", "none"},
    "position": {"static", "relative", "absolute"},
    "box-sizing": {"border-box", "content-box"},
    "flex-direction": {"row", "row-reverse", "column", "column-reverse"},
    "flex-wrap": {"nowrap", "wrap", "wrap-reverse"},
    "align-items": {"stretch", "flex-start", "flex-end", "center", "baseline"},
    "align-content": {"stretch", "flex-start", "flex-end", "center", "space-between", "space-around"},
    "align-self": {"auto", "stretch", "flex-start", "flex-end", "center", "baseline"},
    "justify-content": {"flex-start", "flex-end", "center", "space-between", "space-around", "space-evenly"},
    "justify-items": {"stretch", "start", "end", "center"},
    "justify-self": {"auto", "stretch", "start", "end", "center"},
    "overflow": {"visible", "hidden", "clip", "auto", "scroll"},
    "overflow-x": {"visible", "hidden", "clip", "auto", "scroll"},
    "overflow-y": {"visible", "hidden", "clip", "auto", "scroll"},
    "background-repeat": {"repeat", "repeat-x", "repeat-y", "no-repeat"},
    "border-style": {"none", "solid", "dashed", "dotted", "double", "inset", "outset"},
    "font-style": {"normal", "italic", "oblique"},
    "text-align": {"start", "end", "left", "right", "center", "justify"},
    "text-decoration": {"none", "underline", "line-through", "overline"},
    "text-overflow": {"clip", "ellipsis"},
    "text-transform": {"none", "uppercase", "lowercase", "capitalize"},
    "white-space": {"normal", "nowrap", "pre", "pre-wrap"},
    "word-break": {"normal", "break-all", "keep-all", "break-word"},
    "visibility": {"visible", "hidden"},
}

SAFE_VALUE_PATTERN = re.compile(r"^[\w\s#.,%+\-*/()'\":]+$")
LENGTH_TOKEN_PATTERN = re.compile(r"^-?(?:0|(?:\d+(?:\.\d+)?)(?:px|%|fr|deg|ms)?)$")
COLOR_PATTERN = re.compile(
    r"^(?:#[0-9a-fA-F]{3,8}|transparent|currentcolor|[a-zA-Z][a-zA-Z0-9-]*|rgba?\([^;]+\)|hsla?\([^;]+\))$"
)
ALLOWED_FUNCTIONS = frozenset(
    {
        "rgb", "rgba", "hsl", "hsla", "linear-gradient", "radial-gradient",
        "repeating-linear-gradient", "repeating-radial-gradient", "translate",
        "translatex", "translatey", "scale", "scalex", "scaley", "rotate",
        "blur", "brightness", "contrast", "grayscale", "saturate",
    }
)
ALLOWED_UNITS = frozenset({"px", "%", "fr", "deg", "ms"})


def property_domain(name: str) -> str:
    if name in DECORATION_PROPERTIES:
        return "decoration"
    if name in GEOMETRY_PROPERTIES:
        return "geometry"
    if name in TYPOGRAPHY_PROPERTIES:
        return "typography"
    return "material"


def validate_property_value(name: str, value: str) -> str | None:
    lowered = value.lower()
    if len(value.encode("utf-8")) > MAX_VALUE_BYTES:
        return "value exceeds profile byte limit"
    if any(token in lowered for token in ("url(", "expression(", "javascript:", "@import", "var(--", "!important")):
        return "executable, remote, unresolved, or priority syntax is forbidden"
    if not SAFE_VALUE_PATTERN.fullmatch(value):
        return "value contains a character outside the bounded grammar"
    for function in re.findall(r"([a-zA-Z][a-zA-Z-]*)\s*\(", value):
        if function.lower() not in ALLOWED_FUNCTIONS:
            return f"function {function}() is outside the bounded grammar"
    unit_source = re.sub(r"#[0-9a-fA-F]{3,8}", "", value)
    for unit in re.findall(r"(?<![A-Za-z0-9_#])\d+(?:\.\d+)?([a-zA-Z%]+)\b", unit_source):
        if unit.lower() not in ALLOWED_UNITS:
            return f"unit {unit!r} is outside the bounded grammar"
    allowed = ENUM_VALUES.get(name)
    if allowed is not None and lowered not in allowed:
        return f"expected one of: {', '.join(sorted(allowed))}"
    if name == "content":
        if len(value) < 2 or value[0] not in "\"'" or value[-1] != value[0]:
            return "generated content must be one quoted literal"
    if name in {"opacity", "flex-grow", "flex-shrink"}:
        try:
            number = float(value)
        except ValueError:
            return "expected a finite number"
        if not (0.0 <= number <= 1000.0):
            return "number is outside the bounded range"
    if name in {"color", "background-color", "border-color", "outline-color"} and not COLOR_PATTERN.fullmatch(value):
        return "expected a bounded color literal"
    if name == "background" and not COLOR_PATTERN.fullmatch(value):
        return "the bounded background shorthand accepts only a solid color; use background-image separately"
    return None


def profile_manifest() -> dict[str, object]:
    return {
        "schema": "web.forms.source-profile/0.1-experimental",
        "profile": PROFILE,
        "elements": sorted(ALLOWED_ELEMENTS),
        "control_kinds": sorted(CONTROL_KINDS),
        "properties": sorted(ALL_PROPERTIES),
        "state_pseudo_classes": sorted(STATE_PSEUDOS),
        "structural_pseudo_classes": sorted(STRUCTURAL_PSEUDOS),
        "pseudo_elements": sorted(PSEUDO_ELEMENTS),
        "font_face_profile": {
            "at_rule": "@font-face",
            "source": "bundled-gui.forms-font-pack-only",
            "families": ["Carlito", "Cousine", "Portsmouth Rapids"],
            "maximum_faces_per_stylesheet": 8,
        },
        "attributes": sorted(GLOBAL_ATTRIBUTES | WF_ATTRIBUTES),
        "limits": {
            "source_bytes": MAX_SOURCE_BYTES,
            "stylesheet_bytes": MAX_STYLESHEET_BYTES,
            "static_nodes": MAX_NODES,
            "tree_depth": MAX_DEPTH,
            "attributes_per_node": MAX_ATTRIBUTES_PER_NODE,
            "attribute_value_bytes": MAX_ATTRIBUTE_VALUE_BYTES,
            "id_bytes": MAX_ID_BYTES,
            "text_bytes": MAX_TEXT_BYTES,
            "rules": MAX_RULES,
            "selectors_per_rule": MAX_SELECTORS_PER_RULE,
            "selector_segments": MAX_SELECTOR_SEGMENTS,
            "declarations_per_rule": MAX_DECLARATIONS_PER_RULE,
            "value_bytes": MAX_VALUE_BYTES,
            "custom_properties": MAX_CUSTOM_PROPERTIES,
            "state_and_decoration_variants": MAX_VARIANTS,
        },
        "hard_exclusions": [
            "javascript", "inline-event-handlers", "inline-style-attributes",
            "remote-resources", "runtime-dom", "runtime-css-parser", "unknown-syntax",
        ],
    }
