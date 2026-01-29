# =============================================================================
# OpenCV - Multi-Platform Wrapper
# =============================================================================
# Provides unified OpenCV interface for all platforms.
# Each platform has its own CMakeLists.txt in the corresponding subdirectory.
#
# Supported platforms:
# - Windows (x64, x86) - vc17
# - macOS (Intel, Apple Silicon)
# - iOS (arm64 device, arm64+x86_64 simulator via XCFramework)
# - Linux (x64)
# - Android (arm64-v8a, armeabi-v7a, x86_64, x86)
# =============================================================================

# =============================================================================
# Auto-extract large files from .zip archives
# =============================================================================
# Large binary files (>100MB) are stored as .zip to fit GitHub's file size limit.
# This function automatically extracts them at configure time if needed.
# =============================================================================
function(opencv_extract_if_zipped FILE_PATH)
    set(ZIP_PATH "${FILE_PATH}.zip")
    if(EXISTS "${ZIP_PATH}" AND NOT EXISTS "${FILE_PATH}")
        message(STATUS "[OpenCV] Extracting: ${FILE_PATH}")
        get_filename_component(DEST_DIR "${FILE_PATH}" DIRECTORY)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${ZIP_PATH}"
            WORKING_DIRECTORY "${DEST_DIR}"
            RESULT_VARIABLE EXTRACT_RESULT
        )
        if(NOT EXTRACT_RESULT EQUAL 0)
            message(WARNING "[OpenCV] Failed to extract: ${ZIP_PATH}")
        endif()
    endif()
endfunction()

# Extract platform-specific large files
if(ANDROID)
    opencv_extract_if_zipped("${CMAKE_CURRENT_SOURCE_DIR}/android/${ANDROID_ABI}/libs/libopencv_world.so")
elseif(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_CV_ARCH "x64")
    else()
        set(_CV_ARCH "x86")
    endif()
    opencv_extract_if_zipped("${CMAKE_CURRENT_SOURCE_DIR}/windows/${_CV_ARCH}/vc17/bin/opencv_world4130d.dll")
endif()

if(WIN32)
    add_subdirectory(windows)
elseif(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR CMAKE_SYSTEM_NAME STREQUAL "tvOS" OR CMAKE_SYSTEM_NAME STREQUAL "watchOS")
        add_subdirectory(ios)
    else()
        add_subdirectory(macosx)
    endif()
elseif(ANDROID)
    add_subdirectory(android)
elseif(UNIX)
    add_subdirectory(linux)
else()
    message(WARNING "[OpenCV] Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
