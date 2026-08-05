from __future__ import annotations

import json
import math
import sys
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path
from typing import Iterable

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    BinaryLinearProjectionView,
    build_projected_certificate_masks,
    classify_filename_edit,
    classify_one_edit,
    compile_balanced_binary_rows,
    compile_certificate_schedule,
    compile_protected_binary_rows,
    encode_filename_atom,
    observe_filename,
    projected_certificate_accepts,
    projected_certificate_overlaps,
    query_projected_certificate_keys,
)


RADIUS = 1
DEGREE = 2
MAX_ROWS = 6
CERTIFICATE_COUNTS = (2, 4, 8)


@dataclass(frozen=True, slots=True)
class SplitSpec:
    name: str
    stems: tuple[str, ...]
    qualifiers: tuple[str, ...]
    numbers: tuple[int, ...]
    extensions: tuple[str, ...]
    separator: str
    insertion: str
    substitutions: tuple[tuple[str, str], ...]
    typed_edges: tuple[tuple[str, str], ...]


@dataclass(frozen=True, slots=True)
class Record:
    name: str
    stem: str
    qualifier: str
    number: int
    extension: str


@dataclass(frozen=True, slots=True)
class Query:
    name: str
    source_id: int
    mutation: str
    hard_decoys: tuple[int, ...]


SPLITS = (
    SplitSpec(
        "development",
        ("amber", "birch", "cedar", "drift", "ember", "flint"),
        ("alpha", "index", "local", "trace"),
        (11, 17, 21, 71),
        ("txt", "txx", "tmt"),
        "_",
        "x",
        (("a", "s"), ("b", "v"), ("c", "x"), ("d", "t"), ("e", "p"), ("f", "j")),
        (("1", "7"), ("a", "1"), ("1", "."), ("é", "e"), ("Ａ", "A")),
    ),
    SplitSpec(
        "tuning",
        ("sable", "ivory", "kestrel", "lumen", "raven", "topaz"),
        ("cache", "field", "matrix", "signal"),
        (16, 19, 26, 76),
        ("cfg", "cfx", "ctg"),
        " ",
        "y",
        (("s", "z"), ("i", "u"), ("k", "r"), ("l", "h"), ("r", "d"), ("t", "b")),
        (("2", "9"), ("s", "2"), ("2", "."), ("ö", "o"), ("Ｂ", "B")),
    ),
    SplitSpec(
        "evaluation",
        ("quartz", "nymph", "glyph", "fjord", "façade", "Ωmega"),
        ("orbit", "delta", "prism", "vault"),
        (13, 18, 23, 83),
        ("csv", "tsv", "csg"),
        "-",
        "z",
        (("q", "w"), ("n", "m"), ("g", "h"), ("f", "v"), ("Ω", "Ψ")),
        (("3", "8"), ("q", "3"), ("3", "."), ("ç", "c"), ("Ω", "ω")),
    ),
)


def percentile(values: Iterable[float], fraction: float) -> float:
    ordered = sorted(values)
    if not ordered:
        return 0.0
    return ordered[max(0, math.ceil(fraction * len(ordered)) - 1)]


def distribution(values: Iterable[float]) -> dict[str, float]:
    materialized = tuple(values)
    return {
        "p50": percentile(materialized, 0.50),
        "p95": percentile(materialized, 0.95),
        "p99": percentile(materialized, 0.99),
        "maximum": max(materialized, default=0.0),
        "mean": sum(materialized) / len(materialized) if materialized else 0.0,
    }


def generate_records(spec: SplitSpec) -> tuple[Record, ...]:
    return tuple(
        Record(
            f"{stem}{spec.separator}{qualifier}{spec.separator}{number:02d}.{extension}",
            stem,
            qualifier,
            number,
            extension,
        )
        for stem in spec.stems
        for qualifier in spec.qualifiers
        for number in spec.numbers
        for extension in spec.extensions
    )


