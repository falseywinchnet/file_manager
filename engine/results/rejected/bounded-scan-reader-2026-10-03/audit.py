from pathlib import Path
import hashlib
import json
import re
import statistics

root: Path = Path(__file__).resolve().parent
engine: Path = root.parent.parent
names: list[str] = ["Path", "Name", "Pipeline"]
variants: list[str] = ["baseline", "candidate"]
records: dict[str, list[dict[str, float]]] = {}
manifest: dict[str, object] = {"baseline_parent_reported": "d4dcb245", "source_normalization": "CRLF to LF only for text comparisons and shared decoder/harness copies"}
hashes: dict[str, str] = {}
source_paths: list[str] = ["internal/generation/segment.go", "internal/generation/bounded_scan_candidate.go", "internal/generation/bounded_scan_candidate_test.go", "benchmarks/bounded_scan_candidate_test.go", "internal/generation/cache.go", "benchmarks/m2_generation_test.go"]
variant: str = ""
name: str = ""
relative: str = ""
pair: int = 0
for variant in variants:
    for name in names:
        records[variant + name] = []
    for pair in range(6):
        path: Path = root / f"pair-{pair}-{variant}.txt"
        raw: bytes = path.read_bytes()
        # Windows PowerShell redirects native output as UTF-16; pwsh uses UTF-8.
        text: str = raw.decode("utf-16") if raw.startswith(b"\xff\xfe") else raw.decode("utf-8-sig")
        for name in names:
            pattern: str = rf"aggregate=sampleGeneration{name} calls=(\d+) elapsed=(\S+) ns_per_query=([\d.]+) allocated_bytes=(\d+) mallocs=(\d+)"
            match: re.Match[str] | None = re.search(pattern, text)
            if match is None:
                raise RuntimeError(f"Missing aggregate {variant} {pair} {name}")
            count: int = int(match.group(1))
            observation: dict[str, float] = {"ns": float(match.group(3)), "bytes_per_query": int(match.group(4)) / count, "mallocs_per_query": int(match.group(5)) / count}
            records[variant + name].append(observation)
            print(f"{pair},{variant},{name},{match.group(1)},{match.group(2)},{match.group(3)},{match.group(4)},{match.group(5)}")
        if "6c4437450b9c86ff" not in text or text.count("a69b9113a32327fd") != 2 or "PASS" not in text:
            raise RuntimeError(f"Digest/status mismatch: {path.name}")
    for relative in source_paths:
        source: Path = root / variant / "engine" / relative
        if source.exists():
            hashes[f"{variant}/engine/{relative}"] = hashlib.sha256(source.read_bytes()).hexdigest()

for name in names:
    before: list[float] = []
    after: list[float] = []
    deltas: list[float] = []
    base_bytes: list[float] = []
    candidate_bytes: list[float] = []
    for pair in range(6):
        base: dict[str, float] = records["baseline" + name][pair]
        candidate: dict[str, float] = records["candidate" + name][pair]
        before.append(base["ns"])
        after.append(candidate["ns"])
        deltas.append(100 * (candidate["ns"] / base["ns"] - 1))
        base_bytes.append(base["bytes_per_query"])
        candidate_bytes.append(candidate["bytes_per_query"])
    print(f"SUMMARY {name}: baseline={statistics.median(before):.2f} candidate={statistics.median(after):.2f} paired_percent={statistics.median(deltas):+.4f} bytes={statistics.median(base_bytes):.4f}/{statistics.median(candidate_bytes):.4f} deltas={deltas}")

# Confirm the final active implementation matches the timed candidate snapshot.
for relative in source_paths:
    if relative == "benchmarks/m2_generation_test.go":
        continue
    active_text: str = (engine / relative).read_text(encoding="utf-8").replace("\r\n", "\n")
    snapshot_text: str = (root / "candidate" / "engine" / relative).read_text(encoding="utf-8").replace("\r\n", "\n")
    if active_text != snapshot_text:
        raise RuntimeError(f"Active source changed after measurement: {relative}")
    hashes["active/engine/" + relative] = hashlib.sha256((engine / relative).read_bytes()).hexdigest()

artifact: Path = root
for artifact in sorted(root.iterdir()):
    if artifact.is_file() and artifact.suffix in (".txt", ".exe", ".patch", ".ps1", ".py") and artifact.name != "audit.txt":
        hashes[artifact.name] = hashlib.sha256(artifact.read_bytes()).hexdigest()
manifest["hashes"] = hashes
manifest["observations"] = records
(root / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
print(json.dumps(hashes, indent=2))
