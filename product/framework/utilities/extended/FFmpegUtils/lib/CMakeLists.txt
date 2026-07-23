include(BuildModule)

set(MODULE_MODULE_NAME FFmpegLibUtils)
set(MODULE_IDE_FOLDER framework/utilities/media)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Utilities/FFmpegUtils/Lib/FFmpegExport.h
    include/ucf/Utilities/FFmpegUtils/Lib/FFmpegTypes.h
    include/ucf/Utilities/FFmpegUtils/Lib/FFmpegLib.h
    include/ucf/Utilities/FFmpegUtils/Lib/FFmpegSession.h
    include/ucf/Utilities/FFmpegUtils/Core.h
    include/ucf/Utilities/FFmpegUtils/Core/ErrorHandler.h
    include/ucf/Utilities/FFmpegUtils/Core/TranscodeSession.h
    include/ucf/Utilities/FFmpegUtils/Core/MediaMetadata.h
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
    UtilitiesCommonFile
)
set(MODULE_DEPENDENCIES_PRIVATE
    LoggerUtils
    ffmpeg_lib
)

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
IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
