"""Exhaustive, audit-first oracle for finite symbolic sequences.

This module intentionally favors explicit evidence over bounded cost.  In the
default configuration it emits every contiguous fragment and every ordered
subsequence, so its output may grow exponentially with the input length.  It
is a correctness oracle for short objects, not a production encoder.
"""

from __future__ import annotations

import json
from dataclasses import dataclass
from itertools import combinations, product
from typing import Callable, Hashable, Iterable, Iterator, Sequence, TypeVar

from .contracts import Breakdown, ChannelFeature


def _canonical_json(value: object) -> str:
    return json.dumps(
        value,
        ensure_ascii=False,
        separators=(",", ":"),
        sort_keys=True,
    )


def _feature_identity(family: str, key: object) -> str:
    return _canonical_json({"family": family, "key": key})


@dataclass(frozen=True, slots=True)
class OracleConfiguration:
    """Identity-bearing limits for an oracle run.

    ``None`` means exhaustive through the object length.  A finite limit is a
    deliberately truncated diagnostic configuration and must not be described
    as the perfect breakdown.
    """

    version: str = "symbolic-oracle-v0"
    max_contiguous_degree: int | None = None
    max_subsequence_degree: int | None = None

    def __post_init__(self) -> None:
        if not self.version:
            raise ValueError("oracle version is required")
        for name, value in (
            ("max_contiguous_degree", self.max_contiguous_degree),
            ("max_subsequence_degree", self.max_subsequence_degree),
        ):
            if value is not None and value < 1:
                raise ValueError(f"{name} must be positive or None")

    @property
    def identity(self) -> str:
        return _canonical_json(
            {
                "max_contiguous_degree": self.max_contiguous_degree,
                "max_subsequence_degree": self.max_subsequence_degree,
                "version": self.version,
            }
        )

    @property
    def exhaustive(self) -> bool:
        return (
            self.max_contiguous_degree is None
            and self.max_subsequence_degree is None
        )


@dataclass(frozen=True, slots=True)
class TransformationStep:
    operation: str
    source_object_id: str
    parameters: tuple[tuple[str, str | int], ...]

    def __post_init__(self) -> None:
        if not self.operation:
            raise ValueError("transformation operation is required")
        if not self.source_object_id:
            raise ValueError("transformation source anchor is required")
        if tuple(sorted(self.parameters)) != self.parameters:
            raise ValueError("transformation parameters must be canonically sorted")

    def canonical(self) -> str:
        return _canonical_json(
            {
                "operation": self.operation,
                "parameters": dict(self.parameters),
                "source_object_id": self.source_object_id,
            }
        )


@dataclass(frozen=True, slots=True)
class SymbolicCase:
    object_id: str
    symbols: tuple[str, ...]
    lineage: tuple[TransformationStep, ...] = ()

    def __post_init__(self) -> None:
        if not self.object_id:
            raise ValueError("exact object anchor is required")
        if any(not isinstance(symbol, str) for symbol in self.symbols):
            raise TypeError("every symbol must be a string")


class _FeatureAccumulator:
    def __init__(self) -> None:
        self._features: dict[
            tuple[str, str, str, str], tuple[float, list[str]]
        ] = {}

    def add(
        self,
        channel: str,
        family: str,
        key: object,
        anchor: str,
        *,
        resolution: str,
        direction: str = "symmetric",
        value: float = 1.0,
    ) -> None:
        identity = _feature_identity(family, key)
        feature_key = (channel, identity, resolution, direction)
        total, anchors = self._features.get(feature_key, (0.0, []))
        anchors.append(anchor)
        self._features[feature_key] = (total + value, anchors)

    def freeze(self) -> tuple[ChannelFeature, ...]:
        result = []
        for (channel, identity, resolution, direction), (
            value,
            anchors,
        ) in self._features.items():
            result.append(
                ChannelFeature(
                    channel=channel,
                    identity=identity,
                    value=value,
                    anchors=tuple(anchors),
                    resolution=resolution,
                    direction=direction,
                )
            )
        return tuple(
            sorted(
                result,
                key=lambda feature: (
                    feature.channel,
                    feature.identity,
                    feature.resolution,
                    feature.direction,
                    feature.anchors,
                ),
            )
        )


