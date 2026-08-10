#!/usr/bin/env python3
"""Inventory explicit std::function spellings by their architectural role."""

from __future__ import annotations

import argparse
import collections
import datetime
import pathlib
import re


SOURCE_SUFFIXES = {".cpp", ".hpp", ".h", ".mm"}
TOKEN = re.compile(r"\bstd::function\b")


def category(relative: pathlib.Path, line: str) -> str:
    text = relative.as_posix()
    if "/event/" in f"/{text}":
        return "Event compatibility/storage"
    if any(part in text for part in ("dispatcher", "scheduler", "/timer/")) or any(
        word in line for word in ("begin_invoke", "dispatch", "FrameTime")
    ):
        return "Dispatch, scheduling, and cancellation"
    if any(part in text for part in ("/host/", "/platform/", "/abi/")):
        return "Host, platform, and C ABI adapters"
    if any(part in text for part in ("binding", "inspection", "property_")):
        return "Binding, inspection, and property registries"
    if any(part in text for part in ("command", "menu", "accelerator")):
        return "Commands and input routing"
    if any(part in text for part in ("drawing", "paint", "live_surface")):
        return "Rendering and live content"
    if text.startswith(("demo/", "tests/", "file_manager_demoboard/", "tools/")):
        return "First-party support and dogfooding"
    return "Other retained owning callbacks"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository", type=pathlib.Path, default=pathlib.Path.cwd())
    parser.add_argument("--output", type=pathlib.Path, required=True)
    arguments = parser.parse_args()
    repository = arguments.repository.resolve()
    root = repository / "gui_forms"
    records: list[tuple[str, pathlib.Path, int, str]] = []
    for path in sorted(root.rglob("*")):
        if path.suffix not in SOURCE_SUFFIXES or not path.is_file():
            continue
        relative = path.relative_to(root)
        if any(part in {"third_party", "experiments", "build", ".build"} for part in relative.parts):
            continue
        for line_number, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
            if line.lstrip().startswith("//"):
                continue
            for _match in TOKEN.finditer(line):
                records.append((category(relative, line), relative, line_number, line.strip()))

    counts = collections.Counter(record[0] for record in records)
    generated = datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds")
    lines = [
        "# GUI.Forms `std::function` audit",
        "",
        "Status: **OBSERVED lexical inventory plus measured M4 allocation evidence; retained uses are classified, not banned**.",
        "",
        "## Snapshot",
        "",
        f"Generated UTC: `{generated}`. Scope: first-party GUI.Forms C++/Objective-C++ outside third-party, experiments, and build products. Explicit spellings: **{len(records)}**.",
        "",
        "One spelling is not necessarily one callback object: nested host signatures contain several spellings, while aliases can create many runtime objects from one spelling. This file is a review map, not an allocation profiler.",
        "",
        "## Role classification",
        "",
        "| Role | Spellings | Rewrite decision |",
        "|---|---:|---|",
    ]
    decisions = {
        "Event compatibility/storage": "Retain the legacy owning overload; direct member subscriptions use Delegate first.",
        "Dispatch, scheduling, and cancellation": "Retain: ownership, cancellation, wake, thread transfer, and exception transport differ from Event.",
        "Host, platform, and C ABI adapters": "Retain: these callbacks cross explicit host/provider lifetime boundaries; C ABI remains function pointer plus context.",
        "Binding, inspection, and property registries": "Retain: heterogeneous property/value registries intentionally own erased operations.",
        "Commands and input routing": "Retain unless a concrete member binding can use Delegate without changing result/lifetime semantics.",
        "Rendering and live content": "Retain: owning paint/wake work has distinct surface and cancellation lifetime.",
        "First-party support and dogfooding": "Retain where it exercises the owning API; support code does not redesign production.",
        "Other retained owning callbacks": "Reviewed individually; no allocation-heavy production target was proven by spelling alone.",
    }
    for name in sorted(counts):
        lines.append(f"| {name} | {counts[name]} | {decisions[name]} |")
    lines.extend(
        [
            "",
            "## Quantified decision",
            "",
            "`CALLBACK_LAB_M4_RESULTS.csv` is the reproducible Release-build control on the M4 host. On that libc++ configuration:",
            "",
            "- Delegate binding is 16 bytes and performs zero allocation.",
            "- A small `std::function<void(int)>` binding fits its implementation's small buffer and performs zero allocation; the object is 32 bytes.",
            "- A 200-byte named owning target performs one allocation at `std::function` binding.",
            "- Event subscription performs two allocations for Delegate and three for the large owning target.",
            "- Snapshot Event emission performs one allocation for Delegate and two for the large owning target because the owning callable is copied to preserve callback lifetime during revocation.",
            "",
            "The invocation microtiming is retained as negative/diagnostic evidence, not a winner: an atomic observable dominates the tiny call path and the three generated call sites optimize differently. Delegate is selected for explicit non-ownership and zero binding allocation, not from a claimed nanosecond advantage.",
            "",
            "The conspicuous large-target Event case is now documented, but no matching first-party production subscription was proven. Replacing Event snapshot/copy semantics speculatively would violate O-015. If a future profiler identifies such a target, reduce its captured state or open the measured slot-storage round.",
            "",
            "## Complete spelling inventory",
            "",
            "| Role | Location | Source line |",
            "|---|---|---|",
        ]
    )
    for role, relative, line_number, source in records:
        escaped = source.replace("|", "\\|").replace("`", "'")
        lines.append(f"| {role} | `{relative.as_posix()}:{line_number}` | `{escaped}` |")
    arguments.output.write_text("\n".join(lines) + "\n")
    print(arguments.output.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
