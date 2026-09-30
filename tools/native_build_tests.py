"""Generated-fixture checks; no SDK, real application or distribution writes."""
from __future__ import annotations

import hashlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile
import unittest
from unittest.mock import patch
import zipfile

sys.path.insert(0, str(Path(__file__).resolve().parent))
import collect_diagnostics
import native_build_support
import native_macos_package
import package_native

class BoundedStream(io.BytesIO):
    def __init__(self, contents: bytes) -> None:
        super().__init__(contents)
        self.largest_request: int = 0

    def readinto(self, destination: bytearray) -> int:
        self.largest_request = max(self.largest_request, len(destination))
        count: int = super().readinto(destination)
        return count

class OwnedProcess:
    """Named fake child whose lifetime events remain inspectable."""
    def __init__(self) -> None:
        self.events: list[str] = []
        self.killed: bool = False

    def poll(self) -> int | None:
        self.events.append('poll')
        return None

    def terminate(self) -> None:
        self.events.append('terminate')

    def kill(self) -> None:
        self.events.append('kill')
        self.killed = True

    def wait(self, timeout: float | None = None) -> int:
        self.events.append('wait')
        if not self.killed:
            raise subprocess.TimeoutExpired('fixture-child', timeout)
        return 0

class NativeToolTests(unittest.TestCase):
    def test_bounded_hash_and_empty_scratch(self) -> None:
        contents: bytes = b'fixture\x00' * 400000
        stream: BoundedStream = BoundedStream(contents)
        scratch: bytearray = bytearray(4096)
        expected: str = hashlib.sha256(contents).hexdigest()
        actual: str = native_build_support.stream_sha256(stream, scratch)
        self.assertEqual(actual, expected)
        self.assertEqual(stream.largest_request, 4096)
        with self.assertRaises(ValueError):
            native_build_support.stream_sha256(stream, bytearray())

    def test_sdk_fingerprint_compatibility_and_change(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary)
            header: Path = root / 'include' / 'unicode-Ω.hpp'
            header.parent.mkdir()
            header.write_bytes(b'abc\x00def')
            expected_input: bytes = 'include/unicode-Ω.hpp'.encode('utf-8') + b'\0abc\x00def\0'
            expected: str = hashlib.sha256(expected_input).hexdigest()
            self.assertEqual(native_build_support.sdk_fingerprint(root), expected)
            header.write_bytes(b'changed')
            self.assertNotEqual(native_build_support.sdk_fingerprint(root), expected)

    def test_diagnostic_boundary_hashes_and_exclusive_output(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary).resolve()
            fixture: Path = root / 'one.txt'
            fixture.write_bytes(b'fixture')
            expected: str = hashlib.sha256(b'fixture').hexdigest()
            checks: dict[str, str] = collect_diagnostics.verify_files(root, {'one.txt': expected, 'missing': expected})
            self.assertEqual(checks, {'one.txt': 'matched', 'missing': 'missing'})
            fixture.write_bytes(b'changed')
            self.assertEqual(collect_diagnostics.verify_files(root, {'one.txt': expected}), {'one.txt': 'mismatch'})
            with self.assertRaises(RuntimeError):
                collect_diagnostics.verify_files(root, {'../outside': expected})
            report: Path = root / 'report.json'
            collect_diagnostics.write_report(report, {'schema': 1})
            with self.assertRaises(FileExistsError):
                collect_diagnostics.write_report(report, {'schema': 2})
            self.assertEqual(json.loads(report.read_text()), {'schema': 1})

    def test_child_shutdown_escalates_and_reaps(self) -> None:
        process: OwnedProcess = OwnedProcess()
        collect_diagnostics.stop_owned_process(process)
        self.assertEqual(process.events, ['poll', 'terminate', 'wait', 'kill', 'wait'])

    def test_real_owned_child_is_reaped(self) -> None:
        command: list[str] = [sys.executable, '-c', 'import time; time.sleep(60)']
        process: subprocess.Popen[bytes] = subprocess.Popen(command)
        try:
            collect_diagnostics.stop_owned_process(process)
            self.assertIsNotNone(process.poll())
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()

    def test_smoke_exception_reaps_owned_child(self) -> None:
        process: OwnedProcess = OwnedProcess()
        with patch('package_native.subprocess.Popen', return_value=process):
            with patch.object(process, 'wait', side_effect=[OSError('fixture failure'), subprocess.TimeoutExpired('fixture-child', 10), 0]):
                with self.assertRaises(OSError):
                    package_native.smoke_package(Path('unused'), Path.cwd())
        self.assertIn('terminate', process.events)
        self.assertIn('kill', process.events)

    def test_archive_readback_and_negative_hash_permissions(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary)
            contents: bytes = b'archive fixture' * 100000
            expected: str = hashlib.sha256(contents).hexdigest()
            archive: Path = root / 'fixture.zip'
            packed_zip: zipfile.ZipFile
            with zipfile.ZipFile(archive, 'w') as packed_zip:
                packed_zip.writestr('FileManager/app', contents)
            package_native.verify_archive(archive, 'windows', {'app': expected}, 'app')
            with self.assertRaises(RuntimeError):
                package_native.verify_archive(archive, 'windows', {'app': 'wrong'}, 'app')
            packed_tar: tarfile.TarFile
            mode: int
            for mode in [0o755, 0o644]:
                tar_path: Path = root / ('fixture-' + str(mode) + '.tar.gz')
                member: tarfile.TarInfo = tarfile.TarInfo('FileManager/app')
                member.size = len(contents)
                member.mode = mode
                with tarfile.open(tar_path, 'w:gz') as packed_tar:
                    packed_tar.addfile(member, io.BytesIO(contents))
                if mode == 0o755:
                    package_native.verify_archive(tar_path, 'linux', {'app': expected}, 'app')
                else:
                    with self.assertRaises(RuntimeError):
                        package_native.verify_archive(tar_path, 'linux', {'app': expected}, 'app')

    def test_macos_version_parsing(self) -> None:
        commands: str = 'Load command 0\n cmd LC_BUILD_VERSION\n minos 26.0\nLoad command 1\n cmd LC_VERSION_MIN_MACOSX\n version 11.0.0\n'
        self.assertEqual(native_macos_package.deployment_versions(commands), ['26.0', '11.0.0'])
        self.assertEqual(native_macos_package.version_key('13.0'), native_macos_package.version_key('13.0.0'))
        self.assertGreater(native_macos_package.version_key('26.0'), native_macos_package.version_key('11.0.0'))


if __name__ == '__main__':
    unittest.main()
