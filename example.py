#!/usr/bin/env python3
"""
Example usage of fast-exif-rs-py Python bindings
"""

import fast_exif_rs_py
import os
import sys

def main():
    print(f"fast-exif-rs-py version: {fast_exif_rs_py.get_version()}")
    print(f"Supported formats: {fast_exif_rs_py.get_supported_formats()}")
    
    # Example 1: Read EXIF from a file (if it exists)
    test_file = "test_image.jpg"
    if os.path.exists(test_file):
        print(f"\nReading EXIF from {test_file}:")
        try:
            metadata = fast_exif_rs_py.read_exif_file(test_file)
            for key, value in metadata.items():
                print(f"  {key}: {value}")
        except Exception as e:
            print(f"Error reading {test_file}: {e}")
    else:
        print(f"\nTest file {test_file} not found. Create a test image to see EXIF data.")
    
    # Example 2: Object-oriented API
    print("\nUsing object-oriented API:")
    reader = fast_exif_rs_py.PyFastExifReader()
    print(f"Created reader: {reader}")
    
    # Example 3: Writer API
    print("\nUsing writer API:")
    writer = fast_exif_rs_py.PyFastExifWriter()
    print(f"Created writer: {writer}")
    
    # Example 4: Copier API
    print("\nUsing copier API:")
    copier = fast_exif_rs_py.PyFastExifCopier()
    print(f"Created copier: {copier}")

if __name__ == "__main__":
    main()
