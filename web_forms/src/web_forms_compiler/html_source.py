from __future__ import annotations

from dataclasses import dataclass, field
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import urlparse

from .diagnostics import Diagnostic, WebFormsError
from .keylines import parse_keylines
from .profile import (
    ALLOWED_ELEMENTS, CLASS_PATTERN, CONTROL_KINDS, GLOBAL_ATTRIBUTES,
    ID_PATTERN, INTERACTIVE_ELEMENTS, MAX_ATTRIBUTES_PER_NODE,
    MAX_ATTRIBUTE_VALUE_BYTES, MAX_DEPTH, MAX_ID_BYTES, MAX_NODES,
    MAX_SOURCE_BYTES, MAX_TEXT_BYTES, VOID_ELEMENTS, WF_ATTRIBUTES,
)


@dataclass
class HtmlNode:
    index: int
    parent: int | None
    tag: str
    attributes: dict[str, str]
    line: int
    column: int
    text: list[str] = field(default_factory=list)


@dataclass
class HtmlDocument:
    source_path: Path
    nodes: list[HtmlNode]
    title: str
    style_links: list[str]
    inline_styles: list[str]


class BoundedHtmlParser(HTMLParser):
    def __init__(self, path: Path):
        super().__init__(convert_charrefs=True)
        self.path = path
        self.nodes: list[HtmlNode] = []
        self.stack: list[int] = []
        self.diagnostics: list[Diagnostic] = []
        self.ids: dict[str, int] = {}
        self.style_links: list[str] = []
        self.inline_styles: list[str] = []
        self._style_chunks: list[str] | None = None
        self._title_chunks: list[str] | None = None
        self.title = ""
        self.seen_doctype = False
        self.ignored_tags: list[str] = []

    def location(self) -> tuple[int, int]:
        line, column = self.getpos()
        return line, column + 1

    def issue(self, code: str, message: str, line: int | None = None, column: int | None = None) -> None:
        if len(self.diagnostics) >= 100:
            return
        at_line, at_column = self.location()
        self.diagnostics.append(Diagnostic(code, message, str(self.path), line or at_line, column or at_column))

    def handle_decl(self, decl: str) -> None:
        if decl.strip().lower() != "doctype html" or self.seen_doctype:
            self.issue("WFH001", "only one <!doctype html> declaration is admitted")
        self.seen_doctype = True

    def unknown_decl(self, data: str) -> None:
        self.issue("WFH002", "unknown declarations are forbidden")

    def handle_pi(self, data: str) -> None:
        self.issue("WFH003", "processing instructions are forbidden")

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        self._open(tag.lower(), attrs, False)

    def handle_startendtag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        self._open(tag.lower(), attrs, True)

    def _open(self, tag: str, attrs: list[tuple[str, str | None]], self_closing: bool) -> None:
        line, column = self.location()
        if self.ignored_tags:
            if not self_closing:
                self.ignored_tags.append(tag)
            return
        if tag not in ALLOWED_ELEMENTS:
            code = "WFH027" if tag in {"script", "noscript"} else "WFH010"
            message = "JavaScript and script containers are forbidden" if code == "WFH027" else f"element <{tag}> subtree is outside the profile"
            self.issue(code, message)
            if not self_closing:
                self.ignored_tags.append(tag)
            return
        if len(self.nodes) >= MAX_NODES:
            self.issue("WFH011", "static node limit exceeded")
            return
        if len(self.stack) >= MAX_DEPTH:
            self.issue("WFH012", "tree depth limit exceeded")
            return
        if len(attrs) > MAX_ATTRIBUTES_PER_NODE:
            self.issue("WFH013", "attribute count limit exceeded")

        attributes: dict[str, str] = {}
        for raw_name, raw_value in attrs:
            name = raw_name.lower()
            value = "" if raw_value is None else raw_value
            if name in attributes:
                self.issue("WFH014", f"duplicate attribute {name!r}")
                continue
            if name.startswith("on"):
                self.issue("WFH015", f"event attribute {name!r} is forbidden")
            elif name == "style":
                self.issue("WFH016", "inline style attributes are forbidden; attach CSS")
            elif name.startswith("aria-"):
                pass
            elif name not in GLOBAL_ATTRIBUTES and name not in WF_ATTRIBUTES:
                self.issue("WFH017", f"attribute {name!r} is outside the profile")
            if len(value.encode("utf-8")) > MAX_ATTRIBUTE_VALUE_BYTES:
                self.issue("WFH018", f"attribute {name!r} exceeds its byte limit")
            attributes[name] = value

        stable_id = attributes.get("id")
        if stable_id is not None:
            if len(stable_id.encode("utf-8")) > MAX_ID_BYTES or not ID_PATTERN.fullmatch(stable_id):
                self.issue("WFH020", f"invalid stable id {stable_id!r}")
            elif stable_id in self.ids:
                self.issue("WFH021", f"duplicate stable id {stable_id!r}")
            else:
                self.ids[stable_id] = len(self.nodes)

        for class_name in attributes.get("class", "").split():
            if not CLASS_PATTERN.fullmatch(class_name):
                self.issue("WFH022", f"invalid class token {class_name!r}")

        control_kind = attributes.get("data-wf-control")
        if control_kind is not None and control_kind not in CONTROL_KINDS:
            self.issue("WFH023", f"unknown control kind {control_kind!r}")
        exposure = attributes.get("data-wf-style-exposure", "baked")
        if exposure not in {"baked", "exposed"}:
            self.issue("WFH024", "style exposure must be 'baked' or 'exposed'")
        surface = attributes.get("data-wf-surface")
        if surface is not None and surface not in {"reveal-parent", "own-surface", "baked-into-parent"}:
            self.issue("WFH025", "invalid surface ownership mode")
        dropdown_mode = attributes.get("data-wf-dropdown-mode")
        if dropdown_mode is not None and (
            control_kind != "dropdown-button" or dropdown_mode not in {"menu", "split"}
        ):
            self.issue("WFH039", "dropdown mode requires dropdown-button and must be menu or split")
        if "data-wf-dropdown-width" in attributes and control_kind != "dropdown-button":
            self.issue("WFH056", "dropdown width requires dropdown-button")
        overflow_actuator = attributes.get("data-wf-overflow-actuator")
        if overflow_actuator is not None and overflow_actuator != "true":
            self.issue("WFH040", "overflow actuator must be the literal true")
        drag_region = attributes.get("data-wf-window-drag-region")
        if drag_region is not None and drag_region != "true":
            self.issue("WFH041", "window drag region must be the literal true")
        responsive_orientation = attributes.get("data-wf-responsive-orientation")
        if responsive_orientation is not None and (
            control_kind != "responsive-tracks"
            or responsive_orientation not in {"horizontal", "vertical"}
        ):
            self.issue(
                "WFH043",
                "responsive orientation requires responsive-tracks and must be horizontal or vertical",
            )
        track_mode = attributes.get("data-wf-track-mode")
        if track_mode is not None and track_mode not in {
            "fixed", "content", "remaining"
        }:
            self.issue("WFH044", "track mode must be fixed, content, or remaining")
        image_key = attributes.get("data-wf-image-key")
        if image_key is not None and (
            tag != "button" or not CLASS_PATTERN.fullmatch(image_key)
        ):
            self.issue(
                "WFH046",
                "image key requires a button and a bounded lowercase token",
            )
        connected_axis = attributes.get("data-wf-connected-axis")
        if connected_axis is not None and connected_axis not in {
            "horizontal", "vertical"
        }:
            self.issue("WFH052", "connected axis must be horizontal or vertical")
        keylines = attributes.get("data-wf-keylines")
        if keylines is not None:
            if surface not in {"own-surface", "baked-into-parent"}:
                self.issue(
                    "WFH055",
                    "keylines require an owned or parent-baked surface",
                )
            try:
                parse_keylines(keylines, str(self.path))
            except WebFormsError as error:
                self.diagnostics.extend(error.diagnostics)
        image_list = attributes.get("data-wf-image-list")
        image_source = attributes.get("data-wf-image-src")
        dense_image_source = attributes.get("data-wf-image-src-2x")
        if any(value is not None for value in (
            image_list, image_source, dense_image_source,
            attributes.get("data-wf-image-width"),
            attributes.get("data-wf-image-height"),
        )):
            if (tag != "button" or image_key is None or image_list is None or
                    image_source is None or
                    not CLASS_PATTERN.fullmatch(image_list)):
                self.issue(
                    "WFH053",
                    "local image resources require a button, image key/list, and 1x PNG source",
                )
        image_relation = attributes.get("data-wf-image-relation")
        if image_relation is not None and image_relation not in {
            "overlay", "image-above-text", "image-before-text",
            "text-above-image", "text-before-image",
        }:
            self.issue("WFH047", "image relation is outside the bounded control vocabulary")
        for alignment_name in (
            "data-wf-image-alignment", "data-wf-text-alignment"
        ):
            alignment = attributes.get(alignment_name)
            if alignment is not None and alignment not in {
                "top-left", "top-center", "top-right", "middle-left",
                "middle-center", "middle-right", "bottom-left",
                "bottom-center", "bottom-right",
            }:
                self.issue("WFH048", f"{alignment_name} has an invalid alignment")
        split_orientation = attributes.get("data-wf-split-orientation")
        if split_orientation is not None and (
            control_kind != "split-view"
            or split_orientation not in {"horizontal", "vertical"}
        ):
            self.issue(
                "WFH049",
                "split orientation requires split-view and must be horizontal or vertical",
            )
        split_panel = attributes.get("data-wf-split-panel")
        if split_panel is not None and split_panel not in {"first", "second"}:
            self.issue("WFH050", "split panel must be first or second")
        for panel_name in (
            "data-wf-split-fixed-panel", "data-wf-split-collapse-panel"
        ):
            panel = attributes.get(panel_name)
            if panel is not None and panel not in {"none", "first", "second"}:
                self.issue("WFH051", f"{panel_name} must be none, first, or second")
        for numeric_name in (
            "data-wf-overflow-gap", "data-wf-overflow-minimum",
            "data-wf-dropdown-width",
            "data-wf-overflow-preferred", "data-wf-overflow-maximum",
            "data-wf-overflow-priority", "data-wf-responsive-gap",
            "data-wf-track-index", "data-wf-track-minimum",
            "data-wf-track-preferred", "data-wf-track-maximum",
            "data-wf-track-weight", "data-wf-track-collapse-priority",
            "data-wf-image-gap", "data-wf-content-padding-left",
            "data-wf-content-padding-top", "data-wf-content-padding-right",
            "data-wf-content-padding-bottom",
            "data-wf-split-distance", "data-wf-split-visible-thickness",
            "data-wf-split-hit-before", "data-wf-split-hit-after",
            "data-wf-split-minimum-hit-target", "data-wf-split-first-minimum",
            "data-wf-split-second-minimum", "data-wf-split-first-maximum",
            "data-wf-split-second-maximum",
            "data-wf-split-automatic-collapse-threshold",
            "data-wf-split-transition-ms",
            "data-wf-image-width", "data-wf-image-height",
        ):
            raw = attributes.get(numeric_name)
            if raw is None:
                continue
            try:
                number = float(raw)
            except ValueError:
                number = -1.0
            if not (0.0 <= number <= 1_000_000.0):
                self.issue("WFH042", f"{numeric_name} must be a bounded nonnegative number")
        for integer_name, maximum in (
            ("data-wf-track-index", 63),
            ("data-wf-track-collapse-priority", 65535),
        ):
            raw = attributes.get(integer_name)
            if raw is not None and (not raw.isdigit() or int(raw) > maximum):
                self.issue(
                    "WFH045",
                    f"{integer_name} must be an integer between zero and {maximum}",
                )

        if (tag in INTERACTIVE_ELEMENTS or control_kind is not None or exposure == "exposed") and stable_id is None:
            self.issue("WFH026", f"addressable <{tag}> requires a stable id")

        for url_attribute in ("src", "href", "data-wf-image-src", "data-wf-image-src-2x"):
            value = attributes.get(url_attribute)
            if value is None:
                continue
            parsed = urlparse(value)
            if parsed.scheme or parsed.netloc or value.startswith(("/", "..")):
                self.issue("WFH028", f"{url_attribute} must be a local relative resource")

        if tag == "link":
            if attributes.get("rel") != "stylesheet" or "href" not in attributes:
                self.issue("WFH029", "only local stylesheet links are admitted")
            else:
                self.style_links.append(attributes["href"])

        node = HtmlNode(len(self.nodes), self.stack[-1] if self.stack else None, tag, attributes, line, column)
        self.nodes.append(node)
        if tag == "style":
            self._style_chunks = []
        if tag == "title":
            self._title_chunks = []
        if tag not in VOID_ELEMENTS and not self_closing:
            self.stack.append(node.index)
        elif self_closing and tag not in VOID_ELEMENTS:
            return

    def handle_endtag(self, tag: str) -> None:
        tag = tag.lower()
        if self.ignored_tags:
            expected = self.ignored_tags[-1]
            if tag == expected:
                self.ignored_tags.pop()
            return
        if tag in VOID_ELEMENTS:
            self.issue("WFH030", f"void element <{tag}> must not have an end tag")
            return
        if not self.stack:
            self.issue("WFH031", f"unexpected closing tag </{tag}>")
            return
        node = self.nodes[self.stack[-1]]
        if node.tag != tag:
            self.issue("WFH032", f"closing tag </{tag}> does not match <{node.tag}>")
            return
        self.stack.pop()
        if tag == "style" and self._style_chunks is not None:
            self.inline_styles.append("".join(self._style_chunks))
            self._style_chunks = None
        if tag == "title" and self._title_chunks is not None:
            self.title = "".join(self._title_chunks).strip()
            self._title_chunks = None

    def handle_data(self, data: str) -> None:
        if self._style_chunks is not None:
            self._style_chunks.append(data)
            return
        if self._title_chunks is not None:
            self._title_chunks.append(data)
        if self.stack and data:
            self.nodes[self.stack[-1]].text.append(data)

    def close_and_validate(self) -> HtmlDocument:
        super().close()
        if self.stack:
            node = self.nodes[self.stack[-1]]
            self.issue("WFH033", f"unclosed element <{node.tag}>", node.line, node.column)
        if self.ignored_tags:
            self.issue("WFH038", f"unclosed unsupported subtree <{self.ignored_tags[-1]}>")
        tags = [node.tag for node in self.nodes]
        if tags.count("html") != 1 or tags.count("head") != 1 or tags.count("body") != 1:
            self.issue("WFH034", "document requires exactly one html, head, and body")
        body_nodes = [node for node in self.nodes if node.tag == "body"]
        if body_nodes and "id" not in body_nodes[0].attributes:
            self.issue("WFH035", "body requires the root stable id")
        total_text = sum(len(chunk.encode("utf-8")) for node in self.nodes for chunk in node.text)
        if total_text > MAX_TEXT_BYTES:
            self.issue("WFH036", "text byte limit exceeded")
        self._validate_id_tree()
        self._validate_connected_groups()
        if self.diagnostics:
            raise WebFormsError(self.diagnostics)
        return HtmlDocument(self.path, self.nodes, self.title, self.style_links, self.inline_styles)

    def _validate_id_tree(self) -> None:
        for node in self.nodes:
            stable_id = node.attributes.get("id")
            if stable_id is None:
                continue
            parent_index = node.parent
            parent_id: str | None = None
            while parent_index is not None:
                parent = self.nodes[parent_index]
                if "id" in parent.attributes:
                    parent_id = parent.attributes["id"]
                    break
                parent_index = parent.parent
            if parent_id is not None and not stable_id.startswith(parent_id + "."):
                self.issue(
                    "WFH037",
                    f"stable id {stable_id!r} must extend retained parent {parent_id!r}",
                    node.line,
                    node.column,
                )

    def _validate_connected_groups(self) -> None:
        for node in self.nodes:
            if "data-wf-connected-axis" not in node.attributes:
                continue
            children = [child for child in self.nodes if child.parent == node.index]
            if len(children) < 2 or len(children) > 256 or any(
                child.tag != "button" or child.attributes.get("data-wf-control") not in {
                    None, "button", "dropdown-button"
                }
                for child in children
            ):
                self.issue(
                    "WFH054",
                    "connected group requires two through 256 direct button/dropdown children",
                    node.line,
                    node.column,
                )


def parse_html(path: Path) -> HtmlDocument:
    data = path.read_bytes()
    if len(data) > MAX_SOURCE_BYTES:
        raise WebFormsError([Diagnostic("WFH040", "HTML source byte limit exceeded", str(path))])
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError as error:
        raise WebFormsError([Diagnostic("WFH041", f"source is not strict UTF-8: {error}", str(path))]) from error
    parser = BoundedHtmlParser(path)
    parser.feed(text)
    return parser.close_and_validate()
