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
class FlowRecipe:
    direction: str
    wrap: bool
    main_alignment: str
    cross_alignment: str
    column_gap: float
    row_gap: float
    padding_left: float
    padding_top: float
    padding_right: float
    padding_bottom: float


@dataclass(frozen=True)
class GridTrack:
    mode: str
    value: float


@dataclass(frozen=True)
class GridCell:
    child_node: int
    column: int
    row: int


@dataclass(frozen=True)
class GridRecipe:
    columns: tuple[GridTrack, ...]
    rows: tuple[GridTrack, ...]
    column_gap: float
    row_gap: float
    cells: tuple[GridCell, ...]


@dataclass(frozen=True)
class BoxRecipe:
    width: float | None
    height: float | None
    minimum_width: float | None
    minimum_height: float | None
    maximum_width: float | None
    maximum_height: float | None
    margin: tuple[float, float, float, float] | None
    padding: tuple[float, float, float, float] | None
    fill_width: bool
    fill_height: bool
    horizontal_auto_margin: bool
    clips_overflow: bool
    establishes_position_context: bool
    applied: frozenset[str]


def _diagnostic(code: str, message: str) -> WebFormsError:
    return WebFormsError([Diagnostic(code, message, "<ir>")])


def _properties(record: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {item["name"]: item for item in record["properties"]}


def _keyword(item: dict[str, Any], purpose: str) -> str:
    tokens = item["typed"]["tokens"]
    if len(tokens) != 1 or tokens[0].get("kind") != "keyword":
        raise _diagnostic("WFGL010", f"{purpose} requires one canonical keyword")
    return str(tokens[0].get("text", ""))


def _length_token(token: dict[str, Any], purpose: str) -> float:
    if token.get("kind") not in {"number", "logical_px"}:
        raise _diagnostic("WFGL011", f"{purpose} requires logical pixels")
    value = float(token.get("number", 0.0))
    if not math.isfinite(value) or value < 0.0 or value > 4096.0:
        raise _diagnostic(
            "WFGL012", f"{purpose} must be finite and between 0 and 4096px"
        )
    return value


def _one_length(item: dict[str, Any], purpose: str) -> float:
    tokens = item["typed"]["tokens"]
    if len(tokens) != 1:
        raise _diagnostic("WFGL013", f"{purpose} requires one logical length")
    return _length_token(tokens[0], purpose)


def _edge_lengths(item: dict[str, Any], purpose: str) -> tuple[float, float, float, float]:
    tokens = item["typed"]["tokens"]
    if len(tokens) < 1 or len(tokens) > 4:
        raise _diagnostic("WFGL014", f"{purpose} requires one to four logical lengths")
    values = [_length_token(token, purpose) for token in tokens]
    if len(values) == 1:
        return values[0], values[0], values[0], values[0]
    if len(values) == 2:
        return values[0], values[1], values[0], values[1]
    if len(values) == 3:
        return values[0], values[1], values[2], values[1]
    return values[0], values[1], values[2], values[3]


def _flow_recipe(record: dict[str, Any]) -> FlowRecipe:
    properties = _properties(record)
    if "display" not in properties or _keyword(properties["display"], "display") != "flex":
        raise _diagnostic("WFGL001", "native flow projection requires display:flex")

    direction_value = (
        _keyword(properties["flex-direction"], "flex-direction")
        if "flex-direction" in properties else "row"
    )
    directions = {
        "row": "gui_forms::FlowDirection::left_to_right",
        "row-reverse": "gui_forms::FlowDirection::right_to_left",
        "column": "gui_forms::FlowDirection::top_down",
        "column-reverse": "gui_forms::FlowDirection::bottom_up",
    }
    direction = directions.get(direction_value)
    if direction is None:
        raise _diagnostic("WFGL020", f"unsupported flex-direction {direction_value!r}")

    wrap_value = (
        _keyword(properties["flex-wrap"], "flex-wrap")
        if "flex-wrap" in properties else "nowrap"
    )
    if wrap_value == "wrap-reverse":
        raise _diagnostic("WFGL021", "wrap-reverse needs a retained cross-axis reversal")
    if wrap_value not in {"nowrap", "wrap"}:
        raise _diagnostic("WFGL022", f"unsupported flex-wrap {wrap_value!r}")

    main_value = (
        _keyword(properties["justify-content"], "justify-content")
        if "justify-content" in properties else "flex-start"
    )
    main = {
        "flex-start": "gui_forms::FlowMainAlignment::start",
        "center": "gui_forms::FlowMainAlignment::center",
        "flex-end": "gui_forms::FlowMainAlignment::end",
        "space-between": "gui_forms::FlowMainAlignment::space_between",
        "space-around": "gui_forms::FlowMainAlignment::space_around",
        "space-evenly": "gui_forms::FlowMainAlignment::space_evenly",
    }.get(main_value)
    if main is None:
        raise _diagnostic("WFGL023", f"unsupported justify-content {main_value!r}")

    cross_value = (
        _keyword(properties["align-items"], "align-items")
        if "align-items" in properties else "stretch"
    )
    cross = {
        "flex-start": "gui_forms::FlowCrossAlignment::start",
        "center": "gui_forms::FlowCrossAlignment::center",
        "flex-end": "gui_forms::FlowCrossAlignment::end",
        "stretch": "gui_forms::FlowCrossAlignment::stretch",
    }.get(cross_value)
    if cross is None:
        raise _diagnostic("WFGL024", f"unsupported align-items {cross_value!r}")
    align_content = (
        _keyword(properties["align-content"], "align-content")
        if "align-content" in properties else "stretch"
    )
    if wrap_value == "wrap" and align_content != "flex-start":
        raise _diagnostic(
            "WFGL025",
            "wrapped flex requires explicit align-content:flex-start until retained line distribution exists",
        )

    row_gap = 0.0
    column_gap = 0.0
    if "gap" in properties:
        tokens = properties["gap"]["typed"]["tokens"]
        if len(tokens) not in {1, 2}:
            raise _diagnostic("WFGL030", "gap requires one or two logical lengths")
        row_gap = _length_token(tokens[0], "row gap")
        column_gap = row_gap if len(tokens) == 1 else _length_token(
            tokens[1], "column gap"
        )
    if "row-gap" in properties:
        row_gap = _one_length(properties["row-gap"], "row gap")
    if "column-gap" in properties:
        column_gap = _one_length(properties["column-gap"], "column gap")

    top = right = bottom = left = 0.0
    if "padding" in properties:
        top, right, bottom, left = _edge_lengths(properties["padding"], "padding")
    if "padding-top" in properties:
        top = _one_length(properties["padding-top"], "padding-top")
    if "padding-right" in properties:
        right = _one_length(properties["padding-right"], "padding-right")
    if "padding-bottom" in properties:
        bottom = _one_length(properties["padding-bottom"], "padding-bottom")
    if "padding-left" in properties:
        left = _one_length(properties["padding-left"], "padding-left")

    return FlowRecipe(
        direction,
        wrap_value == "wrap",
        main,
        cross,
        column_gap,
        row_gap,
        left,
        top,
        right,
        bottom,
    )


def _flex_grow(record: dict[str, Any]) -> float:
    properties = _properties(record)
    if "flex-grow" in properties:
        return _one_length(properties["flex-grow"], "flex-grow")
    item = properties.get("flex")
    if item is None:
        return 0.0
    tokens = item["typed"]["tokens"]
    if len(tokens) != 1 or tokens[0].get("kind") != "number":
        raise _diagnostic(
            "WFGL040", "first native flex shorthand projection accepts one grow number"
        )
    grow = float(tokens[0].get("number", 0.0))
    if not math.isfinite(grow) or grow < 0.0 or grow > 1024.0:
        raise _diagnostic("WFGL041", "flex grow must be between zero and 1024")
    return grow


def _grid_tracks(item: dict[str, Any], purpose: str) -> tuple[GridTrack, ...]:
    tokens = item["typed"]["tokens"]
    if not tokens or len(tokens) > 64:
        raise _diagnostic("WFGL050", f"{purpose} requires one through 64 tracks")
    tracks: list[GridTrack] = []
    for token in tokens:
        kind = token.get("kind")
        value = float(token.get("number", 0.0))
        if kind == "logical_px":
            value = _length_token(token, purpose)
            tracks.append(GridTrack("gui_forms::TableSizeMode::absolute", value))
        elif kind == "fraction":
            if not math.isfinite(value) or value <= 0.0 or value > 1024.0:
                raise _diagnostic(
                    "WFGL051", f"{purpose} fractions must be above zero and at most 1024fr"
                )
            tracks.append(GridTrack("gui_forms::TableSizeMode::percent", value))
        elif kind == "keyword" and token.get("text") == "auto":
            tracks.append(GridTrack("gui_forms::TableSizeMode::auto_size", 0.0))
        else:
            raise _diagnostic(
                "WFGL052", f"{purpose} supports only logical px, positive fr, and auto tracks"
            )
    return tuple(tracks)


def _grid_axis_gap(properties: dict[str, dict[str, Any]]) -> tuple[float, float]:
    row_gap = 0.0
    column_gap = 0.0
    if "gap" in properties:
        tokens = properties["gap"]["typed"]["tokens"]
        if len(tokens) not in {1, 2}:
            raise _diagnostic("WFGL053", "grid gap requires one or two logical lengths")
        row_gap = _length_token(tokens[0], "grid row gap")
        column_gap = row_gap if len(tokens) == 1 else _length_token(
            tokens[1], "grid column gap"
        )
    if "row-gap" in properties:
        row_gap = _one_length(properties["row-gap"], "grid row gap")
    if "column-gap" in properties:
        column_gap = _one_length(properties["column-gap"], "grid column gap")
    if row_gap > 256.0 or column_gap > 256.0:
        raise _diagnostic("WFGL054", "native grid gaps must be at most 256px")
    return row_gap, column_gap


def _grid_line(record: dict[str, Any], name: str) -> int | None:
    item = _properties(record).get(name)
    if item is None:
        return None
    tokens = item["typed"]["tokens"]
    if len(tokens) != 1 or tokens[0].get("kind") != "number":
        raise _diagnostic("WFGL055", f"{name} requires one positive integer line")
    value = float(tokens[0].get("number", 0.0))
    if not math.isfinite(value) or value < 1.0 or value > 64.0 or value != int(value):
        raise _diagnostic("WFGL056", f"{name} must be an integer from 1 through 64")
    return int(value) - 1


def _grid_recipe(
    record: dict[str, Any],
    children: list[tuple[dict[str, Any], dict[str, Any]]],
) -> GridRecipe:
    properties = _properties(record)
    if "display" not in properties or _keyword(properties["display"], "display") != "grid":
        raise _diagnostic("WFGL057", "native grid projection requires display:grid")
    if "grid-template-columns" not in properties:
        raise _diagnostic("WFGL058", "native grid projection requires explicit columns")
    columns = _grid_tracks(properties["grid-template-columns"], "grid columns")
    explicit_rows = (
        _grid_tracks(properties["grid-template-rows"], "grid rows")
        if "grid-template-rows" in properties else ()
    )
    for alignment in ("align-items", "justify-items"):
        if alignment in properties and _keyword(properties[alignment], alignment) != "stretch":
            raise _diagnostic(
                "WFGL059", f"native grid projection currently requires {alignment}:stretch"
            )
    row_gap, column_gap = _grid_axis_gap(properties)

    occupied: set[tuple[int, int]] = set()
    placements: dict[int, tuple[int, int]] = {}
    automatic: list[int] = []
    maximum_row = len(explicit_rows) - 1
    for child, child_record in children:
        column = _grid_line(child_record, "grid-column")
        row = _grid_line(child_record, "grid-row")
        if (column is None) != (row is None):
            raise _diagnostic(
                "WFGL060", "partial explicit grid placement is outside the first native subset"
            )
        if column is None or row is None:
            automatic.append(child["index"])
            continue
        if column >= len(columns):
            raise _diagnostic("WFGL061", "explicit grid column exceeds the authored template")
        if (column, row) in occupied:
            raise _diagnostic("WFGL062", "overlapping grid items need a retained overlap primitive")
        occupied.add((column, row))
        placements[child["index"]] = (column, row)
        maximum_row = max(maximum_row, row)

    for child_node in automatic:
        position = 0
        while True:
            column = position % len(columns)
            row = position // len(columns)
            if (column, row) not in occupied:
                occupied.add((column, row))
                placements[child_node] = (column, row)
                maximum_row = max(maximum_row, row)
                break
            position += 1
            if position >= 4096:
                raise _diagnostic("WFGL063", "grid automatic placement exceeded bounded search")

    row_count = max(1, maximum_row + 1)
    if row_count > 64:
        raise _diagnostic("WFGL064", "native grid projection is bounded to 64 rows")
    rows = list(explicit_rows)
    rows.extend(
        GridTrack("gui_forms::TableSizeMode::auto_size", 0.0)
        for unused in range(len(rows), row_count)
    )
    cells = tuple(
        GridCell(child["index"], placements[child["index"]][0], placements[child["index"]][1])
        for child, unused_record in children
    )
    return GridRecipe(columns, tuple(rows), column_gap, row_gap, cells)


def _optional_box_length(
    properties: dict[str, dict[str, Any]], name: str,
    allow_none: bool = False,
) -> tuple[float | None, bool]:
    item = properties.get(name)
    if item is None:
        return None, False
    tokens = item["typed"]["tokens"]
    if len(tokens) != 1:
        raise _diagnostic("WFGL070", f"{name} requires one bounded scalar")
    token = tokens[0]
    if allow_none and token.get("kind") == "keyword" and token.get("text") == "none":
        return None, True
    return _length_token(token, name), True


def _box_dimension(
    properties: dict[str, dict[str, Any]], name: str,
) -> tuple[float | None, bool, bool]:
    item = properties.get(name)
    if item is None:
        return None, False, False
    tokens = item["typed"]["tokens"]
    if len(tokens) != 1:
        raise _diagnostic("WFGL071", f"{name} requires one bounded scalar")
    token = tokens[0]
    if token.get("kind") == "percent" and float(token.get("number", 0.0)) == 100.0:
        return None, True, True
    if token.get("kind") == "keyword" and token.get("text") == "auto":
        return None, False, True
    return _length_token(token, name), False, True


def _margin_edges(
    properties: dict[str, dict[str, Any]],
) -> tuple[tuple[float, float, float, float] | None, bool, set[str]]:
    values: list[float | None] = [0.0, 0.0, 0.0, 0.0]
    present = False
    applied: set[str] = set()
    if "margin" in properties:
        tokens = properties["margin"]["typed"]["tokens"]
        if len(tokens) < 1 or len(tokens) > 4:
            raise _diagnostic("WFGL072", "margin requires one through four bounded values")
        parsed: list[float | None] = []
        for token in tokens:
            if token.get("kind") == "keyword" and token.get("text") == "auto":
                parsed.append(None)
            else:
                parsed.append(_length_token(token, "margin"))
        if len(parsed) == 1:
            values = [parsed[0], parsed[0], parsed[0], parsed[0]]
        elif len(parsed) == 2:
            values = [parsed[0], parsed[1], parsed[0], parsed[1]]
        elif len(parsed) == 3:
            values = [parsed[0], parsed[1], parsed[2], parsed[1]]
        else:
            values = parsed
        present = True
        applied.add("margin")
    for index, name in enumerate(
        ("margin-top", "margin-right", "margin-bottom", "margin-left")
    ):
        item = properties.get(name)
        if item is None:
            continue
        tokens = item["typed"]["tokens"]
        if len(tokens) != 1:
            raise _diagnostic("WFGL073", f"{name} requires one bounded value")
        token = tokens[0]
        values[index] = (
            None
            if token.get("kind") == "keyword" and token.get("text") == "auto"
            else _length_token(token, name)
        )
        present = True
        applied.add(name)
    if not present:
        return None, False, applied
    if values[0] is None or values[2] is None:
        raise _diagnostic("WFGL074", "vertical auto margins need a retained block-axis solver")
    horizontal_auto = values[1] is None or values[3] is None
    if horizontal_auto and not (values[1] is None and values[3] is None):
        raise _diagnostic("WFGL075", "one-sided auto margin needs a retained block alignment solver")
    numeric = tuple(0.0 if value is None else value for value in values)
    return (numeric[0], numeric[1], numeric[2], numeric[3]), horizontal_auto, applied


def _padding_edges(
    properties: dict[str, dict[str, Any]],
) -> tuple[tuple[float, float, float, float] | None, set[str]]:
    top = right = bottom = left = 0.0
    present = False
    applied: set[str] = set()
    if "padding" in properties:
        top, right, bottom, left = _edge_lengths(properties["padding"], "padding")
        present = True
        applied.add("padding")
    for name in ("padding-top", "padding-right", "padding-bottom", "padding-left"):
        if name in properties:
            present = True
            applied.add(name)
    if "padding-top" in properties:
        top = _one_length(properties["padding-top"], "padding-top")
    if "padding-right" in properties:
        right = _one_length(properties["padding-right"], "padding-right")
    if "padding-bottom" in properties:
        bottom = _one_length(properties["padding-bottom"], "padding-bottom")
    if "padding-left" in properties:
        left = _one_length(properties["padding-left"], "padding-left")
    return ((top, right, bottom, left) if present else None), applied


def _box_recipe(record: dict[str, Any]) -> BoxRecipe:
    properties = _properties(record)
    width, fill_width, width_present = _box_dimension(properties, "width")
    height, fill_height, height_present = _box_dimension(properties, "height")
    minimum_width, minimum_width_present = _optional_box_length(properties, "min-width")
    minimum_height, minimum_height_present = _optional_box_length(properties, "min-height")
    maximum_width, maximum_width_present = _optional_box_length(
        properties, "max-width", allow_none=True
    )
    maximum_height, maximum_height_present = _optional_box_length(
        properties, "max-height", allow_none=True
    )
    margin, horizontal_auto_margin, margin_applied = _margin_edges(properties)
    padding, padding_applied = _padding_edges(properties)
    applied = set(margin_applied) | set(padding_applied)
    for name, present in (
        ("width", width_present), ("height", height_present),
        ("min-width", minimum_width_present), ("min-height", minimum_height_present),
        ("max-width", maximum_width_present), ("max-height", maximum_height_present),
    ):
        if present:
            applied.add(name)

    sizes_box = any(
        (width_present, height_present, minimum_width_present, minimum_height_present,
         maximum_width_present, maximum_height_present, padding is not None)
    )
    if sizes_box:
        box_sizing = properties.get("box-sizing")
        if box_sizing is None or _keyword(box_sizing, "box-sizing") != "border-box":
            raise _diagnostic(
                "WFGL076", "native box dimensions require box-sizing:border-box"
            )
        applied.add("box-sizing")
    if maximum_width is not None and minimum_width is not None and maximum_width < minimum_width:
        raise _diagnostic("WFGL077", "max-width must not be below min-width")
    if maximum_height is not None and minimum_height is not None and maximum_height < minimum_height:
        raise _diagnostic("WFGL078", "max-height must not be below min-height")

    clips_overflow = False
    for name in ("overflow", "overflow-x", "overflow-y"):
        if name not in properties:
            continue
        if _keyword(properties[name], name) != "hidden":
            raise _diagnostic("WFGL079", f"native box projection supports only authored {name}:hidden")
        clips_overflow = True
        applied.add(name)
    establishes_position_context = False
    if "position" in properties:
        if _keyword(properties["position"], "position") != "relative":
            raise _diagnostic("WFGL080", "native box projection supports only position:relative")
        if {"top", "right", "bottom", "left"}.intersection(properties):
            raise _diagnostic("WFGL081", "offset relative positioning needs a retained offset recipe")
        establishes_position_context = True
        applied.add("position")
    return BoxRecipe(
        width, height, minimum_width, minimum_height,
        maximum_width, maximum_height, margin, padding,
        fill_width, fill_height, horizontal_auto_margin,
        clips_overflow, establishes_position_context, frozenset(applied),
    )


def _number(value: float) -> str:
    rendered = repr(float(value))
    return rendered if "." in rendered else rendered + ".0"


def generate_gui_layouts(
    ir: dict[str, Any], manifest: dict[str, Any], output_dir: Path,
    unit_name: str | None = None,
) -> tuple[Path, Path, Path]:
    if ir.get("schema") != IR_SCHEMA:
        raise _diagnostic("WFGL002", f"native layout Stage 2 requires {IR_SCHEMA!r}")
    flex_capability = manifest.get("features", {}).get("layout.flex", {})
    if flex_capability.get("status") != "supported":
        raise _diagnostic("WFGL003", "target manifest does not support native flex projection")
    grid_capability = manifest.get("features", {}).get("layout.grid", {})
    if grid_capability.get("status") != "supported":
        raise _diagnostic("WFGL004", "target manifest does not support native grid projection")

    root_id = next(
        (node["id"] for node in ir["document"]["nodes"] if node["tag"] == "body"),
        "form",
    )
    unit = _symbol(unit_name or root_id)
    namespace = f"web_forms_generated_{unit}"
    output_dir.mkdir(parents=True, exist_ok=True)
    header_path = output_dir / f"{unit}.gui_layouts.wf.hpp"
    source_path = output_dir / f"{unit}.gui_layouts.wf.cpp"
    report_path = output_dir / f"{unit}.gui_layouts.json"

    nodes = ir["document"]["nodes"]
    geometry_pool = ir["styles"]["pools"]["geometry"]
    style_by_node = {item["node"]: item for item in ir["styles"]["nodes"]}
    recipes: dict[int, FlowRecipe] = {}
    flow_records: list[dict[str, Any]] = []
    for node in nodes:
        if not node["runtime"]:
            continue
        style = style_by_node[node["index"]]
        record = geometry_pool[style["geometry"]]
        properties = _properties(record)
        if "display" not in properties or properties["display"]["value"] != "flex":
            continue
        try:
            recipe = _flow_recipe(record)
        except WebFormsError as error:
            flow_records.append(
                {
                    "node": node["index"],
                    "id": node["id"],
                    "status": "unavailable",
                    "diagnostics": [item.render() for item in error.diagnostics],
                }
            )
        else:
            recipes[node["index"]] = recipe
            applied = {
                "display", "flex-direction", "flex-wrap", "justify-content",
                "align-items", "align-content", "gap", "row-gap", "column-gap",
                "padding", "padding-top", "padding-right", "padding-bottom",
                "padding-left",
            }
            flow_records.append(
                {
                    "node": node["index"],
                    "id": node["id"],
                    "status": "bounded-native-flow-projection",
                    "unprojected_box_geometry": sorted(set(properties) - applied),
                    "diagnostics": [],
                }
            )

    grid_recipes: dict[int, GridRecipe] = {}
    grid_records: list[dict[str, Any]] = []
    for node in nodes:
        if not node["runtime"]:
            continue
        style = style_by_node[node["index"]]
        record = geometry_pool[style["geometry"]]
        properties = _properties(record)
        if "display" not in properties or properties["display"]["value"] != "grid":
            continue
        direct_children = [
            (
                child,
                geometry_pool[style_by_node[child["index"]]["geometry"]],
            )
            for child in nodes
            if child["runtime"] and child["parent"] == node["index"]
        ]
        try:
            recipe = _grid_recipe(record, direct_children)
        except WebFormsError as error:
            grid_records.append(
                {
                    "node": node["index"],
                    "id": node["id"],
                    "status": "unavailable",
                    "diagnostics": [item.render() for item in error.diagnostics],
                }
            )
        else:
            grid_recipes[node["index"]] = recipe
            applied = {
                "display", "grid-template-columns", "grid-template-rows",
                "gap", "row-gap", "column-gap", "align-items", "justify-items",
            }
            grid_records.append(
                {
                    "node": node["index"],
                    "id": node["id"],
                    "status": "bounded-native-grid-projection",
                    "column_count": len(recipe.columns),
                    "row_count": len(recipe.rows),
                    "cell_count": len(recipe.cells),
                    "unprojected_box_geometry": sorted(set(properties) - applied),
                    "diagnostics": [],
                }
            )
    grows: dict[int, float] = {}
    grow_records: list[dict[str, Any]] = []
    for node in nodes:
        if not node["runtime"]:
            continue
        record = geometry_pool[style_by_node[node["index"]]["geometry"]]
        if not {"flex", "flex-grow"}.intersection(_properties(record)):
            continue
        try:
            grow = _flex_grow(record)
        except WebFormsError as error:
            grow_records.append(
                {
                    "node": node["index"], "id": node["id"],
                    "status": "unavailable",
                    "diagnostics": [item.render() for item in error.diagnostics],
                }
            )
        else:
            grows[node["index"]] = grow
            grow_records.append(
                {
                    "node": node["index"], "id": node["id"],
                    "status": "bounded-native-flex-grow", "grow": grow,
                    "diagnostics": [],
                }
            )

    box_properties = {
        "width", "height", "min-width", "min-height", "max-width", "max-height",
        "margin", "margin-top", "margin-right", "margin-bottom", "margin-left",
        "padding", "padding-top", "padding-right", "padding-bottom", "padding-left",
        "overflow", "overflow-x", "overflow-y", "position", "top", "right",
        "bottom", "left",
    }
    box_recipes: dict[int, BoxRecipe] = {}
    box_records: list[dict[str, Any]] = []
    for node in nodes:
        if not node["runtime"]:
            continue
        record = geometry_pool[style_by_node[node["index"]]["geometry"]]
        properties = _properties(record)
        if not box_properties.intersection(properties):
            continue
        try:
            recipe = _box_recipe(record)
        except WebFormsError as error:
            box_records.append(
                {
                    "node": node["index"], "id": node["id"],
                    "status": "unavailable",
                    "diagnostics": [item.render() for item in error.diagnostics],
                }
            )
        else:
            box_recipes[node["index"]] = recipe
            box_records.append(
                {
                    "node": node["index"], "id": node["id"],
                    "status": "bounded-native-box-projection",
                    "fill_width": recipe.fill_width,
                    "fill_height": recipe.fill_height,
                    "horizontal_auto_margin": recipe.horizontal_auto_margin,
                    "clips_overflow": recipe.clips_overflow,
                    "establishes_position_context": recipe.establishes_position_context,
                    "unprojected_box_geometry": sorted(
                        box_properties.intersection(properties) - recipe.applied
                    ),
                    "diagnostics": [],
                }
            )
    header_lines = [
        "#pragma once",
        "",
        "#include <cstddef>",
        '#include "gui_forms/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.hpp"',
        '#include "gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp"',
        "",
        f"namespace {namespace} {{",
        "",
        "bool has_native_flow_layout(std::size_t node_index) noexcept;",
        "void apply_native_flow_layout(std::size_t node_index, gui_forms::FlowLayoutPanel& panel);",
        "double native_flex_grow(std::size_t node_index) noexcept;",
        "bool has_native_grid_layout(std::size_t node_index) noexcept;",
        "void apply_native_grid_layout(std::size_t node_index, gui_forms::TableLayoutPanel& panel);",
        "bool has_native_grid_cell(std::size_t parent_node_index, std::size_t child_node_index) noexcept;",
        "void apply_native_grid_cell(std::size_t parent_node_index, std::size_t child_node_index, gui_forms::TableLayoutPanel& panel, gui_forms::Control& child);",
        "bool has_native_box_geometry(std::size_t node_index) noexcept;",
        "void apply_native_box_geometry(std::size_t node_index, gui_forms::Control& control);",
        "bool native_box_fills_parent_width(std::size_t node_index) noexcept;",
        "bool native_box_fills_parent_height(std::size_t node_index) noexcept;",
        "bool native_box_has_horizontal_auto_margin(std::size_t node_index) noexcept;",
        "bool native_box_clips_overflow(std::size_t node_index) noexcept;",
        "bool native_box_establishes_position_context(std::size_t node_index) noexcept;",
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
        "",
        "bool has_native_flow_layout(std::size_t node_index) noexcept {",
        "    switch (node_index) {",
    ]
    for node_index in recipes:
        source_lines.append(f"    case {node_index}U:")
    if recipes:
        source_lines.append("        return true;")
    source_lines.extend(
        [
            "    default:", "        return false;", "    }", "}", "",
            "void apply_native_flow_layout(",
            "    std::size_t node_index, gui_forms::FlowLayoutPanel& panel) {",
            "    switch (node_index) {",
        ]
    )
    for node_index, recipe in recipes.items():
        source_lines.extend(
            [
                f"    case {node_index}U:",
                f"        panel.set_flow_direction({recipe.direction});",
                "        panel.set_wrap_contents(" + ("true);" if recipe.wrap else "false);"),
                f"        panel.set_main_alignment({recipe.main_alignment});",
                f"        panel.set_cross_alignment({recipe.cross_alignment});",
                "        panel.set_item_spacing({"
                f"{_number(recipe.column_gap)}, {_number(recipe.row_gap)}}});",
                "        panel.set_padding({"
                f"{_number(recipe.padding_left)}, {_number(recipe.padding_top)}, "
                f"{_number(recipe.padding_right)}, {_number(recipe.padding_bottom)}}});",
                "        return;",
            ]
        )
    source_lines.extend(
        [
            "    default:",
            "        throw std::invalid_argument(\"node has no bounded native flow projection\");",
            "    }", "}", "",
            "double native_flex_grow(std::size_t node_index) noexcept {",
            "    switch (node_index) {",
        ]
    )
    for node_index, grow in grows.items():
        source_lines.extend(
            [f"    case {node_index}U:", f"        return {_number(grow)};"]
        )
    source_lines.extend(
        [
            "    default:", "        return 0.0;", "    }", "}", "",
        ]
    )
    source_lines.extend(
        [
            "bool has_native_grid_layout(std::size_t node_index) noexcept {",
            "    switch (node_index) {",
        ]
    )
    for node_index in grid_recipes:
        source_lines.append(f"    case {node_index}U:")
    if grid_recipes:
        source_lines.append("        return true;")
    source_lines.extend(
        [
            "    default:", "        return false;", "    }", "}", "",
            "void apply_native_grid_layout(",
            "    std::size_t node_index, gui_forms::TableLayoutPanel& panel) {",
            "    static_cast<void>(panel);",
            "    switch (node_index) {",
        ]
    )
    for node_index, recipe in grid_recipes.items():
        source_lines.extend(
            [
                f"    case {node_index}U:",
                f"        panel.set_column_count({len(recipe.columns)}U);",
                f"        panel.set_row_count({len(recipe.rows)}U);",
                "        panel.set_grow_style(gui_forms::TableLayoutGrowStyle::fixed_size);",
                "        panel.set_track_spacing({"
                f"{_number(recipe.column_gap)}, {_number(recipe.row_gap)}}});",
            ]
        )
        for index, track in enumerate(recipe.columns):
            source_lines.append(
                f"        panel.set_column_style({index}U, "
                f"{{{track.mode}, {_number(track.value)}}});"
            )
        for index, track in enumerate(recipe.rows):
            source_lines.append(
                f"        panel.set_row_style({index}U, "
                f"{{{track.mode}, {_number(track.value)}}});"
            )
        source_lines.extend(["        return;"])
    source_lines.extend(
        [
            "    default:",
            "        throw std::invalid_argument(\"node has no bounded native grid projection\");",
            "    }", "}", "",
            "bool has_native_grid_cell(",
            "    std::size_t parent_node_index, std::size_t child_node_index) noexcept {",
            "    static_cast<void>(child_node_index);",
            "    switch (parent_node_index) {",
        ]
    )
    for node_index, recipe in grid_recipes.items():
        source_lines.extend([f"    case {node_index}U:", "        switch (child_node_index) {"])
        for cell in recipe.cells:
            source_lines.append(f"        case {cell.child_node}U:")
        if recipe.cells:
            source_lines.append("            return true;")
        source_lines.extend(
            ["        default:", "            return false;", "        }"]
        )
    source_lines.extend(
        ["    default:", "        return false;", "    }", "}", ""]
    )
    source_lines.extend(
        [
            "void apply_native_grid_cell(",
            "    std::size_t parent_node_index, std::size_t child_node_index,",
            "    gui_forms::TableLayoutPanel& panel, gui_forms::Control& child) {",
            "    static_cast<void>(child_node_index);",
            "    static_cast<void>(panel);",
            "    static_cast<void>(child);",
            "    switch (parent_node_index) {",
        ]
    )
    for node_index, recipe in grid_recipes.items():
        source_lines.extend([f"    case {node_index}U:", "        switch (child_node_index) {"])
        for cell in recipe.cells:
            source_lines.extend(
                [
                    f"        case {cell.child_node}U:",
                    "            child.set_dock(gui_forms::DockStyle::fill);",
                    "            panel.set_cell_position(child, "
                    f"{{{cell.column}U, {cell.row}U}});",
                    "            return;",
                ]
            )
        source_lines.extend(
            ["        default:", "            break;", "        }", "        break;"]
        )
    source_lines.extend(
        [
            "    default:", "        break;", "    }",
            "    throw std::invalid_argument(\"child has no bounded native grid cell projection\");",
            "}", "",
            "bool has_native_box_geometry(std::size_t node_index) noexcept {",
            "    switch (node_index) {",
        ]
    )
    for node_index in box_recipes:
        source_lines.append(f"    case {node_index}U:")
    if box_recipes:
        source_lines.append("        return true;")
    source_lines.extend(
        [
            "    default:", "        return false;", "    }", "}", "",
            "void apply_native_box_geometry(",
            "    std::size_t node_index, gui_forms::Control& control) {",
            "    static_cast<void>(control);",
            "    switch (node_index) {",
        ]
    )
    for node_index, recipe in box_recipes.items():
        source_lines.append(f"    case {node_index}U: {{")
        if {"min-width", "min-height", "max-width", "max-height"}.intersection(
            recipe.applied
        ):
            source_lines.extend(
                [
                    "        gui_forms::Size minimum = control.minimum_size();",
                    "        gui_forms::Size maximum = control.maximum_size();",
                ]
            )
            if "min-width" in recipe.applied:
                source_lines.append(
                    f"        minimum.width = {_number(recipe.minimum_width or 0.0)};"
                )
            if "min-height" in recipe.applied:
                source_lines.append(
                    f"        minimum.height = {_number(recipe.minimum_height or 0.0)};"
                )
            if "max-width" in recipe.applied:
                source_lines.append(
                    f"        maximum.width = {_number(recipe.maximum_width or 0.0)};"
                )
            if "max-height" in recipe.applied:
                source_lines.append(
                    f"        maximum.height = {_number(recipe.maximum_height or 0.0)};"
                )
            source_lines.extend(
                [
                    "        control.set_maximum_size({});",
                    "        control.set_minimum_size(minimum);",
                    "        control.set_maximum_size(maximum);",
                ]
            )
        if recipe.width is not None or recipe.height is not None:
            source_lines.append(
                "        gui_forms::Rect bounds = control.requested_bounds();"
            )
            if recipe.width is not None:
                source_lines.append(
                    f"        bounds.width = {_number(recipe.width)};"
                )
            if recipe.height is not None:
                source_lines.append(
                    f"        bounds.height = {_number(recipe.height)};"
                )
            source_lines.append("        control.set_requested_bounds(bounds);")
        if recipe.margin is not None:
            top, right, bottom, left = recipe.margin
            source_lines.append(
                "        control.set_margin({"
                f"{_number(left)}, {_number(top)}, {_number(right)}, {_number(bottom)}}});"
            )
        if recipe.padding is not None:
            top, right, bottom, left = recipe.padding
            source_lines.append(
                "        control.set_padding({"
                f"{_number(left)}, {_number(top)}, {_number(right)}, {_number(bottom)}}});"
            )
        source_lines.extend(["        return;", "    }"])
    source_lines.extend(
        [
            "    default:",
            "        throw std::invalid_argument(\"node has no bounded native box projection\");",
            "    }", "}", "",
        ]
    )
    box_flags = (
        ("native_box_fills_parent_width", lambda recipe: recipe.fill_width),
        ("native_box_fills_parent_height", lambda recipe: recipe.fill_height),
        (
            "native_box_has_horizontal_auto_margin",
            lambda recipe: recipe.horizontal_auto_margin,
        ),
        ("native_box_clips_overflow", lambda recipe: recipe.clips_overflow),
        (
            "native_box_establishes_position_context",
            lambda recipe: recipe.establishes_position_context,
        ),
    )
    for function_name, predicate in box_flags:
        source_lines.extend(
            [
                f"bool {function_name}(std::size_t node_index) noexcept {{",
                "    switch (node_index) {",
            ]
        )
        matching = [
            node_index for node_index, recipe in box_recipes.items()
            if predicate(recipe)
        ]
        for node_index in matching:
            source_lines.append(f"    case {node_index}U:")
        if matching:
            source_lines.append("        return true;")
        source_lines.extend(
            ["    default:", "        return false;", "    }", "}", ""]
        )
    source_lines.extend([f"}}  // namespace {namespace}", ""])

    header_text = "\n".join(header_lines)
    source_text = "\n".join(source_lines)
    validate_generated_cpp(header_text, header_path)
    validate_generated_cpp(source_text, source_path)
    header_path.write_text(header_text, encoding="utf-8")
    source_path.write_text(source_text, encoding="utf-8")

    nested_surfaces = []
    runtime_nodes = {node["index"] for node in nodes if node["runtime"]}
    for style in ir["styles"]["nodes"]:
        if style["surface"] != "own-surface":
            continue
        node = nodes[style["node"]]
        if not node["runtime"]:
            continue
        parent = node["parent"]
        while parent is not None and parent not in runtime_nodes:
            parent = nodes[parent]["parent"]
        if parent is None:
            continue
        nested_surfaces.append(
            {"node": node["index"], "id": node["id"], "parent": parent}
        )
    report = {
        "schema": "web.forms.gui-layout-projection/0.1-experimental",
        "target": manifest.get("target", "unknown"),
        "source_digest": ir["source"]["digest"],
        "status": "partial-native-relational-layout-projection",
        "exact_flow_count": len(recipes),
        "unavailable_flow_count": len(flow_records) - len(recipes),
        "flows": flow_records,
        "exact_grid_count": len(grid_recipes),
        "unavailable_grid_count": len(grid_records) - len(grid_recipes),
        "grids": grid_records,
        "grid_cell_count": sum(len(recipe.cells) for recipe in grid_recipes.values()),
        "flex_grows": grow_records,
        "exact_box_count": len(box_recipes),
        "unavailable_box_count": len(box_records) - len(box_recipes),
        "boxes": box_records,
        "nested_owned_surfaces": nested_surfaces,
    }
    report_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return header_path, source_path, report_path
