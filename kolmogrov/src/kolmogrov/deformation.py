"""Reference and streaming algorithms for deformation-jet addresses.

The three paths compute the same pre-quantized modular address:

* descendant enumeration is the exact deformation-history oracle;
* occurrence products collapse the history loop;
* streaming dynamic programming collapses both history and occurrence loops.

The modular ring is an implementation control.  It does not select the release
field, phase family, normalization, or quantizer.
"""

from __future__ import annotations

from bisect import bisect_left
from dataclasses import dataclass
from itertools import combinations
from math import comb
from typing import Callable, Sequence

from .geometry import gaps_from_indices


ContentWeight = Callable[[int, str], int]


@dataclass(frozen=True, slots=True)
class IterationCounts:
    """Named loop bodies executed by one address construction."""

    histories: int = 0
    occurrences: int = 0
    factor_cells: int = 0
    state_cells: int = 0

    @property
    def total(self) -> int:
        return self.histories + self.occurrences + self.factor_cells + self.state_cells


@dataclass(frozen=True, slots=True)
class AddressComputation:
    coefficients: tuple[int, ...]
    iterations: IterationCounts


@dataclass(frozen=True, slots=True)
class LadderComputation:
    coefficients_by_degree: tuple[tuple[int, ...], ...]
    iterations: IterationCounts


@dataclass(frozen=True, slots=True)
class WorkEstimate:
    descendant_histories: int
    descendant_occurrences: int
    occurrence_products: int
    occurrence_factor_cells: int
    streaming_state_cells: int


def _symbol_runs(symbols: Sequence[str]) -> tuple[tuple[str, int], ...]:
    runs: list[tuple[str, int]] = []
    for symbol in symbols:
        if runs and runs[-1][0] == symbol:
            prior_symbol, length = runs[-1]
            runs[-1] = (prior_symbol, length + 1)
        else:
            runs.append((symbol, 1))
    return tuple(runs)


def _validate_address_inputs(
    symbols: Sequence[str],
    degree: int,
    max_radius: int,
    gap_weights: Sequence[int],
    position_weights: Sequence[int],
    modulus: int,
) -> None:
    if not 0 <= degree <= len(symbols):
        raise ValueError("degree must lie between zero and object length")
    if max_radius < 0:
        raise ValueError("maximum radius must be non-negative")
    if len(gap_weights) != degree + 1:
        raise ValueError("degree k requires k+1 gap weights")
    if len(position_weights) != len(symbols):
        raise ValueError("one history weight is required per source position")
    if modulus <= 1:
        raise ValueError("modulus must exceed one")


def _pattern_weight(
    symbols: Sequence[str],
    indices: Sequence[int],
    content_weight: ContentWeight,
    modulus: int,
) -> int:
    value = 1
    for slot, index in enumerate(indices):
        value = value * content_weight(slot, symbols[index]) % modulus
    return value


def descendant_enumerated_address(
    symbols: Sequence[str],
    degree: int,
    max_radius: int,
    gap_weights: Sequence[int],
    position_weights: Sequence[int],
    content_weight: ContentWeight,
    modulus: int,
) -> AddressComputation:
    """Enumerate deletion histories and descendant occurrences exactly."""

    _validate_address_inputs(
        symbols,
        degree,
        max_radius,
        gap_weights,
        position_weights,
        modulus,
    )
    size = len(symbols)
    radius_limit = min(max_radius, size - degree)
    coefficients = [0] * (max_radius + 1)
    history_iterations = 0
    occurrence_iterations = 0

    for radius in range(radius_limit + 1):
        for deleted_tuple in combinations(range(size), radius):
            history_iterations += 1
            deleted = set(deleted_tuple)
            kept = tuple(index for index in range(size) if index not in deleted)
            target_symbols = tuple(symbols[index] for index in kept)
            history_weight = 1
            for index in deleted_tuple:
                history_weight = history_weight * position_weights[index] % modulus

            for target_indices in combinations(range(len(kept)), degree):
                occurrence_iterations += 1
                value = history_weight
                value = (
                    value
                    * _pattern_weight(
                        target_symbols,
                        target_indices,
                        content_weight,
                        modulus,
                    )
                    % modulus
                )
                for gap_index, gap in enumerate(
                    gaps_from_indices(len(kept), target_indices)
                ):
                    value = value * pow(gap_weights[gap_index], gap, modulus) % modulus
                coefficients[radius] = (coefficients[radius] + value) % modulus

    return AddressComputation(
        tuple(coefficients),
        IterationCounts(
            histories=history_iterations,
            occurrences=occurrence_iterations,
        ),
    )