def generate_queries(spec: SplitSpec, records: tuple[Record, ...]) -> tuple[Query, ...]:
    replacements = dict(spec.substitutions)
    queries = []
    for source_id in range(0, len(records), 8):
        source = records[source_id]
        decoys = tuple(
            index
            for index, record in enumerate(records)
            if index != source_id
            and record.stem == source.stem
            and record.qualifier == source.qualifier
            and (
                (record.number != source.number and record.extension == source.extension)
                or (record.number == source.number and record.extension != source.extension)
            )
        )
        substitution_index = next(
            index for index, atom in enumerate(source.name) if atom in replacements
        )
        substituted = (
            source.name[:substitution_index]
            + replacements[source.name[substitution_index]]
            + source.name[substitution_index + 1 :]
        )
        transpose_index = next(
            index
            for index in range(len(source.name) - 1)
            if source.name[index].isalpha()
            and source.name[index + 1].isalpha()
            and source.name[index] != source.name[index + 1]
        )
        transposed = (
            source.name[:transpose_index]
            + source.name[transpose_index + 1]
            + source.name[transpose_index]
            + source.name[transpose_index + 2 :]
        )
        mutations = (
            ("exact", source.name),
            ("substitution", substituted),
            ("adjacent-transposition", transposed),
            ("deletion", source.name.replace(spec.separator, "", 1)),
            ("insertion", source.name[:1] + spec.insertion + source.name[1:]),
            ("case", source.name[:1].swapcase() + source.name[1:]),
        )
        queries.extend(Query(name, source_id, mutation, decoys) for mutation, name in mutations)
    return tuple(queries)


def mutation_edges(spec: SplitSpec) -> tuple[tuple[str, str], ...]:
    edges = [(".", spec.separator), *spec.typed_edges, *spec.substitutions]
    for stem in spec.stems:
        edges.append((stem[0], stem[0].swapcase()))
        if len(stem) > 1:
            edges.append((stem[0], stem[1]))
    return tuple(dict.fromkeys(edges))


@lru_cache(maxsize=None)
def observation(name: str):
    return observe_filename(name)


def atom_width(view_id: str) -> int:
    atom = observation("a").stream(view_id).atoms[0]
    return encode_filename_atom(view_id, atom).bit_width


def difference_set(
    spec: SplitSpec,
    view_id: str,
    width: int,
) -> tuple[int, ...]:
    result: set[int] = set()
    for before, after in mutation_edges(spec):
        before_atom = observation(before).stream(view_id).atoms[0]
        after_atom = observation(after).stream(view_id).atoms[0]
        difference = (
            encode_filename_atom(view_id, before_atom).value
            ^ encode_filename_atom(view_id, after_atom).value
        )
        if difference:
            result.update(
                (difference, difference << width, difference | (difference << width))
            )
    return tuple(sorted(result))


def compile_row_families(view_ids: tuple[str, ...]) -> dict[str, dict[str, object]]:
    result = {}
    for view_id in view_ids:
        width = atom_width(view_id)
        groups = {
            spec.name: difference_set(spec, view_id, width) for spec in SPLITS
        }
        guard_union = tuple(
            sorted(set(groups["development"]) | set(groups["tuning"]))
        )
        result[view_id] = {
            "width": width,
            "groups": groups,
            "guard": compile_protected_binary_rows(
                DEGREE, width, MAX_ROWS, guard_union, exact_rank_limit=18
            ),
            "balanced": compile_balanced_binary_rows(DEGREE, width, MAX_ROWS),
        }
    return result


