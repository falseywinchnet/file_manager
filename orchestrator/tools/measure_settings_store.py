#!/usr/bin/env python3
"""Compare bounded settings-store candidates on the current host.

This is a workload probe, not product storage code.  It intentionally uses only
Python's standard library so the M4 measurement does not add a runtime
dependency to Orchestrator.
"""

from __future__ import annotations

import json
import os
from pathlib import Path
import shutil
import sqlite3
import statistics
import tempfile
import time


FIELD_COUNT = 48
SINGLE_WRITES = 300
BATCH_WRITES = 100


def percentile(samples: list[float], fraction: float) -> float:
    ordered = sorted(samples)
    index = min(len(ordered) - 1, int((len(ordered) - 1) * fraction))
    return ordered[index]


def metrics(samples: list[float]) -> dict[str, float]:
    return {
        "p50_ms": round(statistics.median(samples) * 1000, 3),
        "p95_ms": round(percentile(samples, 0.95) * 1000, 3),
        "p99_ms": round(percentile(samples, 0.99) * 1000, 3),
        "max_ms": round(max(samples) * 1000, 3),
    }


def directory_bytes(path: Path) -> int:
    return sum(item.stat().st_size for item in path.iterdir() if item.is_file())


def atomic_write(path: Path, payload: bytes) -> None:
    temporary = path.with_name(path.name + ".next")
    with temporary.open("wb") as output:
        output.write(payload)
        output.flush()
        os.fsync(output.fileno())
    os.replace(temporary, path)
    directory = os.open(path.parent, os.O_RDONLY)
    try:
        os.fsync(directory)
    finally:
        os.close(directory)


def json_workload(root: Path) -> dict[str, object]:
    root.mkdir()
    primary = root / "settings.json"
    backup = root / "settings.previous.json"
    document = {"format": 1, "revision": 0,
                "values": {f"field.{i}": False for i in range(FIELD_COUNT)}}
    atomic_write(primary, json.dumps(document, sort_keys=True).encode())

    single: list[float] = []
    for iteration in range(SINGLE_WRITES):
        before = primary.read_bytes()
        document["revision"] += 1
        document["values"][f"field.{iteration % FIELD_COUNT}"] = bool(iteration % 2)
        start = time.perf_counter()
        atomic_write(backup, before)
        atomic_write(primary, json.dumps(document, sort_keys=True).encode())
        single.append(time.perf_counter() - start)

    batch: list[float] = []
    for iteration in range(BATCH_WRITES):
        before = primary.read_bytes()
        document["revision"] += 1
        for offset in range(8):
            field = (iteration * 8 + offset) % FIELD_COUNT
            document["values"][f"field.{field}"] = bool((iteration + offset) % 2)
        start = time.perf_counter()
        atomic_write(backup, before)
        atomic_write(primary, json.dumps(document, sort_keys=True).encode())
        batch.append(time.perf_counter() - start)

    primary.write_text("{truncated", encoding="utf-8")
    recovered = json.loads(backup.read_text(encoding="utf-8"))["revision"] == document["revision"] - 1
    return {
        "single": metrics(single),
        "batch_8": metrics(batch),
        "store_bytes": directory_bytes(root),
        "corrupt_primary_recovered_previous": recovered,
    }


def sqlite_workload(root: Path) -> dict[str, object]:
    root.mkdir()
    database = root / "settings.sqlite3"
    connection = sqlite3.connect(database)
    connection.execute("PRAGMA journal_mode=WAL")
    connection.execute("PRAGMA synchronous=FULL")
    connection.execute("CREATE TABLE settings (id TEXT PRIMARY KEY, value INTEGER NOT NULL)")
    connection.execute("CREATE TABLE metadata (revision INTEGER NOT NULL)")
    connection.execute("INSERT INTO metadata VALUES (0)")
    connection.executemany("INSERT INTO settings VALUES (?, 0)",
                           [(f"field.{index}",) for index in range(FIELD_COUNT)])
    connection.commit()

    single: list[float] = []
    for iteration in range(SINGLE_WRITES):
        start = time.perf_counter()
        with connection:
            connection.execute("UPDATE settings SET value=? WHERE id=?",
                               (iteration % 2, f"field.{iteration % FIELD_COUNT}"))
            connection.execute("UPDATE metadata SET revision=revision+1")
        single.append(time.perf_counter() - start)

    batch: list[float] = []
    for iteration in range(BATCH_WRITES):
        start = time.perf_counter()
        with connection:
            for offset in range(8):
                field = (iteration * 8 + offset) % FIELD_COUNT
                connection.execute("UPDATE settings SET value=? WHERE id=?",
                                   ((iteration + offset) % 2, f"field.{field}"))
            connection.execute("UPDATE metadata SET revision=revision+1")
        batch.append(time.perf_counter() - start)

    connection.execute("BEGIN IMMEDIATE")
    revision = connection.execute("SELECT revision FROM metadata").fetchone()[0]
    connection.execute("UPDATE metadata SET revision=revision+1")
    connection.rollback()
    rollback_recovered = connection.execute(
        "SELECT revision FROM metadata").fetchone()[0] == revision
    integrity = connection.execute("PRAGMA integrity_check").fetchone()[0]
    connection.execute("PRAGMA wal_checkpoint(TRUNCATE)")
    connection.close()
    return {
        "single": metrics(single),
        "batch_8": metrics(batch),
        "store_bytes": directory_bytes(root),
        "rollback_recovered": rollback_recovered,
        "integrity_check": integrity,
    }


def main() -> None:
    temporary = Path(tempfile.mkdtemp(prefix="fileman-settings-measure-"))
    try:
        result = {
            "workload": {
                "fields": FIELD_COUNT,
                "single_writes": SINGLE_WRITES,
                "batch_writes": BATCH_WRITES,
                "durability": "fsync/FULL per committed transaction",
            },
            "atomic_json_snapshot": json_workload(temporary / "json"),
            "sqlite_wal_full": sqlite_workload(temporary / "sqlite"),
        }
        print(json.dumps(result, indent=2, sort_keys=True))
    finally:
        shutil.rmtree(temporary)


if __name__ == "__main__":
    main()
