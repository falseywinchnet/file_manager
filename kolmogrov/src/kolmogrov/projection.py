"""Nested rich-alphabet projections for observable certificates.

The exact certificate oracle uses one bit for every pattern in ``A**m``.  This
module keeps the certificate geometry but projects each complete pattern into
one or more bounded residue towers.  Projection collisions can only add
candidates; exact external verification remains authoritative.
"""

from __future__ import annotations

from dataclasses import dataclass
from math import ceil, log2
from typing import Iterable, Mapping, Sequence

from .certificates import CertificateSchedule


Pattern = tuple[int, ...]


@dataclass(frozen=True, slots=True)
class ProjectionView:
    """One affine mixed-radix view of a complete certificate pattern."""

    name: str
    radix: int
    depth: int
    coefficients: tuple[int, ...]

    def __post_init__(self) -> None:
        if not self.name:
            raise ValueError("projection view name must not be empty")
        if self.radix < 2:
            raise ValueError("projection radix must be at least two")
        if self.depth < 1:
            raise ValueError("projection depth must be positive")
        if not self.coefficients:
            raise ValueError("projection coefficients must not be empty")

    @property
    def bucket_count(self) -> int:
        return self.radix**self.depth

    @property
    def degree(self) -> int:
        return len(self.coefficients)


@dataclass(frozen=True, slots=True)
class BinaryLinearProjectionView:
    """A prefix of an ordered binary linear transform of all pattern bits."""

    name: str
    depth: int
    degree: int
    atom_bit_width: int
    row_masks: tuple[int, ...]

    def __post_init__(self) -> None:
        if not self.name:
            raise ValueError("projection view name must not be empty")
        if self.depth < 1 or self.degree < 1 or self.atom_bit_width < 1:
            raise ValueError("depth, degree, and atom bit width must be positive")
        if len(self.row_masks) < self.depth:
            raise ValueError("projection requires at least one row per depth bit")
        input_width = self.degree * self.atom_bit_width
        if any(row < 0 or row >= 1 << input_width for row in self.row_masks):
            raise ValueError("binary row mask exceeds the flattened pattern width")

    @property
    def radix(self) -> int:
        return 2

    @property
    def bucket_count(self) -> int:
        return 1 << self.depth


Projection = ProjectionView | BinaryLinearProjectionView


