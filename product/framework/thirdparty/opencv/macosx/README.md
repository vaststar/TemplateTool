# OpenCV macOS Libraries

This directory contains OpenCV prebuilt libraries for macOS.

## Directory Structure

```
macosx/
├── include/
│   └── opencv2/          # OpenCV headers (shared by all architectures)
├── intel/
│   └── lib/              # x86_64 libraries for Intel Macs
│       ├── libopencv_world.dylib
│       └── cmake/opencv4/
└── arm/
    └── lib/              # arm64 libraries for Apple Silicon Macs
        ├── libopencv_world.dylib
        └── cmake/opencv4/
```

## How to Build

Run the GitHub Actions workflow:

1. Go to Actions tab in GitHub
2. Select "Build OpenCV for macOS"
3. Click "Run workflow"
4. Enter OpenCV version (default: 4.12.0)
5. Download the `opencv-macos-combined` artifact
6. Extract and copy contents to this directory

## Version

Current version: 4.12.0 (matching Windows build)
