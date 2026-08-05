"""Bounded exact verification for candidate records.

The certificate index is recall-oriented.  These routines classify the exact
radius-one edit seam without allocating a dynamic-programming matrix.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence

from .filename import observe_filename


@dataclass(frozen=True, slots=True)
class OneEditVerification:
    kind: str | None
    source_positions: tuple[int, ...]
    symbol_comparisons: int

    @property
    def accepted(self) -> bool:
        return self.kind is not None


@dataclass(frozen=True, slots=True)
class TypedFilenameEvidence:
    verification: OneEditVerification
    fold_equivalent: bool
    structural_equivalent: bool
    touches_digit: bool
    touches_extension: bool
    touches_boundary: bool

    @property
    def policy_sensitive(self) -> bool:
        return self.touches_digit or self.touches_extension or self.touches_boundary


def classify_one_edit(
    left: Sequence[str],
    right: Sequence[str],
) -> OneEditVerification:
    """Classify equality or one typed edit in linear time and constant state."""

    left_length = len(left)
    right_length = len(right)
    if abs(left_length - right_length) > 1:
        return OneEditVerification(None, (), 0)

    if left_length == right_length:
        mismatches: list[int] = []
        comparisons = 0
        for index, (left_atom, right_atom) in enumerate(
            zip(left, right, strict=True)
        ):
            comparisons += 1
            if left_atom != right_atom:
                mismatches.append(index)
                if len(mismatches) > 2:
                    return OneEditVerification(None, tuple(mismatches), comparisons)
        if not mismatches:
            return OneEditVerification("exact", (), comparisons)
        if len(mismatches) == 1:
            return OneEditVerification("substitution", tuple(mismatches), comparisons)
        first, second = mismatches
        if (
            second == first + 1
            and left[first] == right[second]
            and left[second] == right[first]
        ):
            return OneEditVerification("adjacent-transposition", (first, second), comparisons)
        return OneEditVerification(None, tuple(mismatches), comparisons)

    if left_length < right_length:
        shorter = left
        longer = right
        kind = "insertion"
    else:
        shorter = right
        longer = left
        kind = "deletion"

    short_index = 0
    long_index = 0
    skipped_position: int | None = None
    comparisons = 0
    while short_index < len(shorter):
        comparisons += 1
        if shorter[short_index] == longer[long_index]:
            short_index += 1
            long_index += 1
            continue
        if skipped_position is not None:
            return OneEditVerification(None, (skipped_position, long_index), comparisons)
        skipped_position = long_index
        long_index += 1
    if skipped_position is None:
        skipped_position = len(longer) - 1
    return OneEditVerification(kind, (skipped_position,), comparisons)


def classify_filename_edit(left: str, right: str) -> TypedFilenameEvidence:
    """Attach exact source-anchored filename evidence to one-edit verification."""

    verification = classify_one_edit(tuple(left), tuple(right))
    if not verification.accepted:
        return TypedFilenameEvidence(verification, False, False, False, False, False)

    left_observation = observe_filename(left)
    right_observation = observe_filename(right)
    fold_view = "filename.anchored-nfkc-casefold"
    structural_view = "filename.structural-class"
    fold_equivalent = (
        left_observation.stream(fold_view).atoms
        == right_observation.stream(fold_view).atoms
    )
    structural_equivalent = (
        left_observation.stream(structural_view).atoms
        == right_observation.stream(structural_view).atoms
    )

    affected_characters: list[str] = []
    affected_positions: list[tuple[str, int]] = []
    for position in verification.source_positions:
        if verification.kind == "deletion" and position < len(left):
            affected_characters.append(left[position])
            affected_positions.append((left, position))
        elif verification.kind == "insertion" and position < len(right):
            affected_characters.append(right[position])
            affected_positions.append((right, position))
        else:
            if position < len(left):
                affected_characters.append(left[position])
                affected_positions.append((left, position))
            if position < len(right):
                affected_characters.append(right[position])
                affected_positions.append((right, position))

    touches_digit = any(character.isdecimal() for character in affected_characters)
    touches_boundary = any(
        character == "." or character in {"_", "-"} or character.isspace()
        for character in affected_characters
    )
    touches_extension = any(
        (dot := source.rfind(".")) >= 0 and position > dot
        for source, position in affected_positions
    )
    return TypedFilenameEvidence(
        verification=verification,
        fold_equivalent=fold_equivalent,
        structural_equivalent=structural_equivalent,
        touches_digit=touches_digit,
        touches_extension=touches_extension,
        touches_boundary=touches_boundary,
    )
