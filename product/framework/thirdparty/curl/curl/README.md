# libcurl - Pre-built Multi-Platform Library

## Overview

This directory contains pre-built [libcurl](https://curl.se/libcurl/) libraries for multiple platforms and architectures. libcurl is a free and easy-to-use client-side URL transfer library, supporting HTTP, HTTPS, FTP, and many other protocols.

## Source

These pre-built libraries are automatically compiled via GitHub Actions:

- **Workflow file**: `.github/workflows/build-libcurl-all-platforms.yml`
- **Source code**: https://github.com/curl/curl
- **Default version**: 8.18.0

### Triggering a Build

On the GitHub repository page, go to **Actions** → **Build libcurl All Platforms** → **Run workflow**, where you can specify:
- `curl_version`: Version to build (e.g., 8.18.0)
- `platforms`: Platforms to build (windows,macos,ios,android,linux)

## Supported Platforms

| Platform | Architectures | Library Type | SSL Backend |
|----------|---------------|--------------|-------------|
| Windows | x64, x86, arm64 | Shared (.dll) | Schannel (System SSL) |
| macOS | Intel (x86_64), Apple Silicon (arm64) | Shared (.dylib) | OpenSSL |
| iOS | arm64 (device), arm64+x86_64 (simulator) | Static (.a) / XCFramework | OpenSSL |
| Android | arm64-v8a, armeabi-v7a, x86_64, x86 | Shared (.so) | OpenSSL |
| Linux | x64 | Shared (.so) | OpenSSL |

## Directory Structure

```
curl/
├── CMakeLists.txt          # Platform selector
├── README.md               # This file
├── windows/
│   ├── CMakeLists.txt
│   ├── x64/
│   │   ├── bin/            # libcurl.dll, libcurl-d.dll
│   │   ├── include/        # curl/*.h
│   │   └── lib/            # libcurl.lib, libcurl-d.lib
│   ├── x86/
│   └── arm64/
├── macosx/
│   ├── CMakeLists.txt
│   ├── intel/
│   │   ├── include/
│   │   └── lib/            # libcurl.dylib
│   └── arm/
│       ├── include/
│       └── lib/
├── ios/
│   ├── CMakeLists.txt
│   ├── include/
│   └── libcurl.xcframework/
├── android/
│   ├── CMakeLists.txt
│   ├── arm64-v8a/
│   │   ├── include/
│   │   └── lib/            # libcurl.so
│   ├── armeabi-v7a/
│   ├── x86_64/
│   └── x86/
└── linux/
    ├── CMakeLists.txt
    └── x64/
        ├── include/
        └── lib/            # libcurl.so
```

## Usage with CMake

### Method 1: Add as Subdirectory (Recommended)

```cmake
# In your CMakeLists.txt
add_subdirectory(path/to/curl/curl)

# Link to your target
target_link_libraries(your_target PRIVATE libcurl)
```

### Method 2: Using find_package

```cmake
# Set search path
set(CURL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/path/to/curl/curl/${PLATFORM}/${ARCH}")

find_package(CURL REQUIRED)
target_link_libraries(your_target PRIVATE CURL::libcurl)
```

## Build Options

Main CMake options used during compilation:

```cmake
-DBUILD_SHARED_LIBS=ON          # Shared libraries (except iOS)
-DCURL_USE_OPENSSL=ON           # Use OpenSSL (Windows uses Schannel)
-DCURL_ZLIB=OFF                 # Disable zlib
-DBUILD_CURL_EXE=OFF            # Don't build curl CLI tool
-DBUILD_TESTING=OFF             # Don't build tests
-DCURL_DISABLE_LDAP=ON          # Disable LDAP
```

## Dependencies

- **Windows**: No external dependencies (uses system Schannel)
- **Other platforms**: Requires OpenSSL (located in `../openssl/` directory)

## Runtime Deployment

### Windows
Copy `libcurl.dll` to the same directory as your executable, or add to `PATH`.

### macOS
The library's `LC_ID_DYLIB` is set to `@rpath/libcurl.dylib`. Ensure your application's RPATH includes the library directory.

### Linux / Android
The library's RPATH is set to `$ORIGIN`. Place the `.so` file in the same directory as your executable.

### iOS
Use the XCFramework. Drag it into your Xcode project and it will automatically handle device/simulator architectures.

## Version History

| Version | Build Date | Notes |
|---------|------------|-------|
| 8.18.0 | - | Current version |

## Related Links

- [libcurl Official Website](https://curl.se/libcurl/)
- [libcurl API Documentation](https://curl.se/libcurl/c/)
- [curl GitHub Repository](https://github.com/curl/curl)

## License

libcurl is licensed under the [curl license](https://curl.se/docs/copyright.html) (MIT/X derivative license).
