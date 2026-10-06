#!/usr/bin/env python3
"""Package and smoke-test native development outputs, preserving dependency notices."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import tempfile
import time
import tarfile
import zipfile
from typing import BinaryIO, TextIO, cast
from build_native import HostConfiguration, host_configuration
from native_build_support import ROOT, BuildValidation, SourceState, HASH_BUFFER_BYTES
from native_build_support import output, run, sdk_fingerprint, record_source_state, sha256, stream_sha256
from native_macos_package import MacRequirements, mac_closure, mac_minimum_versions
from collect_diagnostics import stop_owned_process


def smoke_package(executable: Path, package: Path) -> str:
    """Own one generated-root process; reap it even if verification raises."""
    fixture: str
    with tempfile.TemporaryDirectory(prefix='file-manager-smoke-') as fixture:
        environment: dict[str, str] = os.environ.copy()
        environment.pop('GUI_FORMS_FONT_DIR', None)
        environment.pop('FILE_MANAGER_ROOT', None)
        if platform.system() == 'Windows':
            # A development PATH can hide missing packaged compiler runtime DLLs.
            windows_directory: str = os.environ['SystemRoot']
            system_directory: str = str(Path(windows_directory) / 'System32')
            environment['PATH'] = system_directory + os.pathsep + windows_directory
        start: float = time.monotonic()
        command: list[str] = [str(executable), '--root', fixture]
        process: subprocess.Popen[bytes] = subprocess.Popen(
            command, cwd=package, env=environment,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        try:
            try:
                exit_code: int = process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                return 'process stayed alive for 5 seconds on generated empty root; not GUI workflow acceptance'
            elapsed: float = time.monotonic() - start
            raise RuntimeError(f'Packaged application exited early ({exit_code}) after {elapsed:.2f}s')
        finally:
            stop_owned_process(process)


def package_file_hashes(package: Path) -> dict[str, str]:
    records: dict[str, str] = {}
    paths: list[Path] = sorted(package.rglob('*'))
    path: Path
    for path in paths:
        if path.is_file():
            relative: Path = path.relative_to(package)
            name: str = relative.as_posix()
            records[name] = sha256(path)
    return records


def verify_archive(archive: Path, host: str, files: dict[str, str], executable: str) -> None:
    """Re-read every member using bounded reusable scratch before publication."""
    scratch: bytearray = bytearray(HASH_BUFFER_BYTES)
    relative: str
    member: BinaryIO
    if host == 'windows':
        packed_zip: zipfile.ZipFile
        with zipfile.ZipFile(archive) as packed_zip:
            corrupt_member: str | None = packed_zip.testzip()
            if corrupt_member is not None:
                raise RuntimeError('Archive CRC check failed')
            for relative in files:
                member_name: str = 'FileManager/' + relative
                with packed_zip.open(member_name) as member:
                    actual: str = stream_sha256(member, scratch)
                if actual != files[relative]:
                    raise RuntimeError(f'Archive hash mismatch: {relative}')
    else:
        packed_tar: tarfile.TarFile
        with tarfile.open(archive, 'r:gz') as packed_tar:
            for relative in files:
                member_name = 'FileManager/' + relative
                extracted: BinaryIO | None = packed_tar.extractfile(member_name)
                if extracted is None:
                    raise RuntimeError(f'Archive member missing: {relative}')
                with extracted as member:
                    actual = stream_sha256(member, scratch)
                if actual != files[relative]:
                    raise RuntimeError(f'Archive hash mismatch: {relative}')
            executable_member: tarfile.TarInfo = packed_tar.getmember('FileManager/' + executable)
            if not executable_member.mode & 0o111:
                raise RuntimeError('Archive lost executable permissions')


def main() -> None:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--gui-forms-sdk', type=Path, required=True)
    parser.add_argument('--skip-components', action='store_true')
    args: argparse.Namespace = parser.parse_args()
    build: Path = args.build.resolve()
    sdk: Path = args.gui_forms_sdk.resolve()
    configuration: HostConfiguration = host_configuration()
    host: str = configuration.host
    arch: str = configuration.architecture
    source_state: SourceState = record_source_state(build, 'package')
    revision: str = source_state['source_revision']
    dirty: bool = source_state['source_dirty']
    distribution: Path = build / 'dist'
    distribution.mkdir(exist_ok=True)
    # A new staging folder for every attempt prevents stale files or destructive cleanup.
    stage_name: str = tempfile.mkdtemp(prefix='package-', dir=build)
    stage: Path = Path(stage_name)
    package: Path = stage / 'FileManager'
    package.mkdir()
    frontend: Path = build / 'frontend'
    validation_path: Path = build / 'build-validation.json'
    validation_text: str = validation_path.read_text(encoding='utf-8')
    validation: BuildValidation = cast(BuildValidation, json.loads(validation_text))
    if validation['source_revision'] != revision or validation['frontend_ctest'] != 'passed':
        raise RuntimeError('Build validation is absent or belongs to another revision')
    current_sdk: str = sdk_fingerprint(sdk)
    if current_sdk != validation['gui_forms_sdk_sha256']:
        raise RuntimeError('GUI.Forms SDK changed after frontend compilation; rebuild before packaging')
    macos_requirements: MacRequirements | None = None
    app: Path = package / 'File Manager.app'
    path: Path
    stream: TextIO
    if host == 'macos':
        shutil.copytree(frontend / 'File Manager.app', app, symlinks=False)
        resources: Path = app / 'Contents/Resources'
        resources.mkdir(exist_ok=True)
        executable: Path = app / 'Contents/MacOS/File Manager'
    else:
        resources = package
        executable = package / ('File Manager.exe' if host == 'windows' else 'File Manager')
        shutil.copy2(frontend / executable.name, executable)
        library_pattern: str = '*.so*'
        if host == 'windows':
            library_pattern = '*.dll'
        for path in frontend.glob(library_pattern):
            shutil.copy2(path, package / path.name)
        shutil.copytree(sdk / 'share/GUIForms/fonts', package / 'fonts')
    notices: Path = sdk / 'share/licenses'
    if not notices.is_dir():
        raise RuntimeError('SDK dependency notices are missing')
    shutil.copytree(notices, resources / 'licenses', dirs_exist_ok=True)
    if (frontend / 'licenses').is_dir():
        shutil.copytree(frontend / 'licenses', resources / 'licenses', dirs_exist_ok=True)
    components: list[dict[str, str]] = []
    name: str = ''
    if not args.skip_components:
        suffix: str = '.exe' if host == 'windows' else ''
        component_dir: Path = package / 'components'
        component_dir.mkdir()
        for name in ['fileman-engine', 'orchestrator']:
            source: Path = build / 'components' / (name + suffix)
            if not source.is_file():
                raise RuntimeError(f'Missing component binary: {source}')
            shutil.copy2(source, component_dir)
            components.append({'name': name, 'state': 'bundled, not installed or activated'})
        if host == 'windows':
            shutil.copy2(ROOT / 'tools/launch_windows_search.ps1', package)
            launcher_notes_path: Path = ROOT / 'tools/WINDOWS_SEARCH_LAUNCH.md'
            launcher_notes: str = launcher_notes_path.read_text(encoding='utf-8')
            launcher_notes = launcher_notes.replace(
                '`../orchestrator/conformance/evidence/SHADOW_WINDOWS_PIPES_2026-09-29.md`',
                '[Orchestrator evidence](https://github.com/falseywinchnet/file_manager/blob/' + revision +
                '/orchestrator/conformance/evidence/SHADOW_WINDOWS_PIPES_2026-09-29.md)')
            packaged_notes: Path = package / 'WINDOWS_SEARCH_LAUNCH.md'
            packaged_notes.write_text(launcher_notes, encoding='utf-8')
    if host == 'macos':
        mac_closure(app, sdk)
        if not args.skip_components:
            component: Path
            component_directory: Path = package / 'components'
            for component in component_directory.glob('*'):
                run('codesign', '--force', '--sign', '-', component)
        macos_requirements = mac_minimum_versions(app, package)
    if host == 'linux':
        executable_dependencies: str = output('ldd', executable)
        library_dependencies: str = output('ldd', package / 'libgui_forms_application.so.0')
        dependencies: str = executable_dependencies + '\n' + library_dependencies
        if 'not found' in dependencies:
            raise RuntimeError(dependencies)
        dependency_record: Path = package / 'linux-runtime-dependencies.txt'
        dependency_record.write_text(dependencies + '\n', encoding='utf-8')
    shutil.copy2(ROOT / 'tools/collect_diagnostics.py', package)
    shutil.copy2(build / 'gui-forms-consumption.json', package)
    validation_receipt: dict[str, object] = dict(validation)
    validation_receipt['packaged_startup'] = 'pending'
    relative_executable: Path = executable.relative_to(package)
    receipt_executable: str = relative_executable.as_posix()
    receipt: dict[str, object] = {
        'product': 'File Manager', 'version': '0.001-alpha', 'source_revision': revision,
        'source_dirty': dirty, 'platform': host, 'architecture': arch,
        'source_status_porcelain': source_state['status_porcelain'][:40],
        'source_status_path_count': len(source_state['status_porcelain']),
        'source_status_truncated': len(source_state['status_porcelain']) > 40,
        'build_os': platform.platform(), 'components': components,
        'service_availability': 'Determined by live negotiation; bundling is not activation or readiness',
        'explicit_windows_search_launcher': 'launch_windows_search.ps1' if host == 'windows' and not args.skip_components else None,
        'signature': 'ad-hoc, not Developer ID notarized' if host == 'macos' else 'unsigned',
        'linux_baseline': 'Ubuntu 24.04, X11 or XWayland, system X11/ATK/AT-SPI libraries; xdg-utils for default Open' if host == 'linux' else None,
        'macos_requirements': macos_requirements if host == 'macos' else None,
        'executable': receipt_executable,
        'executable_sha256': sha256(executable),
        'font_rights': 'Portsmouth owner-supplied evaluation fonts; production redistribution-rights gate remains open; attribution retained',
        'validation': validation_receipt,
    }
    readme_path: Path = package / 'README.txt'
    readme_path.write_text(
        'File Manager 0.001-alpha development build\n\n'
        'Extract the complete folder. Open File Manager.app on macOS, File Manager.exe on Windows, '
        'or ./File Manager on Linux. New Folder, same-parent Rename and their one-step Undo '
        'are available without search services or quarantine setup. Copy, Move and Delete '
        'are not yet available in ordinary mode. Pass --read-only for observation-only use.\n'
        'Mac: the bundle is ad-hoc signed and is not notarized. Windows: unsigned development executable.\n'
        'Linux: built on Ubuntu 24.04; requires an X11/XWayland display, system X11/ATK libraries '
        'and xdg-utils for default Open. Terminal Here is unavailable until a terminal contract is configured.\n'
        'Service executables in components are included for independent inspection. Extracting this package '
        'does not install or start services. Search and settings may report unavailable without explicit service activation.\n\n'
        'Diagnostics: python3 collect_diagnostics.py (Windows: python collect_diagnostics.py). '
        'This writes a local JSON report containing build identity, OS/architecture and package hash checks. '
        'No automatic uploads, personal files, environment values or directory listings are collected. '
        'Add --launch to collect a bounded startup log from a new five-second run on a generated empty root. '
        'Known home/package/fixture path prefixes are redacted; review the report before sending it back. '
        'Manually describe what you clicked and what happened.\n',
        encoding='utf-8')
    if host == 'windows' and not args.skip_components:
        with readme_path.open('a', encoding='utf-8') as stream:
            stream.write('\nExplicit Windows search: run .\\launch_windows_search.ps1 -Root "C:\\chosen\\folder" '
                         'from PowerShell in this extracted folder. Read WINDOWS_SEARCH_LAUNCH.md first. '
                         'The root is mandatory; -IndexEnabled separately opts into an initial catalogue scan. '
                         'The launcher owns only its new service processes and stops them when its app closes. '
                         'Private state/logs remain locally for review.\n')
    if host == 'macos':
        with readme_path.open('a', encoding='utf-8') as stream:
            stream.write('\nmacOS load commands/plist require at least ' +
                         macos_requirements['required_minimum_from_load_commands_and_plist'] +
                         '. Startup was tested on macOS ' + macos_requirements['tested_host_macos'] +
                         '. Compatibility with other OS versions has not been tested.\n')
    startup_status: str = smoke_package(executable, package)
    validation_receipt['packaged_startup'] = startup_status
    file_hashes: dict[str, str] = package_file_hashes(package)
    receipt['files'] = file_hashes
    final_sdk_identity: str = sdk_fingerprint(sdk)
    if final_sdk_identity != validation['gui_forms_sdk_sha256']:
        raise RuntimeError('GUI.Forms SDK changed during package verification; discard this staging attempt')
    receipt_path: Path = package / 'build-receipt.json'
    check_path: Path = build / 'package-check.json'
    receipt_text: str = json.dumps(receipt, indent=2) + '\n'
    receipt_path.write_text(receipt_text, encoding='utf-8')
    check_path.write_text(receipt_text, encoding='utf-8')
    name = f'file-manager-0.001-alpha-{host}-{arch}-{revision[:12]}'
    if dirty:
        name += '-dirty'
    archive_format: str = 'gztar'
    if host == 'windows':
        archive_format = 'zip'
    archive_base: Path = distribution / name
    archive_name: str = shutil.make_archive(str(archive_base), archive_format, stage, package.name)
    archive: Path = Path(archive_name)
    verify_archive(archive, host, file_hashes, receipt_executable)
    archive_hash: str = sha256(archive)
    checksum_path: Path = distribution / (archive.name + '.sha256')
    checksum_text: str = archive_hash + '  ' + archive.name + '\n'
    checksum_path.write_text(checksum_text, encoding='utf-8')
    print(archive)


if __name__ == '__main__':
    main()
