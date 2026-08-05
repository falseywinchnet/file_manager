from __future__ import annotations

import json
import math
import sys
import time
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path
from typing import Iterable

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    BinaryLinearProjectionView,
    ProjectionView,
    build_projected_certificate_masks,
    classify_one_edit,
    compile_balanced_binary_rows,
    compile_certificate_schedule,
    compile_protected_binary_rows,
    encode_filename_atom,
    observe_filename,
    query_projected_certificate_keys,
)


DEGREE = 2
RADIUS = 1
MAX_ROWS = 6
CERTIFICATE_LIMIT = 8
SELECTION_GATE = {
    "source_recall": 1.0,
    "p95_candidate_fraction_maximum": 0.10,
    "p95_false_candidates_maximum": 8,
}


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


@dataclass(slots=True)
class PostingIndex:
    postings: dict[tuple[object, ...], tuple[int, ...]]
    payload_bytes: dict[tuple[object, ...], int]
    sparse_bytes: int
    bitmap_bytes: int
    hybrid_bytes: int
    posting_memberships: int
    address_count: int
    build_microseconds: float


DEV_STEMS = ("amber", "birch", "cedar", "drift", "ember", "flint")
DEV_QUALIFIERS = ("alpha", "index", "local", "trace")
DEV_NUMBERS = (11, 27, 45, 73)
DEV_EXTENSIONS = ("txt", "dat", "bin")
DEV_SUBSTITUTIONS = {
    "a": "s",
    "b": "v",
    "c": "x",
    "d": "t",
    "e": "p",
    "f": "j",
}

EVAL_STEMS = ("quartz", "nymph", "glyph", "fjord", "façade", "Ωmega")
EVAL_QUALIFIERS = ("orbit", "delta", "prism", "vault")
EVAL_NUMBERS = (13, 28, 43, 86)
EVAL_EXTENSIONS = ("csv", "png", "log")
EVAL_SUBSTITUTIONS = {
    "q": "w",
    "n": "m",
    "g": "h",
    "f": "v",
    "Ω": "Ψ",
}


def percentile(values: Iterable[float], fraction: float) -> float:
    ordered = sorted(values)
    if not ordered:
        return 0.0
    index = max(0, math.ceil(fraction * len(ordered)) - 1)
    return ordered[index]


def distribution(values: Iterable[float]) -> dict[str, float]:
    materialized = tuple(values)
    return {
        "p50": percentile(materialized, 0.50),
        "p95": percentile(materialized, 0.95),
        "p99": percentile(materialized, 0.99),
        "maximum": max(materialized, default=0.0),
        "mean": sum(materialized) / len(materialized) if materialized else 0.0,
    }


def generate_records(split: str) -> tuple[Record, ...]:
    if split == "development":
        stems = DEV_STEMS
        qualifiers = DEV_QUALIFIERS
        numbers = DEV_NUMBERS
        extensions = DEV_EXTENSIONS
        separator = "_"
    elif split == "evaluation":
        stems = EVAL_STEMS
        qualifiers = EVAL_QUALIFIERS
        numbers = EVAL_NUMBERS
        extensions = EVAL_EXTENSIONS
        separator = "-"
    else:
        raise ValueError("unknown split")
    return tuple(
        Record(
            name=f"{stem}{separator}{qualifier}{separator}{number:02d}.{extension}",
            stem=stem,
            qualifier=qualifier,
            number=number,
            extension=extension,
        )
        for stem in stems
        for qualifier in qualifiers
        for number in numbers
        for extension in extensions
    )


def _first_substitution(name: str, replacements: dict[str, str]) -> str:
    for index, atom in enumerate(name):
        if atom in replacements:
            return name[:index] + replacements[atom] + name[index + 1 :]
    raise ValueError("no declared substitution position")


def _first_transposition(name: str) -> str:
    for index in range(len(name) - 1):
        if name[index].isalpha() and name[index + 1].isalpha() and name[index] != name[index + 1]:
            return name[:index] + name[index + 1] + name[index] + name[index + 2 :]
    raise ValueError("no transposition position")


def generate_queries(split: str, records: tuple[Record, ...]) -> tuple[Query, ...]:
    replacements = DEV_SUBSTITUTIONS if split == "development" else EVAL_SUBSTITUTIONS
    separator = "_" if split == "development" else "-"
    insertion = "x" if split == "development" else "z"
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
        mutations = (
            ("substitution", _first_substitution(source.name, replacements)),
            ("adjacent-transposition", _first_transposition(source.name)),
            ("deletion", source.name.replace(separator, "", 1)),
            ("insertion", source.name[:1] + insertion + source.name[1:]),
            ("case", source.name[:1].swapcase() + source.name[1:]),
        )
        queries.extend(Query(name, source_id, mutation, decoys) for mutation, name in mutations)
    return tuple(queries)