def exhaustive_breakdown(
    case: SymbolicCase,
    configuration: OracleConfiguration = OracleConfiguration(),
) -> Breakdown:
    """Return the canonical sparse observable object for ``case``.

    The literal channel is an explicit reconstruction witness.  Other channels
    remain independently inspectable so later experiments can remove, compare,
    weight, or compress them without confusing similarity with identity.
    """

    symbols = case.symbols
    size = len(symbols)
    features = _FeatureAccumulator()

    features.add(
        "literal",
        "length",
        size,
        "object",
        resolution="exact",
    )
    for index, symbol in enumerate(symbols):
        features.add(
            "literal",
            "indexed_symbol",
            [index, symbol],
            f"index:{index}",
            resolution="exact",
        )
        features.add(
            "content",
            "symbol",
            symbol,
            f"index:{index}",
            resolution="degree:1",
        )
        features.add(
            "position",
            "absolute_symbol",
            [symbol, index],
            f"index:{index}",
            resolution="absolute",
        )
        features.add(
            "position",
            "relative_symbol",
            [symbol, index, size],
            f"index:{index}",
            resolution="exact-rational",
        )

    contiguous_limit = min(
        size,
        configuration.max_contiguous_degree
        if configuration.max_contiguous_degree is not None
        else size,
    )
    for degree in range(2, contiguous_limit + 1):
        for start in range(0, size - degree + 1):
            stop = start + degree
            fragment = list(symbols[start:stop])
            anchor = f"span:{start}:{stop}"
            features.add(
                "content",
                "contiguous_fragment",
                fragment,
                anchor,
                resolution=f"degree:{degree}",
            )
            features.add(
                "combination",
                "contiguous_path",
                fragment,
                anchor,
                resolution=f"degree:{degree}",
            )

    subsequence_limit = min(
        size,
        configuration.max_subsequence_degree
        if configuration.max_subsequence_degree is not None
        else size,
    )
    for degree in range(2, subsequence_limit + 1):
        for indices in combinations(range(size), degree):
            ordered_symbols = [symbols[index] for index in indices]
            features.add(
                "combination",
                "ordered_subsequence",
                ordered_symbols,
                "indices:" + ",".join(str(index) for index in indices),
                resolution=f"degree:{degree}",
            )

    features.add(
        "shape",
        "length",
        size,
        "object",
        resolution="exact",
        value=float(size),
    )
    features.add(
        "shape",
        "unique_symbol_count",
        len(set(symbols)),
        "object",
        resolution="exact",
        value=float(len(set(symbols))),
    )

    return Breakdown(
        object_id=case.object_id,
        literal_size=size,
        features=features.freeze(),
        metadata={
            "oracle_configuration": configuration.identity,
            "oracle_scope": "exhaustive" if configuration.exhaustive else "truncated",
            "transformation_lineage": _canonical_json(
                [json.loads(step.canonical()) for step in case.lineage]
            ),
        },
    )


def reconstruct_literal(breakdown: Breakdown) -> tuple[str, ...]:
    """Reconstruct a symbolic sequence from the oracle's literal witness."""

    length_features = []
    indexed_symbols: dict[int, str] = {}
    for feature in breakdown.features:
        if feature.channel != "literal":
            continue
        identity = json.loads(feature.identity)
        family = identity.get("family")
        if family == "length":
            length_features.append(identity["key"])
        elif family == "indexed_symbol":
            index, symbol = identity["key"]
            if index in indexed_symbols:
                raise ValueError(f"duplicate literal symbol index: {index}")
            indexed_symbols[index] = symbol

    if len(length_features) != 1 or not isinstance(length_features[0], int):
        raise ValueError("breakdown must contain exactly one integer literal length")
    expected_length = length_features[0]
    if expected_length != breakdown.literal_size:
        raise ValueError("literal length and breakdown size differ")
    if set(indexed_symbols) != set(range(expected_length)):
        raise ValueError("literal indexed-symbol evidence is incomplete")
    return tuple(indexed_symbols[index] for index in range(expected_length))


