#!/bin/sh -e

ROOT_DIR="$(pwd)"
SOURCE_DIR="${ROOT_DIR}"
BUILD_PATH="${ROOT_DIR}/build"
OUTPUT_DIR="${ROOT_DIR}/output/build"
GRAPHVIZ_FILE="${BUILD_PATH}/cmake_graph/cmake_graph.dot"
OSX_SYSROOT="$(xcrun --sdk macosx --show-sdk-path)"

CMAKE="${ROOT_DIR}/tools/platforms/macosx/universal/cmake/CMake.app/Contents/bin/cmake"

echo "****************************************************"
echo "ENABLE_PCH is ${ENABLE_PCH}"
echo "PCH_ENABLE_PARAMETER is ${PCH_ENABLE_PARAMETER}"
echo "****************************************************"

${CMAKE} --version

${CMAKE} -S "${SOURCE_DIR}" \
         -B "${BUILD_PATH}" \
         -G"Xcode" \
         -T buildsystem=12 \
         --graphviz="${GRAPHVIZ_FILE}" \
         -DCMAKE_OSX_ARCHITECTURES="x86_64" \
         -DCLANG_DEBUG_INFORMATION_LEVEL_LINE_TABLES_ONLY=YES \
         -DCMAKE_OSX_SYSROOT="${OSX_SYSROOT}" \
         -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${OUTPUT_DIR}" \
         -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="${OUTPUT_DIR}/lib" \
         -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="${OUTPUT_DIR}/lib" \
         -DCMAKE_INSTALL_BINDIR="." \
         -DCMAKE_INSTALL_PREFIX="${BUILD_PATH}/output" \
          $@
