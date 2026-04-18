include(BuildModule)

set(MODULE_MODULE_NAME ScreenRecordingUtils)
set(MODULE_IDE_FOLDER framework/utilities/platform)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Utilities/ScreenRecordingUtils/ScreenRecordingUtils.h
)
set(MODULE_SOURCE_PRIVATE
    src/ScreenRecordingUtils.cpp
)
if (WIN32)
    list(APPEND MODULE_SOURCE_PRIVATE
    src/ScreenRecorder_Win.h
    src/ScreenRecorder_Win.cpp
    src/ScreenRecorderUtils_Win.cpp
    src/WasapiLoopbackCapture.h
    src/WasapiLoopbackCapture.cpp)
elseif (APPLE)
    list(APPEND MODULE_SOURCE_PRIVATE
    src/ScreenRecorder_Mac.h
    src/ScreenRecorder_Mac.mm
    src/ScreenRecorderUtils_Mac.mm)
elseif (UNIX)
    list(APPEND MODULE_SOURCE_PRIVATE
    src/ScreenRecorder_Linux.h
    src/ScreenRecorder_Linux.cpp
    src/ScreenRecorderUtils_Linux.cpp)
endif()

set(MODULE_DEPENDENCIES_PUBLIC
    UtilitiesCommonFile
)

set(MODULE_DEPENDENCIES_PRIVATE
    LoggerUtils
    ProcessBridgeUtils
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

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_UTILITIES_DEFINITIONS}
TARGET_ADD_DEPENDENCIES ${MODULE_NON_LINK_DEPENDENCIES}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
