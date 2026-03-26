# ==========================================
# Linux Post-Install Script
# ==========================================
# This script handles .so deployment and Qt deployment for Linux builds
# - Moves all .so files to lib/ next to the binary
# - Deploys runtime payloads (proxy_addon, ffmpeg, etc.)
# - Runs linuxdeployqt or manual Qt plugin copy for Qt dependencies
# - Verifies deployment results

set(INSTALL_PREFIX "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
set(BIN_DIR "${INSTALL_PREFIX}/bin")
set(LIB_DIR "${INSTALL_PREFIX}/lib")

message(STATUS "")
message(STATUS "========================================")
message(STATUS " Linux .so & Qt Deployment")
message(STATUS "========================================")
message(STATUS "Install Prefix  : ${INSTALL_PREFIX}")
message(STATUS "Bin Dir         : ${BIN_DIR}")
message(STATUS "Lib Dir         : ${LIB_DIR}")
message(STATUS "Config          : ${BUILD_CONFIG_NAME}")
message(STATUS "========================================")
message(STATUS "")

# ==========================================
# Step 1: Move all .so files to lib/
# ==========================================
message(STATUS "[1/4] Organizing shared libraries...")

if(NOT EXISTS "${LIB_DIR}")
    file(MAKE_DIRECTORY "${LIB_DIR}")
    message(STATUS "  Created lib directory: ${LIB_DIR}")
endif()

file(GLOB SO_FILES "${BIN_DIR}/*.so" "${BIN_DIR}/*.so.*")

set(MOVED_COUNT 0)
set(MOVE_FAILED_COUNT 0)

if(NOT SO_FILES)
    message(STATUS "  No .so files found in ${BIN_DIR}")
else()
    foreach(so_path ${SO_FILES})
        if(NOT EXISTS "${so_path}")
            continue()
        endif()

        get_filename_component(so_name "${so_path}" NAME)
        set(dest_path "${LIB_DIR}/${so_name}")

        if(EXISTS "${dest_path}")
            file(REMOVE "${dest_path}")
        endif()

        file(RENAME "${so_path}" "${dest_path}")

        if(EXISTS "${dest_path}")
            message(STATUS "  Moved: ${so_name}")
            math(EXPR MOVED_COUNT "${MOVED_COUNT} + 1")
        else()
            message(STATUS "  Failed: ${so_name}")
            math(EXPR MOVE_FAILED_COUNT "${MOVE_FAILED_COUNT} + 1")
        endif()
    endforeach()
endif()

message(STATUS "")
message(STATUS "  .so Summary:")
message(STATUS "    Moved   : ${MOVED_COUNT}")
message(STATUS "    Failed  : ${MOVE_FAILED_COUNT}")

if(MOVE_FAILED_COUNT GREATER 0)
    message(WARNING "  Some .so files failed to move!")
endif()

message(STATUS "")

# ==========================================
# Step 2: Deploy runtime payloads
# ==========================================
message(STATUS "[2/4] Deploying runtime payloads...")

# Deploy directory payloads (e.g. proxy_addon/)
foreach(payload_dir ${RESOURCE_DIR_PAYLOADS})
    set(payload_src "${BIN_DIR}/${payload_dir}")
    set(payload_dst "${LIB_DIR}/${payload_dir}")
    if(EXISTS "${payload_src}")
        if(EXISTS "${payload_dst}")
            file(REMOVE_RECURSE "${payload_dst}")
        endif()
        file(RENAME "${payload_src}" "${payload_dst}")
        message(STATUS "  Deployed directory payload: ${payload_dir}")
    else()
        message(STATUS "  Directory payload not found: ${payload_src}")
    endif()
endforeach()

# Deploy file payloads (e.g. ffmpeg, ffprobe)
foreach(payload_file ${RESOURCE_FILE_PAYLOADS})
    set(payload_src "${BIN_DIR}/${payload_file}")
    set(payload_dst "${LIB_DIR}/${payload_file}")
    if(EXISTS "${payload_src}")
        if(EXISTS "${payload_dst}")
            file(REMOVE "${payload_dst}")
        endif()
        file(RENAME "${payload_src}" "${payload_dst}")
        # Ensure executable permission
        execute_process(COMMAND chmod +x "${payload_dst}" ERROR_QUIET)
        message(STATUS "  Deployed file payload: ${payload_file}")
    else()
        message(STATUS "  File payload not found: ${payload_src}")
    endif()
endforeach()

message(STATUS "")

# ==========================================
# Step 3: Qt Deployment
# ==========================================
message(STATUS "[3/4] Qt deployment...")

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

set(SKIP_QT_DEPLOY TRUE)

if(NOT QT_DIR)
    message(WARNING "  Qt6_DIR not found, skipping Qt deployment")
else()
    string(REPLACE "\\" "/" QT_DIR "${QT_DIR}")
    string(REPLACE "/lib/cmake/Qt6" "" QT_ROOT "${QT_DIR}")
    message(STATUS "  Found Qt at: ${QT_ROOT}")

    # Try to find linuxdeployqt or linuxdeployqt-continuous
    find_program(LINUXDEPLOYQT_EXE NAMES linuxdeployqt linuxdeployqt-continuous)

    if(LINUXDEPLOYQT_EXE)
        message(STATUS "  Found linuxdeployqt: ${LINUXDEPLOYQT_EXE}")
        set(SKIP_QT_DEPLOY FALSE)

        set(QML_SOURCE_DIR "${TEMPLATE_TOOL_SOURCE_DIR}/product/application/qt")
        set(DEPLOY_CMD
            "${LINUXDEPLOYQT_EXE}"
            "${BIN_DIR}/${APP_NAME}"
            "-verbose=1"
            "-no-translations"
            "-bundle-non-qt-libs"
        )

        if(EXISTS "${QML_SOURCE_DIR}")
            list(APPEND DEPLOY_CMD "-qmldir=${QML_SOURCE_DIR}")
            message(STATUS "  Using QML directory: ${QML_SOURCE_DIR}")
        endif()

        message(STATUS "  Running linuxdeployqt...")

        execute_process(
            COMMAND ${DEPLOY_CMD}
            RESULT_VARIABLE DEPLOY_RESULT
            OUTPUT_VARIABLE DEPLOY_OUTPUT
            ERROR_VARIABLE DEPLOY_ERROR
        )

        if(DEPLOY_RESULT EQUAL 0)
            message(STATUS "  linuxdeployqt completed successfully")
        else()
            message(WARNING "  linuxdeployqt failed with code ${DEPLOY_RESULT}")
            if(DEPLOY_ERROR)
                message(WARNING "    Error: ${DEPLOY_ERROR}")
            endif()
            message(STATUS "  Falling back to manual Qt plugin copy...")
            set(SKIP_QT_DEPLOY TRUE)
        endif()
    else()
        message(STATUS "  linuxdeployqt not found, performing manual Qt plugin copy")
    endif()

    # Manual fallback: copy essential Qt plugins
    if(SKIP_QT_DEPLOY)
        set(QT_PLUGINS_SRC "${QT_ROOT}/plugins")
        set(QT_QML_SRC "${QT_ROOT}/qml")
        set(PLUGINS_DST "${BIN_DIR}")

        # Essential plugin directories for a Qt Quick application
        set(ESSENTIAL_PLUGINS
            platforms
            platforminputcontexts
            imageformats
            xcbglintegrations
            wayland-decoration-client
            wayland-graphics-integration-client
            wayland-shell-integration
            tls
            networkinformation
        )

        set(PLUGIN_COPIED_COUNT 0)

        foreach(plugin_dir ${ESSENTIAL_PLUGINS})
            set(src "${QT_PLUGINS_SRC}/${plugin_dir}")
            set(dst "${PLUGINS_DST}/${plugin_dir}")
            if(EXISTS "${src}")
                if(NOT EXISTS "${dst}")
                    file(MAKE_DIRECTORY "${dst}")
                endif()
                file(GLOB plugin_files "${src}/*.so")
                foreach(pf ${plugin_files})
                    get_filename_component(pf_name "${pf}" NAME)
                    file(COPY "${pf}" DESTINATION "${dst}")
                    math(EXPR PLUGIN_COPIED_COUNT "${PLUGIN_COPIED_COUNT} + 1")
                endforeach()
                message(STATUS "  Copied plugin: ${plugin_dir}")
            endif()
        endforeach()

        # Copy QML modules if available
        if(EXISTS "${QT_QML_SRC}")
            set(QML_DST "${PLUGINS_DST}/qml")
            if(NOT EXISTS "${QML_DST}")
                file(COPY "${QT_QML_SRC}/" DESTINATION "${QML_DST}")
                message(STATUS "  Copied QML modules")
            endif()
        endif()

        message(STATUS "  Manual plugin copy: ${PLUGIN_COPIED_COUNT} plugins copied")
    endif()
endif()

message(STATUS "")

# ==========================================
# Step 4: Create launcher script
# ==========================================
message(STATUS "[4/4] Creating launcher script...")

# Create a wrapper script that sets LD_LIBRARY_PATH
set(LAUNCHER_SCRIPT "${INSTALL_PREFIX}/${APP_NAME}.sh")
file(WRITE "${LAUNCHER_SCRIPT}"
"#!/bin/bash
# Auto-generated launcher for ${APP_NAME}
# Sets up library paths for proper .so resolution

SCRIPT_DIR=\"$(cd \"$(dirname \"$0\")\" && pwd)\"
export LD_LIBRARY_PATH=\"$SCRIPT_DIR/lib:$SCRIPT_DIR/bin:$LD_LIBRARY_PATH\"
export QT_PLUGIN_PATH=\"$SCRIPT_DIR/bin:$QT_PLUGIN_PATH\"
export QML2_IMPORT_PATH=\"$SCRIPT_DIR/bin/qml:$QML2_IMPORT_PATH\"

exec \"$SCRIPT_DIR/bin/${APP_NAME}\" \"$@\"
")

execute_process(COMMAND chmod +x "${LAUNCHER_SCRIPT}" ERROR_QUIET)
message(STATUS "  Created launcher: ${APP_NAME}.sh")

# ==========================================
# Checking deployment results
# ==========================================
message(STATUS "")
message(STATUS "  Deployment Summary:")

# Count .so files in lib/
file(GLOB LIB_SO_FILES "${LIB_DIR}/*.so" "${LIB_DIR}/*.so.*")
list(LENGTH LIB_SO_FILES LIB_SO_COUNT)
message(STATUS "    .so files in lib/: ${LIB_SO_COUNT}")

# Check plugins
if(EXISTS "${BIN_DIR}/platforms")
    file(GLOB PLATFORM_PLUGINS "${BIN_DIR}/platforms/*.so")
    list(LENGTH PLATFORM_PLUGINS PLATFORM_COUNT)
    message(STATUS "    Qt platform plugins: ${PLATFORM_COUNT}")
endif()

# Check QML
if(EXISTS "${BIN_DIR}/qml")
    file(GLOB_RECURSE QML_FILES "${BIN_DIR}/qml/*")
    list(LENGTH QML_FILES QML_COUNT)
    message(STATUS "    QML files deployed: ${QML_COUNT}")
endif()

# Check payloads
foreach(payload_dir ${RESOURCE_DIR_PAYLOADS})
    if(EXISTS "${LIB_DIR}/${payload_dir}")
        message(STATUS "    Resource directory deployed: ${payload_dir}")
    endif()
endforeach()

foreach(payload_file ${RESOURCE_FILE_PAYLOADS})
    if(EXISTS "${LIB_DIR}/${payload_file}")
        message(STATUS "    Resource file deployed: ${payload_file}")
    endif()
endforeach()

message(STATUS "")
message(STATUS "========================================")
message(STATUS " Post-install completed!")
message(STATUS " Binary  : ${BIN_DIR}/${APP_NAME}")
message(STATUS " Libs    : ${LIB_DIR}")
message(STATUS " Launcher: ${LAUNCHER_SCRIPT}")
message(STATUS "========================================")
