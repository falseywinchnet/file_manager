#!/usr/bin/env python3
"""Apply a verified pointer-arrow rewrite plan to the authoritative tree."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import tempfile


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--plan", type=Path, required=True)
    return parser.parse_args()


def apply_file(path: Path, expected_size: int,
               records: list[dict[str, object]]) -> int:
    original = path.read_bytes()
    if len(original) != expected_size:
        raise RuntimeError(
            f"{path}: size changed after AST capture "
            f"({len(original)} != {expected_size})")

    edits: list[tuple[int, int, bytes]] = []
    for record in records:
        base_offset = int(record["baseOffset"])
        operator_offset = int(record["operatorOffset"])
        if original[operator_offset:operator_offset + 2] != b"->":
            raise RuntimeError(
                f"{path}:{operator_offset}: expected pointer arrow")
        edits.append((base_offset, 0, b"(*"))
        edits.append((operator_offset, 2, b")."))

    rewritten = original
    for offset, length, replacement in sorted(
            edits, key=lambda edit: (edit[0], edit[1]), reverse=True):
        rewritten = rewritten[:offset] + replacement + rewritten[offset + length:]

    mode = path.stat().st_mode
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "wb") as temporary:
            temporary.write(rewritten)
            temporary.flush()
            os.fsync(temporary.fileno())
        os.chmod(temporary_name, mode)
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise
    return len(records)


def main() -> int:
    arguments = parse_arguments()
    source_root = arguments.source_root.resolve()
    plan = json.loads(arguments.plan.read_text(encoding="utf-8"))
    if plan.get("schema") != "gui.forms.pointer-arrow-rewrites/v1":
        raise RuntimeError("unexpected pointer-arrow rewrite schema")

    by_path: dict[str, list[dict[str, object]]] = {}
    sizes: dict[str, int] = {}
    for record in plan["rewrites"]:
        relative_path = str(record["path"])
        file_size = int(record["fileSize"])
        if relative_path in sizes and sizes[relative_path] != file_size:
            raise RuntimeError(f"{relative_path}: inconsistent captured sizes")
        sizes[relative_path] = file_size
        by_path.setdefault(relative_path, []).append(record)

    applied = 0
    for relative_path in sorted(by_path):
        path = source_root / relative_path
        applied += apply_file(path, sizes[relative_path], by_path[relative_path])
    print(f"applied {applied} pointer-arrow rewrites in {len(by_path)} files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
