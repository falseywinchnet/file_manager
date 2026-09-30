#!/usr/bin/env python3
"""Collect package identity/hash checks and platform facts; never upload.

This file is standalone in exported packages. Streams and child processes are
owned only within the call that acquires them; no callbacks retain borrowed state.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
from typing import BinaryIO, TextIO, TypedDict, Protocol, cast

HASH_BUFFER_BYTES: int = 1024 * 1024
STARTUP_LOG_BYTES: int = 65536

class Digest(Protocol):
    def update(self, data: memoryview) -> None: ...
    def hexdigest(self) -> str: ...


class DiagnosticReceipt(TypedDict):
    product: str
    version: str
    source_revision: str
    source_dirty: bool
    platform: str
    architecture: str
    executable: str
    files: dict[str, str]


def verify_files(root: Path, files: dict[str, str]) -> dict[str, str]:
    """Observe only receipt-listed files inside root. Read scratch is reused."""
    checks: dict[str, str] = {}
    scratch: bytearray = bytearray(HASH_BUFFER_BYTES)
    view: memoryview = memoryview(scratch)
    relative: str
    stream: BinaryIO
    for relative in files:
        candidate: Path = root / relative
        path: Path = candidate.resolve()
        if not path.is_relative_to(root):
            raise RuntimeError('Package receipt contains an out-of-package path')
        if not path.is_file():
            checks[relative] = 'missing'
            continue
        digest: Digest = hashlib.sha256()
        count: int = 0
        with path.open('rb') as stream:
            while True:
                count = stream.readinto(scratch)
                if count == 0:
                    break
                digest.update(view[:count])
        actual: str = digest.hexdigest()
        checks[relative] = 'mismatch'
        if actual == files[relative]:
            checks[relative] = 'matched'
    return checks


def stop_owned_process(process: subprocess.Popen[bytes]) -> None:
    """Reap only the acquired child, including during exceptional unwinding."""
    if process.poll() is not None:
        process.wait()
        return
    process.terminate()
    try:
        process.wait(timeout=10)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait()


def redact_startup_log(text: str, fixture: Path, root: Path) -> str:
    replacements: dict[Path, str] = {fixture: '<fixture>', root: '<package>', Path.home(): '<home>'}
    path: Path
    result: str = text
    for path in replacements:
        replacement: str = replacements[path]
        result = result.replace(str(path), replacement)
        portable_name: str = path.as_posix()
        result = result.replace(portable_name, replacement)
    return result


def launch_diagnostic(root: Path, executable_name: str, checks: dict[str, str]) -> dict[str, object]:
    candidate: Path = root / executable_name
    executable: Path = candidate.resolve()
    if not executable.is_relative_to(root) or not executable.is_file():
        raise RuntimeError('Package executable is invalid')
    state: str
    for state in checks.values():
        if state != 'matched':
            raise RuntimeError('Package verification failed; refusing diagnostic launch. Run without --launch for the hash report.')
    startup: dict[str, object] = {}
    temporary: str
    stream: BinaryIO
    with tempfile.TemporaryDirectory(prefix='file-manager-diagnostic-') as temporary:
        temporary_path: Path = Path(temporary)
        fixture: Path = temporary_path / 'empty-root'
        fixture.mkdir()
        log: Path = temporary_path / 'startup.log'
        environment: dict[str, str] = os.environ.copy()
        environment.pop('GUI_FORMS_FONT_DIR', None)
        environment.pop('FILE_MANAGER_ROOT', None)
        command: list[str] = [str(executable), '--root', str(fixture)]
        with log.open('wb') as stream:
            process: subprocess.Popen[bytes] = subprocess.Popen(
                command, cwd=root, env=environment, stdout=stream, stderr=stream)
            try:
                try:
                    exit_code: int = process.wait(timeout=5)
                    startup = {'state': 'exited', 'exit_code': exit_code}
                except subprocess.TimeoutExpired:
                    startup = {'state': 'alive after five seconds; diagnostic process terminated'}
            finally:
                stop_owned_process(process)
        with log.open('rb') as stream:
            log_bytes: bytes = stream.read(STARTUP_LOG_BYTES)
        log_text: str = log_bytes.decode('utf-8', errors='replace')
        redacted: str = redact_startup_log(log_text, temporary_path, root)
        startup['log_first_64k'] = redacted
    return startup


def collect_report(root: Path, launch: bool, timestamp: datetime) -> dict[str, object]:
    receipt_path: Path = root / 'build-receipt.json'
    receipt_text: str = receipt_path.read_text(encoding='utf-8')
    receipt: DiagnosticReceipt = cast(DiagnosticReceipt, json.loads(receipt_text))
    checks: dict[str, str] = verify_files(root, receipt['files'])
    host: dict[str, str] = {'system': platform.system(), 'release': platform.release(),
                            'version': platform.version(), 'machine': platform.machine(),
                            'macos': platform.mac_ver()[0]}
    report: dict[str, object] = {
        'schema': 1, 'collected_utc': timestamp.isoformat(),
        'product': receipt['product'], 'version': receipt['version'],
        'source_revision': receipt['source_revision'], 'source_dirty': receipt['source_dirty'],
        'package_platform': receipt['platform'], 'package_architecture': receipt['architecture'],
        'host': host, 'package_checks': checks,
        'privacy': 'No personal paths, directory listings, environment, files or application logs collected. No network upload.'}
    if launch:
        startup: dict[str, object] = launch_diagnostic(root, receipt['executable'], checks)
        report['startup'] = startup
        report['privacy'] = 'Generated empty-root startup only; known home/package/fixture path prefixes redacted. Review log before sharing. No uploads or personal file contents collected.'
    return report


def write_report(destination: Path, report: dict[str, object]) -> None:
    """Exclusive creation preserves an existing report on failure."""
    stream: TextIO
    with destination.open('x', encoding='utf-8') as stream:
        json.dump(report, stream, indent=2)
        stream.write('\n')


def main() -> None:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, help='New report filename; existing files are never overwritten')
    parser.add_argument('--launch', action='store_true', help='Run this package for five seconds on a generated empty root and include a bounded startup log')
    arguments: argparse.Namespace = parser.parse_args()
    script_path: Path = Path(__file__).resolve()
    root: Path = script_path.parent
    timestamp: datetime = datetime.now(timezone.utc)
    report: dict[str, object] = collect_report(root, arguments.launch, timestamp)
    destination: Path | None = arguments.output
    if destination is None:
        suffix: str = timestamp.strftime('%Y%m%dT%H%M%SZ')
        destination = Path.cwd() / ('file-manager-diagnostics-' + suffix + '.json')
    write_report(destination, report)
    print(f'Local report created: {destination}. Review before sharing. Nothing was uploaded.')


if __name__ == '__main__':
    main()