def enumerate_symbolic_cases(
    alphabet: Sequence[str],
    max_length: int,
) -> Iterator[SymbolicCase]:
    """Enumerate by length, then caller-declared alphabet order."""

    if max_length < 0:
        raise ValueError("max_length must be non-negative")
    if not alphabet:
        if max_length == 0:
            yield SymbolicCase("sequence:[]", ())
            return
        raise ValueError("a non-empty alphabet is required for positive lengths")
    if any(not isinstance(symbol, str) for symbol in alphabet):
        raise TypeError("every alphabet symbol must be a string")
    if len(set(alphabet)) != len(alphabet):
        raise ValueError("alphabet symbols must be unique")

    for length in range(max_length + 1):
        for symbols in product(alphabet, repeat=length):
            encoded = _canonical_json(list(symbols))
            yield SymbolicCase(f"sequence:{encoded}", symbols)


def _transformed_case(
    source: SymbolicCase,
    target_object_id: str,
    symbols: Iterable[str],
    operation: str,
    parameters: dict[str, str | int],
) -> SymbolicCase:
    step = TransformationStep(
        operation=operation,
        source_object_id=source.object_id,
        parameters=tuple(sorted(parameters.items())),
    )
    return SymbolicCase(target_object_id, tuple(symbols), source.lineage + (step,))


def substitute(
    source: SymbolicCase,
    index: int,
    symbol: str,
    target_object_id: str,
) -> SymbolicCase:
    if not 0 <= index < len(source.symbols):
        raise IndexError("substitution index is outside the source")
    symbols = list(source.symbols)
    replaced = symbols[index]
    symbols[index] = symbol
    return _transformed_case(
        source,
        target_object_id,
        symbols,
        "substitute",
        {"index": index, "replacement": symbol, "replaced": replaced},
    )


def insert(
    source: SymbolicCase,
    index: int,
    symbol: str,
    target_object_id: str,
) -> SymbolicCase:
    if not 0 <= index <= len(source.symbols):
        raise IndexError("insertion index is outside the source boundary")
    symbols = list(source.symbols)
    symbols.insert(index, symbol)
    return _transformed_case(
        source,
        target_object_id,
        symbols,
        "insert",
        {"index": index, "symbol": symbol},
    )


def delete(
    source: SymbolicCase,
    index: int,
    target_object_id: str,
) -> SymbolicCase:
    if not 0 <= index < len(source.symbols):
        raise IndexError("deletion index is outside the source")
    symbols = list(source.symbols)
    removed = symbols.pop(index)
    return _transformed_case(
        source,
        target_object_id,
        symbols,
        "delete",
        {"index": index, "removed": removed},
    )


def transpose_adjacent(
    source: SymbolicCase,
    index: int,
    target_object_id: str,
) -> SymbolicCase:
    if not 0 <= index < len(source.symbols) - 1:
        raise IndexError("transposition requires an adjacent pair")
    symbols = list(source.symbols)
    symbols[index], symbols[index + 1] = symbols[index + 1], symbols[index]
    return _transformed_case(
        source,
        target_object_id,
        symbols,
        "transpose_adjacent",
        {"left_index": index, "right_index": index + 1},
    )


T = TypeVar("T", bound=Hashable)


def find_minimal_counterexample(
    alphabet: Sequence[str],
    max_length: int,
    claim: Callable[[SymbolicCase], bool],
) -> SymbolicCase | None:
    """Return the first failing case in exhaustive enumeration order."""

    for case in enumerate_symbolic_cases(alphabet, max_length):
        if not claim(case):
            return case
    return None


def find_minimal_collision(
    alphabet: Sequence[str],
    max_length: int,
    key: Callable[[SymbolicCase], T],
) -> tuple[SymbolicCase, SymbolicCase] | None:
    """Return the first pair sharing ``key`` in exhaustive enumeration order."""

    seen: dict[T, SymbolicCase] = {}
    for case in enumerate_symbolic_cases(alphabet, max_length):
        candidate_key = key(case)
        previous = seen.get(candidate_key)
        if previous is not None and previous.symbols != case.symbols:
            return previous, case
        seen[candidate_key] = case
    return None
