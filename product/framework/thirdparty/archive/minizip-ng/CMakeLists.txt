# =============================================================================
# Archive Library - minizip-ng (Prebuilt)
# =============================================================================
# Prebuilt minizip-ng with DEFLATE compression (zlib statically linked)
# Built via: .github/workflows/build-minizip-all-platforms.yml
# Source: https://github.com/zlib-ng/minizip-ng
# =============================================================================

if(WIN32)
    add_subdirectory(minizip-ng/windows)
elseif(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        add_subdirectory(minizip-ng/ios)
    else()
        add_subdirectory(minizip-ng/macosx)
    endif()
elseif(ANDROID)
    add_subdirectory(minizip-ng/android)
elseif(UNIX)
    add_subdirectory(minizip-ng/linux)
else()
    message(FATAL_ERROR "[minizip] Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
