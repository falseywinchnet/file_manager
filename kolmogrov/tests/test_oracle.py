from __future__ import annotations

import sys
import unittest
from collections import Counter
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (
    OracleConfiguration,
    SymbolicCase,
    delete,
    enumerate_symbolic_cases,
    exhaustive_breakdown,
    find_minimal_collision,
    find_minimal_counterexample,
    insert,
    reconstruct_literal,
    substitute,
    transpose_adjacent,
)


class SymbolicOracleTests(unittest.TestCase):
    def test_exhaustive_binary_sequences_reconstruct_through_length_four(self) -> None:
        cases = tuple(enumerate_symbolic_cases(("0", "1"), 4))
        self.assertEqual(len(cases), 31)
        for case in cases:
            with self.subTest(case=case.object_id):
                breakdown = exhaustive_breakdown(case)
                self.assertEqual(reconstruct_literal(breakdown), case.symbols)
                self.assertEqual(
                    breakdown,
                    exhaustive_breakdown(case),
                    "oracle output must be canonical and deterministic",
                )
                self.assertTrue(all(feature.anchors for feature in breakdown.features))

    def test_repeated_features_retain_all_source_anchors(self) -> None:
        breakdown = exhaustive_breakdown(SymbolicCase("record:aaa", ("a", "a", "a")))
        symbol_feature = next(
            feature
            for feature in breakdown.features
            if feature.channel == "content"
            and '"family":"symbol"' in feature.identity
        )
        self.assertEqual(symbol_feature.value, 3.0)
        self.assertEqual(symbol_feature.anchors, ("index:0", "index:1", "index:2"))

        subsequence_feature = next(
            feature
            for feature in breakdown.features
            if feature.channel == "combination"
            and '"family":"ordered_subsequence"' in feature.identity
            and feature.resolution == "degree:2"
        )
        self.assertEqual(subsequence_feature.value, 3.0)
        self.assertEqual(
            subsequence_feature.anchors,
            ("indices:0,1", "indices:0,2", "indices:1,2"),
        )

    def test_truncated_configuration_is_explicit_in_identity_and_metadata(self) -> None:
        configuration = OracleConfiguration(
            max_contiguous_degree=2,
            max_subsequence_degree=2,
        )
        breakdown = exhaustive_breakdown(
            SymbolicCase("record:abcd", tuple("abcd")),
            configuration,
        )
        self.assertEqual(breakdown.metadata["oracle_scope"], "truncated")
        self.assertEqual(
            breakdown.metadata["oracle_configuration"], configuration.identity
        )
        self.assertFalse(
            any(feature.resolution == "degree:3" for feature in breakdown.features)
        )

    def test_elementary_edits_append_exact_transformation_lineage(self) -> None:
        source = SymbolicCase("record:ab", ("a", "b"))
        changed = substitute(source, 1, "c", "record:ac")
        changed = insert(changed, 1, "x", "record:axc")
        changed = transpose_adjacent(changed, 0, "record:xac")
        changed = delete(changed, 2, "record:xa")

        self.assertEqual(changed.symbols, ("x", "a"))
        self.assertEqual(
            tuple(step.operation for step in changed.lineage),
            ("substitute", "insert", "transpose_adjacent", "delete"),
        )
        breakdown = exhaustive_breakdown(changed)
        self.assertIn('"source_object_id":"record:xac"', breakdown.metadata["transformation_lineage"])

    def test_minimal_counterexample_uses_declared_enumeration_order(self) -> None:
        counterexample = find_minimal_counterexample(
            ("a", "b"),
            3,
            lambda case: len(case.symbols) < 2,
        )
        self.assertIsNotNone(counterexample)
        assert counterexample is not None
        self.assertEqual(counterexample.symbols, ("a", "a"))

    def test_minimal_content_multiset_collision_is_ab_ba(self) -> None:
        collision = find_minimal_collision(
            ("a", "b"),
            3,
            lambda case: tuple(sorted(Counter(case.symbols).items())),
        )
        self.assertIsNotNone(collision)
        assert collision is not None
        self.assertEqual(
            (collision[0].symbols, collision[1].symbols),
            (("a", "b"), ("b", "a")),
        )

    def test_invalid_enumeration_and_edit_boundaries_fail_closed(self) -> None:
        with self.assertRaises(ValueError):
            tuple(enumerate_symbolic_cases(("a", "a"), 2))
        with self.assertRaises(ValueError):
            tuple(enumerate_symbolic_cases((), 1))
        with self.assertRaises(IndexError):
            delete(SymbolicCase("empty", ()), 0, "still-empty")
        with self.assertRaises(IndexError):
            transpose_adjacent(SymbolicCase("one", ("a",)), 0, "one-again")


if __name__ == "__main__":
    unittest.main()
