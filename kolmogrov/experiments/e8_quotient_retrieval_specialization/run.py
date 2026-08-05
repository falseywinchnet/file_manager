from __future__ import annotations

import json
import platform
import statistics
import sys
import time
from itertools import combinations, product
from math import comb
from pathlib import Path
from typing import Iterable

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    build_certificate_masks,
    compile_certificate_schedule,
    query_certificate_keys,
    run_quotient_deformation_ladder,
    streaming_deformation_ladder,
    streaming_deformation_ladder_radius2,
)


ALPHABET = "ab"
FIELD_MODULUS = 101


def deletion_descendants(source: str, radius: int) -> set[str]:
    return {
        "".join(
            symbol
            for index, symbol in enumerate(source)
            if index not in history
        )
        for history in map(set, combinations(range(len(source)), radius))
    }


def history_support_masks(source: str, radius: int) -> dict[tuple[str, int], int]:
    masks: dict[tuple[str, int], int] = {}
    for history_index, history_tuple in enumerate(
        combinations(range(len(source)), radius)
    ):
        history = set(history_tuple)
        descendant = "".join(
            symbol
            for index, symbol in enumerate(source)
            if index not in history
        )
        bit = 1 << history_index
        for target_index, symbol in enumerate(descendant):
            key = (symbol, target_index)
            masks[key] = masks.get(key, 0) | bit
    return masks


def intersection(masks: Iterable[int]) -> int:
    iterator = iter(masks)
    try:
        value = next(iterator)
    except StopIteration:
        return 0
    for mask in iterator:
        value &= mask
    return value


