#!/usr/bin/env python3
"""Record the compiler/SDK cache identity in GitHub's caller-owned output file."""
import hashlib
import os
from pathlib import Path
import platform
import subprocess
from typing import TextIO
from native_build_support import Digest


def main() -> None:
    commands: list[list[str]] = [['clang', '--version']]
    if platform.system() == 'Darwin':
        commands.append(['xcrun', '--show-sdk-version'])
    text: str = os.environ.get('ImageVersion', '')
    command: list[str]
    for command in commands:
        captured: str = subprocess.check_output(command, text=True)
        text += captured
    encoded: bytes = text.encode('utf-8')
    digest: Digest = hashlib.sha256(encoded)
    hexadecimal: str = digest.hexdigest()
    identity: str = hexadecimal[:20]
    destination: Path = Path(os.environ['GITHUB_OUTPUT'])
    stream: TextIO
    with destination.open('a', encoding='utf-8') as stream:
        stream.write('identity=' + identity + '\n')


if __name__ == '__main__':
    main()
