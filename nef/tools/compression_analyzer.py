#!/usr/bin/env python3
"""
Advanced TicoRAW Bitstream Analyzer
Analyzes the compression structure to understand the actual algorithm
"""

import sys
import struct
import math

def analyze_compression_structure(file_path):
    """Analyze the compression structure in detail"""
    
    with open(file_path, 'rb') as f:
        data = f.read()
    
    # Find TicoRAW signature
    sig_pos = data.find(b'CONTACT_INTOPIX_')
    if sig_pos == -1:
        print("TicoRAW signature not found")
        return
    
    print(f"TicoRAW signature found at offset {sig_pos}")
    
    # Extract compressed data
    compressed_start = sig_pos + 16
    compressed_data = data[compressed_start:compressed_start + 50000]  # First 50KB
    
    print(f"Analyzing first {len(compressed_data)} bytes of compressed data")
    
    # Look for Huffman-like patterns
    print(f"\nHuffman-like pattern analysis:")
    
    # Analyze byte frequency
    byte_freq = [0] * 256
    for byte in compressed_data:
        byte_freq[byte] += 1
    
    # Find most/least frequent bytes
    sorted_bytes = sorted(enumerate(byte_freq), key=lambda x: x[1], reverse=True)
    
    print("Most frequent bytes:")
    for i, (byte_val, freq) in enumerate(sorted_bytes[:20]):
        if freq > 0:
            print(f"  {byte_val:02x}: {freq} ({freq/len(compressed_data)*100:.1f}%)")
    
    print("\nLeast frequent bytes:")
    for i, (byte_val, freq) in enumerate(sorted_bytes[-20:]):
        if freq > 0:
            print(f"  {byte_val:02x}: {freq} ({freq/len(compressed_data)*100:.1f}%)")
    
    # Look for run-length encoding patterns
    print(f"\nRun-length analysis:")
    runs = []
    current_byte = compressed_data[0]
    current_length = 1
    
    for i in range(1, len(compressed_data)):
        if compressed_data[i] == current_byte:
            current_length += 1
        else:
            if current_length > 1:
                runs.append((current_byte, current_length))
            current_byte = compressed_data[i]
            current_length = 1
    
    if current_length > 1:
        runs.append((current_byte, current_length))
    
    # Show longest runs
    runs.sort(key=lambda x: x[1], reverse=True)
    print("Longest runs:")
    for byte_val, length in runs[:10]:
        print(f"  {byte_val:02x}: {length} bytes")
    
    # Look for delta coding patterns
    print(f"\nDelta coding analysis:")
    deltas = []
    for i in range(1, min(1000, len(compressed_data))):
        delta = compressed_data[i] - compressed_data[i-1]
        deltas.append(delta)
    
    delta_freq = {}
    for delta in deltas:
        delta_freq[delta] = delta_freq.get(delta, 0) + 1
    
    sorted_deltas = sorted(delta_freq.items(), key=lambda x: x[1], reverse=True)
    print("Most common deltas:")
    for delta, freq in sorted_deltas[:10]:
        print(f"  {delta:+3d}: {freq} ({freq/len(deltas)*100:.1f}%)")
    
    # Look for bit-level patterns
    print(f"\nBit-level pattern analysis:")
    
    # Analyze bit transitions
    bit_transitions = [[0] * 8 for _ in range(8)]
    for i in range(1, min(1000, len(compressed_data))):
        prev_byte = compressed_data[i-1]
        curr_byte = compressed_data[i]
        for bit in range(8):
            prev_bit = (prev_byte >> bit) & 1
            curr_bit = (curr_byte >> bit) & 1
            bit_transitions[prev_bit][curr_bit] += 1
    
    print("Bit transition matrix (prev -> curr):")
    print("    0    1")
    for i in range(2):
        print(f"{i}: {bit_transitions[i][0]:4d} {bit_transitions[i][1]:4d}")
    
    # Look for potential codebook patterns
    print(f"\nCodebook analysis:")
    
    # Look for repeated 2-byte patterns that might be codewords
    pattern2_freq = {}
    for i in range(0, len(compressed_data) - 1, 2):
        pattern = compressed_data[i:i+2]
        pattern2_freq[pattern] = pattern2_freq.get(pattern, 0) + 1
    
    sorted_patterns2 = sorted(pattern2_freq.items(), key=lambda x: x[1], reverse=True)
    print("Most frequent 2-byte patterns:")
    for pattern, freq in sorted_patterns2[:10]:
        hex_str = ' '.join(f'{b:02x}' for b in pattern)
        print(f"  {hex_str}: {freq} ({freq/(len(compressed_data)//2)*100:.1f}%)")
    
    # Look for potential tile boundaries
    print(f"\nTile boundary analysis:")
    
    # Look for patterns that might indicate tile boundaries
    boundary_markers = [b'\x00\x00\x00\x00', b'\xFF\xFF\xFF\xFF', b'\xAA\xAA\xAA\xAA']
    for marker in boundary_markers:
        positions = []
        start = 0
        while True:
            pos = compressed_data.find(marker, start)
            if pos == -1:
                break
            positions.append(pos)
            start = pos + 1
        
        if positions:
            print(f"  Found {marker.hex()} at positions: {positions[:10]}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 compression_analyzer.py <file.nef>")
        sys.exit(1)
    
    analyze_compression_structure(sys.argv[1])
