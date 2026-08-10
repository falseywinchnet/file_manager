#!/usr/bin/env python3
"""Inventory GUI.Forms ownership syntax for a future lifecycle design round.

This program is intentionally read-only with respect to gui_forms/. It scans
first-party C++ source, describes explicit ownership and lifetime machinery,
and writes a Markdown evidence report. It is not a C++ parser and does not
claim to prove runtime ownership correctness.
"""

from __future__ import annotations

import argparse
import bisect
import collections
import dataclasses
import datetime
import hashlib
import pathlib
import re
import subprocess
import sys
from typing import Dict, Iterable, List, Optional, Sequence, Tuple


CPP_SUFFIXES = {
    ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".mm"
}

EXCLUDED_PARTS = {
    ".build", "build", "third_party", "experiments", "planning", "docs"
}

SMART_POINTERS = ("unique_ptr", "shared_ptr", "weak_ptr")

ACTION_PATTERNS = {
    "make_shared": re.compile(r"\bstd::make_shared\s*<"),
    "make_unique": re.compile(r"\bstd::make_unique\s*<"),
    "allocate_shared": re.compile(r"\bstd::allocate_shared\s*<"),
    "shared_from_this": re.compile(r"\bshared_from_this\s*\("),
    "weak_from_this": re.compile(r"\bweak_from_this\s*\("),
    "weak_lock": re.compile(r"(?:\.|\)|\])\s*lock\s*\("),
    "dynamic_pointer_cast": re.compile(r"\bstd::dynamic_pointer_cast\s*<"),
    "static_pointer_cast": re.compile(r"\bstd::static_pointer_cast\s*<"),
    "const_pointer_cast": re.compile(r"\bstd::const_pointer_cast\s*<"),
    "reinterpret_pointer_cast": re.compile(r"\bstd::reinterpret_pointer_cast\s*<"),
    "new_expression": re.compile(r"(?<!operator)\bnew\s+[A-Za-z_:]"),
    "delete_expression": re.compile(r"\bdelete(?:\s*\[\s*\])?\s+"),
    "malloc_family": re.compile(
        r"(?<![A-Za-z0-9_])(?:std::)?(?:malloc|calloc|realloc|aligned_alloc)\s*\("
    ),
    "free_call": re.compile(r"(?<![A-Za-z0-9_])(?:std::)?free\s*\("),
    "owner_revocable": re.compile(r"\bown_revocable\s*\("),
    "subscription_token": re.compile(r"\bSubscriptionToken\b"),
    "component_container": re.compile(r"\bComponentContainer\b"),
}

RAW_POINTER_PATTERN = re.compile(
    r"\b((?:const\s+)?(?:void|char|wchar_t|std::byte|[A-Z][A-Za-z0-9_:]*)"
    r"(?:\s*<[^;\n{}()]+>)?)\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)"
)

CLASS_PATTERN = re.compile(
    r"\b(?:class|struct)\s+([A-Za-z_][A-Za-z0-9_]*)\b[^;{}]*\{",
    re.MULTILINE,
)

ALIAS_PATTERN = re.compile(
    r"\busing\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*"
    r"std::(unique_ptr|shared_ptr|weak_ptr)\s*<",
    re.MULTILINE,
)


@dataclasses.dataclass(frozen=True)
class ClassRegion:
    name: str
    open_offset: int
    close_offset: int
    member_depth: int


@dataclasses.dataclass(frozen=True)
class Evidence:
    path: str
    line: int
    owner: str
    kind: str
    target: str
    scope: str
    snippet: str


@dataclasses.dataclass(frozen=True)
class Action:
    path: str
    line: int
    kind: str
    snippet: str


@dataclasses.dataclass
class FileScan:
    path: pathlib.Path
    relative: str
    original: str
    scrubbed: str
    line_starts: List[int]
    depth: List[int]
    classes: List[ClassRegion]


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    script_path = pathlib.Path(__file__).resolve()
    repository = script_path.parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository",
        type=pathlib.Path,
        default=repository,
        help="File Manager repository root (default: inferred from script path)",
    )
    parser.add_argument(
        "--include-support",
        action="store_true",
        help="also scan first-party tests, demo, tools, and generated source",
    )
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        default=repository / "gui_forms_rewrite" / "OWNERSHIP_AUDIT.md",
        help="Markdown report path",
    )
    parser.add_argument(
        "--maximum-action-rows",
        type=int,
        default=400,
        help="maximum detailed lifetime-action rows in the report",
    )
    return parser.parse_args(argv)


