# Compression Library

This directory contains the zstd compression library with a unified CMake interface.

## Backend

**zstd 1.5.6** - Fast lossless compression algorithm by Facebook

- Source: https://github.com/facebook/zstd
- The `zstd/` directory is a complete copy of the official repository (no modifications)

## Usage

### CMake

```cmake
# In your CMakeLists.txt
add_subdirectory(thirdparty/compression)
target_link_libraries(YourTarget PRIVATE compression::core)
```

### C++ Code

```cpp
#include <zstd.h>

// Compression example
size_t compressedSize = ZSTD_compress(dst, dstCapacity, src, srcSize, compressionLevel);

// Decompression example
size_t decompressedSize = ZSTD_decompress(dst, dstCapacity, src, compressedSize);
```

## Directory Structure

```
compression/
├── CMakeLists.txt          # Unified entry point (configures and adds zstd)
├── README.md               # This file
└── zstd/                   # Official zstd repository (v1.5.6)
    ├── build/
    │   └── cmake/          # CMake entry point for zstd
    ├── lib/                # zstd library source
    └── ...                 # Other official zstd files
```

## Build Configuration

The following zstd options are pre-configured:

| Option | Value | Description |
|--------|-------|-------------|
| ZSTD_BUILD_STATIC | ON | Build static library |
| ZSTD_BUILD_SHARED | OFF | Skip shared library |
| ZSTD_BUILD_PROGRAMS | OFF | Skip command-line tools |
| ZSTD_BUILD_TESTS | OFF | Skip tests |
| ZSTD_MULTITHREAD_SUPPORT | ON | Enable multi-threading |

## License

zstd is dual-licensed under BSD and GPLv2.