def occurrence_product_address(
    symbols: Sequence[str],
    degree: int,
    max_radius: int,
    gap_weights: Sequence[int],
    position_weights: Sequence[int],
    content_weight: ContentWeight,
    modulus: int,
) -> AddressComputation:
    """Enumerate source occurrences but collapse every deletion history."""

    _validate_address_inputs(
        symbols,
        degree,
        max_radius,
        gap_weights,
        position_weights,
        modulus,
    )
    size = len(symbols)
    radius_limit = min(max_radius, size - degree)
    coefficients = [0] * (max_radius + 1)
    occurrence_iterations = 0
    factor_cells = 0

    for selected in combinations(range(size), degree):
        occurrence_iterations += 1
        selected_set = set(selected)
        polynomial = [1] + [0] * radius_limit
        factors_seen = 0
        for position in range(size):
            if position in selected_set:
                continue
            gap_index = bisect_left(selected, position)
            survive_weight = gap_weights[gap_index]
            delete_weight = position_weights[position]
            maximum = min(radius_limit, factors_seen + 1)
            next_polynomial = [0] * (radius_limit + 1)
            for radius in range(maximum + 1):
                factor_cells += 1
                value = survive_weight * polynomial[radius]
                if radius:
                    value += delete_weight * polynomial[radius - 1]
                next_polynomial[radius] = value % modulus
            polynomial = next_polynomial
            factors_seen += 1

        content = _pattern_weight(symbols, selected, content_weight, modulus)
        for radius, value in enumerate(polynomial):
            coefficients[radius] = (coefficients[radius] + content * value) % modulus

    return AddressComputation(
        tuple(coefficients),
        IterationCounts(
            occurrences=occurrence_iterations,
            factor_cells=factor_cells,
        ),
    )


def streaming_deformation_address(
    symbols: Sequence[str],
    degree: int,
    max_radius: int,
    gap_weights: Sequence[int],
    position_weights: Sequence[int],
    content_weight: ContentWeight,
    modulus: int,
) -> AddressComputation:
    """Compute the whole address with one position/degree/radius recurrence.

    After reading a source position, a state may retain it in the current gap,
    delete it into the history phase, or select it as the next content slot.
    The direct recurrence performs one assignment per reachable state cell.
    """

    ladder = streaming_deformation_ladder(
        symbols,
        degree,
        max_radius,
        gap_weights,
        position_weights,
        content_weight,
        modulus,
    )
    return AddressComputation(
        ladder.coefficients_by_degree[degree],
        ladder.iterations,
    )


def streaming_deformation_ladder(
    symbols: Sequence[str],
    maximum_degree: int,
    max_radius: int,
    gap_weights: Sequence[int],
    position_weights: Sequence[int],
    content_weight: ContentWeight,
    modulus: int,
) -> LadderComputation:
    """Emit every degree through ``maximum_degree`` from one streaming table."""

    _validate_address_inputs(
        symbols,
        maximum_degree,
        max_radius,
        gap_weights,
        position_weights,
        modulus,
    )
    size = len(symbols)
    radius_limit = min(max_radius, size)
    stride = radius_limit + 1
    cell_count = (maximum_degree + 1) * stride
    previous = [0] * cell_count
    following = [0] * cell_count
    previous[0] = 1
    state_iterations = 0

    for source_index, symbol in enumerate(symbols):
        processed = source_index + 1
        maximum_selected = min(maximum_degree, processed)
        for selected_count in range(maximum_selected + 1):
            maximum_reachable_radius = min(
                radius_limit,
                processed - selected_count,
            )
            row = selected_count * stride
            prior_row = (selected_count - 1) * stride
            gap_weight = gap_weights[selected_count]
            selected_weight = (
                content_weight(selected_count - 1, symbol)
                if selected_count
                else 0
            )
            for radius in range(maximum_reachable_radius + 1):
                state_iterations += 1
                value = gap_weight * previous[row + radius]
                if radius:
                    value += position_weights[source_index] * previous[row + radius - 1]
                if selected_count:
                    value += selected_weight * previous[prior_row + radius]
                following[row + radius] = value % modulus
        previous, following = following, previous

    coefficients_by_degree = tuple(
        tuple(
            previous[degree * stride + radius]
            if radius <= size - degree
            else 0
            for radius in range(max_radius + 1)
        )
        for degree in range(maximum_degree + 1)
    )
    return LadderComputation(
        coefficients_by_degree,
        IterationCounts(state_cells=state_iterations),
    )


