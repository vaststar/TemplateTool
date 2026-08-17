#!/bin/bash
set -uo pipefail

# ==========================================
# Basic configuration
# ==========================================
if [[ -z "${1:-}" ]]; then
    case "$(uname -s)" in
        Linux*)  DEFAULT_PRESET="linux-release" ;;
        Darwin*) DEFAULT_PRESET="macos-release" ;;
        *)       DEFAULT_PRESET="macos-release" ;;
    esac
else
    DEFAULT_PRESET="$1"
fi
PRESET="$DEFAULT_PRESET"
ACTION="${2:-all}"
EXIT_CODE=0

VERBOSE_CONFIG=0
VERBOSE_BUILD=0
for option in "${@:3}"; do
    case "$option" in
        --verbose-config)
            VERBOSE_CONFIG=1
            ;;
        --verbose-build)
            VERBOSE_BUILD=1
            ;;
        --verbose)
            VERBOSE_CONFIG=1
            VERBOSE_BUILD=1
            ;;
        *)
            echo "[ERROR] Unknown option: $option"
            echo "        Run './build.sh --help' for usage."
            exit 1
            ;;
    esac
done

# Handle help request
if [[ "$PRESET" == "help" || "$PRESET" == "--help" || "$PRESET" == "-h" ]]; then
    echo ""
    echo "Usage: ./build.sh [PRESET] [ACTION] [OPTIONS...]"
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
    echo "  install-only Install without rebuilding (cmake --install)"
    echo "  package      Build and create package"
    echo "  test         Build and run tests"
    echo "  all          Configure and build (default)"
    echo ""
    echo "OPTIONS:"
    echo "  --verbose-config  Show detailed TemplateTool CMake configuration"
    echo "  --verbose-build   Show complete compile and link commands"
    echo "  --verbose         Enable both configuration and build details"
    echo ""
    echo "Examples:"
    echo "  ./build.sh                                # Default: macos-release all"
    echo "  ./build.sh macos-debug build              # Debug build only"
    echo "  ./build.sh macos-release install          # Release build and install"
    echo "  ./build.sh linux-release test             # Run tests"
    echo "  ./build.sh macos-release all --verbose    # Configure and build verbosely"
    echo ""
    exit 0
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
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
echo " Config log : $([[ $VERBOSE_CONFIG -eq 1 ]] && echo ON || echo OFF)"
echo " Build log  : $([[ $VERBOSE_BUILD -eq 1 ]] && echo ON || echo OFF)"
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

run_configure() {
    local configure_args=(
        --preset "$PRESET"
        --graphviz="$GRAPHVIZ_FILE"
    )
    if [ $VERBOSE_CONFIG -eq 1 ]; then
        configure_args+=("-DTT_CMAKE_VERBOSE_CONFIG=ON")
    fi
    "$CMAKE" "${configure_args[@]}"
}

run_build() {
    local build_args=(--build --preset "$PRESET")
    if [ $VERBOSE_BUILD -eq 1 ]; then
        build_args+=(--verbose)
    fi
    "$CMAKE" "${build_args[@]}" "$@"
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
        run_configure
        local configure_result=$?
        if [ $configure_result -ne 0 ]; then
            echo ""
            echo "[ERROR] Configuration failed, cannot proceed."
            return $configure_result
        fi
        echo ""
    fi

    return 0
}

