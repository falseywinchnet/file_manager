#!/usr/bin/env python3
"""Black-box semantic fixtures for the GUI.Forms house-policy checker."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import tempfile


BAD_SOURCE = r"""
#include <any>
#include <compare>
#include <coroutine>
#include <ranges>
#include <utility>
#include <vector>

struct Node { int value; };
struct Compared {
    int value;
    auto operator<=>(const Compared&) const = default;
};
struct Task {
    struct promise_type {
        Task get_return_object() { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() noexcept {}
    };
};
template <typename Type>
concept Tiny = sizeof(Type) < 64;
template <typename Type>
constexpr bool has_value = requires(Type value) { value.value; };

auto trailing() -> int { return 1; }
Task coroutine() { co_return; }
int exercise(Node* node) {
    auto value = node->value;
    auto closure = [value] { return value; };
    std::pair<int, int> pair{1, 2};
    auto [left, right] = pair;
    decltype(node->value) recovered = value;
    std::any erased = value;
    std::vector<int> values{3, 1, 2};
    const auto found = std::ranges::find(values, 2);
    static_assert(Tiny<Node>);
    static_assert(has_value<Node>);
    return closure() + left + right + recovered +
           (found != values.end() ? std::any_cast<int>(erased) : 0);
}
"""


OBJECTIVE_CPP_SOURCE = r"""
struct ObjectiveNode { int value; };
int objective_fixture(ObjectiveNode* node) {
    auto closure = [node] { return node->value; };
    return closure();
}
"""


ADMITTED_SOURCE = r"""
#include <any>
#include <bit>
#include <span>

namespace gui_forms {
class Control final {
public:
    const std::any& tag() const noexcept { return tag_; }
    void set_tag(std::any value) { tag_ = value; }
private:
    std::any tag_;
};
}

consteval int authored_value() { return 7; }
template <typename Type>
int explicit_template(Type value) {
    if constexpr (sizeof(Type) == sizeof(int)) return value;
    return 0;
}
int admitted(std::span<const int> values) {
    gui_forms::Control control;
    control.set_tag(3);
    return authored_value() + explicit_template(values[0]) +
           (std::has_single_bit(8U) ? 1 : 0);
}
"""


def run_checker(
    checker: pathlib.Path,
    source_root: pathlib.Path,
    database: pathlib.Path,
    output: pathlib.Path,
    mode: str,
    sources: list[pathlib.Path],
) -> subprocess.CompletedProcess[str]:
    command = [
        str(checker),
        "--mode",
        mode,
        "--source-scope",
        "first-party",
        "--quiet",
        "--source-root",
        str(source_root),
        "--json-output",
        str(output),
        "-p",
        str(database),
    ] + [str(source) for source in sources]
    return subprocess.run(command, text=True, capture_output=True, check=False)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--checker", type=pathlib.Path, required=True)
    parser.add_argument("--clang", type=pathlib.Path, required=True)
    arguments = parser.parse_args()
    checker = arguments.checker.resolve()
    clang = arguments.clang.resolve()
    resource_directory = subprocess.check_output(
        [str(clang), "-print-resource-dir"], text=True
    ).strip()
    sdk_root = subprocess.check_output(
        ["/usr/bin/xcrun", "--show-sdk-path"], text=True
    ).strip()
    common_arguments = [
        str(clang),
        "-std=c++20",
        f"-resource-dir={resource_directory}",
        "-isysroot",
        sdk_root,
    ]
    with tempfile.TemporaryDirectory(prefix="gui-forms-house-policy-") as temporary:
        root = pathlib.Path(temporary)
        bad = root / "bad.cpp"
        objective = root / "objective.mm"
        admitted = root / "admitted.cpp"
        bad.write_text(BAD_SOURCE)
        objective.write_text(OBJECTIVE_CPP_SOURCE)
        admitted.write_text(ADMITTED_SOURCE)
        compilation_database = [
            {
                "directory": str(root),
                "file": str(bad),
                "arguments": common_arguments + ["-c", str(bad)],
            },
            {
                "directory": str(root),
                "file": str(objective),
                "arguments": common_arguments +
                ["-x", "objective-c++", "-c", str(objective)],
            },
            {
                "directory": str(root),
                "file": str(admitted),
                "arguments": common_arguments + ["-c", str(admitted)],
            },
        ]
        (root / "compile_commands.json").write_text(
            json.dumps(compilation_database, indent=2) + "\n"
        )

        bad_output = root / "bad.json"
        inventory = run_checker(
            checker, root, root, bad_output, "inventory", [bad, objective]
        )
        if inventory.returncode != 0:
            raise RuntimeError(inventory.stderr or inventory.stdout)
        ledger = json.loads(bad_output.read_text())
        kinds = {finding["constructKind"] for finding in ledger["findings"]}
        required = {
            "auto_type",
            "trailing_return",
            "pointer_member_arrow",
            "lambda",
            "structured_binding",
            "coroutine",
            "co_return",
            "std_any",
            "std_ranges",
            "defaulted_comparison",
            "requires_expression",
            "decltype_expression",
            "concept_specialization",
        }
        missing = sorted(required - kinds)
        if missing:
            raise RuntimeError(f"checker fixture missed constructs: {missing}")
        closure = run_checker(
            checker, root, root, root / "bad-closure.json", "closure", [bad]
        )
        if closure.returncode == 0:
            raise RuntimeError("closure mode accepted the negative fixture")

        admitted_output = root / "admitted.json"
        admitted_result = run_checker(
            checker, root, root, admitted_output, "closure", [admitted]
        )
        if admitted_result.returncode != 0:
            raise RuntimeError(admitted_result.stderr or admitted_result.stdout)
        admitted_ledger = json.loads(admitted_output.read_text())
        admitted_violations = [
            finding
            for finding in admitted_ledger["findings"]
            if finding["disposition"] == "violation"
        ]
        if admitted_violations:
            raise RuntimeError(f"admitted fixture violations: {admitted_violations}")
    print("house-policy checker fixtures passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
