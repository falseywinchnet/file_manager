#!/usr/bin/env python3
"""Apply the mechanically safe subset of an AST-resolved auto-type plan."""

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

    edits: list[tuple[int, bytes]] = []
    for record in records:
        token_offset = int(record["tokenOffset"])
        if original[token_offset:token_offset + 4] != b"auto":
            raise RuntimeError(f"{path}:{token_offset}: expected auto token")
        replacement = str(record["replacement"])
        if not replacement:
            raise RuntimeError(f"{path}:{token_offset}: empty replacement")
        edits.append((token_offset, replacement.encode("utf-8")))

    rewritten = original
    for offset, replacement in sorted(edits, reverse=True):
        rewritten = rewritten[:offset] + replacement + rewritten[offset + 4:]

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
    if plan.get("schema") != "gui.forms.auto-type-rewrites/v1":
        raise RuntimeError("unexpected auto-type rewrite schema")

    by_path: dict[str, list[dict[str, object]]] = {}
    sizes: dict[str, int] = {}
    skipped = 0
    for record in plan["rewrites"]:
        if not bool(record["safe"]):
            skipped += 1
            continue
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
    print(
        f"applied {applied} resolved auto-type rewrites in "
        f"{len(by_path)} files; left {skipped} candidates for review")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
