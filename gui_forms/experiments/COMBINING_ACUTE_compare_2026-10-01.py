"""Compare the immutable pre-fix golden with the scoped acute-coverage result."""
from pathlib import Path
import gzip


def main() -> None:
    golden_path: Path = Path("gui_forms/experiments/TEXT_LAYOUT_REUSE_geometry_baseline_2026-10-01.txt.gz")
    candidate_path: Path = Path("gui_forms/.build/text-layout-attribution/geometry-acute.txt")
    compressed: bytes = golden_path.read_bytes()
    golden: bytes = gzip.decompress(compressed)
    candidate: bytes = candidate_path.read_bytes()
    before: list[bytes] = golden.splitlines(keepends=True)
    after: list[bytes] = candidate.splitlines(keepends=True)
    count: int = len(before)
    if count != len(after):
        raise RuntimeError("Geometry record count changed")
    differences: int = 0
    index: int = 0
    for index in range(count):
        if before[index] == after[index]:
            continue
        old: list[bytes] = before[index].split(b",")
        new: list[bytes] = after[index].split(b",")
        expected_record: bool = old[0:2] == [b"shape", b"mixed_controls"]
        unchanged_fields: bool = old[:13] == new[:13] and old[14:] == new[14:]
        expected_coverage: bool = old[13] == b"3" and new[13] == b"2"
        if not expected_record or not unchanged_fields or not expected_coverage:
            raise RuntimeError("Unexpected glyph, face, metric, source or coverage change")
        differences += 1
        print("coverage-only", index + 1, old[3], old[13], new[13])
    if differences != 3:
        raise RuntimeError("Expected exactly three mixed-control coverage corrections")
    print("PASS: three coverage-only deltas; all other serialized bytes unchanged")


if __name__ == "__main__":
    main()
