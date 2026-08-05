from __future__ import annotations

import cmath
import json
import platform
import statistics
import sys
import time
from itertools import combinations, product
from math import comb, pi
from pathlib import Path
from typing import Callable, Iterable

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    descendant_enumerated_address,
    occurrence_product_address,
    streaming_deformation_address,
    work_estimate,
)


def is_prime(value: int) -> bool:
    if value < 2:
        return False
    divisor = 2
    while divisor * divisor <= value:
        if value % divisor == 0:
            return False
        divisor += 1
    return True


def next_prime(minimum: int) -> int:
    value = max(2, minimum)
    while not is_prime(value):
        value += 1
    return value


def moment_code(history: tuple[int, ...], radius: int, modulus: int) -> tuple[int, ...]:
    return tuple(
        sum(pow(position, power, modulus) for position in history) % modulus
        for power in range(1, radius + 1)
    )


def character(frequency: tuple[int, ...], code: tuple[int, ...], modulus: int) -> complex:
    exponent = sum(left * right for left, right in zip(frequency, code, strict=True))
    return cmath.exp(2j * pi * (exponent % modulus) / modulus)


def greedy_frequency_order(
    modulus: int,
    radius: int,
    maximum_size: int,
) -> tuple[list[tuple[int, ...]], int]:
    frequencies = list(product(range(modulus), repeat=radius))
    differences = frequencies[1:]
    phase = [
        [character(frequency, difference, modulus) for difference in differences]
        for frequency in frequencies
    ]
    selected_indices: list[int] = []
    selected = set()
    sums = [0j] * len(differences)
    candidate_difference_iterations = 0

    while len(selected_indices) < min(maximum_size, len(frequencies)):
        denominator = len(selected_indices) + 1
        best_index = -1
        best_sidelobe = float("inf")
        for candidate_index, candidate_phase in enumerate(phase):
            if candidate_index in selected:
                continue
            candidate_difference_iterations += len(differences)
            sidelobe = max(
                abs(current + added) / denominator
                for current, added in zip(sums, candidate_phase, strict=True)
            )
            if sidelobe < best_sidelobe - 1e-15:
                best_index = candidate_index
                best_sidelobe = sidelobe
        selected.add(best_index)
        selected_indices.append(best_index)
        sums = [
            current + added
            for current, added in zip(sums, phase[best_index], strict=True)
        ]

    return [frequencies[index] for index in selected_indices], candidate_difference_iterations


def kernel_sidelobe(
    frequencies: Iterable[tuple[int, ...]],
    modulus: int,
    radius: int,
) -> float:
    selected = tuple(frequencies)
    if not selected:
        raise ValueError("at least one frequency is required")
    return max(
        abs(
            sum(character(frequency, difference, modulus) for frequency in selected)
            / len(selected)
        )
        for difference in product(range(modulus), repeat=radius)
        if any(difference)
    )


def history_support_masks(source: str, radius: int) -> tuple[dict[tuple[str, int], int], tuple[tuple[int, ...], ...]]:
    histories = tuple(combinations(range(len(source)), radius))
    masks: dict[tuple[str, int], int] = {}
    for history_index, history in enumerate(histories):
        deleted = set(history)
        descendant = "".join(
            symbol for index, symbol in enumerate(source) if index not in deleted
        )
        bit = 1 << history_index
        for target_index, symbol in enumerate(descendant):
            feature = (symbol, target_index)
            masks[feature] = masks.get(feature, 0) | bit
    return masks, histories


