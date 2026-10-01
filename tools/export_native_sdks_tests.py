"""Reject stale, substituted and independently supplied SDK provenance."""
from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest

from export_native_sdks import require_validation
from native_build_support import sdk_fingerprint


class ValidationTests(unittest.TestCase):
    def test_export_requires_matching_tested_bytes_and_revision(self) -> None:
        scratch_name: str
        with tempfile.TemporaryDirectory() as scratch_name:
            build: Path = Path(scratch_name)
            sdk: Path = build / 'gui-forms-sdk'
            headers: Path = sdk / 'include'
            headers.mkdir(parents=True)
            header: Path = headers / 'public.hpp'
            header.write_text('tested', encoding='utf-8')
            receipt: dict[str, str] = {
                'source_revision': 'revision', 'frontend_ctest': 'passed',
                'gui_forms_ctest': 'passed', 'gui_forms_sdk_sha256': sdk_fingerprint(sdk)}
            path: Path = build / 'build-validation.json'
            path.write_text(json.dumps(receipt), encoding='utf-8')
            require_validation(build, 'revision')
            with self.assertRaisesRegex(RuntimeError, 'revision'):
                require_validation(build, 'other')
            header.write_text('substituted', encoding='utf-8')
            with self.assertRaisesRegex(RuntimeError, 'changed'):
                require_validation(build, 'revision')
            header.write_text('tested', encoding='utf-8')
            receipt['gui_forms_ctest'] = 'externally supplied SDK'
            path.write_text(json.dumps(receipt), encoding='utf-8')
            with self.assertRaisesRegex(RuntimeError, 'together'):
                require_validation(build, 'revision')
            receipt['gui_forms_ctest'] = 'passed'
            receipt['frontend_ctest'] = 'failed'
            path.write_text(json.dumps(receipt), encoding='utf-8')
            with self.assertRaisesRegex(RuntimeError, 'together'):
                require_validation(build, 'revision')


if __name__ == '__main__':
    unittest.main()