def mutation_edges(split: str) -> tuple[tuple[str, str], ...]:
    if split == "development":
        stems = DEV_STEMS
        replacements = DEV_SUBSTITUTIONS
        boundary = (".", "_")
        typed = (("1", "7"), ("a", "1"), ("1", "."), ("é", "e"), ("Ａ", "A"))
    else:
        stems = EVAL_STEMS
        replacements = EVAL_SUBSTITUTIONS
        boundary = (".", "-")
        typed = (("3", "8"), ("q", "3"), ("3", "."), ("ç", "c"), ("Ω", "ω"))
    edges = [boundary, *typed]
    for stem in stems:
        first = stem[0]
        if first in replacements:
            edges.append((first, replacements[first]))
        edges.append((first, first.swapcase()))
        if len(stem) > 1 and stem[0] != stem[1]:
            edges.append((stem[0], stem[1]))
    return tuple(dict.fromkeys(edges))


def stream_atom(character: str, view_id: str) -> str:
    return observed_filename(character).stream(view_id).atoms[0]


@lru_cache(maxsize=None)
def observed_filename(name: str):
    return observe_filename(name)


def protected_differences(
    edges: tuple[tuple[str, str], ...],
    view_id: str,
    atom_width: int,
) -> tuple[int, ...]:
    differences: set[int] = set()
    for before, after in edges:
        before_atom = stream_atom(before, view_id)
        after_atom = stream_atom(after, view_id)
        difference = (
            encode_filename_atom(view_id, before_atom).value
            ^ encode_filename_atom(view_id, after_atom).value
        )
        if not difference:
            continue
        differences.add(difference)
        differences.add(difference << atom_width)
        differences.add(difference | (difference << atom_width))
    return tuple(sorted(differences))


def atom_width(view_id: str) -> int:
    return encode_filename_atom(view_id, stream_atom("a", view_id)).bit_width


def compile_rows(view_ids: tuple[str, ...]) -> dict[str, dict[str, object]]:
    rows: dict[str, dict[str, object]] = {}
    for view_id in view_ids:
        width = atom_width(view_id)
        development = protected_differences(
            mutation_edges("development"), view_id, width
        )
        evaluation = protected_differences(
            mutation_edges("evaluation"), view_id, width
        )
        compilation = compile_protected_binary_rows(
            DEGREE,
            width,
            MAX_ROWS,
            development,
        )
        rows[view_id] = {
            "width": width,
            "development": development,
            "evaluation": evaluation,
            "protected": compilation,
            "balanced": compile_balanced_binary_rows(DEGREE, width, MAX_ROWS),
        }
    return rows


def projection_for(
    layout: str,
    depth: int,
    view_id: str,
    rows: dict[str, dict[str, object]],
):
    width = int(rows[view_id]["width"])
    if layout == "low-bit-affine":
        return ProjectionView(f"{view_id}.low.{depth}", 2, depth, (1, 3))
    row_key = "protected" if layout == "protected-coset" else "balanced"
    row_value = rows[view_id][row_key]
    row_masks = row_value.row_masks if row_key == "protected" else row_value
    return BinaryLinearProjectionView(
        f"{view_id}.{layout}.{depth}", depth, DEGREE, width, tuple(row_masks)
    )


def collect_atoms(
    records_by_split: dict[str, tuple[Record, ...]],
    queries_by_split: dict[str, tuple[Query, ...]],
    view_ids: tuple[str, ...],
) -> dict[str, dict[str, int]]:
    names = {
        record.name for records in records_by_split.values() for record in records
    } | {query.name for queries in queries_by_split.values() for query in queries}
    atoms: dict[str, set[str]] = {view_id: set() for view_id in view_ids}
    for name in names:
        observation = observed_filename(name)
        for view_id in view_ids:
            atoms[view_id].update(observation.stream(view_id).atoms)
    return {
        view_id: {
            atom: encode_filename_atom(view_id, atom).value
            for atom in sorted(view_atoms)
        }
        for view_id, view_atoms in atoms.items()
    }


@lru_cache(maxsize=None)
def schedule_for(source_length: int):
    return compile_certificate_schedule(
        source_length,
        RADIUS,
        DEGREE,
        certificate_count=CERTIFICATE_LIMIT,
        family="affine",
    )


