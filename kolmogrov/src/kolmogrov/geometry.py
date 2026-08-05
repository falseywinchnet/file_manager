"""Exact channel geometry for the finite-symbolic reference oracle.

The gap-simplex construction keeps a subsequence's content, ordered incidence,
position, and scale separately visible.  This is an uncompressed proof object,
not a compact hash or a retrieval-policy decision.
"""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
from fractions import Fraction
from itertools import combinations
from math import comb
from typing import Callable, Hashable, Iterator, TypeVar

from .oracle import SymbolicCase


@dataclass(frozen=True, slots=True)
class GapWitness:
    """One ordered occurrence embedded in a discrete gap simplex."""

    object_id: str
    object_size: int
    pattern: tuple[str, ...]
    indices: tuple[int, ...]
    gaps: tuple[int, ...]

    def __post_init__(self) -> None:
        degree = len(self.pattern)
        if not self.object_id:
            raise ValueError("exact object anchor is required")
        if self.object_size < 0:
            raise ValueError("object size must be non-negative")
        if len(self.indices) != degree:
            raise ValueError("pattern and index degrees differ")
        if len(self.gaps) != degree + 1:
            raise ValueError("a degree-k witness requires k+1 gaps")
        if any(index < 0 or index >= self.object_size for index in self.indices):
            raise ValueError("witness index is outside the object")
        if any(left >= right for left, right in zip(self.indices, self.indices[1:])):
            raise ValueError("witness indices must be strictly increasing")
        if any(gap < 0 for gap in self.gaps):
            raise ValueError("witness gaps must be non-negative")
        if sum(self.gaps) != self.object_size - degree:
            raise ValueError("gap mass must equal unselected object positions")
        if self.indices != indices_from_gaps(self.gaps):
            raise ValueError("indices and gaps are not the same occurrence")

    @property
    def degree(self) -> int:
        return len(self.pattern)

    @property
    def address(self) -> tuple[tuple[str, ...], tuple[int, ...]]:
        return self.pattern, self.gaps


def gaps_from_indices(object_size: int, indices: tuple[int, ...]) -> tuple[int, ...]:
    if object_size < 0:
        raise ValueError("object size must be non-negative")
    if any(index < 0 or index >= object_size for index in indices):
        raise ValueError("index is outside the object")
    if any(left >= right for left, right in zip(indices, indices[1:])):
        raise ValueError("indices must be strictly increasing")
    if not indices:
        return (object_size,)

    gaps = [indices[0]]
    gaps.extend(right - left - 1 for left, right in zip(indices, indices[1:]))
    gaps.append(object_size - indices[-1] - 1)
    return tuple(gaps)


def indices_from_gaps(gaps: tuple[int, ...]) -> tuple[int, ...]:
    if not gaps:
        raise ValueError("at least one gap coordinate is required")
    if any(gap < 0 for gap in gaps):
        raise ValueError("gap coordinates must be non-negative")
    if len(gaps) == 1:
        return ()

    indices = []
    position = gaps[0]
    indices.append(position)
    for interior_gap in gaps[1:-1]:
        position += interior_gap + 1
        indices.append(position)
    return tuple(indices)


def witness_from_indices(
    case: SymbolicCase,
    indices: tuple[int, ...],
) -> GapWitness:
    gaps = gaps_from_indices(len(case.symbols), indices)
    return GapWitness(
        object_id=case.object_id,
        object_size=len(case.symbols),
        pattern=tuple(case.symbols[index] for index in indices),
        indices=indices,
        gaps=gaps,
    )


def gap_witnesses(case: SymbolicCase, degree: int) -> Iterator[GapWitness]:
    if degree < 0 or degree > len(case.symbols):
        raise ValueError("degree must lie between zero and object length")
    for indices in combinations(range(len(case.symbols)), degree):
        yield witness_from_indices(case, indices)


def lift_indices_after_deletion(
    target_indices: tuple[int, ...],
    deleted_source_index: int,
) -> tuple[int, ...]:
    """Lift target indices into a source with one restored position."""

    if deleted_source_index < 0:
        raise ValueError("deleted index must be non-negative")
    return tuple(
        index if index < deleted_source_index else index + 1
        for index in target_indices
    )


