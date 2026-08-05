from __future__ import annotations

import json
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import (  # noqa: E402
    BinaryLinearProjectionView,
    ProjectionView,
    audit_symbol_mutation,
    build_projected_certificate_masks,
    compile_certificate_schedule,
    compile_balanced_binary_rows,
    encode_filename_atom,
    observe_filename,
)


CASES = (
    ("case-only", "Ab1.txt", 0, "a"),
    ("compatibility-width", "Ａb1.txt", 0, "A"),
    ("letter-identity", "Ab1.txt", 1, "c"),
    ("digit-identity", "Ab1.txt", 2, "2"),
    ("dot-boundary", "Ab1.txt", 3, "_"),
    ("extension-letter", "Ab1.txt", 6, "g"),
    ("accent-removal", "éb1.txt", 0, "e"),
)


def changed_name(source: str, position: int, replacement: str) -> str:
    return source[:position] + replacement + source[position + 1 :]


def main() -> None:
    schedule = compile_certificate_schedule(7, 2, 3, family="complete")
    observations = {}
    for _, source, position, replacement in CASES:
        observations[source] = observe_filename(source)
        target = changed_name(source, position, replacement)
        observations[target] = observe_filename(target)

    view_ids = tuple(stream.view_id for stream in next(iter(observations.values())).streams)
    atom_codes = {}
    projection_layouts = {}
    for view_id in view_ids:
        atoms = {
            atom
            for observation in observations.values()
            for atom in observation.stream(view_id).atoms
        }
        encoded = {atom: encode_filename_atom(view_id, atom) for atom in atoms}
        atom_codes[view_id] = {atom: code.value for atom, code in encoded.items()}
        atom_width = next(iter(encoded.values())).bit_width
        projection_layouts[view_id] = {
            "affine-low16": (
                ProjectionView(view_id + ".affine-low16", 2, 4, (1, 3, 5)),
            ),
            "balanced-bit16": (
                BinaryLinearProjectionView(
                    view_id + ".balanced-bit16",
                    4,
                    3,
                    atom_width,
                    compile_balanced_binary_rows(3, atom_width, 4),
                ),
            ),
        }

    rows = []
    for case_id, source, position, replacement in CASES:
        target = changed_name(source, position, replacement)
        before = observations[source]
        after = observations[target]
        for view_id in view_ids:
            before_stream = before.stream(view_id)
            after_stream = after.stream(view_id)
            for layout_id, projection_views in projection_layouts[view_id].items():
                audit = audit_symbol_mutation(
                    before_stream.atoms,
                    position,
                    after_stream.atoms[position],
                    schedule,
                    atom_codes[view_id],
                    projection_views,
                )
                redundant_certificates = sum(
                    flow.affected_offset_states > 0
                    and not flow.removed_patterns
                    and not flow.added_patterns
                    for flow in audit.certificates
                )
                collision_zero_certificates = sum(
                    bool(flow.removed_patterns or flow.added_patterns)
                    and sum(view.bit_changes for view in flow.view_flows) == 0
                    for flow in audit.certificates
                )
                rows.append(
                    {
                        "case": case_id,
                        "source": source,
                        "target": target,
                        "position": position,
                        "view": view_id,
                        "layout": layout_id,
                        "observation_changed": (
                            before_stream.atoms[position]
                            != after_stream.atoms[position]
                        ),
                        "affected_certificates": audit.affected_certificates,
                        "affected_offset_states": audit.affected_offset_states,
                        "exact_pattern_events": audit.exact_pattern_events,
                        "projected_bit_changes": audit.projected_bit_changes,
                        "exact_redundant_certificates": redundant_certificates,
                        "projection_zero_certificates_with_exact_events": (
                            collision_zero_certificates
                        ),
                    }
                )

    posting_rows = []
    for view_id in view_ids:
        for layout_id, projection_views in projection_layouts[view_id].items():
            entries = []
            pattern_counts = []
            for observation in observations.values():
                stream = observation.stream(view_id)
                build = build_projected_certificate_masks(
                    stream.atoms,
                    schedule,
                    atom_codes[view_id],
                    projection_views,
                )
                entries.append(build.posting_entries)
                pattern_counts.append(sum(build.distinct_pattern_counts))
            posting_rows.append(
                {
                    "view": view_id,
                    "layout": layout_id,
                    "object_count": len(observations),
                    "dense_payload_bits_per_object": build.payload_bits,
                    "query_lookups": len(schedule.subsets),
                    "posting_entries_per_object": {
                        "minimum": min(entries),
                        "mean": sum(entries) / len(entries),
                        "maximum": max(entries),
                    },
                    "exact_distinct_patterns_per_object": {
                        "minimum": min(pattern_counts),
                        "mean": sum(pattern_counts) / len(pattern_counts),
                        "maximum": max(pattern_counts),
                    },
                }
            )

    print(
        json.dumps(
            {
                "profile_id": next(iter(observations.values())).profile_id,
                "unicode_version": next(iter(observations.values())).unicode_version,
                "certificate_count": len(schedule.subsets),
                "offset_states_per_certificate": len(schedule.offset_states),
                "mutation_rows": rows,
                "posting_rows": posting_rows,
            },
            indent=2,
            ensure_ascii=False,
        )
    )


if __name__ == "__main__":
    main()