def source_files(gui_forms: pathlib.Path, include_support: bool) -> List[pathlib.Path]:
    roots = [gui_forms / "include", gui_forms / "src"]
    if include_support:
        roots.extend(
            [
                gui_forms / "tests",
                gui_forms / "demo",
                gui_forms / "tools",
                gui_forms / "generated",
            ]
        )
    result: List[pathlib.Path] = []
    for root in roots:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if not path.is_file() or path.suffix.lower() not in CPP_SUFFIXES:
                continue
            relative_parts = path.relative_to(gui_forms).parts
            if any(part in EXCLUDED_PARTS for part in relative_parts):
                continue
            result.append(path)
    return sorted(set(result))


def scrub_cpp(text: str) -> str:
    """Blank comments and quoted contents while retaining offsets/newlines."""
    output = list(text)
    index = 0
    state = "code"
    quote = ""
    while index < len(text):
        current = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if current == "/" and following == "/":
                output[index] = " "
                output[index + 1] = " "
                index += 2
                state = "line_comment"
                continue
            if current == "/" and following == "*":
                output[index] = " "
                output[index + 1] = " "
                index += 2
                state = "block_comment"
                continue
            if current in ('"', "'"):
                quote = current
                output[index] = " "
                index += 1
                state = "quoted"
                continue
        elif state == "line_comment":
            if current == "\n":
                state = "code"
            else:
                output[index] = " "
            index += 1
            continue
        elif state == "block_comment":
            if current == "*" and following == "/":
                output[index] = " "
                output[index + 1] = " "
                index += 2
                state = "code"
                continue
            if current != "\n":
                output[index] = " "
            index += 1
            continue
        elif state == "quoted":
            if current == "\\" and following:
                output[index] = " "
                if following != "\n":
                    output[index + 1] = " "
                index += 2
                continue
            if current == quote:
                output[index] = " "
                index += 1
                state = "code"
                continue
            if current != "\n":
                output[index] = " "
            index += 1
            continue
        index += 1
    return "".join(output)


def make_line_starts(text: str) -> List[int]:
    starts = [0]
    starts.extend(index + 1 for index, value in enumerate(text) if value == "\n")
    return starts


def line_number(starts: Sequence[int], offset: int) -> int:
    return bisect.bisect_right(starts, offset)


def brace_depth(text: str) -> List[int]:
    result = [0] * (len(text) + 1)
    depth = 0
    for index, value in enumerate(text):
        result[index] = depth
        if value == "{":
            depth += 1
        elif value == "}" and depth > 0:
            depth -= 1
    result[len(text)] = depth
    return result


def matching_delimiter(text: str, open_offset: int, opening: str, closing: str) -> int:
    depth = 0
    for index in range(open_offset, len(text)):
        value = text[index]
        if value == opening:
            depth += 1
        elif value == closing:
            depth -= 1
            if depth == 0:
                return index
    return -1


def class_regions(text: str, depth: Sequence[int]) -> List[ClassRegion]:
    result: List[ClassRegion] = []
    for match in CLASS_PATTERN.finditer(text):
        open_offset = match.end() - 1
        close_offset = matching_delimiter(text, open_offset, "{", "}")
        if close_offset < 0:
            continue
        result.append(
            ClassRegion(
                name=match.group(1),
                open_offset=open_offset,
                close_offset=close_offset,
                member_depth=depth[open_offset] + 1,
            )
        )
    return result


def innermost_class(regions: Sequence[ClassRegion], offset: int) -> Optional[ClassRegion]:
    matches = [
        region
        for region in regions
        if region.open_offset < offset < region.close_offset
    ]
    if not matches:
        return None
    return min(matches, key=lambda region: region.close_offset - region.open_offset)


def normalize_type(value: str) -> str:
    return re.sub(r"\s+", " ", value.strip())


