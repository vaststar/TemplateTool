#!/bin/bash
set -e

# ==========================================
# 🧱 Basic configuration
# ==========================================
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
SOURCE_DIR="$ROOT_DIR"
GRAPHVIZ_FILE="$BUILD_DIR/cmake_graph/cmake_graph.dot"

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
echo "Start generating project on macOS"
echo "ROOT_DIR   : $ROOT_DIR"
echo "SOURCE_DIR : $SOURCE_DIR"
echo "BUILD_DIR  : $BUILD_DIR"
echo "CMAKE      : $CMAKE"
echo "CC         : $CC"
echo "CXX        : $CXX"
echo "****************************************************"

# ==========================================
# 🚀 Run CMake configuration
# ==========================================
"$CMAKE" -S "$SOURCE_DIR" -B "$BUILD_DIR" \
    -G "Unix Makefiles" \
    --graphviz="$GRAPHVIZ_FILE" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$BUILD_DIR/bin" \
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$BUILD_DIR/bin" \
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$BUILD_DIR/bin" \
    -DCMAKE_INSTALL_PREFIX="$ROOT_DIR/install" \
    -DCMAKE_VERBOSE_MAKEFILE=ON

EXIT_CODE=$?

# ==========================================
# 📦 Result summary
# ==========================================
if [ $EXIT_CODE -ne 0 ]; then
    echo "[ERROR] CMake configuration failed with code $EXIT_CODE."
    exit $EXIT_CODE
else
    echo "****************************************************"
    echo "[SUCCESS] CMake project generated successfully."
    echo "Build files are located in: $BUILD_DIR"
    echo "****************************************************"
fi

exit $EXIT_CODE
