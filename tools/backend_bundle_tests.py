"""Verify dependency identity and payload integrity before services are copied."""
from __future__ import annotations
import hashlib
import io
import json
from pathlib import Path
import tarfile
import tempfile
import unittest
from backend_bundle import install_bundle


class BackendBundleTests(unittest.TestCase):
    def make_bundle(self, root: Path, platform: str = 'linux-x64',
                    revision: str = 'fixture-revision', corrupt: bool = False,
                    link: bool = False) -> tuple[Path, str]:
        contents: dict[str, bytes] = {'fileman-engine': b'engine', 'orchestrator': b'core'}
        hashes: dict[str, str] = {}
        name: str
        for name in contents:
            hashes[name] = hashlib.sha256(contents[name]).hexdigest()
        manifest: dict[str, object] = {
            'schema': 1, 'repository': 'falseywinchnet/backend',
            'platform': platform, 'revision': revision, 'files': hashes}
        contents['manifest.json'] = json.dumps(manifest).encode('utf-8')
        if corrupt:
            contents['orchestrator'] = b'changed after validation'
        path: Path = root / 'backend.tar.gz'
        archive: tarfile.TarFile
        with tarfile.open(path, 'w:gz') as archive:
            for name in contents:
                member: tarfile.TarInfo = tarfile.TarInfo('backend/' + name)
                member.size = len(contents[name])
                if link and name == 'orchestrator':
                    member.type = tarfile.SYMTYPE
                    member.linkname = '/outside-fixture'
                    member.size = 0
                archive.addfile(member, io.BytesIO(contents[name]))
        digest: str = hashlib.sha256(path.read_bytes()).hexdigest()
        return path, digest

    def test_valid_bundle_copies_only_verified_services(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary)
            path: Path
            digest: str
            path, digest = self.make_bundle(root)
            output: Path = root / 'components'
            install_bundle(path, output, 'linux-x64', 'fixture-revision', digest)
            self.assertEqual((output / 'fileman-engine').read_bytes(), b'engine')
            self.assertEqual((output / 'orchestrator').read_bytes(), b'core')
            self.assertEqual(len(list(output.iterdir())), 2)
            self.assertTrue((root / 'backend-consumption.json').is_file())

    def test_archive_digest_mismatch_is_rejected_before_writing(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary)
            path: Path
            digest: str
            path, digest = self.make_bundle(root)
            with self.assertRaisesRegex(ValueError, 'archive SHA-256'):
                install_bundle(path, root / 'components', 'linux-x64', 'fixture-revision', 'wrong')
            self.assertFalse((root / 'components').exists())

    def test_platform_and_revision_mismatch_are_rejected(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary)
            path: Path
            digest: str
            path, digest = self.make_bundle(root)
            with self.assertRaisesRegex(ValueError, 'manifest identity'):
                install_bundle(path, root / 'components', 'macos-arm64', 'fixture-revision', digest)
            with self.assertRaisesRegex(ValueError, 'manifest identity'):
                install_bundle(path, root / 'components', 'linux-x64', 'another-revision', digest)
            self.assertFalse((root / 'components').exists())

    def test_corrupt_payload_and_symlink_are_rejected_before_writing(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            root: Path = Path(temporary)
            path: Path
            digest: str
            path, digest = self.make_bundle(root, corrupt=True)
            with self.assertRaisesRegex(ValueError, 'executable SHA-256'):
                install_bundle(path, root / 'components', 'linux-x64', 'fixture-revision', digest)
            path, digest = self.make_bundle(root, link=True)
            with self.assertRaisesRegex(ValueError, 'Invalid backend member'):
                install_bundle(path, root / 'components', 'linux-x64', 'fixture-revision', digest)
            self.assertFalse((root / 'components').exists())


if __name__ == '__main__':
    unittest.main()