def snippet_at(scan: FileScan, offset: int, maximum: int = 180) -> str:
    start = scan.original.rfind("\n", 0, offset) + 1
    end = scan.original.find("\n", offset)
    if end < 0:
        end = len(scan.original)
    value = re.sub(r"\s+", " ", scan.original[start:end].strip())
    if len(value) > maximum:
        return value[: maximum - 1] + "…"
    return value


def statement_has_call_syntax(text: str, template_end: int) -> bool:
    semicolon = text.find(";", template_end)
    if semicolon < 0:
        semicolon = min(len(text), template_end + 500)
    suffix = text[template_end + 1 : semicolon]
    return "(" in suffix


def scan_file(path: pathlib.Path, gui_forms: pathlib.Path) -> FileScan:
    original = path.read_text(encoding="utf-8", errors="replace")
    scrubbed = scrub_cpp(original)
    depth = brace_depth(scrubbed)
    return FileScan(
        path=path,
        relative=str(path.relative_to(gui_forms)),
        original=original,
        scrubbed=scrubbed,
        line_starts=make_line_starts(original),
        depth=depth,
        classes=class_regions(scrubbed, depth),
    )


def smart_pointer_evidence(scan: FileScan) -> List[Evidence]:
    result: List[Evidence] = []
    for kind in SMART_POINTERS:
        pattern = re.compile(r"\bstd::" + re.escape(kind) + r"\s*<")
        for match in pattern.finditer(scan.scrubbed):
            open_offset = scan.scrubbed.find("<", match.start(), match.end())
            close_offset = matching_delimiter(scan.scrubbed, open_offset, "<", ">")
            if close_offset < 0:
                target = "<unbalanced>"
                close_offset = match.end() - 1
            else:
                target = normalize_type(scan.scrubbed[open_offset + 1 : close_offset])
            owner_region = innermost_class(scan.classes, match.start())
            owner = owner_region.name if owner_region else "<file/function>"
            direct = bool(
                owner_region
                and scan.depth[match.start()] == owner_region.member_depth
                and not statement_has_call_syntax(scan.scrubbed, close_offset)
            )
            result.append(
                Evidence(
                    path=scan.relative,
                    line=line_number(scan.line_starts, match.start()),
                    owner=owner,
                    kind=kind,
                    target=target,
                    scope="class declaration" if direct else "local/signature/use",
                    snippet=snippet_at(scan, match.start()),
                )
            )
    return result


def alias_evidence(scan: FileScan) -> Tuple[List[Evidence], Dict[Tuple[str, str], Tuple[str, str]]]:
    result: List[Evidence] = []
    aliases: Dict[Tuple[str, str], Tuple[str, str]] = {}
    for match in ALIAS_PATTERN.finditer(scan.scrubbed):
        open_offset = scan.scrubbed.find("<", match.start(), match.end())
        close_offset = matching_delimiter(scan.scrubbed, open_offset, "<", ">")
        if close_offset < 0:
            continue
        alias = match.group(1)
        kind = match.group(2)
        target = normalize_type(scan.scrubbed[open_offset + 1 : close_offset])
        owner_region = innermost_class(scan.classes, match.start())
        owner = owner_region.name if owner_region else "<namespace>"
        aliases[(owner, alias)] = (kind, target)
        result.append(
            Evidence(
                path=scan.relative,
                line=line_number(scan.line_starts, match.start()),
                owner=owner,
                kind="alias:" + kind,
                target=target,
                scope="alias definition",
                snippet=snippet_at(scan, match.start()),
            )
        )
    return result, aliases


def qualified_alias_evidence(
    scan: FileScan, aliases: Dict[Tuple[str, str], Tuple[str, str]]
) -> List[Evidence]:
    result: List[Evidence] = []
    for (alias_owner, alias_name), (kind, target) in aliases.items():
        if alias_owner == "<namespace>":
            continue
        pattern = re.compile(
            r"\b" + re.escape(alias_owner) + r"::" + re.escape(alias_name) + r"\b"
        )
        for match in pattern.finditer(scan.scrubbed):
            owner_region = innermost_class(scan.classes, match.start())
            owner = owner_region.name if owner_region else "<file/function>"
            direct = bool(
                owner_region and scan.depth[match.start()] == owner_region.member_depth
            )
            result.append(
                Evidence(
                    path=scan.relative,
                    line=line_number(scan.line_starts, match.start()),
                    owner=owner,
                    kind="qualified-alias:" + kind,
                    target=target,
                    scope="class declaration" if direct else "local/signature/use",
                    snippet=snippet_at(scan, match.start()),
                )
            )
    return result


