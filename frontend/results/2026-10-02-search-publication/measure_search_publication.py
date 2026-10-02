from pathlib import Path
import json
import math
import statistics
import subprocess

ROOT: Path = Path(__file__).resolve().parents[3]
OUTPUT: Path = ROOT / ".build/search-publication-pairs"
EXECUTABLES: dict[str, Path] = {
    "baseline": ROOT / ".build/details-consumer/file_manager_search_projection_baseline.exe",
    "worker": ROOT / ".build/details-consumer/file_manager_application_latency_benchmark.exe",
}

def quantiles(values: list[float]) -> dict[str, float]:
    ordered: list[float] = sorted(values)
    result: dict[str, float] = {"p50": statistics.median(ordered), "max": ordered[-1]}
    percentile: int = 0
    for percentile in (95, 99):
        index: int = math.ceil(len(ordered) * percentile / 100) - 1
        result["p" + str(percentile)] = ordered[index]
    return result

def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    rows: list[dict[str, str]] = []
    pair: int = 0
    lane: str = ""
    for pair in range(6):
        lanes: tuple[str, str] = ("baseline", "worker")
        if pair % 2 != 0:
            lanes = ("worker", "baseline")
        for lane in lanes:
            result: subprocess.CompletedProcess[str] = subprocess.run(
                [str(EXECUTABLES[lane]), "--search-projection"], cwd=ROOT,
                text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True,
            )
            log: Path = OUTPUT / (str(pair) + "-" + lane + ".csv")
            log.write_text(result.stdout, encoding="utf-8")
            if result.stderr:
                raise RuntimeError(result.stderr)
            count: int = 0
            line: str = ""
            for line in result.stdout.splitlines():
                fields: list[str] = line.split(",")
                if fields[0] != "projection":
                    raise RuntimeError("unexpected benchmark output: " + line)
                row: dict[str, str] = {"lane": lane, "pair": str(pair)}
                field: str = ""
                for field in fields[1:]:
                    key: str = ""
                    value: str = ""
                    key, value = field.split("=", 1)
                    row[key] = value
                rows.append(row)
                count += 1
            if count != 190:
                raise RuntimeError("incomplete projection run")
    summary: list[dict[str, object]] = []
    size: int = 0
    pages: int = 0
    page: int = 0
    for size, pages, page in ((25, 1, 0), (100, 1, 0), (500, 1, 0), (100, 10, 9)):
        for lane in ("baseline", "worker"):
            apply: list[float] = []
            layout: list[float] = []
            prepare: list[float] = []
            for row in rows:
                if (row["lane"] != lane or int(row["page_size"]) != size or
                    int(row["page_count"]) != pages or int(row["page"]) != page or
                    int(row["repetition"]) == 0):
                    continue
                apply.append(float(row["apply_ms"]))
                layout.append(float(row["layout_ms"]))
                prepare.append(float(row.get("prepare_ms", "0")))
            summary.append({"lane": lane, "page_size": size, "page_count": pages,
                            "page": page, "warm_samples": len(apply),
                            "apply_ms": quantiles(apply), "layout_ms": quantiles(layout),
                            "prepare_ms": quantiles(prepare)})
    output: str = json.dumps(summary, indent=2) + "\n"
    (OUTPUT / "summary.json").write_text(output, encoding="utf-8")
    print(output)

if __name__ == "__main__":
    main()
