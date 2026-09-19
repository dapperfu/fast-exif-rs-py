# fast-exif-rs-py

Python bindings for [fast-exif-rs](https://github.com/dapperfu/fast-exif-rs).

I needed ExifTool-level tags from Python without paying for Perl.

ExifTool still wins on coverage, but it's a Perl process. PyExifTool and friends just talk to that process — fine for a handful of files, miserable on a photo library. In-process libraries (Pillow, piexif, exifread) skip maker notes, GPS, most RAWs, and a lot of the computed fields.

This package is the Rust crate compiled as a Python extension. It memory-maps the file, finds the metadata (JPEG APP1, TIFF IFDs, HEIF boxes, video atoms), parses those bits only, and names/formats fields the way ExifTool does. Parallel reads use rayon.

Reads are the point. There's a writer and a copier; treat them as experimental.

## Install

Python 3.8+. Building from source also needs Rust 1.70+.

```bash
pip install git+https://github.com/dapperfu/fast-exif-rs-py.git@v0.3.1
```

From a clone (or an editable install):

```bash
git clone https://github.com/dapperfu/fast-exif-rs-py.git
cd fast-exif-rs-py
pip install .
# or: pip install -e .
```

`pip install fast-exif-rs-py` is not on PyPI yet.

## Usage

```python
import fast_exif_rs_py

tags = fast_exif_rs_py.read_exif_file("photo.jpg")
print(tags["Make"], tags["Model"])

with open("photo.jpg", "rb") as f:
    tags = fast_exif_rs_py.read_exif_bytes(f.read())

all_tags = fast_exif_rs_py.read_exif_files_parallel(["a.jpg", "b.jpg", "c.jpg"])
```

Same thing with a reader you keep around:

```python
reader = fast_exif_rs_py.PyFastExifReader()
tags = reader.read_file("photo.jpg")
```

Writer / copier (experimental):

```python
writer = fast_exif_rs_py.PyFastExifWriter()
writer.write_exif("in.jpg", "out.jpg", {"Make": "Canon", "ISO": "100"})

copier = fast_exif_rs_py.PyFastExifCopier()
copier.copy_high_priority_exif("src.jpg", "dst.jpg", "out.jpg")
copier.copy_specific_exif("src.jpg", "dst.jpg", "out.jpg", ["Make", "Model", "DateTime"])
```

Failures raise `RuntimeError`. `get_supported_formats()` and `get_version()` are there if you need them.

## Formats

JPEG, TIFF, PNG, BMP, GIF, WEBP, HEIF/HIF, CR2, NEF, ARW, RAF, SRW, PEF, RW2, ORF, DNG, MOV/MP4/3GP, AVI, WMV, WEBM, MKV.

Maker notes are best on Canon/Nikon. Other cameras usually still get the standard EXIF/GPS tags.

## License

MIT. ExifTool is Phil Harvey's; this just tries to be faster at the subset I actually use.
