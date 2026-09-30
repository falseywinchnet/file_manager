#!/usr/bin/env python3
"""Build a development distribution using native tools; never install services."""
import argparse
import json
import os
from pathlib import Path
import platform
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def run(*args, cwd=ROOT):
    print('+', ' '.join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), cwd=cwd, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--gui-forms-sdk', type=Path, help='Reuse an already tested native SDK')
    parser.add_argument('--skip-components', action='store_true', help='Explicit frontend-only development package')
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error('--jobs must be positive')
    host = {'Windows': 'windows', 'Darwin': 'macos', 'Linux': 'linux'}[platform.system()]
    arch = {'AMD64': 'x64', 'x86_64': 'x64', 'arm64': 'arm64', 'aarch64': 'arm64'}[platform.machine()]
    build = ROOT / '.build' / f'native-{host}-{arch}'
    build.mkdir(parents=True, exist_ok=True)
    sdk = args.gui_forms_sdk.resolve() if args.gui_forms_sdk else build / 'gui-forms-sdk'
    os.environ['BUILD_JOBS'] = str(args.jobs)
    toolkit = build / 'gui-forms'
    if not args.gui_forms_sdk:
        options = [f'-DGUI_FORMS_ENABLE_{name}_HOST={"ON" if host == value else "OFF"}'
                   for name, value in [('WINDOWS', 'windows'), ('MACOS', 'macos'), ('LINUX', 'linux')]]
        if host == 'windows':
            options += ['-DGUI_FORMS_ENABLE_SKIA=OFF', '-DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=OFF']
        else:
            if host == 'linux':
                os.environ['GUI_FORMS_SYSTEM_BUILD_TOOLS'] = '1'
            run('sh', ROOT / 'gui_forms/third_party/fetch_skia_cpu.sh')
            run('sh', ROOT / 'gui_forms/third_party/fetch_text_stack.sh')
            skia_out = build / 'skia'
            if host == 'linux':
                run('sh', ROOT / 'gui_forms/third_party/build_skia_cpu_linux.sh', skia_out)
            else:
                import shutil
                skia_out.mkdir(exist_ok=True)
                shutil.copy2(ROOT / 'gui_forms/third_party/skia_cpu_args.gn', skia_out / 'args.gn')
                run(ROOT / 'gui_forms/third_party/skia/bin/gn', 'gen', skia_out,
                    '--root=' + str(ROOT / 'gui_forms/third_party/skia'))
                run(ROOT / 'gui_forms/third_party/skia/third_party/ninja/ninja', '-C', skia_out,
                    '-j', args.jobs, 'skia')
            options += ['-DGUI_FORMS_ENABLE_SKIA=ON', '-DGUI_FORMS_ENABLE_HARFBUZZ_TEXT=ON',
                        '-DGUI_FORMS_SKIA_PREBUILT=ON', f'-DGUI_FORMS_SKIA_OUT={skia_out}']
        run('cmake', '-S', ROOT / 'gui_forms', '-B', toolkit, '-G', 'Ninja',
            '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_INSTALL_LIBDIR=lib',
            '-DCMAKE_POSITION_INDEPENDENT_CODE=ON', '-DGUI_FORMS_BUILD_GALLERY=OFF',
            '-DGUI_FORMS_BUILD_TESTS=ON', f'-DCMAKE_INSTALL_PREFIX={sdk}', *options)
        run('cmake', '--build', toolkit, '--parallel', args.jobs)
        os.environ['GUI_FORMS_FONT_DIR'] = str(ROOT / 'gui_forms/assets/fonts')
        run('ctest', '--test-dir', toolkit, '--output-on-failure', '--timeout', '120')
        run('cmake', '--install', toolkit)
    manifest = build / 'gui-forms-consumption.json'
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    manifest.write_text(json.dumps({
        'identity': {'id': f'gui-forms-development-{host}-{arch}-{revision[:12]}',
                     'state': 'development', 'source_revision': revision},
        'validation': {'sdk': 'externally supplied' if args.gui_forms_sdk else 'native build and CTest passed'},
        'limits': ['No platform promotion or installed service claim; see package receipt']
    }, indent=2) + '\n', encoding='utf-8')
    frontend = build / 'frontend'
    run('cmake', '-S', ROOT / 'frontend', '-B', frontend, '-G', 'Ninja',
        '-DCMAKE_BUILD_TYPE=Release', f'-DGUIForms_DIR={sdk}/lib/cmake/GUIForms',
        f'-DFILE_MANAGER_GUI_FORMS_MANIFEST={manifest}',
        f'-DCMAKE_INSTALL_PREFIX={build}/frontend-sdk')
    run('cmake', '--build', frontend, '--parallel', args.jobs)
    run('ctest', '--test-dir', frontend, '--output-on-failure', '--timeout', '120')
    run('cmake', '--install', frontend)
    if not args.skip_components:
        suffix = '.exe' if host == 'windows' else ''
        services = build / 'components'
        services.mkdir(exist_ok=True)
        run('go', 'build', '-trimpath', '-o', services / ('fileman-engine' + suffix),
            './cmd/fileman-engine', cwd=ROOT / 'engine')
        run('cargo', 'build', '--locked', '--release', '--bin', 'orchestrator',
            '--target-dir', build / 'rust', '--jobs', args.jobs, cwd=ROOT / 'orchestrator')
        import shutil
        shutil.copy2(build / 'rust/release' / ('orchestrator' + suffix), services)
    (build / 'build-validation.json').write_text(json.dumps({
        'source_revision': revision, 'frontend_ctest': 'passed',
        'gui_forms_ctest': 'externally supplied SDK' if args.gui_forms_sdk else 'passed'
    }, indent=2) + '\n', encoding='utf-8')
    run(sys.executable, ROOT / 'tools/package_native.py', '--build', build,
        '--gui-forms-sdk', sdk, *(['--skip-components'] if args.skip_components else []))


if __name__ == '__main__':
    main()
