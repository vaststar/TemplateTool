# ==========================================
# macOS Post-Install Script
# ==========================================
# This script handles dylib and Qt deployment for macOS builds
# - Third-party dylibs are installed by CMake install(FILES) rules
# - Moves all dylibs to app bundle Frameworks directory
# - Runs macdeployqt for Qt dependencies
# - Verifies deployment results

set(INSTALL_PREFIX "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
set(APP_NAME "mainEntry")
set(BIN_DIR "${INSTALL_PREFIX}/bin")
set(APP_BUNDLE "${BIN_DIR}/${APP_NAME}.app")
set(DEPLOY_DIR "${APP_BUNDLE}/Contents/Frameworks")

message(STATUS "")
message(STATUS "========================================")
message(STATUS " macOS Dylib & Qt Deployment")
message(STATUS "========================================")
message(STATUS "Install Prefix  : ${INSTALL_PREFIX}")
message(STATUS "App Bundle      : ${APP_NAME}.app")
message(STATUS "Frameworks Dir  : ${DEPLOY_DIR}")
message(STATUS "Config          : ${BUILD_CONFIG_NAME}")
message(STATUS "========================================")
message(STATUS "")

# ==========================================
# Step 1: Move all dylibs to Frameworks
# ==========================================
message(STATUS "[1/4] Moving all dylibs to Frameworks...")

# Create Frameworks directory if not exists
if(NOT EXISTS "${DEPLOY_DIR}")
    file(MAKE_DIRECTORY "${DEPLOY_DIR}")
    message(STATUS "  ✓ Created Frameworks directory")
else()
    message(STATUS "  ✓ Frameworks directory exists")
endif()

# Find all .dylib files in bin directory
file(GLOB DYLIB_FILES "${BIN_DIR}/*.dylib")

set(MOVED_COUNT 0)
set(MOVE_SKIPPED_COUNT 0)
set(MOVE_FAILED_COUNT 0)

if(NOT DYLIB_FILES)
    message(STATUS "  ⊘ No .dylib files found in ${BIN_DIR}")
else()
    foreach(dylib_path ${DYLIB_FILES})
        if(NOT EXISTS "${dylib_path}")
            math(EXPR MOVE_SKIPPED_COUNT "${MOVE_SKIPPED_COUNT} + 1")
            continue()
        endif()

        get_filename_component(dylib_name "${dylib_path}" NAME)
        set(dest_path "${DEPLOY_DIR}/${dylib_name}")

        # Remove existing file to avoid RENAME failure
        if(EXISTS "${dest_path}")
            file(REMOVE "${dest_path}")
        endif()

        # Move dylib to Frameworks directory
        file(RENAME "${dylib_path}" "${dest_path}")
        
        if(EXISTS "${dest_path}")
            message(STATUS "  ✓ Moved: ${dylib_name}")
            math(EXPR MOVED_COUNT "${MOVED_COUNT} + 1")
        else()
            message(STATUS "  ✗ Failed: ${dylib_name}")
            math(EXPR MOVE_FAILED_COUNT "${MOVE_FAILED_COUNT} + 1")
        endif()
    endforeach()
endif()

message(STATUS "")
message(STATUS "  Dylibs Summary:")
message(STATUS "    Moved   : ${MOVED_COUNT}")
message(STATUS "    Skipped : ${MOVE_SKIPPED_COUNT}")
message(STATUS "    Failed  : ${MOVE_FAILED_COUNT}")

if(MOVE_FAILED_COUNT GREATER 0)
    message(WARNING "  Some dylibs failed to move!")
endif()

message(STATUS "")

# ==========================================
# Step 2: Finding Qt installation
# ==========================================
message(STATUS "[2/4] Finding Qt installation...")

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
    message(WARNING "  Qt6_DIR not found, skipping macdeployqt")
    set(SKIP_QT_DEPLOY TRUE)
else()
    set(SKIP_QT_DEPLOY FALSE)
    # Normalize path
    string(REPLACE "\\" "/" QT_DIR "${QT_DIR}")
    string(REPLACE "/lib/cmake/Qt6" "" QT_ROOT "${QT_DIR}")
    set(MACDEPLOYQT "${QT_ROOT}/bin/macdeployqt")

    if(NOT EXISTS "${MACDEPLOYQT}")
        message(WARNING "  macdeployqt not found at: ${MACDEPLOYQT}")
        set(SKIP_QT_DEPLOY TRUE)
    else()
        message(STATUS "  ✓ Found Qt at: ${QT_ROOT}")
        message(STATUS "  ✓ Found macdeployqt: ${MACDEPLOYQT}")
    endif()
endif()

message(STATUS "")

# ==========================================
# Step 3: Running macdeployqt
# ==========================================
message(STATUS "[3/4] Running macdeployqt...")

if(SKIP_QT_DEPLOY)
    message(STATUS "  ⊘ Skipped (Qt not found)")
else()
    if(NOT EXISTS "${APP_BUNDLE}")
        message(WARNING "  App bundle not found: ${APP_BUNDLE}")
    else()
        # Find QML source directory
        set(QML_SOURCE_DIR "${TEMPLATE_TOOL_SOURCE_DIR}/product/application/qt")
        
        set(DEPLOY_CMD "${MACDEPLOYQT}" "${APP_BUNDLE}" "-verbose=1")
        
        if(EXISTS "${QML_SOURCE_DIR}")
            list(APPEND DEPLOY_CMD "-qmldir=${QML_SOURCE_DIR}")
            message(STATUS "  Using QML directory: ${QML_SOURCE_DIR}")
        endif()
        
        message(STATUS "  Running: macdeployqt ${APP_NAME}.app ...")
        
        execute_process(
            COMMAND ${DEPLOY_CMD}
            RESULT_VARIABLE DEPLOY_RESULT
            OUTPUT_VARIABLE DEPLOY_OUTPUT
            ERROR_VARIABLE DEPLOY_ERROR
        )
        
        if(DEPLOY_RESULT EQUAL 0)
            message(STATUS "  ✓ macdeployqt completed successfully")
        else()
            message(WARNING "  ✗ macdeployqt failed with code ${DEPLOY_RESULT}")
            if(DEPLOY_ERROR)
                message(WARNING "    Error: ${DEPLOY_ERROR}")
            endif()
        endif()
    endif()
endif()

message(STATUS "")

# ==========================================
# Step 4: Checking deployment results
# ==========================================
message(STATUS "[4/4] Checking deployment results...")

# Count frameworks
file(GLOB FRAMEWORK_DIRS "${DEPLOY_DIR}/*.framework")
list(LENGTH FRAMEWORK_DIRS FRAMEWORK_COUNT)
message(STATUS "  ✓ Qt Frameworks: ${FRAMEWORK_COUNT}")

# Count dylibs
file(GLOB DYLIB_COUNT_FILES "${DEPLOY_DIR}/*.dylib")
list(LENGTH DYLIB_COUNT_FILES DYLIB_TOTAL_COUNT)
message(STATUS "  ✓ Dylibs in Frameworks: ${DYLIB_TOTAL_COUNT}")

# Check PlugIns
set(PLUGINS_DIR "${APP_BUNDLE}/Contents/PlugIns")
if(EXISTS "${PLUGINS_DIR}")
    file(GLOB PLUGIN_DIRS "${PLUGINS_DIR}/*")
    list(LENGTH PLUGIN_DIRS PLUGIN_COUNT)
    message(STATUS "  ✓ Qt plugin directories: ${PLUGIN_COUNT}")
endif()

# Check QML
set(QML_DEPLOY_DIR "${APP_BUNDLE}/Contents/Resources/qml")
if(EXISTS "${QML_DEPLOY_DIR}")
    file(GLOB_RECURSE QML_FILES "${QML_DEPLOY_DIR}/*")
    list(LENGTH QML_FILES QML_FILE_COUNT)
    message(STATUS "  ✓ QML files deployed: ${QML_FILE_COUNT}")
endif()

message(STATUS "")
message(STATUS "========================================")
message(STATUS " Post-install completed!")
message(STATUS " App Bundle: ${APP_BUNDLE}")
message(STATUS " Frameworks: ${DEPLOY_DIR}")
message(STATUS "========================================")