def query_census(maximum_source_length: int) -> dict[str, object]:
    rows = []
    first_marginal_false = None
    first_pairwise_false = None
    for source_length in range(2, maximum_source_length + 1):
        for radius in range(1, min(2, source_length - 1) + 1):
            target_length = source_length - radius
            row = {
                "exact_descendants": 0,
                "marginal_accepts": 0,
                "marginal_false_accepts": 0,
                "pairwise_accepts": 0,
                "pairwise_false_accepts": 0,
                "radius": radius,
                "source_length": source_length,
                "source_query_pairs": 0,
            }
            for source_tuple in product("ab", repeat=source_length):
                source = "".join(source_tuple)
                masks, _ = history_support_masks(source, radius)
                for query_tuple in product("ab", repeat=target_length):
                    query = "".join(query_tuple)
                    row["source_query_pairs"] += 1
                    feature_masks = [
                        masks.get((symbol, index), 0)
                        for index, symbol in enumerate(query)
                    ]
                    marginal = all(feature_masks)
                    common = feature_masks[0]
                    for mask in feature_masks[1:]:
                        common &= mask
                    exact = bool(common)
                    pairwise = marginal and all(
                        left & right
                        for left, right in combinations(feature_masks, 2)
                    )
                    row["exact_descendants"] += exact
                    row["marginal_accepts"] += marginal
                    row["marginal_false_accepts"] += marginal and not exact
                    row["pairwise_accepts"] += pairwise
                    row["pairwise_false_accepts"] += pairwise and not exact
                    if marginal and not exact and first_marginal_false is None:
                        first_marginal_false = {
                            "query": query,
                            "radius": radius,
                            "source": source,
                        }
                    if pairwise and not exact and first_pairwise_false is None:
                        first_pairwise_false = {
                            "query": query,
                            "radius": radius,
                            "source": source,
                        }
            rows.append(row)
    return {
        "first_marginal_false": first_marginal_false,
        "first_pairwise_false": first_pairwise_false,
        "rows": rows,
    }


def coherence_census(source_length: int, radius: int) -> dict[str, object]:
    modulus = next_prime(max(source_length - 1, radius) + 1)
    histories = tuple(combinations(range(source_length), radius))
    codes = tuple(moment_code(history, radius, modulus) for history in histories)
    full_frequencies = tuple(product(range(modulus), repeat=radius))
    requested_sizes = tuple(
        size for size in (1, 2, 4, 8, 16, 32) if size < len(full_frequencies)
    )
    greedy, greedy_iterations = greedy_frequency_order(
        modulus,
        radius,
        max(requested_sizes, default=1),
    )
    configurations = {
        str(size): tuple(greedy[:size]) for size in requested_sizes
    }
    configurations["full"] = full_frequencies

    phase_by_configuration = {
        label: tuple(
            tuple(character(frequency, code, modulus) for code in codes)
            for frequency in frequencies
        )
        for label, frequencies in configurations.items()
    }
    response_cache: dict[tuple[str, int], tuple[complex, ...]] = {}

    def response(label: str, mask: int) -> tuple[complex, ...]:
        key = (label, mask)
        cached = response_cache.get(key)
        if cached is not None:
            return cached
        rows = phase_by_configuration[label]
        value = tuple(
            sum(
                row[history_index]
                for history_index in range(len(histories))
                if mask & (1 << history_index)
            )
            for row in rows
        )
        response_cache[key] = value
        return value

    stats = {
        label: {
            "frequency_count": len(frequencies),
            "max_absolute_error": 0.0,
            "max_no_common_real_score": float("-inf"),
            "min_positive_real_score": float("inf"),
            "negative_pairs": 0,
            "positive_pairs": 0,
            "sidelobe": kernel_sidelobe(frequencies, modulus, radius),
        }
        for label, frequencies in configurations.items()
    }
    maximum_history_support = 0

    for source_tuple in product("ab", repeat=source_length):
        source = "".join(source_tuple)
        masks, source_histories = history_support_masks(source, radius)
        assert source_histories == histories
        nonempty_masks = tuple(masks.values())
        maximum_history_support = max(
            maximum_history_support,
            *(mask.bit_count() for mask in nonempty_masks),
        )
        for left_mask, right_mask in combinations(nonempty_masks, 2):
            exact_common = (left_mask & right_mask).bit_count()
            for label, row in stats.items():
                left_response = response(label, left_mask)
                right_response = response(label, right_mask)
                score = sum(
                    left * right.conjugate()
                    for left, right in zip(
                        left_response,
                        right_response,
                        strict=True,
                    )
                ) / row["frequency_count"]
                error = abs(score - exact_common)
                row["max_absolute_error"] = max(row["max_absolute_error"], error)
                if exact_common:
                    row["positive_pairs"] += 1
                    row["min_positive_real_score"] = min(
                        row["min_positive_real_score"], score.real
                    )
                else:
                    row["negative_pairs"] += 1
                    row["max_no_common_real_score"] = max(
                        row["max_no_common_real_score"], score.real
                    )

    for row in stats.values():
        row["observed_pair_separation_margin"] = (
            row["min_positive_real_score"] - row["max_no_common_real_score"]
        )
        row["theorem_margin_before_quantization"] = 1 - row["sidelobe"] * (
            2 * maximum_history_support**2 - 1
        )
        for key in (
            "max_absolute_error",
            "max_no_common_real_score",
            "min_positive_real_score",
            "observed_pair_separation_margin",
            "sidelobe",
            "theorem_margin_before_quantization",
        ):
            row[key] = round(row[key], 9)

    return {
        "greedy_candidate_difference_iterations": greedy_iterations,
        "history_count": len(histories),
        "maximum_feature_history_support": maximum_history_support,
        "moment_modulus": modulus,
        "radius": radius,
        "source_length": source_length,
        "statistics_by_frequency_count": stats,
    }


