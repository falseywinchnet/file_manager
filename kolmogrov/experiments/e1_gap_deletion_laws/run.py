from __future__ import annotations

import hashlib
import json
import platform
import sys
from fractions import Fraction
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    SymbolicCase,
    analyze_single_deletion,
    directional_subsequence_containment,
    enumerate_symbolic_cases,
)


def main() -> None:
    profile_digest = hashlib.sha256()
    profile_count = 0
    survivor_correspondences = 0
    destroyed_witnesses = 0
    cases_checked = 0
    deletion_pairs = 0

    for source in enumerate_symbolic_cases(("a", "b"), 7):
        size = len(source.symbols)
        if size == 0:
            continue
        cases_checked += 1
        for deleted_index in range(size):
            target = SymbolicCase(
                f"{source.object_id}:delete:{deleted_index}",
                source.symbols[:deleted_index] + source.symbols[deleted_index + 1 :],
            )
            deletion_pairs += 1
            for degree in range(1, size + 1):
                profile = analyze_single_deletion(
                    source,
                    deleted_index,
                    degree,
                    target_object_id=target.object_id,
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
                assert all(value == 1 for value in profile.survivor_gap_l1)
                assert all(
                    sum(delta) == 1
                    and delta.count(1) == 1
                    and delta.count(0) == degree
                    for delta in profile.survivor_gap_delta
                )
                assert profile.source_damage_fraction == Fraction(degree, size)
                if degree <= len(target.symbols):
                    assert (
                        directional_subsequence_containment(target, source, degree)
                        == 1
                    )

                profile_digest.update(repr(profile).encode("utf-8"))
                profile_count += 1
                survivor_correspondences += profile.surviving_witnesses
                destroyed_witnesses += profile.destroyed_witnesses

    result = {
        "checks": {
            "coordinatewise_pattern_containment": True,
            "destroyed_count_pascal_identity": True,
            "directional_containment_of_target": True,
            "source_damage_fraction_k_over_n": True,
            "survivor_content_and_order_preserved": True,
            "survivor_gap_motion_one_standard_basis_step": True,
        },
        "corpus": {
            "alphabet": ["a", "b"],
            "cases_checked": cases_checked,
            "deletion_pairs": deletion_pairs,
            "max_length": 7,
            "profiles": profile_count,
        },
        "environment": {
            "machine": platform.machine(),
            "python": platform.python_version(),
            "system": platform.system(),
        },
        "profile_stream_sha256": profile_digest.hexdigest(),
        "witness_counts": {
            "destroyed": destroyed_witnesses,
            "survivor_correspondences": survivor_correspondences,
        },
    }
    print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
