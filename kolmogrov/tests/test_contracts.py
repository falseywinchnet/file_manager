from __future__ import annotations

import sys
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT_ROOT / "src"))

from kolmogrov import Breakdown, ChannelFeature, FixedHash, HashConfiguration


class ContractTests(unittest.TestCase):
    def test_breakdown_retains_independent_channels(self) -> None:
        breakdown = Breakdown(
            object_id="record-1",
            literal_size=4,
            features=(
                ChannelFeature("content", "gram:ab", 1.0, ("0:2",)),
                ChannelFeature("position", "bin:0/2", 0.5, ("0",), "2"),
                ChannelFeature("combination", "path:a>b", 1.0, ("0", "1")),
            ),
        )
        self.assertEqual(breakdown.channels, ("combination", "content", "position"))

    def test_hash_configuration_requires_exact_bit_budget(self) -> None:
        with self.assertRaises(ValueError):
            HashConfiguration(
                family="candidate",
                version="1",
                total_bits=256,
                channel_bits={"content": 128, "position": 64},
                seeds={"content": "a", "position": "b"},
            )

    def test_multi_address_hash_validates_width_and_identity(self) -> None:
        configuration = HashConfiguration(
            family="candidate",
            version="1",
            total_bits=256,
            channel_bits={"content": 128, "position": 64, "combination": 64},
            seeds={"content": "a", "position": "b", "combination": "c"},
            normalization="channel-unit",
            quantization="binary-sign",
        )
        value = FixedHash(
            object_id="record-1",
            configuration_id=configuration.identity,
            addresses={
                "content": bytes(16),
                "position": bytes(8),
                "combination": bytes(8),
            },
        )
        value.validate(configuration)

        invalid = FixedHash(
            object_id="record-1",
            configuration_id=configuration.identity,
            addresses={
                "content": bytes(15),
                "position": bytes(8),
                "combination": bytes(8),
            },
        )
        with self.assertRaises(ValueError):
            invalid.validate(configuration)


if __name__ == "__main__":
    unittest.main()