cleanup_macos_cpack_dragndrop() {
    if [[ "$(uname -s)" != "Darwin" ]]; then
        return
    fi

    if [[ "$PRESET" != *"macos"* ]]; then
        return
    fi

    local dragndrop_dir="$BUILD_DIR/_CPack_Packages/Darwin/DragNDrop"
    local temp_dmg="$dragndrop_dir/temp.dmg"

    echo ""
    echo "[Pre-package] Cleaning stale macOS CPack DMG artifacts..."
    echo "----------------------------------------------------"

    if command -v hdiutil >/dev/null 2>&1; then
        while IFS= read -r disk_dev; do
            [ -z "$disk_dev" ] && continue
            echo "  Detaching stale image: $disk_dev"
            hdiutil detach "$disk_dev" -force >/dev/null 2>&1 || true
        done < <(hdiutil info 2>/dev/null | awk '
            /^image-path/ {img=$3}
            /^\/dev\/disk[0-9]+/ && img ~ /_CPack_Packages\/Darwin\/DragNDrop\/temp\.dmg$/ {print $1}
        ')
    fi

    if [ -f "$temp_dmg" ]; then
        echo "  Removing stale temp image: $temp_dmg"
        rm -f "$temp_dmg"
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
    run_configure
    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 0 ]; then
        echo ""
        echo "[Step 2/2] Building project..."
        echo "----------------------------------------------------"
        run_build
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
        run_configure
        EXIT_CODE=$?
        ;;

    build)
        echo ""
        echo "[Step 1/1] Building project..."
        echo "----------------------------------------------------"
        ensure_configured
        EXIT_CODE=$?
        if [ $EXIT_CODE -eq 0 ]; then
            echo "  Preset    : $PRESET"
            echo "  Build Dir : $BUILD_DIR"
            echo ""
            run_build
            EXIT_CODE=$?
        fi
        ;;

    rebuild)
        echo ""
        echo "[Step 1/3] Cleaning previous build..."
        echo "----------------------------------------------------"
        if is_configured; then
            echo "  Cleaning $BUILD_DIR"
            run_build --target clean
            EXIT_CODE=$?
        else
            echo "  No previous build found, skipping clean."
            EXIT_CODE=0
        fi
        if [ $EXIT_CODE -eq 0 ]; then
            echo ""
            echo "[Step 2/3] Configuring project..."
            echo "----------------------------------------------------"
            ensure_graphviz_dir
            run_configure
            EXIT_CODE=$?
        fi
        if [ $EXIT_CODE -eq 0 ]; then
            echo ""
            echo "[Step 3/3] Building project..."
            echo "----------------------------------------------------"
            run_build
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
            run_build --target clean
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
        EXIT_CODE=$?
        if [ $EXIT_CODE -eq 0 ]; then
            echo "  Preset     : $PRESET"
            echo "  Build Dir  : $BUILD_DIR"
            echo "  Install to : $ROOT_DIR/install/$PRESET"
            echo ""
            run_build --target install
            EXIT_CODE=$?
        fi
        ;;

    install-only)
        echo ""
        echo "[Step 1/1] Installing (skip build)..."
        echo "----------------------------------------------------"
        if ! is_configured; then
            echo "  [ERROR] Project not configured. Run './build.sh $PRESET configure' first."
            EXIT_CODE=1
        else
            # Detect build type from preset name
            BUILD_CONFIG="Release"
            if [[ "$PRESET" == *"-debug"* ]]; then
                BUILD_CONFIG="Debug"
            fi
            echo "  Build Dir  : $BUILD_DIR"
            echo "  Install to : $ROOT_DIR/install/$PRESET"
            echo ""
            "$CMAKE" --install "$BUILD_DIR" --config "$BUILD_CONFIG"
            EXIT_CODE=$?
        fi
        ;;

    package)
        echo ""
        echo "[Step 1/2] Building project..."
        echo "----------------------------------------------------"
        ensure_configured
        EXIT_CODE=$?
        if [ $EXIT_CODE -eq 0 ]; then
            echo "  Preset    : $PRESET"
            echo "  Build Dir : $BUILD_DIR"
            echo ""
            run_build
            EXIT_CODE=$?
        fi
        if [ $EXIT_CODE -eq 0 ]; then
            cleanup_macos_cpack_dragndrop
            echo ""
            echo "[Step 2/2] Creating package..."
            echo "----------------------------------------------------"
            cpack --preset "$PRESET" -B "$BUILD_DIR"
            EXIT_CODE=$?
        fi
        ;;

    test)
        echo ""
        echo "[Step 1/2] Building project..."
        echo "----------------------------------------------------"
        ensure_configured
        EXIT_CODE=$?
        if [ $EXIT_CODE -eq 0 ]; then
            echo "  Preset    : $PRESET"
            echo ""
            run_build
            EXIT_CODE=$?
        fi
        if [ $EXIT_CODE -eq 0 ]; then
            echo ""
            echo "[Step 2/2] Running tests..."
            echo "----------------------------------------------------"
            ctest --preset "$PRESET"
            EXIT_CODE=$?
        fi
        ;;

    all)
        do_all
        ;;

    *)
        echo "[ERROR] Unknown action: $ACTION"
        echo "[INFO] Valid actions: configure, build, rebuild, clean, install, install-only, package, test, all"
        EXIT_CODE=1
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
    if [ "$ACTION" = "install" ] || [ "$ACTION" = "install-only" ]; then
        echo " Installed  : $ROOT_DIR/install/$PRESET"
    fi
    if [ "$ACTION" = "package" ]; then
        echo " Package    : $BUILD_DIR"
    fi
    echo "****************************************************"
fi

exit $EXIT_CODE
