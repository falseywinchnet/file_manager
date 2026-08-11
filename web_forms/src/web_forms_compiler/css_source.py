from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re

from .diagnostics import Diagnostic, WebFormsError
from .html_source import HtmlDocument, HtmlNode
from .value_types import typed_value
from .profile import (
    ALLOWED_ELEMENTS, ALL_PROPERTIES, INHERITED_PROPERTIES, MAX_CUSTOM_PROPERTIES,
    MAX_DECLARATIONS_PER_RULE, MAX_RULES, MAX_SELECTOR_SEGMENTS,
    MAX_SELECTORS_PER_RULE, MAX_STYLESHEET_BYTES, MAX_VARIANTS, PSEUDO_ELEMENTS,
    STATE_PSEUDOS, STRUCTURAL_PSEUDOS, property_domain,
    validate_property_value,
)


@dataclass(frozen=True)
class Declaration:
    name: str
    value: str
    source: str
    line: int
    order: int


@dataclass(frozen=True)
class CompoundSelector:
    tag: str | None
    stable_id: str | None
    classes: tuple[str, ...]
    attributes: tuple[tuple[str, str], ...]
    states: tuple[str, ...]
    structural: tuple[str, ...]
    pseudo_element: str | None


@dataclass(frozen=True)
class Selector:
    source: str
    compounds: tuple[CompoundSelector, ...]
    combinators: tuple[str, ...]
    specificity: tuple[int, int, int]

    @property
    def states(self) -> tuple[str, ...]:
        return self.compounds[-1].states

    @property
    def pseudo_element(self) -> str | None:
        return self.compounds[-1].pseudo_element


@dataclass(frozen=True)
class Rule:
    selector: Selector
    declarations: tuple[Declaration, ...]
    order: int


COMMENT_PATTERN = re.compile(r"/\*.*?\*/", re.DOTALL)
FONT_FACE_PATTERN = re.compile(r"@font-face\s*\{([^{}]*)\}", re.IGNORECASE | re.DOTALL)
FONT_SOURCE_PATTERN = re.compile(
    r'^url\(["\']\.\./\.\./\.\./\.\./gui_forms/assets/fonts/'
    r'([A-Za-z0-9-]+\.ttf)["\']\)\s+format\(["\']truetype["\']\)$',
    re.IGNORECASE,
)
ADMITTED_FONT_FACES = {
    ("Carlito-Regular.ttf", "Carlito", "400", "normal"),
    ("Carlito-Bold.ttf", "Carlito", "700", "normal"),
    ("Carlito-Italic.ttf", "Carlito", "400", "italic"),
    ("Carlito-BoldItalic.ttf", "Carlito", "700", "italic"),
    ("Cousine-Regular.ttf", "Cousine", "400", "normal"),
    ("Cousine-Bold.ttf", "Cousine", "700", "normal"),
    ("PortsmouthRapids.ttf", "Portsmouth Rapids", "400", "normal"),
    ("PortsmouthRapids-Bold.ttf", "Portsmouth Rapids", "700", "normal"),
}
TOKEN_PATTERN = re.compile(
    r"^(?:"
    r"(?P<tag>[a-z][a-z0-9-]*|\*)|"
    r"#(?P<id>[a-z][a-z0-9_-]*)|"
    r"\.(?P<class>[a-z][a-z0-9_-]*)|"
    r"\[(?P<attr>[a-z][a-z0-9_-]*)=(?P<quote>['\"])(?P<attr_value>[a-zA-Z0-9_.-]+)(?P=quote)\]|"
    r"::(?P<pseudo_element>[a-z-]+)|"
    r":(?P<pseudo>[a-z-]+)"
    r")"
)


def _strip_comments(text: str) -> str:
    def preserve_lines(match: re.Match[str]) -> str:
        return "\n" * match.group(0).count("\n")
    return COMMENT_PATTERN.sub(preserve_lines, text)


