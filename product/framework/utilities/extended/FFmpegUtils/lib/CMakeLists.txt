include(BuildModule)

set(MODULE_MODULE_NAME FFmpegLibUtils)
set(MODULE_IDE_FOLDER framework/utilities/media)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/utilities/FFmpegUtils/lib/FFmpegExport.h
    include/ucf/utilities/FFmpegUtils/lib/FFmpegTypes.h
    include/ucf/utilities/FFmpegUtils/lib/FFmpegLib.h
    include/ucf/utilities/FFmpegUtils/lib/FFmpegSession.h
    include/ucf/utilities/FFmpegUtils/Core.h
    include/ucf/utilities/FFmpegUtils/core/ErrorHandler.h
    include/ucf/utilities/FFmpegUtils/core/TranscodeSession.h
    include/ucf/utilities/FFmpegUtils/core/MediaMetadata.h
)
set(MODULE_SOURCE_PRIVATE
    src/FFmpegLibLogger.h
    src/FFmpegInternal.h
    src/codecs/AudioDecoder.h
    src/codecs/AudioDecoder.cpp
    src/codecs/AudioEncoder.h
    src/codecs/AudioEncoder.cpp
    src/codecs/VideoDecoder.h
    src/codecs/VideoDecoder.cpp
    src/containers/Demuxer.h
    src/containers/Demuxer.cpp
    src/containers/Muxer.h
    src/containers/Muxer.cpp
    src/filters/Resampler.h
    src/filters/Resampler.cpp
    src/filters/Scaler.h
    src/filters/Scaler.cpp
    src/FFmpegLib.cpp
    src/FFmpegSession.cpp
    src/core/ErrorHandler.cpp
    src/core/TranscodeSession.cpp
    src/core/MediaMetadata.cpp
    src/core/Core.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC
)
set(MODULE_DEPENDENCIES_PRIVATE
    LoggerUtils
    ffmpeg_lib
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (UTILITY_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS FFMPEG_LIB_UTILS_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS FFMPEG_LIB_UTILS_SHARED)
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
IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
