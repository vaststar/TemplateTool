# OpenCV - Pre-built Multi-Platform Library

## Overview

This directory contains pre-built [OpenCV](https://opencv.org/) libraries for multiple platforms and architectures. OpenCV (Open Source Computer Vision Library) is an open-source computer vision and machine learning software library with optimized algorithms for image and video processing.

## Source

These pre-built libraries are automatically compiled via GitHub Actions:

- **Workflow file**: `.github/workflows/build-opencv-all-platforms.yml`
- **Source code**: https://github.com/opencv/opencv
- **Default version**: 4.13.0

### Triggering a Build

On the GitHub repository page, go to **Actions** → **Build OpenCV All Platforms** → **Run workflow**, where you can specify:
- `opencv_version`: Version to build (e.g., 4.13.0)
- `platforms`: Platforms to build (windows,macos,ios,android,linux)

## Supported Platforms

| Platform | Architectures | Library Type | Build Configuration |
|----------|---------------|--------------|---------------------|
| Windows | x64, x86 | Shared (.dll) | Debug + Release, vc17 |
| macOS | Intel (x86_64), Apple Silicon (arm64) | Shared (.dylib) | Release only |
| iOS | arm64 (device), arm64+x86_64 (simulator) | Static (.a) / XCFramework | Release only |
| Android | arm64-v8a, armeabi-v7a, x86_64, x86 | Shared (.so) | Release only |
| Linux | x64 | Shared (.so) | Release only |

## Directory Structure

```
opencv/
├── CMakeLists.txt              # Platform selector
├── README.md                   # This file
├── windows/
│   ├── CMakeLists.txt
│   ├── include/                # Shared headers (opencv2/*.hpp)
│   ├── x64/
│   │   └── vc17/
│   │       ├── bin/            # opencv_world4100.dll, opencv_world4100d.dll
│   │       └── lib/            # opencv_world4100.lib, opencv_world4100d.lib
│   └── x86/
│       └── vc17/
│           ├── bin/
│           └── lib/
├── macosx/
│   ├── CMakeLists.txt
│   ├── include/                # Shared headers
│   ├── intel/
│   │   └── lib/                # libopencv_world.dylib
│   └── arm/
│       └── lib/
├── ios/
│   ├── CMakeLists.txt
│   ├── include/                # Shared headers
│   └── opencv2.xcframework/    # Universal framework
├── android/
│   ├── CMakeLists.txt
│   ├── include/                # Shared headers
│   └── libs/
│       ├── arm64-v8a/          # libopencv_java4.so
│       ├── armeabi-v7a/
│       ├── x86_64/
│       └── x86/
└── linux/
    ├── CMakeLists.txt
    └── x64/
        ├── include/
        └── lib/                # libopencv_world.so
```

## Usage with CMake

### Method 1: Add as Subdirectory (Recommended)

```cmake
# In your CMakeLists.txt
add_subdirectory(path/to/opencv/opencv)

# Link to your target
target_link_libraries(your_target PRIVATE opencv)
```

### Method 2: Using find_package

```cmake
# Set search path
set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/path/to/opencv/opencv/${PLATFORM}/${ARCH}/lib/cmake/opencv4")

find_package(OpenCV REQUIRED)
target_link_libraries(your_target PRIVATE ${OpenCV_LIBS})
```

## Build Configuration

OpenCV is built with the following key options:

```cmake
-DBUILD_SHARED_LIBS=ON          # Shared libraries (except iOS)
-DBUILD_opencv_world=ON         # Single unified library
-DBUILD_TESTS=OFF               # No tests
-DBUILD_PERF_TESTS=OFF          # No performance tests
-DBUILD_EXAMPLES=OFF            # No examples
-DBUILD_DOCS=OFF                # No documentation
-DBUILD_opencv_python3=OFF      # No Python bindings
-DBUILD_opencv_java=ON          # Java bindings (Android only)
-DWITH_FFMPEG=ON                # FFmpeg support (desktop)
-DWITH_CUDA=OFF                 # No CUDA
-DOPENCV_ENABLE_NONFREE=OFF     # No non-free algorithms
```

### Platform-Specific Notes

- **Windows**: Built with Visual Studio 2022 (vc17), both Debug and Release
- **macOS**: Deployment target 11.0, kleidicv disabled for Intel
- **iOS**: Deployment target 13.0, static libraries only
- **Android**: API level 24+, uses c++_shared STL

## opencv_world Module

The libraries are built with `BUILD_opencv_world=ON`, which combines all OpenCV modules into a single library:

| Platform | Library Name |
|----------|--------------|
| Windows | `opencv_world4100.dll` / `opencv_world4100d.dll` |
| macOS | `libopencv_world.dylib` |
| iOS | `opencv2.xcframework` |
| Android | `libopencv_java4.so` |
| Linux | `libopencv_world.so` |

This simplifies linking - you only need to link against one library.

## Runtime Deployment

### Windows
Copy the following to your executable directory:
- Release: `opencv_world4100.dll`
- Debug: `opencv_world4100d.dll`

### macOS
The library's `LC_ID_DYLIB` is set to `@rpath/libopencv_world.dylib`. Ensure your application's RPATH includes the library directory.

### Linux / Android
The library's RPATH is set to `$ORIGIN`. Place the `.so` file in the same directory as your executable.

### iOS
Use the XCFramework. Drag `opencv2.xcframework` into your Xcode project.

## Common Use Cases

```cpp
#include <opencv2/opencv.hpp>

// Image loading and display
cv::Mat image = cv::imread("image.jpg");
cv::imshow("Display", image);

// Video capture
cv::VideoCapture cap(0);
cv::Mat frame;
cap >> frame;

// Image processing
cv::Mat gray, blurred;
cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
```

## Version History

| Version | Build Date | Notes |
|---------|------------|-------|
| 4.13.0 | - | Current version |

## Related Links

- [OpenCV Official Website](https://opencv.org/)
- [OpenCV Documentation](https://docs.opencv.org/)
- [OpenCV GitHub Repository](https://github.com/opencv/opencv)
- [OpenCV Tutorials](https://docs.opencv.org/4.x/d9/df8/tutorial_root.html)

## License

OpenCV is licensed under the [Apache License 2.0](https://opencv.org/license/).