def streaming_deformation_ladder_radius2(
    symbols: Sequence[str],
    maximum_degree: int,
    gap_weights: Sequence[int],
    position_weights: Sequence[int],
    content_weight: ContentWeight,
    modulus: int,
) -> LadderComputation:
    """Radius-two rectangular specialization of the streaming recurrence.

    This form deliberately computes the few unreachable boundary cells.  It
    removes the radius loop, reachability bounds, and conditional branches from
    the active inner body.  The intended native form additionally batches probe
    channels across the three adjacent radius cells.
    """

    _validate_address_inputs(
        symbols,
        maximum_degree,
        2,
        gap_weights,
        position_weights,
        modulus,
    )
    cell_count = (maximum_degree + 1) * 3
    previous = [0] * cell_count
    following = [0] * cell_count
    previous[0] = 1

    for source_index, symbol in enumerate(symbols):
        history_weight = position_weights[source_index]
        following[0] = gap_weights[0] * previous[0] % modulus
        following[1] = (
            gap_weights[0] * previous[1] + history_weight * previous[0]
        ) % modulus
        following[2] = (
            gap_weights[0] * previous[2] + history_weight * previous[1]
        ) % modulus
        for selected_count in range(1, maximum_degree + 1):
            row = selected_count * 3
            prior_row = row - 3
            selected_weight = content_weight(selected_count - 1, symbol)
            gap_weight = gap_weights[selected_count]
            following[row] = (
                gap_weight * previous[row]
                + selected_weight * previous[prior_row]
            ) % modulus
            following[row + 1] = (
                gap_weight * previous[row + 1]
                + history_weight * previous[row]
                + selected_weight * previous[prior_row + 1]
            ) % modulus
            following[row + 2] = (
                gap_weight * previous[row + 2]
                + history_weight * previous[row + 1]
                + selected_weight * previous[prior_row + 2]
            ) % modulus
        previous, following = following, previous

    coefficients_by_degree = tuple(
        tuple(
            previous[degree * 3 + radius]
            if radius <= len(symbols) - degree
            else 0
            for radius in range(3)
        )
        for degree in range(maximum_degree + 1)
    )
    return LadderComputation(
        coefficients_by_degree,
        IterationCounts(
            state_cells=len(symbols) * (maximum_degree + 1) * 3,
        ),
    )


def _matrix_multiply(
    left: Sequence[Sequence[int]],
    right: Sequence[Sequence[int]],
    modulus: int,
) -> list[list[int]]:
    dimension = len(left)
    result = [[0] * dimension for _ in range(dimension)]
    for row in range(dimension):
        for middle in range(row + 1):
            left_value = left[row][middle]
            if not left_value:
                continue
            for column in range(middle + 1):
                result[row][column] = (
                    result[row][column]
                    + left_value * right[middle][column]
                ) % modulus
    return result


def _matrix_power(
    matrix: Sequence[Sequence[int]],
    exponent: int,
    modulus: int,
) -> list[list[int]]:
    dimension = len(matrix)
    result = [
        [int(row == column) for column in range(dimension)]
        for row in range(dimension)
    ]
    factor = [list(row) for row in matrix]
    while exponent:
        if exponent & 1:
            result = _matrix_multiply(factor, result, modulus)
        exponent >>= 1
        if exponent:
            factor = _matrix_multiply(factor, factor, modulus)
    return result


