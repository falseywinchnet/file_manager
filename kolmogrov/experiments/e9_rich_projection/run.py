from __future__ import annotations

import json
import sys
from itertools import combinations, product
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    BinaryLinearProjectionView,
    ProjectionView,
    audit_pattern_projection,
    certificate_pattern_sets,
    compile_certificate_schedule,
    pattern_bucket,
    query_projected_certificate_keys,
)


ALPHABET = "abcd"
ATOM_CODES = {symbol: index for index, symbol in enumerate(ALPHABET)}
SOURCE_LENGTH = 7
RADIUS = 2
DEGREE = 3


LAYOUTS = {
    "exact_64": (
        ProjectionView("base4-forward-exact", 2, 6, (16, 4, 1)),
    ),
    "forward_16": (
        ProjectionView("base4-forward-prefix", 2, 4, (16, 4, 1)),
    ),
    "balanced_16": (
        ProjectionView("balanced-affine", 2, 4, (1, 3, 5)),
    ),
    "balanced_8": (
        ProjectionView("balanced-affine", 2, 3, (1, 3, 5)),
    ),
    "balanced_32": (
        ProjectionView("balanced-affine", 2, 5, (1, 3, 5)),
    ),
    "balanced_64": (
        ProjectionView("balanced-affine", 2, 6, (1, 3, 5)),
    ),
    "linear_mix_8": (
        BinaryLinearProjectionView(
            "binary-jacobian-mix",
            3,
            DEGREE,
            2,
            (21, 42, 41, 38, 1, 4),
        ),
    ),
    "linear_mix_16": (
        BinaryLinearProjectionView(
            "binary-jacobian-mix",
            4,
            DEGREE,
            2,
            (21, 42, 41, 38, 1, 4),
        ),
    ),
    "linear_mix_32": (
        BinaryLinearProjectionView(
            "binary-jacobian-mix",
            5,
            DEGREE,
            2,
            (21, 42, 41, 38, 1, 4),
        ),
    ),
    "linear_mix_64": (
        BinaryLinearProjectionView(
            "binary-jacobian-mix",
            6,
            DEGREE,
            2,
            (21, 42, 41, 38, 1, 4),
        ),
    ),
    "obstruction_mix_16": (
        BinaryLinearProjectionView(
            "private-witness-selected-binary-mix",
            4,
            DEGREE,
            2,
            (7, 9, 18, 35, 1, 2),
        ),
    ),
    "obstruction_mix_32": (
        BinaryLinearProjectionView(
            "private-witness-selected-binary-mix",
            5,
            DEGREE,
            2,
            (7, 9, 18, 35, 1, 2),
        ),
    ),
    "obstruction_mix_64": (
        BinaryLinearProjectionView(
            "private-witness-selected-binary-mix",
            6,
            DEGREE,
            2,
            (7, 9, 18, 35, 1, 2),
        ),
    ),
    "dual_8_8": (
        ProjectionView("balanced-left", 2, 3, (1, 3, 5)),
        ProjectionView("balanced-right", 2, 3, (5, 3, 1)),
    ),
    "cyclic_4x4": (
        ProjectionView("cyclic-0", 2, 2, (1, 2, 3)),
        ProjectionView("cyclic-1", 2, 2, (3, 1, 2)),
        ProjectionView("cyclic-2", 2, 2, (2, 3, 1)),
        ProjectionView("sum-guard", 2, 2, (1, 1, 1)),
    ),
}


def allocate_indexes(schedule, source_count: int):
    byte_count = (source_count + 7) // 8
    return {
        name: tuple(
            tuple(
                [bytearray(byte_count) for _ in range(view.bucket_count)]
                for view in views
            )
            for _ in schedule.subsets
        )
        for name, views in LAYOUTS.items()
    }


