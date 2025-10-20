#!/usr/bin/env python3
"""
Test the C++ parser logic with Python to debug the issue.
"""
from __future__ import annotations
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.nef_probe import TiffReader

def test_parser_logic(path: str) -> None:
    """Test the same logic as the C++ parser."""
    with open(path, "rb") as f:
        tr = TiffReader(f)
        ifd0 = tr.read_ifd(tr.ifd0_offset)
        
        # Find width, height, sub_ifds
        width = height = sub_ifds = 0
        for e in ifd0.entries:
            if e.tag == 0x0100:  # ImageWidth
                width = int.from_bytes(tr.read_value(e), 'little' if tr.endian=='<' else 'big')
            elif e.tag == 0x0101:  # ImageLength
                height = int.from_bytes(tr.read_value(e), 'little' if tr.endian=='<' else 'big')
            elif e.tag == 0x014A:  # SubIFDs
                sub_data = tr.read_value(e)
                sub_ifds = int.from_bytes(sub_data[:4], 'little' if tr.endian=='<' else 'big')
        
        print(f"Width: {width}, Height: {height}")
        print(f"SubIFDs offset: {sub_ifds}")
        
        if width == 0 or height == 0:
            print("ERROR: Missing width/height")
            return
        
        if sub_ifds == 0:
            print("ERROR: No SubIFDs")
            return
        
        # Parse SubIFDs
        tiles = []
        f.seek(sub_ifds)
        for sub_idx in range(6):
            sub_off_data = f.read(4)
            if len(sub_off_data) != 4:
                break
            sub_off = int.from_bytes(sub_off_data, 'little' if tr.endian=='<' else 'big')
            print(f"SubIFD[{sub_idx}] offset: {sub_off}")
            
            if sub_off == 0:
                continue
                
            try:
                sifd = tr.read_ifd(sub_off)
                so = sbc = 0
                for e in sifd.entries:
                    if e.tag == 0x0111:  # StripOffsets
                        so_data = tr.read_value(e)
                        so = int.from_bytes(so_data[:4], 'little' if tr.endian=='<' else 'big')
                    elif e.tag == 0x0117:  # StripByteCounts
                        sbc_data = tr.read_value(e)
                        sbc = int.from_bytes(sbc_data[:4], 'little' if tr.endian=='<' else 'big')
                
                print(f"  StripOffsets: {so}, StripByteCounts: {sbc}")
                if so > 0 and sbc > 0:
                    tiles.append((so, sbc))
                    
            except Exception as e:
                print(f"  Error parsing SubIFD: {e}")
        
        print(f"Found {len(tiles)} tiles")
        if len(tiles) == 0:
            print("ERROR: No valid tiles found")
        else:
            print("SUCCESS: Parser should work")

if __name__ == "__main__":
    test_parser_logic(sys.argv[1])
