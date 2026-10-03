"""Summarize one generated JPEG experiment run; no application files are read."""

import csv
import json
import math
import statistics
import sys
from pathlib import Path


def summarize(path: Path) -> dict[str, object]:
    groups: dict[str, list[float]] = {}
    geometry: dict[str, dict[str, int]] = {}
    with path.open("r", encoding="utf-8", newline="") as source:
        reader: csv.DictReader[str] = csv.DictReader(source)
        row: dict[str, str]
        for row in reader:
            fixture: str = row["fixture"]
            orientation: int = int(row["orientation"])
            if fixture not in ("baseline-24mp", "progressive-3mp"):
                raise ValueError("unknown fixture")
            if orientation not in (1, 6):
                raise ValueError("unknown measured orientation")
            key: str = fixture + "/orientation-" + str(orientation)
            if key not in groups:
                groups[key] = []
            samples: list[float] = groups[key]
            repetition: int = int(row["repetition"])
            if repetition != len(samples) or repetition > 30:
                raise ValueError("duplicate, unordered or excess sample")
            milliseconds: float = float(row["decode_ms"])
            if not math.isfinite(milliseconds) or milliseconds <= 0.0:
                raise ValueError("invalid elapsed time")
            width: int = int(row["width"])
            height: int = int(row["height"])
            extent: int = int(row["output_bytes"])
            encoded: int = int(row["encoded_bytes"])
            if not 0 < width <= 1024 or not 0 < height <= 1024:
                raise ValueError("output dimensions exceed declared bound")
            if extent != width * height * 4 or not 0 < encoded <= 16 * 1024 * 1024:
                raise ValueError("buffer extent differs from declared contract")
            current: dict[str, int] = {
                "encoded_bytes": encoded,
                "output_width": width,
                "output_height": height,
                "output_bytes": extent,
            }
            if key in geometry and geometry[key] != current:
                raise ValueError("fixture changed within measurement group")
            geometry[key] = current
            samples.append(milliseconds)
    if len(groups) != 4:
        raise ValueError("missing measurement group")
    result: dict[str, object] = {}
    key: str
    for key in groups:
        samples = groups[key]
        if len(samples) != 31:
            raise ValueError("expected first invocation and 30 warm samples")
        warm: list[float] = samples[1:]
        warm.sort()
        # Nearest-rank p95: ceil(0.95 * 30) - 1 = 28. Median averages 14/15.
        median: float = statistics.median(warm)
        result[key] = {
            "geometry": geometry[key],
            "first_invocation_ms": samples[0],
            "warm_samples": len(warm),
            "warm_median_ms": median,
            "warm_p95_ms": warm[28],
            "warm_max_ms": warm[-1],
        }
    return result


def main() -> int:
    if len(sys.argv) != 2:
        raise ValueError("usage: summarize.py RESULTS.csv")
    path: Path = Path(sys.argv[1])
    result: dict[str, object] = summarize(path)
    rendered: str = json.dumps(result, indent=2, sort_keys=True)
    print(rendered)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
