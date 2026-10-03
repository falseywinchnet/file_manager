"""Repeat the archived exact-query control; never admit a reader candidate."""

import argparse
from dataclasses import asdict, dataclass
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import statistics
import subprocess
import sys
import time
from typing import Protocol


BASELINE_SHA256: str = "5f945a6d5d2f5b7bff6ce263e4026cc3b77b275543af88785fd0ba1c3ae19b85"
CANDIDATE_SHA256: str = "e0f84ca82754797dce621dc840f3e6db2abbfe7e574b68f01ac405bb73148a2b"
CONTROL_NAMES: tuple[str, ...] = ("Path", "Name", "Pipeline")
AGGREGATE: re.Pattern[str] = re.compile(
    r"aggregate=sampleGeneration(Path|Name|Pipeline) calls=(\d+) elapsed=(\S+) "
    r"ns_per_query=([\d.]+) allocated_bytes=(\d+) mallocs=(\d+)"
)


@dataclass(frozen=True)
class Observation:
    control: str
    calls: int
    nanoseconds_per_query: float
    allocated_bytes: int
    mallocs: int


class Digest(Protocol):
    def hexdigest(self) -> str: ...


def verify_binary(path: Path, expected: str) -> None:
    contents: bytes = path.read_bytes()
    digest: Digest = hashlib.sha256(contents)
    actual: str = digest.hexdigest()
    if actual != expected:
        raise RuntimeError(f"Archived binary hash differs: {path}: {actual}")


def parse_output(output: str) -> dict[str, Observation]:
    result: dict[str, Observation] = {}
    match: re.Match[str]
    for match in AGGREGATE.finditer(output):
        name: str = match.group(1)
        if name in result:
            raise RuntimeError(f"Repeated control output: {name}")
        calls: int = int(match.group(2))
        elapsed: float = float(match.group(4))
        allocated: int = int(match.group(5))
        mallocs: int = int(match.group(6))
        expected_calls: int = 20000 if name == "Path" else 2000
        if calls != expected_calls or elapsed <= 0.0:
            raise RuntimeError(f"Invalid control count or duration: {name}")
        result[name] = Observation(name, calls, elapsed, allocated, mallocs)
    if len(result) != len(CONTROL_NAMES):
        raise RuntimeError("Incomplete aggregate observations")
    if "digest=6c4437450b9c86ff" not in output:
        raise RuntimeError("Exact-path digest differs")
    if output.count("digest=a69b9113a32327fd") != 2:
        raise RuntimeError("Name or pipeline digest differs")
    if "--- PASS: TestImmutableGenerationDistribution" not in output:
        raise RuntimeError("Fixture did not pass")
    return result


def run_fixture(binary: Path, destination: Path, environment: dict[str, str]) -> dict[str, Observation]:
    command: list[str] = [str(binary), "-test.run=^TestImmutableGenerationDistribution$", "-test.v"]
    completed: subprocess.CompletedProcess[str] = subprocess.run(
        command, env=environment, text=True, encoding="utf-8", errors="strict",
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120, check=False
    )
    destination.write_text(completed.stdout, encoding="utf-8")
    if completed.returncode != 0:
        raise RuntimeError(f"Control exited {completed.returncode}; retained {destination}")
    result: dict[str, Observation] = parse_output(completed.stdout)
    return result


def write_json(path: Path, value: object) -> None:
    encoded: str = json.dumps(value, indent=2, allow_nan=False)
    path.write_text(encoded + "\n", encoding="utf-8")