def _split_top_level(text: str, separator: str) -> list[str]:
    result: list[str] = []
    start = 0
    depth = 0
    quote: str | None = None
    escape = False
    for index, char in enumerate(text):
        if escape:
            escape = False
            continue
        if char == "\\" and quote is not None:
            escape = True
            continue
        if quote is not None:
            if char == quote:
                quote = None
            continue
        if char in "'\"":
            quote = char
        elif char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
        elif char == separator and depth == 0:
            result.append(text[start:index])
            start = index + 1
    result.append(text[start:])
    return result


def _strip_admitted_font_faces(source: str, path: str) -> str:
    count = 0

    def admit(match: re.Match[str]) -> str:
        nonlocal count
        count += 1
        if count > 8:
            raise WebFormsError(
                [Diagnostic("WFC008", "font-face limit exceeded", path)]
            )
        declarations: dict[str, str] = {}
        for raw in _split_top_level(match.group(1), ";"):
            raw = raw.strip()
            if not raw:
                continue
            if ":" not in raw:
                raise WebFormsError(
                    [Diagnostic("WFC009", "invalid bounded @font-face declaration", path)]
                )
            name, value = raw.split(":", 1)
            name = name.strip().lower()
            if name in declarations or name not in {
                "font-family", "src", "font-weight", "font-style"
            }:
                raise WebFormsError(
                    [Diagnostic("WFC009", "invalid bounded @font-face declaration", path)]
                )
            declarations[name] = value.strip()
        if set(declarations) != {"font-family", "src", "font-weight", "font-style"}:
            raise WebFormsError(
                [Diagnostic("WFC009", "bounded @font-face requires family, src, weight, and style", path)]
            )
        family = declarations["font-family"]
        if len(family) < 2 or family[0] not in "\"'" or family[-1] != family[0]:
            raise WebFormsError(
                [Diagnostic("WFC009", "bounded font family must be quoted", path)]
            )
        source_match = FONT_SOURCE_PATTERN.fullmatch(declarations["src"])
        signature = (
            source_match.group(1) if source_match is not None else "",
            family[1:-1],
            declarations["font-weight"],
            declarations["font-style"].lower(),
        )
        if signature not in ADMITTED_FONT_FACES:
            raise WebFormsError(
                [Diagnostic("WFC009", "@font-face is outside the bundled GUI.Forms font pack", path)]
            )
        return "\n" * match.group(0).count("\n")

    stripped = FONT_FACE_PATTERN.sub(admit, source)
    if "@" in stripped:
        raise WebFormsError(
            [Diagnostic("WFC002", "only bundled-font @font-face rules are admitted", path)]
        )
    return stripped


def _selector_segments(source: str, path: str, line: int) -> tuple[list[str], list[str]]:
    normalized = re.sub(r"\s*>\s*", ">", source.strip())
    if not normalized:
        raise WebFormsError([Diagnostic("WFC010", "empty selector", path, line)])
    segments: list[str] = []
    combinators: list[str] = []
    current: list[str] = []
    bracket = 0
    index = 0
    while index < len(normalized):
        char = normalized[index]
        if char == "[":
            bracket += 1
            current.append(char)
        elif char == "]":
            bracket -= 1
            current.append(char)
        elif bracket == 0 and char == ">":
            if not current:
                raise WebFormsError([Diagnostic("WFC011", "invalid child combinator", path, line)])
            segments.append("".join(current))
            current = []
            combinators.append(">")
        elif bracket == 0 and char.isspace():
            if current:
                segments.append("".join(current))
                current = []
                combinators.append(" ")
            while index + 1 < len(normalized) and normalized[index + 1].isspace():
                index += 1
        else:
            current.append(char)
        index += 1
    if current:
        segments.append("".join(current))
    if len(combinators) >= len(segments):
        raise WebFormsError([Diagnostic("WFC012", "selector ends in a combinator", path, line)])
    if len(segments) > MAX_SELECTOR_SEGMENTS:
        raise WebFormsError([Diagnostic("WFC013", "selector segment limit exceeded", path, line)])
    return segments, combinators


