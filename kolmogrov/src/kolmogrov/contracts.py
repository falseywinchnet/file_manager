"""Representation contracts shared by reference and optimized experiments.

These types deliberately do not choose the feature families or distance. They
make loss, provenance, width, and configuration identity explicit from the
first experiment.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Mapping, Sequence


@dataclass(frozen=True, slots=True)
class ChannelFeature:
    channel: str
    identity: str
    value: float
    anchors: tuple[str, ...] = ()
    resolution: str = ""
    direction: str = "symmetric"

    def __post_init__(self) -> None:
        if not self.channel:
            raise ValueError("channel is required")
        if not self.identity:
            raise ValueError("feature identity is required")
        if self.direction not in {"symmetric", "query_to_record", "record_to_query"}:
            raise ValueError(f"unsupported direction: {self.direction}")


@dataclass(frozen=True, slots=True)
class Breakdown:
    object_id: str
    literal_size: int
    features: tuple[ChannelFeature, ...]
    metadata: Mapping[str, str] = field(default_factory=dict)

    def __post_init__(self) -> None:
        if not self.object_id:
            raise ValueError("object_id is required")
        if self.literal_size < 0:
            raise ValueError("literal_size must be non-negative")

    @property
    def channels(self) -> tuple[str, ...]:
        return tuple(sorted({feature.channel for feature in self.features}))


@dataclass(frozen=True, slots=True)
class HashConfiguration:
    family: str
    version: str
    total_bits: int
    channel_bits: Mapping[str, int]
    seeds: Mapping[str, str]
    prevalence_generation: str = "none"
    normalization: str = "unspecified"
    quantization: str = "unspecified"

    def __post_init__(self) -> None:
        if not self.family or not self.version:
            raise ValueError("hash family and version are required")
        if self.total_bits <= 0:
            raise ValueError("total_bits must be positive")
        if any(bits <= 0 for bits in self.channel_bits.values()):
            raise ValueError("every channel bit allocation must be positive")
        if sum(self.channel_bits.values()) != self.total_bits:
            raise ValueError("channel bit allocations must equal total_bits")
        if set(self.seeds) != set(self.channel_bits):
            raise ValueError("every channel must have exactly one declared seed")

    @property
    def identity(self) -> str:
        channels = ",".join(
            f"{name}:{self.channel_bits[name]}:{self.seeds[name]}"
            for name in sorted(self.channel_bits)
        )
        return "|".join(
            (
                self.family,
                self.version,
                str(self.total_bits),
                channels,
                self.prevalence_generation,
                self.normalization,
                self.quantization,
            )
        )


@dataclass(frozen=True, slots=True)
class FixedHash:
    object_id: str
    configuration_id: str
    addresses: Mapping[str, bytes]

    def validate(self, configuration: HashConfiguration) -> None:
        if self.configuration_id != configuration.identity:
            raise ValueError("hash and configuration identities differ")
        if set(self.addresses) != set(configuration.channel_bits):
            raise ValueError("hash addresses do not match configured channels")
        for channel, bit_count in configuration.channel_bits.items():
            expected_bytes = (bit_count + 7) // 8
            if len(self.addresses[channel]) != expected_bytes:
                raise ValueError(
                    f"address {channel!r} has {len(self.addresses[channel])} bytes; "
                    f"expected {expected_bytes}"
                )


@dataclass(frozen=True, slots=True)
class DistanceContribution:
    channel: str
    distance: float
    weight: float
    evidence: tuple[str, ...] = ()
    inferred: bool = False


@dataclass(frozen=True, slots=True)
class DistanceReport:
    left_object_id: str
    right_object_id: str
    distance: float
    contributions: Sequence[DistanceContribution]
    ambiguity_class: tuple[str, ...] = ()

    def __post_init__(self) -> None:
        if self.distance < 0:
            raise ValueError("distance must be non-negative")
        if not self.left_object_id or not self.right_object_id:
            raise ValueError("both exact object anchors are required")
