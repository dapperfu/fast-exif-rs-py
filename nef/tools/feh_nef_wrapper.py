#!/usr/bin/env python3
"""
Wrapper to display Nikon NEF (including HE/HE*) in feh by extracting the
embedded JPEG preview to a temporary file.

Usage:
  venv/bin/python tools/feh_nef_wrapper.py /path/to/*.NEF [--] [feh args]
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import List


def is_nef(path: Path) -> bool:
    return path.suffix.lower() in {".nef", ".nrw"}


def extract_preview(nef: Path, out: Path) -> None:
    here = Path(__file__).resolve().parent
    extractor = here / "nef_extract_preview.py"
    cmd = [sys.executable, str(extractor), str(nef), str(out)]
    subprocess.run(cmd, check=True)


def main() -> None:
    ap = argparse.ArgumentParser(add_help=True)
    ap.add_argument("inputs", nargs="+", help="Input files (NEF/others)")
    ap.add_argument("--feh", default="feh", help="feh binary to use")
    args, unknown = ap.parse_known_args()

    tempdir = tempfile.mkdtemp(prefix="nef_preview_")
    previews: List[str] = []
    others: List[str] = []
    try:
        for p in args.inputs:
            path = Path(p)
            if is_nef(path):
                out = Path(tempdir) / (path.stem + ".jpg")
                try:
                    extract_preview(path, out)
                    previews.append(str(out))
                except Exception:
                    # If extraction fails, pass through original; feh will likely fail
                    others.append(str(path))
            else:
                others.append(str(path))

        cmd = [args.feh] + previews + others + unknown
        subprocess.run(cmd, check=False)
    finally:
        shutil.rmtree(tempdir, ignore_errors=True)


if __name__ == "__main__":
    main()


