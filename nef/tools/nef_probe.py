#!/usr/bin/env python3
"""
NEF/HE* probing tool.

This script inspects Nikon NEF files (including HE/HE*) and prints key TIFF/IFD
layout, compression tags, offsets/lengths for preview JPEG, and basic CFA info.

Usage:
  venv/bin/python tools/nef_probe.py /path/to/file.NEF

The script intentionally avoids external tooling at runtime and relies only on
Python's standard library for binary parsing.
"""
from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from typing import BinaryIO, Iterable, List, Optional, Tuple


TIFF_MAGIC_LITTLE = b"II\x2a\x00"
TIFF_MAGIC_BIG = b"MM\x00\x2a"


@dataclass
class IfdEntry:
    tag: int
    type_: int
    count: int
    value_or_offset: int


@dataclass
class Ifd:
    offset: int
    entries: List[IfdEntry]
    next_ifd_offset: int


TypeSizes = {
    1: 1,  # BYTE
    2: 1,  # ASCII
    3: 2,  # SHORT
    4: 4,  # LONG
    5: 8,  # RATIONAL
    7: 1,  # UNDEFINED
    9: 4,  # SLONG
    10: 8,  # SRATIONAL
}


class TiffReader:
    def __init__(self, f: BinaryIO) -> None:
        self.f = f
        self.endian = "<"  # default little
        magic = self.f.read(4)
        if magic == TIFF_MAGIC_LITTLE:
            self.endian = "<"
        elif magic == TIFF_MAGIC_BIG:
            self.endian = ">"
        else:
            raise ValueError("Not a TIFF/NEF file (bad magic)")
        (self.ifd0_offset,) = struct.unpack(self.endian + "I", self.f.read(4))

    def read_ifd(self, offset: int) -> Ifd:
        self.f.seek(offset)
        (num_entries,) = struct.unpack(self.endian + "H", self.f.read(2))
        entries: List[IfdEntry] = []
        for _ in range(num_entries):
            raw = self.f.read(12)
            tag, ttype, count, val = struct.unpack(self.endian + "HHII", raw)
            entries.append(IfdEntry(tag, ttype, count, val))
        (next_off,) = struct.unpack(self.endian + "I", self.f.read(4))
        return Ifd(offset, entries, next_off)

    def read_value(self, entry: IfdEntry) -> bytes:
        size = TypeSizes.get(entry.type_, 1) * entry.count
        if size <= 4:
            # Value is in-place in value_or_offset field (little/big-endian aware)
            data = struct.pack(self.endian + "I", entry.value_or_offset)
            return data[:size]
        self.f.seek(entry.value_or_offset)
        return self.f.read(size)


def find_tag(ifd: Ifd, tag: int) -> Optional[IfdEntry]:
    for e in ifd.entries:
        if e.tag == tag:
            return e
    return None


def parse_long_array(endian: str, data: bytes) -> List[int]:
    if len(data) % 4 != 0:
        return []
    return list(struct.unpack(endian + f"{len(data)//4}I", data))


def main() -> None:
    parser = argparse.ArgumentParser(description="Probe Nikon NEF/HE* structure")
    parser.add_argument("path", help="Path to NEF file")
    args = parser.parse_args()

    with open(args.path, "rb") as f:
        tr = TiffReader(f)
        ifd0 = tr.read_ifd(tr.ifd0_offset)

        # Common TIFF/EXIF tags used in NEF
        TAG_COMPRESSION = 0x0103
        TAG_IMAGE_WIDTH = 0x0100
        TAG_IMAGE_LENGTH = 0x0101
        TAG_STRIP_OFFSETS = 0x0111
        TAG_STRIP_BYTE_COUNTS = 0x0117

        # Nikon MakerNote / SubIFDs often carry RAW and preview info
        TAG_SUB_IFDS = 0x014A

        comp = find_tag(ifd0, TAG_COMPRESSION)
        width = find_tag(ifd0, TAG_IMAGE_WIDTH)
        length = find_tag(ifd0, TAG_IMAGE_LENGTH)
        sub_ifds = find_tag(ifd0, TAG_SUB_IFDS)

        print(f"Endian= {'LE' if tr.endian=='<' else 'BE'}")
        if width:
            w = int.from_bytes(tr.read_value(width), "little" if tr.endian == "<" else "big")
            print(f"IFD0.Width= {w}")
        if length:
            h = int.from_bytes(tr.read_value(length), "little" if tr.endian == "<" else "big")
            print(f"IFD0.Height= {h}")
        if comp:
            c = int.from_bytes(tr.read_value(comp), "little" if tr.endian == "<" else "big")
            print(f"IFD0.Compression= {c}")

        # Enumerate SubIFDs if present (thumbnail, RAW, etc.)
        sub_offsets: List[int] = []
        if sub_ifds:
            raw = tr.read_value(sub_ifds)
            sub_offsets = parse_long_array(tr.endian, raw)
            print(f"SubIFDs= {sub_offsets}")

        # Try to locate a JPEG preview in IFD0 or first SubIFD via Strip Offsets/ByteCounts
        def dump_preview_info(ifd: Ifd, label: str) -> None:
            so = find_tag(ifd, TAG_STRIP_OFFSETS)
            sbc = find_tag(ifd, TAG_STRIP_BYTE_COUNTS)
            if so and sbc:
                offs = parse_long_array(tr.endian, tr.read_value(so))
                lens = parse_long_array(tr.endian, tr.read_value(sbc))
                if offs and lens:
                    print(f"{label}.Preview.Offset= {offs[0]}")
                    print(f"{label}.Preview.Length= {lens[0]}")

        dump_preview_info(ifd0, "IFD0")
        for i, off in enumerate(sub_offsets):
            try:
                sifd = tr.read_ifd(off)
            except Exception:
                continue
            dump_preview_info(sifd, f"SubIFD[{i}]")

        # Heuristic HE/HE* identification: Nikon tag in MakerNotes uses values for "High Efficiency" quality
        # We do not fully parse MakerNotes here; this is a placeholder to surface basic TIFF structure.


if __name__ == "__main__":
    main()