def parse_selector(source: str, path: str, line: int) -> Selector:
    segments, combinators = _selector_segments(source, path, line)
    compounds: list[CompoundSelector] = []
    specificity = [0, 0, 0]
    for segment_index, segment in enumerate(segments):
        remaining = segment
        tag: str | None = None
        stable_id: str | None = None
        classes: list[str] = []
        attributes: list[tuple[str, str]] = []
        states: list[str] = []
        structural: list[str] = []
        pseudo_element: str | None = None
        while remaining:
            match = TOKEN_PATTERN.match(remaining)
            if match is None:
                raise WebFormsError([Diagnostic("WFC014", f"unsupported selector syntax near {remaining!r}", path, line)])
            token = match.groupdict()
            if token["tag"] is not None:
                if tag is not None:
                    raise WebFormsError([Diagnostic("WFC015", "compound selector has multiple types", path, line)])
                tag = token["tag"]
                if tag != "*" and tag not in ALLOWED_ELEMENTS:
                    raise WebFormsError([Diagnostic("WFC019", f"selector element <{tag}> is outside the profile", path, line)])
                if tag != "*":
                    specificity[2] += 1
            elif token["id"] is not None:
                stable_id = token["id"]
                specificity[0] += 1
            elif token["class"] is not None:
                classes.append(token["class"])
                specificity[1] += 1
            elif token["attr"] is not None:
                attributes.append((token["attr"], token["attr_value"] or ""))
                specificity[1] += 1
            elif token["pseudo_element"] is not None:
                pseudo_element = token["pseudo_element"]
                if pseudo_element not in PSEUDO_ELEMENTS:
                    raise WebFormsError([Diagnostic("WFC016", f"unsupported pseudo-element ::{pseudo_element}", path, line)])
                specificity[2] += 1
            else:
                pseudo = token["pseudo"] or ""
                if pseudo in STATE_PSEUDOS:
                    states.append(pseudo)
                elif pseudo in STRUCTURAL_PSEUDOS:
                    structural.append(pseudo)
                else:
                    raise WebFormsError([Diagnostic("WFC017", f"unsupported pseudo-class :{pseudo}", path, line)])
                specificity[1] += 1
            remaining = remaining[match.end():]
        if (states or pseudo_element is not None) and segment_index != len(segments) - 1:
            raise WebFormsError([Diagnostic("WFC018", "state and pseudo-element selectors are final-segment only", path, line)])
        compounds.append(
            CompoundSelector(tag, stable_id, tuple(classes), tuple(attributes), tuple(sorted(states)), tuple(structural), pseudo_element)
        )
    return Selector(source.strip(), tuple(compounds), tuple(combinators), tuple(specificity))


