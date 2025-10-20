#!/usr/bin/env python3
"""
TicoRAW Bitstream Analyzer
Analyzes the structure of TicoRAW compressed data to understand the compression format
"""

import sys
import struct

def analyze_ticoraw_bitstream(file_path):
    """Analyze the TicoRAW bitstream structure"""
    
    with open(file_path, 'rb') as f:
        data = f.read()
    
    print(f"File size: {len(data)} bytes")
    
    # Find TicoRAW signature
    sig_pos = data.find(b'CONTACT_INTOPIX_')
    if sig_pos == -1:
        print("TicoRAW signature not found")
        return
    
    print(f"TicoRAW signature found at offset {sig_pos}")
    print(f"Signature: {data[sig_pos:sig_pos+16]}")
    
    # Analyze header structure around signature
    header_start = max(0, sig_pos - 32)
    header_end = min(len(data), sig_pos + 64)
    header = data[header_start:header_end]
    
    print(f"\nHeader analysis around signature:")
    for i in range(0, len(header), 16):
        offset = header_start + i
        hex_str = ' '.join(f'{b:02x}' for b in header[i:i+16])
        ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in header[i:i+16])
        print(f"{offset:08x}: {hex_str:<48} {ascii_str}")
    
    # Analyze compressed data patterns
    compressed_start = sig_pos + 16
    compressed_data = data[compressed_start:compressed_start + 10000]  # First 10KB
    
    print(f"\nCompressed data analysis (first 1000 bytes):")
    for i in range(0, min(1000, len(compressed_data)), 16):
        hex_str = ' '.join(f'{b:02x}' for b in compressed_data[i:i+16])
        ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in compressed_data[i:i+16])
        print(f"{i:04x}: {hex_str:<48} {ascii_str}")
    
    # Look for patterns in the compressed data
    print(f"\nPattern analysis:")
    
    # Check for repeated patterns
    pattern_counts = {}
    for i in range(0, len(compressed_data) - 4, 4):
        pattern = compressed_data[i:i+4]
        pattern_counts[pattern] = pattern_counts.get(pattern, 0) + 1
    
    # Show most common patterns
    sorted_patterns = sorted(pattern_counts.items(), key=lambda x: x[1], reverse=True)
    print("Most common 4-byte patterns:")
    for pattern, count in sorted_patterns[:10]:
        hex_str = ' '.join(f'{b:02x}' for b in pattern)
        print(f"  {hex_str}: {count} occurrences")
    
    # Analyze bit patterns
    print(f"\nBit pattern analysis:")
    bit_counts = [0] * 8
    for byte in compressed_data[:1000]:  # Analyze first 1000 bytes
        for bit in range(8):
            if byte & (1 << bit):
                bit_counts[bit] += 1
    
    print("Bit frequency in first 1000 bytes:")
    for i, count in enumerate(bit_counts):
        print(f"  Bit {i}: {count} ({count/1000*100:.1f}%)")
    
    # Look for entropy patterns
    print(f"\nEntropy analysis:")
    byte_counts = [0] * 256
    for byte in compressed_data[:10000]:  # Analyze first 10KB
        byte_counts[byte] += 1
    
    # Calculate entropy
    import math
    entropy = 0
    for count in byte_counts:
        if count > 0:
            p = count / 10000
            entropy -= p * math.log2(p)
    
    print(f"Approximate entropy: {entropy:.2f} bits/byte")
    print(f"Compression ratio estimate: {8/entropy:.1f}:1")
    
    # Look for potential tile boundaries or markers
    print(f"\nLooking for potential markers:")
    markers = [b'\x00\x00\x00\x00', b'\xFF\xFF\xFF\xFF', b'\xAA\xAA\xAA\xAA', b'\x55\x55\x55\x55']
    for marker in markers:
        pos = compressed_data.find(marker)
        if pos != -1:
            print(f"  Found {marker.hex()} at offset {pos}")
    
    # Analyze data distribution
    print(f"\nData distribution analysis:")
    zero_count = compressed_data.count(0)
    ff_count = compressed_data.count(0xFF)
    print(f"  Zero bytes: {zero_count} ({zero_count/len(compressed_data)*100:.1f}%)")
    print(f"  0xFF bytes: {ff_count} ({ff_count/len(compressed_data)*100:.1f}%)")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 ticoraw_analyzer.py <file.nef>")
        sys.exit(1)
    
    analyze_ticoraw_bitstream(sys.argv[1])