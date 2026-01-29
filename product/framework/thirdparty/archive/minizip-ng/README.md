# minizip-ng - ZIP Archive Library

## Overview

ZIP archive support using [minizip-ng](https://github.com/zlib-ng/minizip-ng) library.

## Source

- **Repository**: https://github.com/zlib-ng/minizip-ng
- **Version**: 4.0.7
- **License**: zlib License

## Features

- ZIP read/write support
- Deflate compression (via zlib)
- Cross-platform (Windows, macOS, Linux, iOS, Android)
- Modern C API

## CMake Target

| Target | Description |
|--------|-------------|
| `compression::archive` | Static library for ZIP operations |

## Usage Example

```cpp
#include <mz.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

// Create a ZIP file
void create_zip(const char* zip_path) {
    void* zip_writer = mz_zip_writer_create();
    
    if (mz_zip_writer_open_file(zip_writer, zip_path, 0, 0) == MZ_OK) {
        // Add file to zip
        mz_zip_writer_add_path(zip_writer, "file.txt", "file.txt", 0);
        mz_zip_writer_close(zip_writer);
    }
    
    mz_zip_writer_delete(&zip_writer);
}

// Extract a ZIP file
void extract_zip(const char* zip_path, const char* dest_dir) {
    void* zip_reader = mz_zip_reader_create();
    
    if (mz_zip_reader_open_file(zip_reader, zip_path) == MZ_OK) {
        mz_zip_reader_save_all(zip_reader, dest_dir);
        mz_zip_reader_close(zip_reader);
    }
    
    mz_zip_reader_delete(&zip_reader);
}
```

## Dependencies

- **zlib**: Automatically fetched and built by CMake

## Build Configuration

The library is configured with minimal dependencies:

| Option | Value | Description |
|--------|-------|-------------|
| MZ_ZLIB | ON | Deflate compression |
| MZ_BZIP2 | OFF | Disabled |
| MZ_LZMA | OFF | Disabled |
| MZ_ZSTD | OFF | Disabled |
| MZ_OPENSSL | OFF | No encryption |
