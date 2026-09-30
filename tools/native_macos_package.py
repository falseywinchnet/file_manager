"""macOS package inspection, dependency closure and ad-hoc signing."""
from __future__ import annotations

from collections import deque
from dataclasses import dataclass
from pathlib import Path
import platform
import plistlib
import re
import shutil
from typing import TypedDict
from native_build_support import output, run, sha256

class MacRequirements(TypedDict):
    mach_o_minima: dict[str, list[str]]
    plist_minimum: str | None
    required_minimum_from_load_commands_and_plist: str
    tested_host_macos: str
    scope: str

@dataclass(frozen=True)
class LibraryCopy:
    original: Path
    target: Path


def dependencies(path: Path) -> list[str]:
    text: str = output('otool', '-L', path)
    lines: list[str] = text.splitlines()
    result: list[str] = []
    line: str
    for line in lines[1:]:
        stripped: str = line.strip()
        name: str = stripped.split(' (', 1)[0]
        result.append(name)
    return result


def runtime_paths(path: Path) -> list[str]:
    commands: str = output('otool', '-l', path)
    result: list[str] = re.findall(r'cmd LC_RPATH\n\s+cmdsize \d+\n\s+path (.*?) \(offset', commands)
    return result


def expand_path(value: str, owner: Path, executable: Path) -> Path:
    loader_expanded: str = value.replace('@loader_path', str(owner.parent))
    executable_expanded: str = loader_expanded.replace('@executable_path', str(executable.parent))
    result: Path = Path(executable_expanded)
    return result


def dependency_source(dependency: str, original: Path, executable: Path,
                      sdk: Path, frameworks: Path) -> Path:
    name: str = Path(dependency).name
    candidates: list[Path] = []
    if dependency.startswith('@rpath/'):
        bases: list[str] = runtime_paths(original)
        bases.extend(runtime_paths(executable))
        base: str
        for base in bases:
            expanded: Path = expand_path(base, original, executable)
            candidates.append(expanded / dependency[7:])
        candidates.append(sdk / 'lib' / name)
        candidates.append(frameworks / name)
    else:
        candidates.append(expand_path(dependency, original, executable))
    candidate: Path
    for candidate in candidates:
        if candidate.is_file():
            resolved: Path = candidate.resolve()
            return resolved
    raise RuntimeError(f'Unresolved Mach-O dependency {dependency} in {original}')


def library_ids(path: Path) -> list[str]:
    text: str = output('otool', '-D', path)
    lines: list[str] = text.splitlines()
    result: list[str] = lines[1:]
    return result


def mac_closure(app: Path, sdk: Path) -> None:
    """Copy private dependencies, rewrite load commands, verify, then sign.

The queue owns source/destination path records. Only app contents are mutated.
An error preserves the failed staging attempt for inspection, not publication.
"""
    executable: Path = app / 'Contents/MacOS/File Manager'
    frameworks: Path = app / 'Contents/Frameworks'
    frameworks.mkdir(exist_ok=True)
    queue: deque[LibraryCopy] = deque()
    queue.append(LibraryCopy(original=executable, target=executable))
    seen: set[Path] = set()
    names: dict[str, str] = {}
    dependency: str
    while queue:
        item: LibraryCopy = queue.popleft()
        original: Path = item.original
        target: Path = item.target
        if target in seen:
            continue
        seen.add(target)
        ids: list[str] = library_ids(original)
        for dependency in dependencies(original):
            if dependency in ids or dependency.startswith(('/System/', '/usr/lib/')):
                continue
            name: str = Path(dependency).name
            source: Path = dependency_source(dependency, original, executable, sdk, frameworks)
            source_hash: str = sha256(source)
            if name in names and source_hash != names[name]:
                raise RuntimeError(f'Conflicting dependency basename: {name}')
            destination: Path = frameworks / name
            if name not in names:
                names[name] = source_hash
                resolved_destination: Path = destination.resolve()
                if source != resolved_destination:
                    shutil.copy2(source, destination)
                destination.chmod(0o755)
                queue.append(LibraryCopy(original=source, target=destination))
            replacement: str = '@loader_path/' + name
            if target == executable:
                replacement = '@executable_path/../Frameworks/' + name
            run('install_name_tool', '-change', dependency, replacement, target)
        runtime_path: str
        for runtime_path in runtime_paths(target):
            if runtime_path.startswith('/'):
                run('install_name_tool', '-delete_rpath', runtime_path, target)
    target: Path
    for target in seen:
        target_ids: list[str] = library_ids(target)
        for dependency in dependencies(target):
            if dependency in target_ids or dependency.startswith(('/System/', '/usr/lib/')):
                continue
            resolved: Path = expand_path(dependency, target, executable)
            if not dependency.startswith(('@loader_path/', '@executable_path/')) or not resolved.is_file():
                raise RuntimeError(f'Nonportable dependency remains: {dependency}')
    library: Path
    for library in frameworks.iterdir():
        run('codesign', '--force', '--sign', '-', library)
    run('codesign', '--force', '--deep', '--sign', '-', app)
    run('codesign', '--verify', '--deep', '--strict', app)


