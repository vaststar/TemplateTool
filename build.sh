#!/bin/bash
set -e

# ==========================================
# Basic configuration
# ==========================================
PRESET="${1:-macos-release}"
ACTION="${2:-all}"

# Handle help request
if [[ "$PRESET" == "help" || "$PRESET" == "--help" || "$PRESET" == "-h" ]]; then
    echo ""
    echo "Usage: ./build.sh [PRESET] [ACTION]"
    echo ""
    echo "PRESET:"
    echo "  windows-msvc-debug      Windows MSVC Debug"
    echo "  windows-msvc-release    Windows MSVC Release"
    echo "  windows-mingw-debug     Windows MinGW Debug"
    echo "  windows-mingw-release   Windows MinGW Release"
    echo "  macos-debug             macOS Debug"
    echo "  macos-release           macOS Release (default)"
    echo "  linux-debug             Linux Debug"
    echo "  linux-release           Linux Release"
    echo ""
    echo "ACTION:"
    echo "  configure    Configure the project (generate build files)"
    echo "  build        Build the project"
    echo "  rebuild      Clean and rebuild the project"
    echo "  clean        Clean build artifacts"
    echo "  install      Build and install the project"
    echo "  package      Build and create package"
    echo "  test         Build and run tests"
    echo "  all          Configure and build (default)"
    echo ""
    echo "Examples:"
    echo "  ./build.sh                                # Default: macos-release all"
    echo "  ./build.sh macos-debug build              # Debug build only"
    echo "  ./build.sh macos-release install          # Release build and install"
    echo "  ./build.sh linux-release test             # Run tests"
    echo ""
    exit 0
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build/$PRESET"
GRAPHVIZ_DIR="$BUILD_DIR/cmake_graph"
GRAPHVIZ_FILE="$GRAPHVIZ_DIR/cmake_graph.dot"

# ==========================================
# Detect system CMake
# ==========================================
if command -v cmake >/dev/null 2>&1; then
    CMAKE=$(command -v cmake)
else
    echo "[ERROR] System CMake not found. Please install CMake:"
    echo "        macOS:  brew install cmake"
    echo "        Linux:  sudo apt install cmake"
    exit 1
fi

# ==========================================
# Display configuration summary
# ==========================================
START_TIME=$(date "+%Y-%m-%d %H:%M:%S")
echo ""
echo "****************************************************"
echo " CMake Presets Build System"
echo "****************************************************"
echo " Start Time : $START_TIME"
echo " Root Dir   : $ROOT_DIR"
echo " Build Dir  : $BUILD_DIR"
echo " CMake      : $CMAKE"
echo " Preset     : $PRESET"
echo " Action     : $ACTION"
echo "****************************************************"
echo ""

# ==========================================
# Helper functions
# ==========================================
ensure_graphviz_dir() {
    if [ ! -d "$GRAPHVIZ_DIR" ]; then
        mkdir -p "$GRAPHVIZ_DIR"
    fi
}

is_configured() {
    [ -f "$BUILD_DIR/CMakeCache.txt" ]
}

ensure_configured() {
    if ! is_configured; then
        echo ""
        echo "[Step 0] Project not configured, configuring first..."
        echo "----------------------------------------------------"
        ensure_graphviz_dir
        echo "  Preset    : $PRESET"
        echo "  Build Dir : $BUILD_DIR"
        echo ""
        "$CMAKE" --preset "$PRESET" --graphviz="$GRAPHVIZ_FILE"
        if [ $? -ne 0 ]; then
            echo ""
            echo "[ERROR] Configuration failed, cannot proceed."
            exit 1
        fi
        echo ""
    fi
}

do_all() {
    echo ""
    echo "[Step 1/2] Configuring project..."
    echo "----------------------------------------------------"
    ensure_graphviz_dir
    echo "  Preset    : $PRESET"
    echo "  Build Dir : $BUILD_DIR"
    echo ""
    "$CMAKE" --preset "$PRESET" --graphviz="$GRAPHVIZ_FILE"
    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 0 ]; then
        echo ""
        echo "[Step 2/2] Building project..."
        echo "----------------------------------------------------"
        "$CMAKE" --build --preset "$PRESET"
        EXIT_CODE=$?
    fi
}

