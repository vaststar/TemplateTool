#!/bin/sh -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_PATH="${ROOT_DIR}/build"

echo "****************************************************"
echo "Start build project on MacOSX"
echo "ROOT_DIR is ${ROOT_DIR}"
echo "****************************************************"
make -C "${BUILD_PATH}" -j 4 VERBOSE=1

echo "****************************************************"
echo "Finish build project on MacOSX"
echo "****************************************************"