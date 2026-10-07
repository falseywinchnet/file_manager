"""A cache seed may only restore its declared cache tree and provider identity."""
from __future__ import annotations
import io
import json
from pathlib import Path
import tarfile
import tempfile
import unittest
from fetch_build_inputs import restore_cache_seed


class CompilerSeedTests(unittest.TestCase):
    def make_seed(self, root: Path, member_name: str = '.ccache/a/result') -> Path:
        manifest: dict[str, object] = {
            'schema': 1, 'repository': 'falseywinchnet/gui_forms', 'platform': 'linux-x64', 'revision': 'provider-sha'}
        files: dict[str, bytes] = {
            'cache-manifest.json': json.dumps(manifest).encode('utf-8'),
            member_name: b'cached compile result'}
        path: Path = root / 'seed.tar.gz'
        archive: tarfile.TarFile
        with tarfile.open(path, 'w:gz') as archive:
            name: str
            for name in files:
                member: tarfile.TarInfo = tarfile.TarInfo(name)
                member.size = len(files[name])
                archive.addfile(member, io.BytesIO(files[name]))
        return path

    def test_seed_restores_only_after_identity_validation(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary)
            path: Path = self.make_seed(root)
            destination: Path = root / 'consumer'
            restore_cache_seed(path, destination, 'linux-x64', 'provider-sha')
            self.assertEqual((destination / '.ccache/a/result').read_bytes(),
                             b'cached compile result')

    def test_wrong_provider_and_platform_leave_destination_absent(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary)
            path: Path = self.make_seed(root)
            destination: Path = root / 'consumer'
            with self.assertRaisesRegex(ValueError, 'provider identity'):
                restore_cache_seed(path, destination, 'linux-x64', 'wrong-sha')
            with self.assertRaisesRegex(ValueError, 'provider identity'):
                restore_cache_seed(path, destination, 'windows-x64', 'provider-sha')
            self.assertFalse(destination.exists())

    def test_outside_cache_member_is_rejected_before_extraction(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary)
            path: Path = self.make_seed(root, '.ccache/../../outside')
            destination: Path = root / 'consumer'
            with self.assertRaisesRegex(ValueError, 'Invalid compiler-cache'):
                restore_cache_seed(path, destination, 'linux-x64', 'provider-sha')
            self.assertFalse(destination.exists())


if __name__ == '__main__':
    unittest.main()
