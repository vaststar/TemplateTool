#!/bin/bash
set -e

# ==========================================
# 🧱 Basic configuration
# ==========================================
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
TARGET_NAME="mainEntry"
BUILD_TYPE="Debug"

# ==========================================
# 🔍 Detect system CMake
# ==========================================
if command -v cmake >/dev/null 2>&1; then
    CMAKE=$(command -v cmake)
else
    echo "[ERROR] System CMake not found. Please install CMake via Homebrew:"
    echo "        brew install cmake"
    exit 1
fi

# ==========================================
# 🔍 Detect GCC and G++
# ==========================================
if command -v gcc >/dev/null 2>&1 && command -v g++ >/dev/null 2>&1; then
    export CC=gcc
    export CXX=g++
else
    echo "[ERROR] GCC or G++ not found. Please install via Homebrew:"
    echo "        brew install gcc"
    exit 1
fi

# ==========================================
# 🧩 Ensure build directory exists
# ==========================================
if [ ! -d "$BUILD_DIR" ]; then
    echo "[INFO] Build directory not found, creating..."
    mkdir -p "$BUILD_DIR"
fi

# ==========================================
# 📋 Display configuration summary
# ==========================================
echo "****************************************************"
echo "Start building project on macOS"
echo "ROOT_DIR    : $ROOT_DIR"
echo "BUILD_DIR   : $BUILD_DIR"
echo "CMAKE       : $CMAKE"
echo "CC         : $CC"
echo "CXX        : $CXX"
echo "TARGET_NAME : $TARGET_NAME"
echo "BUILD_TYPE  : $BUILD_TYPE"
echo "****************************************************"

# ==========================================
# 🚀 Run CMake build
# ==========================================
"$CMAKE" --build "$BUILD_DIR" \
    --target "$TARGET_NAME" \
    --config "$BUILD_TYPE" \
    --parallel "$(sysctl -n hw.ncpu)"

EXIT_CODE=$?

# ==========================================
# 📦 Build result summary
# ==========================================
if [ $EXIT_CODE -ne 0 ]; then
    echo "[ERROR] Build failed with code $EXIT_CODE."
    exit $EXIT_CODE
else
    echo "****************************************************"
    echo "[SUCCESS] Build succeeded."
    echo "Output directory: $BUILD_DIR/$BUILD_TYPE"
    echo "****************************************************"
fi

exit $EXIT_CODE