def main() -> int:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", required=True, type=Path)
    parser.add_argument("--candidate", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--pairs", type=int, default=12)
    arguments: argparse.Namespace = parser.parse_args()
    baseline_argument: Path = arguments.baseline
    candidate_argument: Path = arguments.candidate
    output_argument: Path = arguments.output
    baseline: Path = baseline_argument.resolve(strict=True)
    candidate: Path = candidate_argument.resolve(strict=True)
    output: Path = output_argument.resolve()
    pairs: int = arguments.pairs
    if pairs < 6 or pairs > 50:
        raise ValueError("Pair count must be in [6, 50]")
    verify_binary(baseline, BASELINE_SHA256)
    verify_binary(candidate, CANDIDATE_SHA256)
    # Refuse an existing destination; failed/partial earlier runs remain intact.
    output.mkdir(parents=True, exist_ok=False)
    environment: dict[str, str] = dict(os.environ)
    environment["FILEMAN_ENGINE_M2_MEASURE"] = "1"
    environment["FILEMAN_ENGINE_RECORDS"] = "10000"
    environment["GOMAXPROCS"] = "2"
    environment["GOTOOLCHAIN"] = "local"
    manifest: dict[str, object] = {
        "baseline": str(baseline), "baseline_sha256": BASELINE_SHA256,
        "candidate": str(candidate), "candidate_sha256": CANDIDATE_SHA256,
        "platform": platform.platform(), "python": sys.version,
        "logical_cpus": os.cpu_count(), "pairs": pairs,
        "records": 10000, "gomaxprocs": 2,
        "status": "incomplete", "started_unix_seconds": time.time(),
        "interpretation": "Aggregate-control noise check; no per-query tail or candidate admission claim."
    }
    manifest_path: Path = output / "manifest.json"
    write_json(manifest_path, manifest)
    warmup_index: int = 0
    for warmup_index in range(2):
        run_fixture(baseline, output / f"warmup-{warmup_index}-baseline.txt", environment)
        run_fixture(candidate, output / f"warmup-{warmup_index}-candidate.txt", environment)

    observations: list[dict[str, object]] = []
    changes: dict[str, dict[str, list[float]]] = {}
    arm: str = ""
    control: str = ""
    for arm in ("same_binary", "candidate"):
        changes[arm] = {}
        for control in CONTROL_NAMES:
            changes[arm][control] = []
    pair: int = 0
    for pair in range(pairs):
        arms: tuple[str, str] = ("same_binary", "candidate")
        if pair % 2 != 0:
            arms = ("candidate", "same_binary")
        for arm in arms:
            other: Path = baseline if arm == "same_binary" else candidate
            binaries: tuple[Path, Path] = (baseline, other)
            slots: tuple[int, int] = (0, 1) if pair % 2 == 0 else (1, 0)
            samples: list[dict[str, Observation]] = [{}, {}]
            slot: int = 0
            for slot in slots:
                destination: Path = output / f"pair-{pair:02d}-{arm}-{slot}.txt"
                samples[slot] = run_fixture(binaries[slot], destination, environment)
            for control in CONTROL_NAMES:
                before: Observation = samples[0][control]
                after: Observation = samples[1][control]
                ratio: float = after.nanoseconds_per_query / before.nanoseconds_per_query
                percent: float = 100.0 * (ratio - 1.0)
                changes[arm][control].append(percent)
                observations.append({
                    "pair": pair, "arm": arm, "control": control,
                    "execution_order": slots, "before": asdict(before),
                    "after": asdict(after), "percent_change": percent
                })
            print(f"completed pair {pair + 1}/{pairs}, {arm}", flush=True)
        # Checkpoint after every pair; interrupted measurements remain inspectable.
        write_json(output / "observations.json", observations)
    summaries: list[dict[str, object]] = []
    for arm in ("same_binary", "candidate"):
        for control in CONTROL_NAMES:
            values: list[float] = changes[arm][control]
            median: float = statistics.median(values)
            summaries.append({"arm": arm, "control": control, "median_percent": median,
                              "minimum_percent": min(values), "maximum_percent": max(values),
                              "paired_percent_changes": values})
            print(f"{arm} {control}: median paired change {median:+.3f}%", flush=True)
    write_json(output / "summary.json", summaries)
    manifest["status"] = "completed"
    manifest["finished_unix_seconds"] = time.time()
    write_json(manifest_path, manifest)
    return 0


if __name__ == "__main__":
    sys.exit(main())