def interaction_order_census(source_length: int) -> dict[str, object]:
    rows = []
    least_obstructions = []
    for radius in range(1, 4):
        target_length = source_length - radius
        queries = tuple(
            "".join(symbols)
            for symbols in product(ALPHABET, repeat=target_length)
        )
        counts = {
            order: {
                "accepts": 0,
                "false_accepts": 0,
                "candidate_loads": [0] * len(queries),
            }
            for order in range(1, radius + 2)
        }
        exact_positives = 0
        for source_symbols in product(ALPHABET, repeat=source_length):
            source = "".join(source_symbols)
            masks = history_support_masks(source, radius)
            for query_index, query in enumerate(queries):
                feature_masks = tuple(
                    masks.get((symbol, index), 0)
                    for index, symbol in enumerate(query)
                )
                exact = bool(intersection(feature_masks))
                exact_positives += exact
                for order, count in counts.items():
                    accepted = all(
                        intersection(feature_masks[index] for index in subset)
                        for subset in combinations(range(target_length), order)
                    )
                    count["accepts"] += accepted
                    count["false_accepts"] += accepted and not exact
                    count["candidate_loads"][query_index] += accepted

        for order, count in counts.items():
            loads = sorted(count.pop("candidate_loads"))
            rows.append(
                {
                    "radius": radius,
                    "interaction_order": order,
                    "exact_positives": exact_positives,
                    "accepts": count["accepts"],
                    "false_accepts": count["false_accepts"],
                    "recall": exact_positives / exact_positives,
                    "precision": exact_positives / count["accepts"],
                    "mean_candidates_per_query": count["accepts"] / len(queries),
                    "p95_candidates_per_query": loads[
                        max(0, (95 * len(loads) + 99) // 100 - 1)
                    ],
                    "maximum_candidates_per_query": loads[-1],
                }
            )

        witness_source = "ab" * radius + "a"
        witness_query = "b" * (radius + 1)
        witness_masks = history_support_masks(witness_source, radius)
        feature_masks = tuple(
            witness_masks.get((symbol, index), 0)
            for index, symbol in enumerate(witness_query)
        )
        lower_order_accepts = all(
            intersection(feature_masks[index] for index in subset)
            for subset in combinations(range(radius + 1), radius)
        )
        exact = bool(intersection(feature_masks))
        assert lower_order_accepts and not exact
        least_obstructions.append(
            {
                "radius": radius,
                "source": witness_source,
                "query": witness_query,
                "accepted_at_order": radius,
                "rejected_at_order": radius + 1,
            }
        )
    return {
        "source_length": source_length,
        "least_obstructions": least_obstructions,
        "rows": rows,
    }


def constant_runs(source: str) -> tuple[tuple[str, int], ...]:
    runs: list[tuple[str, int]] = []
    for symbol in source:
        if runs and runs[-1][0] == symbol:
            runs[-1] = (symbol, runs[-1][1] + 1)
        else:
            runs.append((symbol, 1))
    return tuple(runs)


def run_count_signature(source: str, history: tuple[int, ...]) -> tuple[int, ...]:
    run_ids: list[int] = []
    run_index = -1
    prior_symbol = None
    for symbol in source:
        if symbol != prior_symbol:
            run_index += 1
            prior_symbol = symbol
        run_ids.append(run_index)
    counts = [0] * (run_index + 1)
    for position in history:
        counts[run_ids[position]] += 1
    return tuple(counts)


def add_to_basis(basis: list[tuple[int, list[int]]], vector: list[int]) -> None:
    value = [entry % FIELD_MODULUS for entry in vector]
    for pivot, row in basis:
        if value[pivot]:
            multiplier = value[pivot]
            value = [
                (left - multiplier * right) % FIELD_MODULUS
                for left, right in zip(value, row, strict=True)
            ]
    if not any(value):
        return
    pivot = next(index for index, entry in enumerate(value) if entry)
    inverse = pow(value[pivot], -1, FIELD_MODULUS)
    value = [entry * inverse % FIELD_MODULUS for entry in value]
    for row_index, (other_pivot, row) in enumerate(basis):
        if row[pivot]:
            multiplier = row[pivot]
            basis[row_index] = (
                other_pivot,
                [
                    (left - multiplier * right) % FIELD_MODULUS
                    for left, right in zip(row, value, strict=True)
                ],
            )
    basis.append((pivot, value))
    basis.sort()


def belongs_to_span(basis: list[tuple[int, list[int]]], vector: list[int]) -> bool:
    value = [entry % FIELD_MODULUS for entry in vector]
    for pivot, row in basis:
        if value[pivot]:
            multiplier = value[pivot]
            value = [
                (left - multiplier * right) % FIELD_MODULUS
                for left, right in zip(value, row, strict=True)
            ]
    return not any(value)


def multiplicative_outcome_quotient(
    source: str,
    radius: int,
) -> tuple[int, int, bool, dict[str, object] | None]:
    histories = tuple(combinations(range(len(source)), radius))
    vectors = []
    outcomes: dict[str, list[tuple[int, ...]]] = {}
    for history in histories:
        vector = [0] * len(source)
        for position in history:
            vector[position] = 1
        vectors.append(vector)
        outcome = "".join(
            symbol
            for index, symbol in enumerate(source)
            if index not in history
        )
        outcomes.setdefault(outcome, []).append(history)

    basis: list[tuple[int, list[int]]] = []
    for outcome_histories in outcomes.values():
        reference = [0] * len(source)
        for position in outcome_histories[0]:
            reference[position] = 1
        for history in outcome_histories[1:]:
            vector = [0] * len(source)
            for position in history:
                vector[position] = 1
            add_to_basis(
                basis,
                [
                    left - right
                    for left, right in zip(vector, reference, strict=True)
                ],
            )

    quotient_classes: list[tuple[int, set[str]]] = []
    first_unsafe = None
    for history_index, (history, vector) in enumerate(zip(histories, vectors, strict=True)):
        outcome = "".join(
            symbol
            for index, symbol in enumerate(source)
            if index not in history
        )
        for representative_index, represented_outcomes in quotient_classes:
            difference = [
                left - right
                for left, right in zip(
                    vector,
                    vectors[representative_index],
                    strict=True,
                )
            ]
            if belongs_to_span(basis, difference):
                represented_outcomes.add(outcome)
                if len(represented_outcomes) > 1 and first_unsafe is None:
                    first_unsafe = {
                        "left_history": histories[representative_index],
                        "right_history": history,
                        "outcomes": sorted(represented_outcomes),
                    }
                break
        else:
            quotient_classes.append((history_index, {outcome}))
    return (
        len(source) - len(basis),
        len(quotient_classes),
        first_unsafe is not None,
        first_unsafe,
    )


def quotient_atlas(source_length: int) -> dict[str, object]:
    rows = []
    first_unsafe_by_radius = {}
    for radius in range(1, 4):
        measurements = []
        for source_symbols in product(ALPHABET, repeat=source_length):
            source = "".join(source_symbols)
            histories = tuple(combinations(range(source_length), radius))
            outcomes = {
                "".join(
                    symbol
                    for index, symbol in enumerate(source)
                    if index not in history
                )
                for history in histories
            }
            run_classes = {
                run_count_signature(source, history)
                for history in histories
            }
            dimension, code_classes, unsafe, _ = multiplicative_outcome_quotient(
                source,
                radius,
            )
            measurements.append(
                (
                    len(run_classes),
                    len(outcomes),
                    dimension,
                    code_classes,
                    unsafe,
                )
            )
        fields = (
            "run_count_classes",
            "outcome_classes",
            "multiplicative_quotient_dimension",
            "multiplicative_code_classes",
        )
        row: dict[str, object] = {
            "radius": radius,
            "position_histories": comb(source_length, radius),
            "unsafe_source_count": sum(value[4] for value in measurements),
            "source_count": len(measurements),
        }
        for field_index, field in enumerate(fields):
            values = [value[field_index] for value in measurements]
            row[field] = {
                "minimum": min(values),
                "mean": statistics.fmean(values),
                "maximum": max(values),
            }
        rows.append(row)

        for witness_length in range(radius + 1, source_length + 1):
            found = False
            for source_symbols in product(ALPHABET, repeat=witness_length):
                source = "".join(source_symbols)
                _, _, unsafe, witness = multiplicative_outcome_quotient(
                    source,
                    radius,
                )
                if unsafe:
                    first_unsafe_by_radius[str(radius)] = {
                        "source": source,
                        "source_length": witness_length,
                        **(witness or {}),
                    }
                    found = True
                    break
            if found:
                break
    return {
        "field_modulus": FIELD_MODULUS,
        "source_length": source_length,
        "first_unsafe_by_radius": first_unsafe_by_radius,
        "rows": rows,
    }


def build_postings(
    sources: tuple[str, ...],
    schedule,
) -> tuple[list[list[int]], float]:
    pattern_count = len(ALPHABET) ** schedule.degree
    postings = [
        [0] * pattern_count
        for _ in range(schedule.certificate_count)
    ]
    posting_entries = 0
    for source_index, source in enumerate(sources):
        masks = build_certificate_masks(source, schedule, ALPHABET).masks
        source_bit = 1 << source_index
        for certificate_index, mask in enumerate(masks):
            posting_entries += mask.bit_count()
            remaining = mask
            while remaining:
                bit = remaining & -remaining
                pattern_code = bit.bit_length() - 1
                postings[certificate_index][pattern_code] |= source_bit
                remaining ^= bit
    return postings, posting_entries / len(sources)


def truth_postings(
    sources: tuple[str, ...],
    queries: tuple[str, ...],
    radius: int,
) -> tuple[int, ...]:
    query_indices = {query: index for index, query in enumerate(queries)}
    truth = [0] * len(queries)
    for source_index, source in enumerate(sources):
        source_bit = 1 << source_index
        for descendant in deletion_descendants(source, radius):
            truth[query_indices[descendant]] |= source_bit
    return tuple(truth)


def percentile(values: list[int], numerator: int) -> int:
    ordered = sorted(values)
    index = max(0, (numerator * len(ordered) + 99) // 100 - 1)
    return ordered[index]


def measure_schedule(
    queries: tuple[str, ...],
    truth: tuple[int, ...],
    schedule,
    postings: list[list[int]],
    average_posting_entries_per_source: float,
) -> dict[str, object]:
    all_sources = 0
    for posting in postings[0]:
        all_sources |= posting
    total_candidates = 0
    total_true = 0
    total_missed = 0
    total_false = 0
    loads = []
    for query, truth_mask in zip(queries, truth, strict=True):
        candidates = all_sources
        keys = query_certificate_keys(query, schedule, ALPHABET)
        for certificate_index, key in enumerate(keys):
            candidates &= postings[certificate_index][key]
            if not candidates:
                break
        true_count = truth_mask.bit_count()
        candidate_count = candidates.bit_count()
        total_true += true_count
        total_candidates += candidate_count
        total_missed += (truth_mask & ~candidates).bit_count()
        total_false += (candidates & ~truth_mask).bit_count()
        loads.append(candidate_count)
    build = build_certificate_masks("a" * schedule.source_length, schedule, ALPHABET)
    return {
        "family": schedule.family,
        "degree": schedule.degree,
        "certificate_count": schedule.certificate_count,
        "bit_width_per_source": schedule.bit_width(len(ALPHABET)),
        "pattern_states_per_certificate": schedule.pattern_state_count,
        "build_pattern_iterations_per_source": build.pattern_iterations,
        "build_symbol_reads_per_source": build.symbol_reads,
        "query_posting_lookups": schedule.certificate_count,
        "average_posting_entries_per_source": average_posting_entries_per_source,
        "recall": (total_true - total_missed) / total_true,
        "precision": total_true / total_candidates,
        "false_candidates": total_false,
        "mean_candidates_per_query": total_candidates / len(queries),
        "p95_candidates_per_query": percentile(loads, 95),
        "maximum_candidates_per_query": max(loads),
    }


def retrieval_census(source_length: int) -> dict[str, object]:
    sources = tuple(
        "".join(symbols)
        for symbols in product(ALPHABET, repeat=source_length)
    )
    rows = []
    exact_closure = []
    for radius in range(1, 4):
        target_length = source_length - radius
        queries = tuple(
            "".join(symbols)
            for symbols in product(ALPHABET, repeat=target_length)
        )
        truth = truth_postings(sources, queries, radius)
        for degree in range(1, radius + 2):
            full_affine = compile_certificate_schedule(
                source_length,
                radius,
                degree,
                family="affine",
            )
            postings, entries = build_postings(sources, full_affine)
            seen_counts = set()
            for support_budget in (16, 32, 64, 128, 256):
                certificate_count = min(
                    full_affine.certificate_count,
                    max(1, support_budget // len(ALPHABET) ** degree),
                )
                if certificate_count in seen_counts:
                    continue
                seen_counts.add(certificate_count)
                prefix = compile_certificate_schedule(
                    source_length,
                    radius,
                    degree,
                    certificate_count=certificate_count,
                    family="affine",
                )
                prefix_postings = postings[:certificate_count]
                prefix_entries = sum(
                    posting.bit_count()
                    for certificate in prefix_postings
                    for posting in certificate
                ) / len(sources)
                row = measure_schedule(
                    queries,
                    truth,
                    prefix,
                    prefix_postings,
                    prefix_entries,
                )
                row["requested_support_budget"] = support_budget
                rows.append({"radius": radius, **row})
            if full_affine.certificate_count not in seen_counts:
                row = measure_schedule(
                    queries,
                    truth,
                    full_affine,
                    postings,
                    entries,
                )
                row["requested_support_budget"] = "full_affine"
                rows.append({"radius": radius, **row})

        closure_schedule = compile_certificate_schedule(
            source_length,
            radius,
            radius + 1,
            family="complete",
        )
        closure_postings, closure_entries = build_postings(
            sources,
            closure_schedule,
        )
        closure = measure_schedule(
            queries,
            truth,
            closure_schedule,
            closure_postings,
            closure_entries,
        )
        assert closure["recall"] == 1 and closure["false_candidates"] == 0
        exact_closure.append({"radius": radius, **closure})
    return {
        "alphabet": ALPHABET,
        "source_count": len(sources),
        "source_length": source_length,
        "affine_rows": rows,
        "complete_order_t_plus_one": exact_closure,
    }


def timed(callable_object, repetitions: int) -> tuple[float, float, float]:
    samples = []
    for _ in range(5):
        started = time.perf_counter_ns()
        for _ in range(repetitions):
            callable_object()
        samples.append((time.perf_counter_ns() - started) / repetitions)
    return min(samples), statistics.median(samples), max(samples)


def run_vector_count(source: str, max_radius: int) -> int:
    lengths = [length for _, length in constant_runs(source)]
    return sum(
        sum(counts) <= max_radius
        for counts in product(*(range(min(length, max_radius) + 1) for length in lengths))
    )


def run_quotient_reference(
    source: str,
    maximum_degree: int,
    max_radius: int,
    gap_weights: tuple[int, ...],
    run_weights: tuple[int, ...],
    content_weight,
    modulus: int,
) -> tuple[tuple[int, ...], ...]:
    runs = constant_runs(source)
    coefficients = [
        [0] * (max_radius + 1)
        for _ in range(maximum_degree + 1)
    ]
    for deletion_counts in product(
        *(range(min(length, max_radius) + 1) for _, length in runs)
    ):
        radius = sum(deletion_counts)
        if radius > max_radius:
            continue
        descendant = tuple(
            symbol
            for (symbol, length), deleted in zip(runs, deletion_counts, strict=True)
            for _ in range(length - deleted)
        )
        phase = 1
        for run_weight, deleted in zip(run_weights, deletion_counts, strict=True):
            phase = phase * pow(run_weight, deleted, modulus) % modulus
        local_degree = min(maximum_degree, len(descendant))
        base = streaming_deformation_ladder(
            descendant,
            local_degree,
            0,
            gap_weights[: local_degree + 1],
            (1,) * len(descendant),
            content_weight,
            modulus,
        ).coefficients_by_degree
        for degree in range(local_degree + 1):
            coefficients[degree][radius] = (
                coefficients[degree][radius]
                + phase * base[degree][0]
            ) % modulus
    return tuple(tuple(row) for row in coefficients)


def specialization_census() -> dict[str, object]:
    modulus = 1_000_003
    maximum_degree = 3
    gap_weights = (7, 11, 13, 17)
    content_weight = lambda slot, symbol: 19 + 3 * slot + 5 * (symbol == "b")
    radius_two_rows = []
    for size in (20, 100, 1000):
        symbols = tuple(ALPHABET[index % 2] for index in range(size))
        position_weights = tuple(29 + 2 * index for index in range(size))
        generic = lambda: streaming_deformation_ladder(
            symbols,
            maximum_degree,
            2,
            gap_weights,
            position_weights,
            content_weight,
            modulus,
        )
        specialized = lambda: streaming_deformation_ladder_radius2(
            symbols,
            maximum_degree,
            gap_weights,
            position_weights,
            content_weight,
            modulus,
        )
        generic_value = generic()
        specialized_value = specialized()
        assert generic_value.coefficients_by_degree == specialized_value.coefficients_by_degree
        repetitions = max(20, 20_000 // size)
        generic_time = timed(generic, repetitions)
        specialized_time = timed(specialized, repetitions)
        radius_two_rows.append(
            {
                "size": size,
                "generic_state_cells": generic_value.iterations.state_cells,
                "specialized_rectangular_cells": specialized_value.iterations.state_cells,
                "generic_nanoseconds": {
                    "minimum": generic_time[0],
                    "median": generic_time[1],
                    "maximum": generic_time[2],
                },
                "specialized_nanoseconds": {
                    "minimum": specialized_time[0],
                    "median": specialized_time[1],
                    "maximum": specialized_time[2],
                },
                "median_speedup": generic_time[1] / specialized_time[1],
            }
        )

    quotient_rows = []
    for label, source in (
        ("constant", "a" * 20),
        ("five_runs", "aaaabbbbaaaabbbbaaaa"),
        ("alternating", "ab" * 10),
    ):
        run_count = len(constant_runs(source))
        run_weights = tuple(31 + 2 * index for index in range(run_count))
        computation = run_quotient_deformation_ladder(
            source,
            maximum_degree,
            2,
            gap_weights,
            run_weights,
            content_weight,
            modulus,
        )
        quotient_rows.append(
            {
                "shape": label,
                "run_count": run_count,
                "position_history_states_through_radius_two": sum(
                    comb(len(source), radius) for radius in range(3)
                ),
                "run_count_states_through_radius_two": run_vector_count(source, 2),
                "block_transfer_cells": computation.iterations.state_cells,
            }
        )
    equivalence_cases = 0
    for size in range(1, 8):
        for symbols in product(ALPHABET, repeat=size):
            source = "".join(symbols)
            degree = min(maximum_degree, size)
            local_gap_weights = gap_weights[: degree + 1]
            run_weights = tuple(
                31 + 2 * index
                for index in range(len(constant_runs(source)))
            )
            optimized = run_quotient_deformation_ladder(
                source,
                degree,
                min(3, size),
                local_gap_weights,
                run_weights,
                content_weight,
                modulus,
            ).coefficients_by_degree
            reference = run_quotient_reference(
                source,
                degree,
                min(3, size),
                local_gap_weights,
                run_weights,
                content_weight,
                modulus,
            )
            assert optimized == reference
            equivalence_cases += 1
    return {
        "radius_two_specialization": radius_two_rows,
        "run_quotient_specialization": quotient_rows,
        "run_quotient_equivalence_cases": equivalence_cases,
    }


def main() -> None:
    result = {
        "environment": {
            "machine": platform.machine(),
            "python": platform.python_version(),
            "system": platform.system(),
        },
        "interaction_order": interaction_order_census(10),
        "history_quotient": quotient_atlas(10),
        "retrieval": retrieval_census(10),
        "specialization": specialization_census(),
    }
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
