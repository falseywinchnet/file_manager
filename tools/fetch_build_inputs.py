#!/usr/bin/env python3
"""Download exact release inputs; missing compiler seeds fall back to compilation."""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import subprocess
import tarfile
from typing import BinaryIO
from native_build_support import sha256

ROOT: Path = Path(__file__).resolve().parents[1]


def restore_cache_seed(path: Path, destination: Path, platform: str, revision: str) -> None:
    # Cache entries are build inputs from the pinned provider. Validate the
    # complete member set before extracting anything into the consumer.
    archive: tarfile.TarFile
    with tarfile.open(path, 'r:gz') as archive:
        member: tarfile.TarInfo
        for member in archive.getmembers():
            member_path: Path = Path(member.name)
            if (member_path.is_absolute() or '..' in member_path.parts or
                    not member_path.parts or
                    (member_path.parts[0] != '.ccache' and member.name != 'cache-manifest.json') or
                    not (member.isfile() or member.isdir())):
                raise ValueError('Invalid compiler-cache archive member')
        stream: BinaryIO | None = archive.extractfile('cache-manifest.json')
        if stream is None:
            raise ValueError('Compiler cache manifest is missing')
        with stream:
            manifest: dict[str, object] = json.load(stream)
        if (manifest.get('schema') != 1 or
                manifest.get('repository') != 'falseywinchnet/gui_forms' or
                manifest.get('platform') != platform or
                manifest.get('revision') != revision):
            raise ValueError('Compiler cache provider identity does not match lock')
        archive.extractall(destination, filter='data')


def verify_source_pins(lock: dict[str, dict[str, object]], root: Path) -> None:
    component: str
    for component in ['backend', 'gui_forms']:
        current: str = subprocess.check_output(
            ['git', '-C', str(root / component), 'rev-parse', 'HEAD'], text=True).strip()
        if current != lock[component]['revision']:
            raise ValueError('Source submodule differs from dependency lock: ' + component)


def main() -> None:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--platform', required=True,
                        choices=['macos-arm64', 'linux-x64', 'windows-x64'])
    parser.add_argument('--skip-cache', action='store_true')
    args: argparse.Namespace = parser.parse_args()
    lock: dict[str, dict[str, object]] = json.loads(
        (ROOT / 'dependencies.lock.json').read_text(encoding='utf-8'))
    verify_source_pins(lock, ROOT)
    output: Path = ROOT / '.build/inputs' / args.platform
    output.mkdir(parents=True, exist_ok=True)
    component: str
    for component in ['backend', 'gui_forms']:
        if component == 'gui_forms' and args.skip_cache:
            continue
        entry: dict[str, object] = lock[component]
        prefix: str = 'backend-'
        if component == 'gui_forms':
            prefix = 'gui-forms-cache-'
        name: str = prefix + args.platform + '.tar.gz'
        command: list[str] = ['gh', 'release', 'download', str(entry['release']),
                              '--repo', str(entry['repository']), '--pattern', name,
                              '--dir', str(output), '--clobber']
        result: subprocess.CompletedProcess[bytes] = subprocess.run(command, cwd=ROOT)
        if result.returncode != 0:
            if component == 'backend':
                raise RuntimeError('Required pinned backend bundle is unavailable')
            print('Compiler cache seed unavailable; normal compilation remains enabled')
            continue
        hashes: object = entry['sha256']
        if not isinstance(hashes, dict):
            raise ValueError('Missing archive hashes in dependency lock')
        actual: str = sha256(output / name)
        if actual != hashes.get(args.platform):
            raise ValueError('Downloaded archive does not match dependency lock: ' + name)
        if component == 'gui_forms':
            restore_cache_seed(output / name, ROOT, args.platform, str(entry['revision']))


if __name__ == '__main__':
    main()
