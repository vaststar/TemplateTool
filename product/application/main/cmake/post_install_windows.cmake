# ==========================================
# Windows Post-Install Script
# ==========================================
# This script handles DLL deployment for Windows builds
# - Copies third-party DLLs
# - Runs windeployqt for Qt dependencies
# - Verifies deployment results

set(INSTALL_PREFIX "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
set(DEPLOY_DIR "${INSTALL_PREFIX}/bin")
set(THIRDPARTY_DIR "${TEMPLATE_TOOL_SOURCE_DIR}/product/framework/thirdparty")

message(STATUS "")
message(STATUS "========================================")
message(STATUS " Windows DLL Deployment")
message(STATUS "========================================")
message(STATUS "Install Prefix  : ${INSTALL_PREFIX}")
message(STATUS "Deploy Dir      : ${DEPLOY_DIR}")
message(STATUS "Source Dir      : ${TEMPLATE_TOOL_SOURCE_DIR}")
message(STATUS "ThirdParty Dir  : ${THIRDPARTY_DIR}")
message(STATUS "========================================")
message(STATUS "")

# ==========================================
# Step 1: Copy Third-Party DLLs
# ==========================================
message(STATUS "[1/5] Copying third-party DLLs...")


set(THIRDPARTY_DLLS
    "${THIRDPARTY_DIR}/openssl/windows/x64/bin/libcrypto-1_1-x64.dll"
    "${THIRDPARTY_DIR}/openssl/windows/x64/bin/libssl-1_1-x64.dll"
    "${THIRDPARTY_DIR}/opencv/windows/x64/vc16/bin/opencv_world4120.dll"
    "${THIRDPARTY_DIR}/curl/curl/windows/x64/release/bin/libcurl.dll"
)

set(COPIED_COUNT 0)
set(SKIPPED_COUNT 0)
set(FAILED_COUNT 0)

foreach(dll ${THIRDPARTY_DLLS})
    if(NOT EXISTS "${dll}")
        get_filename_component(dll_name "${dll}" NAME)
        message(STATUS "  ⊘ Skipped: ${dll_name} (not found)")
        math(EXPR SKIPPED_COUNT "${SKIPPED_COUNT} + 1")
        continue()
    endif()
    
    get_filename_component(dll_name "${dll}" NAME)
    
    # Skip system DLLs
    if(dll_name MATCHES "^(msvcp|vcruntime|ucrtbase|concrt)")
        message(STATUS "  ⊘ Skipped: ${dll_name} (system DLL)")
        math(EXPR SKIPPED_COUNT "${SKIPPED_COUNT} + 1")
        continue()
    endif()
    
    set(dest "${DEPLOY_DIR}/${dll_name}")
    
    # Remove existing file
    if(EXISTS "${dest}")
        file(REMOVE "${dest}")
    endif()
    
    # Copy DLL
    file(COPY "${dll}" DESTINATION "${DEPLOY_DIR}")
    
    if(EXISTS "${dest}")
        message(STATUS "  ✓ Copied: ${dll_name}")
        math(EXPR COPIED_COUNT "${COPIED_COUNT} + 1")
    else()
        message(STATUS "  ✗ Failed: ${dll_name}")
        math(EXPR FAILED_COUNT "${FAILED_COUNT} + 1")
    endif()
endforeach()

message(STATUS "")
message(STATUS "Third-party DLLs Summary:")
message(STATUS "  Copied  : ${COPIED_COUNT}")
message(STATUS "  Skipped : ${SKIPPED_COUNT}")
message(STATUS "  Failed  : ${FAILED_COUNT}")
message(STATUS "")

# ==========================================
# Step 2: Project DLLs are already in bin/
# ==========================================
message(STATUS "[2/5] Project DLLs are already in bin/")

file(GLOB PROJECT_DLLS "${DEPLOY_DIR}/*.dll")
list(LENGTH PROJECT_DLLS PROJECT_DLL_COUNT)
message(STATUS "  ✓ Found ${PROJECT_DLL_COUNT} project DLLs in bin/")

# ==========================================
# Step 3: Finding Qt installation
# ==========================================
message(STATUS "[3/5] Finding Qt installation...")

set(QT_DIR "$ENV{Qt6_DIR}")

if(NOT QT_DIR)
    set(CACHE_FILE "${CMAKE_BINARY_DIR}/CMakeCache.txt")
    if(EXISTS "${CACHE_FILE}")
        file(STRINGS "${CACHE_FILE}" QT6_DIR_LINE REGEX "Qt6_DIR:PATH=")
        if(QT6_DIR_LINE)
            string(REGEX REPLACE "Qt6_DIR:PATH=" "" QT_DIR "${QT6_DIR_LINE}")
            string(REGEX REPLACE "//.*$" "" QT_DIR "${QT6_DIR_LINE}")
        endif()
    endif()
endif()

if(NOT QT_DIR)
    message(WARNING "Qt6_DIR not found, skipping windeployqt")
    return()
endif()

string(REPLACE "/" "\\" QT_DIR "${QT_DIR}")
string(REPLACE "\\lib\\cmake\\Qt6" "" QT_ROOT "${QT_DIR}")
set(WINDEPLOYQT "${QT_ROOT}/bin/windeployqt.exe")

if(NOT EXISTS "${WINDEPLOYQT}")
    message(WARNING "windeployqt.exe not found at: ${WINDEPLOYQT}")
    return()
endif()

message(STATUS "  ✓ Found Qt at: ${QT_ROOT}")
message(STATUS "  ✓ Found windeployqt: ${WINDEPLOYQT}")

# ==========================================
# Step 4: Running windeployqt
# ==========================================
message(STATUS "[4/5] Running windeployqt...")

# Find QML source directory
set(QML_SOURCE_DIR "${TEMPLATE_TOOL_SOURCE_DIR}/product/application/qt")
if(EXISTS "${QML_SOURCE_DIR}")
    message(STATUS "  QML source directory: ${QML_SOURCE_DIR}")
else()
    message(STATUS "  QML source directory not found: ${QML_SOURCE_DIR}")
    set(QML_SOURCE_DIR "")
endif()

# DLLs that need Qt deployment
set(UI_DLLS "UIView.dll")

foreach(UI_DLL ${UI_DLLS})
    set(TARGET_DLL "${DEPLOY_DIR}/${UI_DLL}")
    
    if(NOT EXISTS "${TARGET_DLL}")
        message(STATUS "  ${UI_DLL} not found, skipping...")
        continue()
    endif()
    
    message(STATUS "  Running windeployqt for ${UI_DLL}...")
    
    set(DEPLOY_CMD 
        "${WINDEPLOYQT}"
        --release
        --no-translations
        --no-system-d3d-compiler
        --no-opengl-sw
        --no-compiler-runtime
        --dir "${DEPLOY_DIR}"
    )
    
    # Add qmldir parameter
    if(QML_SOURCE_DIR)
        list(APPEND DEPLOY_CMD --qmldir "${QML_SOURCE_DIR}")
        message(STATUS "    Using QML directory: ${QML_SOURCE_DIR}")
    endif()
    
    list(APPEND DEPLOY_CMD "${TARGET_DLL}")
    
    execute_process(
        COMMAND ${DEPLOY_CMD}
        RESULT_VARIABLE DEPLOY_RESULT
        OUTPUT_VARIABLE DEPLOY_OUTPUT
        ERROR_VARIABLE DEPLOY_ERROR
    )
    
    if(DEPLOY_RESULT EQUAL 0)
        message(STATUS "  ✓ windeployqt completed for ${UI_DLL}")
    else()
        message(WARNING "  ✗ windeployqt failed for ${UI_DLL} with code ${DEPLOY_RESULT}")
        if(DEPLOY_ERROR)
            message(WARNING "    Error: ${DEPLOY_ERROR}")
        endif()
    endif()
endforeach()

# ==========================================
# Step 5: Checking deployment results
# ==========================================
message(STATUS "[5/5] Checking deployment results...")

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