def compile_balanced_binary_rows(
    degree: int,
    atom_bit_width: int,
    output_bits: int,
) -> tuple[int, ...]:
    """Compile a deterministic diagnostic mixer with no zero input column.

    This is a support-balanced control, not an obstruction-selected production
    route. Every individual flattened input bit affects at least one of the
    declared output bits.
    """

    if degree < 1 or atom_bit_width < 1 or output_bits < 1:
        raise ValueError("degree, atom width, and output width must be positive")
    column_modulus = (1 << output_bits) - 1
    rows = [0] * output_bits
    for input_bit in range(degree * atom_bit_width):
        column = 1 + (7 * input_bit + 3 * (input_bit // atom_bit_width)) % (
            column_modulus
        )
        for output_bit in range(output_bits):
            if column & (1 << output_bit):
                rows[output_bit] |= 1 << input_bit
    return tuple(rows)


@dataclass(frozen=True, slots=True)
class ProtectedBinaryRowCompilation:
    """Ordered ambient rows with an audited protected-kernel prefix."""

    row_masks: tuple[int, ...]
    protected_difference_count: int
    protected_rank: int
    semantic_depth: int | None
    semantic_depth_is_minimum: bool
    union_bound_depth: int
    unresolved_by_prefix: tuple[int, ...]


def _gf2_reduced_basis(vectors: Sequence[int]) -> tuple[tuple[int, ...], tuple[int, ...]]:
    """Return a reduced binary basis and its unique pivot positions."""

    by_pivot: dict[int, int] = {}
    for vector in vectors:
        value = vector
        while value:
            pivot = value.bit_length() - 1
            if pivot in by_pivot:
                value ^= by_pivot[pivot]
            else:
                by_pivot[pivot] = value
                break
    for pivot in sorted(by_pivot):
        row = by_pivot[pivot]
        for other in tuple(by_pivot):
            if other != pivot and by_pivot[other] & (1 << pivot):
                by_pivot[other] ^= row
    pivots = tuple(sorted(by_pivot))
    return tuple(by_pivot[pivot] for pivot in pivots), pivots


def compile_protected_binary_rows(
    degree: int,
    atom_bit_width: int,
    output_bits: int,
    protected_differences: Iterable[int],
    *,
    exact_rank_limit: int = 18,
) -> ProtectedBinaryRowCompilation:
    """Compile rows whose prefix avoids every named nonzero difference.

    The protected differences live in the flattened pattern-bit space.  Row
    selection is performed on their binary span.  Each selected functional is
    then lifted into the corresponding ambient coset using a support-balanced
    row; this changes unprotected columns without changing any protected
    response.

    Exhaustive functional selection is used only when the protected rank is at
    most ``exact_rank_limit``.  Above that boundary the candidate family is
    explicit and the result remains an upper bound, never a minimum claim.
    """

    if degree < 1 or atom_bit_width < 1 or output_bits < 1:
        raise ValueError("degree, atom width, and output width must be positive")
    input_width = degree * atom_bit_width
    differences = tuple(sorted(set(protected_differences)))
    if not differences:
        raise ValueError("at least one protected difference is required")
    if any(value <= 0 or value >= 1 << input_width for value in differences):
        raise ValueError("protected differences must be nonzero and fit the input")

    basis, pivots = _gf2_reduced_basis(differences)
    rank = len(basis)
    coordinates = tuple(
        sum(((value >> pivot) & 1) << index for index, pivot in enumerate(pivots))
        for value in differences
    )
    full_cover = (1 << len(differences)) - 1

    candidate_functionals: set[int]
    if rank <= exact_rank_limit:
        candidate_functionals = set(range(1, 1 << rank))
    else:
        candidate_functionals = {1 << index for index in range(rank)}
        candidate_functionals.add((1 << rank) - 1)
        for left in range(rank):
            for right in range(left + 1, rank):
                candidate_functionals.add((1 << left) | (1 << right))
        diagnostic = compile_balanced_binary_rows(1, rank, output_bits)
        candidate_functionals.update(diagnostic)

    coverage_to_functional: dict[int, int] = {}
    for functional in candidate_functionals:
        coverage = sum(
            ((functional & coordinate).bit_count() & 1) << index
            for index, coordinate in enumerate(coordinates)
        )
        if coverage:
            current = coverage_to_functional.get(coverage)
            if current is None or (functional.bit_count(), functional) < (
                current.bit_count(),
                current,
            ):
                coverage_to_functional[coverage] = functional

    selected: list[int] = []
    signatures = [0] * len(differences)
    unresolved_by_prefix: list[int] = []
    semantic_depth: int | None = None
    available = set(coverage_to_functional)
    for output_index in range(output_bits):
        unresolved = sum(
            1 << index for index, signature in enumerate(signatures) if signature == 0
        )
        if unresolved:
            coverage = max(
                available,
                key=lambda mask: (
                    (mask & unresolved).bit_count(),
                    -abs(2 * mask.bit_count() - len(differences)),
                    -coverage_to_functional[mask].bit_count(),
                    -coverage_to_functional[mask],
                ),
            )
        elif available:
            equal_pairs = {
                (left, right)
                for left in range(len(signatures))
                for right in range(left + 1, len(signatures))
                if signatures[left] == signatures[right]
            }
            coverage = max(
                available,
                key=lambda mask: (
                    sum(
                        ((mask >> left) ^ (mask >> right)) & 1
                        for left, right in equal_pairs
                    ),
                    -abs(2 * mask.bit_count() - len(differences)),
                    -coverage_to_functional[mask].bit_count(),
                    -coverage_to_functional[mask],
                ),
            )
        else:
            selected.append(0)
            unresolved_by_prefix.append(sum(signature == 0 for signature in signatures))
            continue

        functional = coverage_to_functional[coverage]
        selected.append(functional)
        available.remove(coverage)
        for index, coordinate in enumerate(coordinates):
            signatures[index] |= (
                (functional & coordinate).bit_count() & 1
            ) << output_index
        unresolved_count = sum(signature == 0 for signature in signatures)
        unresolved_by_prefix.append(unresolved_count)
        if unresolved_count == 0 and semantic_depth is None:
            semantic_depth = output_index + 1

    ambient_rows = compile_balanced_binary_rows(degree, atom_bit_width, output_bits)
    lifted_rows = []
    for ambient, desired in zip(ambient_rows, selected, strict=True):
        actual = sum(
            ((ambient & row).bit_count() & 1) << index
            for index, row in enumerate(basis)
        )
        correction = sum(
            1 << pivots[index]
            for index in range(rank)
            if ((actual ^ desired) >> index) & 1
        )
        lifted_rows.append(ambient ^ correction)

    for difference_index, difference in enumerate(differences):
        expected = signatures[difference_index]
        actual = sum(
            ((row & difference).bit_count() & 1) << index
            for index, row in enumerate(lifted_rows)
        )
        if actual != expected:
            raise AssertionError("ambient coset lift changed protected response")

    one_row_exists = full_cover in coverage_to_functional
    minimum_known = semantic_depth == 1 or (
        semantic_depth == 2 and not one_row_exists and rank <= exact_rank_limit
    )
    return ProtectedBinaryRowCompilation(
        row_masks=tuple(lifted_rows),
        protected_difference_count=len(differences),
        protected_rank=rank,
        semantic_depth=semantic_depth,
        semantic_depth_is_minimum=minimum_known,
        union_bound_depth=min(rank, ceil(log2(len(differences) + 1))),
        unresolved_by_prefix=tuple(unresolved_by_prefix),
    )


@dataclass(frozen=True, slots=True)
class ProjectedCertificateBuild:
    """Dense occupancy masks in certificate-major, then view-major order."""

    masks: tuple[tuple[int, ...], ...]
    payload_bits: int
    distinct_pattern_counts: tuple[int, ...]

    @property
    def posting_entries(self) -> int:
        return sum(mask.bit_count() for row in self.masks for mask in row)


@dataclass(frozen=True, slots=True)
class ProjectionBreakdown:
    """Exact collision and private-witness accounting for one pattern set."""

    pattern_count: int
    payload_bits: int
    occupied_cells: tuple[int, ...]
    collision_excess: tuple[int, ...]
    private_witness_counts: tuple[int, ...]
    invisible_pattern_count: int
    minimum_private_witnesses: int


@dataclass(frozen=True, slots=True)
class ViewMutationFlow:
    view_name: str
    cleared_bits: int
    set_bits: int
    bit_changes: int
    pattern_events: int
    collision_cancelled_events: int


@dataclass(frozen=True, slots=True)
class CertificateMutationFlow:
    certificate_index: int
    subset: tuple[int, ...]
    affected_offset_states: int
    before_pattern_count: int
    after_pattern_count: int
    removed_patterns: tuple[Pattern, ...]
    added_patterns: tuple[Pattern, ...]
    view_flows: tuple[ViewMutationFlow, ...]


@dataclass(frozen=True, slots=True)
class SymbolMutationAudit:
    source_position: int
    before_atom: str
    after_atom: str
    affected_certificates: int
    affected_offset_states: int
    exact_pattern_events: int
    projected_bit_changes: int
    certificates: tuple[CertificateMutationFlow, ...]

    @property
    def projection_invisible(self) -> bool:
        return self.projected_bit_changes == 0


def pattern_bucket(pattern: Sequence[int], view: Projection) -> int:
    """Project one whole pattern; no slot marginal is exposed independently."""

    if len(pattern) != view.degree:
        raise ValueError("pattern degree does not match projection view")
    if any(value < 0 for value in pattern):
        raise ValueError("atom codes must be nonnegative")
    if isinstance(view, BinaryLinearProjectionView):
        atom_limit = 1 << view.atom_bit_width
        if any(value >= atom_limit for value in pattern):
            raise ValueError("atom code exceeds the binary view width")
        flattened = sum(
            value << (slot * view.atom_bit_width)
            for slot, value in enumerate(pattern)
        )
        return sum(
            ((flattened & row).bit_count() & 1) << output_bit
            for output_bit, row in enumerate(view.row_masks[: view.depth])
        )
    return sum(
        coefficient * value
        for coefficient, value in zip(view.coefficients, pattern, strict=True)
    ) % view.bucket_count


def contract_count_vector(
    fine: Sequence[int],
    radix: int,
) -> tuple[int, ...]:
    """Contract residue counts from ``k**(d+1)`` cells to ``k**d`` cells."""

    if radix < 2 or len(fine) < radix or len(fine) % radix:
        raise ValueError("fine vector length must be a positive radix multiple")
    coarse_size = len(fine) // radix
    return tuple(
        sum(fine[parent + child * coarse_size] for child in range(radix))
        for parent in range(coarse_size)
    )


def contract_occupancy_mask(fine_mask: int, fine_size: int, radix: int) -> int:
    """OR-contract a fine occupancy mask along the same residue tower."""

    if fine_mask < 0:
        raise ValueError("occupancy mask must be nonnegative")
    if radix < 2 or fine_size < radix or fine_size % radix:
        raise ValueError("fine size must be a positive radix multiple")
    if fine_mask >> fine_size:
        raise ValueError("occupancy mask exceeds the declared fine size")
    coarse_size = fine_size // radix
    coarse_mask = 0
    for parent in range(coarse_size):
        if any(
            fine_mask & (1 << (parent + child * coarse_size))
            for child in range(radix)
        ):
            coarse_mask |= 1 << parent
    return coarse_mask


def certificate_pattern_sets(
    symbols: Sequence[str],
    schedule: CertificateSchedule,
    atom_codes: Mapping[str, int],
) -> tuple[frozenset[Pattern], ...]:
    """Return the idempotent complete-pattern oracle for every certificate."""

    if len(symbols) != schedule.source_length:
        raise ValueError("source length does not match the compiled schedule")
    patterns_by_certificate: list[frozenset[Pattern]] = []
    for subset in schedule.subsets:
        patterns: set[Pattern] = set()
        for offsets in schedule.offset_states:
            try:
                pattern = tuple(
                    atom_codes[symbols[target_index + offset]]
                    for target_index, offset in zip(subset, offsets, strict=True)
                )
            except KeyError as error:
                raise ValueError("source contains an atom without a code") from error
            patterns.add(pattern)
        patterns_by_certificate.append(frozenset(patterns))
    return tuple(patterns_by_certificate)


def build_projected_certificate_masks(
    symbols: Sequence[str],
    schedule: CertificateSchedule,
    atom_codes: Mapping[str, int],
    views: Sequence[Projection],
) -> ProjectedCertificateBuild:
    """Build fixed-width projected membership masks without history storage."""

    if not views:
        raise ValueError("at least one projection view is required")
    if any(view.degree != schedule.degree for view in views):
        raise ValueError("every projection view must match certificate degree")
    pattern_sets = certificate_pattern_sets(symbols, schedule, atom_codes)
    masks: list[tuple[int, ...]] = []
    for patterns in pattern_sets:
        certificate_masks = []
        for view in views:
            mask = 0
            for pattern in patterns:
                mask |= 1 << pattern_bucket(pattern, view)
            certificate_masks.append(mask)
        masks.append(tuple(certificate_masks))
    return ProjectedCertificateBuild(
        masks=tuple(masks),
        payload_bits=len(schedule.subsets) * sum(view.bucket_count for view in views),
        distinct_pattern_counts=tuple(len(patterns) for patterns in pattern_sets),
    )


def query_projected_certificate_keys(
    symbols: Sequence[str],
    schedule: CertificateSchedule,
    atom_codes: Mapping[str, int],
    views: Sequence[Projection],
) -> tuple[tuple[int, ...], ...]:
    """Project every complete query pattern into every declared view."""

    if len(symbols) != schedule.target_length:
        raise ValueError("query length does not match the compiled schedule")
    if any(view.degree != schedule.degree for view in views):
        raise ValueError("every projection view must match certificate degree")
    keys = []
    for subset in schedule.subsets:
        try:
            pattern = tuple(atom_codes[symbols[index]] for index in subset)
        except KeyError as error:
            raise ValueError("query contains an atom without a code") from error
        keys.append(tuple(pattern_bucket(pattern, view) for view in views))
    return tuple(keys)


def projected_certificate_accepts(
    masks: Sequence[Sequence[int]],
    query_keys: Sequence[Sequence[int]],
) -> bool:
    """Require every projected view while retaining candidate-only semantics."""

    if len(masks) != len(query_keys):
        raise ValueError("mask and query-key certificate counts must agree")
    for certificate_masks, certificate_keys in zip(masks, query_keys, strict=True):
        if len(certificate_masks) != len(certificate_keys):
            raise ValueError("mask and query-key view counts must agree")
        if not all(
            mask & (1 << key)
            for mask, key in zip(certificate_masks, certificate_keys, strict=True)
        ):
            return False
    return True


def projected_certificate_overlaps(
    left_masks: Sequence[Sequence[int]],
    right_masks: Sequence[Sequence[int]],
) -> bool:
    """Candidate test for two same-length sources via occupied-cell overlap."""

    if len(left_masks) != len(right_masks):
        raise ValueError("left and right certificate counts must agree")
    for left_views, right_views in zip(left_masks, right_masks, strict=True):
        if len(left_views) != len(right_views):
            raise ValueError("left and right view counts must agree")
        if not all(
            left & right
            for left, right in zip(left_views, right_views, strict=True)
        ):
            return False
    return True


def audit_pattern_projection(
    patterns: Iterable[Pattern],
    views: Sequence[Projection],
) -> ProjectionBreakdown:
    """Expose collisions and per-pattern private cells rather than hiding them."""

    unique_patterns = tuple(sorted(set(patterns)))
    if not views:
        raise ValueError("at least one projection view is required")
    if unique_patterns and any(
        len(pattern) != view.degree
        for pattern in unique_patterns
        for view in views
    ):
        raise ValueError("pattern and projection degrees must agree")

    bucket_counts: list[dict[int, int]] = []
    for view in views:
        counts: dict[int, int] = {}
        for pattern in unique_patterns:
            bucket = pattern_bucket(pattern, view)
            counts[bucket] = counts.get(bucket, 0) + 1
        bucket_counts.append(counts)

    private_counts = tuple(
        sum(
            bucket_counts[index][pattern_bucket(pattern, view)] == 1
            for index, view in enumerate(views)
        )
        for pattern in unique_patterns
    )
    return ProjectionBreakdown(
        pattern_count=len(unique_patterns),
        payload_bits=sum(view.bucket_count for view in views),
        occupied_cells=tuple(len(counts) for counts in bucket_counts),
        collision_excess=tuple(
            len(unique_patterns) - len(counts) for counts in bucket_counts
        ),
        private_witness_counts=private_counts,
        invisible_pattern_count=sum(count == 0 for count in private_counts),
        minimum_private_witnesses=min(private_counts, default=0),
    )


def audit_symbol_mutation(
    symbols: Sequence[str],
    source_position: int,
    after_atom: str,
    schedule: CertificateSchedule,
    atom_codes: Mapping[str, int],
    views: Sequence[Projection],
) -> SymbolMutationAudit:
    """Trace one source-position change through exact sets and projected bits."""

    if len(symbols) != schedule.source_length:
        raise ValueError("source length does not match the compiled schedule")
    if not 0 <= source_position < len(symbols):
        raise ValueError("source position is outside the source")
    if after_atom not in atom_codes:
        raise ValueError("changed atom has no code")
    if any(view.degree != schedule.degree for view in views):
        raise ValueError("every projection view must match certificate degree")

    before_symbols = tuple(symbols)
    changed = list(before_symbols)
    before_atom = changed[source_position]
    changed[source_position] = after_atom
    after_symbols = tuple(changed)
    before_sets = certificate_pattern_sets(before_symbols, schedule, atom_codes)
    after_sets = certificate_pattern_sets(after_symbols, schedule, atom_codes)

    certificate_flows = []
    for certificate_index, (subset, before, after) in enumerate(
        zip(schedule.subsets, before_sets, after_sets, strict=True)
    ):
        affected_states = sum(
            any(
                target_index + offset == source_position
                for target_index, offset in zip(subset, offsets, strict=True)
            )
            for offsets in schedule.offset_states
        )
        removed = tuple(sorted(before - after))
        added = tuple(sorted(after - before))
        pattern_events = len(removed) + len(added)
        view_flows = []
        for view in views:
            before_mask = 0
            after_mask = 0
            for pattern in before:
                before_mask |= 1 << pattern_bucket(pattern, view)
            for pattern in after:
                after_mask |= 1 << pattern_bucket(pattern, view)
            cleared = (before_mask & ~after_mask).bit_count()
            set_count = (after_mask & ~before_mask).bit_count()
            bit_changes = cleared + set_count
            view_flows.append(
                ViewMutationFlow(
                    view_name=view.name,
                    cleared_bits=cleared,
                    set_bits=set_count,
                    bit_changes=bit_changes,
                    pattern_events=pattern_events,
                    collision_cancelled_events=pattern_events - bit_changes,
                )
            )
        certificate_flows.append(
            CertificateMutationFlow(
                certificate_index=certificate_index,
                subset=subset,
                affected_offset_states=affected_states,
                before_pattern_count=len(before),
                after_pattern_count=len(after),
                removed_patterns=removed,
                added_patterns=added,
                view_flows=tuple(view_flows),
            )
        )

    return SymbolMutationAudit(
        source_position=source_position,
        before_atom=before_atom,
        after_atom=after_atom,
        affected_certificates=sum(
            flow.affected_offset_states > 0 for flow in certificate_flows
        ),
        affected_offset_states=sum(
            flow.affected_offset_states for flow in certificate_flows
        ),
        exact_pattern_events=sum(
            len(flow.removed_patterns) + len(flow.added_patterns)
            for flow in certificate_flows
        ),
        projected_bit_changes=sum(
            view_flow.bit_changes
            for flow in certificate_flows
            for view_flow in flow.view_flows
        ),
        certificates=tuple(certificate_flows),
    )
