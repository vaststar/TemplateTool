# ==========================================
# Windows Post-Install Script
# ==========================================
# This script handles Qt deployment for Windows builds
# - Third-party DLLs are installed by CMake install(FILES) rules
# - Runs windeployqt for Qt dependencies
# - Verifies deployment results

set(INSTALL_PREFIX "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
set(DEPLOY_DIR "${INSTALL_PREFIX}/bin")

message(STATUS "")
message(STATUS "========================================")
message(STATUS " Windows Qt Deployment")
message(STATUS "========================================")
message(STATUS "Install Prefix  : ${INSTALL_PREFIX}")
message(STATUS "Deploy Dir      : ${DEPLOY_DIR}")
message(STATUS "Config          : ${BUILD_CONFIG_NAME}")
message(STATUS "========================================")
message(STATUS "")

# ==========================================
# Step 1: Finding Qt installation
# ==========================================
message(STATUS "[1/3] Finding Qt installation...")

set(QT_DIR "$ENV{Qt6_DIR}")

if(NOT QT_DIR)
    set(CACHE_FILE "${TEMPLATE_TOOL_BINARY_DIR}/CMakeCache.txt")
    if(EXISTS "${CACHE_FILE}")
        file(STRINGS "${CACHE_FILE}" QT6_DIR_LINE REGEX "Qt6_DIR:PATH=")
        if(QT6_DIR_LINE)
            string(REGEX REPLACE "Qt6_DIR:PATH=" "" QT_DIR "${QT6_DIR_LINE}")
            string(REGEX REPLACE "//.*$" "" QT_DIR "${QT_DIR}")
        endif()
    endif()
endif()

if(NOT QT_DIR)
    message(WARNING "  Qt6_DIR not found, skipping windeployqt")
    return()
endif()

# Normalize path: convert all backslashes to forward slashes
string(REPLACE "\\" "/" QT_DIR "${QT_DIR}")
string(REPLACE "/lib/cmake/Qt6" "" QT_ROOT "${QT_DIR}")
set(WINDEPLOYQT "${QT_ROOT}/bin/windeployqt.exe")

# Convert to native path for Windows
file(TO_NATIVE_PATH "${WINDEPLOYQT}" WINDEPLOYQT)

if(NOT EXISTS "${WINDEPLOYQT}")
    message(WARNING "  windeployqt.exe not found at: ${WINDEPLOYQT}")
    return()
endif()

message(STATUS "  ✓ Found Qt at: ${QT_ROOT}")
message(STATUS "  ✓ Found windeployqt: ${WINDEPLOYQT}")

message(STATUS "")

# ==========================================
# Step 2: Running windeployqt
# ==========================================
message(STATUS "[2/3] Running windeployqt...")

if(QML_SOURCE_DIR AND EXISTS "${QML_SOURCE_DIR}")
    message(STATUS "  QML source directory: ${QML_SOURCE_DIR}")
else()
    message(STATUS "  QML source directory not found: ${QML_SOURCE_DIR}")
    set(QML_SOURCE_DIR "")
endif()

if(BUILD_CONFIG_NAME MATCHES "^[Dd]ebug$")
    set(QT_DEPLOY_CONFIGURATION --debug)
else()
    set(QT_DEPLOY_CONFIGURATION --release)
endif()
message(STATUS "  Qt deployment configuration: ${QT_DEPLOY_CONFIGURATION}")

foreach(qt_deploy_file IN LISTS QT_DEPLOY_FILE_NAMES)
    set(qt_deploy_binary "${DEPLOY_DIR}/${qt_deploy_file}")

    if(NOT EXISTS "${qt_deploy_binary}")
        message(FATAL_ERROR
            "Qt deployment root not found: ${qt_deploy_binary}")
    endif()

    message(STATUS "  Running windeployqt for ${qt_deploy_file}...")

    set(DEPLOY_CMD
        "${WINDEPLOYQT}"
        ${QT_DEPLOY_CONFIGURATION}
        --no-translations
        --no-system-d3d-compiler
        --no-opengl-sw
        --no-compiler-runtime
        --dir "${DEPLOY_DIR}"
    )

    if(QML_SOURCE_DIR)
        list(APPEND DEPLOY_CMD --qmldir "${QML_SOURCE_DIR}")
        message(STATUS "    Using QML directory: ${QML_SOURCE_DIR}")
    endif()

    list(APPEND DEPLOY_CMD "${qt_deploy_binary}")

    execute_process(
        COMMAND ${DEPLOY_CMD}
        RESULT_VARIABLE DEPLOY_RESULT
        OUTPUT_VARIABLE DEPLOY_OUTPUT
        ERROR_VARIABLE DEPLOY_ERROR
    )

    if(DEPLOY_RESULT EQUAL 0)
        message(STATUS
            "  ✓ windeployqt completed for ${qt_deploy_file}")
    else()
        string(CONCAT deploy_error_message
            "windeployqt failed for ${qt_deploy_file} "
            "with code ${DEPLOY_RESULT}")
        if(DEPLOY_ERROR)
            string(APPEND deploy_error_message
                "\nError: ${DEPLOY_ERROR}")
        endif()
        message(FATAL_ERROR "${deploy_error_message}")
    endif()
endforeach()

message(STATUS "")

# ==========================================
# Step 3: Checking deployment results
# ==========================================
message(STATUS "[3/3] Checking deployment results...")

file(GLOB ALL_DLLS "${DEPLOY_DIR}/*.dll")
list(LENGTH ALL_DLLS TOTAL_DLL_COUNT)

file(GLOB QT_DLLS "${DEPLOY_DIR}/Qt6*.dll")
list(LENGTH QT_DLLS QT_DLL_COUNT)

message(STATUS "  ✓ Total DLLs in bin/: ${TOTAL_DLL_COUNT}")
message(STATUS "  ✓ Qt DLLs: ${QT_DLL_COUNT}")

if(EXISTS "${DEPLOY_DIR}/platforms")
    file(GLOB PLATFORM_PLUGINS "${DEPLOY_DIR}/platforms/*.dll")
    list(LENGTH PLATFORM_PLUGINS PLATFORM_COUNT)
    message(STATUS "  ✓ Qt platform plugins: ${PLATFORM_COUNT}")
endif()

if(EXISTS "${DEPLOY_DIR}/qml")
    file(GLOB_RECURSE QML_MODULES "${DEPLOY_DIR}/qml/*")
    list(LENGTH QML_MODULES QML_COUNT)
    message(STATUS "  ✓ QML modules deployed: ${QML_COUNT} files")
endif()

message(STATUS "")
message(STATUS "========================================")
message(STATUS " Post-install completed!")
message(STATUS " All DLLs location: ${DEPLOY_DIR}")
message(STATUS "========================================")