def run_quotient_deformation_ladder(
    symbols: Sequence[str],
    maximum_degree: int,
    max_radius: int,
    gap_weights: Sequence[int],
    run_weights: Sequence[int],
    content_weight: ContentWeight,
    modulus: int,
) -> LadderComputation:
    """Sum once per run-deletion-count vector, not once per position set.

    Deleting ``d`` positions from one constant-symbol run has one observable
    descendant effect regardless of which positions were deleted.  A run block
    therefore contributes one branch for each ``d`` rather than the binomial
    multiplicity ``C(length, d)``.  The returned radius rows are a safe local
    history quotient; different run-count vectors that happen to yield the same
    complete descendant remain distinct.
    """

    if not 0 <= maximum_degree <= len(symbols):
        raise ValueError("maximum degree must lie between zero and object length")
    if max_radius < 0:
        raise ValueError("maximum radius must be non-negative")
    if len(gap_weights) != maximum_degree + 1:
        raise ValueError("maximum degree K requires K+1 gap weights")
    if modulus <= 1:
        raise ValueError("modulus must exceed one")
    runs = _symbol_runs(symbols)
    if len(run_weights) != len(runs):
        raise ValueError("one history weight is required per constant-symbol run")

    stride = max_radius + 1
    dimension = maximum_degree + 1
    cell_count = dimension * stride
    previous = [0] * cell_count
    following = [0] * cell_count
    previous[0] = 1
    transfer_iterations = 0

    for run_index, (symbol, run_length) in enumerate(runs):
        if run_length == 1:
            history_weight = run_weights[run_index]
            for degree in range(dimension):
                selected_weight = (
                    content_weight(degree - 1, symbol) if degree else 0
                )
                for radius in range(stride):
                    transfer_iterations += 1
                    value = gap_weights[degree] * previous[degree * stride + radius]
                    if radius:
                        value += (
                            history_weight
                            * previous[degree * stride + radius - 1]
                        )
                    if degree:
                        value += (
                            selected_weight
                            * previous[(degree - 1) * stride + radius]
                        )
                    following[degree * stride + radius] = value % modulus
            previous, following = following, previous
            continue

        base = [[0] * dimension for _ in range(dimension)]
        for degree in range(dimension):
            base[degree][degree] = gap_weights[degree] % modulus
            if degree:
                base[degree][degree - 1] = content_weight(degree - 1, symbol) % modulus

        maximum_run_deletions = min(max_radius, run_length)
        minimum_survivors = run_length - maximum_run_deletions
        power = _matrix_power(base, minimum_survivors, modulus)
        powers_by_survivors = {minimum_survivors: power}
        for survivors in range(minimum_survivors + 1, run_length + 1):
            power = _matrix_multiply(base, power, modulus)
            powers_by_survivors[survivors] = power

        for cell in range(cell_count):
            following[cell] = 0
        phase_powers = [1]
        for deletion_count in range(1, maximum_run_deletions + 1):
            phase_powers.append(
                phase_powers[-1] * run_weights[run_index] % modulus
            )

        for prior_radius in range(max_radius + 1):
            for deletion_count in range(
                min(maximum_run_deletions, max_radius - prior_radius) + 1
            ):
                transfer = powers_by_survivors[run_length - deletion_count]
                phase = phase_powers[deletion_count]
                next_radius = prior_radius + deletion_count
                for output_degree in range(dimension):
                    value = 0
                    for input_degree in range(output_degree + 1):
                        transfer_iterations += 1
                        value += (
                            transfer[output_degree][input_degree]
                            * previous[input_degree * stride + prior_radius]
                        )
                    following[output_degree * stride + next_radius] = (
                        following[output_degree * stride + next_radius]
                        + phase * value
                    ) % modulus
        previous, following = following, previous

    coefficients_by_degree = tuple(
        tuple(
            previous[degree * stride + radius]
            if radius <= len(symbols) - degree
            else 0
            for radius in range(max_radius + 1)
        )
        for degree in range(maximum_degree + 1)
    )
    return LadderComputation(
        coefficients_by_degree,
        IterationCounts(state_cells=transfer_iterations),
    )


def work_estimate(size: int, degree: int, max_radius: int) -> WorkEstimate:
    """Closed iteration counts for the three construction shapes."""

    if not 0 <= degree <= size:
        raise ValueError("degree must lie between zero and object size")
    if max_radius < 0:
        raise ValueError("maximum radius must be non-negative")
    radius_limit = min(max_radius, size - degree)
    histories = sum(comb(size, radius) for radius in range(radius_limit + 1))
    descendant_occurrences = sum(
        comb(size, radius) * comb(size - radius, degree)
        for radius in range(radius_limit + 1)
    )
    source_occurrences = comb(size, degree)
    factor_cells_per_occurrence = sum(
        min(radius_limit, factor_index + 1) + 1
        for factor_index in range(size - degree)
    )
    streaming_radius_limit = min(max_radius, size)
    streaming_cells = sum(
        min(streaming_radius_limit, processed - selected) + 1
        for processed in range(1, size + 1)
        for selected in range(min(degree, processed) + 1)
    )
    return WorkEstimate(
        descendant_histories=histories,
        descendant_occurrences=descendant_occurrences,
        occurrence_products=source_occurrences,
        occurrence_factor_cells=source_occurrences * factor_cells_per_occurrence,
        streaming_state_cells=streaming_cells,
    )