def parse_css(text: str, path: str, order_base: int = 0) -> list[Rule]:
    encoded = text.encode("utf-8")
    if len(encoded) > MAX_STYLESHEET_BYTES:
        raise WebFormsError([Diagnostic("WFC001", "stylesheet byte limit exceeded", path)])
    source = _strip_comments(text)
    source = _strip_admitted_font_faces(source, path)
    rules: list[Rule] = []
    position = 0
    rule_order = order_base
    while position < len(source):
        while position < len(source) and source[position].isspace():
            position += 1
        if position == len(source):
            break
        open_brace = source.find("{", position)
        if open_brace < 0:
            raise WebFormsError([Diagnostic("WFC003", "unterminated selector", path, source.count("\n", 0, position) + 1)])
        selector_text = source[position:open_brace].strip()
        depth = 1
        close_brace = open_brace + 1
        while close_brace < len(source) and depth:
            if source[close_brace] == "{":
                depth += 1
            elif source[close_brace] == "}":
                depth -= 1
            close_brace += 1
        if depth:
            raise WebFormsError([Diagnostic("WFC004", "unterminated declaration block", path, source.count("\n", 0, open_brace) + 1)])
        body = source[open_brace + 1:close_brace - 1]
        line = source.count("\n", 0, position) + 1
        selectors = [item.strip() for item in _split_top_level(selector_text, ",") if item.strip()]
        if len(selectors) > MAX_SELECTORS_PER_RULE:
            raise WebFormsError([Diagnostic("WFC005", "selectors-per-rule limit exceeded", path, line)])
        raw_declarations = [item.strip() for item in _split_top_level(body, ";") if item.strip()]
        if len(raw_declarations) > MAX_DECLARATIONS_PER_RULE:
            raise WebFormsError([Diagnostic("WFC006", "declarations-per-rule limit exceeded", path, line)])
        declarations: list[Declaration] = []
        for declaration_order, raw in enumerate(raw_declarations):
            if ":" not in raw:
                raise WebFormsError([Diagnostic("WFC020", f"declaration lacks ':' in {raw!r}", path, line)])
            name, value = raw.split(":", 1)
            name = name.strip().lower()
            value = value.strip()
            if not name or not value:
                raise WebFormsError([Diagnostic("WFC021", "empty property name or value", path, line)])
            if not name.startswith("--") and name not in ALL_PROPERTIES:
                raise WebFormsError([Diagnostic("WFC022", f"unknown property {name!r}", path, line)])
            if name.startswith("--"):
                lowered_value = value.lower()
                if len(value.encode("utf-8")) > 2048 or any(token in lowered_value for token in ("url(", "expression(", "javascript:", "@import", "!important")):
                    raise WebFormsError([Diagnostic("WFC024", "custom property value is outside the bounded grammar", path, line)])
            if not name.startswith("--") and "var(" not in value:
                problem = validate_property_value(name, value)
                if problem is not None:
                    raise WebFormsError([Diagnostic("WFC023", f"invalid {name}: {problem}", path, line)])
            declarations.append(Declaration(name, value, path, line, declaration_order))
        for selector_source in selectors:
            selector = parse_selector(selector_source, path, line)
            rules.append(Rule(selector, tuple(declarations), rule_order))
            rule_order += 1
            if len(rules) > MAX_RULES:
                raise WebFormsError([Diagnostic("WFC007", "stylesheet rule limit exceeded", path, line)])
        position = close_brace
    return rules


def _previous_sibling(document: HtmlDocument, node: HtmlNode) -> HtmlNode | None:
    siblings = [item for item in document.nodes if item.parent == node.parent]
    index = siblings.index(node)
    return siblings[index - 1] if index > 0 else None


def _next_sibling(document: HtmlDocument, node: HtmlNode) -> HtmlNode | None:
    siblings = [item for item in document.nodes if item.parent == node.parent]
    index = siblings.index(node)
    return siblings[index + 1] if index + 1 < len(siblings) else None


def _compound_matches(document: HtmlDocument, node: HtmlNode, compound: CompoundSelector) -> bool:
    if compound.tag not in (None, "*") and node.tag != compound.tag:
        return False
    if compound.stable_id is not None and node.attributes.get("id") != compound.stable_id:
        return False
    node_classes = set(node.attributes.get("class", "").split())
    if not set(compound.classes).issubset(node_classes):
        return False
    for name, value in compound.attributes:
        if node.attributes.get(name) != value:
            return False
    for structural in compound.structural:
        if structural == "root" and node.tag != "html":
            return False
        if structural == "first-child" and _previous_sibling(document, node) is not None:
            return False
        if structural == "last-child" and _next_sibling(document, node) is not None:
            return False
    return True


def selector_matches(document: HtmlDocument, node: HtmlNode, selector: Selector) -> bool:
    compounds = selector.compounds
    if not _compound_matches(document, node, compounds[-1]):
        return False
    current = node
    for compound_index in range(len(compounds) - 2, -1, -1):
        combinator = selector.combinators[compound_index]
        if combinator == ">":
            if current.parent is None:
                return False
            current = document.nodes[current.parent]
            if not _compound_matches(document, current, compounds[compound_index]):
                return False
        else:
            parent_index = current.parent
            found: HtmlNode | None = None
            while parent_index is not None:
                parent = document.nodes[parent_index]
                if _compound_matches(document, parent, compounds[compound_index]):
                    found = parent
                    break
                parent_index = parent.parent
            if found is None:
                return False
            current = found
    return True


