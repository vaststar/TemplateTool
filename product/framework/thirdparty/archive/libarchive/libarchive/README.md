# libarchive Prebuilt Libraries

This directory contains prebuilt **libarchive 3.8.7** with gzip support
(zlib-ng statically linked).

## Features

- tar / pax read & write
- gzip (DEFLATE) — zlib-ng statically linked, no external zlib dependency
- Cross-platform: Windows, macOS, Linux, iOS, Android
- Shared libraries on desktop / Android, static libraries / XCFramework on iOS

## Building

The libraries are produced by GitHub Actions:

- Workflow: `.github/workflows/build-libarchive-all-platforms.yml`

Trigger via GitHub CLI:

```bash
gh workflow run build-libarchive-all-platforms.yml \
  -f libarchive_version=3.8.7 \
  -f zlib_version=2.3.3 \
  -f platforms=windows,macos,ios,android,linux
```

The workflow uploads one artifact per platform/arch and then a final
`libarchive-<version>-<platform>.zip` from the `merge-artifacts` job — that
merged archive is what should be unpacked into this directory.

## Directory Structure

```
libarchive/
├── windows/
│   ├── x64/
│   │   ├── include/             # archive.h, archive_entry.h
│   │   ├── lib/                 # archive.lib (import), archive_static.lib
│   │   └── bin/                 # archive.dll
│   ├── x86/
│   └── arm64/
├── macosx/
│   ├── intel/                   # x86_64
│   │   ├── include/
│   │   └── lib/                 # libarchive.dylib + symlinks
│   └── arm/                     # Apple Silicon
├── linux/
│   └── x64/
│       ├── include/
│       └── lib/                 # libarchive.so + symlinks
├── ios/
│   ├── include/
│   └── libarchive.xcframework/  # static, device + simulator (arm64 + x86_64)
└── android/
    ├── arm64-v8a/
    │   ├── include/
    │   └── lib/                 # libarchive.so
    ├── armeabi-v7a/
    ├── x86_64/
    └── x86/
```

## Usage in CMake

The platform dispatcher (`../CMakeLists.txt`) picks the right subdirectory,
which in turn defines a unified alias:

```cmake
target_link_libraries(your_target PRIVATE archive::multi)
```

## Compression Support

| Algorithm | Status                                         |
|-----------|------------------------------------------------|
| gzip      | ✅ Enabled (zlib-ng statically linked)         |
| bzip2     | ❌ Disabled                                    |
| LZMA / xz | ❌ Disabled                                    |
| zstd      | ❌ Disabled                                    |
| LZ4       | ❌ Disabled                                    |

## Format Support

| Format    | Read | Write |
|-----------|------|-------|
| tar / pax | ✅   | ✅    |
| cpio      | ❌   | ❌    |
| ZIP       | use `archive::zip` (minizip-ng) instead       ||

## Encryption Support

OpenSSL / Nettle backends are disabled. Encrypted archives are not supported.
