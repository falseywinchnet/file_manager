"""Validate a pinned backend archive before copying its two service executables.

No member path is extracted from the archive. Only explicitly named regular
files are read, hashed, and written to the caller's build directory.
"""
from __future__ import annotations
import hashlib
import json
from pathlib import Path
import tarfile
from typing import BinaryIO


def read_member(archive: tarfile.TarFile, name: str) -> bytes:
    member: tarfile.TarInfo = archive.getmember('backend/' + name)
    if not member.isfile() or member.size > 256 * 1024 * 1024:
        raise ValueError('Invalid backend member: ' + name)
    stream: BinaryIO | None = archive.extractfile(member)
    if stream is None:
        raise ValueError('Missing backend member: ' + name)
    with stream:
        contents: bytes = stream.read()
    return contents


def install_bundle(bundle: Path, destination: Path, platform: str,
                   revision: str, expected_digest: str) -> None:
    contents: bytes = bundle.read_bytes()
    digest: str = hashlib.sha256(contents).hexdigest()
    if digest != expected_digest:
        raise ValueError('Backend archive SHA-256 does not match dependency lock')
    suffix: str = ''
    if platform == 'windows-x64':
        suffix = '.exe'
    validated: dict[str, bytes] = {}
    archive: tarfile.TarFile
    with tarfile.open(bundle, 'r:gz') as archive:
        manifest_bytes: bytes = read_member(archive, 'manifest.json')
        manifest: dict[str, object] = json.loads(manifest_bytes)
        if (manifest.get('schema') != 1 or manifest.get('platform') != platform or
                manifest.get('revision') != revision or
                manifest.get('repository') != 'falseywinchnet/backend'):
            raise ValueError('Backend manifest identity does not match dependency lock')
        files: object = manifest.get('files')
        if not isinstance(files, dict):
            raise ValueError('Backend file manifest is missing')
        name: str
        for name in ['fileman-engine' + suffix, 'orchestrator' + suffix]:
            data: bytes = read_member(archive, name)
            actual: str = hashlib.sha256(data).hexdigest()
            if actual != files.get(name):
                raise ValueError('Backend executable SHA-256 mismatch: ' + name)
            validated[name] = data
    destination.mkdir(parents=True, exist_ok=True)
    for name in validated:
        temporary: Path = destination / (name + '.pending')
        temporary.write_bytes(validated[name])
        temporary.chmod(0o755)
        temporary.replace(destination / name)
    (destination.parent / 'backend-consumption.json').write_bytes(manifest_bytes)
