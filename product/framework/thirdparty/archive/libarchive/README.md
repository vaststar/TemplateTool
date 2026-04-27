# libarchive - Multi-format Archive Library

## Overview

Multi-format archive support (tar, tar.gz, tar.bz2 stub, cpio, etc.) using
[libarchive](https://github.com/libarchive/libarchive).

This is the backend used by `ArchiveWrapper` for everything that is **not**
plain ZIP (which is handled by `minizip-ng` / `archive::zip`).

## Source

- **Repository**: https://github.com/libarchive/libarchive
- **Version**: 3.8.7
- **License**: BSD-2-Clause

## Features

- tar / pax read & write
- gzip (DEFLATE) compression — zlib-ng statically linked
- Cross-platform: Windows, macOS, Linux, iOS, Android
- Modern C API (`archive.h`, `archive_entry.h`)

## CMake Target

| Target          | Description                                              |
|-----------------|----------------------------------------------------------|
| `archive::multi`| Interface library for multi-format archive operations    |

The alias is created uniformly on every platform; downstream code does not
need any per-platform branching:

```cmake
target_link_libraries(your_target PRIVATE archive::multi)
```

## Enabling the backend

`libarchive` is **opt-in**. Enable it from the top-level archive thirdparty:

```cmake
# product/framework/thirdparty/archive/CMakeLists.txt
option(UCF_ENABLE_LIBARCHIVE "Enable multi-format libarchive backend" ON)
```

Or pass `-DUCF_ENABLE_LIBARCHIVE=ON` on the cmake command line.

## Usage Example

```cpp
#include <archive.h>
#include <archive_entry.h>

// Extract a tar.gz file
void extract_tar_gz(const char* path, const char* dest_dir) {
    struct archive* a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);

    if (archive_read_open_filename(a, path, 10240) == ARCHIVE_OK) {
        struct archive_entry* entry;
        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            // ... write entry to dest_dir ...
            archive_read_data_skip(a);
        }
    }
    archive_read_free(a);
}
```

## Dependencies

- **zlib-ng**: Statically linked into the libarchive binary at build time
  (no runtime dependency on a separate zlib).

## Build Configuration

Built with the minimum feature set required by the project; everything
optional that pulls in extra third-party code is disabled.

| Option            | Value | Description                |
|-------------------|-------|----------------------------|
| `ENABLE_ZLIB`     | ON    | gzip (DEFLATE)             |
| `ENABLE_TAR`      | ON\*  | bsdtar CLI (off on iOS)    |
| `ENABLE_BZip2`    | OFF   | Disabled                   |
| `ENABLE_LZMA`     | OFF   | Disabled                   |
| `ENABLE_ZSTD`     | OFF   | Disabled                   |
| `ENABLE_LZ4`      | OFF   | Disabled                   |
| `ENABLE_LIBXML2`  | OFF   | Disabled                   |
| `ENABLE_EXPAT`    | OFF   | Disabled                   |
| `ENABLE_ICONV`    | OFF   | Disabled                   |
| `ENABLE_OPENSSL`  | OFF   | Disabled                   |
| `ENABLE_NETTLE`   | OFF   | Disabled                   |
| `ENABLE_XATTR`    | OFF   | Disabled                   |
| `ENABLE_ACL`      | OFF   | Disabled                   |
| `ENABLE_CPIO`     | OFF   | Disabled                   |
| `ENABLE_CAT`      | OFF   | Disabled                   |
| `ENABLE_TEST`     | OFF   | Disabled                   |

\* `ENABLE_TAR` and `ENABLE_UNZIP` are forced OFF on iOS because the bsdtar
CLI cannot install without a `MACOSX_BUNDLE` destination on Apple SDKs.