def raw_pointer_evidence(scan: FileScan) -> List[Evidence]:
    result: List[Evidence] = []
    for match in RAW_POINTER_PATTERN.finditer(scan.scrubbed):
        owner_region = innermost_class(scan.classes, match.start())
        owner = owner_region.name if owner_region else "<file/function>"
        direct = bool(
            owner_region and scan.depth[match.start()] == owner_region.member_depth
        )
        result.append(
            Evidence(
                path=scan.relative,
                line=line_number(scan.line_starts, match.start()),
                owner=owner,
                kind="raw_pointer",
                target=normalize_type(match.group(1)),
                scope="class declaration" if direct else "local/signature/use",
                snippet=snippet_at(scan, match.start()),
            )
        )
    return result


def action_evidence(scan: FileScan) -> List[Action]:
    result: List[Action] = []
    for kind, pattern in ACTION_PATTERNS.items():
        for match in pattern.finditer(scan.scrubbed):
            result.append(
                Action(
                    path=scan.relative,
                    line=line_number(scan.line_starts, match.start()),
                    kind=kind,
                    snippet=snippet_at(scan, match.start()),
                )
            )
    return result


def git_value(repository: pathlib.Path, arguments: Sequence[str]) -> str:
    try:
        completed = subprocess.run(
            ["git", *arguments],
            cwd=repository,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
        )
        return completed.stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return "unavailable"


def corpus_digest(paths: Iterable[pathlib.Path], gui_forms: pathlib.Path) -> str:
    digest = hashlib.sha256()
    for path in paths:
        relative = str(path.relative_to(gui_forms)).encode("utf-8")
        digest.update(len(relative).to_bytes(8, "little"))
        digest.update(relative)
        content = path.read_bytes()
        digest.update(len(content).to_bytes(8, "little"))
        digest.update(content)
    return digest.hexdigest()


def markdown_cell(value: str) -> str:
    return value.replace("|", "\\|").replace("\n", " ")


def top_counts(counter: collections.Counter, maximum: int = 30) -> List[Tuple[str, int]]:
    return sorted(counter.items(), key=lambda item: (-item[1], item[0]))[:maximum]


