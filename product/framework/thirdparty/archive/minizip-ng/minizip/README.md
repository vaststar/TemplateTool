# minizip-ng Prebuilt Libraries

This directory contains prebuilt minizip-ng libraries with DEFLATE compression support.

## Features

- **DEFLATE compression** (zlib statically linked - no external zlib dependency)
- Cross-platform support: Windows, macOS, Linux, iOS, Android
- Both shared (.dll/.dylib/.so) and static (.lib/.a) libraries

## Building

The libraries are built using GitHub Actions:

```bash
# Run the workflow manually from GitHub Actions UI
# Workflow: .github/workflows/build-minizip-all-platforms.yml
```

Or trigger via GitHub CLI:
```bash
gh workflow run build-minizip-all-platforms.yml \
  -f minizip_version=4.0.7 \
  -f zlib_version=2.2.4 \
  -f platforms=windows,macos,ios,android,linux
```

## Directory Structure

```
minizip/
├── windows/
│   ├── x64/
│   │   ├── include/     # Headers
│   │   ├── lib/         # Import libraries (.lib)
│   │   ├── bin/         # DLLs (.dll)
│   │   └── static/      # Static libraries (.lib)
│   ├── x86/
│   └── arm64/
├── macosx/
│   ├── intel/           # x86_64
│   │   ├── include/
│   │   ├── lib/         # Shared (.dylib)
│   │   └── static/      # Static (.a)
│   └── arm/             # Apple Silicon
├── linux/
│   └── x64/
│       ├── include/
│       ├── lib/         # Shared (.so)
│       └── static/      # Static (.a)
├── ios/
│   ├── include/
│   └── libminizip.xcframework/
└── android/
    ├── arm64-v8a/
    ├── armeabi-v7a/
    ├── x86_64/
    └── x86/
```

## Usage in CMake

```cmake
# The library is automatically available as:
target_link_libraries(your_target PRIVATE archive::zip)
# or
target_link_libraries(your_target PRIVATE minizip)
```

## Compression Support

| Algorithm | Status |
|-----------|--------|
| DEFLATE   | ✅ Enabled (zlib statically linked) |
| BZIP2     | ❌ Disabled |
| LZMA      | ❌ Disabled |
| ZSTD      | ❌ Disabled |

## Encryption Support

| Feature | Status |
|---------|--------|
| PKWARE  | ❌ Disabled |
| WinZIP AES | ❌ Disabled |
| OpenSSL | ❌ Disabled |
