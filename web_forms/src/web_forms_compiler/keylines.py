from __future__ import annotations

import math
import re
from typing import Any

from .diagnostics import Diagnostic, WebFormsError


_COLOR = re.compile(r"^#([0-9a-fA-F]{6}|[0-9a-fA-F]{8})$")
_EDGES = frozenset({"top", "right", "bottom", "left"})


def parse_keylines(value: str, source: str = "<source>") -> list[dict[str, Any]]:
    records = [item.strip() for item in value.split(";") if item.strip()]
    if not records or len(records) > 8:
        raise WebFormsError([Diagnostic(
            "WFK001", "keylines require between one and eight ordered records", source
        )])
    result: list[dict[str, Any]] = []
    for record in records:
        parts = record.split()
        if len(parts) != 4 or parts[0] not in _EDGES:
            raise WebFormsError([Diagnostic(
                "WFK002", "keyline syntax is: edge #rrggbb[aa] width inset", source
            )])
        match = _COLOR.fullmatch(parts[1])
        if match is None:
            raise WebFormsError([Diagnostic(
                "WFK003", "keyline color must be #rrggbb or #rrggbbaa", source
            )])
        try:
            width = float(parts[2])
            inset = float(parts[3])
        except ValueError as error:
            raise WebFormsError([Diagnostic(
                "WFK004", "keyline width and inset must be finite numbers", source
            )]) from error
        if not math.isfinite(width) or not math.isfinite(inset):
            raise WebFormsError([Diagnostic(
                "WFK004", "keyline width and inset must be finite numbers", source
            )])
        if width <= 0.0 or width > 64.0 or inset < 0.0 or inset > 64.0:
            raise WebFormsError([Diagnostic(
                "WFK005", "keyline width must be in (0,64] and inset in [0,64]", source
            )])
        packed = match.group(1)
        if len(packed) == 6:
            packed += "ff"
        result.append({
            "edge": parts[0],
            "color": [int(packed[index:index + 2], 16) for index in range(0, 8, 2)],
            "width": width,
            "inset": inset,
        })
    return result