def gap_l1(left: GapWitness, right: GapWitness) -> int:
    if left.degree != right.degree:
        raise ValueError("gap distance requires equal witness degree")
    if len(left.gaps) != len(right.gaps):
        raise ValueError("gap distance requires equal coordinate counts")
    return sum(abs(a - b) for a, b in zip(left.gaps, right.gaps))


def subsequence_multiplicity(
    case: SymbolicCase,
    degree: int,
) -> Counter[tuple[str, ...]]:
    return Counter(witness.pattern for witness in gap_witnesses(case, degree))


def directional_subsequence_containment(
    query: SymbolicCase,
    record: SymbolicCase,
    degree: int,
) -> Fraction:
    """Exact occurrence-mass containment of ``query`` in ``record``."""

    query_mass = subsequence_multiplicity(query, degree)
    if not query_mass:
        raise ValueError("query must contain at least one witness at this degree")
    record_mass = subsequence_multiplicity(record, degree)
    shared = sum(
        min(count, record_mass.get(pattern, 0))
        for pattern, count in query_mass.items()
    )
    return Fraction(shared, query_mass.total())


@dataclass(frozen=True, slots=True)
class DeletionProfile:
    source_object_id: str
    target_object_id: str
    source_size: int
    deleted_index: int
    degree: int
    surviving_witnesses: int
    destroyed_witnesses: int
    pattern_excess_mass: int
    survivor_gap_l1: tuple[int, ...]
    survivor_gap_delta: tuple[tuple[int, ...], ...]

    @property
    def expected_surviving_witnesses(self) -> int:
        return comb(self.source_size - 1, self.degree)

    @property
    def expected_destroyed_witnesses(self) -> int:
        return comb(self.source_size - 1, self.degree - 1)

    @property
    def source_damage_fraction(self) -> Fraction:
        return Fraction(self.destroyed_witnesses, comb(self.source_size, self.degree))


def _comb_or_zero(size: int, degree: int) -> int:
    if degree < 0 or degree > size:
        return 0
    return comb(size, degree)


def analyze_single_deletion(
    source: SymbolicCase,
    deleted_index: int,
    degree: int,
    *,
    target_object_id: str = "deletion-target",
) -> DeletionProfile:
    """Construct the exact survivor correspondence for one deletion."""

    source_size = len(source.symbols)
    if not 0 <= deleted_index < source_size:
        raise IndexError("deleted index is outside the source")
    if not 1 <= degree <= source_size:
        raise ValueError("degree must lie between one and source length")

    target_symbols = (
        source.symbols[:deleted_index] + source.symbols[deleted_index + 1 :]
    )
    target = SymbolicCase(target_object_id, target_symbols)
    source_mass = subsequence_multiplicity(source, degree)
    target_mass = (
        subsequence_multiplicity(target, degree)
        if degree <= len(target.symbols)
        else Counter()
    )

    if any(target_mass[pattern] > source_mass[pattern] for pattern in target_mass):
        raise AssertionError("deletion created ordered-occurrence mass")

    gap_distances = []
    gap_deltas = []
    target_witnesses = (
        gap_witnesses(target, degree)
        if degree <= len(target.symbols)
        else iter(())
    )
    for target_witness in target_witnesses:
        source_indices = lift_indices_after_deletion(
            target_witness.indices,
            deleted_index,
        )
        source_witness = witness_from_indices(source, source_indices)
        if source_witness.pattern != target_witness.pattern:
            raise AssertionError("survivor lift changed content or order")
        delta = tuple(
            source_gap - target_gap
            for source_gap, target_gap in zip(
                source_witness.gaps,
                target_witness.gaps,
            )
        )
        gap_deltas.append(delta)
        gap_distances.append(gap_l1(source_witness, target_witness))

    source_total = comb(source_size, degree)
    target_total = comb(source_size - 1, degree)
    pattern_excess = sum(
        count - target_mass.get(pattern, 0)
        for pattern, count in source_mass.items()
    )
    return DeletionProfile(
        source_object_id=source.object_id,
        target_object_id=target.object_id,
        source_size=source_size,
        deleted_index=deleted_index,
        degree=degree,
        surviving_witnesses=target_total,
        destroyed_witnesses=source_total - target_total,
        pattern_excess_mass=pattern_excess,
        survivor_gap_l1=tuple(gap_distances),
        survivor_gap_delta=tuple(gap_deltas),
    )


