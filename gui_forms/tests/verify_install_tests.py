"""Verify the install-check command contract without building or touching an SDK."""
from __future__ import annotations
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import verify_install

class CommandRecorder:
    def __init__(self) -> None:
        self.commands: list[list[str]] = []
        self.expected_success: list[bool] = []

    def run(self, command: list[str], expected_success: bool = True) -> str:
        self.commands.append(command.copy())
        self.expected_success.append(expected_success)
        if expected_success:
            return 'fixture passed'
        component: str = Path(command[command.index('-B') + 1]).name
        result: str = "Requested GUIForms component '" + component + "' is not installed"
        return result

class VerifyInstallTests(unittest.TestCase):
    def test_expected_command_outcomes(self) -> None:
        command: list[str] = ['generated-fixture-command']
        success: subprocess.CompletedProcess[str] = subprocess.CompletedProcess(command, 0, 'okay')
        failure: subprocess.CompletedProcess[str] = subprocess.CompletedProcess(command, 1, 'rejected')
        with patch('verify_install.subprocess.run', return_value=success):
            self.assertEqual(verify_install.run(command), 'okay')
            with self.assertRaises(RuntimeError):
                verify_install.run(command, expected_success=False)
        with patch('verify_install.subprocess.run', return_value=failure):
            self.assertEqual(verify_install.run(command, expected_success=False), 'rejected')
            with self.assertRaises(RuntimeError):
                verify_install.run(command)

    def test_main_sequence_and_relocation(self) -> None:
        temporary: str
        with tempfile.TemporaryDirectory() as temporary:
            prefix: Path = Path(temporary) / 'fixture-sdk'
            prefix.mkdir()
            marker: Path = prefix / 'marker.txt'
            marker.write_text('unchanged fixture', encoding='utf-8')
            toolchain: Path = Path(temporary) / 'fixture.cmake'
            toolchain.write_text('# generated fixture', encoding='utf-8')
            recorder: CommandRecorder = CommandRecorder()
            arguments: list[str] = ['verify_install.py', str(prefix), '--relocate',
                                    '--toolchain', str(toolchain), '--runner', 'wine64 --fixture']
            with patch.object(sys, 'argv', arguments):
                with patch('verify_install.run', side_effect=recorder.run):
                    verify_install.main()
            self.assertEqual(len(recorder.commands), 5)
            self.assertEqual(recorder.commands[0][0:2], ['cmake', '-S'])
            self.assertEqual(recorder.commands[1][0:2], ['cmake', '--build'])
            self.assertEqual(recorder.commands[2][0:2], ['ctest', '--test-dir'])
            self.assertEqual(recorder.expected_success, [True, True, True, False, False])
            self.assertIn('-DCMAKE_CROSSCOMPILING_EMULATOR=wine64;--fixture', recorder.commands[0])
            self.assertEqual(marker.read_text(encoding='utf-8'), 'unchanged fixture')


if __name__ == '__main__':
    unittest.main()
