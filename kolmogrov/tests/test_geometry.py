from __future__ import annotations

import sys
import unittest
from fractions import Fraction
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (
    SymbolicCase,
    analyze_adjacent_transposition,
    analyze_deletion_set,
    analyze_single_deletion,
    analyze_substitution,
    deletion_lift_compatible,
    directional_subsequence_containment,
    gap_witnesses,
    gaps_from_indices,
    indices_from_gaps,
    is_ordered_subsequence,
    orthant_containment,
    projected_gap_signature,
    witness_from_indices,
)


class GapGeometryTests(unittest.TestCase):
    def test_gap_coordinates_and_indices_are_exact_inverses(self) -> None:
        for object_size in range(6):
            case = SymbolicCase(f"n:{object_size}", tuple(str(i) for i in range(object_size)))
            for degree in range(object_size + 1):
                for witness in gap_witnesses(case, degree):
                    with self.subTest(size=object_size, indices=witness.indices):
                        self.assertEqual(
                            indices_from_gaps(witness.gaps), witness.indices
                        )
                        self.assertEqual(
                            gaps_from_indices(object_size, witness.indices),
                            witness.gaps,
                        )

    def test_gap_witness_exposes_content_position_combination_and_scale(self) -> None:
        case = SymbolicCase("record:abcd", tuple("abcd"))
        witness = witness_from_indices(case, (0, 2))
        self.assertEqual(witness.pattern, ("a", "c"))
        self.assertEqual(witness.indices, (0, 2))
        self.assertEqual(witness.gaps, (0, 1, 1))
        self.assertEqual(witness.object_size, 4)
        self.assertEqual(witness.degree, 2)

    def test_single_deletion_has_exact_survival_and_one_step_gap_motion(self) -> None:
        source = SymbolicCase("record:abca", tuple("abca"))
        for deleted_index in range(4):
            for degree in range(1, 5):
                profile = analyze_single_deletion(source, deleted_index, degree)
                with self.subTest(deleted=deleted_index, degree=degree):
                    self.assertEqual(
                        profile.surviving_witnesses,
                        profile.expected_surviving_witnesses,
                    )
                    self.assertEqual(
                        profile.destroyed_witnesses,
                        profile.expected_destroyed_witnesses,
                    )
                    self.assertEqual(
                        profile.pattern_excess_mass,
                        profile.destroyed_witnesses,
                    )
                    self.assertTrue(
                        all(distance == 1 for distance in profile.survivor_gap_l1)
                    )
                    self.assertTrue(
                        all(
                            sum(delta) == 1
                            and delta.count(1) == 1
                            and delta.count(0) == degree
                            for delta in profile.survivor_gap_delta
                        )
                    )
                    self.assertEqual(profile.source_damage_fraction, Fraction(degree, 4))

    def test_deleted_record_is_directionally_contained_at_every_valid_degree(self) -> None:
        source = SymbolicCase("source", tuple("abaca"))
        target = SymbolicCase("target", tuple("aaca"))
        for degree in range(1, len(target.symbols) + 1):
            self.assertEqual(
                directional_subsequence_containment(target, source, degree),
                Fraction(1),
            )

    def test_deletion_set_distributes_deleted_mass_across_gap_coordinates(self) -> None:
        source = SymbolicCase("source", tuple("abcdef"))
        for deleted_indices in ((1, 4), (0, 2, 5)):
            for degree in range(1, len(source.symbols) + 1):
                profile = analyze_deletion_set(source, deleted_indices, degree)
                with self.subTest(deleted=deleted_indices, degree=degree):
                    self.assertEqual(
                        profile.surviving_witnesses,
                        profile.expected_surviving_witnesses,
                    )
                    self.assertEqual(
                        profile.destroyed_witnesses,
                        profile.expected_destroyed_witnesses,
                    )
                    self.assertEqual(
                        profile.pattern_excess_mass,
                        profile.destroyed_witnesses,
                    )
                    self.assertTrue(
                        all(
                            distance == len(deleted_indices)
                            for distance in profile.survivor_gap_l1
                        )
                    )
                    self.assertTrue(
                        all(
                            all(value >= 0 for value in delta)
                            and sum(delta) == len(deleted_indices)
                            for delta in profile.survivor_gap_delta
                        )
                    )

    def test_substitution_changes_exactly_k_over_n_occurrences(self) -> None:
        source = SymbolicCase("source", tuple("abca"))
        for degree in range(1, 5):
            profile = analyze_substitution(source, 2, "x", degree)
            with self.subTest(degree=degree):
                self.assertEqual(
                    profile.changed_witnesses,
                    profile.expected_changed_witnesses,
                )
                self.assertEqual(
                    profile.total_pattern_hamming,
                    profile.changed_witnesses,
                )
                self.assertEqual(profile.changed_fraction, Fraction(degree, 4))
                self.assertTrue(all(distance == 0 for distance in profile.gap_l1))

    def test_adjacent_transposition_has_exact_incidence_counts(self) -> None:
        source = SymbolicCase("source", tuple("abca"))
        for degree in range(1, 5):
            profile = analyze_adjacent_transposition(source, 0, degree)
            with self.subTest(degree=degree):
                self.assertEqual(
                    profile.unchanged_witnesses,
                    profile.expected_unchanged_witnesses,
                )
                self.assertEqual(
                    profile.one_symbol_changed_witnesses,
                    profile.expected_one_symbol_changed_witnesses,
                )
                self.assertEqual(
                    profile.two_symbol_changed_witnesses,
                    profile.expected_two_symbol_changed_witnesses,
                )
                self.assertTrue(all(distance == 0 for distance in profile.gap_l1))

    def test_projected_signatures_remain_independent_values(self) -> None:
        case = SymbolicCase("source", tuple("abca"))
        endpoints = projected_gap_signature(
            case,
            2,
            lambda gaps: (gaps[0], gaps[-1]),
        )
        parity = projected_gap_signature(
            case,
            2,
            lambda gaps: tuple(gap % 2 for gap in gaps),
        )
        self.assertNotEqual(endpoints, parity)

    def test_deletion_ancestor_has_full_orthant_containment_at_every_degree(self) -> None:
        record = SymbolicCase("record", tuple("axbcy"))
        query = SymbolicCase("query", tuple("abc"))
        self.assertTrue(is_ordered_subsequence(query, record))
        for degree in range(1, len(query.symbols) + 1):
            report = orthant_containment(query, record, degree)
            with self.subTest(degree=degree):
                self.assertEqual(report.score, Fraction(1))
                self.assertEqual(len(report.evidence), report.query_witnesses)

    def test_whole_query_degree_is_exact_subsequence_control(self) -> None:
        query = SymbolicCase("query", tuple("aba"))
        positive = SymbolicCase("positive", tuple("axbya"))
        negative = SymbolicCase("negative", tuple("aaxby"))
        self.assertEqual(orthant_containment(query, positive, 3).score, Fraction(1))
        self.assertEqual(orthant_containment(query, negative, 3).score, Fraction(0))
        self.assertTrue(is_ordered_subsequence(query, positive))
        self.assertFalse(is_ordered_subsequence(query, negative))

    def test_deletion_lift_requires_pattern_and_coordinatewise_gap_dominance(self) -> None:
        query_case = SymbolicCase("query", tuple("ab"))
        record_case = SymbolicCase("record", tuple("axb"))
        query_witness = witness_from_indices(query_case, (0, 1))
        compatible = witness_from_indices(record_case, (0, 2))
        wrong_pattern = witness_from_indices(record_case, (0, 1))
        self.assertTrue(deletion_lift_compatible(query_witness, compatible))
        self.assertFalse(deletion_lift_compatible(query_witness, wrong_pattern))

    def test_invalid_gap_geometry_is_rejected(self) -> None:
        with self.assertRaises(ValueError):
            indices_from_gaps(())
        with self.assertRaises(ValueError):
            gaps_from_indices(3, (2, 1))
        with self.assertRaises(ValueError):
            tuple(gap_witnesses(SymbolicCase("a", ("a",)), 2))


if __name__ == "__main__":
    unittest.main()
