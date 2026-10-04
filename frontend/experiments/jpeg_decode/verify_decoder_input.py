"""Generated-fixture conformance for the decoder-only research executable."""

import argparse
from dataclasses import dataclass
import struct
import subprocess


INPUT_LIMIT: int = 16 * 1024 * 1024
OUTPUT_LIMIT: int = 64 + 4 * 1024 * 1024


@dataclass(frozen=True)
class Refusal:
    name: str
    payload: bytes


def envelope(length: int, payload: bytes = b"") -> bytes:
    encoded_length: bytes = struct.pack("<Q", length)
    result: bytes = b"FMJPEG01" + encoded_length + payload
    return result


def invoke(command: list[str], payload: bytes, output_limit: int = OUTPUT_LIMIT) -> subprocess.CompletedProcess[bytes]:
    # Fixed local test binaries only. communicate/run drains both pipes; timeout
    # terminates and waits for this child. This is not the product supervisor or
    # a hard output-memory limiter; the tested writer itself bounds its output.
    result: subprocess.CompletedProcess[bytes] = subprocess.run(
        command, input=payload, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        timeout=15, check=False,
    )
    if len(result.stdout) > output_limit:
        raise RuntimeError("fixture output exceeds declared frame limit")
    return result


def verify(producer: str, decoder: str) -> None:
    generated: subprocess.CompletedProcess[bytes] = invoke(
        [producer, "--emit-encoded", "1"], b"", INPUT_LIMIT + 16,
    )
    if generated.returncode != 0 or not 16 < len(generated.stdout) <= INPUT_LIMIT + 16:
        raise RuntimeError("encoded fixture generation failed")
    expected: subprocess.CompletedProcess[bytes] = invoke([producer, "--emit-frame", "1"], b"")
    if expected.returncode != 0 or len(expected.stdout) <= 64:
        raise RuntimeError("direct reference frame failed")
    accepted: subprocess.CompletedProcess[bytes] = invoke([decoder], generated.stdout)
    if accepted.returncode != 0 or accepted.stdout != expected.stdout:
        raise RuntimeError("separate decoder differs from direct prepared pixels")
    print("separate decoder exactly matches direct baseline frame")

    cases: list[Refusal] = [
        Refusal("empty", b""),
        Refusal("short-header", b"FMJPEG01"),
        Refusal("wrong-magic", b"BADJPEG1" + struct.pack("<Q", 1) + b"x"),
        Refusal("zero-length", envelope(0)),
        Refusal("oversized", envelope(INPUT_LIMIT + 1)),
        Refusal("maximum-u64", envelope(2**64 - 1)),
        Refusal("short-body", envelope(10, b"abcd")),
        Refusal("trailing-byte", generated.stdout + b"x"),
        Refusal("invalid-jpeg", envelope(4, b"abcd")),
    ]
    case: Refusal
    for case in cases:
        refused: subprocess.CompletedProcess[bytes] = invoke([decoder], case.payload)
        if refused.returncode != 1 or refused.stdout:
            raise RuntimeError("refusal emitted pixels or unexpected exit: " + case.name)
        print("refused with no preview bytes: " + case.name)
    recovered: subprocess.CompletedProcess[bytes] = invoke([decoder], generated.stdout)
    if recovered.returncode != 0 or recovered.stdout != expected.stdout:
        raise RuntimeError("fresh decoder after refusal did not recover")
    print("fresh decoder after refusals exactly matches reference")


def main() -> None:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("producer")
    parser.add_argument("decoder")
    arguments: argparse.Namespace = parser.parse_args()
    verify(arguments.producer, arguments.decoder)


if __name__ == "__main__":
    main()
