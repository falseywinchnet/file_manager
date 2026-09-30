from __future__ import annotations

import base64
from contextlib import contextmanager
from dataclasses import dataclass
import hashlib
import html as html_module
import json
import os
from pathlib import Path
import re
import shutil
import socket
import struct
import subprocess
import tempfile
import time
from typing import Any
from urllib.request import urlopen
import zlib

from .diagnostics import fail
from .stage1 import compile_source


SNAPSHOT_SCHEMA = "web.forms.fidelity-snapshot/0.1-experimental"
REPORT_SCHEMA = "web.forms.fidelity-report/0.1-experimental"


@dataclass(frozen=True)
class FidelityTolerance:
    geometry: float = 0.51
    clip: float = 0.51
    baseline: float = 1.0
    font_size: float = 0.01
    letter_spacing: float = 0.01
    border_width: float = 0.01
    corner_radius: float = 0.01
    color_distance: int = 0
    raster_color_distance: int = 120


_RASTER_PROBE_POSITIONS = (
    (0.15, 0.15), (0.5, 0.15), (0.85, 0.15),
    (0.15, 0.5), (0.85, 0.5),
    (0.15, 0.85), (0.5, 0.85), (0.85, 0.85),
)


def _decode_png_rgba(encoded: bytes) -> tuple[int, int, bytes]:
    if len(encoded) < 33 or encoded[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("screenshot is not PNG")
    position = 8
    width = height = color_type = bit_depth = interlace = 0
    payload = bytearray()
    while position + 12 <= len(encoded):
        length = struct.unpack(">I", encoded[position:position + 4])[0]
        kind = encoded[position + 4:position + 8]
        data = encoded[position + 8:position + 8 + length]
        position += 12 + length
        if len(data) != length:
            raise ValueError("truncated PNG chunk")
        if kind == b"IHDR":
            width, height, bit_depth, color_type, _, _, interlace = struct.unpack(
                ">IIBBBBB", data
            )
        elif kind == b"IDAT":
            payload.extend(data)
        elif kind == b"IEND":
            break
    channels = {2: 3, 6: 4}.get(color_type)
    if (width <= 0 or height <= 0 or bit_depth != 8 or channels is None or
            interlace != 0):
        raise ValueError("screenshot PNG is outside the bounded RGB/RGBA profile")
    raw = zlib.decompress(bytes(payload))
    stride = width * channels
    if len(raw) != height * (stride + 1):
        raise ValueError("screenshot PNG scanline size is inconsistent")
    previous = bytearray(stride)
    rgba = bytearray(width * height * 4)
    source = 0
    destination = 0
    for _ in range(height):
        filter_kind = raw[source]
        source += 1
        row = bytearray(raw[source:source + stride])
        source += stride
        for index in range(stride):
            left = row[index - channels] if index >= channels else 0
            above = previous[index]
            upper_left = previous[index - channels] if index >= channels else 0
            if filter_kind == 1:
                row[index] = (row[index] + left) & 0xff
            elif filter_kind == 2:
                row[index] = (row[index] + above) & 0xff
            elif filter_kind == 3:
                row[index] = (row[index] + ((left + above) // 2)) & 0xff
            elif filter_kind == 4:
                estimate = left + above - upper_left
                left_distance = abs(estimate - left)
                above_distance = abs(estimate - above)
                upper_left_distance = abs(estimate - upper_left)
                predictor = left if left_distance <= above_distance and left_distance <= upper_left_distance else (
                    above if above_distance <= upper_left_distance else upper_left
                )
                row[index] = (row[index] + predictor) & 0xff
            elif filter_kind != 0:
                raise ValueError("unsupported PNG row filter")
        for index in range(0, stride, channels):
            rgba[destination:destination + 3] = row[index:index + 3]
            rgba[destination + 3] = row[index + 3] if channels == 4 else 255
            destination += 4
        previous = row
    return width, height, bytes(rgba)


def _attach_browser_raster_probes(
    snapshot: dict[str, Any], screenshot: bytes
) -> None:
    width, height, rgba = _decode_png_rgba(screenshot)
    scale = float(snapshot.get("environment", {}).get("device_scale", 1.0))
    origin = snapshot.get("environment", {}).get("root_origin", [0.0, 0.0])
    for node in snapshot.get("nodes", []):
        if not node.get("state", {}).get("visible"):
            node["raster_probes"] = []
            continue
        left, top, logical_width, logical_height = node["bounds"]
        probes: list[list[Any]] = []
        for horizontal, vertical in _RASTER_PROBE_POSITIONS:
            x = int((float(origin[0]) + left + logical_width * horizontal) * scale)
            y = int((float(origin[1]) + top + logical_height * vertical) * scale)
            x = max(0, min(width - 1, x))
            y = max(0, min(height - 1, y))
            offset = (y * width + x) * 4
            probes.append([
                horizontal, vertical, list(rgba[offset:offset + 4])
            ])
        node["raster_probes"] = probes


def _rounded(value: float) -> float:
    return round(float(value), 6)


def _rect(values: list[Any], origin: list[float] | None = None) -> list[float]:
    if len(values) != 4:
        raise ValueError("rectangle must contain four values")
    x, y, width, height = (float(value) for value in values)
    if origin is not None:
        x -= float(origin[0])
        y -= float(origin[1])
    return [_rounded(x), _rounded(y), _rounded(width), _rounded(height)]


def _text_operations(control: dict[str, Any]) -> list[dict[str, Any]]:
    return [
        operation
        for operation in control.get("display_operations", [])
        if operation.get("operation") == "draw_text" and operation.get("text")
    ]


def _native_border(material: dict[str, Any], edge: str) -> dict[str, Any] | None:
    edge_border = material.get("border_edges", {}).get(edge)
    return edge_border if edge_border is not None else material.get("border")


def _native_material(
    material: dict[str, Any] | None,
    operations: list[dict[str, Any]] | None = None,
) -> dict[str, Any]:
    if material is None:
        display = operations or []
        fill_kinds: list[str] = []
        solid_color = None
        for operation in display:
            kind = operation.get("operation")
            if kind in {"fill_rect", "fill_rounded_rect"}:
                color = operation.get("color", [0, 0, 0, 0])
                if len(color) == 4 and int(color[3]) != 0:
                    fill_kinds.append("solid")
                    if solid_color is None:
                        solid_color = color
            elif kind in {"fill_linear_gradient", "fill_linear_gradient_spread"}:
                fill_kinds.append(
                    "repeating_linear_gradient"
                    if operation.get("gradient_spread") == "repeat"
                    else "linear_gradient"
                )
            elif kind == "fill_radial_gradient":
                fill_kinds.append("radial_gradient")
            elif kind in {
                "draw_image", "draw_image_region", "draw_image_region_sampled",
                "fill_image_pattern",
            }:
                fill_kinds.append("image")
        shadows = [
            {
                "inset": operation.get("operation") == "draw_inset_box_shadow",
                "offset": operation.get("first", [0.0, 0.0]),
                "blur_radius": operation.get("scalar", 0.0),
                "spread": operation.get("secondary_scalar", 0.0),
                "color": operation.get("color", [0, 0, 0, 0]),
            }
            for operation in display
            if operation.get("operation") in {
                "draw_box_shadow", "draw_inset_box_shadow"
            }
        ]
        stroke = next(
            (
                operation for operation in reversed(display)
                if operation.get("operation") in {
                    "stroke_rect", "stroke_rounded_rect"
                }
            ),
            None,
        )
        border = {
            "width": float(stroke.get("secondary_scalar", 0.0)) if stroke else 0.0,
            "color": stroke.get("color", [0, 0, 0, 0]) if stroke else [0, 0, 0, 0],
            "style": "solid" if stroke else "none",
        }
        clip = next(
            (
                operation for operation in display
                if operation.get("operation") == "clip_rounded_rect"
            ),
            None,
        )
        radius = float(clip.get("scalar", 0.0)) if clip else 0.0
        return {
            "fill_kinds": fill_kinds,
            "solid_color": solid_color,
            "shadows": shadows,
            "borders": {edge: dict(border) for edge in ("top", "right", "bottom", "left")},
            "corner_radii": [radius, radius, radius, radius],
            "keylines": [],
            "raw": {"display_operations": display},
        }
    fills = material.get("fills", [])
    solid = next(
        (fill.get("color") for fill in fills if fill.get("kind") == "solid"),
        None,
    )
    borders: dict[str, Any] = {}
    for edge in ("top", "right", "bottom", "left"):
        border = _native_border(material, edge)
        borders[edge] = {
            "width": float(border.get("width", 0.0)) if border else 0.0,
            "color": border.get("color") if border else [0, 0, 0, 0],
            "style": "solid" if border else "none",
        }
    radius = float(material.get("corner_radius", 0.0))
    return {
        "fill_kinds": [
            "repeating_linear_gradient"
            if fill.get("kind") == "linear_gradient" and fill.get("spread") == "repeat"
            else fill.get("kind", "unknown")
            for fill in fills
            if fill.get("kind") != "solid"
            or int((fill.get("color") or [0, 0, 0, 0])[3]) != 0
        ],
        "solid_color": solid,
        "shadows": [
            {
                "inset": bool(shadow.get("inset", False)),
                "offset": shadow.get("offset", [0.0, 0.0]),
                "blur_radius": shadow.get("blur_radius", 0.0),
                "spread": shadow.get("spread", 0.0),
                "color": shadow.get("color", [0, 0, 0, 0]),
            }
            for shadow in material.get("shadows", [])
        ],
        "borders": borders,
        "corner_radii": [radius, radius, radius, radius],
        "keylines": material.get("keylines", []),
        "raw": material,
    }


def normalize_native_snapshot(
    native: dict[str, Any],
    *,
    projection: dict[str, Any] | None = None,
    root_id: str | None = None,
    source_digest: str = "",
    state: str = "reference",
) -> dict[str, Any]:
    controls = native.get("controls")
    if not isinstance(controls, list):
        fail("WFV001", "native inspection snapshot has no controls array", "<native-snapshot>")
    by_id = {
        item.get("stable_id"): item
        for item in controls
        if isinstance(item, dict) and item.get("stable_id")
    }
    selected_root = root_id or (projection or {}).get("root_id")
    if not selected_root:
        roots = [item for item in controls if not item.get("parent_stable_id")]
        if len(roots) != 1:
            fail("WFV002", "native snapshot root is ambiguous; supply root_id", "<native-snapshot>")
        selected_root = roots[0].get("stable_id")
    if selected_root not in by_id:
        fail("WFV003", f"native root {selected_root!r} is absent", "<native-snapshot>")
    root_bounds = by_id[selected_root].get("layout", {}).get("absolute_bounds", [0, 0, 0, 0])
    projection_nodes = {
        item.get("id"): item
        for item in (projection or {}).get("nodes", [])
        if isinstance(item, dict)
    }
    kinds = {
        identifier: item.get("kind", "control")
        for identifier, item in projection_nodes.items()
    }
    projected_ids = set(kinds) if projection is not None else None

    def authored_parent(control: dict[str, Any]) -> str | None:
        parent = control.get("parent_stable_id") or None
        visited: set[str] = set()
        while (
            parent is not None
            and projected_ids is not None
            and parent not in projected_ids
        ):
            if parent in visited:
                return None
            visited.add(parent)
            private_parent = by_id.get(parent)
            if private_parent is None:
                return None
            parent = private_parent.get("parent_stable_id") or None
        return parent

    nodes: list[dict[str, Any]] = []
    for control in controls:
        stable_id = control.get("stable_id", "")
        if not stable_id or not (
            stable_id == selected_root or stable_id.startswith(selected_root + ".")
        ):
            continue
        # Compound GUI.Forms controls retain source-private children (for
        # example split panels and grips). The browser/source oracle compares
        # authored stable IDs; implementation children have their own focused
        # native tests and must not become false source-structure differences.
        if projected_ids is not None and stable_id not in projected_ids:
            continue
        layout = control.get("layout", {})
        text_operations = _text_operations(control)
        operation = text_operations[0] if text_operations else None
        typography = None
        if operation is not None:
            resolved = operation.get("resolved_text") or {}
            font = operation.get("font") or {}
            first = operation.get("first", [0.0, 0.0])
            absolute = layout.get("absolute_bounds", [0.0, 0.0, 0.0, 0.0])
            operation_texts = [
                str(item.get("text", "")).strip() for item in text_operations
            ]
            combined_text = " ".join(operation_texts).strip()
            combined_runs: list[dict[str, Any]] = []
            resolved_run_coverage_exact = True
            byte_offset = 0
            for text_index, item in enumerate(text_operations):
                item_text = operation_texts[text_index]
                item_resolved = item.get("resolved_text") or {}
                item_runs = item_resolved.get("runs", [])
                expected_item_start = 0
                if not item_runs:
                    resolved_run_coverage_exact = False
                for run in item_runs:
                    if (int(run.get("utf8_start", -1)) != expected_item_start or
                            not run.get("family")):
                        resolved_run_coverage_exact = False
                    expected_item_start += int(run.get("utf8_length", 0))
                    projected_run = dict(run)
                    projected_run["utf8_start"] = (
                        byte_offset + int(run.get("utf8_start", 0))
                    )
                    combined_runs.append(projected_run)
                if expected_item_start != len(item_text.encode("utf-8")):
                    resolved_run_coverage_exact = False
                byte_offset += len(item_text.encode("utf-8"))
                if text_index + 1 < len(text_operations):
                    byte_offset += 1
            baselines = [
                _rounded(
                    float(absolute[1]) - float(root_bounds[1])
                    + float(item.get("first", [0.0, 0.0])[1])
                )
                for item in text_operations
            ]
            typography = {
                "text": combined_text,
                "baseline": baselines[0],
                "baselines": baselines,
                "family": resolved.get("primary_family", ""),
                "runs": combined_runs,
                "resolved_run_coverage_exact": resolved_run_coverage_exact,
                "resolution_status": resolved.get("status", "absent"),
                "font_size": float(font.get("size", 0.0)),
                "weight": int(font.get("weight", 400)),
                "italic": bool(font.get("italic", False)),
                "letter_spacing": float(font.get("letter_spacing", 0.0)),
                "ascent": resolved.get("ascent"),
                "descent": resolved.get("descent"),
                "line_gap": resolved.get("line_gap"),
            }
        state_record = control.get("state", {})
        absolute_bounds = _rect(
            layout.get("absolute_bounds", [0, 0, 0, 0]), root_bounds
        )
        effective_clip = _rect(
            layout.get("effective_clip", [0, 0, 0, 0]), root_bounds
        )
        clipped_left = max(absolute_bounds[0], effective_clip[0])
        clipped_top = max(absolute_bounds[1], effective_clip[1])
        clipped_right = min(
            absolute_bounds[0] + absolute_bounds[2],
            effective_clip[0] + effective_clip[2],
        )
        clipped_bottom = min(
            absolute_bounds[1] + absolute_bounds[3],
            effective_clip[1] + effective_clip[3],
        )
        visible_clip = [
            _rounded(clipped_left),
            _rounded(clipped_top),
            _rounded(max(0.0, clipped_right - clipped_left)),
            _rounded(max(0.0, clipped_bottom - clipped_top)),
        ]
        if not bool(state_record.get("effectively_visible", False)):
            absolute_bounds = [0.0, 0.0, 0.0, 0.0]
            visible_clip = [0.0, 0.0, 0.0, 0.0]
        nodes.append(
            {
                "id": stable_id,
                "parent_id": authored_parent(control),
                "kind": kinds.get(stable_id, "control"),
                "topology": projection_nodes.get(stable_id, {}).get("topology"),
                "resource": projection_nodes.get(stable_id, {}).get("resource"),
                "bounds": absolute_bounds,
                "clip": visible_clip,
                "state": {
                    "authored_visible": bool(state_record.get("visible", False)),
                    "visible": bool(state_record.get("effectively_visible", False)),
                    "enabled": bool(state_record.get("effectively_enabled", False)),
                    "focused": bool(state_record.get("focused", False)),
                    "hovered": bool(state_record.get("hovered", False)),
                    "pressed": bool(state_record.get("pressed", False)),
                    "window_active": bool(state_record.get("window_active", False)),
                    "layout_collapsed": bool(state_record.get("layout_collapsed", False)),
                    "visual_status": state_record.get("visual_status", "normal"),
                },
                "typography": typography,
                "material": _native_material(
                    control.get("authored_material"),
                    control.get("display_operations", []),
                ),
                "raster_probes": native.get("raster_probes", {}).get(
                    stable_id, []
                ),
            }
        )
    snapshot = {
        "schema": SNAPSHOT_SCHEMA,
        "producer": "gui.forms.visual-inspection",
        "source_digest": source_digest or (projection or {}).get("source_digest", ""),
        "root_id": selected_root,
        "state": state,
        "environment": {
            "viewport": native.get("client_size", [0.0, 0.0]),
            "device_scale": native.get("device_scale", 1.0),
            "text_scale": native.get("text_scale", 1.0),
            "high_contrast": bool(native.get("high_contrast", False)),
            "reduced_motion": bool(native.get("reduced_motion", False)),
            "window_active": bool(native.get("window_active", False)),
            "theme_id": native.get("theme_id", ""),
        },
        "nodes": sorted(nodes, key=lambda item: item["id"]),
    }
    validate_fidelity_snapshot(snapshot, "<normalized-native-snapshot>")
    return snapshot


_CAPTURE_EXPRESSION = r"""
(async () => {
  const stateName = __STATE_NAME__;
  const rootId = __ROOT_ID__;
  const kindById = __KIND_BY_ID__;
  const textScale = __TEXT_SCALE__;
  const requestedWindowActive = __WINDOW_ACTIVE__;
  const interactionState = __INTERACTION_STATE__;
  const interactionTarget = __INTERACTION_TARGET__;
  const resourceImages = [];
  for (const element of document.querySelectorAll('[data-wf-image-src]')) {
    const image = document.createElement('img');
    image.src = element.dataset.wfImageSrc;
    const dense = element.getAttribute('data-wf-image-src-2x');
    if (dense) image.srcset = `${element.dataset.wfImageSrc} 1x, ${dense} 2x`;
    image.alt = '';
    image.setAttribute('aria-hidden', 'true');
    const width = Number.parseFloat(element.dataset.wfImageWidth);
    const height = Number.parseFloat(element.dataset.wfImageHeight);
    image.style.width = `${width}px`;
    image.style.height = `${height}px`;
    image.style.objectFit = 'contain';
    image.style.pointerEvents = 'none';
    if (element.dataset.wfImageRelation === 'overlay') {
      element.style.position = 'relative';
      image.style.position = 'absolute';
      image.style.left = '50%';
      image.style.top = '50%';
      image.style.transform = 'translate(-50%, -50%)';
    } else {
      image.style.flex = '0 0 auto';
      element.prepend(image);
      resourceImages.push(image);
      continue;
    }
    element.append(image);
    resourceImages.push(image);
  }
  await Promise.all(resourceImages.map(image => image.decode().catch(() => undefined)));
  await document.fonts.ready;
  if (textScale !== 1) {
    const measured = Array.from(document.querySelectorAll('[id]')).map(element => {
      const style = getComputedStyle(element);
      return {element, fontSize:Number.parseFloat(style.fontSize),
              lineHeight:Number.parseFloat(style.lineHeight)};
    });
    for (const record of measured) {
      if (Number.isFinite(record.fontSize))
        record.element.style.fontSize = `${record.fontSize * textScale}px`;
      if (Number.isFinite(record.lineHeight))
        record.element.style.lineHeight = `${record.lineHeight * textScale}px`;
    }
  }
  await new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve)));
  const root = document.getElementById(rootId);
  if (!root) throw new Error(`missing fidelity root ${rootId}`);
  const rootRect = root.getBoundingClientRect();
  const px = value => {
    const parsed = Number.parseFloat(value);
    return Number.isFinite(parsed) ? parsed : 0;
  };
  const rect = value => [
    value.left - rootRect.left, value.top - rootRect.top, value.width, value.height
  ].map(value => Math.round(value * 1000000) / 1000000);
  const intersect = (left, right) => {
    const x = Math.max(left.left, right.left);
    const y = Math.max(left.top, right.top);
    const r = Math.min(left.right, right.right);
    const b = Math.min(left.bottom, right.bottom);
    return {left:x, top:y, right:Math.max(x, r), bottom:Math.max(y, b),
            width:Math.max(0, r-x), height:Math.max(0, b-y)};
  };
  const clippedRect = element => {
    let result = element.getBoundingClientRect();
    for (let parent = element.parentElement; parent; parent = parent.parentElement) {
      const style = getComputedStyle(parent);
      if ([style.overflowX, style.overflowY].some(
            value => ['hidden', 'clip', 'scroll', 'auto'].includes(value))) {
        result = intersect(result, parent.getBoundingClientRect());
      }
    }
    return intersect(result, {left:0, top:0, right:innerWidth, bottom:innerHeight,
                              width:innerWidth, height:innerHeight});
  };
  const color = value => {
    const numbers = (value.match(/[\d.]+/g) || []).map(Number);
    if (numbers.length < 3) return [0,0,0,0];
    const alpha = numbers.length > 3 ? Math.round(numbers[3] * 255) : 255;
    return [Math.round(numbers[0]), Math.round(numbers[1]), Math.round(numbers[2]), alpha];
  };
  const directText = element => Array.from(element.childNodes)
    .filter(node => node.nodeType === Node.TEXT_NODE)
    .map(node => node.textContent || '').join(' ').replace(/\s+/g, ' ').trim();
  const effectiveVisibility = element => {
    let visible = true;
    for (let current = element; current; current = current.parentElement) {
      const currentStyle = getComputedStyle(current);
      if (currentStyle.display === 'none' || currentStyle.visibility === 'hidden' ||
          px(currentStyle.opacity) === 0) visible = false;
    }
    return {visible};
  };
  const baselines = (element, text) => {
    if (!text || ['INPUT', 'TEXTAREA', 'SELECT', 'IMG'].includes(element.tagName)) return [];
    const saved = element.innerHTML;
    element.textContent = text;
    const textNode = element.firstChild;
    const lines = [];
    let previousTop = null;
    for (let index = 0; index < text.length; ++index) {
      const range = document.createRange();
      range.setStart(textNode, index);
      range.setEnd(textNode, index + 1);
      const candidate = range.getClientRects()[0];
      if (!candidate || candidate.width === 0) continue;
      if (previousTop === null || Math.abs(candidate.top - previousTop) > 0.5) {
        lines.push({top:candidate.top, offset:index});
        previousTop = candidate.top;
      } else {
        lines[lines.length - 1].offset = index;
      }
    }
    const result = [];
    for (const line of [...lines].reverse()) {
      const marker = document.createElement('span');
      marker.style.cssText = 'display:inline-block;width:0;height:0;padding:0;margin:0;border:0;vertical-align:baseline;';
      const range = document.createRange();
      range.setStart(textNode, line.offset);
      range.collapse(true);
      range.insertNode(marker);
      result.unshift(Math.round((marker.getBoundingClientRect().top - rootRect.top) * 1000000) / 1000000);
    }
    element.innerHTML = saved;
    return result;
  };
  const fillKinds = (style, element) => {
    const images = [];
    const image = style.backgroundImage;
    for (const match of image.matchAll(/(repeating-linear-gradient|linear-gradient|radial-gradient|url)\(/g)) {
      images.push(match[1] === 'repeating-linear-gradient' ? 'repeating_linear_gradient' :
                  match[1] === 'linear-gradient' ? 'linear_gradient' :
                  match[1] === 'radial-gradient' ? 'radial_gradient' : 'image');
    }
    images.reverse();
    const result = color(style.backgroundColor)[3] !== 0 ? ['solid'] : [];
    result.push(...images);
    if (element.dataset.wfImageSrc) result.push('image');
    return result;
  };
  const shadowKinds = value => {
    if (!value || value === 'none') return [];
    let depth = 0;
    let token = '';
    const tokens = [];
    for (const character of value) {
      if (character === '(') depth++;
      if (character === ')') depth--;
      if (character === ',' && depth === 0) { tokens.push(token); token=''; }
      else token += character;
    }
    if (token) tokens.push(token);
    return tokens.map(token => ({inset:/\binset\b/.test(token), raw:token.trim()}));
  };
  const nodes = Array.from(root.querySelectorAll('[id]'));
  if (root.id) nodes.unshift(root);
  const records = nodes.map(element => {
    const style = getComputedStyle(element);
    const text = directText(element);
    let ancestor = element.parentElement;
    while (ancestor && !ancestor.id) ancestor = ancestor.parentElement;
    const border = edge => ({
      width:px(style[`border${edge}Width`]),
      color:color(style[`border${edge}Color`]),
      style:style[`border${edge}Style`]
    });
    const tag = element.tagName.toLowerCase();
    const inferred = tag === 'button' ? 'button' :
      (['p','span','strong','small','h1','h2'].includes(tag) ? 'label' : 'panel');
    const visibility = effectiveVisibility(element);
    const conditionalOverflowActuator =
      element.dataset.wfOverflowActuator === 'true';
    return {
      id:element.id,
      parent_id:ancestor ? ancestor.id : null,
      kind:kindById[element.id] || element.dataset.wfControl || inferred,
      topology:element.dataset.wfConnectedAxis ? {
        connected_axis:element.dataset.wfConnectedAxis,
        connected_members:Array.from(element.children).filter(child => child.id)
          .map(child => child.id)
      } : null,
      resource:element.dataset.wfImageSrc ? {
        list:element.dataset.wfImageList, key:element.dataset.wfImageKey,
        source_1x:element.dataset.wfImageSrc,
        source_2x:element.getAttribute('data-wf-image-src-2x')
      } : null,
      bounds:visibility.visible ? rect(element.getBoundingClientRect()) : [0,0,0,0],
      clip:visibility.visible ? rect(clippedRect(element)) : [0,0,0,0],
      state:{
        authored_visible:conditionalOverflowActuator ||
          (style.display !== 'none' && style.visibility !== 'hidden'),
        visible:visibility.visible,
        enabled:!element.matches(':disabled'),
        focused:((interactionState === 'focused' || interactionState === 'pressed') &&
          interactionTarget === element.id) ||
          (document.activeElement === element && element.tabIndex >= 0),
        hovered:(interactionState === 'hover' || interactionState === 'pressed') &&
          interactionTarget === element.id,
        pressed:interactionState === 'pressed' && interactionTarget === element.id,
        window_active:requestedWindowActive,
        layout_collapsed:conditionalOverflowActuator && !visibility.visible,
        visual_status:interactionTarget === element.id ?
          ({hover:'hot', pressed:'pressed', disabled:'disabled'}[interactionState] || 'normal') :
          'normal'
      },
      typography:text ? (() => {
        const measuredBaselines = baselines(element, text);
        return {
        text, baseline:measuredBaselines.length ? measuredBaselines[0] : null,
        baselines:measuredBaselines, family:style.fontFamily,
        runs:[], resolution_status:'browser-computed', font_size:px(style.fontSize),
        weight:Number.parseInt(style.fontWeight, 10) || 400,
        italic:style.fontStyle === 'italic' || style.fontStyle === 'oblique',
        letter_spacing:style.letterSpacing === 'normal' ? 0 : px(style.letterSpacing),
        line_height:style.lineHeight === 'normal' ? null : px(style.lineHeight)
      }; })() : null,
      material:{
        fill_kinds:fillKinds(style, element), solid_color:color(style.backgroundColor),
        shadows:shadowKinds(style.boxShadow),
        borders:{top:border('Top'), right:border('Right'), bottom:border('Bottom'), left:border('Left')},
        corner_radii:[px(style.borderTopLeftRadius), px(style.borderTopRightRadius),
                      px(style.borderBottomRightRadius), px(style.borderBottomLeftRadius)],
        keylines:(element.dataset.wfKeylines || '').split(';').map(value => value.trim())
          .filter(Boolean).map(value => {
            const [edge, packed, width, inset] = value.split(/\s+/);
            const hex = packed.slice(1) + (packed.length === 7 ? 'ff' : '');
            return {edge, color:[0,2,4,6].map(index => Number.parseInt(hex.slice(index,index+2),16)),
                    width:Number(width), inset:Number(inset)};
          }),
        raw:{background_color:style.backgroundColor, background_image:style.backgroundImage,
             background_repeat:style.backgroundRepeat, background_size:style.backgroundSize,
             background_position:style.backgroundPosition, box_shadow:style.boxShadow,
             opacity:style.opacity, filter:style.filter}
      }
    };
  }).sort((left, right) => left.id.localeCompare(right.id));
  const payload = {
    schema:'web.forms.fidelity-snapshot/0.1-experimental', producer:'chromium-computed-style',
    source_digest:__SOURCE_DIGEST__, root_id:rootId, state:stateName,
    environment:{viewport:[innerWidth,innerHeight], device_scale:devicePixelRatio,
      text_scale:textScale, high_contrast:matchMedia('(forced-colors: active)').matches,
      reduced_motion:matchMedia('(prefers-reduced-motion: reduce)').matches,
      window_active:requestedWindowActive, theme_id:'browser-computed',
      root_origin:[rootRect.left, rootRect.top],
      user_agent:navigator.userAgent, font_status:document.fonts.status}, nodes:records
  };
  return payload;
})()
"""


class _CdpConnection:
    def __init__(self, host: str, port: int, path: str) -> None:
        self._socket = socket.create_connection((host, port), timeout=5.0)
        key = base64.b64encode(os.urandom(16)).decode("ascii")
        request = (
            f"GET {path} HTTP/1.1\r\n"
            f"Host: {host}:{port}\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            f"Sec-WebSocket-Key: {key}\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "Origin: http://localhost\r\n\r\n"
        )
        self._socket.sendall(request.encode("ascii"))
        response = b""
        while b"\r\n\r\n" not in response:
            response += self._socket.recv(4096)
        if not response.startswith(b"HTTP/1.1 101"):
            raise RuntimeError(response.decode("utf-8", errors="replace"))
        self._next_id = 1

    def close(self) -> None:
        self._socket.close()

    def _send_text(self, value: str) -> None:
        payload = value.encode("utf-8")
        mask = os.urandom(4)
        header = bytearray([0x81])
        length = len(payload)
        if length < 126:
            header.append(0x80 | length)
        elif length <= 0xFFFF:
            header.append(0x80 | 126)
            header.extend(struct.pack("!H", length))
        else:
            header.append(0x80 | 127)
            header.extend(struct.pack("!Q", length))
        masked = bytes(value ^ mask[index % 4] for index, value in enumerate(payload))
        self._socket.sendall(bytes(header) + mask + masked)

    def _read_exact(self, size: int) -> bytes:
        result = bytearray()
        while len(result) < size:
            block = self._socket.recv(size - len(result))
            if not block:
                raise RuntimeError("Chrome closed the DevTools socket")
            result.extend(block)
        return bytes(result)

    def _receive_text(self) -> str:
        fragments = bytearray()
        while True:
            first, second = self._read_exact(2)
            final = bool(first & 0x80)
            opcode = first & 0x0F
            length = second & 0x7F
            if length == 126:
                length = struct.unpack("!H", self._read_exact(2))[0]
            elif length == 127:
                length = struct.unpack("!Q", self._read_exact(8))[0]
            mask = self._read_exact(4) if second & 0x80 else b""
            payload = self._read_exact(length)
            if mask:
                payload = bytes(
                    value ^ mask[index % 4] for index, value in enumerate(payload)
                )
            if opcode == 0x8:
                raise RuntimeError("Chrome closed the DevTools websocket")
            if opcode == 0x9:
                self._send_control(0xA, payload)
                continue
            if opcode not in (0x0, 0x1):
                continue
            fragments.extend(payload)
            if final:
                return fragments.decode("utf-8")

    def _send_control(self, opcode: int, payload: bytes) -> None:
        mask = os.urandom(4)
        header = bytes([0x80 | opcode, 0x80 | len(payload)])
        masked = bytes(value ^ mask[index % 4] for index, value in enumerate(payload))
        self._socket.sendall(header + mask + masked)

    def call(self, method: str, params: dict[str, Any] | None = None) -> dict[str, Any]:
        identifier = self._next_id
        self._next_id += 1
        self._send_text(json.dumps({"id": identifier, "method": method, "params": params or {}}))
        while True:
            message = json.loads(self._receive_text())
            if message.get("id") != identifier:
                continue
            if "error" in message:
                raise RuntimeError(f"{method}: {message['error']}")
            return message.get("result", {})


def _wait_for_debug_target(profile: Path) -> tuple[int, str]:
    active_port = profile / "DevToolsActivePort"
    deadline = time.monotonic() + 8.0
    while time.monotonic() < deadline:
        if active_port.is_file():
            lines = active_port.read_text(encoding="utf-8").splitlines()
            if lines:
                port = int(lines[0])
                with urlopen(f"http://127.0.0.1:{port}/json/list", timeout=2.0) as response:
                    targets = json.loads(response.read().decode("utf-8"))
                page = next((item for item in targets if item.get("type") == "page"), None)
                if page and page.get("webSocketDebuggerUrl"):
                    path = re.sub(r"^ws://[^/]+", "", page["webSocketDebuggerUrl"])
                    return port, path
        time.sleep(0.05)
    raise RuntimeError("Chrome did not publish a DevTools page target")


def _chrome_path(explicit: Path | None = None) -> Path:
    if explicit is not None:
        if explicit.is_file():
            return explicit
        fail("WFV004", "explicit Chromium executable does not exist", str(explicit))
    candidates = [
        Path("/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"),
        Path("/Applications/Chromium.app/Contents/MacOS/Chromium"),
        Path("/Applications/Brave Browser.app/Contents/MacOS/Brave Browser"),
    ]
    if os.name == "nt":
        for variable in ("PROGRAMFILES", "PROGRAMFILES(X86)", "LOCALAPPDATA"):
            root = os.environ.get(variable)
            if root:
                candidates.extend(Path(root) / relative for relative in (
                    "Google/Chrome/Application/chrome.exe",
                    "Chromium/Application/chrome.exe",
                    "BraveSoftware/Brave-Browser/Application/brave.exe",
                    "Microsoft/Edge/Application/msedge.exe",
                ))
    for name in ("google-chrome", "chromium", "chromium-browser", "brave-browser",
                 "chrome", "brave", "msedge"):
        found = shutil.which(name)
        if found:
            candidates.append(Path(found))
    for candidate in candidates:
        if candidate is not None and str(candidate) and candidate.is_file():
            return candidate
    fail("WFV004", "no Chromium executable was found", "<browser>")
    raise AssertionError("unreachable")


@contextmanager
def _capture_directory():
    directory = tempfile.TemporaryDirectory(prefix="web-forms-fidelity-")
    try:
        yield Path(directory.name)
    finally:
        # Windows browser children can release profile handles shortly after
        # the parent exits. Retry only that sharing failure, never ignore it.
        deadline = time.monotonic() + 5.0
        while True:
            try:
                directory.cleanup()
                break
            except PermissionError:
                if time.monotonic() >= deadline:
                    raise
                time.sleep(0.05)


def capture_browser_snapshot(
    source: Path,
    *,
    styles: list[Path] | None = None,
    root_id: str | None = None,
    state: str = "reference",
    viewport: tuple[int, int] = (1080, 720),
    device_scale: float = 1.0,
    text_scale: float = 1.0,
    window_active: bool = True,
    interaction: str = "reference",
    interaction_target: str = "",
    reduced_motion: bool = False,
    chrome: Path | None = None,
) -> dict[str, Any]:
    ir = compile_source(source, styles or None)
    selected_root = root_id or next(
        node["id"]
        for node in ir["document"]["nodes"]
        if node.get("runtime") and node.get("id")
    )
    document_nodes = ir["document"]["nodes"]
    runtime_nodes = [node for node in document_nodes if node.get("runtime")]
    runtime_indexes = {node["index"] for node in runtime_nodes}
    child_counts = {node["index"]: 0 for node in runtime_nodes}
    for node in runtime_nodes:
        parent = node.get("parent")
        while parent is not None and parent not in runtime_indexes:
            parent = document_nodes[parent].get("parent")
        if parent is not None:
            child_counts[parent] += 1
    kind_by_id = {
        node["id"]: (
            node.get("control")
            if node.get("control") in {"button", "dropdown-button"}
            else "label"
            if child_counts[node["index"]] == 0
            else node.get("attributes", {}).get("data-wf-control", "panel")
        )
        for node in runtime_nodes
    }
    source_text = source.read_text(encoding="utf-8")
    base_tag = f'<base href="{html_module.escape(source.parent.resolve().as_uri())}/">'
    source_text = source_text.replace("<head>", "<head>\n  " + base_tag, 1)
    if styles:
        override = "\n<style data-wf-fidelity-override>\n" + "\n".join(
            path.read_text(encoding="utf-8") for path in styles
        ) + "\n</style>\n"
        source_text = source_text.replace("</head>", override + "</head>", 1)
    expression = (
        _CAPTURE_EXPRESSION.replace("__ROOT_ID__", json.dumps(selected_root))
        .replace("__STATE_NAME__", json.dumps(state))
        .replace("__SOURCE_DIGEST__", json.dumps(ir["source"]["digest"]))
        .replace("__KIND_BY_ID__", json.dumps(kind_by_id, sort_keys=True))
        .replace("__TEXT_SCALE__", json.dumps(text_scale))
        .replace("__WINDOW_ACTIVE__", json.dumps(window_active))
        .replace("__INTERACTION_STATE__", json.dumps(interaction))
        .replace("__INTERACTION_TARGET__", json.dumps(interaction_target))
    )
    executable = _chrome_path(chrome)
    width, height = viewport
    if (width <= 0 or height <= 0 or device_scale <= 0 or
            not 0.5 <= text_scale <= 4.0):
        fail("WFV005", "viewport/device scale must be positive and text scale bounded", str(source))
    if interaction not in {"reference", "hover", "pressed", "focused", "disabled"}:
        fail("WFV005", "interaction state is outside the fidelity vocabulary", str(source))
    if interaction != "reference" and not interaction_target:
        fail("WFV005", "interactive fidelity state requires a target stable ID", str(source))
    with _capture_directory() as temporary:
        instrumented = temporary / "specimen.html"
        instrumented.write_text(source_text, encoding="utf-8")
        profile = temporary / "profile"
        command = [
            str(executable),
            "--headless=new",
            "--disable-gpu",
            "--disable-background-networking",
            "--disable-component-update",
            "--disable-default-apps",
            "--disable-extensions",
            "--disable-component-extensions-with-background-pages",
            "--disable-features=Translate,MediaRouter,OptimizationHints",
            "--disable-sync",
            "--no-first-run",
            "--no-default-browser-check",
            "--hide-scrollbars",
            "--allow-file-access-from-files",
            "--force-color-profile=srgb",
            f"--window-size={width},{height}",
            "--remote-debugging-port=0",
            "--remote-allow-origins=*",
            f"--user-data-dir={profile}",
            "about:blank",
        ]
        process = subprocess.Popen(
            command,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        connection: _CdpConnection | None = None
        try:
            port, websocket_path = _wait_for_debug_target(profile)
            connection = _CdpConnection("127.0.0.1", port, websocket_path)
            browser_version = connection.call("Browser.getVersion")
            connection.call("Page.enable")
            connection.call("DOM.enable")
            connection.call("CSS.enable")
            connection.call(
                "Emulation.setDeviceMetricsOverride",
                {
                    "width": width,
                    "height": height,
                    "deviceScaleFactor": device_scale,
                    "mobile": False,
                    "screenWidth": width,
                    "screenHeight": height,
                },
            )
            connection.call(
                "Emulation.setEmulatedMedia",
                {
                    "media": "screen",
                    "features": [
                        {"name": "prefers-reduced-motion", "value": "no-preference"},
                        {"name": "prefers-color-scheme", "value": "light"},
                    ],
                },
            )
            connection.call(
                "Emulation.setFocusEmulationEnabled", {"enabled": window_active}
            )
            connection.call("Page.navigate", {"url": instrumented.as_uri()})
            deadline = time.monotonic() + 8.0
            ready = False
            while time.monotonic() < deadline:
                result = connection.call(
                    "Runtime.evaluate",
                    {"expression": "document.readyState", "returnByValue": True},
                )
                if result.get("result", {}).get("value") == "complete":
                    ready = True
                    break
                time.sleep(0.05)
            if not ready:
                raise RuntimeError("Chrome did not complete specimen navigation")
            if reduced_motion:
                connection.call(
                    "Emulation.setEmulatedMedia",
                    {
                        "media": "screen",
                        "features": [
                            {"name": "prefers-reduced-motion", "value": "reduce"},
                            {"name": "prefers-color-scheme", "value": "light"},
                        ],
                    },
                )
            if interaction != "reference":
                target_result = connection.call(
                    "Runtime.evaluate",
                    {
                        "expression": (
                            "document.getElementById(" +
                            json.dumps(interaction_target) + ")"
                        ),
                    },
                )
                remote = target_result.get("result", {})
                object_id = remote.get("objectId")
                if not object_id or remote.get("subtype") == "null":
                    raise RuntimeError(
                        f"missing interaction target {interaction_target!r}"
                    )
                document_node = connection.call("DOM.getDocument", {"depth": 0})
                document_id = document_node.get("root", {}).get("nodeId")
                requested_node = connection.call(
                    "DOM.querySelector",
                    {
                        "nodeId": document_id,
                        "selector": f'[id="{interaction_target}"]',
                    },
                )
                node_id = requested_node.get("nodeId")
                if not node_id:
                    raise RuntimeError("Chrome did not resolve interaction target node")
                pseudo_classes = {
                    "hover": ["hover"],
                    "pressed": ["hover", "active"],
                    "focused": ["focus", "focus-visible"],
                    "disabled": [],
                }[interaction]
                if pseudo_classes:
                    connection.call(
                        "CSS.forcePseudoState",
                        {"nodeId": node_id, "forcedPseudoClasses": pseudo_classes},
                    )
                if interaction == "focused":
                    connection.call(
                        "Runtime.callFunctionOn",
                        {
                            "objectId": object_id,
                            "functionDeclaration": "function(){ this.focus(); }",
                        },
                    )
                elif interaction == "disabled":
                    connection.call(
                        "Runtime.callFunctionOn",
                        {
                            "objectId": object_id,
                            "functionDeclaration": "function(){ this.disabled = true; }",
                        },
                    )
            result = connection.call(
                "Runtime.evaluate",
                {
                    "expression": expression,
                    "awaitPromise": True,
                    "returnByValue": True,
                    "userGesture": True,
                },
            )
            if result.get("exceptionDetails"):
                raise RuntimeError(str(result["exceptionDetails"]))
            snapshot = result.get("result", {}).get("value")
            if not isinstance(snapshot, dict):
                raise RuntimeError(f"Chrome returned no fidelity value: {result}")
            screenshot_result = connection.call(
                "Page.captureScreenshot",
                {"format": "png", "fromSurface": True, "captureBeyondViewport": False},
            )
            screenshot = base64.b64decode(
                screenshot_result.get("data", ""), validate=True
            )
            _attach_browser_raster_probes(snapshot, screenshot)
        except (OSError, RuntimeError, ValueError) as error:
            fail("WFV006", f"Chromium capture failed: {error}", str(source))
        finally:
            if connection is not None:
                try:
                    connection.call("Browser.close")
                except (OSError, RuntimeError, ValueError):
                    # Some Chromium builds close the socket before replying.
                    pass
                connection.close()
            try:
                process.wait(timeout=2.0)
            except subprocess.TimeoutExpired:
                process.terminate()
                try:
                    process.wait(timeout=2.0)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=2.0)
    snapshot["environment"]["browser"] = browser_version["product"]
    snapshot["environment"]["browser_executable"] = str(executable.resolve())
    snapshot["environment"]["browser_version"] = browser_version
    snapshot["environment"]["requested_viewport"] = [width, height]
    validate_fidelity_snapshot(snapshot, str(source))
    return snapshot


def validate_fidelity_snapshot(snapshot: dict[str, Any], path: str) -> None:
    if snapshot.get("schema") != SNAPSHOT_SCHEMA:
        fail("WFV009", f"unsupported fidelity snapshot schema {snapshot.get('schema')!r}", path)
    for field in ("producer", "root_id", "state", "environment", "nodes"):
        if field not in snapshot:
            fail("WFV010", f"fidelity snapshot omits {field!r}", path)
    if not isinstance(snapshot["nodes"], list):
        fail("WFV011", "fidelity snapshot nodes must be an array", path)
    identifiers: set[str] = set()
    for index, node in enumerate(snapshot["nodes"]):
        identifier = node.get("id") if isinstance(node, dict) else None
        if not isinstance(identifier, str) or not identifier:
            fail("WFV012", f"fidelity node {index} has no stable id", path)
        if identifier in identifiers:
            fail("WFV013", f"duplicate fidelity stable id {identifier!r}", path)
        identifiers.add(identifier)
        for field in ("bounds", "clip"):
            values = node.get(field)
            if not isinstance(values, list) or len(values) != 4 or not all(
                isinstance(value, (int, float)) for value in values
            ):
                fail("WFV014", f"node {identifier!r} has invalid {field}", path)
    if snapshot["root_id"] not in identifiers:
        fail("WFV015", f"root {snapshot['root_id']!r} is absent from nodes", path)


def read_fidelity_snapshot(path: Path) -> dict[str, Any]:
    try:
        snapshot = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError) as error:
        fail("WFV016", f"cannot read fidelity snapshot: {error}", str(path))
    validate_fidelity_snapshot(snapshot, str(path))
    return snapshot


def write_json(value: dict[str, Any], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def _delta(left: list[Any], right: list[Any]) -> list[float]:
    return [_rounded(float(rvalue) - float(lvalue)) for lvalue, rvalue in zip(left, right)]


def _over(values: list[float], tolerance: float) -> bool:
    return any(abs(value) > tolerance for value in values)


def _color_distance(left: list[Any] | None, right: list[Any] | None) -> int | None:
    if left is None or right is None or len(left) != 4 or len(right) != 4:
        return None
    return sum(abs(int(lvalue) - int(rvalue)) for lvalue, rvalue in zip(left, right))


def _primary_family(value: Any) -> str:
    return str(value or "").split(",", 1)[0].strip().strip('"\'')


def compare_fidelity_snapshots(
    browser: dict[str, Any],
    native: dict[str, Any],
    tolerance: FidelityTolerance = FidelityTolerance(),
) -> dict[str, Any]:
    validate_fidelity_snapshot(browser, "<browser-snapshot>")
    validate_fidelity_snapshot(native, "<native-snapshot>")
    browser_nodes = {item["id"]: item for item in browser["nodes"]}
    native_nodes = {item["id"]: item for item in native["nodes"]}
    all_ids = sorted(set(browser_nodes) | set(native_nodes))
    records: list[dict[str, Any]] = []
    category_counts = {
        "structure": 0,
        "geometry": 0,
        "typography": 0,
        "material": 0,
        "raster": 0,
        "state": 0,
    }
    for identifier in all_ids:
        left = browser_nodes.get(identifier)
        right = native_nodes.get(identifier)
        differences: dict[str, list[dict[str, Any]]] = {
            key: [] for key in category_counts
        }
        if left is None or right is None:
            differences["structure"].append(
                {"field": "presence", "browser": left is not None, "native": right is not None}
            )
        else:
            for field in ("parent_id", "kind", "topology", "resource"):
                if left.get(field) != right.get(field):
                    differences["structure"].append(
                        {"field": field, "browser": left.get(field), "native": right.get(field)}
                    )
            for field, allowed in (("bounds", tolerance.geometry), ("clip", tolerance.clip)):
                delta = _delta(left[field], right[field])
                if _over(delta, allowed):
                    differences["geometry"].append(
                        {"field": field, "delta": delta, "tolerance": allowed,
                         "browser": left[field], "native": right[field]}
                    )
            both_visible = bool(left.get("state", {}).get("visible")) and bool(
                right.get("state", {}).get("visible")
            )
            left_text = left.get("typography") if both_visible else None
            right_text = right.get("typography") if both_visible else None
            if (left_text is None) != (right_text is None):
                differences["typography"].append(
                    {"field": "presence", "browser": left_text is not None,
                     "native": right_text is not None}
                )
            elif left_text is not None and right_text is not None:
                if left_text.get("text") != right_text.get("text"):
                    differences["typography"].append(
                        {"field": "text", "browser": left_text.get("text"),
                         "native": right_text.get("text")}
                    )
                for field, allowed in (
                    ("baseline", tolerance.baseline),
                    ("font_size", tolerance.font_size),
                    ("letter_spacing", tolerance.letter_spacing),
                ):
                    lvalue, rvalue = left_text.get(field), right_text.get(field)
                    if lvalue is not None and rvalue is not None:
                        value_delta = _rounded(float(rvalue) - float(lvalue))
                        if abs(value_delta) > allowed:
                            differences["typography"].append(
                                {"field": field, "delta": value_delta, "tolerance": allowed,
                                 "browser": lvalue, "native": rvalue}
                            )
                left_baselines = left_text.get("baselines", [])
                right_baselines = right_text.get("baselines", [])
                if left_baselines or right_baselines:
                    if len(left_baselines) != len(right_baselines):
                        differences["typography"].append(
                            {"field": "baseline_count", "browser": len(left_baselines),
                             "native": len(right_baselines)}
                        )
                    else:
                        baseline_deltas = _delta(left_baselines, right_baselines)
                        if _over(baseline_deltas, tolerance.baseline):
                            differences["typography"].append(
                                {"field": "baselines", "delta": baseline_deltas,
                                 "tolerance": tolerance.baseline,
                                 "browser": left_baselines, "native": right_baselines}
                            )
                for field in ("weight", "italic"):
                    if left_text.get(field) != right_text.get(field):
                        differences["typography"].append(
                            {"field": field, "browser": left_text.get(field),
                             "native": right_text.get(field)}
                        )
                browser_family = _primary_family(left_text.get("family"))
                native_family = _primary_family(right_text.get("family"))
                if browser_family and native_family and browser_family != native_family:
                    differences["typography"].append(
                        {"field": "primary_family", "browser": browser_family,
                         "native": native_family}
                    )
                if (
                    str(native.get("producer", "")).startswith("gui.forms")
                    and right_text.get("resolution_status") != "exact"
                ):
                    differences["typography"].append(
                        {"field": "native_resolution_status",
                         "native": right_text.get("resolution_status")}
                    )
                if str(native.get("producer", "")).startswith("gui.forms"):
                    runs = right_text.get("runs", [])
                    if not runs or not right_text.get(
                        "resolved_run_coverage_exact", False
                    ):
                        differences["typography"].append(
                            {"field": "native_resolved_run_coverage",
                             "native_runs": runs}
                        )
            left_material = left.get("material", {}) if both_visible else {}
            right_material = right.get("material", {}) if both_visible else {}
            if left_material.get("fill_kinds") != right_material.get("fill_kinds"):
                differences["material"].append(
                    {"field": "fill_kinds", "browser": left_material.get("fill_kinds"),
                     "native": right_material.get("fill_kinds")}
                )
            distance = _color_distance(
                left_material.get("solid_color"), right_material.get("solid_color")
            )
            if distance is not None and distance > tolerance.color_distance:
                differences["material"].append(
                    {"field": "solid_color", "distance": distance,
                     "tolerance": tolerance.color_distance,
                     "browser": left_material.get("solid_color"),
                     "native": right_material.get("solid_color")}
                )
            left_shadows = [bool(item.get("inset")) for item in left_material.get("shadows", [])]
            right_shadows = [bool(item.get("inset")) for item in right_material.get("shadows", [])]
            if left_shadows != right_shadows:
                differences["material"].append(
                    {"field": "shadow_inset_sequence", "browser": left_shadows,
                     "native": right_shadows}
                )
            if left_material.get("keylines", []) != right_material.get("keylines", []):
                differences["material"].append(
                    {"field": "keylines", "browser": left_material.get("keylines", []),
                     "native": right_material.get("keylines", [])}
                )
            for index, edge in enumerate(("top", "right", "bottom", "left")):
                lborder = left_material.get("borders", {}).get(edge, {})
                rborder = right_material.get("borders", {}).get(edge, {})
                border_delta = _rounded(float(rborder.get("width", 0.0)) - float(lborder.get("width", 0.0)))
                if abs(border_delta) > tolerance.border_width:
                    differences["material"].append(
                        {"field": f"border.{edge}.width", "delta": border_delta,
                         "tolerance": tolerance.border_width,
                         "browser": lborder.get("width", 0.0), "native": rborder.get("width", 0.0)}
                    )
                lradii = left_material.get("corner_radii", [0, 0, 0, 0])
                rradii = right_material.get("corner_radii", [0, 0, 0, 0])
                radius_delta = _rounded(float(rradii[index]) - float(lradii[index]))
                if abs(radius_delta) > tolerance.corner_radius:
                    differences["material"].append(
                        {"field": f"corner_radius.{edge}", "delta": radius_delta,
                         "tolerance": tolerance.corner_radius,
                         "browser": lradii[index], "native": rradii[index]}
                    )
            for field in (
                "authored_visible", "visible", "enabled", "focused", "hovered",
                "pressed", "layout_collapsed",
            ):
                if left.get("state", {}).get(field) != right.get("state", {}).get(field):
                    differences["state"].append(
                        {"field": field, "browser": left.get("state", {}).get(field),
                         "native": right.get("state", {}).get(field)}
                    )
            left_probes = left.get("raster_probes", []) if both_visible else []
            right_probes = right.get("raster_probes", []) if both_visible else []
            if len(left_probes) != len(right_probes):
                differences["raster"].append(
                    {"field": "probe_count", "browser": len(left_probes),
                     "native": len(right_probes)}
                )
            else:
                for probe_index, (left_probe, right_probe) in enumerate(
                    zip(left_probes, right_probes)
                ):
                    if left_probe[:2] != right_probe[:2]:
                        differences["raster"].append(
                            {"field": f"probe.{probe_index}.position",
                             "browser": left_probe[:2], "native": right_probe[:2]}
                        )
                        continue
                    probe_distance = _color_distance(left_probe[2], right_probe[2])
                    if (probe_distance is None or
                            probe_distance > tolerance.raster_color_distance):
                        differences["raster"].append(
                            {"field": f"probe.{probe_index}.color",
                             "distance": probe_distance,
                             "tolerance": tolerance.raster_color_distance,
                             "browser": left_probe[2], "native": right_probe[2]}
                        )
        failed_categories = [key for key, values in differences.items() if values]
        for key in failed_categories:
            category_counts[key] += 1
        records.append(
            {"id": identifier, "matched": not failed_categories, "differences": differences}
        )
    digest_match = bool(browser.get("source_digest")) and (
        browser.get("source_digest") == native.get("source_digest")
    )
    state_match = browser.get("state") == native.get("state")
    report = {
        "schema": REPORT_SCHEMA,
        "status": "match" if not any(category_counts.values()) and digest_match and state_match else "different",
        "browser_producer": browser.get("producer"),
        "native_producer": native.get("producer"),
        "source_digest_match": digest_match,
        "state_match": state_match,
        "browser_state": browser.get("state"),
        "native_state": native.get("state"),
        "tolerances": tolerance.__dict__,
        "measurements": {
            "browser_nodes": len(browser_nodes),
            "native_nodes": len(native_nodes),
            "matched_nodes": sum(1 for item in records if item["matched"]),
            "different_nodes": sum(1 for item in records if not item["matched"]),
            "category_node_counts": category_counts,
            "maximum_absolute_bounds_delta": max(
                (
                    abs(value)
                    for item in records
                    for difference in item["differences"]["geometry"]
                    if difference.get("field") == "bounds"
                    for value in difference.get("delta", [])
                ),
                default=0.0,
            ),
            "maximum_absolute_clip_delta": max(
                (
                    abs(value)
                    for item in records
                    for difference in item["differences"]["geometry"]
                    if difference.get("field") == "clip"
                    for value in difference.get("delta", [])
                ),
                default=0.0,
            ),
            "maximum_absolute_baseline_delta": max(
                (
                    abs(float(difference["delta"]))
                    for item in records
                    for difference in item["differences"]["typography"]
                    if difference.get("field") == "baseline" and "delta" in difference
                ),
                default=0.0,
            ),
            "maximum_raster_color_distance": max(
                (
                    int(difference["distance"])
                    for item in records
                    for difference in item["differences"]["raster"]
                    if difference.get("field", "").endswith(".color")
                    and difference.get("distance") is not None
                ),
                default=0,
            ),
        },
        "nodes": records,
    }
    canonical = json.dumps(report, sort_keys=True, separators=(",", ":")).encode("utf-8")
    report["report_digest"] = hashlib.sha256(canonical).hexdigest()
    return report
