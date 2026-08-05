"""Observable deletion quotients for indexable retrieval certificates.

The certificate builder works in the Boolean/idempotent domain: histories that
produce the same pattern on one protected target-position subset set the same
bit.  This is intentionally different from the additive deformation jet, which
retains multiplicity and phase.
"""

from __future__ import annotations

from dataclasses import dataclass
from itertools import combinations, combinations_with_replacement
from typing import Sequence


@dataclass(frozen=True, slots=True)
class CertificateSchedule:
    source_length: int
    radius: int
    degree: int
    subsets: tuple[tuple[int, ...], ...]
    offset_states: tuple[tuple[int, ...], ...]
    family: str

    @property
    def target_length(self) -> int:
        return self.source_length - self.radius

    @property
    def certificate_count(self) -> int:
        return len(self.subsets)

    @property
    def pattern_state_count(self) -> int:
        return len(self.offset_states)

    def bit_width(self, alphabet_size: int) -> int:
        if alphabet_size < 1:
            raise ValueError("alphabet size must be positive")
        return self.certificate_count * alphabet_size**self.degree


@dataclass(frozen=True, slots=True)
class CertificateBuild:
    masks: tuple[int, ...]
    pattern_iterations: int
    symbol_reads: int


def _coverage_order(
    candidates: set[tuple[int, ...]],
    target_length: int,
) -> tuple[tuple[int, ...], ...]:
    remaining = list(candidates)
    selected: list[tuple[int, ...]] = []
    coverage = [0] * target_length
    while remaining:
        def priority(subset: tuple[int, ...]) -> tuple[object, ...]:
            boundary_gaps = (
                (subset[0] + 1,)
                + tuple(right - left for left, right in zip(subset, subset[1:]))
                + (target_length - subset[-1],)
            )
            return (
                -sum(coverage[index] for index in subset),
                subset[-1] - subset[0],
                min(boundary_gaps),
                sum(gap * gap for gap in boundary_gaps),
                tuple(-index for index in subset),
            )

        chosen = max(remaining, key=priority)
        remaining.remove(chosen)
        selected.append(chosen)
        for index in chosen:
            coverage[index] += 1
    return tuple(selected)


def _affine_candidates(
    target_length: int,
    degree: int,
) -> set[tuple[int, ...]]:
    if degree == 1:
        return {(index,) for index in range(target_length)}
    return {
        tuple(sorted((start + slot * step) % target_length for slot in range(degree)))
        for step in range(1, target_length)
        for start in range(target_length)
        if len(
            {(start + slot * step) % target_length for slot in range(degree)}
        ) == degree
    }


def compile_certificate_schedule(
    source_length: int,
    radius: int,
    degree: int,
    certificate_count: int | None = None,
    family: str = "affine",
) -> CertificateSchedule:
    """Compile a frozen target-position and monotone-offset schedule.

    ``affine`` uses bounded modular progressions and coverage-first ordering.
    ``complete`` retains every position subset and is the exact semantic oracle,
    not the intended fixed-width configuration.
    """

    if source_length < 1:
        raise ValueError("source length must be positive")
    if not 0 <= radius < source_length:
        raise ValueError("radius must be smaller than source length")
    target_length = source_length - radius
    if not 1 <= degree <= target_length:
        raise ValueError("degree must lie between one and target length")
    if certificate_count is not None and certificate_count < 1:
        raise ValueError("certificate count must be positive")
    if family == "affine":
        candidates = _affine_candidates(target_length, degree)
    elif family == "complete":
        candidates = set(combinations(range(target_length), degree))
    else:
        raise ValueError("family must be 'affine' or 'complete'")
    ordered = _coverage_order(candidates, target_length)
    if certificate_count is not None:
        ordered = ordered[:certificate_count]
    offsets = tuple(combinations_with_replacement(range(radius + 1), degree))
    return CertificateSchedule(
        source_length=source_length,
        radius=radius,
        degree=degree,
        subsets=ordered,
        offset_states=offsets,
        family=family,
    )


def _alphabet_codes(alphabet: Sequence[str]) -> dict[str, int]:
    if not alphabet:
        raise ValueError("alphabet must not be empty")
    codes = {symbol: index for index, symbol in enumerate(alphabet)}
    if len(codes) != len(alphabet):
        raise ValueError("alphabet symbols must be unique")
    return codes


def build_certificate_masks(
    symbols: Sequence[str],
    schedule: CertificateSchedule,
    alphabet: Sequence[str],
) -> CertificateBuild:
    """Build one exact pattern-presence mask per scheduled certificate."""

    if len(symbols) != schedule.source_length:
        raise ValueError("source length does not match the compiled schedule")
    codes = _alphabet_codes(alphabet)
    base = len(codes)
    masks: list[int] = []
    for subset in schedule.subsets:
        mask = 0
        for offsets in schedule.offset_states:
            pattern_code = 0
            for target_index, offset in zip(subset, offsets, strict=True):
                try:
                    symbol_code = codes[symbols[target_index + offset]]
                except KeyError as error:
                    raise ValueError("source contains a symbol outside the alphabet") from error
                pattern_code = pattern_code * base + symbol_code
            mask |= 1 << pattern_code
        masks.append(mask)
    pattern_iterations = len(schedule.subsets) * len(schedule.offset_states)
    return CertificateBuild(
        masks=tuple(masks),
        pattern_iterations=pattern_iterations,
        symbol_reads=pattern_iterations * schedule.degree,
    )


def query_certificate_keys(
    symbols: Sequence[str],
    schedule: CertificateSchedule,
    alphabet: Sequence[str],
) -> tuple[int, ...]:
    """Encode the query pattern selected by each certificate subset."""

    if len(symbols) != schedule.target_length:
        raise ValueError("query length does not match the compiled schedule")
    codes = _alphabet_codes(alphabet)
    base = len(codes)
    keys: list[int] = []
    for subset in schedule.subsets:
        pattern_code = 0
        for target_index in subset:
            try:
                symbol_code = codes[symbols[target_index]]
            except KeyError as error:
                raise ValueError("query contains a symbol outside the alphabet") from error
            pattern_code = pattern_code * base + symbol_code
        keys.append(pattern_code)
    return tuple(keys)


def certificate_accepts(
    masks: Sequence[int],
    query_keys: Sequence[int],
) -> bool:
    """Return whether every query certificate occurs in some descendant."""

    if len(masks) != len(query_keys):
        raise ValueError("mask and query-key counts must agree")
    return all(mask & (1 << key) for mask, key in zip(masks, query_keys, strict=True))
