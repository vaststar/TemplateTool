# =============================================================================
# OpenSSL - Multi-Platform Wrapper
# =============================================================================
# Provides unified OpenSSL interface for all platforms.
# Each platform has its own CMakeLists.txt in the corresponding subdirectory.
#
# Supported platforms:
# - Windows (x64, x86, arm64)
# - macOS (Intel, Apple Silicon)
# - iOS (arm64 device, arm64 simulator via XCFramework)
# - Linux (x64)
# - Android (arm64-v8a, armeabi-v7a, x86_64, x86)
# =============================================================================

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
    message(WARNING "[OpenSSL] Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
