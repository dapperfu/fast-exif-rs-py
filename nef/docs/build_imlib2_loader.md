Imlib2 NEF Loader (Nikon HE* specific scaffold)

Build

- Ensure imlib2 development headers are available. You can supply flags via env:
  - IMLIB2_CFLAGS and IMLIB2_LIBS or rely on pkg-config imlib2
- From /projects/nef/loaders:

```
# Example with pkg-config available
make

# Or specify include/lib flags explicitly (userspace)
IMLIB2_CFLAGS="-I$HOME/.local/include/Imlib2" IMLIB2_LIBS="-L$HOME/.local/lib -lImlib2" make
```

This produces loader_nef.so in the same directory.

Run feh with userspace loader path (claims only HE* NEF; classic NEF remains with default loaders)

```
IMLIB2_LOADER_PATH=/projects/nef/loaders feh /path/to/file.NEF
```

Notes

- Current loader recognizes NEF/NRW container and returns unsupported for HE/HE* until decoder is implemented.
- The decoder will be integrated subsequently; no root install is required, only IMLIB2_LOADER_PATH.


