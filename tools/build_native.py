#!/usr/bin/env python3
"""Build a development distribution using native tools; never install services."""
from __future__ import annotations

from dataclasses import dataclass
import argparse
import json
import os
from pathlib import Path
import platform
import shutil
import sys
from native_build_support import ROOT, BuildValidation, git_output, record_source_state
from native_build_support import run, sdk_fingerprint


@dataclass(frozen=True)
class HostConfiguration:
    host: str
    architecture: str


def host_configuration() -> HostConfiguration:
    systems: dict[str, str] = {'Windows': 'windows', 'Darwin': 'macos', 'Linux': 'linux'}
    architectures: dict[str, str] = {'AMD64': 'x64', 'x86_64': 'x64', 'arm64': 'arm64', 'aarch64': 'arm64'}
    system: str = platform.system()
    machine: str = platform.machine()
    host: str = systems[system]
    architecture: str = architectures[machine]
    result: HostConfiguration = HostConfiguration(host=host, architecture=architecture)
    return result


def build_toolkit(host: str, build: Path, sdk: Path, jobs: int) -> None:
    """Build, test, then install only into the explicitly supplied build SDK."""
    toolkit: Path = build / 'gui-forms'
    options: list[str] = []
    hosts: dict[str, str] = {'WINDOWS': 'windows', 'MACOS': 'macos', 'LINUX': 'linux'}
    name: str
    for name in hosts:
        enabled: str = 'OFF'
        if hosts[name] == host:
            enabled = 'ON'
        options.append(f'-DGUI_FORMS_ENABLE_{name}_HOST={enabled}')
    if host == 'windows':
        options.extend(['-DGUI_FORMS_ENABLE_SKIA=OFF', '-DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=OFF'])
    else:
        if host == 'linux':
            os.environ['GUI_FORMS_SYSTEM_BUILD_TOOLS'] = '1'
        run('sh', ROOT / 'gui_forms/third_party/fetch_skia_cpu.sh')
        run('sh', ROOT / 'gui_forms/third_party/fetch_text_stack.sh')
        skia_out: Path = build / 'skia'
        if host == 'linux':
            run('sh', ROOT / 'gui_forms/third_party/build_skia_cpu_linux.sh', skia_out)
        else:
            skia_out.mkdir(exist_ok=True)
            shutil.copy2(ROOT / 'gui_forms/third_party/skia_cpu_args.gn', skia_out / 'args.gn')
            skia_root: Path = ROOT / 'gui_forms/third_party/skia'
            run(skia_root / 'bin/gn', 'gen', skia_out, '--root=' + str(skia_root))
            run(skia_root / 'third_party/ninja/ninja', '-C', skia_out, '-j', jobs, 'skia')
        options.extend(['-DGUI_FORMS_ENABLE_SKIA=ON', '-DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=ON',
                        '-DGUI_FORMS_SKIA_PREBUILT=ON', f'-DGUI_FORMS_SKIA_OUT={skia_out}'])
    run('cmake', '-S', ROOT / 'gui_forms', '-B', toolkit, '-G', 'Ninja',
        '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_INSTALL_LIBDIR=lib',
        '-DCMAKE_POSITION_INDEPENDENT_CODE=ON', '-DGUI_FORMS_BUILD_GALLERY=OFF',
        '-DGUI_FORMS_BUILD_TESTS=ON', f'-DCMAKE_INSTALL_PREFIX={sdk}', *options)
    run('cmake', '--build', toolkit, '--parallel', jobs)
    os.environ['GUI_FORMS_FONT_DIR'] = str(ROOT / 'gui_forms/assets/fonts')
    run('ctest', '--test-dir', toolkit, '--output-on-failure', '--timeout', '120')
    run('cmake', '--install', toolkit)


def build_components(host: str, build: Path, jobs: int) -> None:
    suffix: str = ''
    if host == 'windows':
        suffix = '.exe'
    services: Path = build / 'components'
    services.mkdir(exist_ok=True)
    engine_executable: Path = services / ('fileman-engine' + suffix)
    run('go', 'build', '-trimpath', '-o', engine_executable,
        './cmd/fileman-engine', cwd=ROOT / 'engine')
    run('cargo', 'build', '--locked', '--release', '--bin', 'orchestrator',
        '--target-dir', build / 'rust', '--jobs', jobs, cwd=ROOT / 'orchestrator')
    orchestrator_executable: Path = build / 'rust/release' / ('orchestrator' + suffix)
    shutil.copy2(orchestrator_executable, services)