def set_source(bitmap: bytearray, source_index: int) -> None:
    bitmap[source_index // 8] |= 1 << (source_index % 8)


def least_pair_collision(views):
    patterns = tuple(product(range(len(ALPHABET)), repeat=DEGREE))
    signatures = {}
    for pattern in patterns:
        signature = tuple(pattern_bucket(pattern, view) for view in views)
        if signature in signatures:
            return {
                "left": signatures[signature],
                "right": pattern,
                "signature": signature,
            }
        signatures[signature] = pattern
    return None


def least_provenance_mix(views):
    if len(views) < 2:
        return None
    patterns = tuple(product(range(len(ALPHABET)), repeat=DEGREE))
    for left, right in combinations(patterns, 2):
        source = (left, right)
        for query in patterns:
            if query in source:
                continue
            if all(
                any(
                    pattern_bucket(query, view) == pattern_bucket(pattern, view)
                    for pattern in source
                )
                for view in views
            ):
                return {"source_patterns": source, "query_pattern": query}
    return None


def main() -> None:
    schedule = compile_certificate_schedule(
        SOURCE_LENGTH,
        RADIUS,
        DEGREE,
        family="complete",
    )
    sources = tuple(product(ALPHABET, repeat=SOURCE_LENGTH))
    queries = tuple(product(ALPHABET, repeat=schedule.target_length))
    indexes = allocate_indexes(schedule, len(sources))
    audits = {
        name: {
            "pattern_incidence": 0,
            "private_witness_incidence": 0,
            "invisible_pattern_incidence": 0,
            "certificate_count": 0,
            "certificates_with_invisible_pattern": 0,
            "minimum_private_witnesses": None,
        }
        for name in LAYOUTS
    }

    for source_index, source in enumerate(sources):
        pattern_sets = certificate_pattern_sets(source, schedule, ATOM_CODES)
        for name, views in LAYOUTS.items():
            audit_row = audits[name]
            for certificate_index, patterns in enumerate(pattern_sets):
                breakdown = audit_pattern_projection(patterns, views)
                audit_row["pattern_incidence"] += breakdown.pattern_count
                audit_row["private_witness_incidence"] += sum(
                    breakdown.private_witness_counts
                )
                audit_row["invisible_pattern_incidence"] += (
                    breakdown.invisible_pattern_count
                )
                audit_row["certificate_count"] += 1
                audit_row["certificates_with_invisible_pattern"] += (
                    breakdown.invisible_pattern_count > 0
                )
                prior_minimum = audit_row["minimum_private_witnesses"]
                audit_row["minimum_private_witnesses"] = (
                    breakdown.minimum_private_witnesses
                    if prior_minimum is None
                    else min(prior_minimum, breakdown.minimum_private_witnesses)
                )
                for view_index, view in enumerate(views):
                    for pattern in patterns:
                        bucket = pattern_bucket(pattern, view)
                        set_source(
                            indexes[name][certificate_index][view_index][bucket],
                            source_index,
                        )

    integer_indexes = {
        name: tuple(
            tuple(
                tuple(int.from_bytes(bitmap, "little") for bitmap in buckets)
                for buckets in certificate
            )
            for certificate in certificates
        )
        for name, certificates in indexes.items()
    }
    all_sources = (1 << len(sources)) - 1
    candidate_totals = {name: 0 for name in LAYOUTS}
    candidate_maxima = {name: 0 for name in LAYOUTS}
    exact_total = 0
    false_negative_totals = {name: 0 for name in LAYOUTS}

    for query in queries:
        query_keys = {
            name: query_projected_certificate_keys(
                query,
                schedule,
                ATOM_CODES,
                views,
            )
            for name, views in LAYOUTS.items()
        }
        candidates = {}
        for name, keys_by_certificate in query_keys.items():
            bits = all_sources
            for certificate_index, keys in enumerate(keys_by_certificate):
                for view_index, key in enumerate(keys):
                    bits &= integer_indexes[name][certificate_index][view_index][key]
                    if not bits:
                        break
                if not bits:
                    break
            candidates[name] = bits
            count = bits.bit_count()
            candidate_totals[name] += count
            candidate_maxima[name] = max(candidate_maxima[name], count)
        truth = candidates["exact_64"]
        exact_total += truth.bit_count()
        for name, bits in candidates.items():
            false_negative_totals[name] += (truth & ~bits).bit_count()

    rows = []
    for name, views in LAYOUTS.items():
        audit = audits[name]
        pattern_incidence = audit["pattern_incidence"]
        candidate_total = candidate_totals[name]
        rows.append(
            {
                "layout": name,
                "views": [
                    ({
                        "name": view.name,
                        "bucket_count": view.bucket_count,
                        "coefficients": view.coefficients,
                    } if isinstance(view, ProjectionView) else {
                        "name": view.name,
                        "bucket_count": view.bucket_count,
                        "atom_bit_width": view.atom_bit_width,
                        "row_masks": view.row_masks,
                    })
                    for view in views
                ],
                "bits_per_certificate": sum(view.bucket_count for view in views),
                "total_payload_bits": len(schedule.subsets)
                * sum(view.bucket_count for view in views),
                "precision_against_exact_certificate_oracle": exact_total
                / candidate_total,
                "mean_candidates_per_query": candidate_total / len(queries),
                "maximum_candidates_per_query": candidate_maxima[name],
                "false_negatives_against_exact_certificate_oracle": (
                    false_negative_totals[name]
                ),
                "invisible_pattern_fraction": audit[
                    "invisible_pattern_incidence"
                ]
                / pattern_incidence,
                "mean_private_witnesses_per_pattern": audit[
                    "private_witness_incidence"
                ]
                / pattern_incidence,
                "certificate_invisibility_fraction": audit[
                    "certificates_with_invisible_pattern"
                ]
                / audit["certificate_count"],
                "minimum_private_witnesses": audit["minimum_private_witnesses"],
                "least_pair_collision": least_pair_collision(views),
                "least_cross_view_provenance_mix": least_provenance_mix(views),
            }
        )

    result = {
        "domain": {
            "alphabet_size": len(ALPHABET),
            "source_length": SOURCE_LENGTH,
            "query_length": schedule.target_length,
            "deletion_radius": RADIUS,
            "certificate_degree": DEGREE,
            "certificate_count": len(schedule.subsets),
            "offset_states_per_certificate": len(schedule.offset_states),
            "source_count": len(sources),
            "query_count": len(queries),
            "exact_truth_mean_candidates": exact_total / len(queries),
        },
        "least_slot_marginal_obstruction": {
            "source_patterns": ((0, 0), (1, 1)),
            "query_pattern": (0, 1),
        },
        "rows": rows,
    }
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