def version_key(value: str) -> tuple[int, int, int]:
    """Compare up to three decimal components, padding missing components with zero."""
    fields: list[str] = value.split('.')
    numbers: list[int] = [0, 0, 0]
    count: int = min(len(fields), 3)
    index: int = 0
    for index in range(count):
        numbers[index] = int(fields[index])
    result: tuple[int, int, int] = (numbers[0], numbers[1], numbers[2])
    return result


def deployment_versions(commands: str) -> list[str]:
    versions: list[str] = []
    blocks: list[str] = re.split(r'Load command \d+\n', commands)
    command: str
    for command in blocks:
        match: re.Match[str] | None = None
        if re.search(r'\bcmd LC_BUILD_VERSION\b', command):
            match = re.search(r'\bminos ([0-9.]+)', command)
        elif re.search(r'\bcmd LC_VERSION_MIN_MACOSX\b', command):
            match = re.search(r'\bversion ([0-9.]+)', command)
        if match is not None:
            versions.append(match.group(1))
    return versions


def mac_minimum_versions(app: Path, package: Path) -> MacRequirements:
    """Report linker minima, not a claim of older-OS runtime verification."""
    records: dict[str, list[str]] = {}
    macos_directory: Path = app / 'Contents/MacOS'
    framework_directory: Path = app / 'Contents/Frameworks'
    component_directory: Path = package / 'components'
    candidates: list[Path] = list(macos_directory.iterdir())
    candidates.extend(framework_directory.iterdir())
    if component_directory.is_dir():
        candidates.extend(component_directory.iterdir())
    all_versions: list[str] = []
    path: Path
    for path in candidates:
        if not path.is_file():
            continue
        commands: str = output('otool', '-l', path)
        versions: list[str] = deployment_versions(commands)
        if not versions:
            raise RuntimeError(f'No macOS deployment minimum load command found: {path}')
        relative: Path = path.relative_to(package)
        records[relative.as_posix()] = versions
        all_versions.extend(versions)
    plist_path: Path = app / 'Contents/Info.plist'
    plist_bytes: bytes = plist_path.read_bytes()
    plist: dict[str, object] = plistlib.loads(plist_bytes)
    plist_value: object = plist.get('LSMinimumSystemVersion')
    plist_minimum: str | None = None
    if plist_value is not None:
        if not isinstance(plist_value, str):
            raise ValueError('LSMinimumSystemVersion must be a string')
        plist_minimum = plist_value
    if plist_minimum:
        all_versions.append(plist_minimum)
    required: str = max(all_versions, key=version_key)
    result: MacRequirements = {
        'mach_o_minima': records, 'plist_minimum': plist_minimum,
        'required_minimum_from_load_commands_and_plist': required,
        'tested_host_macos': platform.mac_ver()[0],
        'scope': 'Linker/plist requirement only; runtime tested on the named CI host, not on every OS at or above this minimum'}
    return result
