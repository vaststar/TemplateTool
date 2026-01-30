# Compression Library - zlib-ng

This directory contains the zlib-ng library for DEFLATE/gzip compression.

## Backend

**zlib-ng 2.2.4** - High-performance zlib replacement

- Source: https://github.com/zlib-ng/zlib-ng
- The `zlib-ng/` directory is a complete copy of the official repository (no modifications)

## Features

- DEFLATE compression algorithm (RFC 1951)
- gzip format support (RFC 1952)
- zlib format support (RFC 1950)
- SIMD optimizations (SSE2, AVX2, NEON)
- Drop-in replacement for zlib

## Usage

### CMake

```cmake
# In your CMakeLists.txt
target_link_libraries(YourTarget PRIVATE compression::gzip)
```

### C++ Code

```cpp
#include <zlib.h>

// gzip compression example
gzFile file = gzopen("output.gz", "wb");
gzwrite(file, data, size);
gzclose(file);

// In-memory compression
z_stream strm;
deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
// 15 + 16 = gzip format
```

## Update Instructions

To update zlib-ng:

```bash
cd zlib-ng
rm -rf zlib-ng
git clone --depth 1 https://github.com/zlib-ng/zlib-ng.git zlib-ng
rm -rf zlib-ng/.git
# Update version in CMakeLists.txt
```
