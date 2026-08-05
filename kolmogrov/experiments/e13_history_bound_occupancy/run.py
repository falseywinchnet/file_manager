from __future__ import annotations

import importlib.util
import json
import sys
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    HistoryOccupancyView,
    build_deletion_history_occupancy,
    certificate_pattern_sets,
    classify_one_edit,
    compile_certificate_schedule,
    history_occupancy_accepts,
    history_occupancy_overlaps,
    coupled_history_accepts,
    coupled_history_overlaps,
    encode_filename_atom,
    query_projected_certificate_keys,
    sequence_fingerprint_key,
)


def load_e12():
    path = PROJECT_ROOT / "experiments" / "e12_guard_coverage_specialization" / "run.py"
    spec = importlib.util.spec_from_file_location("kolmogrov_e12", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("could not load E12 workload module")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


E12 = load_e12()
DEGREE = 2
CERTIFICATE_COUNT = 8
HISTORY_CONFIGURATIONS = (
    (1, 128),
    (2, 64),
    (4, 32),
    (8, 16),
    (1, 512),
    (2, 512),
    (1, 2048),
    (2, 2048),
    (1, 8192),
    (2, 8192),
)
COUPLED_CONFIGURATIONS = (
    (1, 256),   # 8 key bits
    (1, 512),   # 9 key bits
    (1, 1024),  # 10 key bits
    (1, 2048),  # 11 key bits
    (2, 64),    # 12 key bits
    (1, 8192),  # 13 key bits
    (2, 128),   # 14 key bits
    (3, 32),    # 15 key bits
    (2, 256),   # 16 key bits
    (4, 16),    # 16 key bits, more coordinate work
    (2, 512),   # 18 key bits
    (3, 64),    # 18 key bits, more coordinate work
)
BASES = (1000003, 1000033, 1000037, 1000039, 1000081, 1000099, 1000117, 1000121)
MIXERS = (
    636413622384679300,
    1442695040888963407,
    393555900037000384,
    2862933555777941757 % ((1 << 61) - 1),
    3202034522624059733 % ((1 << 61) - 1),
    1181783497276652981,
    704602925438635313 % ((1 << 61) - 1),
    1609587929392839161,
)


def exact_target(left: str, right: str) -> bool:
    if len(left) == len(right):
        left_descendants = {left[:index] + left[index + 1 :] for index in range(len(left))}
        return any(
            right[:index] + right[index + 1 :] in left_descendants
            for index in range(len(right))
        )
    if len(left) + 1 == len(right):
        return any(right[:index] + right[index + 1 :] == left for index in range(len(right)))
    if len(right) + 1 == len(left):
        return any(left[:index] + left[index + 1 :] == right for index in range(len(left)))
    return False


@lru_cache(maxsize=None)
def exact_schedule(source_length: int, bounded: bool):
    return compile_certificate_schedule(
        source_length,
        1,
        DEGREE,
        certificate_count=CERTIFICATE_COUNT if bounded else None,
        family="affine" if bounded else "complete",
    )


class ExactCertificateCache:
    def __init__(self, bounded: bool, view_ids, atom_codes) -> None:
        self.bounded = bounded
        self.view_ids = view_ids
        self.atom_codes = atom_codes
        self.patterns: dict[str, tuple[tuple[frozenset[tuple[int, ...]], ...], ...]] = {}
        self.keys: dict[tuple[str, int], tuple[tuple[tuple[int, ...], ...], ...]] = {}

    def pattern_sets(self, name: str):
        if name not in self.patterns:
            observed = E12.observation(name)
            source_schedule = exact_schedule(len(name), self.bounded)
            self.patterns[name] = tuple(
                certificate_pattern_sets(
                    observed.stream(view_id).atoms,
                    source_schedule,
                    self.atom_codes[view_id],
                )
                for view_id in self.view_ids
            )
        return self.patterns[name]

    def target_keys(self, name: str, source_length: int):
        cache_key = (name, source_length)
        if cache_key not in self.keys:
            observed = E12.observation(name)
            source_schedule = exact_schedule(source_length, self.bounded)
            by_view = []
            for view_id in self.view_ids:
                atoms = observed.stream(view_id).atoms
                codes = self.atom_codes[view_id]
                by_view.append(
                    tuple(
                        tuple(codes[atoms[index]] for index in subset)
                        for subset in source_schedule.subsets
                    )
                )
            self.keys[cache_key] = tuple(by_view)
        return self.keys[cache_key]

    def accepts(self, query: str, record: str) -> bool:
        if len(query) == len(record):
            query_sets = self.pattern_sets(query)
            record_sets = self.pattern_sets(record)
            return all(
                left & right
                for left_view, right_view in zip(query_sets, record_sets, strict=True)
                for left, right in zip(left_view, right_view, strict=True)
            )
        if len(record) == len(query) + 1:
            source_sets = self.pattern_sets(record)
            keys = self.target_keys(query, len(record))
            return all(
                key in patterns
                for view_sets, view_keys in zip(source_sets, keys, strict=True)
                for patterns, key in zip(view_sets, view_keys, strict=True)
            )
        if len(query) == len(record) + 1:
            source_sets = self.pattern_sets(query)
            keys = self.target_keys(record, len(query))
            return all(
                key in patterns
                for view_sets, view_keys in zip(source_sets, keys, strict=True)
                for patterns, key in zip(view_sets, view_keys, strict=True)
            )
        return False


class HistoryCache:
    def __init__(self, address_count, bucket_count, view_ids, atom_codes) -> None:
        self.address_count = address_count
        self.bucket_count = bucket_count
        self.view_ids = view_ids
        self.atom_codes = atom_codes
        self.views = {
            view_id: tuple(
                HistoryOccupancyView(
                    f"{view_id}.history.{index}",
                    bucket_count,
                    BASES[index],
                    MIXERS[index],
                )
                for index in range(address_count)
            )
            for view_id in view_ids
        }
        self.cells: dict[str, tuple[frozenset[int], ...]] = {}
        self.keys: dict[str, tuple[int, ...]] = {}

    def deletion_cells(self, name: str) -> tuple[frozenset[int], ...]:
        if name not in self.cells:
            observed = E12.observation(name)
            occupied = []
            for view_id in self.view_ids:
                build = build_deletion_history_occupancy(
                    observed.stream(view_id).atoms,
                    self.atom_codes[view_id],
                    self.views[view_id],
                )
                occupied.extend(build.cells)
            self.cells[name] = tuple(occupied)
        return self.cells[name]

    def full_keys(self, name: str) -> tuple[int, ...]:
        if name not in self.keys:
            observed = E12.observation(name)
            keys = []
            for view_id in self.view_ids:
                keys.extend(
                    sequence_fingerprint_key(
                        observed.stream(view_id).atoms,
                        self.atom_codes[view_id],
                        self.views[view_id],
                    )
                )
            self.keys[name] = tuple(keys)
        return self.keys[name]

    def accepts(self, query: str, record: str) -> bool:
        if len(query) == len(record):
            return history_occupancy_overlaps(
                self.deletion_cells(query), self.deletion_cells(record)
            )
        if len(record) == len(query) + 1:
            return history_occupancy_accepts(
                self.deletion_cells(record), self.full_keys(query)
            )
        if len(query) == len(record) + 1:
            return history_occupancy_accepts(
                self.deletion_cells(query), self.full_keys(record)
            )
        return False


class CoupledHistoryCache(HistoryCache):
    def __init__(self, address_count, bucket_count, view_ids, atom_codes) -> None:
        super().__init__(address_count, bucket_count, view_ids, atom_codes)
        self.coupled: dict[str, tuple[frozenset[tuple[int, ...]], ...]] = {}

    def deletion_keys(self, name: str) -> tuple[frozenset[tuple[int, ...]], ...]:
        if name not in self.coupled:
            observed = E12.observation(name)
            by_semantic_view = []
            for view_id in self.view_ids:
                build = build_deletion_history_occupancy(
                    observed.stream(view_id).atoms,
                    self.atom_codes[view_id],
                    self.views[view_id],
                )
                by_semantic_view.append(frozenset(build.coupled_history_keys))
            self.coupled[name] = tuple(by_semantic_view)
        return self.coupled[name]

    def coupled_full_keys(self, name: str) -> tuple[tuple[int, ...], ...]:
        flat = self.full_keys(name)
        return tuple(
            tuple(flat[start : start + self.address_count])
            for start in range(0, len(flat), self.address_count)
        )

    def accepts(self, query: str, record: str) -> bool:
        if len(query) == len(record):
            return all(
                coupled_history_overlaps(left, right)
                for left, right in zip(
                    self.deletion_keys(query),
                    self.deletion_keys(record),
                    strict=True,
                )
            )
        if len(record) == len(query) + 1:
            return all(
                coupled_history_accepts(keys, full_key)
                for keys, full_key in zip(
                    self.deletion_keys(record),
                    self.coupled_full_keys(query),
                    strict=True,
                )
            )
        if len(query) == len(record) + 1:
            return all(
                coupled_history_accepts(keys, full_key)
                for keys, full_key in zip(
                    self.deletion_keys(query),
                    self.coupled_full_keys(record),
                    strict=True,
                )
            )
        return False


def history_posting_stats(records, cache: HistoryCache):
    mutable = {}
    memberships = 0
    for record_id, record in enumerate(records):
        for view_index, cells in enumerate(cache.deletion_cells(record.name)):
            for cell in cells:
                mutable.setdefault(("mask", len(record.name), view_index, cell), []).append(record_id)
                memberships += 1
        for view_index, cell in enumerate(cache.full_keys(record.name)):
            mutable.setdefault(("key", len(record.name) + 1, view_index, cell), []).append(record_id)
            memberships += 1
    payload = sum(
        E12.hybrid_payload(tuple(posting), len(records)) for posting in mutable.values()
    )
    return {
        "memberships_per_record": memberships / len(records),
        "address_count": len(mutable),
        "hybrid_payload_bytes_per_record": payload / len(records),
    }


def coupled_history_posting_stats(records, cache: CoupledHistoryCache):
    mutable = {}
    memberships = 0
    for record_id, record in enumerate(records):
        for semantic_index, keys in enumerate(cache.deletion_keys(record.name)):
            for key in keys:
                mutable.setdefault(
                    ("history", len(record.name), semantic_index, key), []
                ).append(record_id)
                memberships += 1
        for semantic_index, key in enumerate(cache.coupled_full_keys(record.name)):
            mutable.setdefault(
                ("full", len(record.name) + 1, semantic_index, key), []
            ).append(record_id)
            memberships += 1
    payload = sum(
        E12.hybrid_payload(tuple(posting), len(records)) for posting in mutable.values()
    )
    return {
        "memberships_per_record": memberships / len(records),
        "address_count": len(mutable),
        "hybrid_payload_bytes_per_record": payload / len(records),
    }


def evaluate_history(records, queries, cache: HistoryCache):
    candidate_counts = []
    excess_counts = []
    relation_counts = []
    source_hits = 0
    relation_recall = 0.0
    for query in queries:
        candidates = {
            record_id
            for record_id, record in enumerate(records)
            if cache.accepts(query.name, record.name)
        }
        relation = {
            record_id
            for record_id, record in enumerate(records)
            if exact_target(query.name, record.name)
        }
        source_hits += query.source_id in candidates
        relation_recall += len(candidates & relation) / len(relation)
        candidate_counts.append(len(candidates))
        relation_counts.append(len(relation))
        excess_counts.append(len(candidates - relation))
    return {
        "source_recall": source_hits / len(queries),
        "declared_relation_recall": relation_recall / len(queries),
        "candidate_count": E12.distribution(candidate_counts),
        "exact_relation_count": E12.distribution(relation_counts),
        "hash_excess_count": E12.distribution(excess_counts),
    }


def influence_stats(records, queries, cache: HistoryCache):
    same_length_mutations = {
        "substitution",
        "adjacent-transposition",
        "case",
    }
    rows = []
    by_mutation = {}
    for query in queries:
        if query.mutation not in same_length_mutations:
            continue
        source = records[query.source_id]
        before = cache.deletion_cells(source.name)
        after = cache.deletion_cells(query.name)
        changed_cells = sum(
            len(left.symmetric_difference(right))
            for left, right in zip(before, after, strict=True)
        )
        changed_addresses = sum(
            left != right for left, right in zip(before, after, strict=True)
        )
        row = (changed_cells, changed_addresses)
        rows.append(row)
        by_mutation.setdefault(query.mutation, []).append(row)

    def summarize(values):
        return {
            "changed_cells": E12.distribution(value[0] for value in values),
            "changed_addresses": E12.distribution(value[1] for value in values),
            "invisible_count": sum(value[0] == 0 for value in values),
        }

    return {
        "same_length_nonexact_query_count": len(rows),
        **summarize(rows),
        "by_mutation": {
            mutation: summarize(values)
            for mutation, values in sorted(by_mutation.items())
        },
    }


def candidate_breakdown(records, queries, projected_cache, complete_cache, bounded_cache):
    category_rows = []
    nested_failures = 0
    for query in queries:
        counts = {
            "relation_and_parent_exact": 0,
            "relation_beyond_parent_exact": 0,
            "incompatible_history": 0,
            "schedule_omission": 0,
            "projection_collision": 0,
        }
        for record in records:
            target = exact_target(query.name, record.name)
            parent_exact = classify_one_edit(tuple(query.name), tuple(record.name)).accepted
            complete = complete_cache.accepts(query.name, record.name)
            bounded = bounded_cache.accepts(query.name, record.name)
            projected = projected_cache.accepts(query.name, record.name)
            if not (target <= complete <= bounded <= projected):
                nested_failures += 1
                continue
            if not projected:
                continue
            if target and parent_exact:
                counts["relation_and_parent_exact"] += 1
            elif target:
                counts["relation_beyond_parent_exact"] += 1
            elif complete:
                counts["incompatible_history"] += 1
            elif bounded:
                counts["schedule_omission"] += 1
            else:
                counts["projection_collision"] += 1
        category_rows.append(counts)
    return {
        "nested_failures": nested_failures,
        "per_query": {
            category: E12.distribution(row[category] for row in category_rows)
            for category in category_rows[0]
        },
        "aggregate": {
            category: sum(row[category] for row in category_rows)
            for category in category_rows[0]
        },
    }


def explicit_coupled_obstruction(view_ids, atom_codes):
    left = "ldfioia"
    right = "kbmedfa"
    for name in (left, right):
        observed = E12.observation(name)
        for view_id in view_ids:
            for atom in observed.stream(view_id).atoms:
                atom_codes[view_id].setdefault(
                    atom, encode_filename_atom(view_id, atom).value
                )
    cache = CoupledHistoryCache(2, 512, view_ids, atom_codes)
    shared_by_view = []
    for left_keys, right_keys in zip(
        cache.deletion_keys(left), cache.deletion_keys(right), strict=True
    ):
        shared_by_view.append(sorted(left_keys & right_keys))
    return {
        "alphabet": "abcdefghijklmnop",
        "no_direct_combined_literal_fold_collision_through_descendant_length": 5,
        "colliding_descendants": ["ldfioi", "kbmedf"],
        "lifted_sources": [left, right],
        "declared_relation": exact_target(left, right),
        "coupled_acceptance": cache.accepts(left, right),
        "shared_history_keys_by_semantic_view": shared_by_view,
    }


def main() -> None:
    specs = {spec.name: spec for spec in E12.SPLITS}
    records = {name: E12.generate_records(spec) for name, spec in specs.items()}
    queries = {name: E12.generate_queries(spec, records[name]) for name, spec in specs.items()}
    sample = E12.observation(records["development"][0].name)
    view_ids = tuple(stream.view_id for stream in sample.streams)
    rows = E12.compile_row_families(view_ids)
    atom_codes = E12.collect_atom_codes(records, queries, view_ids)
    obstruction = explicit_coupled_obstruction(view_ids, atom_codes)

    history_results = {}
    coupled_results = {}
    for split in records:
        split_rows = []
        for address_count, bucket_count in HISTORY_CONFIGURATIONS:
            cache = HistoryCache(address_count, bucket_count, view_ids, atom_codes)
            quality = evaluate_history(records[split], queries[split], cache)
            support = history_posting_stats(records[split], cache)
            split_rows.append(
                {
                    "addresses_per_semantic_view": address_count,
                    "cells_per_address": bucket_count,
                    "total_dense_cells": len(view_ids) * address_count * bucket_count,
                    "quality": quality,
                    "support": support,
                    "influence": influence_stats(records[split], queries[split], cache),
                }
            )
        history_results[split] = split_rows

        coupled_rows = []
        for address_count, bucket_count in COUPLED_CONFIGURATIONS:
            cache = CoupledHistoryCache(
                address_count, bucket_count, view_ids, atom_codes
            )
            coupled_rows.append(
                {
                    "tuple_width": address_count,
                    "cells_per_coordinate": bucket_count,
                    "key_bits_per_semantic_view": address_count
                    * (bucket_count - 1).bit_length(),
                    "quality": evaluate_history(
                        records[split], queries[split], cache
                    ),
                    "support": coupled_history_posting_stats(records[split], cache),
                }
            )
        coupled_results[split] = coupled_rows

    projected = E12.RepresentationCache(
        "coverage-4", CERTIFICATE_COUNT, view_ids, atom_codes, rows
    )
    complete = ExactCertificateCache(False, view_ids, atom_codes)
    bounded = ExactCertificateCache(True, view_ids, atom_codes)
    breakdown = candidate_breakdown(
        records["evaluation"], queries["evaluation"], projected, complete, bounded
    )
    certificate_quality = E12.evaluate(
        records["evaluation"], queries["evaluation"], projected
    )
    certificate_support = E12.posting_stats(records["evaluation"], projected)
    certificate_support.pop("membership_by_record")

    print(
        json.dumps(
            {
                "profile_id": sample.profile_id,
                "unicode_version": sample.unicode_version,
                "configuration": {
                    "radius": 1,
                    "certificate_control": {
                        "degree": 2,
                        "certificate_count": CERTIFICATE_COUNT,
                        "cells": 16,
                        "total_dense_cells": len(view_ids) * CERTIFICATE_COUNT * 16,
                    },
                    "history_total_dense_cells": 384,
                },
                "certificate_evaluation": {
                    "quality": {
                        key: value
                        for key, value in certificate_quality.items()
                        if key != "candidate_sets"
                    },
                    "support": certificate_support,
                    "candidate_breakdown": breakdown,
                },
                "history_occupancy": history_results,
                "coupled_history_tuples": coupled_results,
                "explicit_coupled_obstruction": obstruction,
            },
            indent=2,
            ensure_ascii=False,
        )
    )


if __name__ == "__main__":
    main()
