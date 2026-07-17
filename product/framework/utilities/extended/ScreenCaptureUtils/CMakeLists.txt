include(BuildModule)

set(MODULE_MODULE_NAME ScreenCaptureUtils)
set(MODULE_IDE_FOLDER framework/utilities/platform)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Utilities/ScreenCaptureUtils/ScreenCaptureUtils.h
)
set(MODULE_SOURCE_PRIVATE
    src/ScreenCaptureUtils.cpp
)
if (WIN32)
    list(APPEND MODULE_SOURCE_PRIVATE
    src/windows/ScreenCaptureUtils_Win.h
    src/windows/ScreenCaptureUtils_Win.cpp)
elseif (APPLE)
    list(APPEND MODULE_SOURCE_PRIVATE
    src/macos/ScreenCaptureUtils_Mac.h
    src/macos/ScreenCaptureUtils_Mac.mm)
elseif (UNIX)
    list(APPEND MODULE_SOURCE_PRIVATE
    src/linux/ScreenCaptureUtils_Linux.h
    src/linux/ScreenCaptureUtils_Linux.cpp
    src/linux/PortalFrameGrabber_Linux.h
    src/linux/PortalFrameGrabber_Linux.cpp)
endif()

set(MODULE_DEPENDENCIES_PUBLIC
    UtilitiesCommonFile
)

set(MODULE_DEPENDENCIES_PRIVATE
)
if (APPLE)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics)
    find_library(SCREENCAPTUREKIT_FRAMEWORK ScreenCaptureKit)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE
        ${COREGRAPHICS_FRAMEWORK}
        ${SCREENCAPTUREKIT_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        objc
    )
elseif (WIN32)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE
        dwmapi
        shcore
    )
elseif (UNIX)
    # Linux screen capture uses xdg-desktop-portal (via GDBus/GIO) plus
    # PipeWire to receive raw frames — no ffmpeg, no external screenshot tools.
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GIO REQUIRED gio-2.0 gio-unix-2.0)
    pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE
        ProcessBridgeUtils
        ${GIO_LIBRARIES}
        ${PIPEWIRE_LIBRARIES}
    )
endif()

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)
if (WIN32)
    list(APPEND MODULE_BUILD_PRIVATE_DIR src/windows)
elseif (APPLE)
    list(APPEND MODULE_BUILD_PRIVATE_DIR src/macos)
elseif (UNIX)
    list(APPEND MODULE_BUILD_PRIVATE_DIR
        src/linux
        ${GIO_INCLUDE_DIRS}
        ${PIPEWIRE_INCLUDE_DIRS})
endif()

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
