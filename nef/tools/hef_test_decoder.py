#!/usr/bin/env python3
"""
Test HE* decoder and export results.
"""
from __future__ import annotations
import argparse
import sys
import os
import struct

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.nef_probe import TiffReader

def extract_raw_payload(path: str) -> tuple[bytes, int, int]:
    """Extract the main RAW payload from HE* NEF."""
    with open(path, "rb") as f:
        tr = TiffReader(f)
        ifd0 = tr.read_ifd(tr.ifd0_offset)
        
        # Find SubIFDs
        sub = None
        for e in ifd0.entries:
            if e.tag == 0x014A:
                sub = tr.read_value(e)
                break
        if not sub:
            raise ValueError("No SubIFDs found")
        
        # Parse SubIFD offsets
        offs = []
        for i in range(0, len(sub), 4):
            offs.append(int.from_bytes(sub[i:i+4], 'little' if tr.endian=='<' else 'big'))
        
        # Find the main RAW tile (largest)
        max_len = 0
        raw_offset = 0
        raw_len = 0
        for off in offs:
            try:
                sifd = tr.read_ifd(off)
            except Exception:
                continue
            so = next((e for e in sifd.entries if e.tag==0x0111), None)
            sbc = next((e for e in sifd.entries if e.tag==0x0117), None)
            if so and sbc:
                so_v = int.from_bytes(tr.read_value(so)[:4], 'little' if tr.endian=='<' else 'big')
                sbc_v = int.from_bytes(tr.read_value(sbc)[:4], 'little' if tr.endian=='<' else 'big')
                if sbc_v > max_len:
                    max_len = sbc_v
                    raw_offset = so_v
                    raw_len = sbc_v
        
        if raw_len == 0:
            raise ValueError("No RAW data found")
        
        # Extract RAW payload
        f.seek(raw_offset)
        payload = f.read(raw_len)
        return payload, raw_offset, raw_len

def main() -> None:
    ap = argparse.ArgumentParser(description="Test HE* decoder")
    ap.add_argument("input", help="Input HE* NEF file")
    ap.add_argument("--dump-raw", help="Dump raw payload to file")
    args = ap.parse_args()
    
    try:
        payload, offset, length = extract_raw_payload(args.input)
        print(f"Extracted {length} bytes from offset {offset}")
        
        # Check for TicoRAW signature
        if len(payload) >= 20:
            sig = payload[4:20]
            print(f"Signature: {sig}")
            if sig == b"CONTACT_INTOPIX_":
                print("✓ Confirmed TicoRAW format")
            else:
                print("✗ Unknown format")
        
        if args.dump_raw:
            with open(args.dump_raw, "wb") as f:
                f.write(payload)
            print(f"Raw payload saved to {args.dump_raw}")
            
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
