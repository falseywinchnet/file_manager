"""Export matching installed SDKs after the native build's validation gates.

Each invocation owns a new staging directory. Existing SDKs are never modified.
The archive is published only after relocated consumer configure/link checks.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import shutil
import tempfile

from build_native import HostConfiguration, host_configuration
from native_build_support import ROOT, git_output, run, sdk_fingerprint, sha256


def require_validation(build: Path, revision: str) -> None:
    receipt: dict[str, object] = json.loads(
        (build / 'build-validation.json').read_text(encoding='utf-8'))
    if receipt.get('source_revision') != revision:
        raise RuntimeError('Native build revision does not match this checkout')
    if receipt.get('frontend_ctest') != 'passed' or receipt.get('gui_forms_ctest') != 'passed':
        raise RuntimeError('Both SDKs must be built and tested together')
    fingerprint: str = sdk_fingerprint(build / 'gui-forms-sdk')
    if receipt.get('gui_forms_sdk_sha256') != fingerprint:
        raise RuntimeError('Validated GUI.Forms SDK changed after testing')


def cache_options(build: Path) -> dict[str, str]:
    options: dict[str, str] = {}
    line: str
    for line in (build / 'gui-forms' / 'CMakeCache.txt').read_text(encoding='utf-8').splitlines():
        if line.startswith('GUI_FORMS_ENABLE_') or line.startswith('CMAKE_BUILD_TYPE:'):
            name: str
            value: str
            name, value = line.split('=', 1)
            options[name] = value
    return options


def verify_consumers(bundle: Path, scratch: Path) -> None:
    # Use copied, independent projects: no add_subdirectory into provider source.
    picker_source: Path = scratch / 'picker-consumer'
    shutil.copytree(ROOT / 'frontend/examples/document_picker_consumer', picker_source)
    prefix: str = str(bundle / 'gui-forms-sdk') + ';' + str(bundle / 'picker-sdk')
    run('cmake', '-S', picker_source, '-B', scratch / 'picker-build', '-G', 'Ninja',
        '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON',
        '-DCMAKE_PREFIX_PATH=' + prefix)
    run('cmake', '--build', scratch / 'picker-build', '--parallel', 2)
    # Native Application verification links the host library but does not launch UI.
    application_source: Path = scratch / 'application-consumer'
    shutil.copytree(ROOT / 'gui_forms/examples/paint_contract', application_source)
    run('cmake', '-S', application_source, '-B', scratch / 'application-build', '-G', 'Ninja',
        '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON',
        '-DGUI_FORMS_EXAMPLE_NATIVE=ON',
        '-DCMAKE_PREFIX_PATH=' + prefix)
    run('cmake', '--build', scratch / 'application-build', '--parallel', 2)


def export(build: Path, platform: str, revision: str) -> Path:
    require_validation(build, revision)
    output: Path = build / 'sdk-dist'
    output.mkdir(exist_ok=True)
    archive_name: str = 'gui-forms-picker-' + platform + '-' + revision
    archive: Path = output / (archive_name + '.zip')
    if archive.exists():
        raise RuntimeError('Refusing to overwrite an existing SDK archive')
    scratch_name: str
    with tempfile.TemporaryDirectory(prefix='sdk-export-', dir=build) as scratch_name:
        scratch: Path = Path(scratch_name)
        bundle: Path = scratch / archive_name
        gui: Path = bundle / 'gui-forms-sdk'
        picker: Path = bundle / 'picker-sdk'
        run('cmake', '--install', build / 'gui-forms', '--prefix', gui)
        run('cmake', '--install', build / 'frontend', '--prefix', picker,
            '--component', 'DocumentPicker')
        licenses: Path = picker / 'share/licenses/FileManagerDocumentPicker'
        licenses.mkdir(parents=True)
        shutil.copy2(ROOT / 'LICENSE', licenses / 'LICENSE')
        verify_consumers(bundle, scratch)
        manifest: dict[str, object] = {
            'schema': 1, 'provider_revision': revision, 'platform': platform,
            'gui_forms_source_revision': git_output('-C', str(ROOT / 'gui_forms'), 'rev-parse', 'HEAD'),
            'backend_source_revision': git_output('-C', str(ROOT / 'backend'), 'rev-parse', 'HEAD'),
            'build_options': cache_options(build),
            'gui_forms_sha256': sdk_fingerprint(gui),
            'picker_sha256': sdk_fingerprint(picker),
            'validation': 'relocated independent Application and picker configure/link passed',
            'prepared_text_availability': 'not asserted by this packaging receipt'}
        (bundle / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
        temporary_archive: str = shutil.make_archive(str(scratch / 'verified-sdk'), 'zip', bundle)
        shutil.move(temporary_archive, archive)
    digest: str = sha256(archive)
    archive.with_suffix('.zip.sha256').write_text(digest + '  ' + archive.name + '\n', encoding='utf-8')
    return archive


def main() -> None:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    if git_output('status', '--porcelain'):
        raise RuntimeError('SDK export requires a clean provider checkout')
    configuration: HostConfiguration = host_configuration()
    platform: str = configuration.host + '-' + configuration.architecture
    revision: str = git_output('rev-parse', 'HEAD')
    build: Path = ROOT / '.build' / ('native-' + platform)
    archive: Path = export(build, platform, revision)
    print('Verified SDK archive: ' + str(archive))


if __name__ == '__main__':
    main()