@dataclass(frozen=True, slots=True)
class DeletionSetProfile:
    source_size: int
    deletion_count: int
    degree: int
    surviving_witnesses: int
    destroyed_witnesses: int
    pattern_excess_mass: int
    survivor_gap_l1: tuple[int, ...]
    survivor_gap_delta: tuple[tuple[int, ...], ...]

    @property
    def expected_surviving_witnesses(self) -> int:
        return _comb_or_zero(self.source_size - self.deletion_count, self.degree)

    @property
    def expected_destroyed_witnesses(self) -> int:
        return comb(self.source_size, self.degree) - self.expected_surviving_witnesses

    @property
    def source_damage_fraction(self) -> Fraction:
        return Fraction(self.destroyed_witnesses, comb(self.source_size, self.degree))


def analyze_deletion_set(
    source: SymbolicCase,
    deleted_indices: tuple[int, ...],
    degree: int,
    *,
    target_object_id: str = "deletion-set-target",
) -> DeletionSetProfile:
    """Construct the survivor geometry after deleting several source indices."""

    source_size = len(source.symbols)
    if not deleted_indices:
        raise ValueError("at least one deletion is required")
    if tuple(sorted(set(deleted_indices))) != deleted_indices:
        raise ValueError("deleted indices must be unique and canonically sorted")
    if any(index < 0 or index >= source_size for index in deleted_indices):
        raise IndexError("deleted index is outside the source")
    if not 1 <= degree <= source_size:
        raise ValueError("degree must lie between one and source length")

    deleted = set(deleted_indices)
    kept_source_indices = tuple(
        index for index in range(source_size) if index not in deleted
    )
    target = SymbolicCase(
        target_object_id,
        tuple(source.symbols[index] for index in kept_source_indices),
    )
    source_mass = subsequence_multiplicity(source, degree)
    target_mass = (
        subsequence_multiplicity(target, degree)
        if degree <= len(target.symbols)
        else Counter()
    )
    if any(target_mass[pattern] > source_mass[pattern] for pattern in target_mass):
        raise AssertionError("deletion set created ordered-occurrence mass")

    gap_distances = []
    gap_deltas = []
    target_witnesses = (
        gap_witnesses(target, degree)
        if degree <= len(target.symbols)
        else iter(())
    )
    for target_witness in target_witnesses:
        source_indices = tuple(
            kept_source_indices[target_index]
            for target_index in target_witness.indices
        )
        source_witness = witness_from_indices(source, source_indices)
        if source_witness.pattern != target_witness.pattern:
            raise AssertionError("survivor lift changed content or order")
        delta = tuple(
            source_gap - target_gap
            for source_gap, target_gap in zip(
                source_witness.gaps,
                target_witness.gaps,
            )
        )
        gap_deltas.append(delta)
        gap_distances.append(gap_l1(source_witness, target_witness))

    target_total = _comb_or_zero(len(target.symbols), degree)
    source_total = comb(source_size, degree)
    pattern_excess = sum(
        count - target_mass.get(pattern, 0)
        for pattern, count in source_mass.items()
    )
    return DeletionSetProfile(
        source_size=source_size,
        deletion_count=len(deleted_indices),
        degree=degree,
        surviving_witnesses=target_total,
        destroyed_witnesses=source_total - target_total,
        pattern_excess_mass=pattern_excess,
        survivor_gap_l1=tuple(gap_distances),
        survivor_gap_delta=tuple(gap_deltas),
    )


