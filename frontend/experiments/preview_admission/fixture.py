"""Generate bounded same-pixel PNG controls; never overwrite an existing fixture."""
import argparse
from pathlib import Path
import struct
from typing import BinaryIO
import zlib


def chunk(kind: bytes, data: bytes) -> bytes:
    payload: bytes = kind + data
    checksum: int = zlib.crc32(payload)
    result: bytes = struct.pack(">I", len(data)) + payload + struct.pack(">I", checksum)
    return result


def write_fixture(root: Path, height: int, compression: int, prefix: str) -> None:
    width: int = 1024
    row: bytes = b"\x00" + bytes((12, 226, 198, 255)) * width
    raw: bytes = row * height
    encoded: bytes = zlib.compress(raw, compression)
    header: bytes = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    png: bytes = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", header)
    png += chunk(b"IDAT", encoded)
    png += chunk(b"IEND", b"")
    path: Path = root / f"{prefix}-{height}.png"
    output: BinaryIO
    with path.open("xb") as output:
        output.write(png)
    print(f"{path.name}: {len(png)} encoded bytes")


def main() -> None:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    arguments: argparse.Namespace = parser.parse_args()
    root: Path = arguments.output.resolve()
    root.mkdir(parents=True, exist_ok=True)
    write_fixture(root, 64, 0, "rgba")
    write_fixture(root, 256, 0, "rgba")
    write_fixture(root, 1024, 0, "rgba")
    write_fixture(root, 64, 6, "deflate")
    write_fixture(root, 256, 6, "deflate")
    write_fixture(root, 1024, 6, "deflate")


if __name__ == "__main__":
    main()
