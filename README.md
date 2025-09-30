# fast-exif-rs-py

Python bindings for [fast-exif-rs](https://github.com/dapperfu/fast-exif-rs) - A high-performance EXIF metadata extraction library written in Rust.

## Features

- **High Performance**: Pure Rust implementation with optimized parsing
- **Comprehensive Format Support**: JPEG, CR2, NEF, ARW, RAF, SRW, PEF, RW2, ORF, DNG, HEIF, MOV, MP4, 3GP, AVI, WMV, WEBM, PNG, BMP, GIF, WEBP, MKV
- **Parallel Processing**: Read multiple files simultaneously
- **Memory Optimized**: Efficient memory usage for large-scale operations
- **Python Native**: Clean Python API with proper error handling

## Installation

### From Source (Development)

```bash
# Install maturin for building Python extensions
pip install maturin

# Clone the repository
git clone https://github.com/dapperfu/fast-exif-rs.git
cd fast-exif-rs/fast-exif-rs-py

# Build and install in development mode
maturin develop
```

### From PyPI (Future Release)

```bash
pip install fast-exif-rs-py
```

## Quick Start

```python
import fast_exif_rs_py

# Read EXIF data from a file
metadata = fast_exif_rs_py.read_exif_file("path/to/image.jpg")
print(metadata)

# Read EXIF data from bytes
with open("image.jpg", "rb") as f:
    data = f.read()
metadata = fast_exif_rs_py.read_exif_bytes(data)
print(metadata)

# Read multiple files in parallel
file_paths = ["image1.jpg", "image2.jpg", "image3.jpg"]
all_metadata = fast_exif_rs_py.read_exif_files_parallel(file_paths)
for i, metadata in enumerate(all_metadata):
    print(f"Image {i+1}: {metadata}")
```

## Object-Oriented API

```python
import fast_exif_rs_py

# Create a reader instance
reader = fast_exif_rs_py.PyFastExifReader()

# Read EXIF data
metadata = reader.read_file("path/to/image.jpg")
print(metadata)

# Read from bytes
with open("image.jpg", "rb") as f:
    data = f.read()
metadata = reader.read_bytes(data)
print(metadata)

# Process multiple files
file_paths = ["image1.jpg", "image2.jpg", "image3.jpg"]
all_metadata = reader.read_files_parallel(file_paths)
```

## Writing EXIF Data

```python
import fast_exif_rs_py

# Create a writer instance
writer = fast_exif_rs_py.PyFastExifWriter()

# Prepare metadata
metadata = {
    "Make": "Canon",
    "Model": "EOS R5",
    "DateTime": "2024:01:01 12:00:00",
    "ISO": "100",
    "FNumber": "2.8",
    "FocalLength": "50.0"
}

# Write EXIF to a new file
writer.write_exif("input.jpg", "output.jpg", metadata)

# Write EXIF to bytes
with open("input.jpg", "rb") as f:
    input_data = f.read()
output_data = writer.write_exif_to_bytes(input_data, metadata)
with open("output.jpg", "wb") as f:
    f.write(output_data)
```

## Copying EXIF Data

```python
import fast_exif_rs_py

# Create a copier instance
copier = fast_exif_rs_py.PyFastExifCopier()

# Copy high-priority EXIF fields
copier.copy_high_priority_exif("source.jpg", "target.jpg", "output.jpg")

# Copy all EXIF fields
copier.copy_all_exif("source.jpg", "target.jpg", "output.jpg")

# Copy specific fields
fields_to_copy = ["Make", "Model", "DateTime", "ISO"]
copier.copy_specific_exif("source.jpg", "target.jpg", "output.jpg", fields_to_copy)

# Get available fields from source
available_fields = copier.get_available_fields("source.jpg")
print("Available fields:", available_fields)

# Get high-priority fields
high_priority_fields = copier.get_high_priority_fields("source.jpg")
print("High-priority fields:", high_priority_fields)
```

## Utility Functions

```python
import fast_exif_rs_py

# Get version information
version = fast_exif_rs_py.get_version()
print(f"fast-exif-rs-py version: {version}")

# Get supported formats
formats = fast_exif_rs_py.get_supported_formats()
print("Supported formats:", formats)
```

## Performance

The Python bindings maintain the high performance of the underlying Rust implementation:

- **Parallel Processing**: Multiple files are processed simultaneously using Rust's rayon
- **Memory Mapping**: Efficient file I/O using memory-mapped files
- **Zero-Copy Parsing**: Minimal data copying during EXIF extraction
- **SIMD Optimizations**: Vectorized operations where available

## Error Handling

All functions raise appropriate Python exceptions on errors:

```python
import fast_exif_rs_py

try:
    metadata = fast_exif_rs_py.read_exif_file("nonexistent.jpg")
except RuntimeError as e:
    print(f"Error reading EXIF: {e}")
```

## Requirements

- Python 3.8+
- Rust 1.70+ (for building from source)

## License

MIT License - see LICENSE file for details.

## Contributing

Contributions are welcome! Please see the main [fast-exif-rs](https://github.com/dapperfu/fast-exif-rs) repository for contribution guidelines.
# fast-exif-rs-py
# fast-exif-rs-py
