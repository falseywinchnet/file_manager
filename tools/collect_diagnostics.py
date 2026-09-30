#!/usr/bin/env python3
"""Collect only package identity/hash checks and basic platform facts; never upload."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, help='New report filename; existing files are never overwritten')
    parser.add_argument('--launch', action='store_true', help='Run this package for five seconds on a generated empty root and include a bounded startup log')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent
    receipt = json.loads((root / 'build-receipt.json').read_text(encoding='utf-8'))
    checks = {}
    for relative, expected in receipt['files'].items():
        path = (root / relative).resolve()
        if not path.is_relative_to(root):
            raise RuntimeError('Package receipt contains an out-of-package path')
        if not path.is_file():
            checks[relative] = 'missing'
            continue
        digest = hashlib.sha256()
        with path.open('rb') as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b''):
                digest.update(chunk)
        checks[relative] = 'matched' if digest.hexdigest() == expected else 'mismatch'
    timestamp = datetime.now(timezone.utc)
    report = {'schema': 1, 'collected_utc': timestamp.isoformat(),
              'product': receipt['product'], 'version': receipt['version'],
              'source_revision': receipt['source_revision'], 'source_dirty': receipt['source_dirty'],
              'package_platform': receipt['platform'], 'package_architecture': receipt['architecture'],
              'host': {'system': platform.system(), 'release': platform.release(),
                       'version': platform.version(), 'machine': platform.machine(),
                       'macos': platform.mac_ver()[0]},
              'package_checks': checks,
              'privacy': 'No personal paths, directory listings, environment, files or application logs collected. No network upload.'}
    if args.launch:
        executable = (root / receipt['executable']).resolve()
        if not executable.is_relative_to(root) or not executable.is_file():
            raise RuntimeError('Package executable is invalid')
        with tempfile.TemporaryDirectory(prefix='file-manager-diagnostic-') as temporary:
            fixture = Path(temporary) / 'empty-root'
            fixture.mkdir()
            log = Path(temporary) / 'startup.log'
            env = os.environ.copy()
            env.pop('GUI_FORMS_FONT_DIR', None)
            env.pop('FILE_MANAGER_ROOT', None)
            with log.open('wb') as stream:
                process = subprocess.Popen([str(executable), '--root', str(fixture)], cwd=root,
                                           env=env, stdout=stream, stderr=stream)
                try:
                    result = process.wait(timeout=5)
                    report['startup'] = {'state': 'exited', 'exit_code': result}
                except subprocess.TimeoutExpired:
                    process.terminate()
                    try:
                        process.wait(timeout=10)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
                    report['startup'] = {'state': 'alive after five seconds; diagnostic process terminated'}
            with log.open('rb') as stream:
                text = stream.read(65536).decode('utf-8', errors='replace')
            for path, replacement in [(Path(temporary), '<fixture>'), (root, '<package>'), (Path.home(), '<home>')]:
                text = text.replace(str(path), replacement).replace(path.as_posix(), replacement)
            report['startup']['log_first_64k'] = text
            report['privacy'] = 'Generated empty-root startup only; known home/package/fixture path prefixes redacted. Review log before sharing. No uploads or personal file contents collected.'
    destination = args.output or Path.cwd() / ('file-manager-diagnostics-' + timestamp.strftime('%Y%m%dT%H%M%SZ') + '.json')
    with destination.open('x', encoding='utf-8') as stream:
        json.dump(report, stream, indent=2)
        stream.write('\n')
    print(f'Local report created: {destination}. Review before sharing. Nothing was uploaded.')


if __name__ == '__main__':
    main()
