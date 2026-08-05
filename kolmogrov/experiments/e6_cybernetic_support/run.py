from __future__ import annotations

import json
import platform
from collections import defaultdict
from fractions import Fraction
from itertools import product
from typing import Callable, Hashable, Iterable, Mapping, TypeVar


State = TypeVar("State", bound=Hashable)
Observation = TypeVar("Observation", bound=Hashable)
Kernel = Callable[[State], Mapping[State, Fraction]]


def binary_words(length: int) -> list[str]:
    return ["".join(bits) for bits in product("01", repeat=length)]


def all_binary_words(maximum_length: int) -> list[str]:
    return [
        word
        for length in range(maximum_length + 1)
        for word in binary_words(length)
    ]


def normalized_kernel_signature(
    distribution: Mapping[State, Fraction],
    block_of: Mapping[State, int],
) -> tuple[tuple[int, int, int], ...]:
    mass_by_block: defaultdict[int, Fraction] = defaultdict(Fraction)
    total = Fraction()
    for target, probability in distribution.items():
        assert probability >= 0
        mass_by_block[block_of[target]] += probability
        total += probability
    assert total == 1
    return tuple(
        (block, mass.numerator, mass.denominator)
        for block, mass in sorted(mass_by_block.items())
        if mass
    )


def coarsest_stable_partition(
    states: Iterable[State],
    observation: Callable[[State], Observation],
    actions: Iterable[Kernel[State]],
) -> dict[str, object]:
    ordered_states = list(states)
    action_list = list(actions)

    observation_blocks: dict[Observation, int] = {}
    block_of: dict[State, int] = {}
    for state in ordered_states:
        value = observation(state)
        block_of[state] = observation_blocks.setdefault(
            value, len(observation_blocks)
        )

    history = [len(observation_blocks)]
    while True:
        signature_blocks: dict[tuple[object, ...], int] = {}
        refined: dict[State, int] = {}
        for state in ordered_states:
            signature = (
                block_of[state],
                *(
                    normalized_kernel_signature(action(state), block_of)
                    for action in action_list
                ),
            )
            refined[state] = signature_blocks.setdefault(
                signature, len(signature_blocks)
            )

        if len(signature_blocks) == history[-1]:
            break
        block_of = refined
        history.append(len(signature_blocks))

    block_sizes: defaultdict[int, int] = defaultdict(int)
    for block in block_of.values():
        block_sizes[block] += 1

    return {
        "block_count": len(block_sizes),
        "block_count_history": history,
        "block_sizes": sorted(block_sizes.values()),
        "refinement_steps": len(history) - 1,
    }


def deterministic_flip(position: int) -> Kernel[str]:
    def action(word: str) -> Mapping[str, Fraction]:
        replacement = "1" if word[position] == "0" else "0"
        target = word[:position] + replacement + word[position + 1 :]
        return {target: Fraction(1)}

    return action


def averaged_flip(length: int) -> Kernel[str]:
    def action(word: str) -> Mapping[str, Fraction]:
        distribution: defaultdict[str, Fraction] = defaultdict(Fraction)
        for position in range(length):
            replacement = "1" if word[position] == "0" else "0"
            target = word[:position] + replacement + word[position + 1 :]
            distribution[target] += Fraction(1, length)
        return dict(distribution)

    return action


def indexed_delete(position: int) -> Kernel[str]:
    def action(word: str) -> Mapping[str, Fraction]:
        if position >= len(word):
            return {word: Fraction(1)}
        return {word[:position] + word[position + 1 :]: Fraction(1)}

    return action


def averaged_delete(word: str) -> Mapping[str, Fraction]:
    if not word:
        return {word: Fraction(1)}
    distribution: defaultdict[str, Fraction] = defaultdict(Fraction)
    for position in range(len(word)):
        target = word[:position] + word[position + 1 :]
        distribution[target] += Fraction(1, len(word))
    return dict(distribution)


def fixed_length_census(maximum_length: int) -> list[dict[str, object]]:
    rows = []
    for length in range(1, maximum_length + 1):
        states = binary_words(length)
        controlled = coarsest_stable_partition(
            states,
            lambda word: word.count("1"),
            [deterministic_flip(position) for position in range(length)],
        )
        averaged = coarsest_stable_partition(
            states,
            lambda word: word.count("1"),
            [averaged_flip(length)],
        )
        assert controlled["block_count"] == 2**length
        assert averaged["block_count"] == length + 1
        rows.append(
            {
                "averaged_flip": averaged,
                "controlled_position_flips": controlled,
                "exact_state_count": 2**length,
                "hamming_weight_state_count": length + 1,
                "length": length,
            }
        )
    return rows


def variable_length_census(maximum_lengths: Iterable[int]) -> list[dict[str, object]]:
    rows = []
    for maximum_length in maximum_lengths:
        states = all_binary_words(maximum_length)
        observation = lambda word: (len(word), word.count("1"))
        controlled = coarsest_stable_partition(
            states,
            observation,
            [indexed_delete(position) for position in range(maximum_length)],
        )
        averaged = coarsest_stable_partition(
            states,
            observation,
            [averaged_delete],
        )
        exact_count = 2 ** (maximum_length + 1) - 1
        triangular_count = (maximum_length + 1) * (maximum_length + 2) // 2
        assert controlled["block_count"] == exact_count
        assert averaged["block_count"] == triangular_count
        rows.append(
            {
                "averaged_uniform_deletion": averaged,
                "controlled_position_deletions": controlled,
                "exact_state_count": exact_count,
                "length_weight_state_count": triangular_count,
                "maximum_length": maximum_length,
            }
        )
    return rows


def main() -> None:
    result = {
        "environment": {
            "machine": platform.machine(),
            "python": platform.python_version(),
            "system": platform.system(),
        },
        "fixed_length_binary_flip_census": fixed_length_census(8),
        "variable_length_binary_deletion_census": variable_length_census(
            range(1, 9)
        ),
    }
    print(json.dumps(result, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
