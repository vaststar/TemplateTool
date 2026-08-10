include(BuildModule)

set(MODULE_MODULE_NAME ScreenRecordingUtils)
set(MODULE_IDE_FOLDER framework/utilities/platform)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtilsExport.h
    include/ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h
)
set(MODULE_SOURCE_PRIVATE
    src/ScreenRecordingUtils.cpp
)
if (WIN32)
    list(APPEND MODULE_SOURCE_PRIVATE
    src/windows/ScreenRecorder_Win.h
    src/windows/ScreenRecorder_Win.cpp
    src/windows/WasapiLoopbackCapture.h
    src/windows/WasapiLoopbackCapture.cpp
    src/windows/WinEncodingUtils.h
    src/windows/DShowDeviceHelper.h
    src/windows/DShowDeviceHelper.cpp)
elseif (APPLE)
    list(APPEND MODULE_SOURCE_PRIVATE
    src/macos/ScreenRecorder_Mac.h
    src/macos/ScreenRecorder_Mac.mm)
elseif (UNIX)
    list(APPEND MODULE_SOURCE_PRIVATE
    src/linux/ScreenRecorder_Linux.h
    src/linux/ScreenRecorder_Linux.cpp)
endif()

set(MODULE_DEPENDENCIES_PUBLIC
)

set(MODULE_DEPENDENCIES_PRIVATE
    LoggerUtils
    ProcessBridgeUtils
    FFmpegCliUtils
)

if (WIN32)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE
        ole32
        uuid
        strmiids
    )
elseif (APPLE)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics)
    find_library(COREAUDIO_FRAMEWORK CoreAudio)
    find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE
        ${COREGRAPHICS_FRAMEWORK}
        ${COREAUDIO_FRAMEWORK}
        ${AVFOUNDATION_FRAMEWORK}
        ${COREFOUNDATION_FRAMEWORK}
    )
endif()

# Ensure ffmpeg executables are copied before this module is built
set(MODULE_NON_LINK_DEPENDENCIES FFmpegRuntime)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)
if (WIN32)
    list(APPEND MODULE_BUILD_PRIVATE_DIR src/windows)
elseif (APPLE)
    list(APPEND MODULE_BUILD_PRIVATE_DIR src/macos)
elseif (UNIX)
    list(APPEND MODULE_BUILD_PRIVATE_DIR src/linux)
endif()

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (UTILITY_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS SCREEN_RECORDING_UTILS_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS SCREEN_RECORDING_UTILS_SHARED)
endif()

BuildModule(
${UTILITY_LIB_TYPE}
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
TARGET_ADD_DEPENDENCIES ${MODULE_NON_LINK_DEPENDENCIES}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
