#!/usr/bin/env python3
"""
Dump HE* structure using existing TIFF parser.
"""
from __future__ import annotations
import argparse
import sys
from typing import Tuple

from tools.nef_probe import TiffReader  # reuse

def main() -> None:
    ap = argparse.ArgumentParser(description="Dump HE* structure (heuristic)")
    ap.add_argument("path")
    args = ap.parse_args()
    with open(args.path, "rb") as f:
        tr = TiffReader(f)
        ifd0 = tr.read_ifd(tr.ifd0_offset)
        print(f"Endian={'LE' if tr.endian=='<' else 'BE'} ifd0={tr.ifd0_offset}")
        # subIFDs
        sub = None
        for e in ifd0.entries:
            if e.tag == 0x014A:
                sub = tr.read_value(e)
                break
        if not sub:
            print("No SubIFDs found")
            return
        offs = []
        for i in range(0, len(sub), 4):
            offs.append(int.from_bytes(sub[i:i+4], 'little' if tr.endian=='<' else 'big'))
        print(f"SubIFDs: {offs}")
        for i, off in enumerate(offs):
            try:
                sifd = tr.read_ifd(off)
            except Exception:
                continue
            so = next((e for e in sifd.entries if e.tag==0x0111), None)
            sbc = next((e for e in sifd.entries if e.tag==0x0117), None)
            if so and sbc:
                so_v = int.from_bytes(tr.read_value(so)[:4], 'little' if tr.endian=='<' else 'big')
                sbc_v = int.from_bytes(tr.read_value(sbc)[:4], 'little' if tr.endian=='<' else 'big')
                print(f"SubIFD[{i}] StripOffsets={so_v} StripByteCounts={sbc_v}")

if __name__ == "__main__":
    main()