from __future__ import annotations

import hashlib
import json
import platform
import sys
from collections import Counter
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    SymbolicCase,
    delete,
    enumerate_symbolic_cases,
    exhaustive_breakdown,
    find_minimal_collision,
    insert,
    reconstruct_literal,
    substitute,
    transpose_adjacent,
)


def main() -> None:
    cases = tuple(enumerate_symbolic_cases(("a", "b"), 5))
    breakdowns = tuple(exhaustive_breakdown(case) for case in cases)
    for case, breakdown in zip(cases, breakdowns, strict=True):
        assert reconstruct_literal(breakdown) == case.symbols
        assert exhaustive_breakdown(case) == breakdown
        assert all(feature.anchors for feature in breakdown.features)

    collision = find_minimal_collision(
        ("a", "b"),
        5,
        lambda case: tuple(sorted(Counter(case.symbols).items())),
    )
    assert collision is not None

    transformed = SymbolicCase("record:ab", ("a", "b"))
    transformed = substitute(transformed, 1, "c", "record:ac")
    transformed = insert(transformed, 1, "x", "record:axc")
    transformed = transpose_adjacent(transformed, 0, "record:xac")
    transformed = delete(transformed, 2, "record:xa")

    canonical_digest = hashlib.sha256()
    for breakdown in breakdowns:
        canonical_digest.update(repr(breakdown).encode("utf-8"))

    result = {
        "canonical_breakdown_sha256": canonical_digest.hexdigest(),
        "checks": {
            "all_features_source_anchored": True,
            "canonical_repeat_equal": True,
            "literal_reconstruction": True,
        },
        "corpus": {
            "alphabet": ["a", "b"],
            "max_length": 5,
            "objects": len(cases),
            "total_features": sum(len(item.features) for item in breakdowns),
        },
        "environment": {
            "machine": platform.machine(),
            "python": platform.python_version(),
            "system": platform.system(),
        },
        "minimal_content_multiset_collision": [
            list(collision[0].symbols),
            list(collision[1].symbols),
        ],
        "transformation_lineage": [
            step.operation for step in transformed.lineage
        ],
    }
    print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
