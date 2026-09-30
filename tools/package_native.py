#!/usr/bin/env python3
"""Package and smoke-test native development outputs, preserving dependency notices."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def output(*args):
    return subprocess.check_output(list(map(str, args)), text=True).strip()


def sha256(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def mac_closure(app, sdk):
    """Resolve source load commands, rewrite private libraries, then sign the result."""
    executable = app / 'Contents/MacOS/File Manager'
    frameworks = app / 'Contents/Frameworks'
    frameworks.mkdir(exist_ok=True)

    def deps(path):
        return [line.strip().split(' (', 1)[0] for line in output('otool', '-L', path).splitlines()[1:]]

    def rpaths(path):
        return re.findall(r'cmd LC_RPATH\n\s+cmdsize \d+\n\s+path (.*?) \(offset', output('otool', '-l', path))

    def expand(value, owner):
        return Path(value.replace('@loader_path', str(owner.parent)).replace('@executable_path', str(executable.parent)))

    queue = [(executable, executable)]
    seen = set()
    names = {}
    while queue:
        original, target = queue.pop(0)
        if target in seen:
            continue
        seen.add(target)
        ids = output('otool', '-D', original).splitlines()[1:]
        for dependency in deps(original):
            if dependency in ids or dependency.startswith(('/System/', '/usr/lib/')):
                continue
            name = Path(dependency).name
            if dependency.startswith('@rpath/'):
                bases = rpaths(original) + rpaths(executable)
                candidates = [expand(base, original) / dependency[7:] for base in bases]
                candidates += [sdk / 'lib' / name, frameworks / name]
            else:
                candidates = [expand(dependency, original)]
            source = next((path.resolve() for path in candidates if path.is_file()), None)
            if source is None:
                raise RuntimeError(f'Unresolved Mach-O dependency {dependency} in {original}')
            if name in names and sha256(source) != names[name]:
                raise RuntimeError(f'Conflicting dependency basename: {name}')
            destination = frameworks / name
            if name not in names:
                names[name] = sha256(source)
                if source != destination.resolve():
                    shutil.copy2(source, destination)
                destination.chmod(0o755)
                queue.append((source, destination))
            replacement = ('@executable_path/../Frameworks/' if target == executable else '@loader_path/') + name
            subprocess.run(['install_name_tool', '-change', dependency, replacement, str(target)], check=True)
        for path in rpaths(target):
            if path.startswith('/'):
                subprocess.run(['install_name_tool', '-delete_rpath', path, str(target)], check=True)
    for target in seen:
        ids = output('otool', '-D', target).splitlines()[1:]
        for dependency in deps(target):
            if dependency in ids or dependency.startswith(('/System/', '/usr/lib/')):
                continue
            resolved = expand(dependency, target)
            if not dependency.startswith(('@loader_path/', '@executable_path/')) or not resolved.is_file():
                raise RuntimeError(f'Nonportable dependency remains: {dependency}')
    for library in frameworks.iterdir():
        subprocess.run(['codesign', '--force', '--sign', '-', str(library)], check=True)
    subprocess.run(['codesign', '--force', '--deep', '--sign', '-', str(app)], check=True)
    subprocess.run(['codesign', '--verify', '--deep', '--strict', str(app)], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--gui-forms-sdk', type=Path, required=True)
    parser.add_argument('--skip-components', action='store_true')
    args = parser.parse_args()
    build, sdk = args.build.resolve(), args.gui_forms_sdk.resolve()
    host = {'Windows': 'windows', 'Darwin': 'macos', 'Linux': 'linux'}[platform.system()]
    arch = {'AMD64': 'x64', 'x86_64': 'x64', 'arm64': 'arm64', 'aarch64': 'arm64'}[platform.machine()]
    revision = output('git', '-C', ROOT, 'rev-parse', 'HEAD')
    dirty = bool(output('git', '-C', ROOT, 'status', '--porcelain', '--untracked-files=normal'))
    distribution = build / 'dist'
    distribution.mkdir(exist_ok=True)
    # A new staging folder for every attempt prevents stale files or destructive cleanup.
    stage = Path(tempfile.mkdtemp(prefix='package-', dir=build))
    package = stage / 'FileManager'
    package.mkdir()
    frontend = build / 'frontend'
    validation = json.loads((build / 'build-validation.json').read_text(encoding='utf-8'))
    if validation['source_revision'] != revision or validation['frontend_ctest'] != 'passed':
        raise RuntimeError('Build validation is absent or belongs to another revision')
    if host == 'macos':
        app = package / 'File Manager.app'
        shutil.copytree(frontend / 'File Manager.app', app, symlinks=False)
        resources = app / 'Contents/Resources'
        resources.mkdir(exist_ok=True)
        executable = app / 'Contents/MacOS/File Manager'
    else:
        resources = package
        executable = package / ('File Manager.exe' if host == 'windows' else 'File Manager')
        shutil.copy2(frontend / executable.name, executable)
        for path in frontend.glob('*.dll' if host == 'windows' else '*.so*'):
            shutil.copy2(path, package / path.name)
        shutil.copytree(sdk / 'share/GUIForms/fonts', package / 'fonts')
    notices = sdk / 'share/licenses'
    if not notices.is_dir():
        raise RuntimeError('SDK dependency notices are missing')
    shutil.copytree(notices, resources / 'licenses', dirs_exist_ok=True)
    if (frontend / 'licenses').is_dir():
        shutil.copytree(frontend / 'licenses', resources / 'licenses', dirs_exist_ok=True)
    components = []
    if not args.skip_components:
        suffix = '.exe' if host == 'windows' else ''
        component_dir = package / 'components'
        component_dir.mkdir()
        for name in ['fileman-engine', 'orchestrator']:
            source = build / 'components' / (name + suffix)
            if not source.is_file():
                raise RuntimeError(f'Missing component binary: {source}')
            shutil.copy2(source, component_dir)
            components.append({'name': name, 'state': 'bundled, not installed or activated'})
    if host == 'macos':
        mac_closure(app, sdk)
        for component in (package / 'components').glob('*') if not args.skip_components else []:
            subprocess.run(['codesign', '--force', '--sign', '-', str(component)], check=True)
    if host == 'linux':
        dependencies = output('ldd', executable) + '\n' + output('ldd', package / 'libgui_forms_application.so.0')
        if 'not found' in dependencies:
            raise RuntimeError(dependencies)
        (package / 'linux-runtime-dependencies.txt').write_text(dependencies + '\n', encoding='utf-8')
    shutil.copy2(ROOT / 'tools/collect_diagnostics.py', package)
    shutil.copy2(build / 'gui-forms-consumption.json', package)
    receipt = {
        'product': 'File Manager', 'version': '0.001-alpha', 'source_revision': revision,
        'source_dirty': dirty, 'platform': host, 'architecture': arch,
        'build_os': platform.platform(), 'components': components,
        'service_availability': 'Determined by live negotiation; bundling is not activation or readiness',
        'signature': 'ad-hoc, not Developer ID notarized' if host == 'macos' else 'unsigned',
        'linux_baseline': 'Ubuntu 24.04, X11 or XWayland, system GTK accessibility/X11 libraries' if host == 'linux' else None,
        'executable': executable.relative_to(package).as_posix(),
        'executable_sha256': sha256(executable),
        'font_rights': 'Portsmouth owner-supplied evaluation fonts; production redistribution-rights gate remains open; attribution retained',
        'validation': {**validation, 'packaged_startup': 'pending'},
    }
    (package / 'README.txt').write_text(
        'File Manager 0.001-alpha development build\n\n'
        'Extract the complete folder. Open File Manager.app on macOS, File Manager.exe on Windows, '
        'or ./File Manager on Linux. Read-only browsing is the default.\n'
        'Mac: the bundle is ad-hoc signed and is not notarized. Windows: unsigned development executable.\n'
        'Linux: built on Ubuntu 24.04; requires an X11/XWayland display and system X11/ATK libraries.\n'
        'Service executables in components are included for independent inspection. This package does not '
        'install, activate or configure services. Search and settings may report unavailable.\n\n'
        'Diagnostics: python3 collect_diagnostics.py (Windows: python collect_diagnostics.py). '
        'This writes a local JSON report containing build identity, OS/architecture and package hash checks. '
        'No automatic uploads, personal files, environment values or directory listings are collected. '
        'Add --launch to collect a bounded startup log from a new five-second run on a generated empty root. '
        'Known home/package/fixture path prefixes are redacted; review the report before sending it back. '
        'Manually describe what you clicked and what happened.\n',
        encoding='utf-8')
    # Use an empty generated root and a clean font environment, never a personal directory.
    with tempfile.TemporaryDirectory(prefix='file-manager-smoke-') as fixture:
        env = os.environ.copy()
        env.pop('GUI_FORMS_FONT_DIR', None)
        env.pop('FILE_MANAGER_ROOT', None)
        start = time.monotonic()
        process = subprocess.Popen([str(executable), '--root', fixture], cwd=package, env=env,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        try:
            code = process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
            receipt['validation']['packaged_startup'] = 'process stayed alive for 5 seconds on generated empty root; not GUI workflow acceptance'
        else:
            raise RuntimeError(f'Packaged application exited early ({code}) after {time.monotonic() - start:.2f}s')
    receipt['files'] = {p.relative_to(package).as_posix(): sha256(p) for p in sorted(package.rglob('*')) if p.is_file()}
    (package / 'build-receipt.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    (build / 'package-check.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    name = f'file-manager-0.001-alpha-{host}-{arch}-{revision[:12]}' + ('-dirty' if dirty else '')
    archive = Path(shutil.make_archive(str(distribution / name), 'zip' if host == 'windows' else 'gztar', stage, package.name))
    (distribution / (archive.name + '.sha256')).write_text(sha256(archive) + '  ' + archive.name + '\n', encoding='utf-8')
    print(archive)


if __name__ == '__main__':
    main()
