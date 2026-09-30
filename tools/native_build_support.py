"""Native build command, source-provenance and bounded hashing primitives.

Commands are synchronous and never retain arguments. Hashing borrows its input
stream and reuses caller-owned scratch; it does not close or retain the stream.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import shutil
import subprocess
from typing import BinaryIO, Protocol, TypedDict

ROOT: Path = Path(__file__).resolve().parents[1]
HASH_BUFFER_BYTES: int = 1024 * 1024

class Digest(Protocol):
    def update(self, data: bytes | bytearray | memoryview) -> None: ...
    def hexdigest(self) -> str: ...

class SourceState(TypedDict):
    git_executable: str | None
    git_version: str
    source_revision: str
    source_dirty: bool
    status_porcelain: list[str]
    tracked_diff_stat: str
    git_config: list[str]

class BuildValidation(TypedDict):
    source_revision: str
    frontend_ctest: str
    gui_forms_sdk_sha256: str
    gui_forms_ctest: str


def command_arguments(arguments: tuple[str | Path | int, ...]) -> list[str]:
    """Convert the foreign process arguments once, preserving order."""
    converted: list[str] = []
    argument: str | Path | int
    for argument in arguments:
        converted.append(str(argument))
    return converted


def run(*arguments: str | Path | int, cwd: Path = ROOT) -> None:
    command: list[str] = command_arguments(arguments)
    description: str = ' '.join(command)
    print('+ ' + description, flush=True)
    subprocess.run(command, cwd=cwd, check=True)


def output(*arguments: str | Path | int, cwd: Path = ROOT) -> str:
    command: list[str] = command_arguments(arguments)
    captured: str = subprocess.check_output(command, cwd=cwd, text=True)
    result: str = captured.strip()
    return result


def git_output(*arguments: str) -> str:
    command: list[str] = ['git', '-C', str(ROOT)]
    command.extend(arguments)
    captured: str = subprocess.check_output(
        command, text=True, encoding='utf-8', errors='replace')
    result: str = captured.rstrip('\n')
    return result


def record_source_state(build: Path, stage: str) -> SourceState:
    """Write this Git implementation's view; never suppress dirty paths."""
    status: str = git_output('status', '--porcelain=v1', '--untracked-files=normal')
    command: list[str] = ['git', '-C', str(ROOT), 'config', '--show-origin',
                         '--get-regexp', r'^core\.(autocrlf|eol|filemode|ignorecase)$']
    config: subprocess.CompletedProcess[str] = subprocess.run(
        command, text=True, encoding='utf-8', errors='replace', capture_output=True)
    if config.returncode not in (0, 1):
        raise RuntimeError('Cannot inspect Git checkout policy: ' + config.stderr)
    implementation: str | None = shutil.which('git')
    version: str = git_output('--version')
    revision: str = git_output('rev-parse', 'HEAD')
    changed_paths: list[str] = status.splitlines()
    diff_stat: str = git_output('diff', 'HEAD', '--stat')
    config_lines: list[str] = config.stdout.splitlines()
    state: SourceState = {
        'git_executable': implementation, 'git_version': version,
        'source_revision': revision, 'source_dirty': bool(status),
        'status_porcelain': changed_paths, 'tracked_diff_stat': diff_stat,
        'git_config': config_lines}
    destination: Path = build / ('source-state-' + stage + '.json')
    encoded: str = json.dumps(state, indent=2) + '\n'
    destination.write_text(encoded, encoding='utf-8')
    count: int = len(changed_paths)
    print(f'Source state ({stage}): {count} changed paths; {destination}', flush=True)
    line: str
    for line in changed_paths[:40]:
        print('  ' + line, flush=True)
    return state


def update_digest(stream: BinaryIO, digest: Digest, scratch: bytearray) -> None:
    """Read at most len(scratch) bytes per step; scratch must be nonempty."""
    if len(scratch) == 0:
        raise ValueError('Hash scratch must not be empty')
    view: memoryview = memoryview(scratch)
    count: int = 0
    while True:
        count = stream.readinto(scratch)
        if count == 0:
            break
        digest.update(view[:count])


def stream_sha256(stream: BinaryIO, scratch: bytearray) -> str:
    digest: Digest = hashlib.sha256()
    update_digest(stream, digest, scratch)
    result: str = digest.hexdigest()
    return result


def sha256(path: Path) -> str:
    scratch: bytearray = bytearray(HASH_BUFFER_BYTES)
    stream: BinaryIO
    with path.open('rb') as stream:
        result: str = stream_sha256(stream, scratch)
    return result


def sdk_fingerprint(sdk: Path) -> str:
    """Bind relative UTF-8 names and exact bytes; one reusable read buffer."""
    digest: Digest = hashlib.sha256()
    scratch: bytearray = bytearray(HASH_BUFFER_BYTES)
    directory: str
    path: Path
    stream: BinaryIO
    for directory in ['include', 'lib', 'bin', 'share']:
        directory_path: Path = sdk / directory
        paths: list[Path] = sorted(directory_path.rglob('*'))
        for path in paths:
            if not path.is_file():
                continue
            relative: Path = path.relative_to(sdk)
            portable_name: str = relative.as_posix()
            encoded_name: bytes = portable_name.encode('utf-8')
            digest.update(encoded_name)
            digest.update(b'\0')
            with path.open('rb') as stream:
                update_digest(stream, digest, scratch)
            digest.update(b'\0')
    result: str = digest.hexdigest()
    return result