def projection_bundle(
    layout: str,
    view_id: str,
    rows: dict[str, dict[str, object]],
):
    width = int(rows[view_id]["width"])
    guard = rows[view_id]["guard"].row_masks
    balanced = rows[view_id]["balanced"]
    if layout == "coverage-3":
        return (BinaryLinearProjectionView(view_id + ".coverage3", 3, DEGREE, width, balanced),)
    if layout == "coverage-4":
        return (BinaryLinearProjectionView(view_id + ".coverage4", 4, DEGREE, width, balanced),)
    if layout == "union-guard-4":
        return (BinaryLinearProjectionView(view_id + ".guard4", 4, DEGREE, width, guard),)
    if layout == "split-guard3-coverage3":
        return (
            BinaryLinearProjectionView(view_id + ".guard3", 3, DEGREE, width, guard),
            BinaryLinearProjectionView(view_id + ".coverage3", 3, DEGREE, width, balanced),
        )
    if layout == "coupled-guard3-coverage3":
        return (
            BinaryLinearProjectionView(
                view_id + ".coupled6",
                6,
                DEGREE,
                width,
                tuple(guard[:3]) + tuple(balanced[:3]),
            ),
        )
    raise ValueError("unknown layout")


def collect_atom_codes(
    records_by_split: dict[str, tuple[Record, ...]],
    queries_by_split: dict[str, tuple[Query, ...]],
    view_ids: tuple[str, ...],
) -> dict[str, dict[str, int]]:
    names = {
        record.name for records in records_by_split.values() for record in records
    } | {query.name for queries in queries_by_split.values() for query in queries}
    result = {view_id: {} for view_id in view_ids}
    for name in names:
        observed = observation(name)
        for view_id in view_ids:
            for atom in observed.stream(view_id).atoms:
                result[view_id][atom] = encode_filename_atom(view_id, atom).value
    return result


@lru_cache(maxsize=None)
def schedule(source_length: int, certificate_count: int):
    return compile_certificate_schedule(
        source_length,
        RADIUS,
        DEGREE,
        certificate_count=certificate_count,
        family="affine",
    )


class RepresentationCache:
    def __init__(
        self,
        layout: str,
        certificate_count: int,
        view_ids: tuple[str, ...],
        atom_codes: dict[str, dict[str, int]],
        rows: dict[str, dict[str, object]],
    ) -> None:
        self.layout = layout
        self.certificate_count = certificate_count
        self.view_ids = view_ids
        self.atom_codes = atom_codes
        self.rows = rows
        self._masks: dict[str, tuple[tuple[int, ...], ...]] = {}
        self._keys: dict[tuple[str, int], tuple[tuple[int, ...], ...]] = {}

    def masks(self, name: str) -> tuple[tuple[int, ...], ...]:
        if name not in self._masks:
            observed = observation(name)
            source_schedule = schedule(len(name), self.certificate_count)
            combined = [[] for _ in source_schedule.subsets]
            for view_id in self.view_ids:
                build = build_projected_certificate_masks(
                    observed.stream(view_id).atoms,
                    source_schedule,
                    self.atom_codes[view_id],
                    projection_bundle(self.layout, view_id, self.rows),
                )
                for index, view_masks in enumerate(build.masks):
                    combined[index].extend(view_masks)
            self._masks[name] = tuple(tuple(row) for row in combined)
        return self._masks[name]

    def keys(self, name: str, source_length: int) -> tuple[tuple[int, ...], ...]:
        cache_key = (name, source_length)
        if cache_key not in self._keys:
            observed = observation(name)
            source_schedule = schedule(source_length, self.certificate_count)
            combined = [[] for _ in source_schedule.subsets]
            for view_id in self.view_ids:
                keys = query_projected_certificate_keys(
                    observed.stream(view_id).atoms,
                    source_schedule,
                    self.atom_codes[view_id],
                    projection_bundle(self.layout, view_id, self.rows),
                )
                for index, view_keys in enumerate(keys):
                    combined[index].extend(view_keys)
            self._keys[cache_key] = tuple(tuple(row) for row in combined)
        return self._keys[cache_key]

    def accepts(self, query_name: str, record_name: str) -> bool:
        query_length = len(query_name)
        record_length = len(record_name)
        if record_length == query_length:
            return projected_certificate_overlaps(
                self.masks(query_name), self.masks(record_name)
            )
        if record_length == query_length + 1:
            return projected_certificate_accepts(
                self.masks(record_name), self.keys(query_name, record_length)
            )
        if query_length == record_length + 1:
            return projected_certificate_accepts(
                self.masks(query_name), self.keys(record_name, query_length)
            )
        return False


