# OpenSSL - Pre-built Multi-Platform Library

## Overview

This directory contains pre-built [OpenSSL](https://www.openssl.org/) libraries for multiple platforms and architectures. OpenSSL is a robust, commercial-grade, full-featured toolkit for general-purpose cryptography and secure communication.

## Source

These pre-built libraries are automatically compiled via GitHub Actions:

- **Workflow file**: `.github/workflows/build-openssl-all-platforms.yml`
- **Source code**: https://github.com/openssl/openssl
- **Default version**: 3.6.1

### Triggering a Build

On the GitHub repository page, go to **Actions** → **Build OpenSSL All Platforms** → **Run workflow**, where you can specify:
- `openssl_version`: Version to build (e.g., 3.6.1)
- `platforms`: Platforms to build (windows,macos,ios,android,linux)

## Supported Platforms

| Platform | Architectures | Library Type | Notes |
|----------|---------------|--------------|-------|
| Windows | x64, x86, arm64 | Shared (.dll) + Static (.lib) | Both libssl and libcrypto |
| macOS | Intel (x86_64), Apple Silicon (arm64) | Shared (.dylib) | RPATH set to @rpath |
| iOS | arm64 (device), arm64+x86_64 (simulator) | Static (.a) / XCFramework | libssl + libcrypto XCFrameworks |
| Android | arm64-v8a, armeabi-v7a, x86_64, x86 | Shared (.so) | RPATH set to $ORIGIN |
| Linux | x64 | Shared (.so) | RPATH set to $ORIGIN |

## Directory Structure

```
openssl/
├── CMakeLists.txt          # Platform selector
├── README.md               # This file
├── windows/
│   ├── CMakeLists.txt
│   ├── x64/
│   │   ├── bin/            # libssl-3-x64.dll, libcrypto-3-x64.dll
│   │   ├── include/        # openssl/*.h
│   │   ├── lib/            # libssl.lib, libcrypto.lib (import libs)
│   │   └── static/         # libssl_static.lib, libcrypto_static.lib
│   ├── x86/
│   └── arm64/
├── macosx/
│   ├── CMakeLists.txt
│   ├── intel/
│   │   ├── include/
│   │   └── lib/            # libssl.dylib, libcrypto.dylib
│   └── arm/
│       ├── include/
│       └── lib/
├── ios/
│   ├── CMakeLists.txt
│   ├── include/            # Shared headers
│   ├── libssl.xcframework/
│   └── libcrypto.xcframework/
├── android/
│   ├── CMakeLists.txt
│   ├── arm64-v8a/
│   │   ├── include/
│   │   └── lib/            # libssl.so, libcrypto.so
│   ├── armeabi-v7a/
│   ├── x86_64/
│   └── x86/
└── linux/
    ├── CMakeLists.txt
    └── x64/
        ├── include/
        └── lib/            # libssl.so, libcrypto.so
```

## Usage with CMake

### Method 1: Add as Subdirectory (Recommended)

```cmake
# In your CMakeLists.txt
add_subdirectory(path/to/openssl/openssl)

# Link to your target (both ssl and crypto are included)
target_link_libraries(your_target PRIVATE openssl)
```

### Method 2: Using find_package

```cmake
# Set search path
set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/path/to/openssl/openssl/${PLATFORM}/${ARCH}")

find_package(OpenSSL REQUIRED)
target_link_libraries(your_target PRIVATE OpenSSL::SSL OpenSSL::Crypto)
```

## Build Configuration

OpenSSL is built with the following configuration:

### Windows
```bash
perl Configure VC-WIN64A shared --prefix=...
nmake
nmake install
```

### macOS / Linux
```bash
./Configure darwin64-arm64-cc shared --prefix=...  # macOS ARM
./Configure darwin64-x86_64-cc shared --prefix=... # macOS Intel
./Configure linux-x86_64 shared --prefix=...       # Linux
make -j$(nproc)
make install
```

### iOS (Static)
```bash
./Configure ios64-xcrun no-shared --prefix=...     # Device
./Configure iossimulator-arm64-xcrun no-shared ... # Simulator ARM64
./Configure iossimulator-x86_64-xcrun no-shared ...# Simulator x86_64
```

### Android
```bash
./Configure android-arm64 shared --prefix=... -D__ANDROID_API__=24
./Configure android-arm shared --prefix=... -D__ANDROID_API__=24
./Configure android-x86_64 shared --prefix=... -D__ANDROID_API__=24
./Configure android-x86 shared --prefix=... -D__ANDROID_API__=24
```

## Libraries Provided

OpenSSL provides two main libraries:

| Library | Description |
|---------|-------------|
| **libssl** | Implementation of TLS/SSL protocols |
| **libcrypto** | Core cryptographic functions (AES, RSA, SHA, etc.) |

Most applications need both. The CMake wrapper links them together as a single `openssl` target.

## Runtime Deployment

### Windows
Copy the following DLLs to your executable directory:
- `libssl-3-x64.dll` (or `-x86` / `-arm64`)
- `libcrypto-3-x64.dll`

### macOS
The libraries' `LC_ID_DYLIB` is set to `@rpath/libssl.dylib` and `@rpath/libcrypto.dylib`. Ensure your application's RPATH includes the library directory.

### Linux / Android
The libraries' RPATH is set to `$ORIGIN`. Place the `.so` files in the same directory as your executable.

### iOS
Use the XCFrameworks. Drag `libssl.xcframework` and `libcrypto.xcframework` into your Xcode project.

## Version History

| Version | Build Date | Notes |
|---------|------------|-------|
| 3.6.1 | - | Current version |

## Related Links

- [OpenSSL Official Website](https://www.openssl.org/)
- [OpenSSL Documentation](https://www.openssl.org/docs/)
- [OpenSSL GitHub Repository](https://github.com/openssl/openssl)
- [OpenSSL Wiki](https://wiki.openssl.org/)

## License

OpenSSL is licensed under the [Apache License 2.0](https://www.openssl.org/source/license.html) (since OpenSSL 3.0).
