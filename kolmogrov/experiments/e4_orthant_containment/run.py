from __future__ import annotations

import hashlib
import json
import platform
import sys
from collections import defaultdict
from itertools import product
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    SymbolicCase,
    directional_subsequence_containment,
    is_ordered_subsequence,
    orthant_containment,
)


def main() -> None:
    max_record_length = 6
    digest = hashlib.sha256()
    statistics: defaultdict[int, dict[str, object]] = defaultdict(
        lambda: {
            "evaluated_pairs": 0,
            "exact_positive_pairs": 0,
            "orthant_false_positives": 0,
            "orthant_positive_accepts": 0,
            "pattern_only_false_positives": 0,
            "pattern_positive_accepts": 0,
            "first_orthant_false_positive": None,
        }
    )

    for record_length in range(1, max_record_length + 1):
        for record_symbols in product(("a", "b"), repeat=record_length):
            record = SymbolicCase("record", record_symbols)
            for query_length in range(1, record_length + 1):
                for query_symbols in product(("a", "b"), repeat=query_length):
                    query = SymbolicCase("query", query_symbols)
                    exact = is_ordered_subsequence(query, record)
                    for degree in range(1, query_length + 1):
                        pattern_score = directional_subsequence_containment(
                            query,
                            record,
                            degree,
                        )
                        orthant_report = orthant_containment(query, record, degree)
                        assert orthant_report.score <= pattern_score
                        pattern_accepts = pattern_score == 1
                        orthant_accepts = orthant_report.score == 1

                        row = statistics[degree]
                        row["evaluated_pairs"] += 1
                        row["exact_positive_pairs"] += exact
                        row["pattern_positive_accepts"] += exact and pattern_accepts
                        row["orthant_positive_accepts"] += exact and orthant_accepts
                        row["pattern_only_false_positives"] += (
                            not exact and pattern_accepts
                        )
                        row["orthant_false_positives"] += (
                            not exact and orthant_accepts
                        )
                        if (
                            not exact
                            and orthant_accepts
                            and row["first_orthant_false_positive"] is None
                        ):
                            row["first_orthant_false_positive"] = {
                                "query": list(query_symbols),
                                "record": list(record_symbols),
                            }

                        assert not exact or pattern_accepts
                        assert not exact or orthant_accepts
                        if degree == query_length:
                            assert orthant_accepts == exact

                        digest.update(
                            repr(
                                (
                                    query_symbols,
                                    record_symbols,
                                    degree,
                                    exact,
                                    pattern_score,
                                    orthant_report,
                                )
                            ).encode("utf-8")
                        )

    result = {
        "checks": {
            "all_exact_subsequences_accepted_at_every_degree": True,
            "full_query_degree_equals_exact_subsequence_control": True,
            "orthant_score_never_exceeds_pattern_only_score": True,
        },
        "corpus": {
            "alphabet": ["a", "b"],
            "max_record_length": max_record_length,
            "query_lengths": "one through record length",
        },
        "environment": {
            "machine": platform.machine(),
            "python": platform.python_version(),
            "system": platform.system(),
        },
        "pair_stream_sha256": digest.hexdigest(),
        "statistics_by_degree": {
            str(degree): row for degree, row in sorted(statistics.items())
        },
    }
    print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