def render_report(
    repository: pathlib.Path,
    gui_forms: pathlib.Path,
    paths: Sequence[pathlib.Path],
    evidence: Sequence[Evidence],
    aliases: Sequence[Evidence],
    raw_pointers: Sequence[Evidence],
    actions: Sequence[Action],
    include_support: bool,
    maximum_action_rows: int,
) -> str:
    generated = datetime.datetime.now(datetime.timezone.utc).replace(microsecond=0)
    commit = git_value(repository, ["rev-parse", "HEAD"])
    branch = git_value(repository, ["branch", "--show-current"])
    dirty_lines = git_value(repository, ["status", "--short"])
    dirty_count = 0 if not dirty_lines or dirty_lines == "unavailable" else len(dirty_lines.splitlines())
    digest = corpus_digest(paths, gui_forms)

    all_pointer_evidence = list(evidence) + list(aliases) + list(raw_pointers)
    kind_counts = collections.Counter(item.kind for item in all_pointer_evidence)
    file_counts = collections.Counter(item.path for item in all_pointer_evidence)
    action_counts = collections.Counter(item.kind for item in actions)
    class_edges = [
        item
        for item in list(evidence) + list(aliases)
        if item.scope == "class declaration"
        or item.scope == "alias definition"
    ]
    retained_terms = (
        "Control", "Window", "Component", "Event", "Subscription", "Binding",
        "Dispatcher", "Timer", "Image", "Popup", "Host", "State"
    )
    retained_edges = [
        item
        for item in class_edges
        if any(term in item.owner or term in item.target for term in retained_terms)
    ]

    lines: List[str] = []
    lines.append("# GUI.Forms ownership inventory")
    lines.append("")
    lines.append("Status: **OBSERVED lexical evidence for a future lifecycle round; no ownership change authorized**.")
    lines.append("")
    lines.append("## Snapshot")
    lines.append("")
    lines.append("| Field | Value |")
    lines.append("|---|---|")
    lines.append("| Generated UTC | `{}` |".format(generated.isoformat()))
    lines.append("| Branch | `{}` |".format(markdown_cell(branch)))
    lines.append("| Commit | `{}` |".format(markdown_cell(commit)))
    lines.append("| Dirty entries at scan | `{}` |".format(dirty_count))
    lines.append("| Scope | `{}` |".format(
        "production include/src plus support" if include_support else "production include/src"
    ))
    lines.append("| C/C++ files | `{}` |".format(len(paths)))
    lines.append("| Corpus SHA-256 | `{}` |".format(digest))
    lines.append("")
    lines.append("The report was produced by `gui_forms_rewrite/tools/trace_ownership.py`.")
    lines.append("Run it again at the start of any future lifecycle round; do not treat this dirty-tree snapshot as timeless.")
    lines.append("")
    lines.append("## What this proves—and what it does not")
    lines.append("")
    lines.append("The scanner removes ordinary comments/string contents, finds explicit smart-pointer templates and aliases, approximates class-scope ownership edges, and records lifetime operations. It exposes where ownership decisions are written and where a later semantic/AST audit should concentrate.")
    lines.append("")
    lines.append("It does **not** prove runtime reachability, cycle absence, destruction order, callback capture behavior, thread safety, or whether a raw pointer is always non-owning. Macro expansion, inherited unqualified aliases, complex declarators, and type erasure require compilation-database/AST and behavioral evidence. Every lifecycle proposal must still trace construction, transfer, revocation, disposal, and destruction dynamically.")
    lines.append("")
    lines.append("## Inventory summary")
    lines.append("")
    lines.append("### Pointer and alias syntax")
    lines.append("")
    lines.append("| Kind | Occurrences |")
    lines.append("|---|---:|")
    for kind, count in sorted(kind_counts.items()):
        lines.append("| `{}` | {} |".format(kind, count))
    lines.append("")
    lines.append("### Lifetime operations")
    lines.append("")
    lines.append("| Operation | Occurrences |")
    lines.append("|---|---:|")
    for kind, count in sorted(action_counts.items()):
        lines.append("| `{}` | {} |".format(kind, count))
    lines.append("")
    lines.append("### Files with the densest explicit ownership surface")
    lines.append("")
    lines.append("| File | Pointer/alias occurrences |")
    lines.append("|---|---:|")
    for path, count in top_counts(file_counts):
        lines.append("| `{}` | {} |".format(markdown_cell(path), count))
    lines.append("")
    lines.append("## Retained/lifecycle-focused class declarations")
    lines.append("")
    lines.append("This is a review queue, not a proposed graph rewrite.")
    lines.append("")
    lines.append("| Owner | Kind | Target | Location | Declaration evidence |")
    lines.append("|---|---|---|---|---|")
    for item in sorted(retained_edges, key=lambda value: (value.owner, value.path, value.line, value.kind)):
        lines.append(
            "| `{}` | `{}` | `{}` | `{}:{}` | `{}` |".format(
                markdown_cell(item.owner),
                markdown_cell(item.kind),
                markdown_cell(item.target),
                markdown_cell(item.path),
                item.line,
                markdown_cell(item.snippet),
            )
        )
    lines.append("")
    lines.append("## All detected smart-pointer and alias evidence")
    lines.append("")
    lines.append("| Owner/scope | Kind | Target | Scope | Location |")
    lines.append("|---|---|---|---|---|")
    for item in sorted(list(evidence) + list(aliases), key=lambda value: (value.path, value.line, value.kind)):
        lines.append(
            "| `{}` | `{}` | `{}` | {} | `{}:{}` |".format(
                markdown_cell(item.owner),
                markdown_cell(item.kind),
                markdown_cell(item.target),
                markdown_cell(item.scope),
                markdown_cell(item.path),
                item.line,
            )
        )
    lines.append("")
    lines.append("## Raw-pointer plumbing")
    lines.append("")
    lines.append("Raw pointers are recorded because a future lifecycle round must distinguish non-owning direct access, platform handles, optional links, array traversal, and accidental ownership. Their presence is not a defect under the house policy.")
    lines.append("")
    lines.append("| Owner/scope | Target | Scope | Location | Evidence |")
    lines.append("|---|---|---|---|---|")
    for item in sorted(raw_pointers, key=lambda value: (value.path, value.line)):
        lines.append(
            "| `{}` | `{}` | {} | `{}:{}` | `{}` |".format(
                markdown_cell(item.owner),
                markdown_cell(item.target),
                markdown_cell(item.scope),
                markdown_cell(item.path),
                item.line,
                markdown_cell(item.snippet),
            )
        )
    lines.append("")
    lines.append("## Lifetime-operation evidence")
    lines.append("")
    lines.append("| Operation | Location | Evidence |")
    lines.append("|---|---|---|")
    sorted_actions = sorted(actions, key=lambda value: (value.path, value.line, value.kind))
    for item in sorted_actions[:maximum_action_rows]:
        lines.append(
            "| `{}` | `{}:{}` | `{}` |".format(
                markdown_cell(item.kind),
                markdown_cell(item.path),
                item.line,
                markdown_cell(item.snippet),
            )
        )
    if len(sorted_actions) > maximum_action_rows:
        lines.append("")
        lines.append("Detailed action rows were capped at {}; {} additional rows remain represented in the summary counts.".format(maximum_action_rows, len(sorted_actions) - maximum_action_rows))
    lines.append("")
    lines.append("## Required next evidence before any lifecycle change")
    lines.append("")
    lines.append("1. Build an AST-backed member/parameter/return graph from the exact compilation database, resolving aliases and templates.")
    lines.append("2. Trace retained Control child ownership, parent/observer weakness, Window attachment, Component disposal, Event slot/token revocation, dispatcher work ownership, and ABI subscription records as separate semantic graphs.")
    lines.append("3. Instrument construction, attachment, detachment, disposal, revocation, final destruction, and cross-thread work with stable identities and sequence numbers.")
    lines.append("4. Identify and reproduce cycles, delayed release, premature release, or allocation pressure before proposing a replacement.")
    lines.append("5. Compare any unique ownership, generation-handle pool, intrusive link, or stable-pool candidate against the current shared/weak lifecycle traces and failure behavior.")
    lines.append("6. Treat C ABI generational handles and native/platform ownership as fixed boundaries unless separately approved.")
    lines.append("")
    lines.append("## Current decision")
    lines.append("")
    lines.append("The present rewrite preserves shared/weak retained ownership. This inventory is intentionally banked for a future lifecycle-change round and must not be used to smuggle ownership changes into syntax, Delegate/Event, sorting, or storage batches.")
    lines.append("")
    return "\n".join(lines)


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)
    repository = args.repository.resolve()
    gui_forms = repository / "gui_forms"
    if not gui_forms.is_dir():
        print("error: GUI.Forms source directory not found: {}".format(gui_forms), file=sys.stderr)
        return 2
    paths = source_files(gui_forms, args.include_support)
    if not paths:
        print("error: no C/C++ sources found", file=sys.stderr)
        return 2

    scans = [scan_file(path, gui_forms) for path in paths]
    direct_evidence: List[Evidence] = []
    alias_definitions: List[Evidence] = []
    aliases_by_name: Dict[Tuple[str, str], Tuple[str, str]] = {}
    raw_pointers: List[Evidence] = []
    actions: List[Action] = []

    for scan in scans:
        direct_evidence.extend(smart_pointer_evidence(scan))
        definitions, aliases = alias_evidence(scan)
        alias_definitions.extend(definitions)
        aliases_by_name.update(aliases)
        raw_pointers.extend(raw_pointer_evidence(scan))
        actions.extend(action_evidence(scan))

    qualified_aliases: List[Evidence] = []
    for scan in scans:
        qualified_aliases.extend(qualified_alias_evidence(scan, aliases_by_name))

    report = render_report(
        repository=repository,
        gui_forms=gui_forms,
        paths=paths,
        evidence=direct_evidence + qualified_aliases,
        aliases=alias_definitions,
        raw_pointers=raw_pointers,
        actions=actions,
        include_support=args.include_support,
        maximum_action_rows=max(0, args.maximum_action_rows),
    )
    output = args.output
    if not output.is_absolute():
        output = repository / output
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(report, encoding="utf-8")
    print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