# ==========================================
# Execute action
# ==========================================
case "$ACTION" in
    configure|config|generate)
        echo ""
        echo "[Step 1/1] Configuring project..."
        echo "----------------------------------------------------"
        ensure_graphviz_dir
        echo "  Preset    : $PRESET"
        echo "  Build Dir : $BUILD_DIR"
        echo "  Graphviz  : $GRAPHVIZ_FILE"
        echo ""
        "$CMAKE" --preset "$PRESET" --graphviz="$GRAPHVIZ_FILE"
        EXIT_CODE=$?
        ;;
    
    build)
        echo ""
        echo "[Step 1/1] Building project..."
        echo "----------------------------------------------------"
        ensure_configured
        echo "  Preset    : $PRESET"
        echo "  Build Dir : $BUILD_DIR"
        echo ""
        "$CMAKE" --build --preset "$PRESET"
        EXIT_CODE=$?
        ;;
    
    rebuild)
        echo ""
        echo "[Step 1/3] Cleaning previous build..."
        echo "----------------------------------------------------"
        if is_configured; then
            echo "  Cleaning $BUILD_DIR"
            "$CMAKE" --build --preset "$PRESET" --target clean
        else
            echo "  No previous build found, skipping clean."
        fi
        echo ""
        echo "[Step 2/3] Configuring project..."
        echo "----------------------------------------------------"
        ensure_graphviz_dir
        "$CMAKE" --preset "$PRESET" --graphviz="$GRAPHVIZ_FILE"
        EXIT_CODE=$?
        if [ $EXIT_CODE -eq 0 ]; then
            echo ""
            echo "[Step 3/3] Building project..."
            echo "----------------------------------------------------"
            "$CMAKE" --build --preset "$PRESET"
            EXIT_CODE=$?
        fi
        ;;
    
    clean)
        echo ""
        echo "[Step 1/1] Cleaning build artifacts..."
        echo "----------------------------------------------------"
        if is_configured; then
            echo "  Build Dir : $BUILD_DIR"
            echo ""
            "$CMAKE" --build --preset "$PRESET" --target clean
            EXIT_CODE=$?
        else
            echo "  [WARNING] Project not configured, nothing to clean."
            EXIT_CODE=0
        fi
        ;;
    
    install)
        echo ""
        echo "[Step 1/1] Building and installing..."
        echo "----------------------------------------------------"
        ensure_configured
        echo "  Preset     : $PRESET"
        echo "  Build Dir  : $BUILD_DIR"
        echo "  Install to : $ROOT_DIR/install/$PRESET"
        echo ""
        "$CMAKE" --build --preset "$PRESET" --target install
        EXIT_CODE=$?
        ;;
    
    package)
        echo ""
        echo "[Step 1/2] Building project..."
        echo "----------------------------------------------------"
        ensure_configured
        echo "  Preset    : $PRESET"
        echo "  Build Dir : $BUILD_DIR"
        echo ""
        "$CMAKE" --build --preset "$PRESET"
        BUILD_RESULT=$?
        if [ $BUILD_RESULT -eq 0 ]; then
            echo ""
            echo "[Step 2/2] Creating package..."
            echo "----------------------------------------------------"
            cpack --preset "$PRESET" -B "$BUILD_DIR"
            EXIT_CODE=$?
        else
            EXIT_CODE=$BUILD_RESULT
        fi
        ;;
    
    test)
        echo ""
        echo "[Step 1/2] Building project..."
        echo "----------------------------------------------------"
        ensure_configured
        echo "  Preset    : $PRESET"
        echo ""
        "$CMAKE" --build --preset "$PRESET"
        BUILD_RESULT=$?
        if [ $BUILD_RESULT -eq 0 ]; then
            echo ""
            echo "[Step 2/2] Running tests..."
            echo "----------------------------------------------------"
            ctest --preset "$PRESET"
            EXIT_CODE=$?
        else
            EXIT_CODE=$BUILD_RESULT
        fi
        ;;
    
    all)
        do_all
        ;;
    
    *)
        echo "[WARNING] Unknown action: $ACTION"
        echo "[INFO] Valid actions: configure, build, rebuild, clean, install, package, test, all"
        echo "[INFO] Falling back to 'all'"
        do_all
        ;;
esac

# ==========================================
# Result summary
# ==========================================
echo ""
echo "****************************************************"
if [ $EXIT_CODE -ne 0 ]; then
    echo " [FAILED] Operation failed with exit code $EXIT_CODE"
    echo "****************************************************"
    exit $EXIT_CODE
else
    END_TIME=$(date "+%Y-%m-%d %H:%M:%S")
    echo " [SUCCESS] Operation completed successfully"
    echo "****************************************************"
    echo " End Time   : $END_TIME"
    echo " Preset     : $PRESET"
    echo " Action     : $ACTION"
    echo " Build Dir  : $BUILD_DIR"
    if [ "$ACTION" = "install" ]; then
        echo " Installed  : $ROOT_DIR/install/$PRESET"
    fi
    if [ "$ACTION" = "package" ]; then
        echo " Package    : $BUILD_DIR"
    fi
    echo "****************************************************"
fi

exit $EXIT_CODE