import argparse
import pathlib
import struct
from typing import BinaryIO
import zlib


def chunk(kind: bytes, data: bytes) -> bytes:
    payload: bytes = kind + data
    checksum: int = zlib.crc32(payload)
    result: bytes = struct.pack(">I", len(data)) + payload + struct.pack(">I", checksum)
    return result


def main() -> None:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(
        description="Write owned generated PNG admission fixtures to a build directory")
    parser.add_argument("output", type=pathlib.Path)
    arguments: argparse.Namespace = parser.parse_args()
    root: pathlib.Path = arguments.output.resolve()
    root.mkdir(parents=True, exist_ok=True)
    width: int = 1024
    row: bytes = bytes((0,)) + bytes((12, 226, 198)) * width
    height: int
    for height in (256, 2048, 4096):
        raw: bytes = row * height
        encoded: bytes = zlib.compress(raw, 0)
        header: bytes = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
        png: bytes = bytes((137, 80, 78, 71, 13, 10, 26, 10))
        png += chunk(b"IHDR", header)
        png += chunk(b"IDAT", encoded)
        png += chunk(b"IEND", b"")
        target: pathlib.Path = root / ("uncompressed-" + str(height) + ".png")
        output: BinaryIO
        with target.open("xb") as output:
            output.write(png)
        print(str(target), len(png))


if __name__ == "__main__":
    main()
