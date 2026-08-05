from __future__ import annotations

import json
import platform
import sys
from collections import Counter, defaultdict
from itertools import product
from pathlib import Path
from typing import Callable, Hashable

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import SymbolicCase, projected_gap_signature  # noqa: E402


Signature = Hashable


def content_signature(symbols: tuple[str, ...]) -> Signature:
    return tuple(sorted(Counter(symbols).items()))


def pair_residue_signature(symbols: tuple[str, ...], modulus: int) -> Signature:
    """Direct reference spelling of the degree-2 projected gap address."""

    size = len(symbols)
    counts = Counter()
    for left in range(size):
        for right in range(left + 1, size):
            gaps = (left, right - left - 1, size - right - 1)
            counts[
                (
                    (symbols[left], symbols[right]),
                    tuple(gap % modulus for gap in gaps),
                )
            ] += 1
    return tuple(sorted((pattern, gaps, count) for (pattern, gaps), count in counts.items()))


def pair_order_signature(symbols: tuple[str, ...]) -> Signature:
    counts = Counter()
    for left in range(len(symbols)):
        for right in range(left + 1, len(symbols)):
            counts[(symbols[left], symbols[right])] += 1
    return tuple(sorted(counts.items()))


def first_collision(
    signature: Callable[[tuple[str, ...]], Signature],
    max_length: int,
) -> tuple[tuple[str, ...], tuple[str, ...]] | None:
    for length in range(2, max_length + 1):
        seen: dict[Signature, tuple[str, ...]] = {}
        for symbols in product(("a", "b"), repeat=length):
            value = signature(symbols)
            previous = seen.get(value)
            if previous is not None and previous != symbols:
                return previous, symbols
            seen[value] = symbols
    return None


def collision_statistics(
    signature: Callable[[tuple[str, ...]], Signature],
    max_length: int,
) -> dict[str, object]:
    collision_pairs = 0
    maximum_class_size = 1
    first: tuple[tuple[str, ...], tuple[str, ...]] | None = None
    for length in range(2, max_length + 1):
        classes: defaultdict[Signature, list[tuple[str, ...]]] = defaultdict(list)
        for symbols in product(("a", "b"), repeat=length):
            classes[signature(symbols)].append(symbols)
        for members in classes.values():
            member_count = len(members)
            collision_pairs += member_count * (member_count - 1) // 2
            maximum_class_size = max(maximum_class_size, member_count)
            if member_count > 1 and first is None:
                first = members[0], members[1]
    return {
        "collision_pairs": collision_pairs,
        "first_collision": None
        if first is None
        else [list(first[0]), list(first[1])],
        "maximum_class_size": maximum_class_size,
    }


def main() -> None:
    max_table_length = 12
    signatures: dict[str, Callable[[tuple[str, ...]], Signature]] = {
        "content": content_signature,
        "order_degree_2": pair_order_signature,
        "residue_mod_2": lambda symbols: pair_residue_signature(symbols, 2),
        "residue_mod_3": lambda symbols: pair_residue_signature(symbols, 3),
        "residue_mod_5": lambda symbols: pair_residue_signature(symbols, 5),
        "independent_mod_2_3": lambda symbols: (
            pair_residue_signature(symbols, 2),
            pair_residue_signature(symbols, 3),
        ),
        "joint_mod_6": lambda symbols: pair_residue_signature(symbols, 6),
        "independent_mod_2_3_5": lambda symbols: (
            pair_residue_signature(symbols, 2),
            pair_residue_signature(symbols, 3),
            pair_residue_signature(symbols, 5),
        ),
    }

    # Differentially check the direct experiment spelling against the library
    # construction over the complete table corpus.
    for length in range(2, max_table_length + 1):
        for symbols in product(("a", "b"), repeat=length):
            case = SymbolicCase("differential", symbols)
            for modulus in (2, 3, 5, 6):
                library_value = projected_gap_signature(
                    case,
                    2,
                    lambda gaps, modulus=modulus: tuple(
                        gap % modulus for gap in gaps
                    ),
                )
                assert pair_residue_signature(symbols, modulus) == library_value

    table = {
        name: collision_statistics(signature, max_table_length)
        for name, signature in signatures.items()
    }

    mod_5_collision = first_collision(
        lambda symbols: pair_residue_signature(symbols, 5),
        16,
    )
    assert mod_5_collision is not None
    mod_2_3_collision = first_collision(signatures["independent_mod_2_3"], 12)
    assert mod_2_3_collision is not None

    def distinguished(pair: tuple[tuple[str, ...], tuple[str, ...]], modulus: int) -> bool:
        return pair_residue_signature(pair[0], modulus) != pair_residue_signature(
            pair[1], modulus
        )

    result = {
        "checks": {
            "direct_and_library_signatures_match": True,
            "mod_2_3_collision_rescued_by_joint_mod_6": distinguished(
                mod_2_3_collision, 6
            ),
            "mod_2_3_collision_rescued_by_mod_5": distinguished(
                mod_2_3_collision, 5
            ),
            "mod_5_collision_rescued_by_mod_2": distinguished(mod_5_collision, 2),
            "mod_5_collision_rescued_by_mod_3": distinguished(mod_5_collision, 3),
        },
        "collision_table_through_length_12": table,
        "environment": {
            "machine": platform.machine(),
            "python": platform.python_version(),
            "system": platform.system(),
        },
        "targeted_minimal_collisions": {
            "independent_mod_2_3": [
                list(mod_2_3_collision[0]),
                list(mod_2_3_collision[1]),
            ],
            "residue_mod_5_through_length_16": [
                list(mod_5_collision[0]),
                list(mod_5_collision[1]),
            ],
        },
    }
    print(json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
