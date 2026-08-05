from __future__ import annotations

import json
import platform
from fractions import Fraction
from itertools import product
from math import comb


def maximum_exact_length(alphabet_size: int, bits: int) -> int:
    capacity = 1 << bits
    length = 0
    objects = 1
    while objects * alphabet_size <= capacity:
        objects *= alphabet_size
        length += 1
    return length


def maximum_cumulative_length(alphabet_size: int, bits: int) -> int:
    capacity = 1 << bits
    length = 0
    cumulative = 1
    next_layer = alphabet_size
    while cumulative + next_layer <= capacity:
        cumulative += next_layer
        next_layer *= alphabet_size
        length += 1
    return length


def hamming_ball_volume(alphabet_size: int, length: int, radius: int) -> int:
    return sum(
        comb(length, changed) * (alphabet_size - 1) ** changed
        for changed in range(radius + 1)
    )


def fuzzy_cover_lower_bits(alphabet_size: int, length: int, radius: int) -> int:
    object_count = alphabet_size**length
    volume = hamming_ball_volume(alphabet_size, length, radius)
    bits = 0
    while (1 << bits) * volume < object_count:
        bits += 1
    return bits


def fraction_record(value: Fraction) -> dict[str, int]:
    return {"denominator": value.denominator, "numerator": value.numerator}


def check_block_contractions() -> dict[str, int | bool]:
    checked_blocks = 0
    for block_size in (2, 3):
        for block in product(range(-2, 3), repeat=block_size):
            checked_blocks += 1
            fine_l1 = sum(abs(value) for value in block)
            coarse_l1 = abs(sum(block))
            assert coarse_l1 <= fine_l1

            average = Fraction(sum(block), block_size)
            assert abs(average) <= max(abs(value) for value in block)

            fine_energy = sum(Fraction(value * value) for value in block)
            coarse_energy = Fraction(sum(block) ** 2, block_size)
            mean = Fraction(sum(block), block_size)
            detail_energy = sum((Fraction(value) - mean) ** 2 for value in block)
            assert fine_energy == coarse_energy + detail_energy
    return {
        "all_l1_l2_linf_checks_pass": True,
        "blocks_checked": checked_blocks,
    }


def main() -> None:
    bit_widths = (8, 16, 32, 64, 128, 256)
    alphabet_sizes = (2, 26, 256)
    exact_capacity = {
        str(bits): {
            str(alphabet_size): {
                "all_lengths_through": maximum_cumulative_length(
                    alphabet_size, bits
                ),
                "exact_length": maximum_exact_length(alphabet_size, bits),
            }
            for alphabet_size in alphabet_sizes
        }
        for bits in bit_widths
    }

    fuzzy_bounds = {
        str(length): {
            str(radius): {
                "hamming_ball_volume": hamming_ball_volume(2, length, radius),
                "minimum_cover_bits": fuzzy_cover_lower_bits(2, length, radius),
            }
            for radius in (0, 1, 2, 4, 8)
            if radius <= length
        }
        for length in (32, 64, 128)
    }

    binary_influence = {
        str(bits): {
            "one_bit_squared_energy": fraction_record(Fraction(4, bits)),
            "one_bit_fraction": fraction_record(Fraction(1, bits)),
        }
        for bits in (64, 128, 256, 512, 1024, 2048)
    }

    positive_sum = (100, -99)
    negative_sum = (1, -100)
    assert tuple(value > 0 for value in positive_sum) == tuple(
        value > 0 for value in negative_sum
    )
    assert sum(positive_sum) > 0 > sum(negative_sum)

    result = {
        "banach_hilbert_finite_checks": check_block_contractions(),
        "binary_exact_capacity": exact_capacity,
        "binary_fuzzy_cover_bounds": fuzzy_bounds,
        "binary_normalized_influence": binary_influence,
        "environment": {
            "machine": platform.machine(),
            "python": platform.python_version(),
            "system": platform.system(),
        },
        "quantized_scale_counterexample": {
            "fine_signs": ["positive", "negative"],
            "negative_coarse_sum_vector": list(negative_sum),
            "positive_coarse_sum_vector": list(positive_sum),
        },
        "scale_dimensions": {
            str(base): [base**level for level in range(9)] for base in (2, 3)
        },
    }
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