@dataclass(frozen=True, slots=True)
class SubstitutionProfile:
    object_size: int
    degree: int
    unchanged_witnesses: int
    changed_witnesses: int
    total_pattern_hamming: int
    gap_l1: tuple[int, ...]

    @property
    def expected_changed_witnesses(self) -> int:
        return _comb_or_zero(self.object_size - 1, self.degree - 1)

    @property
    def changed_fraction(self) -> Fraction:
        return Fraction(self.changed_witnesses, comb(self.object_size, self.degree))


def analyze_substitution(
    source: SymbolicCase,
    index: int,
    replacement: str,
    degree: int,
) -> SubstitutionProfile:
    """Factor a genuine substitution into pattern and gap effects."""

    size = len(source.symbols)
    if not 0 <= index < size:
        raise IndexError("substitution index is outside the source")
    if replacement == source.symbols[index]:
        raise ValueError("replacement must differ for the substitution theorem")
    if not 1 <= degree <= size:
        raise ValueError("degree must lie between one and object length")

    target_symbols = list(source.symbols)
    target_symbols[index] = replacement
    target = SymbolicCase("substitution-target", tuple(target_symbols))
    changed = 0
    hamming = 0
    gap_distances = []
    for source_witness, target_witness in zip(
        gap_witnesses(source, degree),
        gap_witnesses(target, degree),
        strict=True,
    ):
        witness_hamming = sum(
            left != right
            for left, right in zip(
                source_witness.pattern,
                target_witness.pattern,
                strict=True,
            )
        )
        changed += witness_hamming > 0
        hamming += witness_hamming
        gap_distances.append(gap_l1(source_witness, target_witness))

    total = comb(size, degree)
    return SubstitutionProfile(
        object_size=size,
        degree=degree,
        unchanged_witnesses=total - changed,
        changed_witnesses=changed,
        total_pattern_hamming=hamming,
        gap_l1=tuple(gap_distances),
    )


@dataclass(frozen=True, slots=True)
class AdjacentTranspositionProfile:
    object_size: int
    degree: int
    unchanged_witnesses: int
    one_symbol_changed_witnesses: int
    two_symbol_changed_witnesses: int
    total_pattern_hamming: int
    gap_l1: tuple[int, ...]

    @property
    def expected_unchanged_witnesses(self) -> int:
        return _comb_or_zero(self.object_size - 2, self.degree)

    @property
    def expected_one_symbol_changed_witnesses(self) -> int:
        return 2 * _comb_or_zero(self.object_size - 2, self.degree - 1)

    @property
    def expected_two_symbol_changed_witnesses(self) -> int:
        return _comb_or_zero(self.object_size - 2, self.degree - 2)


def analyze_adjacent_transposition(
    source: SymbolicCase,
    left_index: int,
    degree: int,
) -> AdjacentTranspositionProfile:
    """Factor a genuine adjacent swap into pattern and gap effects."""

    size = len(source.symbols)
    if not 0 <= left_index < size - 1:
        raise IndexError("transposition requires an adjacent pair")
    if source.symbols[left_index] == source.symbols[left_index + 1]:
        raise ValueError("transposed symbols must differ for this theorem")
    if not 1 <= degree <= size:
        raise ValueError("degree must lie between one and object length")

    target_symbols = list(source.symbols)
    target_symbols[left_index], target_symbols[left_index + 1] = (
        target_symbols[left_index + 1],
        target_symbols[left_index],
    )
    target = SymbolicCase("transposition-target", tuple(target_symbols))
    hamming_counts = Counter()
    gap_distances = []
    for source_witness, target_witness in zip(
        gap_witnesses(source, degree),
        gap_witnesses(target, degree),
        strict=True,
    ):
        witness_hamming = sum(
            left != right
            for left, right in zip(
                source_witness.pattern,
                target_witness.pattern,
                strict=True,
            )
        )
        hamming_counts[witness_hamming] += 1
        gap_distances.append(gap_l1(source_witness, target_witness))

    return AdjacentTranspositionProfile(
        object_size=size,
        degree=degree,
        unchanged_witnesses=hamming_counts[0],
        one_symbol_changed_witnesses=hamming_counts[1],
        two_symbol_changed_witnesses=hamming_counts[2],
        total_pattern_hamming=hamming_counts[1] + 2 * hamming_counts[2],
        gap_l1=tuple(gap_distances),
    )