def hybrid_payload_size(posting: tuple[int, ...], record_count: int) -> tuple[int, int, int]:
    previous = 0
    sparse = 0
    for index, record_id in enumerate(posting):
        delta = record_id if index == 0 else record_id - previous
        previous = record_id
        sparse += max(1, (delta.bit_length() + 6) // 7)
    bitmap = (record_count + 7) // 8
    hybrid = 1 + min(sparse, bitmap)
    return sparse, bitmap, hybrid


def build_index(
    records: tuple[Record, ...],
    view_ids: tuple[str, ...],
    atom_codes: dict[str, dict[str, int]],
    rows: dict[str, dict[str, object]],
    layout: str,
    depth: int,
) -> PostingIndex:
    started = time.perf_counter_ns()
    mutable: dict[tuple[object, ...], list[int]] = {}
    for record_id, record in enumerate(records):
        observation = observed_filename(record.name)
        length = len(record.name)
        source_schedule = schedule_for(length)
        reverse_schedule = schedule_for(length + 1)
        for view_index, view_id in enumerate(view_ids):
            stream = observation.stream(view_id).atoms
            projection = (projection_for(layout, depth, view_id, rows),)
            build = build_projected_certificate_masks(
                stream, source_schedule, atom_codes[view_id], projection
            )
            for certificate_index, certificate_masks in enumerate(build.masks):
                mask = certificate_masks[0]
                while mask:
                    low = mask & -mask
                    cell = low.bit_length() - 1
                    address = ("mask", length, view_index, certificate_index, cell)
                    mutable.setdefault(address, []).append(record_id)
                    mask ^= low
            reverse_keys = query_projected_certificate_keys(
                stream, reverse_schedule, atom_codes[view_id], projection
            )
            for certificate_index, certificate_keys in enumerate(reverse_keys):
                address = (
                    "key",
                    length + 1,
                    view_index,
                    certificate_index,
                    certificate_keys[0],
                )
                mutable.setdefault(address, []).append(record_id)

    postings = {address: tuple(values) for address, values in mutable.items()}
    payload_bytes = {}
    sparse_bytes = bitmap_bytes = hybrid_bytes = 0
    for address, posting in postings.items():
        sparse, bitmap, hybrid = hybrid_payload_size(posting, len(records))
        sparse_bytes += sparse
        bitmap_bytes += bitmap
        hybrid_bytes += hybrid
        payload_bytes[address] = hybrid
    return PostingIndex(
        postings=postings,
        payload_bytes=payload_bytes,
        sparse_bytes=sparse_bytes,
        bitmap_bytes=bitmap_bytes,
        hybrid_bytes=hybrid_bytes,
        posting_memberships=sum(len(posting) for posting in postings.values()),
        address_count=len(postings),
        build_microseconds=(time.perf_counter_ns() - started) / 1000,
    )


def execute_predicates(
    predicates: list[tuple[tuple[object, ...], ...]],
    index: PostingIndex,
) -> tuple[set[int], int, int]:
    ordered = sorted(
        predicates,
        key=lambda addresses: sum(len(index.postings.get(address, ())) for address in addresses),
    )
    candidates: set[int] | None = None
    lists_read = 0
    bytes_read = 0
    for addresses in ordered:
        union: set[int] = set()
        for address in addresses:
            posting = index.postings.get(address)
            if posting is None:
                continue
            lists_read += 1
            bytes_read += index.payload_bytes[address]
            union.update(posting)
        candidates = union if candidates is None else candidates & union
        if not candidates:
            break
    return candidates or set(), lists_read, bytes_read


def query_index(
    query_name: str,
    index: PostingIndex,
    view_ids: tuple[str, ...],
    atom_codes: dict[str, dict[str, int]],
    rows: dict[str, dict[str, object]],
    layout: str,
    depth: int,
) -> tuple[set[int], int, int, int]:
    observation = observed_filename(query_name)
    query_length = len(query_name)
    all_candidates: set[int] = set()
    total_lists = 0
    total_bytes = 0
    logical_predicates = 0

    for mode, source_length in (
        ("same", query_length),
        ("stored-longer", query_length + 1),
        ("query-longer", query_length),
    ):
        if source_length <= RADIUS or (mode == "query-longer" and query_length <= 2):
            continue
        schedule = schedule_for(source_length)
        predicates: list[tuple[tuple[object, ...], ...]] = []
        for view_index, view_id in enumerate(view_ids):
            stream = observation.stream(view_id).atoms
            projection = (projection_for(layout, depth, view_id, rows),)
            if mode == "stored-longer":
                keys = query_projected_certificate_keys(
                    stream, schedule, atom_codes[view_id], projection
                )
                for certificate_index, certificate_keys in enumerate(keys):
                    predicates.append(
                        (("mask", source_length, view_index, certificate_index, certificate_keys[0]),)
                    )
            else:
                build = build_projected_certificate_masks(
                    stream, schedule, atom_codes[view_id], projection
                )
                address_kind = "mask" if mode == "same" else "key"
                for certificate_index, certificate_masks in enumerate(build.masks):
                    mask = certificate_masks[0]
                    addresses = []
                    while mask:
                        low = mask & -mask
                        cell = low.bit_length() - 1
                        addresses.append(
                            (
                                address_kind,
                                source_length,
                                view_index,
                                certificate_index,
                                cell,
                            )
                        )
                        mask ^= low
                    predicates.append(tuple(addresses))
        candidates, lists_read, bytes_read = execute_predicates(predicates, index)
        all_candidates.update(candidates)
        total_lists += lists_read
        total_bytes += bytes_read
        logical_predicates += len(predicates)
    return all_candidates, total_lists, total_bytes, logical_predicates


def evaluate(
    records: tuple[Record, ...],
    queries: tuple[Query, ...],
    view_ids: tuple[str, ...],
    atom_codes: dict[str, dict[str, int]],
    rows: dict[str, dict[str, object]],
    layout: str,
    depth: int,
) -> dict[str, object]:
    index = build_index(records, view_ids, atom_codes, rows, layout, depth)
    candidate_counts = []
    false_counts = []
    verified_counts = []
    verification_comparisons = []
    lists_read = []
    bytes_read = []
    query_microseconds = []
    source_hits = 0
    ambiguity_recall_sum = 0.0
    hard_decoy_hits = hard_decoy_total = 0
    by_mutation: dict[str, list[bool]] = {}

    for query in queries:
        started = time.perf_counter_ns()
        candidates, read_count, read_bytes, _ = query_index(
            query.name, index, view_ids, atom_codes, rows, layout, depth
        )
        query_microseconds.append((time.perf_counter_ns() - started) / 1000)
        source_hit = query.source_id in candidates
        source_hits += source_hit
        by_mutation.setdefault(query.mutation, []).append(source_hit)
        valid = set()
        comparisons = 0
        for record_id, record in enumerate(records):
            verification = classify_one_edit(tuple(query.name), tuple(record.name))
            if verification.accepted:
                valid.add(record_id)
        for record_id in candidates:
            comparisons += classify_one_edit(
                tuple(query.name), tuple(records[record_id].name)
            ).symbol_comparisons
        ambiguity_recall_sum += len(candidates & valid) / len(valid)
        decoys = set(query.hard_decoys)
        hard_decoy_hits += len(candidates & decoys)
        hard_decoy_total += len(decoys)
        candidate_counts.append(len(candidates))
        false_counts.append(len(candidates - valid))
        verified_counts.append(len(candidates & valid))
        verification_comparisons.append(comparisons)
        lists_read.append(read_count)
        bytes_read.append(read_bytes)

    record_count = len(records)
    single_record_segment_payload = 2 * index.posting_memberships / record_count
    return {
        "layout": layout,
        "depth": depth,
        "record_count": record_count,
        "query_count": len(queries),
        "source_recall": source_hits / len(queries),
        "ambiguity_class_recall": ambiguity_recall_sum / len(queries),
        "source_recall_by_mutation": {
            key: sum(values) / len(values) for key, values in sorted(by_mutation.items())
        },
        "candidate_count": distribution(candidate_counts),
        "candidate_fraction": distribution(value / record_count for value in candidate_counts),
        "false_candidate_count": distribution(false_counts),
        "verified_ambiguity_count": distribution(verified_counts),
        "hard_decoy_admission_rate": hard_decoy_hits / hard_decoy_total,
        "query_posting_lists_read": distribution(lists_read),
        "query_posting_payload_bytes": distribution(bytes_read),
        "verification_symbol_comparisons": distribution(verification_comparisons),
        "reference_query_microseconds": distribution(query_microseconds),
        "index": {
            "build_microseconds": index.build_microseconds,
            "posting_memberships": index.posting_memberships,
            "memberships_per_record": index.posting_memberships / record_count,
            "address_count": index.address_count,
            "sparse_delta_payload_bytes": index.sparse_bytes,
            "dense_bitmap_payload_bytes": index.bitmap_bytes,
            "hybrid_payload_bytes": index.hybrid_bytes,
            "hybrid_payload_bytes_per_record": index.hybrid_bytes / record_count,
            "single_record_minisegment_payload_bytes_mean": single_record_segment_payload,
        },
    }


def invisible_difference_counts(
    rows: dict[str, dict[str, object]],
    view_ids: tuple[str, ...],
) -> dict[str, object]:
    result = {}
    for view_id in view_ids:
        row_masks = rows[view_id]["protected"].row_masks
        split_rows = {}
        for split_key in ("development", "evaluation"):
            differences = rows[view_id][split_key]
            split_rows[split_key] = [
                sum(
                    all((difference & row).bit_count() % 2 == 0 for row in row_masks[:depth])
                    for difference in differences
                )
                for depth in range(1, MAX_ROWS + 1)
            ]
        compilation = rows[view_id]["protected"]
        result[view_id] = {
            "development_difference_count": len(rows[view_id]["development"]),
            "evaluation_difference_count": len(rows[view_id]["evaluation"]),
            "rank": compilation.protected_rank,
            "semantic_depth": compilation.semantic_depth,
            "semantic_depth_is_minimum": compilation.semantic_depth_is_minimum,
            "union_bound_depth": compilation.union_bound_depth,
            "invisible_by_prefix": split_rows,
        }
    return result


def main() -> None:
    records_by_split = {
        split: generate_records(split) for split in ("development", "evaluation")
    }
    queries_by_split = {
        split: generate_queries(split, records_by_split[split])
        for split in records_by_split
    }
    sample = observed_filename(records_by_split["development"][0].name)
    view_ids = tuple(stream.view_id for stream in sample.streams)
    rows = compile_rows(view_ids)
    atom_codes = collect_atoms(records_by_split, queries_by_split, view_ids)

    development_curve = [
        evaluate(
            records_by_split["development"],
            queries_by_split["development"],
            view_ids,
            atom_codes,
            rows,
            "protected-coset",
            depth,
        )
        for depth in range(1, MAX_ROWS + 1)
    ]
    selected = next(
        (
            row
            for row in development_curve
            if row["source_recall"] >= SELECTION_GATE["source_recall"]
            and row["candidate_fraction"]["p95"]
            <= SELECTION_GATE["p95_candidate_fraction_maximum"]
            and row["false_candidate_count"]["p95"]
            <= SELECTION_GATE["p95_false_candidates_maximum"]
        ),
        development_curve[-1],
    )
    selected_depth = int(selected["depth"])
    evaluation_controls = [
        evaluate(
            records_by_split["evaluation"],
            queries_by_split["evaluation"],
            view_ids,
            atom_codes,
            rows,
            layout,
            selected_depth,
        )
        for layout in ("low-bit-affine", "balanced", "protected-coset")
    ]
    evaluation_curve = [
        evaluate(
            records_by_split["evaluation"],
            queries_by_split["evaluation"],
            view_ids,
            atom_codes,
            rows,
            "protected-coset",
            depth,
        )
        for depth in range(1, MAX_ROWS + 1)
    ]

    print(
        json.dumps(
            {
                "profile_id": sample.profile_id,
                "unicode_version": sample.unicode_version,
                "configuration": {
                    "radius": RADIUS,
                    "degree": DEGREE,
                    "certificate_limit": CERTIFICATE_LIMIT,
                    "maximum_rows": MAX_ROWS,
                    "queried_source_length_offsets": [-1, 0, 1],
                    "selection_gate": SELECTION_GATE,
                },
                "splits": {
                    split: {
                        "records": len(records_by_split[split]),
                        "queries": len(queries_by_split[split]),
                        "stems": len(DEV_STEMS if split == "development" else EVAL_STEMS),
                    }
                    for split in records_by_split
                },
                "protected_rows": invisible_difference_counts(rows, view_ids),
                "selected_depth": selected_depth,
                "selection_gate_passed": selected is not development_curve[-1]
                or all(
                    (
                        selected["source_recall"] >= SELECTION_GATE["source_recall"],
                        selected["candidate_fraction"]["p95"]
                        <= SELECTION_GATE["p95_candidate_fraction_maximum"],
                        selected["false_candidate_count"]["p95"]
                        <= SELECTION_GATE["p95_false_candidates_maximum"],
                    )
                ),
                "development_protected_curve": development_curve,
                "evaluation_equal_depth_controls": evaluation_controls,
                "evaluation_protected_curve": evaluation_curve,
            },
            indent=2,
            ensure_ascii=False,
        )
    )


if __name__ == "__main__":
    main()
