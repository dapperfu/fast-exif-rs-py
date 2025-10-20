#!/usr/bin/env python3
"""
Extract embedded JPEG preview from Nikon NEF (including HE/HE*).

Usage:
  venv/bin/python tools/nef_extract_preview.py input.NEF output.jpg

Reads TIFF IFD(s), finds a JPEG preview via StripOffsets/StripByteCounts in
either IFD0 or a SubIFD, and writes it to the specified output path.
"""
from __future__ import annotations

import argparse
import struct
from typing import BinaryIO, List, Optional


TIFF_MAGIC_LITTLE = b"II\x2a\x00"
TIFF_MAGIC_BIG = b"MM\x00\x2a"


class Tiff:
    def __init__(self, f: BinaryIO) -> None:
        self.f = f
        magic = self.f.read(4)
        if magic == TIFF_MAGIC_LITTLE:
            self.endian = "<"
        elif magic == TIFF_MAGIC_BIG:
            self.endian = ">"
        else:
            raise ValueError("Not a TIFF/NEF file")
        (self.ifd0_offset,) = struct.unpack(self.endian + "I", self.f.read(4))

    def read_ifd(self, offset: int):
        self.f.seek(offset)
        (num_entries,) = struct.unpack(self.endian + "H", self.f.read(2))
        entries = []
        for _ in range(num_entries):
            tag, ttype, count, val = struct.unpack(self.endian + "HHII", self.f.read(12))
            entries.append((tag, ttype, count, val))
        (next_off,) = struct.unpack(self.endian + "I", self.f.read(4))
        return entries, next_off

    def read_value(self, entry) -> bytes:
        tag, ttype, count, val = entry
        type_sizes = {1:1,2:1,3:2,4:4,5:8,7:1,9:4,10:8}
        size = type_sizes.get(ttype, 1) * count
        if size <= 4:
            data = struct.pack(self.endian + "I", val)
            return data[:size]
        self.f.seek(val)
        return self.f.read(size)


def parse_longs(endian: str, data: bytes) -> List[int]:
    if len(data) % 4 != 0:
        return []
    return list(struct.unpack(endian + f"{len(data)//4}I", data))


def find_tag(entries, tag: int):
    for e in entries:
        if e[0] == tag:
            return e
    return None


def extract_preview(path_in: str, path_out: str) -> None:
    with open(path_in, "rb") as f:
        t = Tiff(f)
        TAG_SUB_IFDS = 0x014A
        TAG_STRIP_OFFSETS = 0x0111
        TAG_STRIP_BYTE_COUNTS = 0x0117

        ifd0, next0 = t.read_ifd(t.ifd0_offset)

        def try_ifd(entries) -> Optional[tuple[int,int]]:
            so = find_tag(entries, TAG_STRIP_OFFSETS)
            sbc = find_tag(entries, TAG_STRIP_BYTE_COUNTS)
            if not so or not sbc:
                return None
            offs = parse_longs(t.endian, t.read_value(so))
            lens = parse_longs(t.endian, t.read_value(sbc))
            if offs and lens and lens[0] > 0:
                return offs[0], lens[0]
            return None

        # IFD0 first
        res = try_ifd(ifd0)
        if res is None:
            sub = find_tag(ifd0, TAG_SUB_IFDS)
            if sub:
                offs = parse_longs(t.endian, t.read_value(sub))
                for off in offs:
                    try:
                        entries, _ = t.read_ifd(off)
                    except Exception:
                        continue
                    res = try_ifd(entries)
                    if res:
                        break

        if not res:
            raise RuntimeError("Could not locate embedded JPEG preview")

        start, length = res
        f.seek(start)
        blob = f.read(length)
        # Basic validation: JPEG SOI
        if not (len(blob) >= 2 and blob[0] == 0xFF and blob[1] == 0xD8):
            # Some cameras store preview as other compressed data; we still write it.
            pass
        with open(path_out, "wb") as out:
            out.write(blob)


def main() -> None:
    ap = argparse.ArgumentParser(description="Extract embedded JPEG from NEF")
    ap.add_argument("input", help="Input NEF path")
    ap.add_argument("output", help="Output JPEG path")
    args = ap.parse_args()
    extract_preview(args.input, args.output)


if __name__ == "__main__":
    main()


