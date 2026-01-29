# =============================================================================
# Archive Library - minizip-ng (Prebuilt)
# =============================================================================
# Prebuilt minizip-ng with DEFLATE compression (zlib statically linked)
# Built via: .github/workflows/build-minizip-all-platforms.yml
# Source: https://github.com/zlib-ng/minizip-ng
# =============================================================================

if(WIN32)
    add_subdirectory(minizip/windows)
elseif(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        add_subdirectory(minizip/ios)
    else()
        add_subdirectory(minizip/macosx)
    endif()
elseif(ANDROID)
    add_subdirectory(minizip/android)
elseif(UNIX)
    add_subdirectory(minizip/linux)
else()
    message(FATAL_ERROR "[minizip] Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