VAR_PATTERN = re.compile(r"var\(\s*(--[a-z][a-z0-9-]*)\s*(?:,\s*([^()]+))?\)")


def resolve_value(value: str, custom: dict[str, str], path: str, line: int) -> str:
    current = value
    for _ in range(16):
        match = VAR_PATTERN.search(current)
        if match is None:
            if "var(" in current:
                raise WebFormsError([Diagnostic("WFC030", "invalid or nested var() expression", path, line)])
            return current
        name = match.group(1)
        replacement = custom.get(name)
        if replacement is None:
            replacement = (match.group(2) or "").strip()
        if not replacement:
            raise WebFormsError([Diagnostic("WFC031", f"custom property {name!r} has no value or fallback", path, line)])
        current = current[:match.start()] + replacement + current[match.end():]
    raise WebFormsError([Diagnostic("WFC032", "custom property expansion limit exceeded", path, line)])


def _apply_computed_declaration(values: dict[str, str], name: str,
                                value: str) -> tuple[str, ...]:
    if name != "background":
        values[name] = value
        return (name,)
    values["background-color"] = value
    values["background-image"] = "none"
    values.pop("background-position", None)
    values.pop("background-repeat", None)
    values.pop("background-size", None)
    return ("background-color", "background-image")


def compute_styles(document: HtmlDocument, rules: list[Rule]) -> dict[str, object]:
    pool_values: dict[str, list[tuple[tuple[str, str], ...]]] = {
        "geometry": [tuple()], "typography": [tuple()], "material": [tuple()]
    }
    pool_indices: dict[str, dict[tuple[tuple[str, str], ...], int]] = {
        name: {tuple(): 0} for name in pool_values
    }

    def intern(domain: str, values: dict[str, str]) -> int:
        key = tuple(sorted(values.items()))
        existing = pool_indices[domain].get(key)
        if existing is not None:
            return existing
        index = len(pool_values[domain])
        pool_values[domain].append(key)
        pool_indices[domain][key] = index
        return index

    computed_by_node: list[dict[str, str]] = []
    custom_by_node: list[dict[str, str]] = []
    node_styles: list[dict[str, object]] = []
    variants: list[dict[str, object]] = []
    decorations: list[dict[str, object]] = []
    custom_names: set[str] = set()

    for node in document.nodes:
        parent_values = computed_by_node[node.parent] if node.parent is not None else {}
        values = {name: value for name, value in parent_values.items() if name in INHERITED_PROPERTIES}
        custom = dict(custom_by_node[node.parent]) if node.parent is not None else {}
        base_rules = [
            rule for rule in rules
            if not rule.selector.states and rule.selector.pseudo_element is None and selector_matches(document, node, rule.selector)
        ]
        base_rules.sort(key=lambda item: (item.selector.specificity, item.order))
        provenance: dict[str, str] = {}
        for rule in base_rules:
            for declaration in rule.declarations:
                if declaration.name.startswith("--"):
                    custom[declaration.name] = declaration.value
                    custom_names.add(declaration.name)
        for rule in base_rules:
            for declaration in rule.declarations:
                if not declaration.name.startswith("--"):
                    resolved = resolve_value(declaration.value, custom, declaration.source, declaration.line)
                    problem = validate_property_value(declaration.name, resolved)
                    if problem is not None:
                        raise WebFormsError([Diagnostic("WFC033", f"invalid resolved {declaration.name}: {problem}", declaration.source, declaration.line)])
                    applied = _apply_computed_declaration(
                        values, declaration.name, resolved)
                    for name in applied:
                        provenance[name] = (
                            f"{declaration.source}:{declaration.line}:"
                            f"{rule.selector.source}"
                        )
        if len(custom_names) > MAX_CUSTOM_PROPERTIES:
            raise WebFormsError([Diagnostic("WFC034", "custom property count limit exceeded", str(document.source_path))])
        computed_by_node.append(values)
        custom_by_node.append(custom)
        domains = {"geometry": {}, "typography": {}, "material": {}}
        for name, value in values.items():
            domain = property_domain(name)
            if domain in domains:
                domains[domain][name] = value
        gradient_value = values.get("background-image", "")
        if "gradient(" in gradient_value and "background-color" not in values:
            raise WebFormsError([
                Diagnostic("WFC037", "gradient material requires an authored background-color fallback", str(document.source_path), node.line, node.column)
            ])
        surface = node.attributes.get("data-wf-surface")
        if surface is None:
            surface = "own-surface" if node.tag == "body" else "reveal-parent"
        if any(name.startswith("background") for name in domains["material"]) and surface == "reveal-parent":
            raise WebFormsError([
                Diagnostic("WFC035", "a node painting background material must declare data-wf-surface='own-surface' or 'baked-into-parent'", str(document.source_path), node.line, node.column)
            ])
        node_styles.append(
            {
                "node": node.index,
                "geometry": intern("geometry", domains["geometry"]),
                "typography": intern("typography", domains["typography"]),
                "material": intern("material", domains["material"]),
                "surface": surface,
                "exposure": node.attributes.get("data-wf-style-exposure", "baked"),
                "provenance": provenance,
            }
        )

        variant_groups: dict[tuple[tuple[str, ...], str | None], list[Rule]] = {}
        for rule in rules:
            if (rule.selector.states or rule.selector.pseudo_element is not None) and selector_matches(document, node, rule.selector):
                key = (rule.selector.states, rule.selector.pseudo_element)
                variant_groups.setdefault(key, []).append(rule)
        for (states, pseudo), matching in sorted(variant_groups.items(), key=lambda item: (item[0][1] or "", item[0][0])):
            matching.sort(key=lambda item: (item.selector.specificity, item.order))
            delta = {"geometry": {}, "typography": {}, "material": {}}
            content = ""
            variant_custom = dict(custom)
            for rule in matching:
                for declaration in rule.declarations:
                    if declaration.name.startswith("--"):
                        variant_custom[declaration.name] = declaration.value
            for rule in matching:
                for declaration in rule.declarations:
                    if declaration.name.startswith("--"):
                        continue
                    resolved = resolve_value(declaration.value, variant_custom, declaration.source, declaration.line)
                    if declaration.name == "content":
                        content = resolved[1:-1]
                        continue
                    domain = property_domain(declaration.name)
                    _apply_computed_declaration(
                        delta[domain], declaration.name, resolved)
            variant_gradient = delta["material"].get("background-image", "")
            if "gradient(" in variant_gradient and "background-color" not in delta["material"] and "background-color" not in values:
                raise WebFormsError([
                    Diagnostic("WFC037", "gradient state material requires a base or state background-color fallback", str(document.source_path), node.line, node.column)
                ])
            record = {
                "node": node.index,
                "states": list(states),
                "pseudo": pseudo,
                "content": content,
                "geometry": intern("geometry", delta["geometry"]),
                "typography": intern("typography", delta["typography"]),
                "material": intern("material", delta["material"]),
            }
            if pseudo is None:
                variants.append(record)
            else:
                decorations.append(record)
            if len(variants) + len(decorations) > MAX_VARIANTS:
                raise WebFormsError([Diagnostic("WFC036", "state/decorative variant limit exceeded", str(document.source_path))])

    pools = {
        domain: [
            {"properties": [{"name": name, "value": value, "typed": typed_value(name, value)} for name, value in record]}
            for record in records
        ]
        for domain, records in pool_values.items()
    }
    return {
        "pools": pools,
        "nodes": node_styles,
        "variants": variants,
        "decorations": decorations,
    }
