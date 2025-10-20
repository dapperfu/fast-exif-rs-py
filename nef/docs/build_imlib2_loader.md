Imlib2 NEF Loader (HE/HE* scaffold)

Build

- Ensure imlib2 development headers are available in the system include path.
- From /projects/nef/loaders:

```
make
```

This produces loader_nef.so in the same directory.

Run feh with userspace loader path

```
IMLIB2_LOADER_PATH=/projects/nef/loaders feh /path/to/file.NEF
```

Notes

- Current loader recognizes NEF/NRW container and returns unsupported for HE/HE* until decoder is implemented.
- The decoder will be integrated subsequently; no root install is required, only IMLIB2_LOADER_PATH.


