#!/usr/bin/env python3
"""Capture a bounded provider-owned diagnostic; never certify SDK performance."""
from __future__ import annotations

import json
from pathlib import Path
import platform
import subprocess
import time
from typing import TextIO

from native_build_support import ROOT, git_output


def main() -> int:
    if platform.system() != 'Darwin':
        raise RuntimeError('The native paint-cost experiment requires macOS')
    build: Path = ROOT / '.build/native-macos-arm64'
    evidence: Path = build / 'paint-cost'
    evidence.mkdir(parents=True, exist_ok=True)
    executable: Path = build / 'gui-forms/gui_forms_macos_idle_visibility_tests.app/Contents/MacOS/gui_forms_macos_idle_visibility_tests'
    command: list[str] = [str(executable), '--paint-cost-experiment']
    log_path: Path = evidence / 'experiment.log'
    status: str = 'failed'
    exit_code: int | None = None
    detail: str = ''
    started: float = time.monotonic()
    log_stream: TextIO
    with log_path.open('w', encoding='utf-8') as log_stream:
        try:
            result: subprocess.CompletedProcess[str] = subprocess.run(
                command, cwd=ROOT, stdout=log_stream, stderr=subprocess.STDOUT,
                text=True, timeout=90, check=False)
            exit_code = result.returncode
            status = 'rejected'
        except subprocess.TimeoutExpired:
            status = 'timeout'
            detail = 'Experiment exceeded the 90-second process bound'
        except OSError as error:
            detail = str(error)
    elapsed: float = time.monotonic() - started
    output: str = log_path.read_text(encoding='utf-8', errors='replace')
    accepted_marker: bool = 'cost-experiment=accepted-comparable-intervals|' in output
    if exit_code == 0 and accepted_marker:
        status = 'accepted'
    if status == 'rejected':
        line: str
        for line in output.splitlines():
            if line.startswith('cost-experiment-rejected:'):
                detail = line
                break
    receipt: dict[str, object] = {
        'source_revision': git_output('rev-parse', 'HEAD'),
        'command': command, 'status': status, 'exit_code': exit_code,
        'wall_seconds': elapsed, 'process_timeout_seconds': 90,
        'accepted_marker': accepted_marker, 'detail': detail,
        'limits': 'Static unfocused surrogate; not consumer idle CPU or pixel-equivalence proof',
    }
    receipt_text: str = json.dumps(receipt, indent=2) + '\n'
    receipt_path: Path = evidence / 'receipt.json'
    receipt_path.write_text(receipt_text, encoding='utf-8')
    print(receipt_text, end='')
    if status != 'accepted':
        return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
