#!/usr/bin/env python3
"""Four bounded profiling-only holds; retain raw evidence without accepting a comparison."""
from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import hashlib
import json
import os
from pathlib import Path
import platform
import signal
import subprocess
import time
from typing import BinaryIO
import uuid
import xml.etree.ElementTree as ET

from native_build_support import Digest, ROOT

RUN_LIMIT: float = 180.0
OVERALL_LIMIT: float = 1080.0
READ_LIMIT: int = 16 * 1024 * 1024
INSPECTABILITY_SAMPLES: int = 200


@dataclass
class CommandResult:
    command: list[str]
    status: str
    exit_code: int | None
    seconds: float
    stdout: str
    stderr: str
    detail: str = ''


class OwnedProcess:
    """Own a new session/process group and its log streams; never signal another group."""
    def __init__(self, command: list[str], directory: Path, name: str,
                 environment: dict[str, str] | None = None) -> None:
        self.command: list[str] = list(command)
        self.stdout_path: Path = directory / (name + '.stdout.log')
        self.stderr_path: Path = directory / (name + '.stderr.log')
        self.stdout: BinaryIO = self.stdout_path.open('wb')
        self.stderr: BinaryIO
        try:
            self.stderr = self.stderr_path.open('wb')
        except OSError:
            self.stdout.close()
            raise
        self.started: float = time.monotonic()
        self.process: subprocess.Popen[bytes]
        try:
            self.process = subprocess.Popen(command, cwd=ROOT, env=environment,
                stdin=subprocess.DEVNULL, stdout=self.stdout, stderr=self.stderr,
                start_new_session=True)
        except OSError:
            self.stdout.close()
            self.stderr.close()
            raise

    def stop(self) -> None:
        # This runner operates on Darwin; each Popen above created this group.
        # A reaped leader is not signalled, avoiding a recycled PID/group identity.
        if self.process.poll() is not None:
            return
        try:
            os.killpg(self.process.pid, signal.SIGTERM)
        except ProcessLookupError:
            pass
        try:
            self.process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            try:
                os.killpg(self.process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            self.process.wait(timeout=3)

    def close(self) -> None:
        try:
            self.stop()
        finally:
            self.stdout.close()
            self.stderr.close()

    def result(self, status: str, detail: str = '') -> CommandResult:
        elapsed: float = time.monotonic() - self.started
        result: CommandResult = CommandResult(self.command, status, self.process.poll(),
            elapsed, str(self.stdout_path), str(self.stderr_path), detail)
        return result


def write_json(path: Path, value: object) -> None:
    text: str = json.dumps(value, indent=2) + '\n'
    path.write_text(text, encoding='utf-8')


def read_small(path: Path) -> str:
    if path.stat().st_size > READ_LIMIT:
        raise ValueError('diagnostic text exceeds 16 MiB read bound: ' + str(path))
    result: str = path.read_text(encoding='utf-8', errors='replace')
    return result


def file_hash(path: Path) -> str:
    digest: Digest = hashlib.sha256()
    storage: bytearray = bytearray(1024 * 1024)
    view: memoryview = memoryview(storage)
    stream: BinaryIO
    with path.open('rb') as stream:
        while True:
            count: int = stream.readinto(storage)
            if count == 0:
                break
            digest.update(view[:count])
    result: str = digest.hexdigest()
    return result


def run_command(command: list[str], directory: Path, name: str,
                seconds: float, deadline: float) -> CommandResult:
    available: float = min(seconds, deadline - time.monotonic())
    if available <= 0:
        return CommandResult(command, 'budget_exhausted', None, 0.0, '', '')
    owner: OwnedProcess | None = None
    try:
        owner = OwnedProcess(command, directory, name)
        try:
            exit_code: int = owner.process.wait(timeout=available)
            status: str = 'completed' if exit_code == 0 else 'failed'
            return owner.result(status)
        except subprocess.TimeoutExpired:
            owner.stop()
            return owner.result('timeout')
    except OSError as error:
        return CommandResult(command, 'unavailable', None, 0.0, '', '', str(error))
    finally:
        if owner is not None:
            owner.close()


def parse_fields(line: str) -> dict[str, str]:
    fields: dict[str, str] = {}
    part: str
    for part in line.split('|')[1:]:
        key: str
        separator: str
        value: str
        key, separator, value = part.partition('=')
        if separator:
            fields[key] = value
    return fields


def summarize_workload(text: str, mode: str, exit_code: int | None) -> dict[str, object]:
    results: list[dict[str, str]] = []
    spans: list[dict[str, str]] = []
    clocks: dict[str, dict[str, str]] = {}
    windows: set[tuple[str, str]] = set()
    line: str
    for line in text.splitlines():
        fields: dict[str, str] = parse_fields(line)
        if line.startswith('cpu-profile-result|'):
            results.append(fields)
        elif line.startswith('cpu-profile-span|'):
            spans.append(fields)
        elif line.startswith('cpu-profile-clock|'):
            name: str = fields.get('name', '')
            if name in clocks:
                return {'status': 'invalid', 'detail': 'duplicate clock record'}
            clocks[name] = fields
        elif line.startswith('cpu-profile-window|'):
            window: tuple[str, str] = (fields.get('boundary', ''), fields.get('window', ''))
            if window in windows:
                return {'status': 'invalid', 'detail': 'duplicate window record'}
            windows.add(window)
    if len(results) != 1 or len(spans) != 1:
        return {'status': 'invalid', 'detail': 'missing or duplicate result/span', 'results': results}
    result: dict[str, str] = results[0]
    span: dict[str, str] = spans[0]
    valid: bool = exit_code == 0 and result.get('status') == 'completed'
    valid = valid and result.get('mode') == mode and result.get('closed') == '9'
    valid = valid and span.get('mode') == mode and span.get('hold_finished') == '1'
    valid = valid and span.get('diagnostic_scalar_and_trace') == 'off'
    valid = valid and result.get('comparison_accepted') == 'false'
    valid = valid and span.get('comparison_accepted') == 'false' and len(windows) == 18
    boundary: str
    for boundary in ('before', 'after'):
        index: int
        for index in range(9):
            valid = valid and (boundary, str(index)) in windows
    name: str
    for name in ('process_start', 'thread_start', 'thread_end', 'process_end'):
        valid = valid and clocks.get(name, {}).get('valid') == '1'
    summary: dict[str, object] = {'status': 'completed' if valid else 'invalid',
        'result': result, 'span': span, 'clocks': clocks, 'window_records': len(windows)}
    if valid:
        try:
            wall: float = float(span['wall_seconds'])
            process_ns: int = int(clocks['process_end']['ns']) - int(clocks['process_start']['ns'])
            main_ns: int = int(clocks['thread_end']['ns']) - int(clocks['thread_start']['ns'])
            if not 120.0 <= wall <= 130.0 or process_ns < 0 or main_ns < 0:
                raise ValueError('invalid clocks or duration')
            summary['process_cpu_ns'] = process_ns
            summary['main_cpu_ns'] = main_ns
            summary['process_one_core_percent'] = process_ns / 1.0e7 / wall
            summary['main_one_core_percent'] = main_ns / 1.0e7 / wall
        except (KeyError, ValueError) as error:
            summary['status'] = 'invalid'
            summary['detail'] = str(error)
    return summary


def analysis_state(mapped_span: bool | None, executing_main_samples: int | None) -> str:
    # Unknown schema/counts are NOT zero samples. This is an inspectability guard,
    # never a confidence level or a performance acceptance criterion.
    if mapped_span is False:
        return 'unmapped'
    if mapped_span is None or executing_main_samples is None:
        return 'analysis_pending'
    if executing_main_samples < INSPECTABILITY_SAMPLES:
        return 'too_sparse'
    return 'analysis_pending'


def preflight(directory: Path, deadline: float, template_override: str | None) -> dict[str, object]:
    commands: list[tuple[str, list[str]]] = [
        ('revision', ['git', 'rev-parse', 'HEAD']),
        ('source-status', ['git', 'status', '--porcelain=v1']),
        ('os', ['/usr/bin/sw_vers']), ('architecture', ['/usr/bin/uname', '-m']),
        ('hardware', ['/usr/sbin/sysctl', 'hw.model', 'machdep.cpu.brand_string']),
        ('developer', ['/usr/bin/xcode-select', '-p']),
        ('xcode', ['/usr/bin/xcodebuild', '-version']),
        ('tool', ['/usr/bin/xcrun', '--find', 'xctrace']),
        ('version', ['/usr/bin/xcrun', 'xctrace', 'version']),
        ('templates', ['/usr/bin/xcrun', 'xctrace', 'list', 'templates']),
        ('record-help', ['/usr/bin/xcrun', 'xctrace', 'help', 'record']),
        ('export-help', ['/usr/bin/xcrun', 'xctrace', 'help', 'export']),
    ]
    results: dict[str, dict[str, object]] = {}
    texts: dict[str, str] = {}
    item: tuple[str, list[str]]
    for item in commands:
        name: str = item[0]
        outcome: CommandResult = run_command(item[1], directory, name, 10.0, deadline)
        results[name] = asdict(outcome)
        texts[name] = ''
        if outcome.stdout:
            texts[name] = read_small(Path(outcome.stdout))
            texts[name] += read_small(Path(outcome.stderr))
    result: dict[str, object] = {'status': 'unavailable', 'commands': results,
        'DEVELOPER_DIR': os.environ.get('DEVELOPER_DIR'), 'template': None}
    if results['tool']['status'] != 'completed' or results['templates']['status'] != 'completed':
        result['detail'] = 'xctrace discovery or template listing failed'
        return result
    required: str
    for required in ('--template', '--attach', '--output', '--time-limit', '--no-prompt'):
        if required not in texts['record-help']:
            result['detail'] = 'record help does not advertise ' + required
            return result
    for required in ('--input', '--output', '--toc'):
        if required not in texts['export-help']:
            result['detail'] = 'export help does not advertise ' + required
            return result
    template: str | None = None
    template_names: set[str] = set()
    template_line: str
    for template_line in texts['templates'].splitlines():
        template_names.add(template_line.strip())
    if template_override is not None:
        candidate: Path = Path(template_override)
        if candidate.is_file():
            template = str(candidate.resolve())
            result['template_sha256'] = file_hash(candidate)
        elif template_override in template_names:
            template = template_override
    elif 'CPU Profiler' in template_names:
        template = 'CPU Profiler'
    elif 'Time Profiler' in template_names:
        template = 'Time Profiler'
    if template is None:
        result['detail'] = 'requested or supported CPU template unavailable'
        return result
    result['status'] = 'discovered'
    result['template'] = template
    result['detail'] = 'Discovery does not establish recording permissions, counters, or signpost capture'
    return result


def export_trace(trace: Path, directory: Path, deadline: float) -> dict[str, object]:
    toc: Path = directory / 'toc.xml'
    command: list[str] = ['/usr/bin/xcrun', 'xctrace', 'export', '--input', str(trace),
        '--toc', '--output', str(toc)]
    outcome: CommandResult = run_command(command, directory, 'export-toc', 60.0, deadline)
    result: dict[str, object] = {'status': 'export_failed', 'command': asdict(outcome)}
    if outcome.status != 'completed' or not toc.is_file():
        return result
    try:
        text: str = read_small(toc)
        root: ET.Element = ET.fromstring(text)
        tables: list[dict[str, str]] = []
        element: ET.Element
        for element in root.iter('table'):
            tables.append(dict(element.attrib))
        result['status'] = analysis_state(None, None)
        result['observed_tables'] = tables
        result['detail'] = 'TOC retained; full trace is raw evidence. No schema-specific sample or span inference.'
    except (ValueError, ET.ParseError, OSError) as error:
        result['detail'] = str(error)
    return result


def execute_hold(executable: Path, expected_hash: str, mode: str, profiled: bool,
                 template: str | None, directory: Path, deadline: float) -> dict[str, object]:
    directory.mkdir()
    result: dict[str, object] = {'mode': mode, 'profiled': profiled,
        'status': 'unavailable', 'analysis': 'unavailable', 'comparison_accepted': False,
        'executable_sha256': expected_hash}
    actual_hash: str = file_hash(executable)
    if actual_hash != expected_hash:
        result['status'] = 'binary_changed'
        return result
    if profiled and template is None:
        result['detail'] = 'No usable profiling template discovered; no substitute unprofiled run'
        return result
    environment: dict[str, str] = dict(os.environ)
    environment['GUI_FORMS_CPU_PROFILE_EXPECT_RECORDER'] = '1' if profiled else '0'
    application: OwnedProcess | None = None
    recorder: OwnedProcess | None = None
    limit: float = min(deadline, time.monotonic() + RUN_LIMIT)
    trace: Path = directory / 'recording.trace'
    try:
        application = OwnedProcess([str(executable), '--cpu-profile-hold', mode],
            directory, 'application', environment)
        if profiled:
            command: list[str] = ['/usr/bin/xcrun', 'xctrace', 'record', '--template', str(template),
                '--attach', str(application.process.pid), '--output', str(trace),
                '--time-limit', '150s', '--no-prompt']
            recorder = OwnedProcess(command, directory, 'recorder')
        outcome: str = 'completed'
        while application.process.poll() is None:
            if time.monotonic() >= limit:
                outcome = 'timeout'
                break
            if recorder is not None and recorder.process.poll() is not None:
                outcome = 'recorder_ended_before_workload'
                break
            time.sleep(0.2)
        if outcome != 'completed':
            application.stop()
        if recorder is not None:
            available: float = max(0.0, limit - time.monotonic())
            try:
                recorder.process.wait(timeout=available)
            except subprocess.TimeoutExpired:
                recorder.stop()
                outcome = 'timeout'
            result['recorder'] = asdict(recorder.result(outcome))
        result['application'] = asdict(application.result(outcome))
        output: str = read_small(application.stdout_path)
        output += read_small(application.stderr_path)
        workload: dict[str, object] = summarize_workload(output, mode, application.process.poll())
        result['workload'] = workload
        result['status'] = outcome if outcome != 'completed' else workload['status']
        if recorder is not None and recorder.process.returncode != 0:
            result['status'] = 'capture_failed'
        result['analysis'] = 'not_profiled'
        if profiled:
            result['analysis'] = 'analysis_pending'
            if trace.is_dir():
                exported: dict[str, object] = export_trace(trace, directory, deadline)
                result['export'] = exported
                result['analysis'] = exported['status']
            else:
                result['analysis'] = 'unavailable'
            if '|signposts_enabled_at_begin=0|' in output or 'readiness_or_recorder_unmapped' in output:
                result['analysis'] = analysis_state(False, None)
        result['executable_sha256_after'] = file_hash(executable)
        if result['executable_sha256_after'] != expected_hash:
            result['status'] = 'binary_changed'
    except (OSError, ValueError, subprocess.TimeoutExpired) as error:
        result['detail'] = str(error)
        result['status'] = 'unavailable'
    finally:
        try:
            if recorder is not None:
                recorder.close()
        finally:
            if application is not None:
                application.close()
    return result


def main() -> int:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, default=ROOT / '.build/native-macos-arm64/cpu-profile-build')
    parser.add_argument('--executable', type=Path)
    parser.add_argument('--evidence-dir', type=Path, default=ROOT / '.build/native-macos-arm64/cpu-profile')
    parser.add_argument('--template', help='Exact discovered template name or an existing custom template file')
    arguments: argparse.Namespace = parser.parse_args()
    build: Path = arguments.build_dir.resolve()
    executable: Path = arguments.executable or build / 'gui_forms_macos_idle_visibility_tests.app/Contents/MacOS/gui_forms_macos_idle_visibility_tests'
    executable = executable.resolve()
    directory: Path = arguments.evidence_dir.resolve() / ('attempt-' + uuid.uuid4().hex)
    directory.mkdir(parents=True, exist_ok=False)
    deadline: float = time.monotonic() + OVERALL_LIMIT
    receipt: dict[str, object] = {'status': 'unavailable', 'executable': str(executable),
        'build_directory': str(build), 'evidence_directory': str(directory),
        'comparison_accepted': False, 'overall_timeout_seconds': OVERALL_LIMIT,
        'run_timeout_seconds': RUN_LIMIT, 'record_time_limit_seconds': 150,
        'inspectability_sample_guard': INSPECTABILITY_SAMPLES,
        'sample_guard_is_statistical_confidence': False,
        'sample_analysis': 'analysis_pending', 'runs': [],
        'limits': 'No OS-versus-queue, CA residual, or user-workload CPU attribution without executing stack evidence'}
    try:
        if platform.system() != 'Darwin':
            receipt['detail'] = 'macOS required; no profiling processes started'
            return 1
        if not executable.is_file():
            receipt['detail'] = 'fixture executable missing'
            return 1
        expected_hash: str = file_hash(executable)
        receipt['executable_sha256'] = expected_hash
        environment_directory: Path = directory / 'preflight'
        environment_directory.mkdir()
        checks: dict[str, object] = preflight(environment_directory,
            min(deadline, time.monotonic() + 60.0), arguments.template)
        receipt['preflight'] = checks
        write_json(directory / 'preflight.json', checks)
        template_value: object = checks.get('template')
        template: str | None = template_value if isinstance(template_value, str) else None
        cases: tuple[tuple[str, str, bool], ...] = (
            ('01-unprofiled-focused', 'focused', False),
            ('02-profiled-focused', 'focused', True),
            ('03-profiled-cleared', 'cleared', True),
            ('04-unprofiled-cleared', 'cleared', False))
        runs: list[dict[str, object]] = []
        case: tuple[str, str, bool]
        for case in cases:
            if time.monotonic() >= deadline:
                result: dict[str, object] = {'status': 'budget_exhausted', 'mode': case[1], 'profiled': case[2]}
            else:
                result = execute_hold(executable, expected_hash, case[1], case[2], template,
                    directory / case[0], deadline)
            runs.append(result)
            write_json(directory / (case[0] + '.json'), result)
            receipt['runs'] = runs
            write_json(directory / 'receipt.json', receipt)
        completed: bool = True
        run: dict[str, object]
        for run in runs:
            completed = completed and run.get('status') == 'completed'
        receipt['status'] = 'workloads_completed_analysis_pending' if completed else 'incomplete'
        return 0 if completed else 1
    except (OSError, ValueError) as error:
        receipt['detail'] = str(error)
        return 1
    finally:
        write_json(directory / 'receipt.json', receipt)
        print(json.dumps(receipt, indent=2), flush=True)


if __name__ == '__main__':
    raise SystemExit(main())
