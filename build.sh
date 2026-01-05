#!/bin/bash
set -e

# ==========================================
# 🧱 Basic configuration
# ==========================================
PRESET="${1:-macos-release}"
ACTION="${2:-all}"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build/$PRESET"
GRAPHVIZ_DIR="$BUILD_DIR/cmake_graph"
GRAPHVIZ_FILE="$GRAPHVIZ_DIR/cmake_graph.dot"

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
# 📋 Display configuration summary
# ==========================================
echo "****************************************************"
echo "Start CMake Presets workflow"
echo "ROOT_DIR   : $ROOT_DIR"
echo "BUILD_DIR  : $BUILD_DIR"
echo "CMAKE      : $CMAKE"
echo "PRESET     : $PRESET"
echo "ACTION     : $ACTION"
echo "****************************************************"

# ==========================================
# 🧩 Ensure graphviz directory exists
# ==========================================
ensure_graphviz_dir() {
    if [ ! -d "$GRAPHVIZ_DIR" ]; then
        mkdir -p "$GRAPHVIZ_DIR"
    fi
}

# ==========================================
# 🔧 Check if project is configured
# ==========================================
is_configured() {
    [ -f "$BUILD_DIR/CMakeCache.txt" ]
}

# ==========================================
# 🔧 Ensure project is configured
# ==========================================
ensure_configured() {
    if ! is_configured; then
        echo "[INFO] Project not configured yet, configuring first..."
        ensure_graphviz_dir
        "$CMAKE" --preset "$PRESET" --graphviz="$GRAPHVIZ_FILE"
        if [ $? -ne 0 ]; then
            echo "[ERROR] Configuration failed."
            exit 1
        fi
    fi
}

# ==========================================
# 🚀 Execute action
# ==========================================
case "$ACTION" in
    configure|config|generate)
        ensure_graphviz_dir
        echo "[INFO] Configuring project..."
        "$CMAKE" --preset "$PRESET" --graphviz="$GRAPHVIZ_FILE"
        EXIT_CODE=$?
        ;;
    
    build)
        ensure_configured
        echo "[INFO] Building project..."
        "$CMAKE" --build --preset "$PRESET"
        EXIT_CODE=$?
        ;;
    
    clean)
        if is_configured; then
            echo "[INFO] Cleaning build directory..."
            "$CMAKE" --build --preset "$PRESET" --target clean
            EXIT_CODE=$?
        else
            echo "[WARNING] Project not configured, nothing to clean."
            EXIT_CODE=0
        fi
        ;;
    
    install)
        ensure_configured
        echo "[INFO] Building and installing..."
        "$CMAKE" --build --preset "$PRESET"
        if [ $? -eq 0 ]; then
            "$CMAKE" --install "$BUILD_DIR"
            EXIT_CODE=$?
        else
            EXIT_CODE=$?
        fi
        ;;
    
    package)
        ensure_configured
        echo "[INFO] Building and packaging..."
        "$CMAKE" --build --preset "$PRESET"
        if [ $? -eq 0 ]; then
            # 在根目录运行 cpack，指定 build 目录
            cpack --preset "$PRESET" -B "$BUILD_DIR"
            EXIT_CODE=$?
        else
            EXIT_CODE=$?
        fi
        ;;
    
    all|*)
        ensure_graphviz_dir
        echo "[INFO] Configure + Build"
        "$CMAKE" --preset "$PRESET" --graphviz="$GRAPHVIZ_FILE"
        EXIT_CODE=$?
        if [ $EXIT_CODE -eq 0 ]; then
            "$CMAKE" --build --preset "$PRESET"
            EXIT_CODE=$?
        fi
        ;;
esac

# ==========================================
# 📦 Result summary
# ==========================================
if [ $EXIT_CODE -ne 0 ]; then
    echo "[ERROR] Operation failed with code $EXIT_CODE."
    exit $EXIT_CODE
else
    echo "****************************************************"
    echo "[SUCCESS] Operation completed successfully."
    echo "Build files are located in: $BUILD_DIR"
    if [ "$ACTION" = "package" ] && [ -d "$BUILD_DIR" ]; then
        echo "Package files are located in: $BUILD_DIR"
    fi
    echo "****************************************************"
fi

exit $EXIT_CODE