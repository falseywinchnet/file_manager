from __future__ import annotations

import hashlib
import json
import platform
import sys
from fractions import Fraction
from itertools import combinations
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    analyze_adjacent_transposition,
    analyze_deletion_set,
    analyze_substitution,
    enumerate_symbolic_cases,
)


def main() -> None:
    digest = hashlib.sha256()
    deletion_set_profiles = 0
    deletion_survivors = 0
    substitution_profiles = 0
    substitution_changed = 0
    transposition_profiles = 0
    transposition_hamming_mass = 0
    cases_checked = 0

    for source in enumerate_symbolic_cases(("a", "b"), 6):
        size = len(source.symbols)
        if size == 0:
            continue
        cases_checked += 1

        for deletion_count in range(1, size + 1):
            for deleted_indices in combinations(range(size), deletion_count):
                for degree in range(1, size + 1):
                    profile = analyze_deletion_set(
                        source,
                        deleted_indices,
                        degree,
                    )
                    assert (
                        profile.surviving_witnesses
                        == profile.expected_surviving_witnesses
                    )
                    assert (
                        profile.destroyed_witnesses
                        == profile.expected_destroyed_witnesses
                    )
                    assert profile.pattern_excess_mass == profile.destroyed_witnesses
                    assert all(
                        distance == deletion_count
                        for distance in profile.survivor_gap_l1
                    )
                    assert all(
                        all(value >= 0 for value in delta)
                        and sum(delta) == deletion_count
                        for delta in profile.survivor_gap_delta
                    )
                    digest.update(repr(profile).encode("utf-8"))
                    deletion_set_profiles += 1
                    deletion_survivors += profile.surviving_witnesses

        for index, symbol in enumerate(source.symbols):
            replacement = "b" if symbol == "a" else "a"
            for degree in range(1, size + 1):
                profile = analyze_substitution(source, index, replacement, degree)
                assert profile.changed_witnesses == profile.expected_changed_witnesses
                assert profile.total_pattern_hamming == profile.changed_witnesses
                assert profile.changed_fraction == Fraction(degree, size)
                assert all(distance == 0 for distance in profile.gap_l1)
                digest.update(repr(profile).encode("utf-8"))
                substitution_profiles += 1
                substitution_changed += profile.changed_witnesses

        for left_index in range(size - 1):
            if source.symbols[left_index] == source.symbols[left_index + 1]:
                continue
            for degree in range(1, size + 1):
                profile = analyze_adjacent_transposition(
                    source,
                    left_index,
                    degree,
                )
                assert (
                    profile.unchanged_witnesses
                    == profile.expected_unchanged_witnesses
                )
                assert (
                    profile.one_symbol_changed_witnesses
                    == profile.expected_one_symbol_changed_witnesses
                )
                assert (
                    profile.two_symbol_changed_witnesses
                    == profile.expected_two_symbol_changed_witnesses
                )
                assert all(distance == 0 for distance in profile.gap_l1)
                digest.update(repr(profile).encode("utf-8"))
                transposition_profiles += 1
                transposition_hamming_mass += profile.total_pattern_hamming

    result = {
        "checks": {
            "deletion_set_gap_l1_equals_deletion_count": True,
            "deletion_set_pattern_containment": True,
            "substitution_changed_fraction_k_over_n": True,
            "substitution_gap_motion_zero": True,
            "transposition_exact_incidence_partition": True,
            "transposition_gap_motion_zero": True,
        },
        "corpus": {
            "alphabet": ["a", "b"],
            "cases_checked": cases_checked,
            "max_length": 6,
        },
        "environment": {
            "machine": platform.machine(),
            "python": platform.python_version(),
            "system": platform.system(),
        },
        "profile_stream_sha256": digest.hexdigest(),
        "profiles": {
            "deletion_sets": deletion_set_profiles,
            "substitutions": substitution_profiles,
            "transpositions": transposition_profiles,
        },
        "witness_totals": {
            "deletion_survivors": deletion_survivors,
            "substitution_changed": substitution_changed,
            "transposition_pattern_hamming": transposition_hamming_mass,
        },
    }
    print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