def hybrid_payload(posting: tuple[int, ...], record_count: int) -> int:
    previous = 0
    sparse = 0
    for index, record_id in enumerate(posting):
        delta = record_id if index == 0 else record_id - previous
        previous = record_id
        sparse += max(1, (delta.bit_length() + 6) // 7)
    return 1 + min(sparse, (record_count + 7) // 8)


def posting_stats(
    records: tuple[Record, ...],
    cache: RepresentationCache,
) -> dict[str, object]:
    mutable: dict[tuple[object, ...], list[int]] = {}
    memberships = [0] * len(records)
    for record_id, record in enumerate(records):
        source_masks = cache.masks(record.name)
        for certificate_index, masks in enumerate(source_masks):
            for address_view, mask_value in enumerate(masks):
                mask = mask_value
                while mask:
                    low = mask & -mask
                    cell = low.bit_length() - 1
                    address = ("mask", len(record.name), address_view, certificate_index, cell)
                    mutable.setdefault(address, []).append(record_id)
                    memberships[record_id] += 1
                    mask ^= low
        reverse_keys = cache.keys(record.name, len(record.name) + 1)
        for certificate_index, keys in enumerate(reverse_keys):
            for address_view, cell in enumerate(keys):
                address = ("key", len(record.name) + 1, address_view, certificate_index, cell)
                mutable.setdefault(address, []).append(record_id)
                memberships[record_id] += 1
    postings = {address: tuple(ids) for address, ids in mutable.items()}
    return {
        "posting_memberships": sum(memberships),
        "memberships_per_record": sum(memberships) / len(records),
        "membership_by_record": memberships,
        "address_count": len(postings),
        "hybrid_payload_bytes": sum(
            hybrid_payload(posting, len(records)) for posting in postings.values()
        ),
        "hybrid_payload_bytes_per_record": sum(
            hybrid_payload(posting, len(records)) for posting in postings.values()
        )
        / len(records),
    }


def evaluate(
    records: tuple[Record, ...],
    queries: tuple[Query, ...],
    cache: RepresentationCache,
    live_records: set[int] | None = None,
) -> dict[str, object]:
    live = set(range(len(records))) if live_records is None else live_records
    candidate_counts = []
    false_counts = []
    source_hits = 0
    ambiguity_recall = 0.0
    hard_hits = hard_total = verified_hard_hits = typed_verified_hard_hits = 0
    source_policy_sensitive: dict[str, list[bool]] = {}
    candidate_sets = []
    for query in queries:
        candidates = {
            record_id
            for record_id, record in enumerate(records)
            if record_id in live and cache.accepts(query.name, record.name)
        }
        candidate_sets.append(candidates)
        valid = {
            record_id
            for record_id, record in enumerate(records)
            if record_id in live
            and classify_one_edit(tuple(query.name), tuple(record.name)).accepted
        }
        if query.source_id in live:
            source_hits += query.source_id in candidates
            source_evidence = classify_filename_edit(
                query.name, records[query.source_id].name
            )
            source_policy_sensitive.setdefault(query.mutation, []).append(
                source_evidence.policy_sensitive
            )
        ambiguity_recall += len(candidates & valid) / len(valid) if valid else 1.0
        decoys = set(query.hard_decoys) & live
        admitted_decoys = candidates & decoys
        hard_hits += len(admitted_decoys)
        hard_total += len(decoys)
        for record_id in admitted_decoys:
            evidence = classify_filename_edit(query.name, records[record_id].name)
            if evidence.verification.accepted:
                verified_hard_hits += 1
                typed_verified_hard_hits += evidence.policy_sensitive
        candidate_counts.append(len(candidates))
        false_counts.append(len(candidates - valid))
    live_source_queries = sum(query.source_id in live for query in queries)
    return {
        "source_recall": source_hits / live_source_queries if live_source_queries else 1.0,
        "ambiguity_class_recall": ambiguity_recall / len(queries),
        "candidate_count": distribution(candidate_counts),
        "false_candidate_count": distribution(false_counts),
        "hard_decoy_admission_rate": hard_hits / hard_total if hard_total else 0.0,
        "admitted_hard_decoy_exact_neighbor_rate": (
            verified_hard_hits / hard_hits if hard_hits else 0.0
        ),
        "verified_hard_decoy_typed_evidence_rate": (
            typed_verified_hard_hits / verified_hard_hits
            if verified_hard_hits
            else 0.0
        ),
        "source_policy_sensitive_rate_by_mutation": {
            mutation: sum(values) / len(values)
            for mutation, values in sorted(source_policy_sensitive.items())
        },
        "candidate_sets": candidate_sets,
    }


def row_audit(
    rows: dict[str, dict[str, object]],
    view_ids: tuple[str, ...],
) -> dict[str, object]:
    result = {}
    for view_id in view_ids:
        guard = rows[view_id]["guard"]
        groups = rows[view_id]["groups"]
        result[view_id] = {
            "union_rank": guard.protected_rank,
            "union_semantic_depth": guard.semantic_depth,
            "union_semantic_depth_is_minimum": guard.semantic_depth_is_minimum,
            "difference_counts": {name: len(values) for name, values in groups.items()},
            "invisible_by_prefix": {
                name: [
                    sum(
                        all((difference & row).bit_count() % 2 == 0 for row in guard.row_masks[:depth])
                        for difference in differences
                    )
                    for depth in range(1, MAX_ROWS + 1)
                ]
                for name, differences in groups.items()
            },
        }
    return result


def slim_evaluation(result: dict[str, object]) -> dict[str, object]:
    return {key: value for key, value in result.items() if key != "candidate_sets"}


def main() -> None:
    specs = {spec.name: spec for spec in SPLITS}
    records = {name: generate_records(spec) for name, spec in specs.items()}
    queries = {name: generate_queries(spec, records[name]) for name, spec in specs.items()}
    sample = observation(records["development"][0].name)
    view_ids = tuple(stream.view_id for stream in sample.streams)
    rows = compile_row_families(view_ids)
    atom_codes = collect_atom_codes(records, queries, view_ids)

    equal_support_layouts = (
        "coverage-4",
        "union-guard-4",
        "split-guard3-coverage3",
    )
    tuning_rows = []
    for layout in equal_support_layouts:
        cache = RepresentationCache(layout, 8, view_ids, atom_codes, rows)
        quality = evaluate(records["tuning"], queries["tuning"], cache)
        support = posting_stats(records["tuning"], cache)
        tuning_rows.append({"layout": layout, "quality": slim_evaluation(quality), "support": support})
    selected_layout_row = min(
        tuning_rows,
        key=lambda row: (
            row["quality"]["source_recall"] < 1.0,
            row["quality"]["candidate_count"]["p95"],
            row["quality"]["false_candidate_count"]["p95"],
            row["support"]["hybrid_payload_bytes_per_record"],
            row["layout"],
        ),
    )
    selected_layout = selected_layout_row["layout"]

    tuning_certificate_rows = []
    for certificate_count in CERTIFICATE_COUNTS:
        cache = RepresentationCache(
            selected_layout, certificate_count, view_ids, atom_codes, rows
        )
        quality = evaluate(records["tuning"], queries["tuning"], cache)
        support = posting_stats(records["tuning"], cache)
        tuning_certificate_rows.append(
            {
                "certificate_count": certificate_count,
                "quality": slim_evaluation(quality),
                "support": support,
            }
        )
    selected_certificate_row = next(
        (
            row
            for row in tuning_certificate_rows
            if row["quality"]["source_recall"] == 1.0
            and row["quality"]["candidate_count"]["p95"] <= 8
        ),
        tuning_certificate_rows[-1],
    )
    selected_certificate_count = selected_certificate_row["certificate_count"]

    evaluation_rows = []
    evaluation_full: dict[str, dict[str, object]] = {}
    for layout in (
        "coverage-3",
        *equal_support_layouts,
        "coupled-guard3-coverage3",
    ):
        cache = RepresentationCache(
            layout, selected_certificate_count, view_ids, atom_codes, rows
        )
        quality = evaluate(records["evaluation"], queries["evaluation"], cache)
        evaluation_full[layout] = quality
        support = posting_stats(records["evaluation"], cache)
        cells_per_semantic_view = sum(
            projection.bucket_count
            for projection in projection_bundle(layout, view_ids[0], rows)
        )
        evaluation_rows.append(
            {
                "layout": layout,
                "dense_cells_per_certificate_per_semantic_view": cells_per_semantic_view,
                "quality": slim_evaluation(quality),
                "support": support,
            }
        )

    split_sets = evaluation_full["split-guard3-coverage3"]["candidate_sets"]
    coupled_sets = evaluation_full["coupled-guard3-coverage3"]["candidate_sets"]
    provenance_excess = [
        len(split - coupled) for split, coupled in zip(split_sets, coupled_sets, strict=True)
    ]
    provenance_residual = {
        "split_candidates_not_in_coupled": distribution(provenance_excess),
        "queries_with_split_only_candidates": sum(value > 0 for value in provenance_excess),
        "query_count": len(provenance_excess),
    }

    selected_cache = RepresentationCache(
        selected_layout, selected_certificate_count, view_ids, atom_codes, rows
    )
    selected_full = evaluate(
        records["evaluation"], queries["evaluation"], selected_cache
    )
    selected_support = posting_stats(records["evaluation"], selected_cache)
    deleted = set(range(0, len(records["evaluation"]), 10))
    live = set(range(len(records["evaluation"]))) - deleted
    live_quality = evaluate(
        records["evaluation"], queries["evaluation"], selected_cache, live
    )
    dead_memberships = sum(
        selected_support["membership_by_record"][record_id] for record_id in deleted
    )
    liveness = {
        "deleted_records": len(deleted),
        "live_bitmap_bytes": (len(records["evaluation"]) + 7) // 8,
        "dead_posting_memberships_before_compaction": dead_memberships,
        "live_quality": slim_evaluation(live_quality),
    }

    for row in tuning_rows:
        row["support"].pop("membership_by_record")
    for row in tuning_certificate_rows:
        row["support"].pop("membership_by_record")
    for row in evaluation_rows:
        row["support"].pop("membership_by_record")
    selected_support.pop("membership_by_record")

    print(
        json.dumps(
            {
                "profile_id": sample.profile_id,
                "unicode_version": sample.unicode_version,
                "splits": {
                    name: {"records": len(records[name]), "queries": len(queries[name])}
                    for name in records
                },
                "row_audit": row_audit(rows, view_ids),
                "tuning_equal_support_layouts": tuning_rows,
                "selected_layout": selected_layout,
                "tuning_certificate_sweep": tuning_certificate_rows,
                "selected_certificate_count": selected_certificate_count,
                "evaluation_layouts": evaluation_rows,
                "separate_address_provenance_residual": provenance_residual,
                "selected_evaluation": slim_evaluation(selected_full),
                "selected_support": selected_support,
                "liveness": liveness,
            },
            indent=2,
            ensure_ascii=False,
        )
    )


if __name__ == "__main__":
    main()
