#!/usr/bin/env python3
from pathlib import Path
import sys

PACKAGE_ROOT = Path(__file__).resolve().parents[1] / "src"
sys.path.insert(0, str(PACKAGE_ROOT))

from web_forms_compiler.cli import main


if __name__ == "__main__":
    raise SystemExit(main())