ProjectionKey = TypeVar("ProjectionKey", bound=Hashable)


def projected_gap_signature(
    case: SymbolicCase,
    degree: int,
    projection: Callable[[tuple[int, ...]], ProjectionKey],
) -> tuple[tuple[tuple[str, ...], ProjectionKey, int], ...]:
    """Aggregate one candidate position projection without fusing channels."""

    counts = Counter(
        (witness.pattern, projection(witness.gaps))
        for witness in gap_witnesses(case, degree)
    )
    return tuple(
        sorted(
            (pattern, projected_gap, count)
            for (pattern, projected_gap), count in counts.items()
        )
    )


def deletion_lift_compatible(query: GapWitness, record: GapWitness) -> bool:
    """Whether ``record`` can be a deletion ancestor of one query witness."""

    if query.degree != record.degree or query.pattern != record.pattern:
        return False
    if record.object_size < query.object_size:
        return False
    return all(
        record_gap >= query_gap
        for query_gap, record_gap in zip(query.gaps, record.gaps, strict=True)
    )


@dataclass(frozen=True, slots=True)
class OrthantContainmentReport:
    query_object_id: str
    record_object_id: str
    degree: int
    matched_witnesses: int
    query_witnesses: int
    evidence: tuple[tuple[tuple[int, ...], tuple[int, ...]], ...]

    @property
    def score(self) -> Fraction:
        return Fraction(self.matched_witnesses, self.query_witnesses)


def orthant_containment(
    query: SymbolicCase,
    record: SymbolicCase,
    degree: int,
) -> OrthantContainmentReport:
    """Maximum distinct deletion-compatible witness matching.

    This is an exact, small-object reference matcher.  It intentionally uses a
    deterministic augmenting-path algorithm rather than choosing a production
    transport or indexing strategy.
    """

    if len(record.symbols) < len(query.symbols):
        raise ValueError("record must be at least as long as the query")
    if not 1 <= degree <= len(query.symbols):
        raise ValueError("degree must lie between one and query length")

    query_witnesses = tuple(gap_witnesses(query, degree))
    record_witnesses = tuple(gap_witnesses(record, degree))
    candidates = tuple(
        tuple(
            record_index
            for record_index, record_witness in enumerate(record_witnesses)
            if deletion_lift_compatible(query_witness, record_witness)
        )
        for query_witness in query_witnesses
    )

    record_to_query: dict[int, int] = {}

    def augment(query_index: int, visited_records: set[int]) -> bool:
        for record_index in candidates[query_index]:
            if record_index in visited_records:
                continue
            visited_records.add(record_index)
            previous_query = record_to_query.get(record_index)
            if previous_query is None or augment(previous_query, visited_records):
                record_to_query[record_index] = query_index
                return True
        return False

    for query_index in range(len(query_witnesses)):
        augment(query_index, set())

    query_to_record = {
        query_index: record_index
        for record_index, query_index in record_to_query.items()
    }
    evidence = tuple(
        (
            query_witnesses[query_index].indices,
            record_witnesses[query_to_record[query_index]].indices,
        )
        for query_index in sorted(query_to_record)
    )
    return OrthantContainmentReport(
        query_object_id=query.object_id,
        record_object_id=record.object_id,
        degree=degree,
        matched_witnesses=len(record_to_query),
        query_witnesses=len(query_witnesses),
        evidence=evidence,
    )


def is_ordered_subsequence(query: SymbolicCase, record: SymbolicCase) -> bool:
    """Literal exact control for whole-query deletion containment."""

    query_index = 0
    for symbol in record.symbols:
        if query_index < len(query.symbols) and query.symbols[query_index] == symbol:
            query_index += 1
    return query_index == len(query.symbols)