def main() -> None:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--gui-forms-sdk', type=Path, help='Reuse an already tested native SDK')
    parser.add_argument('--skip-components', action='store_true', help='Explicit frontend-only development package')
    arguments: argparse.Namespace = parser.parse_args()
    jobs: int = arguments.jobs
    supplied_sdk: Path | None = arguments.gui_forms_sdk
    skip_components: bool = arguments.skip_components
    if jobs < 1:
        parser.error('--jobs must be positive')
    configuration: HostConfiguration = host_configuration()
    host: str = configuration.host
    architecture: str = configuration.architecture
    build: Path = ROOT / '.build' / f'native-{host}-{architecture}'
    build.mkdir(parents=True, exist_ok=True)
    record_source_state(build, 'before-build')
    sdk: Path = build / 'gui-forms-sdk'
    sdk_validation: str = 'native build and CTest passed'
    toolkit_validation: str = 'passed'
    if supplied_sdk is not None:
        sdk = supplied_sdk.resolve()
        sdk_validation = 'externally supplied'
        toolkit_validation = 'externally supplied SDK'
    os.environ['BUILD_JOBS'] = str(jobs)
    os.environ.setdefault('CC', 'clang')
    os.environ.setdefault('CXX', 'clang++')
    if supplied_sdk is None:
        build_toolkit(host, build, sdk, jobs)
    manifest: Path = build / 'gui-forms-consumption.json'
    revision: str = git_output('rev-parse', 'HEAD')
    manifest_record: dict[str, object] = {
        'identity': {'id': f'gui-forms-development-{host}-{architecture}-{revision[:12]}',
                     'state': 'development', 'source_revision': revision},
        'validation': {'sdk': sdk_validation},
        'limits': ['No platform promotion or installed service claim; see package receipt']}
    manifest_text: str = json.dumps(manifest_record, indent=2) + '\n'
    manifest.write_text(manifest_text, encoding='utf-8')
    frontend: Path = build / 'frontend'
    sdk_identity: str = sdk_fingerprint(sdk)
    run('cmake', '-S', ROOT / 'frontend', '-B', frontend, '-G', 'Ninja',
        '-DCMAKE_BUILD_TYPE=Release', f'-DGUIForms_DIR={sdk}/lib/cmake/GUIForms',
        f'-DFILE_MANAGER_GUI_FORMS_MANIFEST={manifest}',
        f'-DCMAKE_INSTALL_PREFIX={build}/frontend-sdk')
    sdk_stamp: Path = frontend / 'sdk-fingerprint.txt'
    previous_identity: str = ''
    if sdk_stamp.is_file():
        previous_text: str = sdk_stamp.read_text(encoding='utf-8')
        previous_identity = previous_text.strip()
    if previous_identity != sdk_identity:
        # Installed header mtimes alone cannot protect a changed public C++ layout.
        run('cmake', '--build', frontend, '--target', 'clean')
    run('cmake', '--build', frontend, '--parallel', jobs)
    run('ctest', '--test-dir', frontend, '--output-on-failure', '--timeout', '120')
    tested_identity: str = sdk_fingerprint(sdk)
    if tested_identity != sdk_identity:
        raise RuntimeError('GUI.Forms SDK changed during frontend build/tests; rebuild against a stable SDK')
    sdk_stamp.write_text(sdk_identity + '\n', encoding='utf-8')
    run('cmake', '--install', frontend)
    if not skip_components:
        build_components(host, build, jobs)
    validation: BuildValidation = {
        'source_revision': revision, 'frontend_ctest': 'passed',
        'gui_forms_sdk_sha256': sdk_identity, 'gui_forms_ctest': toolkit_validation}
    validation_path: Path = build / 'build-validation.json'
    validation_text: str = json.dumps(validation, indent=2) + '\n'
    validation_path.write_text(validation_text, encoding='utf-8')
    package_options: list[str] = []
    if skip_components:
        package_options.append('--skip-components')
    run(sys.executable, ROOT / 'tools/package_native.py', '--build', build,
        '--gui-forms-sdk', sdk, *package_options)


if __name__ == '__main__':
    main()