def median_build_ns(function: Callable[[], object], target_batch_ns: int = 100_000_000) -> int:
    started = time.perf_counter_ns()
    function()
    first = max(1, time.perf_counter_ns() - started)
    inner = max(1, min(2_000, target_batch_ns // first))
    batches = []
    for _ in range(5):
        started = time.perf_counter_ns()
        for _ in range(inner):
            function()
        batches.append((time.perf_counter_ns() - started) // inner)
    return int(statistics.median(batches))


def algorithm_census() -> list[dict[str, object]]:
    modulus = (1 << 61) - 1
    rows = []
    for size in (8, 12, 16, 20):
        degree = 3
        max_radius = 2
        symbols = tuple("ab"[(index * index + 3 * index + 1) & 1] for index in range(size))
        gap_weights = tuple(3 + 2 * index for index in range(degree + 1))
        position_weights = tuple(pow(7, index + 1, modulus) for index in range(size))

        def content_weight(slot: int, symbol: str) -> int:
            return 11 + 5 * slot + (3 if symbol == "b" else 0)

        def descendant() -> object:
            return descendant_enumerated_address(
                symbols,
                degree,
                max_radius,
                gap_weights,
                position_weights,
                content_weight,
                modulus,
            )

        def occurrence() -> object:
            return occurrence_product_address(
                symbols,
                degree,
                max_radius,
                gap_weights,
                position_weights,
                content_weight,
                modulus,
            )

        def streaming() -> object:
            return streaming_deformation_address(
                symbols,
                degree,
                max_radius,
                gap_weights,
                position_weights,
                content_weight,
                modulus,
            )

        descendant_result = descendant()
        occurrence_result = occurrence()
        streaming_result = streaming()
        assert (
            descendant_result.coefficients
            == occurrence_result.coefficients
            == streaming_result.coefficients
        )
        estimate = work_estimate(size, degree, max_radius)
        timings = {
            "descendant_enumeration": median_build_ns(descendant),
            "occurrence_product": median_build_ns(occurrence),
            "streaming_dp": median_build_ns(streaming),
        }
        rows.append(
            {
                "degree": degree,
                "iteration_counts": {
                    "descendant_enumeration": descendant_result.iterations.total,
                    "occurrence_product": occurrence_result.iterations.total,
                    "streaming_dp": streaming_result.iterations.total,
                },
                "max_radius": max_radius,
                "nanoseconds_per_build": timings,
                "size": size,
                "speedup_over_descendant": round(
                    timings["descendant_enumeration"] / timings["streaming_dp"],
                    3,
                ),
                "speedup_over_occurrence": round(
                    timings["occurrence_product"] / timings["streaming_dp"],
                    3,
                ),
                "theoretical_work": {
                    "descendant_histories": estimate.descendant_histories,
                    "descendant_occurrences": estimate.descendant_occurrences,
                    "occurrence_factor_cells": estimate.occurrence_factor_cells,
                    "occurrence_products": estimate.occurrence_products,
                    "streaming_state_cells": estimate.streaming_state_cells,
                },
            }
        )
    return rows


def main() -> None:
    result = {
        "algorithm_equivalence_and_scaling": algorithm_census(),
        "coherence": [coherence_census(8, radius) for radius in (1, 2)],
        "environment": {
            "machine": platform.machine(),
            "python": platform.python_version(),
            "system": platform.system(),
        },
        "query_level_history_loss": query_census(8),
    }
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
